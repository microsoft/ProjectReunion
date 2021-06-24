﻿// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "pch.h"
#include "EnvironmentManagerUWPTests.h"
#include "EnvironmentVariableHelper.h"
#include "TestSetupAndTeardownHelper.h"

using namespace winrt::Microsoft::ProjectReunion;

namespace ProjectReunionEnvironmentManagerTests
{
    void EnvironmentManagerUWPTests::UWPTestGetForProcess()
    {
        EnvironmentManager environmentManager{ EnvironmentManager::GetForProcess() };
        VERIFY_IS_NOT_NULL(environmentManager);
    }

    void EnvironmentManagerUWPTests::UWPTestGetForUser()
    {
        EnvironmentManager environmentManager{ EnvironmentManager::GetForUser() };
        VERIFY_IS_NOT_NULL(environmentManager);
    }

    void EnvironmentManagerUWPTests::UWPTestGetForMachine()
    {
        EnvironmentManager environmentManager{ EnvironmentManager::GetForMachine() };
        VERIFY_IS_NOT_NULL(environmentManager);
    }


    void EnvironmentManagerUWPTests::UWPTestGetEnvironmentVariablesForProcess()
    {
        EnvironmentManager environmentManager{ EnvironmentManager::GetForProcess() };
        EnvironmentVariables environmentVariablesFromWinRTAPI{ environmentManager.GetEnvironmentVariables() };

        EnvironmentVariables environmentVariablesFromWindowsAPI{ GetEnvironmentVariablesForProcess() };

        CompareIMapViews(environmentVariablesFromWinRTAPI, environmentVariablesFromWindowsAPI);
    }

    void EnvironmentManagerUWPTests::UWPTestGetEnvironmentVariablesForUser()
    {
        EnvironmentManager environmentManager{ EnvironmentManager::GetForUser() };
        EnvironmentVariables environmentVariablesFromWinRTAPI{ environmentManager.GetEnvironmentVariables() };

        EnvironmentVariables environmentVariablesFromWindowsAPI{ GetEnvironmentVariablesForUser() };

        CompareIMapViews(environmentVariablesFromWinRTAPI, environmentVariablesFromWindowsAPI);
    }

    void EnvironmentManagerUWPTests::UWPTestGetEnvironmentVariablesForMachine()
    {
        EnvironmentVariables environmentVariablesFromWindowsAPI{ GetEnvironmentVariablesForMachine() };

        EnvironmentManager environmentManager{ EnvironmentManager::GetForMachine() };
        EnvironmentVariables environmentVariablesFromWinRTAPI{ environmentManager.GetEnvironmentVariables() };

        CompareIMapViews(environmentVariablesFromWinRTAPI, environmentVariablesFromWindowsAPI);
    }

    void EnvironmentManagerUWPTests::UWPTestGetEnvironmentVariableForProcess()
    {
        ProcessSetup();
        EnvironmentManager environmentManager{ EnvironmentManager::GetForProcess() };
        winrt::hstring environmentValue{ environmentManager.GetEnvironmentVariable(c_evKeyName) };

        ProcessCleanup();

        VERIFY_ARE_EQUAL(std::wstring{ c_evValueName }, environmentValue);
    }

    void EnvironmentManagerUWPTests::UWPTestGetEnvironmentVariableForUser()
    {
        EnvironmentManager environmentManager{ EnvironmentManager::GetForUser() };
        winrt::hstring environmentValue{ environmentManager.GetEnvironmentVariable(c_evKeyNameForGet) };

        VERIFY_ARE_EQUAL(std::wstring{ c_evValueName }, environmentValue);
    }

    void EnvironmentManagerUWPTests::UWPTestGetEnvironmentVariableForMachine()
    {
        EnvironmentManager environmentManager{ EnvironmentManager::GetForUser() };
        winrt::hstring environmentValue{ environmentManager.GetEnvironmentVariable(c_evKeyNameForGet) };

        VERIFY_ARE_EQUAL(std::wstring{ c_evValueName }, environmentValue);
    }


