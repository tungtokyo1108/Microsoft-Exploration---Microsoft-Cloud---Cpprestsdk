/*
 * cpprest_compat.h
 *
 *  Created on: Oct 12, 2018
 *      Student (MIG Virtual Developer): Tung Dang
 */

#ifndef INCLUDE_CPPREST_DETAILS_CPPREST_COMPAT_H_
#define INCLUDE_CPPREST_DETAILS_CPPREST_COMPAT_H_

#if defined(_WIN32)

#if _MSC_VER >= 1900
#define CPPREST_NOEXCEPT noexcept
#define CPPREST_CONSTEXPR constexpr
#else
#define CPPREST_NOEXCEPT
#define CPPREST_CONSTEXPR const
#endif

#define CASABLANCA_UNREFERENCED_PARAMETER(x) (x)
#include <sal.h>
#else

#define __declspec(x) __attribute__((x))
#define dllimport
#define novtable
#define __assume(x) do {if (!(x)) __builtin_unreachable();} while (false)
#define CASABLANCA_UNREFERENCED_PARAMETER(x) (void)x
#define CPPREST_NOEXCEPT noexcept
#define CPPREST_CONSTEXPR constexpr
#include <assert.h>
#define _ASSERTE(x) assert(x)
#include "cpprest/details/nosal.h"
#if not defined __cdecl
#if defined cdecl
#define __cdecl __attribute__ ((cdecl))
#else
#define __cdecl
#endif

#if defined(__ANDROID__)
#if not defined BOOST_ASIO_DISABLE_THREAD_KEYWORD_EXTENSION
#define BOOST_ASIO_DISABLE_THREAD_KEYWORD_EXTENSION
#endif
#endif

#ifdef __clang__
#include <cstdio>
#endif

#endif // defined(__APPLE__)
#endif

#ifdef _NO_ASYNCRITIMP
#define _ASYNCRTIMP
#else
#ifdef _ASYNCRT_EXPORT
#define _ASYNCRTIMP __declspec(dllexport)
#else
#define _ASYNCRTIMP __declspec(dllimport)
#endif
#endif

#ifdef CASABLANCA_DEPRECATION_NO_WARNINGS
#define CASABLANCA_DEPRECATED(x)
#else
#define CASABLANCA_DEPRECATED(x) __declspec(deprecated(x))
#endif

#endif /* INCLUDE_CPPREST_DETAILS_CPPREST_COMPAT_H_ */
