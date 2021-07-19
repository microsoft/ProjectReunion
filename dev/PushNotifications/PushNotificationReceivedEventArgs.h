#pragma once
#include "Microsoft.WindowsApp.PushNotifications.PushNotificationReceivedEventArgs.g.h"

namespace winrt::Microsoft::WindowsApp::PushNotifications::implementation
{
    struct PushNotificationReceivedEventArgs : PushNotificationReceivedEventArgsT<PushNotificationReceivedEventArgs>
    {
        PushNotificationReceivedEventArgs() = delete;

        PushNotificationReceivedEventArgs(winrt::Windows::ApplicationModel::Background::IBackgroundTaskInstance const& backgroundTask);
        PushNotificationReceivedEventArgs(winrt::Windows::Networking::PushNotifications::PushNotificationReceivedEventArgs const& args);

        com_array<uint8_t> Payload();
        winrt::Windows::ApplicationModel::Background::BackgroundTaskDeferral GetDeferral();
        winrt::event_token Canceled(winrt::Windows::ApplicationModel::Background::BackgroundTaskCanceledEventHandler const& handler);
        void Canceled(winrt::event_token const& token) noexcept;
        bool Handled();
        void Handled(bool value);

    private:
        const winrt::Windows::Storage::Streams::IBuffer m_rawNotification{};
        const winrt::Windows::ApplicationModel::Background::IBackgroundTaskInstance m_backgroundTaskInstance{};
        const winrt::Windows::Networking::PushNotifications::PushNotificationReceivedEventArgs m_args = nullptr;
    };
}
