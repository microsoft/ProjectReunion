﻿#pragma once
#include "TestCommon.h"

inline void RestoreProcessPath(std::wstring originalPath)
{
    BOOL result = ::SetEnvironmentVariable(L"Path", originalPath.c_str());

    if (result == 0)
    {
        FAILED(HRESULT_FROM_WIN32(GetLastError()));
    }
}

inline void RestoreUserPath(std::wstring originalPath)
{
    wil::unique_hkey userEnvironmentVariablesHKey{};
    VERIFY_WIN32_SUCCEEDED(RegOpenKeyEx(HKEY_CURRENT_USER, c_userEvRegLocation, 0, KEY_WRITE, userEnvironmentVariablesHKey.addressof()));
    VERIFY_WIN32_SUCCEEDED(RegSetValueExW(userEnvironmentVariablesHKey.get(), c_pathName, 0, REG_EXPAND_SZ, (LPBYTE)originalPath.c_str(), static_cast<DWORD>(wcslen(originalPath.c_str()) * sizeof(wchar_t))));
}

inline void RestoreMachinePath(std::wstring originalPath)
{
    wil::unique_hkey machineEnvironmentVariablesHKey{};
    VERIFY_WIN32_SUCCEEDED(RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_machineEvRegLocation, 0, KEY_WRITE, machineEnvironmentVariablesHKey.addressof()));
    VERIFY_WIN32_SUCCEEDED(RegSetValueExW(machineEnvironmentVariablesHKey.get(), c_pathName, 0, REG_EXPAND_SZ, (LPBYTE)originalPath.c_str(), static_cast<DWORD>(wcslen(originalPath.c_str()) * sizeof(wchar_t))));
}

inline void WriteProcessEV()
{
    BOOL setResult = SetEnvironmentVariable(c_evKeyName, c_evValueName);

    if (!setResult)
    {
        FAILED(HRESULT_FROM_WIN32(GetLastError()));
    }
}

