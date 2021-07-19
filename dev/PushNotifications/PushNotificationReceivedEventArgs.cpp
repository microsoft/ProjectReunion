// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "pch.h"

#include <winrt/Windows.ApplicationModel.background.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Networking.PushNotifications.h>

#include "PushNotificationReceivedEventArgs.h"
#include "Microsoft.WindowsApp.PushNotifications.PushNotificationReceivedEventArgs.g.cpp"
#include <iostream>
#include <externs.h>
#include "ValueMarshaling.h"

namespace winrt
{
    using namespace Windows::ApplicationModel::Background;
    using namespace Windows::Storage::Streams;
    using namespace Windows::Storage;
    using namespace Windows::Networking::PushNotifications;
}

namespace winrt::Microsoft::WindowsApp::PushNotifications::implementation
{
    PushNotificationReceivedEventArgs::PushNotificationReceivedEventArgs(winrt::IBackgroundTaskInstance const& backgroundTask): m_backgroundTaskInstance(backgroundTask), m_rawNotification(backgroundTask.TriggerDetails().as<RawNotification>().ContentBytes()) {}

    PushNotificationReceivedEventArgs::PushNotificationReceivedEventArgs(winrt::PushNotificationReceivedEventArgs const& args): m_args(args), m_rawNotification(args.RawNotification().ContentBytes()) {}

    winrt::com_array<uint8_t> PushNotificationReceivedEventArgs::Payload()
    {
        auto rawNotificationData = m_rawNotification.data();
        return { rawNotificationData, rawNotificationData + (m_rawNotification.Length() * sizeof(uint8_t)) };
    }

    winrt::BackgroundTaskDeferral PushNotificationReceivedEventArgs::GetDeferral()
    {
        THROW_HR_IF_NULL_MSG(E_ILLEGAL_METHOD_CALL, m_backgroundTaskInstance, "Foreground activation cannot call this.");

        return m_backgroundTaskInstance.GetDeferral();
    }

    winrt::event_token PushNotificationReceivedEventArgs::Canceled(winrt::BackgroundTaskCanceledEventHandler const& handler)
    {
        THROW_HR_IF_NULL_MSG(E_ILLEGAL_METHOD_CALL, m_backgroundTaskInstance, "Foreground activation cannot call this.");

        return m_backgroundTaskInstance.Canceled(handler);
    }

    void PushNotificationReceivedEventArgs::Canceled(winrt::event_token const& token) noexcept
    {
        THROW_HR_IF_NULL_MSG(E_ILLEGAL_METHOD_CALL, m_backgroundTaskInstance, "Foreground activation cannot call this.");

        m_backgroundTaskInstance.Canceled(token);
    }

    bool PushNotificationReceivedEventArgs::Handled()
    {
        THROW_HR_IF_NULL_MSG(E_ILLEGAL_METHOD_CALL, m_args, "Background activation cannot call this.");

        return m_args.Cancel();
    }

    void PushNotificationReceivedEventArgs::Handled(bool value)
    {
        THROW_HR_IF_NULL_MSG(E_ILLEGAL_METHOD_CALL, m_args, "Background activation cannot call this.");

        m_args.Cancel(value);
    }
}

