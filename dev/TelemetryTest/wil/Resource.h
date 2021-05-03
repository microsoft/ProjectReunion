#pragma once
// Windows Internal Libraries (wil)
// Resource.h: WIL Resource Wrappers (RAII) Library
//
// Usage Guidelines:
// https://osgwiki.com/wiki/WIL_Resource_Wrappers_(RAII)
//
// wil Discussion Alias (wildisc):
// http://idwebelements/GroupManagement.aspx?Group=wildisc&Operation=join  (one-click join)
//
//! @file
//! WIL Resource Wrappers (RAII): Provides a family of smart pointer patterns and resource wrappers to enable customers to
//! consistently use RAII in all code.

// Common.h contains logic and configuration macros common to all consumers in Windows and therefore needs to be
// included before any 'opensource' headers
#include "Common.h"

#include <wil/resource.h>

// Internal WIL header files that were historically included before WIL was split into public and internal versions.
// These exist only to maintain back-compat
#include "ResultMacros.h"

#pragma warning(push)
#pragma warning(disable:26135 26110)    // Missing locking annotation, Caller failing to hold lock
#pragma warning(disable:4714)           // __forceinline not honored

#ifndef __WIL_INTERNAL_RESOURCE
#define __WIL_INTERNAL_RESOURCE
namespace wil
{
    /// @cond
#ifndef WIL_HIDE_DEPRECATED_1611
    WIL_WARN_DEPRECATED_1611_PRAGMA(DetachToOptParam)

        // DEPRECATED: Use wil::detach_to_opt_param
        template <typename T, typename TSmartPointer>
    inline void DetachToOptParam(_Out_opt_ T* outParam, TSmartPointer&& smartPtr)
    {
        detach_to_opt_param(outParam, wistd::forward<TSmartPointer>(smartPtr));
    }
#endif
    /// @endcond
}
#endif // __WIL_INTERNAL_RESOURCE

namespace wil
{
#ifdef __WIL_WINBASE_
    /// @cond
    namespace details
    {
#if defined(_NTURTL_) && !defined(__WIL_NTURTL_THREADPOOL_)
#define __WIL_NTURTL_THREADPOOL_
        // RTL implementation for threadpool_t parameter of DestroyThreadPoolTimer<>
        struct RtlThreadPoolMethods
        {
            static void WINAPI SetThreadpoolTimer(_Inout_ PTP_TIMER Timer, _In_opt_ PFILETIME DueTime, _In_ DWORD Period, _In_opt_ DWORD WindowLength) WI_NOEXCEPT
            {
                ::TpSetTimer(Timer, reinterpret_cast<TP_TIMESTAMP*>(DueTime), Period, WindowLength);
            }
            static void WaitForThreadpoolTimerCallbacks(_Inout_ PTP_TIMER Timer, _In_ BOOL CancelPendingCallbacks) WI_NOEXCEPT
            {
                ::TpWaitForTimer(Timer, CancelPendingCallbacks);
            }
            static void CloseThreadpoolTimer(_Inout_ PTP_TIMER Timer) WI_NOEXCEPT
            {
                ::TpReleaseTimer(Timer);
            }
        };
#endif //__WIL_NTURTL_THREADPOOL_
    }
    /// @endcond

#if defined(__WIL_NTURTL_THREADPOOL_)
    typedef unique_any<PTP_TIMER, void(*)(PTP_TIMER), details::DestroyThreadPoolTimer<details::RtlThreadPoolMethods, details::PendingCallbackCancellationBehavior::Cancel>::Destroy> unique_rtlthreadpool_timer;
    typedef unique_any<PTP_TIMER, void(*)(PTP_TIMER), details::DestroyThreadPoolTimer<details::RtlThreadPoolMethods, details::PendingCallbackCancellationBehavior::Wait>::Destroy> unique_rtlthreadpool_timer_nocancel;
    typedef unique_any<PTP_TIMER, void(*)(PTP_TIMER), details::DestroyThreadPoolTimer<details::RtlThreadPoolMethods, details::PendingCallbackCancellationBehavior::NoWait>::Destroy> unique_rtlthreadpool_timer_nowait;
#endif

    // nturtl_wrap.h changes the definition of RTL_SRWLOCK such that calls to Rtl*SRWLock*(RTL_SRWLOCK*) will fail to compile
    // Therefore, exclude these wrappers if _NTURTL_WRAP_H_ is defined
#if defined(_NTURTL_) && !defined(__WIL_NTURTL_SRWLOCK_) && !defined(_NTURTL_WRAP_H_)
#define __WIL_NTURTL_SRWLOCK_
    typedef unique_any<RTL_SRWLOCK*, decltype(&::RtlReleaseSRWLockExclusive), ::RtlReleaseSRWLockExclusive, details::pointer_access_none> rtlrwlock_release_exclusive_scope_exit;
    typedef unique_any<RTL_SRWLOCK*, decltype(&::RtlReleaseSRWLockShared), ::RtlReleaseSRWLockShared, details::pointer_access_none> rtlrwlock_release_shared_scope_exit;

    WI_NODISCARD inline rtlrwlock_release_exclusive_scope_exit RtlAcquireSRWLockExclusive(_Inout_ RTL_SRWLOCK* lock) WI_NOEXCEPT
    {
        ::RtlAcquireSRWLockExclusive(lock);
        return rtlrwlock_release_exclusive_scope_exit(lock);
    }

    WI_NODISCARD inline rtlrwlock_release_shared_scope_exit RtlAcquireSRWLockShared(_Inout_ RTL_SRWLOCK* lock) WI_NOEXCEPT
    {
        ::RtlAcquireSRWLockShared(lock);
        return rtlrwlock_release_shared_scope_exit(lock);
    }

    WI_NODISCARD inline rtlrwlock_release_exclusive_scope_exit RtlTryAcquireSRWLockExclusive(_Inout_ RTL_SRWLOCK* lock) WI_NOEXCEPT
    {
        return rtlrwlock_release_exclusive_scope_exit(::RtlTryAcquireSRWLockExclusive(lock) ? lock : nullptr);
    }

    WI_NODISCARD inline rtlrwlock_release_shared_scope_exit RtlTryAcquireSRWLockShared(_Inout_ RTL_SRWLOCK* lock) WI_NOEXCEPT
    {
        return rtlrwlock_release_shared_scope_exit(::RtlTryAcquireSRWLockShared(lock) ? lock : nullptr);
    }

    class rtlsrwlock
    {
    public:
        rtlsrwlock(const rtlsrwlock&) = delete;
        rtlsrwlock(rtlsrwlock&&) = delete;
        rtlsrwlock& operator=(const rtlsrwlock&) = delete;
        rtlsrwlock& operator=(rtlsrwlock&&) = delete;

        rtlsrwlock() = default;

        WI_NODISCARD rtlrwlock_release_exclusive_scope_exit lock_exclusive() WI_NOEXCEPT
        {
            return wil::RtlAcquireSRWLockExclusive(&m_lock);
        }

        WI_NODISCARD rtlrwlock_release_exclusive_scope_exit try_lock_exclusive() WI_NOEXCEPT
        {
            return wil::RtlTryAcquireSRWLockExclusive(&m_lock);
        }

        WI_NODISCARD rtlrwlock_release_shared_scope_exit lock_shared() WI_NOEXCEPT
        {
            return wil::RtlAcquireSRWLockShared(&m_lock);
        }

        WI_NODISCARD rtlrwlock_release_shared_scope_exit try_lock_shared() WI_NOEXCEPT
        {
            return wil::RtlTryAcquireSRWLockShared(&m_lock);
        }

    private:
        RTL_SRWLOCK m_lock = SRWLOCK_INIT;
    };
#endif //__WIL_NTURTL_SRWLOCK_

#if defined(_NTURTL_) && !defined(__WIL_NTURTL_CRITSEC_)
#define __WIL_NTURTL_CRITSEC_
    typedef unique_any<RTL_CRITICAL_SECTION*, decltype(&::RtlLeaveCriticalSection), RtlLeaveCriticalSection, details::pointer_access_none> rtlcs_leave_scope_exit;

    WI_NODISCARD inline rtlcs_leave_scope_exit RtlEnterCriticalSection(_Inout_ RTL_CRITICAL_SECTION* cs) WI_NOEXCEPT
    {
        ::RtlEnterCriticalSection(cs);
        return rtlcs_leave_scope_exit(cs);
    }

    WI_NODISCARD inline rtlcs_leave_scope_exit RtlTryEnterCriticalSection(_Inout_ RTL_CRITICAL_SECTION* cs) WI_NOEXCEPT
    {
        return rtlcs_leave_scope_exit(::RtlTryEnterCriticalSection(cs) ? cs : nullptr);
    }

    // Critical sections are worse than srwlocks in performance and memory usage (their only unique attribute
    // being recursive acquisition). Prefer srwlocks over critical sections when you don't need recursive acquisition.
    class rtlcritical_section
    {
    public:
        rtlcritical_section(const rtlcritical_section&) = delete;
        rtlcritical_section(rtlcritical_section&&) = delete;
        rtlcritical_section& operator=(const rtlcritical_section&) = delete;
        rtlcritical_section& operator=(rtlcritical_section&&) = delete;

