#pragma once
// Windows Internal Libraries (wil)
// ResultMacros.h:  WIL Error Handling Helpers Library supporting file (error handling macros)
//
// Usage Guidelines:
// https://osgwiki.com/wiki/WIL_Error_Handling_Helpers
//
// WIL Discussion Alias (wildisc):
// http://idwebelements/GroupManagement.aspx?Group=wildisc&Operation=join  (one-click join)
//
//! @file
//! WIL Error Handling Helpers: supporting file defining a family of macros and functions designed
//! to uniformly handle errors across return codes, fail fast, exceptions and logging

#ifndef __WIL_INTERNAL_RESULTMACROS_INCLUDED
#define __WIL_INTERNAL_RESULTMACROS_INCLUDED

// Common.h contains logic and configuration macros common to all consumers in Windows and therefore needs to be
// included before any 'opensource' headers
#include "Common.h"

#include <wil/result_macros.h>

/// @cond
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP | WINAPI_PARTITION_SYSTEM)
#if !defined(WIL_SUPPRESS_PRIVATE_API_USE) && !defined(__WIL_MIN_KERNEL) && !defined(WIL_KERNEL_MODE)
__declspec(selectany) HMODULE g_wil_details_ntdllModuleHandle = NULL;
__declspec(selectany) HMODULE g_wil_details_kernelbaseModuleHandle = NULL;

// ordinarily caching a module handle like this would be unsafe (could be unloaded) but ntdll
// and kernelbase cannot be unloaded from a process.
inline HMODULE wil_details_GetNtDllModuleHandle() WI_NOEXCEPT
{
    if (!g_wil_details_ntdllModuleHandle)
    {
        g_wil_details_ntdllModuleHandle = GetModuleHandleW(L"ntdll.dll");
    }
    return g_wil_details_ntdllModuleHandle;
}
inline HMODULE wil_details_GetKernelBaseModuleHandle() WI_NOEXCEPT
{
    if (!g_wil_details_kernelbaseModuleHandle)
    {
        g_wil_details_kernelbaseModuleHandle = GetModuleHandleW(L"kernelbase.dll");
    }
    return g_wil_details_kernelbaseModuleHandle;
}
#endif
#endif
/// @endcond

#if defined(__cplusplus) && !defined(__WIL_MIN_KERNEL) && !defined(WIL_KERNEL_MODE)

#pragma warning(push)
#pragma warning(disable:4714 6262)    // __forceinline not honored, stack size

// Old macros names -- to be removed -- do not use
#ifndef WIL_HIDE_DEPRECATED_1611
WIL_WARN_DEPRECATED_1611_PRAGMA("THROW_HR_IF_TRUE_MSG", "THROW_LAST_ERROR_IF_TRUE", "THROW_HR_IF_TRUE", "RETURN_IF_NTSTATUS_FAILED_LOG",
    "RETURN_LAST_ERROR_IF_NULL_LOG", "RETURN_LAST_ERROR_IF_FALSE_LOG", "RETURN_LAST_ERROR_IF_LOG", "RETURN_HR_IF_NULL_LOG",
    "RETURN_HR_IF_FALSE_LOG", "RETURN_HR_IF_LOG", "RETURN_IF_NULL_ALLOC_LOG", "RETURN_IF_HANDLE_NULL_LOG", "RETURN_IF_HANDLE_INVALID_LOG",
    "RETURN_IF_WIN32_ERROR_LOG", "RETURN_IF_WIN32_BOOL_FALSE_LOG", "RETURN_IF_FAILED_LOG", "RETURN_NTSTATUS_LOG", "RETURN_WIN32_LOG",
    "RETURN_LAST_ERROR_LOG", "RETURN_HR_LOG", "FAIL_FAST_HR_IF_TRUE", "LOG_LAST_ERROR_IF_TRUE", "LOG_HR_IF_TRUE_MSG", "LOG_HR_IF_TRUE",
    "RETURN_HR_IF_TRUE_EXPECTED", "RETURN_HR_IF_TRUE_MSG", "RETURN_HR_IF_TRUE_LOG", "RETURN_LAST_ERROR_IF_TRUE", "RETURN_HR_IF_TRUE",
    "THROW_LAST_ERROR_IF_FALSE_MSG", "THROW_HR_IF_FALSE_MSG", "THROW_IF_HANDLE_NULL_MSG", "THROW_IF_HANDLE_INVALID_MSG",
    "THROW_LAST_ERROR_IF_FALSE", "THROW_HR_IF_FALSE", "", "THROW_IF_HANDLE_INVALID", "FAIL_FAST_IMMEDIATE_IF_FALSE",
    "FAIL_FAST_IF_FALSE_MSG", "FAIL_FAST_IF_FALSE", "FAIL_FAST_LAST_ERROR_IF_FALSE_MSG", "FAIL_FAST_HR_IF_FALSE_MSG",
    "FAIL_FAST_IF_HANDLE_NULL_MSG", "FAIL_FAST_IF_HANDLE_INVALID_MSG", "FAIL_FAST_LAST_ERROR_IF_FALSE", "FAIL_FAST_HR_IF_FALSE",
    "FAIL_FAST_IF_HANDLE_NULL", "FAIL_FAST_IF_HANDLE_INVALID", "LOG_LAST_ERROR_IF_FALSE_MSG", "LOG_HR_IF_FALSE_MSG",
    "LOG_IF_HANDLE_NULL_MSG", "LOG_IF_HANDLE_INVALID_MSG", "LOG_LAST_ERROR_IF_FALSE", "LOG_HR_IF_FALSE", "LOG_IF_HANDLE_NULL",
    "LOG_IF_HANDLE_INVALID", "RETURN_LAST_ERROR_IF_FALSE_EXPECTED", "RETURN_HR_IF_FALSE_EXPECTED", "RETURN_IF_HANDLE_NULL_EXPECTED",
    "RETURN_IF_HANDLE_INVALID_EXPECTED", "RETURN_LAST_ERROR_IF_FALSE_MSG", "RETURN_HR_IF_FALSE_MSG", "RETURN_IF_HANDLE_NULL_MSG",
    "RETURN_IF_HANDLE_INVALID_MSG", "RETURN_LAST_ERROR_IF_FALSE", "RETURN_HR_IF_FALSE", "RETURN_IF_HANDLE_NULL", "RETURN_IF_HANDLE_INVALID")
    // DEPRECATED: Use RAII(unique_hfile) and RETURN_LAST_ERROR_IF(!handle)
