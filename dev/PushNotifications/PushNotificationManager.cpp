﻿#include "pch.h"

#include "PushNotificationManager.h"
#include "Microsoft.Windows.PushNotifications.PushNotificationManager.g.cpp"

#include "PushNotificationCreateChannelResult.h"

#include <winrt/Windows.ApplicationModel.background.h>
#include <winrt/Windows.Networking.PushNotifications.h>
#include "PushNotificationBackgroundTask.h"

#include <winerror.h>
#include <algorithm>
#include "PushNotificationChannel.h"
#include "externs.h"
#include <string_view>

using namespace std::literals;

constexpr std::wstring_view backgroundTaskName = L"PushBackgroundTaskName"sv;

namespace winrt
{
    using namespace Windows::ApplicationModel::Background;
    using namespace Windows::Networking::PushNotifications;
    using namespace Windows::Foundation;
}

wil::critical_section g_lock;

namespace winrt::Microsoft::Windows::PushNotifications::implementation
{
    inline constexpr std::uint32_t c_maxBackoffSeconds{ 960 };
    inline constexpr std::uint32_t c_minBackoffSeconds{ 30 };

    const HRESULT WNP_E_NOT_CONNECTED = (HRESULT)0x880403E8L;
    const HRESULT WNP_E_RECONNECTING = (HRESULT)0x880403E9L;
    const HRESULT WNP_E_BIND_USER_BUSY = (HRESULT)0x880403FEL;