        rtlcritical_section(ULONG spincount = 0) WI_NOEXCEPT
        {
            // Initialization will not fail without invalid params...
            ::RtlInitializeCriticalSectionEx(&m_cs, spincount, 0);
        }

        ~rtlcritical_section() WI_NOEXCEPT
        {
            ::RtlDeleteCriticalSection(&m_cs);
        }

        WI_NODISCARD rtlcs_leave_scope_exit lock() WI_NOEXCEPT
        {
            return wil::RtlEnterCriticalSection(&m_cs);
        }

        WI_NODISCARD rtlcs_leave_scope_exit try_lock() WI_NOEXCEPT
        {
            return wil::RtlTryEnterCriticalSection(&m_cs);
        }

    private:
        RTL_CRITICAL_SECTION m_cs;
    };

#ifdef __WIL_RESOURCE_LOCK_ENFORCEMENT
    namespace details
    {
        template<>
        struct lock_proof_traits<wil::rtlcs_leave_scope_exit> : exclusive_lock_proof {};
    }
#endif // __WIL_RESOURCE_LOCK_ENFORCEMENT

#endif //__WIL_NTURTL_CRITSEC_

#ifndef __WIL_INTERNAL_WINBASE_
#define __WIL_INTERNAL_WINBASE_
#ifndef WIL_HIDE_DEPRECATED_1611
    WIL_WARN_DEPRECATED_1611_PRAGMA(unique_hheap_ptr)

        // deprecated, use unique_process_heap_ptr instead as that has the correct name.
        template <typename T>
    using unique_hheap_ptr = wistd::unique_ptr<T, process_heap_deleter>;
#endif
#endif // __WIL_INTERNAL_WINBASE_
#endif // __WIL_WINBASE_

    // do not add new uses of UNIQUE_STRING_VALUE_EXPERIEMENT, see http://osgvsowi/11252630 for details
#if defined(_WINBASE_) && defined(UNIQUE_STRING_VALUE_EXPERIMENT) && !defined(__WIL_UNIQUE_STRING_VALUE_)
#define __WIL_UNIQUE_STRING_VALUE_
    /** Helper to make use of wil::unique_cotaskmem_string, wil::unique_hlocal_string, wil::unique_process_heap_string
    easier to use. Defaults to using unique_cotaskmem_string.

    wil::unique_string_value<> m_value;

    wil::unique_string_value<wil::unique_hlocal_string> m_localAllocValue;
    */
    template<typename string_type = unique_cotaskmem_string>
    class unique_string_value : public string_type
    {
    public:
        typedef string_type string_type;

        unique_string_value() = default;
        unique_string_value(PWSTR value) : string_type(value)
        {
        }

        unique_string_value(string_type&& other) : string_type(wistd::move(other))
        {
        }

        PCWSTR get_not_null() const
        {
            return get() ? get() : L"";
        }

        HRESULT set_nothrow(_In_opt_ PCWSTR value) WI_NOEXCEPT
        {
            if (value)
            {
                reset(wil::make_unique_string_nothrow<string_type>(value).release());
                return get() ? S_OK : E_OUTOFMEMORY; // log errors at the caller of this method.
            }
            else
            {
                reset();
                return S_OK;
            }
        }

        HRESULT copy_to_nothrow(_Outptr_opt_result_maybenull_ PWSTR* result) WI_NOEXCEPT
        {
            unique_string_value<string_type> temp;
            RETURN_IF_FAILED(temp.set_nothrow(get()));
            *result = temp.release();
            return S_OK;
        }

#ifdef WIL_ENABLE_EXCEPTIONS
        void set(_In_opt_ PCWSTR value)
        {
            THROW_IF_FAILED(set_nothrow(value));
        }
#endif

        unique_string_value<string_type>& operator=(string_type&& other)
        {
            string_type::operator=(wistd::move(other));
            return *this;
        }

        /** returns a pointer to and the size of the buffer. Useful for updating
        the value when the updated value is guaranteed to fit based on the size. */
        PWSTR get_dangerous_writeable_buffer(_Out_ size_t* size)
        {
            if (get())
            {
                *size = wcslen(get()) + 1; // include null
                return const_cast<PWSTR>(get());
            }
            else
            {
                *size = 0;
                return L""; // no space for writing, can only read the null value.
            }
        }
    };

    /// @cond
    namespace details
    {
        template<typename string_type> struct string_allocator<unique_string_value<string_type>>
        {
            static void* allocate(size_t size) WI_NOEXCEPT
            {
                return string_allocator<string_type>::allocate(size);
            }
        };
    }
    /// @endcond

#endif

#if defined(__WIL_WINBASE_DESKTOP) && !defined(__WIL_INTERNAL_WINBASE_DESKTOP)
#define __WIL_INTERNAL_WINBASE_DESKTOP
#ifndef WIL_HIDE_DEPRECATED_1611
    WIL_WARN_DEPRECATED_1611_PRAGMA(unique_security_descriptor)

        // WARNING: 'unique_security_descriptor' has been deprecated and is pending deletion!
        // Use:
        // 1. 'unique_hlocal_security_descriptor' if you need to free using 'LocalFree'.
        // 2. 'unique_private_security_descriptor' if you need to free using 'DestroyPrivateObjectSecurity'.

        // DEPRECATED: use unique_private_security_descriptor
        using unique_security_descriptor = unique_private_security_descriptor;
#endif
#endif // __WIL_INTERNAL_WINBASE_DESKTOP

#if defined(_COM_CLIENT_IDENTITY_HPP_) && defined(__WIL__COMBASEAPI_H_) && !defined(__WIL__COM_CLIENT_IDENTITY_HPP_)
#define __WIL__COM_CLIENT_IDENTITY_HPP_
    //* A type that calls CoRevertToSelf on destruction once it has been explicitly set to active.
    using unique_coreverttoself_call_conditional = unique_call<decltype(&::CoRevertToSelf), ::CoRevertToSelf, false>;

    /** Calls CoImpersonateClientOfObject. If the current call context isn't targeting serverObject, returns without
    actually impersonating. Sets the RAII reverter object to automatically revert if this call impersonated.
    */
    inline HRESULT CoImpersonateClientOfObject_nothrow(IUnknown* serverObject, unique_coreverttoself_call_conditional& reverter) WI_NOEXCEPT
    {
        BOOL foundCallForServer = FALSE;
        RETURN_IF_FAILED(::CoImpersonateClientOfObject(serverObject, &foundCallForServer));
        if (foundCallForServer)
        {
            // Only do the revert if we actually impersonated.
            reverter.activate();
        }
        return S_OK;
    }

    /** Calls CoImpersonateClientOfObject. If the current call context isn't targeting serverObject, returns without
    actually impersonating. Returns an RAII object that reverts if this call impersonated. Fail-fasts on failure.
    */
    WI_NODISCARD inline unique_coreverttoself_call_conditional CoImpersonateClientOfObject_failfast(IUnknown* serverObject) WI_NOEXCEPT
    {
        unique_coreverttoself_call_conditional reverter;
        FAIL_FAST_IF_FAILED(CoImpersonateClientOfObject_nothrow(serverObject, reverter));
        return reverter;
    }

    /** Probes for a call context targeting serverObject. If found, opens the client token provided by COM's call
    channel. If no call context is found, assumes that the call was on-thread, and opens an impersonation token for the
    effective thread token.
    This function follows the CoImpersonateClient semantics of impersonating at the highest level granted by the
    client. That means:
    - If the call has a context, you get the level granted by the client's proxy blanket.
    - If the thread is already impersonating, you get that token.
    - Otherwise, open the process token at the impersonate level.
    */
    inline HRESULT GetImpersonationTokenForClientOfObject_nothrow(IUnknown* serverObject, DWORD desiredAccess,
        unique_handle& clientToken) WI_NOEXCEPT
    {
        unique_coreverttoself_call_conditional reverter;
        RETURN_IF_FAILED(CoImpersonateClientOfObject_nothrow(serverObject, reverter));

        if (!OpenThreadToken(GetCurrentThread(), desiredAccess, TRUE, &clientToken))
        {
            const DWORD result = ::GetLastError();
            if (!reverter && result == ERROR_NO_TOKEN)
            {
                // We didn't find a call context to impersonate, and the thread wasn't already impersonating. Open
                // the process token as the effective client token, and then duplicate the primary token to get an
                // impersonation token.
                unique_process_handle processToken;
                RETURN_IF_WIN32_BOOL_FALSE(OpenProcessToken(GetCurrentProcess(), desiredAccess | TOKEN_DUPLICATE, &processToken));
                RETURN_IF_WIN32_BOOL_FALSE(DuplicateTokenEx(processToken.get(), desiredAccess, nullptr, SecurityImpersonation,
                    TokenImpersonation, &clientToken));
            }
            else
            {
                RETURN_WIN32(result);
            }
        }
        return S_OK;
    }
#endif // __WIL__COM_CLIENT_IDENTITY_HPP_

#if defined(__WIL__COM_CLIENT_IDENTITY_HPP_) && defined(WIL_ENABLE_EXCEPTIONS) && !defined(__WIL__COM_CLIENT_IDENTITY_HPP_EXCEPTIONAL)
#define __WIL__COM_CLIENT_IDENTITY_HPP_EXCEPTIONAL
    /** Calls CoImpersonateClientOfObject. If the current call context isn't targeting serverObject, returns without
    actually impersonating. Returns an RAII object that reverts if this call impersonated. Throws on failure.
    */
    WI_NODISCARD inline unique_coreverttoself_call_conditional CoImpersonateClientOfObject(IUnknown* serverObject)
    {
        unique_coreverttoself_call_conditional reverter;
        THROW_IF_FAILED(CoImpersonateClientOfObject_nothrow(serverObject, reverter));
        return reverter;
    }

