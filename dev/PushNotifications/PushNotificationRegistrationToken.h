// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#include "Microsoft.WindowsApp.PushNotifications.PushNotificationRegistrationToken.g.h"

namespace winrt::Microsoft::WindowsApp::PushNotifications::implementation
{
    struct PushNotificationRegistrationToken : PushNotificationRegistrationTokenT<PushNotificationRegistrationToken>
    {
        PushNotificationRegistrationToken(uint32_t const& cookie, winrt::Windows::ApplicationModel::Background::IBackgroundTaskRegistration const& taskRegistration);
        uint32_t Cookie();
        winrt::Windows::ApplicationModel::Background::IBackgroundTaskRegistration TaskRegistration();

    private:
        const uint32_t m_cookie;
        const winrt::Windows::ApplicationModel::Background::IBackgroundTaskRegistration m_taskRegistration;
    };
}
namespace winrt::Microsoft::WindowsApp::PushNotifications::factory_implementation
{
    struct PushNotificationRegistrationToken : PushNotificationRegistrationTokenT<PushNotificationRegistrationToken, implementation::PushNotificationRegistrationToken>
    {
    };
}
