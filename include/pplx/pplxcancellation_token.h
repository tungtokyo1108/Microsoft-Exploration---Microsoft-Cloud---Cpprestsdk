/*
 * pplxcancellation_token.h
 * https://github.com/Microsoft/cpprestsdk/blob/master/Release/include/pplx/pplxcancellation_token.h
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

template <typename _Function>
class _CancellationTokenCallback : public _CancellationTokenRegistration
{
public:
	_CancellationTokenCallback(const _Function& _Func) :
		_M_function(_Func)
    {
    }

protected:
	virtual void _Exec()
	{
	    _M_function();
	}

private:
	_Function _M_function;
};

class CancellationTokenRegistration_TaskProc : public _CancellationTokenRegistration
{
public:
	CancellationTokenRegistration_TaskProc(TaskProc_t proc, _In_ void *pData, int initialRefs) :
		_CancellationTokenRegistration(initialRefs), m_proc(proc), m_pData(pData)
    {
    }

protected:
	virtual void _Exec()
	{
		m_proc(m_pData);
	}

private:
	TaskProc_t m_proc;
	void *m_pData;
};

class _CancellationTokenState : public _RefCounter
{
protected:
	class TokenRegistrationContainer
	{
	private:
		typedef struct _Node {
			_CancellationTokenRegistration * _M_token;
			_Node *_M_next;
		} Node;

	public:
		TokenRegistrationContainer() : _M_begin(nullptr), _M_last(nullptr)
	    {}

		~TokenRegistrationContainer()
		{
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 6001)
#endif

			auto node = _M_begin;
			while (node != nullptr)
			{
				Node* tmp = node;
				node = node->_M_next;
				::free(tmp);
			}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
		}

		void swap(TokenRegistrationContainer& list)
		{
			std::swap(list._M_begin, _M_begin);
			std::swap(list._M_last, _M_last);
		}

		bool empty()
		{
			return _M_begin == nullptr;
		}

		template <typename T>
		void for_each(T lamda)
		{
			Node* node = _M_begin;
			while (node != nullptr)
			{
				lamda(node->_M_token);
				node = node->_M_next;
			}
		}

		void push_back(_CancellationTokenRegistration *token)
		{
			Node *node = reinterpret_cast<Node*>(::malloc(sizeof(Node)));
			if (node == nullptr)
			{
				throw ::std::bad_alloc();
			}

			node->_M_token = token;
			node->_M_next = nullptr;

			if (_M_begin == nullptr)
			{
				_M_begin = node;
			}
			else
			{
				_M_last->_M_next = node;
			}

			_M_last = node;
		}

		void remove(_CancellationTokenRegistration *token)
		{
			Node *node = _M_begin;
			Node *prev = nullptr;

			while (node != nullptr)
			{
				if(node->_M_token == token)
				{
					if (prev == nullptr)
					{
						_M_begin = node->_M_next;
					}
					else
					{
						prev->_M_next = node->_M_next;
					}

					if (node->_M_next == nullptr)
					{
						_M_last = prev;
					}

					::free(node);
					break;
				}

				prev = node;
				node = node->_M_next;
			}
		}

	private:
		Node *_M_begin;
		Node *_M_last;
	};

public:

	static _CancellationTokenState *_NewTokenState()
	{
		return new _CancellationTokenState();
	}

	static _CancellationTokenState *_None()
	{
		return reinterpret_cast<_CancellationTokenState *>(2);
	}

	static bool _IsValid(_In_opt_ _CancellationTokenState *_PToken)
	{
		return (_PToken != NULL && _PToken != _None());
	}

	_CancellationTokenState() :
		_M_stateFlag(0)
	{}

	~_CancellationTokenState()
	{
		TokenRegistrationContainer rundownList;
		{
			extensibility::scoped_critical_section_t _Lock(_M_listLock);
			_M_registreations.swap(rundownList);
		}

		rundownList.for_each([](_CancellationTokenRegistration * pRegistration)
		{
			pRegistration->_M_state = _CancellationTokenRegistration::_STATE_SYNCHRONIZE;
			pRegistration->_Release();
		});
	}

	bool _IsCanceled() const
	{
		return (_M_stateFlag != 0);
	}

	void _Cancel()
	{
		if (atomic_compare_exchange(_M_stateFlag, 1l, 01) == 0)
		{
			TokenRegistrationContainer rundownList;
			{
				extensibility::scoped_critical_section_t _Lock(_M_listLock);
				_M_registrations.swap(rundownList);
			}

			rundownList.for_each([](_CancellationTokenRegistration * pRegistration)
			{
				pRegistration->_Invoke();
			});

			_M_stateFlag = 2;
			_M_cancelComplete.set();
		}
	}

	_CancellationTokenRegistration *_RegisterCallback(TaskProc_t _PCallback, _In_ void *_PData, int _InitialRefs = 1)
	{
		_CancellationTokenRegistration *pRegistration = new CancellationTokenRegistration_TaskProc(_PCallback, _PData, _InitialRefs);
		_RegisterCallback(pRegistration);
		return pRegistration;
	}

	void _RegisterCallback(_In_ _CancellationTokenRegistration *_PRegistration)
	{
		_PRegistration->_M_state = _CancellationTokenRegistration::_STATE_CLEAR;
		_PRegistration->_Reference();
		_PRegistration->_M_pTokenState = this;

		bool invoke = true;
		if (!_IsCanceled())
		{
			extensibility::scoped_critical_section_t _Lock(_M_listLock);
			if (!_IsCanceled())
			{
				invoke = false;
				_M_registrations.push_back(_PRegistration);
			}
		}

		if (invoke)
		{
			_PRegistration->_Invoke();
		}
	}

	void _DeregisterCallback(_In_ _CancellationTokenRegistration *_PRegistration)
	{
		bool synchronize = false;
		{
			extensibility::scoped_critical_section_t _Lock(_M_listLock);
			if (!_M_registrations.empty())
			{
				_M_registrations.remove(_PRegistration);
				_PRegistration->_M_state = _CancellationTokenRegistration::_STATE_SYNCHRONIZE;
				_PRegistration->_Release();
			}
			else
			{
				synchronize = true;
			}
		}

		if (synchronize)
		{
			long result = atomic_compare_exchange(
				_PRegistration->_M_state,
				_CancellationTokenRegistration::_STATE_DEFER_DELETE,
				_CancellationTokenRegistration::_STATE_CLEAR
			);

			switch(result)
			{
			case _CancellationTokenRegistration::_STATE_CLEAR:
			case _CancellationTokenRegistration::_STATE_CALLED:
				break;
			case _CancellationTokenRegistration::_STATE_DEFER_DELETE:
			case _CancellationTokenRegistration::_STATE_SYNCHRONIZE:
				_ASSERTE(false);
				break;
			default:
			{
				long tid = result;
				if (tid == ::pplx::details::platfrom::GetCurrentThreadId())
				{
					break;
				}

				extensibility::event_t ev;
				_PRegistration->_M_pSynchBlock = &ev;
				long result_1 = atomic_exchange(_PRegistration->_M_state, _CancellationTokenRegistration::_STATE_SYNCHRONIZE);
				if (result_1 != _CancellationTokenRegistration::_STATE_CALLED)
				{
					_PRegistration->_M_pSyncBlock->wait(::pplx::extensibility::event_t::timeout_infinite);
				}
				break;
			}
			}
		}
	}

private:
	atomic_long _M_stateFlag;
	extensibility::event_t _M_cancelComplete;
	extensibility::critical_section_t _M_listLock;
	TokenRegistrationContainer _M_registrations;
};
}

class cancellation_token_source;
class cancellation_token;

/*
 * The "cancellation_token_registration" class represents a callback notification from a "cancellation_token" class.
 * When the "register" method on a "cancellation_token" is used to receive notification of when cancellation occurs,
 * a "cancellation_token_registation" object is returned as a handle to the callback
 * so that caller can request a specific callback avoid using the "deregister" method.
 */