    /** Probes for a call context targeting serverObject. If found, returns the client token provided by COM's call
    channel. If no call context is found, assumes that the call was on-thread, and returns an impersonation token for the
    effective thread token.
    This function follows the CoImpersonateClient semantics of impersonating at the highest level granted by the
    client. That means:
    - If the call has a context, you get the level granted by the client's proxy blanket.
    - If the thread is already impersonating, you get that token.
    - Otherwise, open the process token at the impersonate level.
    Throws on failure.
    */
    WI_NODISCARD inline unique_handle GetImpersonationTokenForClientOfObject(IUnknown* serverObject, DWORD desiredAccess = TOKEN_QUERY)
    {
        unique_handle clientToken;
        THROW_IF_FAILED(GetImpersonationTokenForClientOfObject_nothrow(serverObject, desiredAccess, clientToken));
        return clientToken;
    }
#endif // __WIL__COM_CLIENT_IDENTITY_HPP_EXCEPTIONAL

#if defined(__WIL__COM_CLIENT_IDENTITY_HPP_) && defined(__contxt_h__) && !defined(__WIL__COM_CLIENT_IDENTITY_HPP_CONTEXT)
#define __WIL__COM_CLIENT_IDENTITY_HPP_CONTEXT
    /** Probes for a call context targeting serverObject. If found, opens a handle to the call client's process. If no
    call context is found, assumes that the call was inproc and opens a handle to the current process. Note that this API
    should not generally be used for security/identity checks; the GetTokenForClientOfObject APIs should be preferred
    in those cases, because they respect client impersonation.
    */
    inline HRESULT GetProcessHandleForClientOfObject_nothrow(IUnknown* serverObject, DWORD desiredAccess,
        unique_process_handle& processHandle) WI_NOEXCEPT
    {
        // Use a raw interface pointer plus a Release scope_exit, since com_ptr_t is unavailable.
        ICallingProcessInfo* callingProcessInfo = nullptr;
        auto releaseComPtr = wil::scope_exit([&callingProcessInfo]
            {
                if (callingProcessInfo)
                {
                    callingProcessInfo->Release();
                }
            });

        RETURN_IF_FAILED(::CoGetCallContextOfObject(serverObject, IID_PPV_ARGS(&callingProcessInfo)));
        if (callingProcessInfo)
        {
            RETURN_IF_FAILED(callingProcessInfo->OpenCallerProcessHandle(desiredAccess, &processHandle));
        }
        else
        {
            processHandle.reset(GetCurrentProcess());
        }
        return S_OK;
    }
#endif // __WIL__COM_CLIENT_IDENTITY_HPP_CONTEXT

#if defined(__WIL__COM_CLIENT_IDENTITY_HPP_CONTEXT) && defined(WIL_ENABLE_EXCEPTIONS) && !defined(__WIL__COM_CLIENT_IDENTITY_HPP_CONTEXT_EXCEPTIONAL)
#define __WIL__COM_CLIENT_IDENTITY_HPP_CONTEXT_EXCEPTIONAL
    /** Probes for a call context targeting serverObject. If found, returns a handle to the call client's process. If no
    call context is found, assumes that the call was inproc and returns a handle to the current process. Note that this API
    should not generally be used for security/identity checks; the GetTokenForClientOfObject APIs should be preferred
    in those cases, because they respect client impersonation. Throws on failure.
    */
    WI_NODISCARD inline unique_process_handle GetProcessHandleForClientOfObject(IUnknown* serverObject,
        DWORD desiredAccess = PROCESS_QUERY_LIMITED_INFORMATION)
    {
        unique_process_handle clientProcess;
        THROW_IF_FAILED(GetProcessHandleForClientOfObject_nothrow(serverObject, desiredAccess, clientProcess));
        return clientProcess;
    }
#endif // __WIL__COM_CLIENT_IDENTITY_HPP_CONTEXT_EXCEPTIONAL

#if defined(__WIL__COM_CLIENT_IDENTITY_HPP_) && defined(__CAPABILITYAPI_H__) && !defined(__WIL__COM_CLIENT_IDENTITY_HPP_CAPABILITY)
#define __WIL__COM_CLIENT_IDENTITY_HPP_CAPABILITY
    /** Probes for a call context targeting serverObject. If found, performs the capability check against the call's
    client token. If no call context is found, assumes that the call was inproc and performs the capability check
    under the thread's current effective token.
    */
    inline HRESULT CheckCapabilityForClientOfObject_nothrow(IUnknown* serverObject, PCWSTR capabilityId, _Out_ bool* hasCapability) WI_NOEXCEPT
    {
        *hasCapability = false;

        unique_handle clientToken;
        RETURN_IF_FAILED(GetImpersonationTokenForClientOfObject_nothrow(serverObject, TOKEN_QUERY, clientToken));
        BOOLEAN capabilityPresent = FALSE;
        RETURN_IF_NTSTATUS_FAILED(CapabilityCheck(clientToken.get(), capabilityId, &capabilityPresent));
        *hasCapability = (capabilityPresent != FALSE);
        return S_OK;
    }
#endif // __WIL__COM_CLIENT_IDENTITY_HPP_CAPABILITY

#if defined(__WIL__COM_CLIENT_IDENTITY_HPP_CAPABILITY) && defined(WIL_ENABLE_EXCEPTIONS) && !defined(__WIL__COM_CLIENT_IDENTITY_HPP_CAPABILITY_EXCEPTIONAL)
#define __WIL__COM_CLIENT_IDENTITY_HPP_CAPABILITY_EXCEPTIONAL
    /** Probes for a call context targeting serverObject. If found, performs the capability check against the call's
    client token. If no call context is found, assumes that the call was inproc and performs the capability check
    under the thread's current effective token. Throws on failure.
    */
    WI_NODISCARD inline bool CheckCapabilityForClientOfObject(IUnknown* serverObject, PCWSTR capabilityId)
    {
        bool hasCapability = false;
        THROW_IF_FAILED(CheckCapabilityForClientOfObject_nothrow(serverObject, capabilityId, &hasCapability));
        return hasCapability;
    }
#endif // __WIL__COM_CLIENT_IDENTITY_HPP_CAPABILITY_EXCEPTIONAL

#if defined(_CO_DEVICE_CATALOG_) && !defined(__WIL__COM_DEVICE_CATALOG) && !defined(WIL_KERNEL_MODE)
#define __WIL__COM_DEVICE_CATALOG
    //* A type that calls CoRevokeDeviceCatalog on destruction.
    using unique_device_catalog_cookie = unique_any<CO_DEVICE_CATALOG_COOKIE, decltype(&::CoRevokeDeviceCatalog), ::CoRevokeDeviceCatalog>;
#endif //__WIL__COM_DEVICE_CATALOG

#if defined(__WIL__COM_DEVICE_CATALOG) && !defined(__WIL__COM_DEVICE_CATALOG__STL) && defined(WIL_RESOURCE_STL)
#define __WIL__COM_DEVICE_CATALOG__STL
    typedef shared_any<unique_device_catalog_cookie> shared_device_catalog_cookie;
    typedef weak_any<shared_device_catalog_cookie> weak_device_catalog_cookie;
#endif // __WIL__COM_DEVICE_CATALOG__STL

#if defined(_PACKAGEIDENTITY_H_) && !defined(__WIL_PACKAGEIDENTITY_H_)
#define __WIL_PACKAGEIDENTITY_H_
    template<typename T>
    using unique_appxmem_ptr = wistd::unique_ptr<T, function_deleter<decltype(&AppXFreeMemory), AppXFreeMemory>>;
#endif // __WIL_PACKAGEIDENTITY_H_

#if defined(_DSREGAPI_) && !defined(__WIL_DSREGAPI__) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define __WIL_DSREGAPI__
    typedef unique_any<PDSREG_JOIN_INFO, decltype(&::DsrFreeJoinInfo), ::DsrFreeJoinInfo> unique_join_info;
    typedef unique_any<PCXH_SCENARIO_INFO, decltype(&::DsrFreeCxhScenarioInfo), ::DsrFreeCxhScenarioInfo> unique_scenario_info;
#endif // __WIL_DSREGAPI__
#if defined(__WIL_DSREGAPI__) && !defined(__WIL_DSREGAPI_STL) && defined(WIL_RESOURCE_STL)
#define __WIL_DSREGAPI_STL
    typedef shared_any<unique_join_info> shared_join_info;
    typedef shared_any<unique_scenario_info> shared_scenario_info;

