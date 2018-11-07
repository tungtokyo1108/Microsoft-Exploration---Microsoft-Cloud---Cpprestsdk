/*
 * pplxlinux.h
 *
 *  Created on: Nov 7, 2018
 *      Student (MIG Virtual Developer): Tung Dang
 */

#ifndef INCLUDE_PPLX_PPLXLINUX_H_
#define INCLUDE_PPLX_PPLXLINUX_H_

#ifndef _WIN32
#include <signal.h>
#include "pthread.h"
#include "cpprest/details/cpprest_compat.h"

#if defined(__APPLE__)
#include <dispatch/dispatch.h>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#else
#include <mutex>
#include <condition_variable>
#endif

#include "pplx/pplxinterface.h"

namespace pplx
{
#if defined(__APPLE__)
namespace cpprest_synchronization = ::boost;
#else
namespace cpprest_synchronization = ::std;
#endif

namespace details
{
namespace platform
{

/*
 * Returns a unique identifier for the execution thread where this routine in invoked
 */
_PPLXIMP long _pplx_cdecl GetCurrentThreadId();

/*
 * Yields the execution of the current execution thread - typically when spin-waiting
 */
_PPLXIMP void _pplx_cdecl YieldExecution();

/*
 * Caputeres the callstack
 */
__declspec(noinline) inline static size_t CaptureCallstack(void**, size_t, size_t)
{
	return 0;
}
}

class event_impl
{
private:
	cpprest_synchronization::mutex _lock;
	cpprest_synchronization::condition_variable _condition;
	bool _signaled;

public:
	static const unsigned int timeout_infinite = 0xFFFFFFFF;
	event_impl() :
		_signaled(false)
	{}

	void set()
	{
		cpprest_synchronization::lock_guard<cpprest_synchronization::mutex> lock(_lock);
	}

};

}



}

#endif

#endif /* INCLUDE_PPLX_PPLXLINUX_H_ */






































