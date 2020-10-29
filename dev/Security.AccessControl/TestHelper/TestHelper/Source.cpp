#pragma comment (lib, "WindowsApp.lib")

#include <Security.AccessControl.h>
#include <wil/result.h>
#include <wil/resource.h>

int wmain(int argc, wchar_t* argv[])
{
    if (argc < 2)
        return 1;

    const auto containerName = argv[1];

    AppContainerNameAndAccess ac[1] = { {containerName, EVENT_MODIFY_STATE | SYNCHRONIZE} };
    wil::unique_hlocal_security_descriptor sd;
    THROW_IF_FAILED(::GetSecurityDescriptorForAppContainerNames(1, ac, nullptr, EVENT_MODIFY_STATE | SYNCHRONIZE, &sd));

    wil::unique_event win32_event;
    SECURITY_ATTRIBUTES sa{ sizeof(sa), sd.get(), FALSE };
    win32_event.create(wil::EventOptions::None, L"AccessControlTest_Event", &sa);

    win32_event.wait();
}