    typedef weak_any<shared_join_info> weak_join_info;
    typedef weak_any<shared_scenario_info> weak_scenario_info;
#endif // __WIL_DSREGAPI_STL

#if (defined(__SECURITY_RUNTIME_H__) || defined(__ACCESSHLPR_H__)) && !defined(__WIL__SECURITY_RUNTIME_H__)
#define __WIL__SECURITY_RUNTIME_H__
    typedef unique_any<PSECURITY_DESCRIPTOR, decltype(&::FreeTransientObjectSecurityDescriptor), ::FreeTransientObjectSecurityDescriptor> unique_transient_security_descriptor;
#endif // __WIL__SECURITY_RUNTIME_H__
#if defined(__WIL__SECURITY_RUNTIME_H__) && !defined(__WIL__SECURITY_RUNTIME_H__STL) && defined(WIL_RESOURCE_STL)
#define __WIL__SECURITY_RUNTIME_H__STL
    typedef shared_any<unique_transient_security_descriptor> shared_transient_security_descriptor;
    typedef weak_any<shared_transient_security_descriptor> weak_transient_security_descriptor;
#endif // __WIL__SECURITY_RUNTIME_H__STL

#if defined(_VAULTCLIEXT_H_) && !defined(__WIL_VAULTCLEXT_H__) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define __WIL_VAULTCLEXT_H__
    /// @cond
    namespace details
    {
        inline void __stdcall VaultCloseVault(_Pre_opt_valid_ _Frees_ptr_opt_ VAULT_HANDLE hVault) WI_NOEXCEPT
        {
#pragma prefast(suppress:6031, "annotation issue: VaultCloseVault is annotated __checkreturn, but a failure is not actionable to us");
            ::VaultCloseVault(&hVault);
        }
    }
    /// @endcond

    typedef unique_any<VAULT_HANDLE, decltype(&details::VaultCloseVault), details::VaultCloseVault> unique_vaulthandle;

    typedef unique_any<PVAULT_ITEM, decltype(&::VaultFree), ::VaultFree> unique_vaultitem;
#endif // __WIL_VAULTCLEXT_H__
#if defined(__WIL_VAULTCLEXT_H__) && !defined(__WIL_VAULTCLEXT_H__STL) && defined(WIL_RESOURCE_STL)
#define __WIL_VAULTCLEXT_H__
    typedef shared_any<unique_vaulthandle> shared_vaulthandle;
    typedef weak_any<unique_vaulthandle> weak_vaulthandle;

    typedef shared_any<unique_vaultitem> shared_vaultitem;
    typedef weak_any<unique_vaultitem> weak_vaultitem;
#endif // __WIL_VAULTCLEXT_H__

#if defined(_INC_DEVOBJ) && !defined(__WIL_INC_DEVOBJ_) && defined(__WIL_WINBASE_)
#define __WIL_INC_DEVOBJ_
    typedef unique_any_handle_invalid<decltype(&::DevObjDestroyDeviceInfoList), ::DevObjDestroyDeviceInfoList> unique_hdolist;
#endif // __WIL_INC_DEVOBJ_
#if defined(__WIL_INC_DEVOBJ_) && !defined(__WIL_INC_DEVOBJ_STL) && defined(WIL_RESOURCE_STL)
#define __WIL_INC_DEVOBJ_STL
    typedef shared_any<unique_hdolist> shared_hdolist;
    typedef weak_any<shared_hdolist> weak_hdolist;
#endif // __WIL_INC_DEVOBJ_STL

#if (defined(_NTURTL_) || defined(_WNF_APIS_DEFINED_)) && defined(__NOTHROW_T_DEFINED) && !defined(__WIL_NTURTL_WNF_) && !defined(__WIL_MIN_KERNEL) && !defined(WIL_KERNEL_MODE)
#define __WIL_NTURTL_WNF_
    /** Windows Notification Facility (WNF) helpers that make it easy to produce and consume wnf state blocks.
    Learn more about WNF here: http://windowsarchive/sites/windows8docs/Win8%20Feature%20Docs/Windows%20Core%20(CORE)/Kernel%20Platform%20Group%20(KPG)/Kernel%20Core%20Software%20Services/Communications/Windows%20Notification%20Facility%20(WNF)/WNF%20-%20Functional%20Specification%20BETA.docm

    Clients must include <new> or <new.h> to enable use of these helpers as they use new(std::nothrow).
    This is to avoid the dependency on those headers that some clients can't tolerate.
    Clients must also include <nt.h>, <ntrtl.h>, and <nturtl.h> before any other windows-related headers such as <windows.h>.

    NOTE: The subscription wrappers will wait for outstanding callbacks on destruction

    Example use of throwing version:
    ~~~
    auto stateChangeSubscription = wil::make_wnf_subscription<TYPE>([](const TYPE& type)
    {
    // Do stuff with type
    });
    ~~~
    Example use of non-throwing version:
    ~~~
    auto stateChangeSubscription = wil::make_wnf_subscription_nothrow<TYPE>([](const TYPE& type)
    {
    // Do stuff with type
    });
    RETURN_HR_IF_FALSE(E_OUTOFMEMORY, stateChangeSubscription);
    ~~~
    */

    //! WNF_CHANGE_STAMP is an alias for ULONG that causes callers that don't specify changeStamp to select the zero size query with change stamp.
    //! To avoid this use WNF_CHANGE_STAMP_STRUCT instead of WNF_CHANGE_STAMP for this parameter.
    struct WNF_CHANGE_STAMP_STRUCT
    {
        WNF_CHANGE_STAMP value = 0;
        operator WNF_CHANGE_STAMP /* ULONG */ () const { return value; }
        static void AssignToOptParam(_In_opt_ WNF_CHANGE_STAMP_STRUCT* changeStampStruct, WNF_CHANGE_STAMP value)
        {
            if (changeStampStruct)
            {
                changeStampStruct->value = value;
            }
        }
    };

    //! Checks the published state and optionally the change stamp for state blocks of any size.
    inline HRESULT wnf_is_published_nothrow(WNF_STATE_NAME const& stateName, _Out_ bool* isPublished, _Out_opt_ WNF_CHANGE_STAMP_STRUCT* changeStamp = nullptr) WI_NOEXCEPT
    {
        *isPublished = false;
        WNF_CHANGE_STAMP_STRUCT::AssignToOptParam(changeStamp, static_cast<WNF_CHANGE_STAMP>(0));
        ULONG stateDataSize = 0;
        WNF_CHANGE_STAMP tempChangeStamp;
        HRESULT queryResult = HRESULT_FROM_NT(NtQueryWnfStateData(&stateName, nullptr, nullptr, &tempChangeStamp, nullptr, &stateDataSize));
        RETURN_HR_IF(queryResult, FAILED(queryResult) && (queryResult != HRESULT_FROM_NT(STATUS_BUFFER_TOO_SMALL)));
        *isPublished = (tempChangeStamp != 0);
        WNF_CHANGE_STAMP_STRUCT::AssignToOptParam(changeStamp, tempChangeStamp);
        return S_OK;
    }

    //! Checks the published state and optionally the change stamp for state blocks of any size, with an explicit scope.
    inline HRESULT wnf_scoped_is_published_nothrow(WNF_STATE_NAME const& stateName, _Out_ bool* isPublished, _In_ void* explicitScope, _Out_opt_ WNF_CHANGE_STAMP_STRUCT* changeStamp = nullptr) WI_NOEXCEPT
    {
        *isPublished = false;
        WNF_CHANGE_STAMP_STRUCT::AssignToOptParam(changeStamp, static_cast<WNF_CHANGE_STAMP>(0));
        ULONG stateDataSize = 0;
        WNF_CHANGE_STAMP tempChangeStamp;
        HRESULT queryResult = HRESULT_FROM_NT(NtQueryWnfStateData(&stateName, nullptr, explicitScope, &tempChangeStamp, nullptr, &stateDataSize));
        RETURN_HR_IF(queryResult, FAILED(queryResult) && (queryResult != HRESULT_FROM_NT(STATUS_BUFFER_TOO_SMALL)));
        *isPublished = (tempChangeStamp != 0);
        WNF_CHANGE_STAMP_STRUCT::AssignToOptParam(changeStamp, tempChangeStamp);
        return S_OK;
    }

    //! Query the published state and optionally change stamp for zero sized state blocks. This will fail fast for not-zero sized state blocks.
    inline HRESULT wnf_query_nothrow(WNF_STATE_NAME const& stateName, _Out_ bool* isPublished, _Out_opt_ WNF_CHANGE_STAMP_STRUCT* changeStamp = nullptr) WI_NOEXCEPT
    {
        *isPublished = false;
        WNF_CHANGE_STAMP_STRUCT::AssignToOptParam(changeStamp, static_cast<WNF_CHANGE_STAMP>(0));
        ULONG stateDataSize = 0;
        WNF_CHANGE_STAMP tempChangeStamp;
        HRESULT queryResult = HRESULT_FROM_NT(NtQueryWnfStateData(&stateName, nullptr, nullptr, &tempChangeStamp, nullptr, &stateDataSize));
        RETURN_HR_IF(queryResult, FAILED(queryResult) && (queryResult != HRESULT_FROM_NT(STATUS_BUFFER_TOO_SMALL)));
        __FAIL_FAST_ASSERT__((tempChangeStamp == 0) || (stateDataSize == 0));
        LOG_HR_IF_MSG(E_UNEXPECTED, (tempChangeStamp != 0) && (stateDataSize != 0), "Inconsistent state data size in wnf_query");
        *isPublished = (tempChangeStamp != 0) && (stateDataSize == 0);  // if the size is non zero, consider it unpublished
        WNF_CHANGE_STAMP_STRUCT::AssignToOptParam(changeStamp, tempChangeStamp);
        return S_OK;
    }

