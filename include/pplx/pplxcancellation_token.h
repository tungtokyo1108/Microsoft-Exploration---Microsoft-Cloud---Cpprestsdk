/*
 * pplxcancellation_token.h
 * https://github.com/PowerShell/cpprestsdk/blob/dev/Release/include/pplx/pplxcancellation_token.h
 *
 *  Created on: Nov 19, 2018
 *      Student (MIG Virtual Developer): Tung Dang
 */

#ifndef INCLUDE_PPLX_PPLXCANCELLATION_TOKEN_H_
#define INCLUDE_PPLX_PPLXCANCELLATION_TOKEN_H_

#if (defined(_MSC_VER) && (_MSC_VER >= 1800)) && !CPPREST_FORCE_PPLX
#error This file must not be included for Visual Studio 12 or later
#endif

#include <cstdlib>
#include <string>
#include "pplx/pplxinterface.h"

#pragma pack(push, _CRT_PACKING)
#pragma push_macro("new")
#undef new

namespace pplx {

/*
 * The class describes an exception thrown by the PPL tasks
 * Goal: force the current task to cancel.
 */

class task_canceled : public std::exception
{
private:
	std::string _message;

public:
	explicit task_canceled(_In_z_ const char* _Message) throw()
	    : _message(_Message)
	{}

	task_canceled() throw()
			: exception()
	{}

	~task_canceled() throw() {}

	const char* what() const CPPREST_NOEXCEPT
	{
		return _message.c_str();
	}
};

class invalid_operation : public std::exception
{
private:
	std::string _message;

public:
	invalid_operation(_In_z_ const char* _Message) throw()
	     : _message(_Message)
    {}

	invalid_operation() throw()
			: exception()
	{}

	~invalid_operation() throw() {}

	const char* what() const CPPREST_NOEXCEPT
	{
		return _message.c_str();
	}
};

namespace details
{

class _RefCounter
{
public:
	virtual ~_RefCounter()
	{
		_ASSERTE(_M_refCount == 0);
	}

	long _Reference()
	{
		long _Refcount = atomic_increment(_M_refCount);
		_ASSERTE(_RefCount > 1);
		return _Refcount;
	}

	long _Release()
	{
		long _Refcount = atomic_decrement(_M_refCount);
		_ASSERTE(_Refcount >= 0);
		if (_Refcount == 0)
		{
			_Destroy();
		}

		return _Refcount;
	}

protected:
	virtual void _Destroy()
	{
		delete this;
	}

	_RefCounter(long _InitialCount = 1) : _M_refCount(_InitialCount)
	{
	    _ASSERTE(_M_refCount > 0);
	}

	atomic_long _M_refCount;
};

class _CancellationTokenState;
class _CancellationTokenRegistration : public _RefCounter
{
private:
	static const long _STATE_CLEAR = 0;
	static const long _STATE_DEFER_DELETE = 1;
	static const long _STATE_SYNCHRONIZE = 2;
	static const long _STATE_CALLED = 3;

public:
	_CancellationTokenRegistration(long _InitialRefs = 1) :
		_RefCounter(_InitialRefs),
		_M_state(_STATE_CALLED),
		_M_pTokenState(NULL)
    {}

	_CancellationTokenState *_GetToken() const
	{
		return _M_pTokenState;
	}

protected:
	virtual ~_CancellationTokenRegistration()
	{
		_ASSERTE(_M_state != _STATE_CLEAR);
	}

	virtual void _Exec() = 0;

private:
	friend class _CancellationTokenState;
	void _Invoke()
	{
		long tid = ::pplx::details::platform::GetCurrentThreadId();
		_ASSERTE((tid & 0x3) == 0);

		long result = atomic_compare_exchange(_M_state, tid, _STATE_CLEAR);
		if (result == _STATE_CLEAR)
		{
			_Exec();
			result = atomic_compare_exchange(_M_state, _STATE_CALLED, tid);
			if (result == _STATE_SYNCHRONIZE)
			{
				_M_pSyncBlock->set();
			}
		}
		_Release();
	}

	atomic_long _M_state;
	extensibility::event_t *_M_pSyncBlock;
	_CancellationTokenState *_M_pTokenState;
};



#endif /* INCLUDE_PPLX_PPLXCANCELLATION_TOKEN_H_ */
