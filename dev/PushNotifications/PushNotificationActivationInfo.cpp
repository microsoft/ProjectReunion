// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "pch.h"
#include "PushNotificationActivationInfo.h"
#include "Microsoft.WindowsApp.PushNotifications.PushNotificationActivationInfo.g.cpp"

#include <Windows.ApplicationModel.background.h>

namespace winrt
{
    using namespace Microsoft::WindowsApp::PushNotifications;
    using namespace Windows::ApplicationModel::Background;
}

namespace winrt::Microsoft::WindowsApp::PushNotifications::implementation
{
    PushNotificationActivationInfo::PushNotificationActivationInfo(winrt::PushNotificationRegistrationOptions const& options, winrt::guid const& taskClsid) : m_options(options), m_taskClsid(taskClsid) {}

    winrt::guid PushNotificationActivationInfo::TaskClsid()
    {
        return m_taskClsid;
    }

    winrt::PushNotificationRegistrationOptions PushNotificationActivationInfo::Options()
    {
        return m_options;
    }

    winrt::com_array<winrt::IBackgroundCondition> PushNotificationActivationInfo::GetConditions()
    {
        auto lock = m_lock.lock_shared();
        return { m_backgroundConditions.begin(), m_backgroundConditions.end() };
    }

    void PushNotificationActivationInfo::SetConditions(array_view<winrt::IBackgroundCondition const> conditions)
    {
        // Need to destroy old conditions outside of lock.
        auto replacement = winrt::com_array(conditions.begin(), conditions.end());
        auto lock = m_lock.lock_exclusive();
        swap(m_backgroundConditions, replacement);
    }
}
