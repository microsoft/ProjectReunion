// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#include "PushNotificationReceivedEventArgs.h"
#include <winrt/Windows.ApplicationModel.Core.h>
#include "externs.h"

constexpr PCWSTR c_pushContractId = L"Windows.Push";

namespace winrt::Microsoft::WindowsApp::PushNotifications
{
    static winrt::Windows::Foundation::IInspectable Deserialize(winrt::Windows::Foundation::Uri const&)
    {
        const DWORD receiveArgsTimeoutInMSec{ 2000 };
        THROW_HR_IF(HRESULT_FROM_WIN32(ERROR_TIMEOUT), !GetWaitHandleForArgs().wait(receiveArgsTimeoutInMSec));
        return winrt::Windows::ApplicationModel::Core::CoreApplication::Properties().TryLookup(ACTIVATED_EVENT_ARGS_KEY).as<PushNotificationReceivedEventArgs>();
    }
}