    bool PushNotificationManager::IsChannelRequestRetryable(const hresult& hr)
    {
        switch (hr)
        {
        case HRESULT_FROM_WIN32(ERROR_TIMEOUT):
        case WNP_E_NOT_CONNECTED:
        case WPN_E_OUTSTANDING_CHANNEL_REQUEST:
        case WNP_E_RECONNECTING:
        case WNP_E_BIND_USER_BUSY:
        case HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE):
            return true;
        default:
            return false;
        }
    }

    inline bool IsPackagedProcess()
    {
        wchar_t packageFullName[PACKAGE_FULL_NAME_MAX_LENGTH + 1] = {};

        UINT32 packageFullNameLength = static_cast<UINT32>(ARRAYSIZE(packageFullName));

        const auto packagedProcessHResult = ::GetCurrentPackageFullName(&packageFullNameLength, packageFullName);

        if (packagedProcessHResult == APPMODEL_ERROR_NO_PACKAGE)
        {
            return false;
        }

        THROW_IF_WIN32_ERROR(packagedProcessHResult);

        return true;
    }

    winrt::IAsyncOperationWithProgress<winrt::Microsoft::Windows::PushNotifications::PushNotificationCreateChannelResult, winrt::Microsoft::Windows::PushNotifications::PushNotificationCreateChannelStatus> PushNotificationManager::CreateChannelAsync(const winrt::guid &remoteId)
    {
        static bool s_remoteIdInProgress;
        static wil::critical_section s_lock;

        THROW_HR_IF(E_INVALIDARG, (remoteId == winrt::guid()));

        // API supports channel requests only for packaged applications for v0.8 version
        THROW_HR_IF(E_NOTIMPL, !IsPackagedProcess());

        {
            auto lock = s_lock.lock();
            if (s_remoteIdInProgress == false)
            {
                s_remoteIdInProgress = true;
            }
            else
            {
                co_return winrt::make<PushNotificationCreateChannelResult>(
                    nullptr, WPN_E_OUTSTANDING_CHANNEL_REQUEST, PushNotificationChannelStatus::CompletedFailure);
            }
        }

        auto scopeExit = wil::scope_exit([&]()
            {
                auto lock = s_lock.lock();
                s_remoteIdInProgress = false;
            });

        auto cancellation{ co_await winrt::get_cancellation_token() };

        cancellation.enable_propagation(true);

        // Allow to register the progress and complete handler
        co_await resume_background();

        auto progress{ co_await winrt::get_progress_token() };

        uint8_t retryCount = 0;
        winrt::hresult channelRequestResult = E_PENDING;
        PushNotificationChannelStatus status = PushNotificationChannelStatus::InProgress;

        PushNotificationCreateChannelStatus
            channelStatus = { status, channelRequestResult, retryCount };

        progress(channelStatus);

        for (auto backOffTimeInSeconds = c_minBackoffSeconds; backOffTimeInSeconds <= c_maxBackoffSeconds * 2; backOffTimeInSeconds *= 2)
        {
            try
            {
                PushNotificationChannelManager channelManager{};
                winrt::PushNotificationChannel pushChannelReceived{ nullptr };

                pushChannelReceived = co_await channelManager.CreatePushNotificationChannelForApplicationAsync();

                // Returns a com_ptr to the implementation type
                auto pushChannel =
                    winrt::make_self<PushNotificationChannel>(pushChannelReceived);

                co_return winrt::make<PushNotificationCreateChannelResult>(
                    winrt::make<PushNotificationChannel>(pushChannelReceived),
                    S_OK,
                    PushNotificationChannelStatus::CompletedSuccess);

                break;
            }
            catch (...)
            {

                auto channelRequestException = hresult_error(to_hresult(), take_ownership_from_abi);

                if ((backOffTimeInSeconds <= c_maxBackoffSeconds) && IsChannelRequestRetryable(channelRequestException.code()))
                {
                    channelStatus.extendedError = channelRequestException.code();
                    channelStatus.status = PushNotificationChannelStatus::InProgressRetry;
                    channelStatus.retryCount = ++retryCount;

                    progress(channelStatus);
                }
                else
                {
                    co_return winrt::make<PushNotificationCreateChannelResult>(
                        nullptr,
                        channelRequestException.code(),
                        PushNotificationChannelStatus::CompletedFailure);
                }
            }

            co_await winrt::resume_after(std::chrono::seconds(backOffTimeInSeconds));
        }

        co_return nullptr;
    }

    PushNotificationRegistrationToken PushNotificationManager::RegisterActivator(PushNotificationActivationInfo const& details)
    {
        THROW_HR_IF(E_INVALIDARG, details.TaskClsid() == winrt::guid());
        THROW_HR_IF(E_INVALIDARG, !(WI_IsFlagSet(details.Option(), PushNotificationRegistrationOption::PushTrigger) || WI_IsFlagSet(details.Option(), PushNotificationRegistrationOption::ComActivator)));

        GUID taskClsid = details.TaskClsid();
        DWORD cookie = 0;
        BackgroundTaskRegistration registeredTask = nullptr;
        BackgroundTaskBuilder builder = nullptr;

        if (WI_IsFlagSet(details.Option(), PushNotificationRegistrationOption::PushTrigger))
        {
            winrt::hstring taskClsidStr = winrt::to_hstring(taskClsid);
            auto tasks = BackgroundTaskRegistration::AllTasks();
            bool taskRegistered = std::any_of(begin(tasks), end(tasks),
            [&](auto&& task)
            {
                std::wstring backgroundTaskNameStr = task.Value().Name().c_str();
                if (backgroundTaskNameStr.find(backgroundTaskName) != std::wstring::npos)
                {
                    if (backgroundTaskNameStr.find(taskClsidStr) != std::wstring::npos)
                    {
                        return true;
                    }
                    else
                    {
                        // Unregister background task here
                        throw winrt::hresult_invalid_argument(L"RegisterActivator has different clsid registered.");
                    }
                };
                return false;
            });

            if (!taskRegistered)
            {
                builder = BackgroundTaskBuilder();
                builder.Name(backgroundTaskName + taskClsidStr.c_str());

                PushNotificationTrigger trigger{};
                builder.SetTrigger(trigger);

                auto builder5 = builder.try_as<winrt::IBackgroundTaskBuilder5>();
                if (IsPackagedProcess() && builder5)
                {
                    builder5.SetTaskEntryPointClsid(taskClsid);
                    winrt::com_array<winrt::IBackgroundCondition> conditions = details.GetConditions();
                    for (auto condition : conditions)
                    {
                        builder.AddCondition(condition);
                    }
                }
                else
                {
                    throw winrt::hresult_not_implemented();
                }
            }
        }

        if (WI_IsFlagSet(details.Option(), PushNotificationRegistrationOption::ComActivator))
        {
            {
                auto lock = g_lock.lock();
                // Define handle that will be set during background task execution
                g_waitHandleForArgs = wil::unique_handle(CreateEvent(nullptr, FALSE, FALSE, nullptr));
                THROW_HR_IF_NULL(E_UNEXPECTED, g_waitHandleForArgs);
            }

            THROW_IF_FAILED(::CoRegisterClassObject(
                taskClsid,
                winrt::make<PushNotificationBackgroundTaskFactory>().get(),
                CLSCTX_LOCAL_SERVER,
                REGCLS_MULTIPLEUSE,
                &cookie));
        }

        if (builder)
        {
            registeredTask = builder.Register();
        }

        return PushNotificationRegistrationToken{ cookie, registeredTask };
    }

    void PushNotificationManager::UnregisterActivator(PushNotificationRegistrationToken const& token, PushNotificationRegistrationOption const& option)
    {

        if (WI_IsFlagSet(option, PushNotificationRegistrationOption::PushTrigger))
        {
            for (auto task : BackgroundTaskRegistration::AllTasks())
            {
                if (task.Value().Name() == token.TaskRegistration().Name())
                {
                    task.Value().Unregister(true);
                }
            }
        }

        if (WI_IsFlagSet(option, PushNotificationRegistrationOption::ComActivator) && token.Cookie())
        {
            LOG_IF_FAILED(::CoRevokeClassObject(static_cast<DWORD>(token.Cookie())));
        }
    }
}
