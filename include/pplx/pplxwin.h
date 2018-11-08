/*
 * pplxwin.h
 *
 *  Created on: Nov 7, 2018
 *      Student (MIG Virtual Developer): Tung Dang
 */

#ifndef INCLUDE_PPLX_PPLXWIN_H_
#define INCLUDE_PPLX_PPLXWIN_H_

#if !defined(_WIN32) || _MSC_VER < 1800 || CPPREST_FORCE_PPLX

#include "cpprest/details/cpprest_compat.h"
#include "pplx/pplxinterface.h"

namespace details
{

namespace platform
{

/*
 * Return a unique identifier for the execution thread where this routine in invoked
 */
_PPLXIMP long __cdecl GetCurrentThreadId();

/*
 * Yields the execution of the current execution thread - typically when spin-waiting
 */
_PPLXIMP void __cdecl YieldExecution();

/*
 * Captures the callstack
 */
__declspec(noninline) __PPLXIMP size_t __cdecl CaptureCallstack(void **, size_t , size_t);

#if defined(__cplusplus_winrt)
_PPLXIMP unsigned int __cdecl GetNextAsyncId();
#endif
}

class event_impl
{
public:
	static const unsigned int timeout_infinite = 0xFFFFFFFF;
	_PPLXIMP event_impl();
	_PPLXIMP ~event_impl();
	_PPLXIMP void set();
	_PPLXIMP void reset();
	_PPLXIMP unsigned int wait(unsigned int timeout);
	unsigned int wait()
	{
		return wait(event_impl::timeout_infinite);
	}

private:
	void * _M_impl;
	event_impl(const event_impl&);
	event_impl const & operator=(const event_impl&);
};

}

#endif

#endif /* INCLUDE_PPLX_PPLXWIN_H_ */