#define RETURN_IF_HANDLE_INVALID(handle)                        do { HANDLE __hRet = (handle); if (__hRet == INVALID_HANDLE_VALUE) { __RETURN_GLE_FAIL(#handle); }} while (0, 0)
// DEPRECATED: Use RAII(unique_handle) and RETURN_LAST_ERROR_IF(!handle)
#define RETURN_IF_HANDLE_NULL(handle)                           do { HANDLE __hRet = (handle); if (__hRet == nullptr) { __RETURN_GLE_FAIL(#handle); }} while (0, 0)
// DEPRECATED: Use RETURN_HR_IF(hr, !condition)
#define RETURN_HR_IF_FALSE(hr, condition)                       RETURN_HR_IF(hr, !(wil::verify_bool(condition)))
// DEPRECATED: Use RETURN_LAST_ERROR_IF(!condition)
#define RETURN_LAST_ERROR_IF_FALSE(condition)                   RETURN_LAST_ERROR_IF(!(wil::verify_bool(condition)))
// DEPRECATED: Use RAII(unique_hfile) and RETURN_ERROR_IF_MSG(!handle, fmt, ...)
#define RETURN_IF_HANDLE_INVALID_MSG(handle, fmt, ...)          do { HANDLE __hRet = (handle); if (__hRet == INVALID_HANDLE_VALUE) { __RETURN_GLE_MSG_FAIL(#handle, fmt, __VA_ARGS__); }} while (0, 0)
// DEPRECATED: Use RAII(unique_handle) and RETURN_LAST_ERROR_IF_MSG(!handle, fmt, ...)
#define RETURN_IF_HANDLE_NULL_MSG(handle, fmt, ...)             do { HANDLE __hRet = (handle); if (__hRet == nullptr) { __RETURN_GLE_MSG_FAIL(#handle, fmt, __VA_ARGS__); }} while (0, 0)
// DEPRECATED: Use RETURN_HR_IF_MSG(hr, !condition, fmt, ...)
#define RETURN_HR_IF_FALSE_MSG(hr, condition, fmt, ...)         RETURN_HR_IF_MSG(hr, !(wil::verify_bool(condition)), fmt, __VA_ARGS__)
// DEPRECATED: Use RETURN_LAST_ERROR_IF_MSG(!condition, fmt, ...)
#define RETURN_LAST_ERROR_IF_FALSE_MSG(condition, fmt, ...)     RETURN_LAST_ERROR_IF_MSG(!(wil::verify_bool(condition)), fmt, __VA_ARGS__)
// DEPRECATED: Use RAII(unique_hfile) and RETURN_LAST_ERROR_IF_EXPECTED(!handle)
#define RETURN_IF_HANDLE_INVALID_EXPECTED(handle)               do { HANDLE __hRet = (handle); if (__hRet == INVALID_HANDLE_VALUE) { return wil::details::GetLastErrorFailHr(); }} while (0, 0)
// DEPRECATED: Use RAII(unique_handle) and RETURN_LAST_ERROR_IF_EXPECTED(!handle)
#define RETURN_IF_HANDLE_NULL_EXPECTED(handle)                  do { HANDLE __hRet = (handle); if (__hRet == nullptr) { return wil::details::GetLastErrorFailHr(); }} while (0, 0)
// DEPRECATED: Use RETURN_HR_IF_EXPECTED(hr, !condition)
#define RETURN_HR_IF_FALSE_EXPECTED(hr, condition)              RETURN_HR_IF_EXPECTED(hr, !(wil::verify_bool(condition)))
// DEPRECATED: Use RETURN_LAST_ERROR_IF_EXPECTED(!condition)
#define RETURN_LAST_ERROR_IF_FALSE_EXPECTED(condition)          RETURN_LAST_ERROR_IF_EXPECTED(!(wil::verify_bool(condition)))
// DEPRECATED: Use RAII(unique_hfile) and LOG_ERROR_IF(!handle)
#define LOG_IF_HANDLE_INVALID(handle)                           __R_FN(Log_IfHandleInvalid)(__R_INFO(#handle) handle)
// DEPRECATED: Use RAII(unique_handle) and LOG_LAST_ERROR_IF(!handle)
#define LOG_IF_HANDLE_NULL(handle)                              __R_FN(Log_IfHandleNull)(__R_INFO(#handle) handle)
// DEPRECATED: Use LOG_HR_IF(hr, !condition)
#define LOG_HR_IF_FALSE(hr, condition)                          __R_FN(Log_HrIfFalse)(__R_INFO(#condition) wil::verify_hresult(hr), wil::verify_bool(condition))
// DEPRECATED: Use LOG_LAST_ERROR_IF(!condition)
#define LOG_LAST_ERROR_IF_FALSE(condition)                      __R_FN(Log_GetLastErrorIfFalse)(__R_INFO(#condition) wil::verify_bool(condition))
// DEPRECATED: Use RAII(unique_hfile) and LOG_ERROR_IF_MSG(!handle, fmt, ...)
#define LOG_IF_HANDLE_INVALID_MSG(handle, fmt, ...)             __R_FN(Log_IfHandleInvalidMsg)(__R_INFO(#handle) handle, fmt, __VA_ARGS__)
// DEPRECATED: Use RAII(unique_handle) and LOG_LAST_ERROR_IF_MSG(!handle, fmt, ...)
#define LOG_IF_HANDLE_NULL_MSG(handle, fmt, ...)                __R_FN(Log_IfHandleNullMsg)(__R_INFO(#handle) handle, fmt, __VA_ARGS__)
// DEPRECATED: Use LOG_HR_IF_MSG(hr, !condition, fmt, ...)
#define LOG_HR_IF_FALSE_MSG(hr, condition, fmt, ...)            __R_FN(Log_HrIfFalseMsg)(__R_INFO(#condition) wil::verify_hresult(hr), wil::verify_bool(condition), fmt, __VA_ARGS__)
// DEPRECATED: Use LOG_LAST_ERROR_IF_MSG(!condition, fmt, ...)
#define LOG_LAST_ERROR_IF_FALSE_MSG(condition, fmt, ...)        __R_FN(Log_GetLastErrorIfFalseMsg)(__R_INFO(#condition) wil::verify_bool(condition), fmt, __VA_ARGS__)
// DEPRECATED: Use RAII(unique_hfile) and FAIL_FAST_ERROR_IF(!handle)
#define FAIL_FAST_IF_HANDLE_INVALID(handle)                     __RFF_FN(FailFast_IfHandleInvalid)(__RFF_INFO(#handle) handle)
// DEPRECATED: Use RAII(unique_handle) and FAIL_FAST_LAST_ERROR_IF(!handle)
#define FAIL_FAST_IF_HANDLE_NULL(handle)                        __RFF_FN(FailFast_IfHandleNull)(__RFF_INFO(#handle) handle)
// DEPRECATED: Use FAIL_FAST_HR_IF(hr, !condition)
#define FAIL_FAST_HR_IF_FALSE(hr, condition)                    __RFF_FN(FailFast_HrIfFalse)(__RFF_INFO(#condition) wil::verify_hresult(hr), wil::verify_bool(condition))
// DEPRECATED: Use FAIL_FAST_LAST_ERROR_IF(!condition)
#define FAIL_FAST_LAST_ERROR_IF_FALSE(condition)                __RFF_FN(FailFast_GetLastErrorIfFalse)(__RFF_INFO(#condition) wil::verify_bool(condition))
// DEPRECATED: Use RAII(unique_hfile) and FAIL_FAST_ERROR_IF_MSG(!handle, fmt, ...)
#define FAIL_FAST_IF_HANDLE_INVALID_MSG(handle, fmt, ...)       __RFF_FN(FailFast_IfHandleInvalidMsg)(__RFF_INFO(#handle) handle, fmt, __VA_ARGS__)
// DEPRECATED: Use RAII(unique_handle) and FAIL_FAST_LAST_ERROR_IF_MSG(!handle, fmt, ...)
#define FAIL_FAST_IF_HANDLE_NULL_MSG(handle, fmt, ...)          __RFF_FN(FailFast_IfHandleNullMsg)(__RFF_INFO(#handle) handle, fmt, __VA_ARGS__)
// DEPRECATED: Use FAIL_FAST_HR_IF_MSG(hr, !condition, fmt, ...)
#define FAIL_FAST_HR_IF_FALSE_MSG(hr, condition, fmt, ...)      __RFF_FN(FailFast_HrIfFalseMsg)(__RFF_INFO(#condition) wil::verify_hresult(hr), wil::verify_bool(condition), fmt, __VA_ARGS__)
// DEPRECATED: Use FAIL_FAST_LAST_ERROR_IF_MSG(!condition, fmt, ...)
#define FAIL_FAST_LAST_ERROR_IF_FALSE_MSG(condition, fmt, ...)  __RFF_FN(FailFast_GetLastErrorIfFalseMsg)(__RFF_INFO(#condition) wil::verify_bool(condition), fmt, __VA_ARGS__)
// DEPRECATED: Use FAIL_FAST_IF(!condition)
#define FAIL_FAST_IF_FALSE(condition)                           __RFF_FN(FailFast_IfFalse)(__RFF_INFO(#condition) wil::verify_bool(condition))
// DEPRECATED: Use FAIL_FAST_IF_MSG(!condition, fmt, ...)
#define FAIL_FAST_IF_FALSE_MSG(condition, fmt, ...)             __RFF_FN(FailFast_IfFalseMsg)(__RFF_INFO(#condition) wil::verify_bool(condition), fmt, __VA_ARGS__)
// DEPRECATED: Use FAIL_FAST_IMMEDIATE_IF(condition)
#define FAIL_FAST_IMMEDIATE_IF_FALSE(condition)                 __RFF_FN(FailFastImmediate_IfFalse)(wil::verify_bool(condition))
// DEPRECATED: Use RAII(unique_hfile) and THROW_LAST_ERROR_IF(!handle)
#define THROW_IF_HANDLE_INVALID(handle)                         __R_FN(Throw_IfHandleInvalid)(__R_INFO(#handle) handle)
// DEPRECATED: Use RAII(unique_handle) and THROW_LAST_ERROR_IF(!handle)
#define THROW_IF_HANDLE_NULL(handle)                            __R_FN(Throw_IfHandleNull)(__R_INFO(#handle) handle)
// DEPRECATED: Use THROW_HR_IF(hr, !condition)
#define THROW_HR_IF_FALSE(hr, condition)                        __R_FN(Throw_HrIfFalse)(__R_INFO(#condition) wil::verify_hresult(hr), wil::verify_bool(condition))
// DEPRECATED: Use THROW_LAST_ERROR_IF(!condition)
#define THROW_LAST_ERROR_IF_FALSE(condition)                    __R_FN(Throw_GetLastErrorIfFalse)(__R_INFO(#condition) wil::verify_bool(condition))
// DEPRECATED: Use RAII(unique_hfile) and THROW_ERROR_IF_MSG(!handle, fmt, ...)
#define THROW_IF_HANDLE_INVALID_MSG(handle, fmt, ...)           __R_FN(Throw_IfHandleInvalidMsg)(__R_INFO(#handle) handle, fmt, __VA_ARGS__)
// DEPRECATED: Use RAII(unique_handle) and THROW_LAST_ERROR_IF_MSG(!handle, fmt, ...)
#define THROW_IF_HANDLE_NULL_MSG(handle, fmt, ...)              __R_FN(Throw_IfHandleNullMsg)(__R_INFO(#handle) handle, fmt, __VA_ARGS__)
// DEPRECATED: Use THROW_HR_IF_MSG(hr, !condition, fmt, ...)
#define THROW_HR_IF_FALSE_MSG(hr, condition, fmt, ...)          __R_FN(Throw_HrIfFalseMsg)(__R_INFO(#condition) wil::verify_hresult(hr), wil::verify_bool(condition), fmt, __VA_ARGS__)
// DEPRECATED: Use THROW_LAST_ERROR_IF_MSG(!condition, fmt, ...)
#define THROW_LAST_ERROR_IF_FALSE_MSG(condition, fmt, ...)      __R_FN(Throw_GetLastErrorIfFalseMsg)(__R_INFO(#condition) wil::verify_bool(condition), fmt, __VA_ARGS__)
// DEPRECATED: Use RETURN_HR_IF
#define RETURN_HR_IF_TRUE               RETURN_HR_IF
// DEPRECATED: Use RETURN_LAST_ERROR_IF
#define RETURN_LAST_ERROR_IF_TRUE       RETURN_LAST_ERROR_IF
// DEPRECATED: Use RETURN_HR_IF_LOG
#define RETURN_HR_IF_TRUE_LOG           RETURN_HR_IF_LOG
// DEPRECATED: Use RETURN_HR_IF_MSG
#define RETURN_HR_IF_TRUE_MSG           RETURN_HR_IF_MSG
// DEPRECATED: Use RETURN_HR_IF_EXPECTED
#define RETURN_HR_IF_TRUE_EXPECTED      RETURN_HR_IF_EXPECTED
// DEPRECATED: Use LOG_HR_IF
#define LOG_HR_IF_TRUE                  LOG_HR_IF
// DEPRECATED: Use LOG_HR_IF_MSG
#define LOG_HR_IF_TRUE_MSG              LOG_HR_IF_MSG
// DEPRECATED: Use LOG_LAST_ERROR_IF
#define LOG_LAST_ERROR_IF_TRUE          LOG_LAST_ERROR_IF
// DEPRECATED: Use FAIL_FAST_HR_IF
#define FAIL_FAST_HR_IF_TRUE            FAIL_FAST_HR_IF
// DEPRECATED: Use RETURN_HR
#define RETURN_HR_LOG                   RETURN_HR
// DEPRECATED: Use RETURN_LAST_ERROR
#define RETURN_LAST_ERROR_LOG           RETURN_LAST_ERROR
// DEPRECATED: Use RETURN_WIN32
#define RETURN_WIN32_LOG                RETURN_WIN32
// DEPRECATED: Use RETURN_NTSTATUS
#define RETURN_NTSTATUS_LOG             RETURN_NTSTATUS
// DEPRECATED: Use RETURN_IF_FAILED
#define RETURN_IF_FAILED_LOG            RETURN_IF_FAILED
// DEPRECATED: Use RETURN_IF_WIN32_BOOL_FALSE
#define RETURN_IF_WIN32_BOOL_FALSE_LOG  RETURN_IF_WIN32_BOOL_FALSE
// DEPRECATED: Use RETURN_IF_WIN32_ERROR
#define RETURN_IF_WIN32_ERROR_LOG       RETURN_IF_WIN32_ERROR
// DEPRECATED: Use RETURN_IF_HANDLE_INVALID
#define RETURN_IF_HANDLE_INVALID_LOG    RETURN_IF_HANDLE_INVALID
// DEPRECATED: Use RETURN_IF_HANDLE_NULL
#define RETURN_IF_HANDLE_NULL_LOG       RETURN_IF_HANDLE_NULL
// DEPRECATED: Use RETURN_IF_NULL_ALLOC
#define RETURN_IF_NULL_ALLOC_LOG        RETURN_IF_NULL_ALLOC
// DEPRECATED: Use RETURN_HR_IF
#define RETURN_HR_IF_LOG                RETURN_HR_IF
// DEPRECATED: Use RETURN_HR_IF_FALSE
#define RETURN_HR_IF_FALSE_LOG          RETURN_HR_IF_FALSE
// DEPRECATED: Use RETURN_HR_IF_NULL
#define RETURN_HR_IF_NULL_LOG           RETURN_HR_IF_NULL
// DEPRECATED: Use RETURN_LAST_ERROR_IF
#define RETURN_LAST_ERROR_IF_LOG        RETURN_LAST_ERROR_IF
// DEPRECATED: Use RETURN_LAST_ERROR_IF_FALSE
#define RETURN_LAST_ERROR_IF_FALSE_LOG  RETURN_LAST_ERROR_IF_FALSE
// DEPRECATED: Use RETURN_LAST_ERROR_IF_NULL
#define RETURN_LAST_ERROR_IF_NULL_LOG   RETURN_LAST_ERROR_IF_NULL
// DEPRECATED: Use RETURN_IF_NTSTATUS_FAILED
#define RETURN_IF_NTSTATUS_FAILED_LOG   RETURN_IF_NTSTATUS_FAILED