    //! Query the published state and optionally change stamp for zero sized state blocks, with an explicit scope. This will fail fast for not-zero sized state blocks.
    inline HRESULT wnf_scoped_query_nothrow(WNF_STATE_NAME const& stateName, _Out_ bool* isPublished, _In_ void* explicitScope, _Out_opt_ WNF_CHANGE_STAMP_STRUCT* changeStamp = nullptr) WI_NOEXCEPT
    {
        *isPublished = false;
        WNF_CHANGE_STAMP_STRUCT::AssignToOptParam(changeStamp, static_cast<WNF_CHANGE_STAMP>(0));
        ULONG stateDataSize = 0;
        WNF_CHANGE_STAMP tempChangeStamp;
        HRESULT queryResult = HRESULT_FROM_NT(NtQueryWnfStateData(&stateName, nullptr, explicitScope, &tempChangeStamp, nullptr, &stateDataSize));
        RETURN_HR_IF(queryResult, FAILED(queryResult) && (queryResult != HRESULT_FROM_NT(STATUS_BUFFER_TOO_SMALL)));
        __FAIL_FAST_ASSERT__((tempChangeStamp == 0) || (stateDataSize == 0));
        LOG_HR_IF_MSG(E_UNEXPECTED, (tempChangeStamp != 0) && (stateDataSize != 0), "Inconsistent state data size in wnf_query");
        *isPublished = (tempChangeStamp != 0) && (stateDataSize == 0);  // if the size is non zero, consider it unpublished
        WNF_CHANGE_STAMP_STRUCT::AssignToOptParam(changeStamp, tempChangeStamp);
        return S_OK;
    }

    //! Query the published state, state data and optionally the change stamp for fixed size state blocks.
    template <typename state_data_t>
    HRESULT wnf_query_nothrow(WNF_STATE_NAME const& stateName, _Out_ bool* isPublished,
        _Pre_ _Notnull_ _Pre_ _Writable_elements_(1) _Post_ _When_((*isPublished == true), _Valid_) state_data_t* stateData, _Out_opt_ WNF_CHANGE_STAMP_STRUCT* changeStamp = nullptr) WI_NOEXCEPT
    {
        *isPublished = false;
        WNF_CHANGE_STAMP_STRUCT::AssignToOptParam(changeStamp, static_cast<WNF_CHANGE_STAMP>(0));
        ULONG stateDataSize = sizeof(*stateData);
        WNF_CHANGE_STAMP tempChangeStamp;
        HRESULT queryResult = HRESULT_FROM_NT(NtQueryWnfStateData(&stateName, nullptr, nullptr, &tempChangeStamp, stateData, &stateDataSize));
        RETURN_HR_IF(queryResult, FAILED(queryResult) && (queryResult != HRESULT_FROM_NT(STATUS_BUFFER_TOO_SMALL)));
        __FAIL_FAST_ASSERT__((tempChangeStamp == 0) || (stateDataSize == sizeof(*stateData)));
        LOG_HR_IF_MSG(E_UNEXPECTED, (tempChangeStamp != 0) && (stateDataSize != sizeof(*stateData)), "Inconsistent state data size in wnf_query");
        *isPublished = (tempChangeStamp != 0) && (stateDataSize == sizeof(*stateData));  // if the size is mismatched, consider it unpublished
        WNF_CHANGE_STAMP_STRUCT::AssignToOptParam(changeStamp, tempChangeStamp);
        return S_OK;
    }

    //! Query the published state, state data and optionally the change stamp for fixed size state blocks, with an explicit scope.
    template <typename state_data_t>
    HRESULT wnf_scoped_query_nothrow(WNF_STATE_NAME const& stateName, _Out_ bool* isPublished, _In_ void* explicitScope,
        _Pre_ _Notnull_ _Pre_ _Writable_elements_(1) _Post_ _When_((*isPublished == true), _Valid_) state_data_t* stateData, _Out_opt_ WNF_CHANGE_STAMP_STRUCT* changeStamp = nullptr) WI_NOEXCEPT
    {
        *isPublished = false;
        WNF_CHANGE_STAMP_STRUCT::AssignToOptParam(changeStamp, static_cast<WNF_CHANGE_STAMP>(0));
        ULONG stateDataSize = sizeof(*stateData);
        WNF_CHANGE_STAMP tempChangeStamp;
        HRESULT queryResult = HRESULT_FROM_NT(NtQueryWnfStateData(&stateName, nullptr, explicitScope, &tempChangeStamp, stateData, &stateDataSize));
        RETURN_HR_IF(queryResult, FAILED(queryResult) && (queryResult != HRESULT_FROM_NT(STATUS_BUFFER_TOO_SMALL)));
        __FAIL_FAST_ASSERT__((tempChangeStamp == 0) || (stateDataSize == sizeof(*stateData)));
        LOG_HR_IF_MSG(E_UNEXPECTED, (tempChangeStamp != 0) && (stateDataSize != sizeof(*stateData)), "Inconsistent state data size in wnf_query");
        *isPublished = (tempChangeStamp != 0) && (stateDataSize == sizeof(*stateData));  // if the size is mismatched, consider it unpublished
        WNF_CHANGE_STAMP_STRUCT::AssignToOptParam(changeStamp, tempChangeStamp);
        return S_OK;
    }

    //! Query the published state, state data and optionally the change stamp for variable size state blocks.
    inline HRESULT wnf_query_nothrow(WNF_STATE_NAME const& stateName, _Out_ bool* isPublished,
        _Out_writes_bytes_(stateDataByteCount) void* stateData, size_t stateDataByteCount,
        _Out_ size_t* stateDataWrittenByteCount, _Out_opt_ WNF_CHANGE_STAMP_STRUCT* changeStamp = nullptr) WI_NOEXCEPT
    {
        *isPublished = false;
        *stateDataWrittenByteCount = 0;
        WNF_CHANGE_STAMP_STRUCT::AssignToOptParam(changeStamp, static_cast<WNF_CHANGE_STAMP>(0));

        WNF_CHANGE_STAMP tempChangeStamp;
        ULONG size = static_cast<ULONG>(stateDataByteCount);
        RETURN_IF_NTSTATUS_FAILED(NtQueryWnfStateData(&stateName, nullptr, nullptr, &tempChangeStamp, stateData, &size));
        *isPublished = (tempChangeStamp != 0);
        *stateDataWrittenByteCount = static_cast<size_t>(size);
        WNF_CHANGE_STAMP_STRUCT::AssignToOptParam(changeStamp, tempChangeStamp);
        return S_OK;
    }

    //! Query the published state, state data and optionally the change stamp for variable size state blocks, with an explicit scope.
    inline HRESULT wnf_scoped_query_nothrow(WNF_STATE_NAME const& stateName, _Out_ bool* isPublished, _In_ void* explicitScope,
        _Out_writes_bytes_(stateDataByteCount) void* stateData, size_t stateDataByteCount,
        _Out_ size_t* stateDataWrittenByteCount, _Out_opt_ WNF_CHANGE_STAMP_STRUCT* changeStamp = nullptr) WI_NOEXCEPT
    {
        *isPublished = false;
        *stateDataWrittenByteCount = 0;
        WNF_CHANGE_STAMP_STRUCT::AssignToOptParam(changeStamp, static_cast<WNF_CHANGE_STAMP>(0));
        WNF_CHANGE_STAMP tempChangeStamp;
        ULONG size = static_cast<ULONG>(stateDataByteCount);
        RETURN_IF_NTSTATUS_FAILED(NtQueryWnfStateData(&stateName, nullptr, explicitScope, &tempChangeStamp, stateData, &size));
        *isPublished = (tempChangeStamp != 0);
        *stateDataWrittenByteCount = static_cast<size_t>(size);
        WNF_CHANGE_STAMP_STRUCT::AssignToOptParam(changeStamp, tempChangeStamp);
        return S_OK;
    }

    //! Returns true if the value is published, false if not (or malformed or on error).
    inline _Success_(return) bool try_wnf_query(WNF_STATE_NAME const& stateName, _Out_opt_ WNF_CHANGE_STAMP_STRUCT* changeStamp = nullptr) WI_NOEXCEPT
    {
        bool isPublished;
        return SUCCEEDED(wnf_query_nothrow(stateName, &isPublished, changeStamp)) && isPublished;
    }