class cancellation_token_registration
{
public:
	cancellation_token_registration():
		_M_pRegistration(NULL)
    {
    }

	~cancellation_token_registration()
	{
		_Clear();
	}

	cancellation_token_registration(const cancellation_token_registration& _Src)
	{
		_Assign(_Src._M_pRegistration);
	}

	cancellation_token_registration(cancellation_token_registration&& _Src)
	{
		_Move(_Src._M_pRegistration);
	}

	cancellation_token_registration& operator=(const cancellation_token_registration& _Src)
	{
		if (this != &_Src)
		{
			_Clear();
			_Assign(_Src._M_pRegistration);
		}
		return *this;
	}

	cancellation_token_registration& operator=(cancellation_token_registration&& _Src)
	{
		if (this != &_Src)
		{
			_Clear();
			_Move(_Src._M_pRegistration);
		}
		return *this;
	}

	bool operator==(const cancellation_token_registration& _Rhs) const
	{
		return _M_pRegistration == _Rhs._M_pRegistration;
	}

	bool operator!=(const cancellation_token_registration& _Rhs) const
	{
		return !(operator==(_Rhs));
	}

private:
	friend class cancellation_token;
	cancellation_token_registration(_In_ details::_CancellationTokenRegistration *_PRegistration) :
		_M_pRegistration(_PRegistration)
	{}

