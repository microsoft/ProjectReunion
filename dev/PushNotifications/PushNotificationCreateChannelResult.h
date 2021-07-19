// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#include "Microsoft.WindowsApp.PushNotifications.PushNotificationCreateChannelResult.g.h"

namespace winrt::Microsoft::WindowsApp::PushNotifications::implementation
{
    struct PushNotificationCreateChannelResult : PushNotificationCreateChannelResultT<PushNotificationCreateChannelResult>
    {
        PushNotificationCreateChannelResult(Microsoft::WindowsApp::PushNotifications::PushNotificationChannel const& channel, hresult const& extendedError, Microsoft::WindowsApp::PushNotifications::PushNotificationChannelStatus const& status);
        Microsoft::WindowsApp::PushNotifications::PushNotificationChannel Channel();
        winrt::hresult ExtendedError();
        Microsoft::WindowsApp::PushNotifications::PushNotificationChannelStatus Status();

    private:
        const Microsoft::WindowsApp::PushNotifications::PushNotificationChannel m_channel{ nullptr };
        const winrt::hresult m_extendedError;
        const Microsoft::WindowsApp::PushNotifications::PushNotificationChannelStatus m_status;
    };
}
namespace winrt::Microsoft::WindowsApp::PushNotifications::factory_implementation
{
    struct PushNotificationCreateChannelResult : PushNotificationCreateChannelResultT<PushNotificationCreateChannelResult, implementation::PushNotificationCreateChannelResult>
    {
    };
}
