#include "pch.h"

#include <wil/resource.h>
#include <wil/result.h>
#include "Security.AccessControl.h"
#include <vector>
#include <sddl.h>
#include <AclAPI.h>
#include <UserEnv.h>

using namespace std;

namespace
{
    wil::unique_any_psid GetCurrentProcessSid()
    {
        wil::unique_any_psid result;
        wil::unique_process_heap_ptr<TOKEN_GROUPS> tg;
        DWORD length = 0;
        if (!::GetTokenInformation(
            ::GetCurrentProcessToken(),
            TokenLogonSid,
            nullptr,
            0,
            &length
        ))
        {
            auto err = ::GetLastError();
            if (err != ERROR_INSUFFICIENT_BUFFER)
            {
                THROW_WIN32(err);
            }
            tg.reset(static_cast<TOKEN_GROUPS*>(::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, length)));
        }

        THROW_IF_WIN32_BOOL_FALSE(::GetTokenInformation(
            ::GetCurrentProcessToken(),
            TokenLogonSid,
            tg.get(),
            length,
            &length));

        auto sidLength = ::GetLengthSid(tg->Groups[0].Sid);
        result.reset(::HeapAlloc(::GetProcessHeap(), 0, sidLength));
        THROW_IF_WIN32_BOOL_FALSE(::CopySid(
            sidLength, result.get(), tg->Groups[0].Sid));
        return result;
    }
}

STDAPI GetSecurityDescriptorForAppContainerNames(
    uint32_t accessRequestCount,
    _In_reads_(accessRequestCount)
        const AppContainerNameAndAccess* accessRequests,
    _In_opt_ PSID principal,
    uint32_t principalAccessMask,
    _Outptr_ PSECURITY_DESCRIPTOR* securityDescriptor
)
{
    *securityDescriptor = nullptr;
    try
    {
        vector<EXPLICIT_ACCESS> ea;
        if (accessRequestCount == UINT32_MAX)
        {
            THROW_HR(E_OUTOFMEMORY);
        }
        ea.reserve(static_cast<size_t>(accessRequestCount) + 1);

        wil::unique_any_psid currentProcessSid;
        if (!principal)
        {
            currentProcessSid = GetCurrentProcessSid();
            principal = currentProcessSid.get();
        }

        {
            EXPLICIT_ACCESS entry{};
            entry.grfAccessMode = GRANT_ACCESS;
            entry.grfAccessPermissions = principalAccessMask;
            entry.grfInheritance = NO_INHERITANCE;
            entry.Trustee.TrusteeForm = TRUSTEE_IS_SID;
            entry.Trustee.TrusteeType = TRUSTEE_IS_USER;
            entry.Trustee.ptstrName = static_cast<PWSTR>(principal);
            ea.push_back(entry);
        }

        std::vector<wil::unique_sid> appSids;
        appSids.reserve(accessRequestCount);
        for (auto const end = accessRequests + accessRequestCount; accessRequests < end; ++accessRequests)
        {
            EXPLICIT_ACCESS entry{};
            wil::unique_sid sid;
            THROW_IF_FAILED(::DeriveAppContainerSidFromAppContainerName(accessRequests->appContainerName, &sid));
            entry.grfAccessMode = GRANT_ACCESS;
            entry.grfAccessPermissions = accessRequests->accessMask;
            entry.grfInheritance = NO_INHERITANCE;
            entry.Trustee.TrusteeForm = TRUSTEE_IS_SID;
            entry.Trustee.TrusteeType = TRUSTEE_IS_USER;
            entry.Trustee.ptstrName = static_cast<PWSTR>(sid.get());
            appSids.push_back(std::move(sid));
            ea.push_back(entry);
        }

        wil::unique_any<PACL, decltype(&::LocalFree), ::LocalFree> acl;
        THROW_IF_WIN32_ERROR(::SetEntriesInAclW(static_cast<uint32_t>(ea.size()), ea.data(), nullptr, &acl));

        auto result = wil::make_unique_hlocal<SECURITY_DESCRIPTOR>();
        THROW_IF_WIN32_BOOL_FALSE(::InitializeSecurityDescriptor(result.get(), SECURITY_DESCRIPTOR_REVISION));
        THROW_IF_WIN32_BOOL_FALSE(::SetSecurityDescriptorDacl(result.get(), TRUE, acl.get(), FALSE));

        *securityDescriptor = result.release();
        return S_OK;
    }
    CATCH_RETURN();
}
