// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "pch.h"

#include "PushNotificationBackgroundTask.h"

#include <winrt/base.h>
#include <winrt/Windows.ApplicationModel.Core.h>

#include "PushNotificationReceivedEventArgs.h"
#include "externs.h"

namespace winrt
{
    using namespace Windows::ApplicationModel::Background;
    using namespace Microsoft::WindowsApp::PushNotifications;
    using namespace Windows::ApplicationModel::Core;
}

void PushNotificationBackgroundTask::Run(winrt::IBackgroundTaskInstance const& taskInstance)
{
    auto appProperties = winrt::CoreApplication::Properties();
    winrt::PushNotificationReceivedEventArgs activatedEventArgs = winrt::make<winrt::Microsoft::WindowsApp::PushNotifications::implementation::PushNotificationReceivedEventArgs>(taskInstance);
    appProperties.Insert(ACTIVATED_EVENT_ARGS_KEY, activatedEventArgs);

    SetEvent(GetWaitHandleForArgs().get());
}
