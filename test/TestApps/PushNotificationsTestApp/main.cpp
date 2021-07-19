﻿#include "pch.h"
#include <testdef.h>
#include <iostream>
#include <sstream>
#include <wil/win32_helpers.h>
#include <winrt/Windows.ApplicationModel.Background.h> // we need this for BackgroundTask APIs

using namespace winrt;
using namespace winrt::Microsoft::Windows::AppLifecycle;
using namespace winrt::Microsoft::WindowsApp::PushNotifications;
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
    if (channelOperation.wait_for(timeout) != AsyncStatus::Completed)
    {
        channelOperation.Cancel();
        return HRESULT_FROM_WIN32(ERROR_TIMEOUT); // timed out or failed
    }

    auto result = channelOperation.GetResults();
    auto status = result.Status();
    if (status != PushNotificationChannelStatus::CompletedSuccess)
    {
        return result.ExtendedError(); // did not produce a channel
    }

    result.Channel().Close();
    return S_OK;
}

bool ChannelRequestUsingRemoteId()
{
    auto channelOperation = PushNotificationManager::CreateChannelAsync(remoteId1);
    auto channelOperationResult = ChannelRequestHelper(channelOperation);

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
    auto const& switchMapping = GetSwitchMapping();
    auto it = switchMapping.find(unitTest);
    if (it == switchMapping.end())
    {
        return false;
    }

    return it->second();
}

int main() try
{
    bool testResult = false;
    auto scope_exit = wil::scope_exit([&] {
        if (g_appToken)
        {
            PushNotificationManager::UnregisterActivator(g_appToken, PushNotificationRegistrationOptions::PushTrigger | PushNotificationRegistrationOptions::ComActivator);
        }
    });

    PushNotificationActivationInfo info(
        PushNotificationRegistrationOptions::PushTrigger | PushNotificationRegistrationOptions::ComActivator,
        winrt::guid(c_comServerId)); // same clsid as app manifest

    g_appToken = PushNotificationManager::RegisterActivator(info);
    
    auto args = AppInstance::GetCurrent().GetActivatedEventArgs();
    auto kind = args.Kind();

    if (kind == ExtendedActivationKind::Launch)
    {
        auto launchArgs = args.Data().as<ILaunchActivatedEventArgs>();
        std::string unitTest = to_string(launchArgs.Arguments());
        std::cout << unitTest << std::endl;

        testResult = runUnitTest(unitTest);
    }
    else if (kind == ExtendedActivationKind::Push)
    {
        PushNotificationReceivedEventArgs pushArgs = args.Data().as<PushNotificationReceivedEventArgs>();
        auto payload = pushArgs.Payload();
        std::wstring payloadString(payload.begin(), payload.end());

        testResult = payloadString == c_rawNotificationPayload;
    }

    return testResult ? 0 : 1; // We want 0 to be success and 1 failure
}
catch (...)
{
    std::cout << winrt::to_string(winrt::to_message()) << std::endl;
    return 1; // in the event of unhandled test crash
}