    //! Returns true if the value is published, false if not (or malformed or on error)
    //! and returns the fixed size state data.
    template <typename state_data_t>
    _Success_(return) bool try_wnf_query(WNF_STATE_NAME const& stateName, _Out_ state_data_t* stateData, _Out_opt_ WNF_CHANGE_STAMP_STRUCT* changeStamp = nullptr) WI_NOEXCEPT
    {
        bool isPublished;
        return SUCCEEDED(wnf_query_nothrow(stateName, &isPublished, stateData, changeStamp)) && isPublished;
    }

    //! Publish a zero sized state block.
    inline HRESULT wnf_publish_nothrow(WNF_STATE_NAME const& stateName) WI_NOEXCEPT
    {
        return HRESULT_FROM_NT(RtlPublishWnfStateData(stateName, nullptr, nullptr, 0, nullptr));
    }

    //! Publish a zero sized state block, with an explicit scope.
    inline HRESULT wnf_scoped_publish_nothrow(WNF_STATE_NAME const& stateName, _In_ void* explicitScope) WI_NOEXCEPT
    {
        return HRESULT_FROM_NT(RtlPublishWnfStateData(stateName, nullptr, nullptr, 0, explicitScope));
    }

    //! Publish a variable sized state block.
    inline HRESULT wnf_publish_nothrow(WNF_STATE_NAME const& stateName, _In_reads_bytes_(size) const void* stateData, const size_t size) WI_NOEXCEPT
    {
        return HRESULT_FROM_NT(RtlPublishWnfStateData(stateName, nullptr, stateData, static_cast<const ULONG>(size), nullptr));
    }

    //! Publish a variable sized state block, with an explicit scope.
    inline HRESULT wnf_scoped_publish_nothrow(WNF_STATE_NAME const& stateName, _In_reads_bytes_(size) const void* stateData, const size_t size, _In_ void* explicitScope) WI_NOEXCEPT
    {
        return HRESULT_FROM_NT(RtlPublishWnfStateData(stateName, nullptr, stateData, static_cast<const ULONG>(size), explicitScope));
    }

    //! Publish a fixed sized state block.
    template <typename state_data_t>
    HRESULT wnf_publish_nothrow(WNF_STATE_NAME const& stateName, state_data_t const& stateData) WI_NOEXCEPT
    {
        return HRESULT_FROM_NT(RtlPublishWnfStateData(stateName, nullptr, &stateData, sizeof(stateData), nullptr));
    }

    //! Publish a fixed sized state block, with an explicit scope.
    template <typename state_data_t>
    HRESULT wnf_scoped_publish_nothrow(WNF_STATE_NAME const& stateName, state_data_t const& stateData, _In_ void* explicitScope) WI_NOEXCEPT
    {
        return HRESULT_FROM_NT(RtlPublishWnfStateData(stateName, nullptr, &stateData, sizeof(stateData), explicitScope));
    }

#ifdef WIL_ENABLE_EXCEPTIONS
    //! Checks the published state and optionally the change stamp for state blocks of any size.
    inline _Success_(return) bool wnf_is_published(WNF_STATE_NAME const& stateName, _Out_opt_ WNF_CHANGE_STAMP_STRUCT* changeStamp = nullptr)
    {
        bool isPublished;
        THROW_IF_FAILED(wnf_is_published_nothrow(stateName, &isPublished, changeStamp));
        return isPublished;
    }

    //! Query the published state, state data and optionally the change stamp for fixed size state blocks.
    template <typename state_data_t>
    _Success_(return) bool wnf_query(WNF_STATE_NAME const& stateName, _Out_ state_data_t* stateData, _Out_opt_ WNF_CHANGE_STAMP_STRUCT* changeStamp = nullptr)
    {
        bool isPublished;
        THROW_IF_FAILED(wnf_query_nothrow(stateName, &isPublished, stateData, changeStamp));
        return isPublished;
    }

    //! Query the published state and optioanlly change stamp for zero sized state blocks. This will fail fast for not-zero sized state blocks.
    inline _Success_(return) bool wnf_query(WNF_STATE_NAME const& stateName, _Out_opt_ WNF_CHANGE_STAMP_STRUCT* changeStamp = nullptr)
    {
        bool isPublished;
        THROW_IF_FAILED(wnf_query_nothrow(stateName, &isPublished, changeStamp));
        return isPublished;
    }

    //! Publish a zero-size state block.
    inline void wnf_publish(WNF_STATE_NAME const& stateName)
    {
        THROW_IF_FAILED(wnf_publish_nothrow(stateName));
    }

    //! Publish a fixed size state block.
    template <typename state_data_t>
    void wnf_publish(WNF_STATE_NAME const& stateName, state_data_t const& stateData)
    {
        THROW_IF_FAILED(wnf_publish_nothrow(stateName, stateData));
    }

#endif // WIL_ENABLE_EXCEPTIONS

    //! Manage the lifetime of a user subscription.
    typedef unique_any<PWNF_USER_SUBSCRIPTION, decltype(&::RtlUnsubscribeWnfNotificationWaitForCompletion), ::RtlUnsubscribeWnfNotificationWaitForCompletion, details::pointer_access_all> unique_pwnf_user_subscription;
    WNF_CHANGE_STAMP const WnfChangeStampLatest = MAXULONG;

    /// @cond
    namespace details
    {
        struct empty_wnf_state
        {
        };

        template <typename state_data_t>
        struct wnf_array_callback_type
        {
            typedef typename wistd::function<void(state_data_t const*, size_t)> type;
        };

        template <typename state_data_t>
        struct wnf_callback_type
        {
            typedef typename wistd::function<void(state_data_t const&)> type;
        };

        template <>
        struct wnf_callback_type<empty_wnf_state>
        {
            typedef wistd::function<void()> type;
        };

        struct wnf_subscription_state_base
        {
            virtual ~wnf_subscription_state_base() {};
            unique_pwnf_user_subscription m_subscription;
        };

        template <typename state_data_t = empty_wnf_state>
        struct wnf_subscription_state : public wnf_subscription_state_base
        {
            typedef typename details::wnf_callback_type<state_data_t>::type callback_type;

            wnf_subscription_state(callback_type&& callback) : m_callback(wistd::move(callback)) {}

            ~wnf_subscription_state() override
            {
                m_subscription.reset();     // subscription must be released prior to the callback (ordering)
            }

            template <typename state_data_t>
            void InternalCallback(state_data_t const* stateData, ULONG stateDataSize)
            {
                // Must check the size here vs. in the subscription lambda in make_wnf_subscription_state since the default
                // case uses empty_wnf_state which has a size > 0
                __FAIL_FAST_ASSERT__(stateDataSize == sizeof(*stateData));
                if (stateDataSize != sizeof(*stateData))
                {
                    LOG_HR_MSG(E_UNEXPECTED, "Inconsistent state data size in WNF callback");
                    return;
                }
                m_callback(*stateData);
            }

            template <>
            void InternalCallback<empty_wnf_state>(empty_wnf_state const* /*stateData*/, ULONG /*stateDataSize*/)
            {
                m_callback();
            }

            callback_type m_callback;
        };

        template <typename state_data_t = empty_wnf_state>
        HRESULT make_wnf_subscription_state(WNF_STATE_NAME const& stateName, typename details::wnf_callback_type<state_data_t>::type&& callback, _In_ WNF_CHANGE_STAMP subscribeFrom, _Outptr_ wnf_subscription_state<state_data_t>** subscriptionState) WI_NOEXCEPT
        {
            *subscriptionState = nullptr;
            wistd::unique_ptr<wnf_subscription_state<state_data_t>> subscriptionStateT(new(std::nothrow) wnf_subscription_state<state_data_t>(wistd::move(callback)));
            RETURN_IF_NULL_ALLOC(subscriptionStateT.get());

            if (subscribeFrom == WnfChangeStampLatest)
            {
                // Retrieve the latest changestamp and use that as the basis for change notifications
                ULONG bufferSize = 0;
                HRESULT queryResult = HRESULT_FROM_NT(NtQueryWnfStateData(&stateName, 0, nullptr, &subscribeFrom, nullptr, &bufferSize));
                RETURN_HR_IF(queryResult, FAILED(queryResult) && (queryResult != HRESULT_FROM_NT(STATUS_BUFFER_TOO_SMALL)));
            }

            const auto result = RtlSubscribeWnfStateChangeNotification(wil::out_param(subscriptionStateT->m_subscription), stateName, subscribeFrom,
                [](WNF_STATE_NAME, WNF_CHANGE_STAMP changeStamp, WNF_TYPE_ID*, void* callbackContext, const void* buffer, ULONG length) -> NTSTATUS
                {
                    // Delete notifications always have a change stamp of 0.  Since this class doesn't
                    // provide for a way to handle them differently, it drops them to avoid a fast-fail
                    // due to the payload size of a delete being potentially different than an update.
                    if (changeStamp != 0)
                    {
                        static_cast<details::wnf_subscription_state<state_data_t>*>(callbackContext)->InternalCallback(static_cast<state_data_t const*>(buffer), length);
                    }
                    return STATUS_SUCCESS;
                }, subscriptionStateT.get(), nullptr, 0, 0);
            RETURN_IF_NTSTATUS_FAILED(result);

            *subscriptionState = subscriptionStateT.release();
            return S_OK;
        }

