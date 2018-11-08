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

namespace pplx {

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

/*
 * Manual reset event
 */

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

/*
 * Mutex-lock for mutual exclusion
 */

class critical_section_impl
{
public:
	_PPLXIMP critical_section_impl();
	_PPLXIMP ~critical_section_impl();
	_PPLXIMP void lock();
	_PPLXIMP void unlock();

private:
	typedef void * _PPLX_BUFFER;
	_PPLX_BUFFER _M_impl[8];

	critical_section_impl(const critical_section_impl&);
	critical_section_impl const & operator = (const critical_section_impl&);
};

#if _WIN32_WINNT >= _WIN32_WINNT_VISTA
class reader_writer_lock_impl
{
public:
	class scoped_lock_read
	{
	public:
		explicit scoped_lock_read(reader_writer_lock_impl & _Reader_writer_lock) : _M_reader_writer_lock(_Reader_writer_lock)
		{
			_M_reader_writer_lock.lock_read();
		}

		~scoped_lock_read()
		{
			_M_reader_writer_lock.unlock();
		}

	private:
		reader_writer_lock_impl& _M_reader_writer_lock;
		scoped_lock_read(const scoped_lock_read&);
		scoped_lock_read const & operator=(const scoped_lock_read&);
	};

	_PPLXIMP reader_writer_lock_impl();
	_PPLXIMP void lock();
	_PPLXIMP void lock_read();
	_PPLXIMP void unlock();

private:
	void *_M_impl;
	bool m_locked_execlusive;
};
#endif

/*
 * Recursive mutex
 */
class recursive_lock_impl
{
public:
	recursive_lock_impl() :
		_M_owner(-1), _M_recursionCount()
    {}

	~recursive_lock_impl()
	{
		_ASSERT(_M_owner == -1);
		_ASSERT(_M_recursionCount == 0);
	}

	void lock()
	{
		auto id = ::pplx::details::platform::GetCurrentThreadId();
		if (_M_owner == id)
		{
			_M_recursionCount++;
		}
		else
		{
			_M_cs.lock();
			_M_owner = id;
			_M_recursionCount = 1;
		}
	}

	void unlock()
	{
		_ASSERT(_M_owner == ::pplx::details::platform::GetCurrentThreadId());
		_ASSERT(_M_recursionCount >= 1);

		_M_recursionCount--;

		if(_M_recursionCount == 0)
		{
			_M_owner = -1;
			_M_cs.unlock();
		}
	}

private:
	pplx::details::critical_section_impl _M_cs;
	long _M_recursionCount;
	volatile long _M_owner;
};

class windows_scheduler : public pplx::scheduler_interface
{
public:
	_PPLXIMP virtual void schedule(TaskProc_t proc, _In_ void* param);
};

}

/*
 * A generic RAII wrapper for locks that implement the critical_section interface
 */
template <class _Lock>
class scoped_lock
{
public:
	explicit scoped_lock(_Lock& _Critical_section) : _M_critical_section(_Critical_section)
	{
		_M_critical_section.lock();
	}

	~scoped_lock()
	{
		_M_critical_section.unlock();
	}

private:
	_Lock& _M_critical_section;
	scoped_lock(const scoped_lock&);
	scoped_lock const & operator=(const scoped_lock&);
};

namespace extensibility
{
typedef ::pplx::details::event_impl event_t;
typedef ::pplx::details::critical_section_impl critical_section_t;

#if _WIN32_WINNT >= _WIN32_WINNT_VISTA
    typedef ::pplx::details::reader_writer_lock_impl reader_writer_lock_t;
    typedef scoped_lock<reader_writer_lock_t> scoped_rw_lock_t;
    typedef reader_writer_lock_t::scoped_lock_read scoped_read_lock_t;
#endif

typedef ::pplx::details::recursive_lock_impl recursive_lock_t;
typedef scoped_lock<recursive_lock_t> scoped_recursive_lock_t;
}

typedef details::windows_scheduler default_scheduler_t;

namespace details
{
#ifndef _REPORT_PPLTASK_UNOBSERVED_EXCEPTION
    #define _REPORT_PPLTASK_UNOBSERVED_EXCEPTION() do { \
        __debugbreak(); \
        std::terminate(); \
    } while(false)
#endif
}

}
#endif
#endif /* INCLUDE_PPLX_PPLXWIN_H_ */
