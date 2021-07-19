// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#include "Microsoft.WindowsApp.PushNotifications.PushNotificationManager.g.h"

namespace winrt::Microsoft::WindowsApp::PushNotifications::implementation
{
    struct PushNotificationManager
    {
        PushNotificationManager() = delete;

        static Microsoft::WindowsApp::PushNotifications::PushNotificationRegistrationToken RegisterActivator(Microsoft::WindowsApp::PushNotifications::PushNotificationActivationInfo const& details);
        static void UnregisterActivator(Microsoft::WindowsApp::PushNotifications::PushNotificationRegistrationToken const& token, Microsoft::WindowsApp::PushNotifications::PushNotificationRegistrationOptions const& options);

        static winrt::Windows::Foundation::IAsyncOperationWithProgress<winrt::Microsoft::WindowsApp::PushNotifications::PushNotificationCreateChannelResult, winrt::Microsoft::WindowsApp::PushNotifications::PushNotificationCreateChannelStatus> CreateChannelAsync(const winrt::guid &remoteId);

    private:
        static bool IsChannelRequestRetryable(const winrt::hresult& hrException);
    };
}
namespace winrt::Microsoft::WindowsApp::PushNotifications::factory_implementation
{
    struct PushNotificationManager : PushNotificationManagerT<PushNotificationManager, implementation::PushNotificationManager>
    {
    };
}