        template <typename state_data_t>
        struct wnf_array_subscription_state : public wnf_subscription_state_base
        {
            typedef typename details::wnf_array_callback_type<state_data_t>::type callback_type;

            wnf_array_subscription_state(callback_type&& callback) : m_callback(wistd::move(callback))
            {
            }

            ~wnf_array_subscription_state() override
            {
                m_subscription.reset();     // subscription must be released prior to the callback (ordering)
            }

            template <typename state_data_t>
            void InternalCallback(state_data_t const* stateData, ULONG stateDataSize)
            {
                if ((stateDataSize % sizeof(state_data_t)) != 0)
                {
                    LOG_HR_MSG(E_UNEXPECTED, "Inconsistent state data size in WNF callback");
                    return;
                }

                // for empty payload, return zero sized null array.
                size_t itemSize = static_cast<size_t>(stateDataSize / sizeof(state_data_t));
                m_callback(stateData, itemSize);
            }

            callback_type m_callback;
        };

        template <typename state_data_t>
        HRESULT make_wnf_array_subscription_state(
            WNF_STATE_NAME const& stateName,
            typename details::wnf_array_callback_type<state_data_t>::type&& callback,
            _In_ WNF_CHANGE_STAMP subscribeFrom,
            _Outptr_ wnf_array_subscription_state<state_data_t>** subscriptionState) WI_NOEXCEPT
        {
            *subscriptionState = nullptr;
            wistd::unique_ptr<wnf_array_subscription_state<state_data_t>> subscriptionStateT(new(std::nothrow) wnf_array_subscription_state<state_data_t>(wistd::move(callback)));
            RETURN_IF_NULL_ALLOC(subscriptionStateT.get());

            if (subscribeFrom == WnfChangeStampLatest)
            {
                // Retrieve the latest changestamp and use that as the basis for change notifications
                ULONG bufferSize = 0;
                const HRESULT queryResult = HRESULT_FROM_NT(NtQueryWnfStateData(&stateName, 0, nullptr, &subscribeFrom, nullptr, &bufferSize));
                RETURN_HR_IF(queryResult, FAILED(queryResult) && (queryResult != HRESULT_FROM_NT(STATUS_BUFFER_TOO_SMALL)));
            }

            RETURN_IF_FAILED(HRESULT_FROM_NT(RtlSubscribeWnfStateChangeNotification(wil::out_param(subscriptionStateT->m_subscription), stateName, subscribeFrom,
                [](WNF_STATE_NAME /*stateName*/, WNF_CHANGE_STAMP /*changeStamp*/,
                    WNF_TYPE_ID* /*typeID*/, void* callbackContext, const void* buffer, ULONG length) -> NTSTATUS
                {
                    static_cast<details::wnf_array_subscription_state<state_data_t>*>(callbackContext)->InternalCallback(static_cast<state_data_t const*>(buffer), length);
                    return STATUS_SUCCESS;
                }, subscriptionStateT.get(), nullptr, 0, 0)));

            *subscriptionState = subscriptionStateT.release();
            return S_OK;
        }

        inline void delete_wnf_subscription_state(_In_opt_ wnf_subscription_state_base* subscriptionState) { delete subscriptionState; }
    }
    /// @endcond

    //! Manage the lifetime of a subscription.
    typedef unique_any<details::wnf_subscription_state_base*, decltype(&details::delete_wnf_subscription_state), details::delete_wnf_subscription_state, details::pointer_access_none> unique_wnf_subscription;

    //! Subscribe to changes for a zero or fixed size wnf state block.
    //! the callback will only be called if the payload size matches the size of the state block.
    template<typename state_data_t = details::empty_wnf_state>
    unique_wnf_subscription make_wnf_subscription_nothrow(WNF_STATE_NAME const& stateName, typename details::wnf_callback_type<state_data_t>::type&& callback, _In_ WNF_CHANGE_STAMP subscribeFrom = 0) WI_NOEXCEPT
    {
        details::wnf_subscription_state<state_data_t>* subscriptionState;
        if (SUCCEEDED(details::make_wnf_subscription_state<state_data_t>(stateName, wistd::move(callback), subscribeFrom, &subscriptionState)))
        {
            return unique_wnf_subscription(subscriptionState);
        }
        return nullptr;
    }

    //! Subscribe to changes for a dynamic size wnf state block that is an array of elements each of size state_data_t
    //! the callback will only be called with the pointer to the first item of the array and the number of items in the array.
    //!
    //! auto varArraySubscription = wil::make_wnf_array_subscription_nothrow<wchar_t>(
    //!     WNF_FDBK_QUESTION_NOTIFICATION,
    //!     [&](wchar_t const* array, size_t length)
    //!     {
    //!     });
    template<typename state_data_t>
    unique_wnf_subscription make_wnf_array_subscription_nothrow(
        WNF_STATE_NAME const& stateName,
        typename details::wnf_array_callback_type<state_data_t>::type&& callback,
        _In_ WNF_CHANGE_STAMP subscribeFrom = 0) WI_NOEXCEPT
    {
        details::wnf_array_subscription_state<state_data_t>* subscriptionState;
        if (SUCCEEDED(details::make_wnf_array_subscription_state<state_data_t>(stateName, wistd::move(callback), subscribeFrom, &subscriptionState)))
        {
            return unique_wnf_subscription(subscriptionState);
        }
        return nullptr;
    }
#ifdef WIL_ENABLE_EXCEPTIONS

    //! Subscribe to changes for a zero or fixed size wnf state block.
    //! the callback will only be called if the payload size matches the size of the state block.
    template<typename state_data_t = details::empty_wnf_state>
    unique_wnf_subscription make_wnf_subscription(WNF_STATE_NAME const& stateName, typename details::wnf_callback_type<state_data_t>::type&& callback, _In_ WNF_CHANGE_STAMP subscribeFrom = 0)
    {
        details::wnf_subscription_state<state_data_t>* subscriptionState;
        THROW_IF_FAILED(details::make_wnf_subscription_state<state_data_t>(stateName, wistd::move(callback), subscribeFrom, &subscriptionState));
        return unique_wnf_subscription(subscriptionState);
    }

    //! Subscribe to changes for a dynamic size wnf state block that is an array of elements each of size state_data_t
    //! the callback will only be called if the payload size has at least one item in the array.
    //!
    //! auto varArraySubscription = wil::make_wnf_array_subscription<wchar_t>(
    //!     WNF_FDBK_QUESTION_NOTIFICATION,
    //!     [&](wchar_t const* array, size_t length)
    //!     {
    //!     });
    template<typename state_data_t>
    unique_wnf_subscription make_wnf_array_subscription(
        WNF_STATE_NAME const& stateName,
        typename details::wnf_array_callback_type<state_data_t>::type&& callback,
        _In_ WNF_CHANGE_STAMP subscribeFrom = 0)
    {
        details::wnf_array_subscription_state<state_data_t>* subscriptionState;
        THROW_IF_FAILED(details::make_wnf_array_subscription_state<state_data_t>(stateName, wistd::move(callback), subscribeFrom, &subscriptionState));
        return unique_wnf_subscription(subscriptionState);
    }

#endif // WIL_ENABLE_EXCEPTIONS

#endif // __WIL_NTURTL_WNF_

#if defined(__SRUMAPI_H__) && !defined(__WIL_SRUMAPI_H__)
#define __WIL_SRUMAPI_H__
    typedef unique_any<PSRU_STATS_RECORD_SET, decltype(&::SruFreeRecordSet), ::SruFreeRecordSet> unique_srum_recordset;
#endif
#if defined(__WIL_SRUMAPI_H__) && !defined(__WIL_SRUMAPI_H_STL) && defined(WIL_RESOURCE_STL)
#define __WIL_SRUMAPI_H_STL
    typedef shared_any<unique_srum_recordset> shared_srum_recordset;
    typedef weak_any<shared_srum_recordset> weak_srum_recordset;
#endif

#if defined(_DUSMAPI_H) && !defined(__WIL_DUSMAPI_H_)
#define __WIL_DUSMAPI_H_
    using dusm_deleter = function_deleter<decltype(&::DusmFree), DusmFree>;

    template<typename T>
    using unique_dusm_ptr = wistd::unique_ptr<T, dusm_deleter>;
#endif

#if defined(__WWAN_API_DECL__) && !defined(__WIL_WWAN_API_DECL__) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define __WIL_WWAN_API_DECL__
    using wwan_deleter = function_deleter<decltype(&::WwanFreeMemory), WwanFreeMemory>;

    template<typename T>
    using unique_wwan_ptr = wistd::unique_ptr<T, wwan_deleter>;

    /// @cond
    namespace details
    {
        inline void __stdcall CloseWwanHandle(_Frees_ptr_ HANDLE hClientHandle)
        {
            ::WwanCloseHandle(hClientHandle, nullptr);
        }
        inline void __stdcall CloseWwan2Handle(_Frees_ptr_ HANDLE hClientHandle)
        {
            ::Wwan2CloseHandle(hClientHandle);
        }
    }
    /// @endcond

