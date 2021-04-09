﻿// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "pch.h"
#include "EnvironmentReaderUWPTests.h"
#include "EnvironmentVariableHelper.h"
#include "TestSetupAndTeardownHelper.h"

using namespace winrt::Microsoft::ProjectReunion;

namespace ProjectReunionEnvironmentReaderTests
{
    void EnvironmentReaderUWPTests::UWPTestGetForProcess()
    {
        EnvironmentReader environmentReader{ EnvironmentReader::GetForProcess() };
        VERIFY_IS_NOT_NULL(environmentReader);
    }

    void EnvironmentReaderUWPTests::UWPTestGetForUser()
    {
        EnvironmentReader environmentReader{ EnvironmentReader::GetForUser() };
        VERIFY_IS_NOT_NULL(environmentReader);
    }

    void EnvironmentReaderUWPTests::UWPTestGetForMachine()
    {
        EnvironmentReader environmentReader{ EnvironmentReader::GetForMachine() };
        VERIFY_IS_NOT_NULL(environmentReader);
    }

    void EnvironmentReaderUWPTests::UWPTestGetEnvironmentVariablesForProcess()
    {
        EnvironmentReader environmentReader = EnvironmentReader::GetForProcess();
        EnvironmentVariables environmentVariablesFromWinRTAPI = environmentReader.GetEnvironmentVariables();

        EnvironmentVariables environmentVariablesFromWindowsAPI = GetEnvironmentVariablesForProcess();

        CompareIMapViews(environmentVariablesFromWinRTAPI, environmentVariablesFromWindowsAPI);
    }

    void EnvironmentReaderUWPTests::UWPTestGetEnvironmentVariablesForUser()
    {

        EnvironmentReader environmentReader = EnvironmentReader::GetForUser();
        EnvironmentVariables environmentVariablesFromWinRTAPI = environmentReader.GetEnvironmentVariables();

        EnvironmentVariables environmentVariablesFromWindowsAPI = GetEnvironmentVariablesForUser();

        CompareIMapViews(environmentVariablesFromWinRTAPI, environmentVariablesFromWindowsAPI);
    }

    void EnvironmentReaderUWPTests::UWPTestGetEnvironmentVariablesForMachine()
    {
        EnvironmentVariables environmentVariablesFromWindowsAPI = GetEnvironmentVariablesForMachine();

        EnvironmentReader environmentReader = EnvironmentReader::GetForMachine();
        EnvironmentVariables environmentVariablesFromWinRTAPI = environmentReader.GetEnvironmentVariables();

        CompareIMapViews(environmentVariablesFromWinRTAPI, environmentVariablesFromWindowsAPI);
    }

    void EnvironmentReaderUWPTests::UWPTestGetEnvironmentVariableForProcess()
    {
        WriteProcessEV();
        EnvironmentReader environmentReader = EnvironmentReader::GetForProcess();
        winrt::hstring environmentValue = environmentReader.GetEnvironmentVariable(c_evKeyName);

        VERIFY_ARE_EQUAL(std::wstring(c_evValueName), environmentValue);
    }

    void EnvironmentReaderUWPTests::UWPTestGetEnvironmentVariableForUser()
    {
        EnvironmentReader environmentReader = EnvironmentReader::GetForUser();
        winrt::hstring environmentValue = environmentReader.GetEnvironmentVariable(c_evKeyName);

        VERIFY_ARE_EQUAL(std::wstring(c_evValueName), environmentValue);
    }

    void EnvironmentReaderUWPTests::UWPTestGetEnvironmentVariableForMachine()
    {
        EnvironmentReader environmentReader = EnvironmentReader::GetForUser();
        winrt::hstring environmentValue = environmentReader.GetEnvironmentVariable(c_evKeyName);

        VERIFY_ARE_EQUAL(std::wstring(c_evValueName), environmentValue);
    }

    void EnvironmentReaderUWPTests::UWPTestSetEnvironmentVariableForProcess()
    {
        EnvironmentReader environmentReader = EnvironmentReader::GetForProcess();
        VERIFY_NO_THROW(environmentReader.SetEnvironmentVariable(c_evKeyName2, c_evValueName));

        std::wstring writtenEV = GetEnvironmentVariableForProcess(c_evKeyName2);
        VERIFY_ARE_EQUAL(std::wstring(c_evValueName), writtenEV);

        // Update the environment variable
        VERIFY_NO_THROW(environmentReader.SetEnvironmentVariable(c_evKeyName2, c_evValueName2));
        writtenEV = GetEnvironmentVariableForProcess(c_evKeyName2);
        VERIFY_ARE_EQUAL(std::wstring(c_evValueName2), writtenEV);


        // Remove the value
        // setting the value to empty is the same as deleting the variable
        VERIFY_NO_THROW(environmentReader.SetEnvironmentVariable(c_evKeyName2, L""));
        VERIFY_ARE_EQUAL(0, ::GetEnvironmentVariable(c_evKeyName2, nullptr, 0));
        VERIFY_ARE_EQUAL(ERROR_ENVVAR_NOT_FOUND, GetLastError());
    }

    void EnvironmentReaderUWPTests::UWPTestSetEnvironmentVariableForUser()
    {
        EnvironmentReader environmentMananger = EnvironmentReader::GetForUser();
        VERIFY_THROWS(environmentMananger.SetEnvironmentVariable(c_evKeyName, c_evValueName), winrt::hresult_access_denied);
    }

    void EnvironmentReaderUWPTests::UWPTestSetEnvironmentVariableForMachine()
    {
        EnvironmentReader environmentMananger = EnvironmentReader::GetForMachine();
        VERIFY_THROWS(environmentMananger.SetEnvironmentVariable(c_evKeyName, c_evValueName), winrt::hresult_access_denied);
    }

    void EnvironmentReaderUWPTests::UWPTestAppendToPathForProcess()
    {
        std::wstring originalPath = GetEnvironmentVariableForProcess(c_pathName);
        EnvironmentReader environmentReader = EnvironmentReader::GetForProcess();
        VERIFY_NO_THROW(environmentReader.AppendToPath(c_evValueName));

        std::wstring alteredPath = GetEnvironmentVariableForProcess(c_pathName);

        if (originalPath.back() != L';')
        {
            originalPath += L';';
        }

        VERIFY_ARE_EQUAL(originalPath.append(c_evValueName).append(L";"), alteredPath);
    }

    void EnvironmentReaderUWPTests::UWPTestAppendToPathForUser()
    {
        std::wstring originalPath = GetEnvironmentVariableForUser(c_pathName);
        EnvironmentReader environmentReader = EnvironmentReader::GetForUser();
        VERIFY_THROWS(environmentReader.AppendToPath(c_evValueName), winrt::hresult_access_denied);
    }

    void EnvironmentReaderUWPTests::UWPTestAppendToPathForMachine()
    {
        std::wstring originalPath = GetEnvironmentVariableForMachine(c_pathName);
        EnvironmentReader environmentReader = EnvironmentReader::GetForMachine();
        VERIFY_THROWS(environmentReader.AppendToPath(c_evValueName), winrt::hresult_access_denied);
    }
}
