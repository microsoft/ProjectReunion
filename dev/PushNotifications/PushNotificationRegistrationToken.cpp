// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "pch.h"
#include "PushNotificationRegistrationToken.h"
#include "Microsoft.WindowsApp.PushNotifications.PushNotificationRegistrationToken.g.cpp"

#include <Windows.ApplicationModel.background.h>

namespace winrt::Microsoft::WindowsApp::PushNotifications::implementation
{
    PushNotificationRegistrationToken::PushNotificationRegistrationToken(uint32_t const& cookie, winrt::Windows::ApplicationModel::Background::IBackgroundTaskRegistration const& taskRegistration): m_cookie(cookie), m_taskRegistration(taskRegistration) { }

    uint32_t PushNotificationRegistrationToken::Cookie()
    {
        return m_cookie;
    }

    winrt::Windows::ApplicationModel::Background::IBackgroundTaskRegistration PushNotificationRegistrationToken::TaskRegistration()
    {
        return m_taskRegistration;
    }
}
