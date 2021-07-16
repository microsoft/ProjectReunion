#include "pch.h"
#include <testdef.h>
#include <iostream>
#include <sstream>
#include <wil/win32_helpers.h>
#include <winrt/Windows.ApplicationModel.Background.h> // we need this for BackgroundTask APIs

#include <MddBootstrap.h>

namespace AppModel::Identity
{
    inline bool IsPackagedProcess()
    {
        UINT32 n{};
        const auto rc{ ::GetCurrentPackageFullName(&n, nullptr) };
        THROW_HR_IF_MSG(HRESULT_FROM_WIN32(rc), (rc != APPMODEL_ERROR_NO_PACKAGE) && (rc != ERROR_INSUFFICIENT_BUFFER), "GetCurrentPackageFullName rc=%d", rc);
        return rc == ERROR_INSUFFICIENT_BUFFER;
    }
}

using namespace winrt;
using namespace winrt::Microsoft::Windows::AppLifecycle;
using namespace winrt::Microsoft::Windows::PushNotifications;
using namespace winrt::Windows::ApplicationModel::Activation;
using namespace winrt::Windows::ApplicationModel::Background; // BackgroundTask APIs
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;

winrt::guid remoteId1(L"a2e4a323-b518-4799-9e80-0b37aeb0d225"); // Generated from ms.portal.azure.com
winrt::guid remoteId2(L"CA1A4AB2-AC1D-4EFC-A132-E5A191CA285A"); // Dummy guid from visual studio guid tool generator

PushNotificationRegistrationToken g_appToken = nullptr;

constexpr auto timeout{ std::chrono::seconds(300) };

bool ChannelRequestUsingNullRemoteId()
{
    try
    {
        auto channelOperation = PushNotificationManager::CreateChannelAsync(winrt::guid()).get();
    }
    catch (...)
    {
        return to_hresult() == E_INVALIDARG;
    }
    return false;
}

HRESULT ChannelRequestHelper(IAsyncOperationWithProgress<PushNotificationCreateChannelResult, PushNotificationCreateChannelStatus> const& channelOperation)
{
    printf("elx - ChannelRequestHelper -1\n");
    if (channelOperation.wait_for(timeout) != AsyncStatus::Completed)
    {
        printf("elx - ChannelRequestHelper -2\n");
        channelOperation.Cancel();
        printf("elx - ChannelRequestHelper -3\n");
        return HRESULT_FROM_WIN32(ERROR_TIMEOUT); // timed out or failed
    }

    printf("elx - ChannelRequestHelper -4\n");
    auto result = channelOperation.GetResults();
    auto status = result.Status();
    printf("elx - ChannelRequestHelper -5\n");
    if (status != PushNotificationChannelStatus::CompletedSuccess)
    {
        printf("elx - ChannelRequestHelper -6\n");
        return result.ExtendedError(); // did not produce a channel
    }

    printf("elx - ChannelRequestHelper -7\n");
    if (AppModel::Identity::IsPackagedProcess())
    {
        result.Channel().Close();
    }
    printf("elx - ChannelRequestHelper -8\n");
    return S_OK;
}

bool ChannelRequestUsingRemoteId()
{
    printf("elx - ChannelRequestUsingRemoteId - 1\n");
    auto channelOperation = PushNotificationManager::CreateChannelAsync(remoteId1);
    printf("elx - ChannelRequestUsingRemoteId - 2\n");
    auto channelOperationResult = ChannelRequestHelper(channelOperation);
    printf("elx - ChannelRequestUsingRemoteId - 3\n");
    return channelOperationResult == S_OK;
}

