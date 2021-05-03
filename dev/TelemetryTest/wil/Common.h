#pragma once
// Windows Internal Libraries (wil)
// Resource.h: WIL Resource Wrappers (RAII) Library
//
// Usage Guidelines:
// https://osgwiki.com/wiki/Windows_Internal_Libraries_(wil)
//
// wil Discussion Alias (wildisc):
// http://idwebelements/GroupManagement.aspx?Group=wildisc&Operation=join  (one-click join)
//
//! @file
//! WIL Common Helpers: Provides broadly applicable, dependency-free pure C++ helpers, macros and type traits.

#ifndef __WIL_INTERNAL_COMMON_INCLUDED
#define __WIL_INTERNAL_COMMON_INCLUDED

#include <wil/common.h>

#if defined(__cplusplus) && !defined(__WIL_MIN_KERNEL)

/// @cond
#ifndef WIL_HIDE_DEPRECATED_1611
WIL_WARN_DEPRECATED_1611_PRAGMA("WI_SET_FLAG_IF_TRUE", "WI_IS_CLEAR_OR_SINGLE_FLAG_SET_IN_MASK", "WI_IS_CLEAR_OR_SINGLE_FLAG_SET",
    "WI_IS_SINGLE_FLAG_SET_IN_MASK", "WI_IS_SINGLE_FLAG_SET", "WI_IS_FLAG_CLEAR", "WI_IS_ANY_FLAG_CLEAR", "WI_ARE_ALL_FLAGS_CLEAR",
    "WI_IS_FLAG_SET", "WI_IS_ANY_FLAG_SET", "WI_ARE_ALL_FLAGS_SET", "WI_TOGGLE_ALL_FLAGS", "WI_TOGGLE_FLAG", "WI_UPDATE_FLAGS_IN_MASK",
    "WI_UPDATE_FLAG", "WI_CLEAR_FLAG_IF_FALSE", "WI_CLEAR_FLAG_IF", "WI_CLEAR_FLAG", "WI_CLEAR_ALL_FLAGS", "WI_SET_FLAG_IF_FALSE",
    "WI_SET_FLAG_IF", "WI_SET_FLAG", "WI_SET_ALL_FLAGS", "WI_COMPILETIME_COMBINE_FLAGS", AssignToOptParam, AssignNullToOptParam);
#define __WI_COMPILETIME_COMBINE_FLAGS1(a1) (a1)
#define __WI_COMPILETIME_COMBINE_FLAGS2(a1, a2) static_cast<decltype((a1) | (a2))>( \
    static_cast<::wil::integral_from_enum<decltype(a1)>>(a1) | \
    static_cast<::wil::integral_from_enum<decltype(a2)>>(a2))
#define __WI_COMPILETIME_COMBINE_FLAGS3(a1, a2, a3) static_cast<decltype((a1) | (a2) | (a3))>( \
    static_cast<::wil::integral_from_enum<decltype(a1)>>(a1) | \
    static_cast<::wil::integral_from_enum<decltype(a2)>>(a2) | \
    static_cast<::wil::integral_from_enum<decltype(a3)>>(a3))
#define __WI_COMPILETIME_COMBINE_FLAGS4(a1, a2, a3, a4) static_cast<decltype((a1) | (a2) | (a3) | (a4))>( \
    static_cast<::wil::integral_from_enum<decltype(a1)>>(a1) | \
    static_cast<::wil::integral_from_enum<decltype(a2)>>(a2) | \
    static_cast<::wil::integral_from_enum<decltype(a3)>>(a3) | \
    static_cast<::wil::integral_from_enum<decltype(a4)>>(a4))
#define __WI_COMPILETIME_COMBINE_FLAGS5(a1, a2, a3, a4, a5) static_cast<decltype((a1) | (a2) | (a3) | (a4) | (a5))>( \
    static_cast<::wil::integral_from_enum<decltype(a1)>>(a1) | \
    static_cast<::wil::integral_from_enum<decltype(a2)>>(a2) | \
    static_cast<::wil::integral_from_enum<decltype(a3)>>(a3) | \
    static_cast<::wil::integral_from_enum<decltype(a4)>>(a4) | \
    static_cast<::wil::integral_from_enum<decltype(a5)>>(a5))