#ifdef WIL_ENABLE_EXCEPTIONS
// DEPRECATED: Use THROW_HR_IF
#define THROW_HR_IF_TRUE                THROW_HR_IF
// DEPRECATED: Use THROW_LAST_ERROR_IF
#define THROW_LAST_ERROR_IF_TRUE        THROW_LAST_ERROR_IF
// DEPRECATED: Use THROW_HR_IF_MSG
#define THROW_HR_IF_TRUE_MSG            THROW_HR_IF_MSG
#endif  // WIL_ENABLE_EXCEPTIONS
#endif // WIL_HIDE_DEPRECATED_1611

#ifndef WIL_SUPPRESS_PRIVATE_API_USE

#if !defined(_NTSYSTEM_)
#define NTSYSAPI     DECLSPEC_IMPORT
#else
#define NTSYSAPI
#endif

extern "C" NTSYSAPI VOID NTAPI LdrFastFailInLoaderCallout(VOID);
#endif

namespace wil
{
    /// @cond
    namespace details
    {
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP | WINAPI_PARTITION_SYSTEM)
#ifndef WIL_SUPPRESS_PRIVATE_API_USE
        inline void __stdcall FailFastInLoaderCallout() WI_NOEXCEPT
        {
            // GetProcAddress is used here since we may be linking against an ntdll that doesn't contain this function
            // e.g. if this header is used on an OS older than Threshold (which may be the case for IE)
            auto pfn = reinterpret_cast<decltype(&::LdrFastFailInLoaderCallout)>(GetProcAddress(wil_details_GetNtDllModuleHandle(), "LdrFastFailInLoaderCallout"));
            if (pfn != nullptr)
            {
                pfn();  // don't do anything non-trivial from DllMain, fail fast.
            }
        }