    typedef unique_any<HANDLE, decltype(&details::CloseWwanHandle), details::CloseWwanHandle, details::pointer_access_all, HANDLE, INT_PTR, -1> unique_wwan_handle;
    typedef unique_any<HANDLE, decltype(&details::CloseWwan2Handle), details::CloseWwan2Handle, details::pointer_access_all, HANDLE, INT_PTR, -1> unique_wwan2_handle;
#endif
#if defined(__WIL_WWAN_API_DECL__) && !defined(__WIL_WWAN_API_DECL_STL) && defined(WIL_RESOURCE_STL)
#define __WIL_WWAN_API_DECL_STL
    typedef shared_any<unique_wwan_handle> shared_wwan_handle;
    typedef weak_any<shared_wwan_handle> weak_wwan_handle;
    typedef shared_any<unique_wwan2_handle> shared_wwan2_handle;
    typedef weak_any<shared_wwan2_handle> weak_wwan2_handle;
#endif

#if defined(_NTEXAPI_) && !defined(__WIL_NTEXAPI_)
#define __WIL_NTEXAPI_
    typedef wil::unique_struct<WNF_STATE_NAME, decltype(&::NtDeleteWnfStateName), ::NtDeleteWnfStateName> unique_wnf_state_name;
#endif

#if defined(STATELOCK_FLAG_TRANSIENT) && !defined(__WIL_STATELOCK_FLAG_TRANSIENT)
#define __WIL_STATELOCK_FLAG_TRANSIENT
    typedef unique_any<HSTATE, decltype(&::CloseState), ::CloseState> unique_hstate;
    typedef unique_any<HSTATE_NOTIFICATION, decltype(&::CloseStateChangeNotification), ::CloseStateChangeNotification> unique_hstate_notification;
#endif

#if defined(_EFSWRTEXT_H_) && !defined(__WIL_EFSWRTEXT_H_)
#define __WIL_EFSWRTEXT_H_
    typedef unique_any<PENCRYPTION_PROTECTOR_LIST, decltype(&::FreeIdentityProtectorList), ::FreeIdentityProtectorList> unique_pencryption_protector_list;
#endif

#if defined(_EDPUTIL_H_) && !defined(__WIL_EDPUTIL_H_)
#define __WIL_EDPUTIL_H_
    typedef unique_any<EDP_CONTEXT*, decltype(&::EdpFreeContext), ::EdpFreeContext> unique_edp_context;
#endif

#if (defined(POLICYMANAGER_H) || defined(DMPOLICYEXT_H)) && !defined(__WIL_POLICYMANAGER_DEF_H_)
#define __WIL_POLICYMANAGER_DEF_H_
    typedef unique_any<PWSTR, decltype(&::PolicyManager_FreeStringValue), ::PolicyManager_FreeStringValue> unique_policy_string;
    typedef unique_any<PMPolicyRetrievedInfo*, decltype(&::PolicyManager_FreeGetPolicyData), ::PolicyManager_FreeGetPolicyData> unique_policy_retrieved_info;
#endif


#if defined(COREMESSAGING_API) && !defined(__WIL_UNIQUE_HENDPOINT_)
#define __WIL_UNIQUE_HENDPOINT_

    namespace details
    {
        inline void __stdcall IMessageSessionCloseEndpointFunction(IMessageSession* source, HENDPOINT token)
        {
            source->CloseEndpoint(token);
        }
    }

    using unique_hendpoint = unique_com_token<IMessageSession, HENDPOINT, decltype(details::IMessageSessionCloseEndpointFunction), details::IMessageSessionCloseEndpointFunction, 0>;
#endif

#if defined(MESSAGESTRINGAPI) && !defined(__WIL_MESSAGESTRINGAPI_)
#define __WIL_MESSAGESTRINGAPI_
    typedef wil::unique_any<HMESSAGEBLOB, decltype(&::MsgRelease), ::MsgRelease> unique_hmessageblob;
    typedef wil::unique_any<HMESSAGESTRING, decltype(&::MsgRelease), ::MsgRelease> unique_hmessagestring;
#endif // __WIL_UNIQUE_HMESSAGESTRING_
#if defined(__WIL_MESSAGESTRINGAPI_) && !defined(__WIL_MESSAGESTRINGAPI_STL) && defined(WIL_RESOURCE_STL)
#define __WIL_MESSAGESTRINGAPI_STL
    typedef shared_any<unique_hmessageblob> shared_hmessageblob;
    typedef shared_any<unique_hmessagestring> shared_hmessagestring;
    typedef weak_any<shared_hmessageblob> weak_hmessageblob;
    typedef weak_any<shared_hmessagestring> weak_hmessagestring;
#endif // __WIL_HMESSAGESTRING_STL

#if defined(__STATEREPOSITORYCACHE_INCLUDED) && !defined(__WIL___STATEREPOSITORYCACHE_)
#define __WIL___STATEREPOSITORYCACHE_
    using unique_srcache_deleter = function_deleter<decltype(&::SRCache_Free), ::SRCache_Free>;

    template <typename T = void>
    using unique_srcache_ptr = wistd::unique_ptr<T, unique_srcache_deleter>;
#endif

#if defined(_STATEREPOSITORYINTERNAL_H_) && !defined(__WIL_STATEREPOSITORYINTERNAL_)
#define __WIL_STATEREPOSITORYINTERNAL_
    typedef unique_any<SRDictionarySerialized*, decltype(&::SRDictionaryFree), ::SRDictionaryFree> unique_srdictionary_serialized;
#endif

#if defined(_DEVQUERY_H_) && !defined(_WIL_DEVQUERY_)
#define _WIL_DEVQUERY_
    typedef unique_any<HDEVQUERY, decltype(&::DevCloseObjectQuery), ::DevCloseObjectQuery> unique_hdevquery;
#endif

#if defined(_DEVICEASSOCIATION_H) && !defined(_WIL_DAF_)
#define _WIL_DAF_
    typedef wil::unique_any<DAF_ASSOCIATION_HANDLE, decltype(&::DafCloseAssociationContext), ::DafCloseAssociationContext> unique_daf_association_handle;
    typedef wil::unique_any<DAF_INBOUND_HANDLE, decltype(&::DafCloseInboundContext), ::DafCloseInboundContext> unique_daf_inbound_handle;
    typedef wil::unique_any<DAF_CHALLENGE_HANDLE, decltype(&::DafCloseChallengeContext), ::DafCloseChallengeContext> unique_daf_challenge_handle;
    typedef wil::unique_any<DAF_IMPORT_EXPORT_HANDLE, decltype(&::DafCloseImportExportContext), ::DafCloseImportExportContext> unique_daf_import_export_handle;

    struct DafUserMemoryDeleter
    {
        template <typename T>
        void operator()(_Pre_opt_valid_ _Frees_ptr_opt_ T* memory) const
        {
            DAF_user_free(memory);
        }
    };

    template <typename T>
    using unique_daf_user_ptr = wistd::unique_ptr<T, DafUserMemoryDeleter>;

    struct DafMemoryDeleter
    {
        template <typename T>
        void operator()(_Pre_opt_valid_ _Frees_ptr_opt_ T* memory) const
        {
            DafMemFree(memory);
        }
    };

    template <typename T>
    using unique_daf_ptr = wistd::unique_ptr<T, DafMemoryDeleter>;
#endif

#if defined(_DOT3_DOT3API_H) && !defined(_WIL_DOT3_DOT3API_H) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define _WIL_DOT3_DOT3API_H
    using dot3_deleter = function_deleter<decltype(&::Dot3FreeMemory), ::Dot3FreeMemory>;

    template <typename T>
    using unique_dot3_ptr = wistd::unique_ptr<T, dot3_deleter>;

    /// @cond
    namespace details {
        inline void __stdcall CloseDot3Handle(_Frees_ptr_ HANDLE hClientHandle)
        {
            ::Dot3CloseHandle(hClientHandle, nullptr);
        }
    }
    /// @endcond

    typedef unique_any_handle_null<decltype(&details::CloseDot3Handle), details::CloseDot3Handle> unique_dot3_handle;
#endif

#if defined(_WIL_DOT3_DOT3API_H) && !defined(_WIL_DOT3_DOT3API_H_STL) && defined(WIL_RESOURCE_STL)
#define _WIL_DOT3_DOT3API_H_STL
    typedef shared_any<unique_dot3_handle> shared_dot3_handle;
    typedef weak_any<shared_dot3_handle> weak_dot3_handle;
#endif

#if defined(_LANPRFL_H) && !defined(_WIL_LANPRFL_H) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define _WIL_LANPRFL_H
    using lp_profile_deleter = function_deleter<decltype(&::LpFreeProfile), ::LpFreeProfile>;

    using unique_lp_profile = wistd::unique_ptr<PM_PROFILE, lp_profile_deleter>;
#endif

#if defined(_PNPRTL_H_) && !defined(_WIL_PNPRTL_)
#define _WIL_PNPRTL_
    typedef unique_any<HPNPCTX, decltype(&::_PnpCtxCloseMachine), ::_PnpCtxCloseMachine> unique_hpnpctx;
#endif

} // namespace wil

#pragma warning(pop)