// Verify calling channel close will fail when called twice.
bool MultipleChannelClose()
{
    auto channelOperation = PushNotificationManager::CreateChannelAsync(remoteId1);
    if (channelOperation.wait_for(timeout) != AsyncStatus::Completed)
    {
        channelOperation.Cancel();
        return false; // timed out or failed
    }

    auto result = channelOperation.GetResults();
    auto status = result.Status();
    if (status != PushNotificationChannelStatus::CompletedSuccess)
    {
        return false; // did not produce a channel
    }

    result.Channel().Close();
    try
    {
        result.Channel().Close();
    }
    catch (...)
    {
        return to_hresult() == WPN_E_CHANNEL_CLOSED;
    }
    return false;
}

bool MultipleChannelRequestUsingSameRemoteId()
{

    auto channelOperation1 = PushNotificationManager::CreateChannelAsync(remoteId1);
    auto channelOperation2 = PushNotificationManager::CreateChannelAsync(remoteId1);
    auto channelOperationResult2 = ChannelRequestHelper(channelOperation2);
    auto channelOperationResult1 = ChannelRequestHelper(channelOperation1);

    return channelOperationResult2 == S_OK;
}

bool MultipleChannelRequestUsingMultipleRemoteId()
{
    auto channelOperation1 = PushNotificationManager::CreateChannelAsync(remoteId1);
    auto channelOperation2 = PushNotificationManager::CreateChannelAsync(remoteId2);
    auto channelOperationResult2 = ChannelRequestHelper(channelOperation2);
    auto channelOperationResult1 = ChannelRequestHelper(channelOperation1);

    return channelOperationResult2 == S_OK;
}

bool ActivatorTest()
{
    PushNotificationManager::UnregisterActivator(std::exchange(g_appToken, nullptr), PushNotificationRegistrationOptions::PushTrigger | PushNotificationRegistrationOptions::ComActivator);

    try
    {
        PushNotificationActivationInfo info(
            PushNotificationRegistrationOptions::PushTrigger | PushNotificationRegistrationOptions::ComActivator,
            c_fakeComServerId);

        PushNotificationRegistrationToken fakeToken = nullptr;
        auto scope_exit = wil::scope_exit([&] {
            if (fakeToken)
            {
                PushNotificationManager::UnregisterActivator(fakeToken, PushNotificationRegistrationOptions::PushTrigger | PushNotificationRegistrationOptions::ComActivator);
            }
        });

        fakeToken = PushNotificationManager::RegisterActivator(info);
        if (!fakeToken.TaskRegistration())
        {
            return false;
        }

        PushNotificationManager::UnregisterActivator(std::exchange(fakeToken, nullptr), PushNotificationRegistrationOptions::PushTrigger | PushNotificationRegistrationOptions::ComActivator);
    }
    catch (...)
    {
        return false;
    }
    return true;
}

// Verify calling register activator with null PushNotificationActivationInfo is not allowed.
bool RegisterActivatorNullDetails()
{
    try
    {
        PushNotificationRegistrationToken fakeToken = nullptr;
        auto scope_exit = wil::scope_exit([&] {
            if (fakeToken)
            {
                PushNotificationManager::UnregisterActivator(fakeToken, PushNotificationRegistrationOptions::PushTrigger | PushNotificationRegistrationOptions::ComActivator);
            }
        });
        fakeToken = PushNotificationManager::RegisterActivator(nullptr);
    }
    catch (...)
    {
        return to_hresult() == E_INVALIDARG;
    }
    return false;
}

// Verify calling register activator with null clsid is not allowed.
bool RegisterActivatorNullClsid()
{
    winrt::hresult hr = S_OK;
    try
    {
        PushNotificationActivationInfo info(
            PushNotificationRegistrationOptions::PushTrigger | PushNotificationRegistrationOptions::ComActivator,
            winrt::guid()); // Null guid

        PushNotificationRegistrationToken fakeToken = nullptr;
        auto scope_exit = wil::scope_exit([&] {
            if (fakeToken)
            {
                PushNotificationManager::UnregisterActivator(fakeToken, PushNotificationRegistrationOptions::PushTrigger | PushNotificationRegistrationOptions::ComActivator);
            }
        });

        fakeToken = PushNotificationManager::RegisterActivator(info);
    }
    catch (...)
    {
        return to_hresult() == E_INVALIDARG;
    }
    return false;
}

