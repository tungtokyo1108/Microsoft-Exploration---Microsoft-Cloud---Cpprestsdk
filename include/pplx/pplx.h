/*
 * pplx.h
 * https://github.com/Microsoft/cpprestsdk/blob/master/Release/include/pplx/pplx.h
 *
 *  Created on: Nov 21, 2018
 *      Student (MIG Virtual Developer): Tung Dang
 */

#ifndef INCLUDE_PPLX_PPLX_H_
#define INCLUDE_PPLX_PPLX_H_

#ifndef _PPLX_H
#define _PPLX_H

#ifndef _WIN32
#if defined(_WIN32) || defined(__cplusplus_winrt)
#define _WIN32
#endif
#endif

#ifdef _NO_PPLXIMP
#define _PPLXIMP
#else
#ifdef _PPLX_EXPORT
#define _PPLXIMP __declspec(dllexport)
#else
#define _PPLXIMP __declspec(dllimport)
#endif
#endif

#include "cpprest/details/cpprest_compat.h"

// Use PPLx
#ifdef _WIN32
#include "pplx/pplxwin.h"
#elif defined(__APPLE__)
#undef _PPLXIMP
#define _PPLXIMP
#include "pplx/pplxlinux.h"
#else
#include "pplx/pplxlinux.h"
#endif // _WIN32

// Common implementation across all the non-concrt versions
#include "pplx/pplxcancellation_token.h"
#include <functional>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4127)
#endif

#pragma pack(push,_CRT_PACKING)

namespace pplx 
{
    _PPLXIMP void _pplx_cdecl set_ambient_scheduler(std::shared_ptr<pplx::scheduler_interface> _Scheduler);
    _PPLXIMP std::shared_ptr<pplx::scheduler_interface> _pplx_cdecl get_ambient_scheduler();

namespace details 
{

class _Interruption_exception : public std::exception
{
    public:
    _Interruption_exception() {}
};

template<typename _T>
struct _AutoDeleter
{
    _AutoDeleter(_T *_PPtr) : _Ptr(_PPtr) {}
    ~_AutoDeleter() {delete _Ptr;}
    _T *_Ptr;
};

struct _TaskProcHandle
{
    _TaskProcHandle() 
    {}

    virtual ~_TaskProcHandle() {}
    virtual void invoke() const = 0;

    static void _pplx_cdecl _RunChoreBridge(void * _Parameter)
    {
        auto _PTaskHandle = static_cast<_TaskProcHandle *>(_Parameter);
        _AutoDeleter<_TaskProcHandle> _AutoDeleter(_PTaskHandle);
        _PTaskHandle->invoke();
    }
};

enum _TaskInliningMode
{
    _NoInline = 0,
    _DefaultAutoInline = 16,
    _ForceInline = -1,
};

class _TaskCollectionImpl
{
    public:
    typedef _TaskProcHandle _TaskProcHandle_t;
    _TaskCollectionImpl(scheduler_ptr _PScheduler) 
        : _M_pScheduler(_PScheduler)
    {}

    void _ScheduleTask(_TaskProcHandle_t* _PTaskHandle, _TaskInliningMode _InliningMode)
    {
        if (_InliningMode == _ForceInline)
        {
            _TaskProcHandle_t::_RunChoreBridge(_PTaskHandle);
        }
        else 
        {
            _M_pScheduler->schedule(_TaskProcHandle_t::_RunChoreBridge, _PTaskHandle);
        }
    }

    void _Cancel()
    {
    }

    void _RunAndWait()
    {
        _Wait();
    }

    void _Wait()
    {
        _M_Completed.wait();
    }

    void _Complete()
    {
        _M_Completed.set();
    }

    scheduler_ptr _GetScheduler() const 
    {
        return _M_pScheduler;
    }

    static void _RunTask(TaskProc_t _Proc, void* _Parameter, _TaskInliningMode _InliningMode)
    {
        if (_InliningMode == _ForceInline)
        {
            _Proc(_Parameter);
        }
        else
        {
            get_ambient_scheduler()->schedule(_Proc, _Parameter);
        }
    }

    static bool _pplx_cdecl _Is_cancellation_requested()
    {
        return false;
    }

    private:
    extensibility::event_t _M_Completed;
    scheduler_ptr _M_pScheduler;
};

struct _Task_generator_oversubscriber {};

typedef _TaskCollectionImpl _TaskCollection_t;
typedef _TaskInliningMode _TaskInliningMode_t;
typedef _Task_generator_oversubscriber _Task_generator_oversubscriber_t;

}
}
#endif
#endif /* INCLUDE_PPLX_PPLX_H_ */