        inline ULONG __stdcall RtlNtStatusToDosErrorNoTeb(_In_ NTSTATUS status) WI_NOEXCEPT
        {
            static decltype(RtlNtStatusToDosErrorNoTeb)* s_pfnRtlNtStatusToDosErrorNoTeb = nullptr;
            if (s_pfnRtlNtStatusToDosErrorNoTeb == nullptr)
            {
                s_pfnRtlNtStatusToDosErrorNoTeb = reinterpret_cast<decltype(RtlNtStatusToDosErrorNoTeb)*>(GetProcAddress(wil_details_GetNtDllModuleHandle(), "RtlNtStatusToDosErrorNoTeb"));
            }
            return s_pfnRtlNtStatusToDosErrorNoTeb ? s_pfnRtlNtStatusToDosErrorNoTeb(status) : 0;
        }

        inline BOOLEAN __stdcall RtlDllShutdownInProgress() WI_NOEXCEPT
        {
            static decltype(RtlDllShutdownInProgress)* s_pfnRtlDllShutdownInProgress = nullptr;
            if (s_pfnRtlDllShutdownInProgress == nullptr)
            {
                s_pfnRtlDllShutdownInProgress = reinterpret_cast<decltype(RtlDllShutdownInProgress)*>(GetProcAddress(wil_details_GetNtDllModuleHandle(), "RtlDllShutdownInProgress"));
            }
            return s_pfnRtlDllShutdownInProgress ? s_pfnRtlDllShutdownInProgress() : FALSE;
        }