// DEPRECATED: constexpr support for the '|' flags operator should no longer require use of this construct
#define WI_COMPILETIME_COMBINE_FLAGS(...)                   WI_MACRO_DISPATCH(__WI_COMPILETIME_COMBINE_FLAGS, __VA_ARGS__)
// DEPRECATED: Use WI_SetAllFlags
#define WI_SET_ALL_FLAGS                                    WI_SetAllFlags
// DEPRECATED: Use WI_SetFlag
#define WI_SET_FLAG                                         WI_SetFlag
// DEPRECATED: Use WI_SetFlagIf
#define WI_SET_FLAG_IF                                      WI_SetFlagIf
// DEPRECATED: Use WI_SetFlagIf(var, flag, !condition)
#define WI_SET_FLAG_IF_FALSE(var, flag, condition)          do { if (!(wil::verify_bool(condition))) { WI_SetFlag(var, flag); } } while (0, 0)
// DEPRECATED: Use WI_ClearAllFlags
#define WI_CLEAR_ALL_FLAGS                                  WI_ClearAllFlags
// DEPRECATED: Use WI_ClearFlag
#define WI_CLEAR_FLAG                                       WI_ClearFlag
// DEPRECATED: Use WI_ClearFlagIf
#define WI_CLEAR_FLAG_IF                                    WI_ClearFlagIf
// DEPRECATED: Use ClearFlagIf(var, flag, !condition)
#define WI_CLEAR_FLAG_IF_FALSE(var, flag, condition)        do { if (!(wil::verify_bool(condition))) { WI_ClearFlag(var, flag); } } while (0, 0)
// DEPRECATED: Use WI_UpdateFlag
#define WI_UPDATE_FLAG                                      WI_UpdateFlag
// DEPRECATED: Use WI_UpdateFlagsInMask
#define WI_UPDATE_FLAGS_IN_MASK                             WI_UpdateFlagsInMask
// DEPRECATED: Use WI_ToggleFlag
#define WI_TOGGLE_FLAG                                      WI_ToggleFlag
// DEPRECATED: Use WI_ToggleAllFlags
#define WI_TOGGLE_ALL_FLAGS                                 WI_ToggleAllFlags
// DEPRECATED: Use WI_AreAllFlagsSet
#define WI_ARE_ALL_FLAGS_SET                                WI_AreAllFlagsSet
// DEPRECATED: Use WI_IsAnyFlagSet
#define WI_IS_ANY_FLAG_SET                                  WI_IsAnyFlagSet
// DEPRECATED: Use WI_IsFlagSet
#define WI_IS_FLAG_SET                                      WI_IsFlagSet
// DEPRECATED: Use WI_AreAllFlagsClear
#define WI_ARE_ALL_FLAGS_CLEAR                              WI_AreAllFlagsClear
// DEPRECATED: Use WI_IsAnyFlagClear
#define WI_IS_ANY_FLAG_CLEAR                                WI_IsAnyFlagClear
// DEPRECATED: Use WI_IsFlagClear
#define WI_IS_FLAG_CLEAR                                    WI_IsFlagClear
// DEPRECATED: Use WI_IsSingleFlagSet
#define WI_IS_SINGLE_FLAG_SET                               WI_IsSingleFlagSet
// DEPRECATED: Use WI_IsSingleFlagSetInMask
#define WI_IS_SINGLE_FLAG_SET_IN_MASK                       WI_IsSingleFlagSetInMask
// DEPRECATED: Use WI_IsClearOrSingleFlagSet
#define WI_IS_CLEAR_OR_SINGLE_FLAG_SET                      WI_IsClearOrSingleFlagSet
// DEPRECATED: Use WI_IsClearOrSingleFlagSetInMask
#define WI_IS_CLEAR_OR_SINGLE_FLAG_SET_IN_MASK              WI_IsClearOrSingleFlagSetInMask
// DEPRECATED: Use WI_SetFlagIf
#define WI_SET_FLAG_IF_TRUE                                 WI_SetFlagIf
#if defined(WIL_SUPPORT_BITOPERATION_PASCAL_NAMES) || defined(SUPPORT_BITOPERATION_PASCAL_NAMES)
#define SetAllFlags                                         WI_SetAllFlags
#define SetFlag                                             WI_SetFlag
#define SetFlagIf                                           WI_SetFlagIf
#define SetFlagIfFalse                                      WI_SetFlagIfFalse
#define ClearAllFlags                                       WI_ClearAllFlags
#define ClearFlag                                           WI_ClearFlag
#define ClearFlagIf                                         WI_ClearFlagIf
#define ClearFlagIfFalse                                    WI_ClearFlagIfFalse
#define UpdateFlag                                          WI_UpdateFlag
#define UpdateFlagsInMask                                   WI_UpdateFlagsInMask
#define ToggleAllFlags                                      WI_ToggleAllFlags
#define ToggleFlag                                          WI_ToggleFlag
#define AreAllFlagsSet                                      WI_AreAllFlagsSet
#define IsAnyFlagSet                                        WI_IsAnyFlagSet
#define IsFlagSet                                           WI_IsFlagSet
#define AreAllFlagsClear                                    WI_AreAllFlagsClear
#define IsAnyFlagClear                                      WI_IsAnyFlagClear
#define IsFlagClear                                         WI_IsFlagClear
#define IsSingleFlagSet                                     WI_IsSingleFlagSet
#define IsSingleFlagSetInMask                               WI_IsSingleFlagSetInMask
#define IsClearOrSingleFlagSet                              WI_IsClearOrSingleFlagSet
#define IsClearOrSingleFlagSetInMask                        WI_IsClearOrSingleFlagSetInMask
#endif // WIL_SUPPORT_BITOPERATION_PASCAL_NAMES
namespace wil
{
    // DEPRECATED: use assign_to_opt_param
    template <typename T>
    inline void AssignToOptParam(_Out_opt_ T* outParam, T val)
    {
        if (outParam != nullptr)
        {
            *outParam = val;
        }
    }
    // DEPRECATED: use assign_null_to_opt_param
    template <typename T>
    inline void AssignNullToOptParam(_Out_opt_ T* outParam)
    {
        if (outParam != nullptr)
        {
            *outParam = nullptr;
        }
    }
}
#endif // WIL_HIDE_DEPRECATED_1611
/// @endcond

#endif // defined(__cplusplus) && !defined(__WIL_MIN_KERNEL)

#endif // __WIL_INTERNAL_COMMON_INCLUDED
