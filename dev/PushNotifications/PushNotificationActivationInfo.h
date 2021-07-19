// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#include "Microsoft.WindowsApp.PushNotifications.PushNotificationActivationInfo.g.h"

namespace winrt::Microsoft::WindowsApp::PushNotifications::implementation
{
    struct PushNotificationActivationInfo : PushNotificationActivationInfoT<PushNotificationActivationInfo>
    {
        PushNotificationActivationInfo(Microsoft::WindowsApp::PushNotifications::PushNotificationRegistrationOptions const& option, winrt::guid const& taskClsid);
        winrt::guid TaskClsid();
        Microsoft::WindowsApp::PushNotifications::PushNotificationRegistrationOptions Options();
        winrt::com_array<winrt::Windows::ApplicationModel::Background::IBackgroundCondition> GetConditions();
        void SetConditions(array_view<winrt::Windows::ApplicationModel::Background::IBackgroundCondition const> conditions);

    private:
        const Microsoft::WindowsApp::PushNotifications::PushNotificationRegistrationOptions m_options;
        const winrt::guid m_taskClsid;
        winrt::com_array<winrt::Windows::ApplicationModel::Background::IBackgroundCondition> m_backgroundConditions{};
        wil::srwlock m_lock;
    };
}
namespace winrt::Microsoft::WindowsApp::PushNotifications::factory_implementation
{
    struct PushNotificationActivationInfo : PushNotificationActivationInfoT<PushNotificationActivationInfo, implementation::PushNotificationActivationInfo>
    {
    };
}