        inline NTSTATUS __stdcall RtlDisownModuleHeapAllocation(_In_ HANDLE heapHandle, _In_ PVOID address) WI_NOEXCEPT
        {
            static decltype(RtlDisownModuleHeapAllocation)* s_pfnRtlDisownModuleHeapAllocation = nullptr;
            if (s_pfnRtlDisownModuleHeapAllocation == nullptr)
            {
                s_pfnRtlDisownModuleHeapAllocation = reinterpret_cast<decltype(RtlDisownModuleHeapAllocation)*>(GetProcAddress(wil_details_GetNtDllModuleHandle(), "RtlDisownModuleHeapAllocation"));
            }
            return s_pfnRtlDisownModuleHeapAllocation ? s_pfnRtlDisownModuleHeapAllocation(heapHandle, address) : STATUS_SUCCESS;
        }

        inline void __stdcall FormatNtStatusMsg(NTSTATUS status,
            _Out_writes_(messageStringSizeChars) _Post_z_ PWSTR messageString,
            _Pre_satisfies_(messageStringSizeChars > 0) DWORD messageStringSizeChars)
        {
            FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
                wil_details_GetNtDllModuleHandle(), status, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                messageString, messageStringSizeChars, nullptr);
        }


#endif // !DEFINED(WIL_SUPPRESS_PRIVATE_API_USE)
#endif  // WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP | WINAPI_PARTITION_SYSTEM)