    void EnvironmentManagerUWPTests::UWPTestSetEnvironmentVariableForProcess()
    {
        EnvironmentManager environmentManager{ EnvironmentManager::GetForProcess() };
        VERIFY_NO_THROW(environmentManager.SetEnvironmentVariable(c_evKeyName, c_evValueName));

        std::wstring writtenEV = GetEnvironmentVariableForProcess(c_evKeyName);
        VERIFY_ARE_EQUAL(std::wstring{ c_evValueName }, writtenEV);

        // Update the environment variable
        VERIFY_NO_THROW(environmentManager.SetEnvironmentVariable(c_evKeyName, c_evValueName2));
        writtenEV = GetEnvironmentVariableForProcess(c_evKeyName);
        VERIFY_ARE_EQUAL(std::wstring{ c_evValueName2 }, writtenEV);


        // Remove the value
        // setting the value to empty is the same as deleting the variable
        VERIFY_NO_THROW(environmentManager.SetEnvironmentVariable(c_evKeyName, L""));
        VERIFY_ARE_EQUAL(0, ::GetEnvironmentVariable(c_evKeyName, nullptr, 0));
        VERIFY_ARE_EQUAL(ERROR_ENVVAR_NOT_FOUND, GetLastError());
    }

    void EnvironmentManagerUWPTests::UWPTestSetEnvironmentVariableForUser()
    {
        EnvironmentManager environmentMananger{ EnvironmentManager::GetForUser() };
        VERIFY_THROWS(environmentMananger.SetEnvironmentVariable(c_evKeyName, c_evValueName), winrt::hresult_access_denied);
    }

    void EnvironmentManagerUWPTests::UWPTestSetEnvironmentVariableForMachine()
    {
        EnvironmentManager environmentMananger{ EnvironmentManager::GetForMachine() };
        VERIFY_THROWS(environmentMananger.SetEnvironmentVariable(c_evKeyName, c_evValueName), winrt::hresult_access_denied);
    }

    void EnvironmentManagerUWPTests::UWPTestAppendToPathForProcess()
    {
        ProcessSetup();

        // Keep a local string to match all operations to PATH
        std::wstring pathToManipulate{ GetEnvironmentVariableForProcess(c_pathName) };

        EnvironmentManager environmentManager{ EnvironmentManager::GetForProcess() };

        VERIFY_NO_THROW(environmentManager.AppendToPath(c_evValueName));

        // Current path should have the semi-colon
        std::wstring currentPath{ GetEnvironmentVariableForProcess(c_pathName) };
        if (pathToManipulate.back() != L';')
        {
            pathToManipulate += L";";
        }

        pathToManipulate += c_evValueName;
        pathToManipulate += L";";

        ProcessCleanup();

        VERIFY_ARE_EQUAL(currentPath, pathToManipulate);

        VERIFY_THROWS(environmentManager.AppendToPath(L""), winrt::hresult_invalid_argument);
    }

    void EnvironmentManagerUWPTests::UWPTestAppendToPathForUser()
    {
        EnvironmentManager environmentManager{ EnvironmentManager::GetForUser() };
        VERIFY_THROWS(environmentManager.AppendToPath(c_evValueName), winrt::hresult_access_denied);
    }

    void EnvironmentManagerUWPTests::UWPTestAppendToPathForMachine()
    {
        EnvironmentManager environmentManager{ EnvironmentManager::GetForMachine() };
        VERIFY_THROWS(environmentManager.AppendToPath(c_evValueName), winrt::hresult_access_denied);
    }

    void EnvironmentManagerUWPTests::UWPTestRemoveFromPathForProcess()
    {
        ProcessSetup();

        // Keep a local string to match all operations to PATH
        std::wstring pathToManipulate{ GetEnvironmentVariableForProcess(c_pathName) };

        EnvironmentManager environmentManager{ EnvironmentManager::GetForProcess() };

        environmentManager.RemoveFromPath(c_evValueName);

        std::wstring currentPath{ GetEnvironmentVariableForProcess(c_pathName) };

        ProcessCleanup();

        VERIFY_ARE_EQUAL(currentPath, pathToManipulate);

        VERIFY_NO_THROW(environmentManager.RemoveFromPath(L"I do not exist"));

        VERIFY_ARE_EQUAL(currentPath, pathToManipulate);

    }

