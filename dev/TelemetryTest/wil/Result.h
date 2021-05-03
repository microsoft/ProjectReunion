#pragma once
// Windows Internal Libraries (wil)
// Result.h:  WIL Error Handling Helpers Library
//
// Usage Guidelines:
// https://osgwiki.com/wiki/WIL_Error_Handling_Helpers
//
// WIL Discussion Alias (wildisc):
// http://idwebelements/GroupManagement.aspx?Group=wildisc&Operation=join  (one-click join)
//
//! @file
//! WIL Error Handling Helpers: a family of macros and functions designed to uniformly handle and report
//! errors across return codes, fail fast, exceptions and logging

#ifndef __WIL_INTERNAL_RESULT_INCLUDED
#define __WIL_INTERNAL_RESULT_INCLUDED

// Common.h contains logic and configuration macros common to all consumers in Windows and therefore needs to be
// included before any 'opensource' headers
#include "Common.h"

// Note that we avoid pulling in STL's memory header from Result.h through Resource.h as we have
// Result.h customers who are still on older versions of STL (without std::shared_ptr<>).
#pragma push_macro("RESOURCE_SUPPRESS_STL")
#ifndef RESOURCE_SUPPRESS_STL
#define RESOURCE_SUPPRESS_STL
#endif

#include <wil/result.h>

// Internal WIL header files that were historically included before WIL was split into public and internal versions.
// These exist only to maintain back-compat
#include "Resource.h"

#pragma pop_macro("RESOURCE_SUPPRESS_STL")
#endif // __WIL_INTERNAL_RESULT_INCLUDED
