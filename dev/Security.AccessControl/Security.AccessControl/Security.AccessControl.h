#pragma once

#include <Windows.h>
#include <stdint.h>

struct AppContainerNameAndAccess
{
    PCWSTR appContainerName;
    uint32_t accessMask;
};

STDAPI GetSecurityDescriptorForAppContainerNames(
    uint32_t accessRequestCount,
    _In_reads_(accessRequestCount)
        const AppContainerNameAndAccess* accessRequests,
    _In_opt_ PSID principal,
    uint32_t principalAccessMask,
    _Outptr_ PSECURITY_DESCRIPTOR* securityDescriptor
);