    void EnvironmentManagerUWPTests::UWPTestRemoveFromPathForUser()
    {
        EnvironmentManager environmentManager{ EnvironmentManager::GetForUser() };
        VERIFY_THROWS(environmentManager.AppendToPath(c_evValueName), winrt::hresult_access_denied);
    }

    void EnvironmentManagerUWPTests::UWPTestRemoveFromPathForMachine()
    {
        EnvironmentManager environmentManager{ EnvironmentManager::GetForMachine() };
        VERIFY_THROWS(environmentManager.AppendToPath(c_evValueName), winrt::hresult_access_denied);
    }

    void EnvironmentManagerUWPTests::UWPTestAppendToPathExtForProcess()
    {
        ProcessSetup();
        // Keep a local string to match all operations to PATH
        std::wstring pathToManipulate{ GetEnvironmentVariableForProcess(c_pathExtName) };

        EnvironmentManager environmentManager{ EnvironmentManager::GetForProcess() };

        VERIFY_NO_THROW(environmentManager.AddExecutableFileExtension(c_evValueName));

        // Current path should have the semi-colon
        std::wstring currentPath{ GetEnvironmentVariableForProcess(c_pathExtName) };
        if (pathToManipulate.back() != L';')
        {
            pathToManipulate += L";";
        }

        pathToManipulate += c_evValueName;
        pathToManipulate += L";";

        ProcessCleanup();
        VERIFY_ARE_EQUAL(currentPath, pathToManipulate);

        VERIFY_THROWS(environmentManager.AddExecutableFileExtension(L""), winrt::hresult_invalid_argument);
    }

    void EnvironmentManagerUWPTests::UWPTestAppendToPathExtForUser()
    {
        EnvironmentManager environmentManager{ EnvironmentManager::GetForUser() };

        VERIFY_THROWS(environmentManager.AddExecutableFileExtension(c_evValueName), winrt::hresult_access_denied);
    }

    void EnvironmentManagerUWPTests::UWPTestAppendToPathExtForMachine()
    {
        EnvironmentManager environmentManager{ EnvironmentManager::GetForMachine() };

        VERIFY_THROWS(environmentManager.AddExecutableFileExtension(c_evValueName), winrt::hresult_access_denied);
    }

    void EnvironmentManagerUWPTests::UWPTestRemoveFromPathExtForProcess()
    {
        ProcessSetup();

        // Keep a local string to match all operations to PATH
        std::wstring pathToManipulate{ GetEnvironmentVariableForProcess(c_pathExtName) };

        EnvironmentManager environmentManager{ EnvironmentManager::GetForProcess() };

        InjectIntoPath(true, false, c_evValueName, 5);
        environmentManager.RemoveExecutableFileExtension(c_evValueName);

        std::wstring currentPath{ GetEnvironmentVariableForProcess(c_pathExtName) };

        ProcessCleanup();

        VERIFY_ARE_EQUAL(currentPath, pathToManipulate);

        VERIFY_NO_THROW(environmentManager.RemoveExecutableFileExtension(L"I do not exist"));

        VERIFY_ARE_EQUAL(currentPath, pathToManipulate);
    }

    void EnvironmentManagerUWPTests::UWPTestRemoveFromPathExtForUser()
    {
        EnvironmentManager environmentManager{ EnvironmentManager::GetForUser() };
        std::wstring pathPart{ GetSecondValueFromPathExt(false, true) };
        VERIFY_THROWS(environmentManager.RemoveExecutableFileExtension(pathPart), winrt::hresult_access_denied);
    }

    void EnvironmentManagerUWPTests::UWPTestRemoveFromPathExtForMachine()
    {
        EnvironmentManager environmentManager{ EnvironmentManager::GetForMachine() };
        std::wstring pathPart{ GetSecondValueFromPathExt(false, false) };
        VERIFY_THROWS(environmentManager.RemoveExecutableFileExtension(pathPart), winrt::hresult_access_denied);
    }
}
