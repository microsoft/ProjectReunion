// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#include "Microsoft.WindowsApp.PushNotifications.PushNotificationChannel.g.h"

namespace winrt::Microsoft::WindowsApp::PushNotifications::implementation
{
    struct PushNotificationChannel : PushNotificationChannelT<PushNotificationChannel>
    {
        PushNotificationChannel(winrt::Windows::Networking::PushNotifications::PushNotificationChannel const& channel);
        winrt::Windows::Foundation::Uri Uri();
        winrt::Windows::Foundation::DateTime ExpirationTime();
        void Close();

        winrt::event_token PushReceived(winrt::Windows::Foundation::TypedEventHandler<Microsoft::WindowsApp::PushNotifications::PushNotificationChannel, Microsoft::WindowsApp::PushNotifications::PushNotificationReceivedEventArgs> handler);
        void PushReceived(winrt::event_token const& token) noexcept;

    private:
        const winrt::Windows::Networking::PushNotifications::PushNotificationChannel m_channel{ nullptr };

    };
}
namespace winrt::Microsoft::WindowsApp::PushNotifications::factory_implementation
{
    struct PushNotificationChannel : PushNotificationChannelT<PushNotificationChannel, implementation::PushNotificationChannel>
    {
    };
}