#ifndef WIL_HIDE_DEPRECATED_1611
        template <typename TLambda>
        class ScopeExitFn
        {
        public:
            explicit ScopeExitFn(TLambda&& exitFunctor) WI_NOEXCEPT :
                m_exitFunctor(wistd::move(exitFunctor)),
                m_shouldRun(true)
            {
                static_assert(!wistd::is_lvalue_reference<TLambda>::value && !wistd::is_rvalue_reference<TLambda>::value,
                    "ScopeExit should only be directly used with lambdas");
            }

            ScopeExitFn(ScopeExitFn&& other) WI_NOEXCEPT :
            m_exitFunctor(wistd::move(other.m_exitFunctor)),
                m_shouldRun(other.m_shouldRun)
            {
                other.m_shouldRun = false;
            }

            ~ScopeExitFn() WI_NOEXCEPT
            {
                operator()();
            }

            void operator()() WI_NOEXCEPT
            {
                if (m_shouldRun)
                {
                    m_shouldRun = false;
                    m_exitFunctor();
                }
            }

            void Dismiss() WI_NOEXCEPT
            {
                m_shouldRun = false;
            }

            ScopeExitFn(ScopeExitFn const&) = delete;
            ScopeExitFn& operator=(ScopeExitFn const&) = delete;

        private:
            TLambda m_exitFunctor;
            bool m_shouldRun;
        };

