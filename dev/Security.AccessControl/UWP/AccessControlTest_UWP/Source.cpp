#include "pch.h"

#include <Security.AccessControl.h>

//#include <Security.AccessControl.h>
using namespace winrt;

struct AccessControlTest
{
    BEGIN_TEST_CLASS(AccessControlTest)
        TEST_CLASS_PROPERTY(L"RunAs", L"UAP")
        TEST_CLASS_PROPERTY(L"UAP:AppXManifest", L"Package.appxmanifest")
    END_TEST_CLASS()

    TEST_METHOD(SimpleTest);
};

void AccessControlTest::SimpleTest()
{
    auto packageId = Windows::ApplicationModel::Package::Current().Id();
    auto familyName = packageId.FamilyName();
    WEX::Logging::Log::Comment(familyName.data());

    std::wstring commandLine = L"AccessControlTest_Helper.exe ";
    commandLine += familyName.c_str();

    wil::unique_handle child_process;
    wil::unique_handle child_thread;
    {
        STARTUPINFO si{};
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi{};

        THROW_IF_WIN32_BOOL_FALSE(::CreateProcessW(
            nullptr,
            commandLine.data(),
            nullptr,
            nullptr,
            FALSE,
            0,
            nullptr,
            nullptr,
            &si,
            &pi
        ));

        *(child_process.put()) = pi.hProcess;
        *(child_thread.put()) = pi.hThread;
    }
    wil::unique_event win32_event;
    for (int i = 0; i < 100; ++i)
    {
        if (win32_event.try_open(L"AccessControlTest_Event", SYNCHRONIZE | EVENT_MODIFY_STATE))
        {
            break;
        }
        WEX::Logging::Log::Comment(L"Waiting for event...");
        ::Sleep(1000);
    }
    if (!win32_event.is_valid())
    {
        THROW_WIN32(::GetLastError());
    }

    win32_event.SetEvent();
}
