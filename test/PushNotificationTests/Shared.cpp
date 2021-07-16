// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.
#include "pch.h"
#include <TestDef.h>
#include "Shared.h"

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

//using namespace winrt::Microsoft::Windows::AppLifecycle;
using namespace winrt;
using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Activation;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Management::Deployment;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::System;

namespace Test::PushNotifications
{
    StorageFile CreateDocFile(std::wstring filename)
    {
        return KnownFolders::DocumentsLibrary().CreateFileAsync(filename,
            CreationCollisionOption::OpenIfExists).get();
    }

    StorageFile OpenDocFile(std::wstring filename)
    {
        return KnownFolders::DocumentsLibrary().GetFileAsync(filename).get();
    }

    wil::unique_handle Execute(const std::wstring& command, const std::wstring& args,
        const std::wstring& directory)
    {
        SHELLEXECUTEINFO ei{};
        ei.cbSize = sizeof(SHELLEXECUTEINFO);
        ei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_DOENVSUBST;
        ei.lpFile = command.c_str();
        ei.lpParameters = args.c_str();
        ei.lpDirectory = directory.c_str();
        ei.nShow = SW_NORMAL;

        if (!ShellExecuteEx(&ei))
        {
            auto lastError = GetLastError();
            VERIFY_WIN32_SUCCEEDED(lastError);
        }

        wil::unique_handle process{ ei.hProcess };
        return process;
    }

    void InstallPackage(const std::wstring& packagePath)
    {
        // Deploy packaged app to register handler through the manifest.
        PackageManager manager;
        IVector<Uri> depPackagePaths;
        auto result = manager.AddPackageAsync(Uri(packagePath), depPackagePaths,
            DeploymentOptions::ForceApplicationShutdown).get();
        auto errorText = result.ErrorText();
        auto errorCode = result.ExtendedErrorCode();
        VERIFY_SUCCEEDED(errorCode, errorText.c_str());
    }

    void UninstallPackage(const std::wstring& packageFullName)
    {
        // Deploy packaged app to register handler through the manifest.
        PackageManager manager;
        IVector<Uri> depPackagePaths;
        auto result = manager.RemovePackageAsync(packageFullName).get();
        auto errorText = result.ErrorText();
        auto errorCode = result.ExtendedErrorCode();
        VERIFY_SUCCEEDED(errorCode, errorText.c_str());
    }

    wil::unique_event CreateTestEvent(const std::wstring& eventName)
    {
        bool alreadyExists = false;
        SECURITY_ATTRIBUTES attributes = {};
        wil::unique_hlocal_security_descriptor descriptor;

        // Grant access to world and appcontainer.
        THROW_IF_WIN32_BOOL_FALSE(ConvertStringSecurityDescriptorToSecurityDescriptor(
            L"D:(A;;GA;;;WD)(A;;GA;;;AC)", SDDL_REVISION_1, &descriptor, nullptr));
        attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
        attributes.lpSecurityDescriptor = descriptor.get();

        wil::unique_event event;
        event.create(wil::EventOptions::None, eventName.c_str(), &attributes, &alreadyExists);
        return event;
    }

    void WaitForEvent(const wil::unique_event& successEvent, const wil::unique_event& failedEvent)
    {
        HANDLE waitEvents[2] = { successEvent.get(), failedEvent.get() };
        auto waitResult = WaitForMultipleObjects(_countof(waitEvents), waitEvents, FALSE,
            c_phaseTimeout);

        // If waitResult == failureEventIndex, it means the remote test process signaled a
        // failure event while we were waiting for a different event.
        auto failureEventIndex = WAIT_OBJECT_0 + 1;
        VERIFY_ARE_NOT_EQUAL(waitResult, failureEventIndex);

        VERIFY_ARE_NOT_EQUAL(waitResult, WAIT_TIMEOUT);
        if (waitResult == WAIT_FAILED)
        {
            auto lastError = GetLastError();
            VERIFY_WIN32_FAILED(lastError);
        }

        successEvent.ResetEvent();
        failedEvent.ResetEvent();
    }

    const std::wstring GetDeploymentDir()
    {
        WEX::Common::String testDeploymentDir;
        WEX::TestExecution::RuntimeParameters::TryGetValue(L"TestDeploymentDir", testDeploymentDir);
        return reinterpret_cast<PCWSTR>(testDeploymentDir.GetBuffer());
    }

    void WriteContentFile(std::wstring filename)
    {
        auto file = CreateDocFile(filename);
        FileIO::WriteTextAsync(file, L"Hello, World!").get();
    }

    void DeleteContentFile(std::wstring filename)
    {
        auto file = CreateDocFile(filename);
        file.DeleteAsync().get();
    }
}