// Verify unregistering activator with a null token is not allowed.
bool UnregisterActivatorNullToken()
{
    winrt::hresult hr = S_OK;
    try
    {
        PushNotificationManager::UnregisterActivator(nullptr, PushNotificationRegistrationOptions::PushTrigger | PushNotificationRegistrationOptions::ComActivator);
    }
    catch (...)
    {
        return to_hresult() == E_INVALIDARG;
    }
    return false;
}

// Verify unregistering an activator with null background registration is not allowed
// if PushTrigger option is specified.
bool UnregisterActivatorNullBackgroundRegistration()
{
    winrt::hresult hr = S_OK;
    try
    {
        PushNotificationRegistrationToken badToken{ 0, nullptr };
        PushNotificationManager::UnregisterActivator(badToken, PushNotificationRegistrationOptions::PushTrigger);
    }
    catch (...)
    {
        return to_hresult() == HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    }
    return false;
}

// Verify registering multiple activators is not allowed.
bool MultipleRegisterActivatorTest()
{
    winrt::hresult hr = S_OK;
    try
    {
        PushNotificationActivationInfo info(
            PushNotificationRegistrationOptions::PushTrigger | PushNotificationRegistrationOptions::ComActivator,
            c_fakeComServerId); // Fake clsid to test multiple activators

        PushNotificationRegistrationToken fakeToken = nullptr;
        auto scope_exit = wil::scope_exit([&] {
            if (fakeToken)
            {
                PushNotificationManager::UnregisterActivator(fakeToken, PushNotificationRegistrationOptions::PushTrigger | PushNotificationRegistrationOptions::ComActivator);
            }
        });

        fakeToken = PushNotificationManager::RegisterActivator(info);
    }
    catch (...)
    {
        return to_hresult() == E_INVALIDARG;
    }
    return false;
}

bool BackgroundActivationTest() // Activating application for background test.
{
    return true;
}

std::map<std::string, bool(*)()> const& GetSwitchMapping()
{
    static std::map<std::string, bool(*)()> switchMapping = {
        { "ChannelRequestUsingNullRemoteId",  &ChannelRequestUsingNullRemoteId },
        { "ChannelRequestUsingRemoteId", &ChannelRequestUsingRemoteId },
        { "MultipleChannelClose", &MultipleChannelClose},
        { "MultipleChannelRequestUsingSameRemoteId", &MultipleChannelRequestUsingSameRemoteId},
        { "MultipleChannelRequestUsingMultipleRemoteId", &MultipleChannelRequestUsingMultipleRemoteId},
        { "RegisterActivatorNullDetails", &RegisterActivatorNullDetails},
        { "RegisterActivatorNullClsid", &RegisterActivatorNullClsid},
        { "UnregisterActivatorNullToken", &UnregisterActivatorNullToken},
        { "UnregisterActivatorNullBackgroundRegistration", &UnregisterActivatorNullBackgroundRegistration},
        { "ActivatorTest", &ActivatorTest},
        { "MultipleRegisterActivatorTest", &MultipleRegisterActivatorTest},
        { "BackgroundActivationTest", &BackgroundActivationTest}
    };
    return switchMapping;
}

bool runUnitTest(std::string unitTest)
{
    printf("elx - runUnitTest - 1\n");
    auto const& switchMapping = GetSwitchMapping();
    printf("elx - runUnitTest - 2\n");
    auto it = switchMapping.find(unitTest);
    printf("elx - runUnitTest - 3\n");
    if (it == switchMapping.end())
    {
        printf("elx - runUnitTest - 4\n");
        return false;
    }

    printf("elx - runUnitTest - 5\n");
    return it->second();
}