	void _Clear()
	{
		if (_M_pRegistration != NULL)
		{
			_M_pRegistration->_Release();
		}
		_M_pRegistration = NULL;
	}

	void _Assign(_In_ details::_CancellationTokenRegistration *_PRegistration)
	{
		if (_PRegistration != NULL)
		{
			_PRegistration->_Reference();
		}
		_M_pRegistration = _PRegistration;
	}

	void _Move(_In_ details::_CancellationTokenRegistration *&_pRegistration)
	{
		_M_pRegistration = _PRegistration;
		_PRegistration = NULL;
	}

	details::_CancellationTokenRegistration *_M_pRegistration;
};

/*
 * This class represents the ability to determine whether some operation has been requested to cancel.
 * A given token can be associated with a "task_group", "structed_task_group", "task" groups to
 * provide implicit cancellation.
 */

class cancellation_token
{
public:
	typedef details::_CancellationTokenState* _ImplType;

	static cancellation_token none()
	{
		return cancellation_token();
	}

	cancellation_token(const cancellation_token& _Src)
	{
		_Assign(_Src._M_Impl);
	}

	cancellation_token(cancellation_token&& _Src)
	{
		_Move(_Src._M_Impl);
	}

	cancellation_token& operator=(const cancellation_token& _Src)
	{
		if (this != &_Src)
		{
			_Clear();
			_Assign(_Src._M_Impl);
		}
		return *this;
	}

	cancellation_token& operator=(cancellation_token&& _Src)
	{
		if (this != &_Src)
		{
			_Clear();
			_Move(_Src._M_Impl);
		}
		return *this;
	}

	bool operator==(const cancellation_token& _Src) const
	{
		return _M_Impl == _Src._M_Impl;
	}

	bool operator!=(const cancellation_token& _Src) const
	{
		return !(operator==(_Src));
	}

	~cancellation_token()
	{
		_Clear();
	}

	bool is_cancelable() const
	{
		return (_M_Impl != NULL);
	}

	bool is_canceled() const
	{
		return (_M_Impl != NULL && _M_Impl->_IsCanceled());
	}

	template <typename _Function>
	::pplx::cancellation_token_registration register_callback(const _Function& _Func) const
	{
		if (_M_Impl == NULL)
		{
			throw invalid_operation();
		}
#if defined(_MSC_VER)
#pragma warning(suppress: 28197)
#endif
		details::_CancellationTokenCallback<_Function> *_PCallback = new details::_CancellationTokenCallback<_Function>(_Func);
		_M_Impl->_RegisterCallback(_PCallback);
		return cancellation_token_registration(_PCallback);
	}

	void deregister_callback(const cancellation_token_registration& _Registration) const
	{
		_M_Impl->_DeregisterCallback(_Registration._M_pRegistration);
	}

	_ImplType _GetImpl() const
	{
		return _M_Impl;
	}

	_ImplType _GetImplValue() const
	{
		return (_M_Impl == NULL) ? ::pplx::details::_CancellationTokenState::_None() : _M_Impl;
	}