#endif // WIL_HIDE_DEPRECATED_1611
    } // details namespace
    /// @endcond

    //*****************************************************************************
    // Calling WilInitialize_ResultMacros_DesktopOrSystem additionally provides:
    // - FAIL_FAST_IMMEDIATE_IF_IN_LOADER_CALLOUT enforcement
    // - Higher fidelity mapping of NTSTATUS->HRESULT for RETURN_IF_NTSTATUS*
    // - wil::ProcessShutdownInProgress returns 'true' during process shutdown (false when not called or set)
    //*****************************************************************************

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP | WINAPI_PARTITION_SYSTEM)
#ifndef WIL_SUPPRESS_PRIVATE_API_USE
    //! Call this method to initialize WIL manually in a module where RESULT_SUPPRESS_STATIC_INITIALIZERS is required. WIL will
    //! use internal methods to provide additional diagnostic information & behavior.
    inline void WilInitialize_ResultMacros_DesktopOrSystem()
    {
        WilInitialize_ResultMacros_DesktopOrSystem_SuppressPrivateApiUse();
        details::g_pfnFailFastInLoaderCallout = details::FailFastInLoaderCallout;
        details::g_pfnRtlNtStatusToDosErrorNoTeb = details::RtlNtStatusToDosErrorNoTeb;
        details::g_pfnDllShutdownInProgress = details::RtlDllShutdownInProgress;
        details::g_pfnRtlDisownModuleHeapAllocation = details::RtlDisownModuleHeapAllocation;
        details::g_pfnFormatNtStatusMsg = details::FormatNtStatusMsg;
    }
#endif // WIL_SUPPRESS_PRIVATE_API_USE

    /// @cond
    namespace details
    {
#ifndef RESULT_SUPPRESS_STATIC_INITIALIZERS
#ifndef WIL_SUPPRESS_PRIVATE_API_USE
        WI_HEADER_INITITALIZATION_FUNCTION(WilInitialize_ResultMacros_DesktopOrSystem, []
            {
                ::wil::WilInitialize_ResultMacros_DesktopOrSystem();
                return 1;
            });
#endif // WIL_SUPPRESS_PRIVATE_API_USE
#endif // RESULT_SUPPRESS_STATIC_INITIALIZERS
    }
    /// @endcond
#endif // WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP | WINAPI_PARTITION_SYSTEM)

