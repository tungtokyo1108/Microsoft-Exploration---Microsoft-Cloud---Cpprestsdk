/*
 * pplxinterface.h
 *
 *  Created on: Nov 6, 2018
 *      Student (MIG Virtual Developer): Tung Dang
 */

#ifndef INCLUDE_PPLX_PPLXINTERFACE_H_
#define INCLUDE_PPLX_PPLXINTERFACE_H_

#if (defined(_MSC_VER) && (_MSC_VER >= 1800)) && !CPPREST_FORCE_PPLX
#error This file must not be included for Visual Studio 12 or later
#endif

#if defined(_CRTBLD)
#elif defined(_WIN32)
#if (_MSC_VER >= 1700)
#define _USE_REAL_ATOMICS
#endif
#else // GCC compiler
#define _USE_REAL_ATOMICS
#endif

#include <memory>
#ifdef _USE_REAL_ATOMICS
#include <atomic>
#endif
#include <iostream>

#define _pplx_cdecl __cdecl

namespace pplx
{

typedef void (_pplx_cdecl * TaskProc_t)(void*);

/*
 * Scheduler Interface
 */
struct __declspec(novtable) scheduler_interface
{
	virtual void schedule (TaskProc_t, _In_ void*) = 0;
};

struct scheduler_ptr
{
	explicit scheduler_ptr(std::shared_ptr<scheduler_interface> scheduler) : m_sharedScheduler(std::move(scheduler))
	{
		m_scheduler = m_sharedScheduler.get();
	}

	explicit scheduler_ptr(_In_opt_ scheduler_interface *pScheduler) : m_scheduler(pScheduler)
	{
	}

	/*
	 * Behave like a pointer
	 */
	scheduler_interface *operator->() const
	{
		return get();
	}

	scheduler_interface * get() const
	{
		return m_scheduler;
	}

	operator bool() const {return get() != nullptr;}

private:
	std::shared_ptr<scheduler_interface> m_sharedScheduler;
	scheduler_interface *m_scheduler;
};

/*
 * Describes the execution status of a task group or structured task group object.
 * A value of this type is returned by numerous method that wait on tasks scheduled
 * to a task group to complete
 */
enum task_group_status
{
	/*
	 * The task queued to the task group object have not completed.
	 * This value is not presently returned to the Concurrency Runtime
	 */
	not_completed,

	/*
	 * The tasks queued to the task group completed successfully
	 */
	completed,

	/*
	 * The task_group object was canceled
	 */
	canceled,
};

namespace details {

#ifdef _USE_REAL_ATOMICS
typedef std::atomic<long> atomic_long;
typedef std::atomic<size_t> atomic_size_t;

template <typename _T>
_T atomic_compare_exchange(std::atomic<_T>& _Target, _T _Exchange, _T _Comparand)
{
	_T _Result = _Comparand;
	_Target.compare_exchange_strong(_Result, _Exchange);
	return _Result;
}

template <typename _T>
_T atomic_increment(std::atomic<_T>& _Target)
{
	return _Target.fetch_add(1) + 1;
}

template <typename _T>
_T atomic_decrement(std::atomic<_T>& _Target)
{
	return _Target.fetch_sub(1) - 1;
}

template <typename _T>
_T atomic_add(std::atomic<_T>& _Target, _T value)
{
	return _Target.fetch_add(value) + value;
}


typedef long volatile atomic_long;
typedef size_t volatile atomic_size_t;

template <class T>
inline T atomic_exchange(T volatile& _Target, T _Value)
{
	return _InterlockedExchange(&_Target, _Value);
}

inline long atomic_increment(long volatile& _Target)
{
	return _InterlockedIncrement(& _Target);
}

inline long atomic_add(long volatile& _Target, long value )
{
	return _InterlockedExchangeAdd(&_Target, value) + value;
}

inline size_t atomic_increment(size_t volatile& _Target)
{
#if (defined(_M_IX86) || defined(_M_ARM))
	return static_cast<size_t>(_InterlockedIncrement(reinterpret_cast<long volatile*>(&_Target)));
#else
	return static_cast<size_t>(_InterlockedIncrement64(reinterpret_cast<__int64 volatile*>(&_Target)));
#endif
}

inline long atomic_compare_exchange(long volatile & _Target, long _Exchange, long _Comparand)
{
    return _InterlockedCompareExchange(&_Target, _Exchange, _Comparand);
}

inline size_t atomic_compare_exchange(size_t volatile& _Target, size_t _Exchange, size_t _Comparand)
{
#if (defined(_M_IX86) || defined(_M_ARM))
	return static_cast<size_t>(_InterlockedCompareExchange(reinterpret_cast<long volatile*>(_Target), static_cast<long>(_Exchange), static_cast<long>(_Comparand)));
#else
    return static_cast<size_t>(_InterlockedCompareExchange64(reinterpret_cast<__int64 volatile *>(_Target), static_cast<__int64>(_Exchange), static_cast<__int64>(_Comparand)));
#endif
}

#endif
}}
#endif /* INCLUDE_PPLX_PPLXINTERFACE_H_ */