int main() try
{
    printf("elx - 1\n");
    if (!AppModel::Identity::IsPackagedProcess())
    {
        printf("elx - 2\n");
        PACKAGE_VERSION version{};
        version.Major = 4;
        version.Minor = 1;
        version.Build = 1967;
        version.Revision = 333;

        const UINT32 majorMinorVersion = static_cast<UINT32>((version.Major << 16) | version.Minor);
        //const UINT32 majorMinorVersion{ 0x00040001 };
        //const UINT32 majorMinorVersion{ 0x00000008 };
        PCWSTR versionTag{};
        const PACKAGE_VERSION minVersion{};
        printf("elx - 3\n");
        const HRESULT hr{ MddBootstrapInitialize(majorMinorVersion, nullptr, minVersion) };
        printf("elx - 4\n");

        // Check the return code for errors. If there is an error, display the result.
        if (FAILED(hr))
        {
            printf("elx - 5\n");
            wprintf(L"Error 0x%X in MddBootstrapInitialize(0x%08X, %s, %hu.%hu.%hu.%hu)\n",
                hr, majorMinorVersion, versionTag, minVersion.Major, minVersion.Minor, minVersion.Build, minVersion.Revision);
            printf("elx - 6\n");
            return hr;
        }
    }

    printf("elx - 7\n");
    bool testResult = false;
    auto scope_exit = wil::scope_exit([&] {
        if (g_appToken)
        {
            printf("elx - 8\n");
            PushNotificationManager::UnregisterActivator(g_appToken, PushNotificationRegistrationOptions::PushTrigger | PushNotificationRegistrationOptions::ComActivator);
            printf("elx - 9\n");
        }
    });

    printf("elx - 10\n");
    PushNotificationActivationInfo info(
        PushNotificationRegistrationOptions::PushTrigger | PushNotificationRegistrationOptions::ComActivator,
        winrt::guid(c_comServerId)); // same clsid as app manifest

    printf("elx - 11\n");
    g_appToken = PushNotificationManager::RegisterActivator(info);
    
    printf("elx - 12\n");
    auto args = AppInstance::GetCurrent().GetActivatedEventArgs();
    auto kind = args.Kind();
   
    printf("elx - 13\n");
    if (kind == ExtendedActivationKind::Launch)
    {
        printf("elx - 14\n");
        auto launchArgs = args.Data().as<ILaunchActivatedEventArgs>();
        std::string unitTest = to_string(launchArgs.Arguments());
        auto argStart = unitTest.rfind(" ");
        if (argStart != std::wstring::npos)
        {
            unitTest = unitTest.substr(argStart + 1);
        }
        std::cout << unitTest << std::endl;

        printf("elx - 15 - unitTest: %s\n", unitTest.c_str());
        testResult = runUnitTest(unitTest);
        printf("elx - 16\n");
    }
    else if (kind == ExtendedActivationKind::Push)
    {
        printf("elx - 17\n");
        PushNotificationReceivedEventArgs pushArgs = args.Data().as<PushNotificationReceivedEventArgs>();
        auto payload = pushArgs.Payload();
        std::wstring payloadString(payload.begin(), payload.end());

        printf("elx - 18\n");
        testResult = payloadString == c_rawNotificationPayload;
        printf("elx - 19\n");
    }

    if (!AppModel::Identity::IsPackagedProcess())
    {
        printf("elx - 20\n");
        // Release the DDLM and clean up.
        MddBootstrapShutdown();
        printf("elx - 21\n");
    }

    printf("elx - 22\n");
    //char dontbehasty;
    //std::cin >> dontbehasty;
    return testResult ? 0 : 1; // We want 0 to be success and 1 failure
}
catch (...)
{
    std::cout << winrt::to_string(winrt::to_message()) << std::endl;
    //char dontbehasty;
    //std::cin >> dontbehasty;
    return 1; // in the event of unhandled test crash
}