#ifndef WIL_HIDE_DEPRECATED_1611
    WIL_WARN_DEPRECATED_1611_PRAGMA(ScopeExit)

        // DEPRECATED: use wil::scope_exit
        template <typename TLambda>
    WI_NODISCARD inline wil::details::ScopeExitFn<TLambda> ScopeExit(TLambda&& exitFunctor) WI_NOEXCEPT
    {
        return wil::details::ScopeExitFn<TLambda>(wistd::forward<TLambda>(exitFunctor));
    }
#endif

    //! @cond
    namespace details
    {
#ifndef WIL_HIDE_DEPRECATED_1611
#ifdef WIL_ENABLE_EXCEPTIONS
        enum class GuardType : unsigned char
        {
            Ignore,
            Log,
            FailFast
        };

        template <typename TLambda>
        class ScopeExitFnGuard
        {
        public:
            explicit ScopeExitFnGuard(GuardType type, TLambda&& exitFunctor) WI_NOEXCEPT :
                m_exitFunctor(wistd::move(exitFunctor)),
                m_type(type)
            {
                static_assert(!wistd::is_lvalue_reference<TLambda>::value && !wistd::is_rvalue_reference<TLambda>::value,
                    "ScopeExit should only be directly used with lambdas");
            }

            ScopeExitFnGuard(ScopeExitFnGuard&& other) WI_NOEXCEPT :
            m_exitFunctor(wistd::move(other.m_exitFunctor)),
                m_type(other.m_type),
                m_shouldRun(other.m_shouldRun)
            {
                other.m_shouldRun = false;
            }

            ~ScopeExitFnGuard() WI_NOEXCEPT
            {
                operator()();
            }

            void operator()() WI_NOEXCEPT
            {
                try
                {
                    RunWithoutGuard();
                }
                catch (...)
                {
                    if (m_type == GuardType::FailFast)
                    {
                        __WIL_PRIVATE_FAIL_FAST_HR(wil::ResultFromCaughtException());
                    }
                    else if (m_type == GuardType::Log)
                    {
                        __WIL_PRIVATE_LOG_HR(wil::ResultFromCaughtException());
                    }
                    // GuardType::Ignore = no-op
                }
            }

            void RunWithoutGuard()
            {
                if (m_shouldRun)
                {
                    m_shouldRun = false;
                    m_exitFunctor();
                }
            }

            void Dismiss() WI_NOEXCEPT
            {
                m_shouldRun = false;
            }

            ScopeExitFnGuard(const ScopeExitFnGuard&) = delete;
            ScopeExitFnGuard& operator=(const ScopeExitFnGuard&) = delete;

        private:
            TLambda m_exitFunctor;
            GuardType m_type;
            bool m_shouldRun = true;
        };
    }

    WIL_WARN_DEPRECATED_1611_PRAGMA(ScopeExitNoExcept, ScopeExitLog, ScopeExitIgnore)

        // DEPRECATED: use wil::scope_exit
        template <typename TLambda>
    WI_NODISCARD inline wil::details::ScopeExitFnGuard<TLambda> ScopeExitNoExcept(TLambda&& exitFunctor) WI_NOEXCEPT
    {
        return wil::details::ScopeExitFnGuard<TLambda>(wil::details::GuardType::FailFast, wistd::forward<TLambda>(exitFunctor));
    }

    // DEPRECATED: use wil::scope_exit_log(WI_DIAGNOSTICS_INFO, [&] {})
    template <typename TLambda>
    WI_NODISCARD inline wil::details::ScopeExitFnGuard<TLambda> ScopeExitLog(TLambda&& exitFunctor) WI_NOEXCEPT
    {
        return wil::details::ScopeExitFnGuard<TLambda>(wil::details::GuardType::Log, wistd::forward<TLambda>(exitFunctor));
    }

    // DEPRECATED: use wil::scope_exit_log or try/catch(...) within the lambda
    template <typename TLambda>
    WI_NODISCARD inline wil::details::ScopeExitFnGuard<TLambda> ScopeExitIgnore(TLambda&& exitFunctor) WI_NOEXCEPT
    {
        return wil::details::ScopeExitFnGuard<TLambda>(wil::details::GuardType::Ignore, wistd::forward<TLambda>(exitFunctor));
    }

    namespace details {

#endif // WIL_ENABLE_EXCEPTIONS
#endif // WIL_HIDE_DEPRECATED_1611
    }   // details namespace
    /// @endcond
} // namespace wil

#pragma warning(pop)

#endif // defined(__cplusplus) && !defined(__WIL_MIN_KERNEL) && !defined(WIL_KERNEL_MODE)

#endif // __WIL_INTERNAL_RESULTMACROS_INCLUDED