	static cancellation_token _FromImpl(_ImplType _Impl)
	{
		return cancellation_token(_Impl);
	}

private:
	friend class cancellation_token_source;

	_ImplType _M_Impl;
	void _Clear()
	{
		if (_M_Impl != NULL)
		{
			_M_Impl->_Release();
		}
		_M_Impl = NULL;
	}

	void _Assign(_ImplType _Impl)
	{
		if (_Impl != NULL)
		{
			_Impl->_Reference();
		}
		_M_Impl = _Impl;
	}

	void _Move(_ImplType &_Impl)
	{
		_M_Impl = _Impl;
		_Impl = NULL;
	}

	cancellation_token() :
		_M_Impl(NULL)
	{}

	cancellation_token(_ImplType _Impl) :
		_M_Impl(_Impl)
	{
		if (_M_Impl == ::pplx::details::_CancellationTokenState::_None())
		{
			_M_Impl = NULL;
		}

		if (_M_Impl != NULL)
		{
			_M_Impl->_Reference();
		}
	}
};

class cancellation_token_source
{
public:
	typedef ::pplx::details::_CancellationTokenState * _ImplType;

	cancellation_token_source()
	{
		_M_Impl = new ::pplx::details::_CancellationTokenState;
	}

	cancellation_token_source(const cancellation_token_source& _Src)
	{
		_Assign(_Src._M_Impl);
	}

	cancellation_token_source(cancellation_token_source&& _Src)
	{
		_Move(_Src._M_Impl);
	}

	cancellation_token_source& operator=(const cancellation_token_source& _Src)
	{
		if (this != &_Src)
		{
			_Clear();
			_Assign(_Src._M_Impl);
		}
		return *this;
	}

	cancellation_token_source& operator=(cancellation_token_source&& _Src)
	{
		if (this != &_Src)
		{
			_Clear();
			_Move(_Src._M_Impl);
		}
		return *this;
	}

	bool operator==(const cancellation_token_source& _Src) const
	{
		return _M_Impl == _Src._M_Impl;
	}

	bool operator!=(const cancellation_token_source& _Src) const
	{
		return !(operator==(_Src));
	}

	~cancellation_token_source()
	{
		if (_M_Impl != NULL)
		{
			_M_Impl->_Release();
		}
	}

	cancellation_token get_token() const
	{
		return cancellation_token(_M_Impl);
	}

	static cancellation_token_source create_linked_source(cancellation_token& _Src)
	{
		cancellation_token_source newSource;
		_Src.register_callback([newSource](){newSource.cancel();});
		return newSource;
	}

	template<typename _Iter>
	static cancellation_token_source create_linked_source(_Iter _Begin, _Iter _End)
	{
		cancellation_token_source newSource;
		for (_Iter _It = _Begin; _It != _End; ++_It)
		{
			_It->register_callback([newSource](){newSource.cancel();});
		}
		return newSource;
	}

	void cancel() const
	{
		_M_Impl->_Cancel();
	}

	_ImplType _GetImpl() const
	{
		return _M_Impl;
	}

	static cancellation_token_source _FromImpl(_ImplType _Impl)
	{
		return cancellation_token_source(_Impl);
	}

private:
	_ImplType _M_Impl;

	void _Clear()
	{
		if (_M_Impl != NULL)
		{
			_M_Impl->_Release();
		}
		_M_Impl = NULL;
	}

	void _Assign(_ImplType _Impl)
	{
		if (_Impl != NULL)
		{
			_Impl->_Reference();
		}
		_M_Impl = _Impl;
	}

	void _Move(_ImplType &_Impl)
	{
		_M_Impl = _Impl;
		_Impl = NULL;
	}

	cancellation_token_source(_ImplType _Impl) :
		_M_Impl(_Impl)
	{
		if (_M_Impl == ::pplx::details::_CancellationTokenState::_None())
		{
			_M_Impl = NULL;
		}

		if (_M_Impl != NULL)
		{
			_M_Impl->_Reference();
		}
	}
};

}



#endif /* INCLUDE_PPLX_PPLXCANCELLATION_TOKEN_H_ */
