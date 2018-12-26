/*
 * pplxtasks.h
 * https://github.com/Microsoft/cpprestsdk/blob/master/Release/include/pplx/pplxtasks.h
 *
 *  Created on: Nov 22, 2018
 *      Student (MIG Virtual Developer): Tung Dang
 */

#ifndef INCLUDE_PPLX_PPLXTASKS_H_
#define INCLUDE_PPLX_PPLXTASKS_H_

#if (defined(_MSC_VER) && (_MSC_VER >= 1800)) && !CPPREST_FORCE_PPLX
#include <ppltask.h>
namespace pplx = Concurrency;
#if (_MSC_VER >= 1900)
#include <concrt.h>
namespace Concurrency {
    namespace extensibility {
        typedef ::std::condition_variable condition_variable_t;
        typedef ::std::mutex critical_section_t;
        typedef ::std::unique_lock<::std::mutex> scoped_critical_section_t;
        
        typedef ::Concurrency::event event_t;
        typedef ::Concurrency::reader_writer_lock reader_writer_lock_t;
        typedef ::Concurrency::reader_writer_lock::scoped_lock scoped_rw_lock_t;
        typedef ::Concurrency::reader_writer_lock::scoped_lock_read scoped_read_lock_t;

        typedef ::Concurrency::details::_ReentrantBlockingLock recursive_lock_t;
        typedef recursive_lock_t::_Scoped_lock scoped_recursive_lock_t; 
    } 
}
#endif
#else
#include "pplx.h"
#if defined(__ANDROID__)
#include <jni.h>
void cpprest_init(JavaVM*);
#endif
#if defined(_MSC_VER)
#if _MSC_FULL_VER < 160040219 /*IFSTRIP=IGN*/
#error ERROR: Visual Studio 2010 SP1 or later is required to build ppltasks
#endif 
#endif 

#include <functional>
#include <vector>
#include <utility>
#include <exception>
#include <algorithm>

#if defined(_MSC_VER)
#include <intrin.h>
#if defined(__cplusplus_winrt)
#include <Windows.h>
#include <ctxtcall.h>
#include <agile.h>
#include <winapifamily.h>
#ifndef _UITHREADCTXT_SUPPORT
#ifdef WINAPI_FAMILY 
#include <winapifamily.h>
#if WINAPI_FAMILY == WINAPI_FAMILY_APP
#define _UITHREADCTXT_SUPPORT 0
#elif WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP
#define _UITHREADCTXT_SUPPORT 0
#else
#define _UITHREADCTXT_SUPPORT 1
#endif
#else
#define _UITHREADCTXT_SUPPORT 0
#endif /*WINAPI_FAMILY*/
#endif /*_UITHREADCTXT_SUPPORT*/

#if _UITHREADCTXT_SUPPORT
#include <uithreadctxt.h>
#endif
#pragma detect_mismatch("_PPLTASKS_WITH_WINRT", "1")
#else
#pragma detect_mismatch("_PPLTASKS_WITH_WINRT", "0")
#endif /*__cplusplus_winrt*/
#endif /*_MSC_VER*/

#ifdef _DEBUG
#define _DBG_ONLY(X) X 
#else
#define _DBG_ONLY(X)
#endif

#ifdef _MSC_VER
#if _MSC_VER < 1700
namespace std  
{
    template <class _E> exception_ptr make_exception_ptr(_E _Except) 
    {
        return copy_exception(_Except);
    }
}
#endif /*_MSC_VER < 1700*/
#ifndef _PPLTASK_ASYNC_LOGGING
#if _MSC_VER >= 1800 && defined(__cplusplus_winrt)
#define _PPLTASK_ASYNC_LOGGING 1
#else
#define _PPLTASK_ASYNC_LOGGING 0
#endif
#endif /*_PPLTASK_ASYNC_LOGGING*/
#endif /*_MSC_VER*/

#pragma pack(push, _CRT_PACKING)

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 28197)
#pragma warning(disable: 4100) // Unreferenced formal parameter - needed for document generation
#pragma warning(disable: 4127) // constant express in if condition - we use it for meta programming
#endif /*defined(_MSC_VER)*/

#pragma push_macro("new")
#undef new 

namespace stdx 
{
    template <class _T>
    _T&& declval();
}

namespace pplx 
{
    /*A type that represents the terminal state of a task.*/
    typedef task_group_status task_status;
    template <typename _Type> class task;
    template <> class task<void>;

    #ifndef PPL_TASK_SAVE_FRAME_COUNT
    #ifdef _DEBUG
    #define PPL_TASK_SAVE_FRAME_COUNT 10
    #else
    #define PPL_TASK_SAVE_FRAME_COUNT 1
    #endif
    #endif

    /**
     * Helper macro to determine how many stack frames need to be saved. 
     * When any number less of equal to 1 is specified, only one frame is captured and no stackwalk will be involved. 
     * The number of callstack frames will be captured. 
     * This need to be defined as macro rather than a function so that if we are only gathering one frame. 
    */
   #if PPL_TASK_SAVE_FRAME_COUNT > 1
   #if defined(__cplusplus_winrt) && !defined(_DEBUG)
   #pragma message ("WARNING: Redefinning PPL_TASK_SAVE_FRAME_COUNT under Release build 
                    for non-desktop applications is not supported; only one frame will be captured!")
    #define _CAPTURE_CALLSTACK() ::pplx::details::_TaskCreationCallstack::_CaptureSingleFrameCallstack(_ReturnAddress()) 
    #else
    #define _CAPTURE_CALLSTACK() ::pplx::details::_TaskCreationCallstack::_CaptureMultiFramesCallstack(PPL_TASK_SAVE_FRAME_COUNT)               
   #endif
   #else
   #define _CAPTURE_CALLSTACK() ::pplx::details::_TaskCreationCallstack::_CaptureSingleFrameCallstack(_ReturnAddress())
   #endif

   /**
    * Returns an indication of whether the task that is currently executing has received a request to cancel its execution. 
    * Cancellation is required on a task if the task was created with a cancellation token, 
    * and the token source associated with that token is cancelled. 
    * A task is not cancellable if it was created without a cancellation token. 
    * 
    * This method is called in the body of a task and it returns "true", you must repond with a call to "cancel_current_task" 
    * to acknowledge the cancellation request and to abort the execution of task, after performing any cleanup you need.
   */
  inline bool _pplx_cdecl is_task_cancellation_requested()
  {
      return ::pplx::details::_TaskCollection_t::_Is_cancellation_requested();
  }

  /**
   * Cancels the currently executing task.
   * - can be called from within the body of a task to abort the task's execution
   * - can be used in response to the "is_task_cancellation_requested" function or 
   *   can be used by itself to initiate cancellation of the task. 
  */
  inline __declspec(noreturn) void _pplx_cdecl cancel_current_task()
  {
      throw task_canceled(); 
  }

  namespace details
  {
      /**
       * Callstack container is used to capture and preserve callstacks in ppltasks.
       * Members of this class is examined by vc debugger, there will be no public access methods
      */
     class _TaskCreationCallstack
     {
         private:
         void *_M_SingleFrame;
         std::vector<void*> _M_frames;
         public:
         _TaskCreationCallstack()
         {
             _M_SingleFrame = nullptr;
         }

         // Store one of frame of callstack.
         static _TaskCreationCallstack _CaptureSingleFrameCallstack(void * _SingleFrame)
         {
             _TaskCreationCallstack _csc;
             _csc._M_SingleFrame = _SingleFrame;
             return _csc;
         }

         // Capture _CaptureFrames number of callstack frames. 
         __declspec(noinline)
         static _TaskCreationCallstack _CaptureMutilFrameCallstack(size_t _CaptureFrames)
         {
             _TaskCreationCallstack _csc;
             _csc._M_frames.resize(_CaptureFrames);
             _csc._M_frames.resize(::pplx::details::platform::CaptureCallstack(&_csc._M_frames[0], 2, _CaptureFrames));
             return _csc;
         }
     };
     typedef unsigned char _Unit_type;

     struct _TypeSelectorNoAsync {};
     struct _TypeSelectorAsyncOperationOrTask{};
     struct _TypeSelectorAsyncOperation : public _TypeSelectorAsyncOperationOrTask {};
     struct _TypeSelectorAsyncTask : public _TypeSelectorAsyncOperationOrTask {};
     struct _TypeSelectorAsyncAction {};
     struct _TypeSelectorAsyncActionWithProgress {};
     struct _TypeSelectorAsyncOperationWithProgress {};

     template <typename _Ty>
     struct _NormalizeVoidToUnitType
     {
         typedef _Ty _Type;
     };

     template <>
     struct _NormalizeVoidToUnitType<void>
     {
         typedef _Unit_type _Type;
     };

     template <typename _T>
     struct _IsUnwrappedAsyncSelector 
     {
         static const bool _Value = true;
     };

     template <>
     struct _IsUnwrappedAsyncSelector<_TypeSelectorNoAsync>
     {
         static const bool _Value = false;
     };

     template <typename _Ty>
     struct _UnwrapTaskType
     {
         typedef _Ty _Type;
     };

     template <typename _Ty>
     struct _UnwrapTaskType<task<_Ty>>
     {
         typedef _Ty _Type;
     };

     template <typename _T>
     _TypeSelectorAsyncTask _AsyncOperationKindSelector(task<_T>);

     _TypeSelectorAsyncTask _AsyncOperationKindSelector(...);

     #if defined(__cplusplus_winrt)
     template <typename _Type>
     struct _Unhat
     {
         typedef _Type _Value;
     };

     template <typename _Type>
     struct _Unhat<_Type^>
     {
         typedef _Type _Value;
     };

     value struct _NonUserType {public: int _Dummy; };

     template <typename _Type, bool _IsValueTypeOrRefType = __is_valid_winrt_type(_Type)>
     struct _ValueTypeOrRefType
     {
         typedef _NonUserType _Value;
     };

     template <typename _Type>
     struct _ValueTypeOrRefType<_Type, true>
     {
         typedef _Type _Value;
     };

     template <typename _T1, typename _T2>
     _T2 _ProgressTypeSelector(Window::Foundation::IAsyncOperationWithProgress<_T1,_T2>^);

     template <typename _T1>
     _T1 _ProgressTypeSelector(Window::Foundation::IAsyncOperationWithProgress<_T1>^);

     template <typename _Type>
     struct _GetProcessType 
     {
         typedef decltype(_ProcessTypeSelector(stdx::declval<_Type>())) _Value;
     };

     template <typename _Type>
     struct _IsIAsyncInfor
     {
         static const bool _Value = __is_base_of(Windows::Foundation::IAsyncInfo, typename _Unhat<_Type>::_Value);
     };

     template <typename _T>
     _TypeSelectorAsyncOperation _AsyncOperationKindSelector(Window::Foundation::IAsyncOperation<_T>^);
     _TypeSelectorAsyncAction _AsyncOperationKindSelector(Windows::Foundation::IAsyncAction^);

     template <typename _T1, typename _T2>
     _TypeSelectorAsyncOperationWithProgress _AsyncOperationKindSelector(Windows::Foundation::IAsyncOperationWithProgress<_T1, _T2>^);

     template <typename _T>
     _TypeSelectorAsyncActionWithProgress _AsyncOperationKindSelector(Windows::Foundation::IAsyncActionWithProgress<_T>^);

     template <typename _Type, bool _IsAsync = _IsIAsyncInfo<_Type>::_Value>
     struct _TaskTypeTraits
     {
         typedef typename _UnwrapTaskType<_Type>::_Type _TaskRetType;
         typedef decltype(_AsyncOperationKindSelector(stdx::declval<_Type>())) _AsyncKind;
         typedef typename _NormalizeVoidToUnitType<_TaskRetType>::_Type _NormalizedTaskRetType;

         static const bool _IsAsyncTask = _IsAsync;
         static const bool _IsUnwrappedTaskOrAsync = _IsUnwrappedAsyncSelector<_AsyncKind>::_Value;
     };

     template <typename _Type>
     struct _TaskTypeTraits<_Type, true>
     {
         typedef decltype(((_Type)nullptr)->GetResults()) _TaskRetType;
         typedef _TaskRetType _NormalizedTaskRetType;
         typedef decltype(_AsyncOperationKindSelector((_Type)nullptr)) _AsyncKind;
     };

     #else 
     template <typename _Type>
     struct _IsIAsyncInfo 
     {
         static const bool _Value = false;
     };

     template <typename _Type, bool _IsAsync = false>
     struct _TaskTypeTraits
     {
         typedef typename _UnwrapTaskType<_Type>::_Type _TaskRetType;
         typedef decltype(_AsyncOperationKindSelector(stdx::declval<_Type>())) _AsyncKind;
         typedef typename _NormalizeVoidToUintType<_TaskRetType>::_Type _NormalizedTaskRetType;

         static const bool _IsAsyncTask = false;
         static const bool _IsUnwrappedTaskOrAsync = _IsUnwrappedAsyncSelector<_AsyncKind>::_Value;
     };
     #endif

     template <typename _Function>
     auto _IsCallable(_Function _Func, int)->decltype(_Func(), std::true_type())
     {
         (void)(_Func);
         return std::true_type();
     }

     template <typename _Function>
     std::false_type _IsCallable(_Function, ...) 
     {
         return std::false_type();
     }

     template <>
     struct _TaskTypeTraits<void>
     {
         typedef void _TaskRetType;
         typedef _TypeSelectorNoAsync _AsyncKind;
         typedef _Unit_type _NormalizedTaskRetType;

         static const bool _IsAsyncTask = false;
         static const bool _IsUnwrappedTaskOrAsync = false;
     };

     template <typename _Type>
     task<_Type> _To_task(_Func f);

     struct _BadContinuationParamType{};

     template <typename _Function, typename _Type> 
     auto _ReturnTypeHelper(_Type t, _Function _Func, int, int) -> decltype(_Func(_To_task(t)));
     template <typename _Function, typename _Type>
     auto _ReturnTypeHelper(_Type t, _Function _Func, int, ...) -> decltype(_Func(t));
     template <typename _Function, typename _Type>
     auto _ReturnTypeHelper(_Type t, _Function _Func, ...) -> _BadContinuationParamType;

     template <typename _Function, typename _Type> 
     auto _IsTaskHelper(_Type t, _Function _Func, int, int) ->decltype(_Func(_To_task(t)), std::true_type());
     template <typename _Function, typename _Type>
     std::false_type _IsTaskHelper(_Type t, _Function _Func, int, ...);

     template <typename _Function>
     auto _VoidReturnTypeHelper(_Function _Func, int, int) -> decltype(_Func(_To_task_void(_Func)));
     template <typename _Function>
     auto _VoidReturnTypeHelper(_Function _Func, int, ...) -> decltype(_Func());

     template <typename _Function>
     auto _VoidIsTaskHelper(_Function _Func, int, int) -> decltype(_Func(_To_task_void(_Func)), std::true_type());
     template <typename _Function>
     std::false_type _VoidIsTaskHelper(_Function _Func, int, ...);

     template <typename _Function, typename _ExpectedParameterType>
     struct _FunctionTypeTraits
     {
         typedef decltype(_ReturnTypeHelper(stdx::declval<_ExpectedParameterType>(), stdx::declval<_Function>(), 0, 0)) _FuncRetType;
         typedef decltype(_IsTaskHelper(stdx::declval<_ExpectedParameterType>(), stdx::declval<_Function>(), 0, 0)) _Takes_task;
     };

     template <typename _Function>
     struct _FunctionTypeTraits<_Function, void>
     {
         typedef decltype(_VoidReturnTypeHelper(stdx::declval<_Function>(), 0, 0)) _FuncRetType;
         typedef decltype(_VoidIsTaskHelper(stdx::declval<_Function>(), 0, 0)) _Takes_task;
     };

     template <typename _Function, typename _ReturnType>
     struct _ContinuationTypeTraits
     {
         typedef task<typename _TaskTypeTraits<typename _FunctionTypeTraits<_Function, _ReturnType>::_FuncRetType>::_TaskRetType> _TaskOfType;
     };

     template <typename _TaskType, typename _FuncRetType>
     struct _InitFunctorTypeTraits
     {
         typedef typename _TaskTypeTraits<_FuncRetType>::_AsyncKind _AsyncKind;
         static const bool _IsAsyncTask = _TaskTypeTraits<_FuncRetType>::_IsAsyncTask;
         static const bool _IsUnwrappedTaskOrAsync = _TaskTypeTraits<_FuncRetType>::_IsUnwrappedTaskOrAsync;
     };

     template <typename T>
     struct _InitFunctorTypeTraits<T, T>
     {
         typedef _TypeSelectorNoAsync _AsyncKind;
         static const bool _IsAsyncTask = false;
         static const bool _IsUnwrappedTaskOrAsync = false;
     };

     struct _TaskProcThunk 
     {
         _TaskProcThunk(const std::function<void ()> &_Callback) : 
         _M_func(_Callback)
         {}

         static void _pplx_cdecl _Bridge(void* PData)
         {
             _TaskProcThunk *_PThunk = reinterpret_cast<_TaskProcThunk *>(_PData);
             _Holder _ThunkHolder(_PThunk);
             _PThunk->_M_func();
         }
         private:
         struct _Holder
         {
             _Holder(_TaskProcThunk *_PThunk) : _M_pThunk(_PThunk)
             {}

             ~Holder()
             {
                 delete _M_pThunk;
             }

             _TaskProcThunk * _M_pThunk;
             private:
             _Holder& operator=(const _Holder&);
         };
         std::function<void()> _M_func;
         _TaskProcThunk& operator=(const _TaskProcThunk&);
     };

     /**
      * Schedule a function with automatic inlining. 
      * Note that this is "fire and forget" scheduling, which cannot be waited on or canceled after scheduling
     */
    static void _ScheduleFuncWithAutoInline(const std::function<void ()> & _Func, _TaskInliningMode_t _InliningMode)
    {
        _TaskCollection_t::_RunTask(&_TaskProcThunk::_Bridge, new _TaskProcThunk(_Func), _InliningMode);
    }

    class _ContextCallback
    {
        typedef std::function<void(void)> _CallbackFunction;
        #if defined(__cplusplus_winrt)
        public:
        static _ContextCallback _CaptureCurrent()
        {
            _ContextCallback _Context;
            _Context._Capture();
            return _Context;
        }

        ~_ContextCallback()
        {
            _Reset();
        }

        _ContextCallback(bool _DeferCapture = false)
        {
            if (_DeferCapture)
            {
                _M_context._M_captureMethod = _S_captureDeferred;
            }
            else
            {
                _M_context._M_pContextCallback = nullptr;
            }
        }

        void _Resolve(bool _CaptureCurrent)
        {
            if (_M_context._M_captureMethod == _S_captureDeferred)
            {
                _M_context._M_pContextCallback = nullptr;
                if (_CaptureCurrent)
                {
                    if (_IsCurrentOriginSTA())
                    {
                        _Capture();
                    }
                    #if _UITHREADCTXT_SUPPORT
                    else
                    {
                        HRESULT _Hr = CaptureUiThreadContext(&_M_context._M_pContextCallback);
                        if (FAILED(_Hr))
                        {
                            _M_context._M_pContextCallback = nullptr;
                        }
                    }
                    #endif
                }
            }
        }

        void _Capture()
        {
            HRESULT _Hr = CoGetObjectContext(IID_IContextCallback, reinterpret_cast<void**>(&_M_context._M_pContextCallback));
            if (FAILED(_Hr))
            {
                _M_context._M_pContextCallback = nullptr;
            }
        }

        _ContextCallback(const _ContextCallback& _Src)
        {
            _Assign(_Src._M_context._M_pContextCallback);
        }

        _ContextCallback(_ContextCallback&& _Src)
        {
            _M_context._M_pContextCallback = _Src._M_context._M_pContextCallback;
            _Src._M_context._M_pContextCallback = nullptr;
        }

        _ContextCallback& operator=(const _ContextCallback& _Src)
        {
            if (this != &_Src)
            {
                _Reset();
                _Assign(_Src._M_context._M_pContextCallback);
            }
            return *this;
        }

        _ContextCallback& operator=(_ContextCallback&& _Src)
        {
            if (this != &_Src)
            {
                _M_context._M_pContextCallback = _Src._M_context._M_pContextCallback;
                _Src._M_context._M_pContextCallback = nullptr;
            }
            return *this;
        }

        bool _HasCapturedContext() const 
        {
            _ASSERTE(_M_context._M_captureMethod != _S_captureDeferred);
            return (_M_context._M_pContextCallback != nullptr);
        }

        void _CallInContext(_CallbackFunction _Func) const
        {
            if (!_HasCapturedContext())
            {
                _Func();
            }
            else
            {
                ComCallData callData;
                ZeroMemory(&callData, sizeof(callData));
                callData.pUserDefined = reinterpret_cast<void *>(&_Func);

                HRESULT _Hr = _M_context._M_pContextCallback->_ContextCallback(&_Bridge, &callData, 
                IID_ICallbackWithNoReentrancyToApplicationSTA, 5, nullptr);
                if (FAILED(_Hr))
                {
                    throw ::platform::exception::CreateEXception(_Hr);
                }
            }
        }

        bool operator==(const _ContextCallback& _Rhs) const 
        {
            return (_M_context._M_pContextCallback == _Rhs._M_context._M_pContextCallback);
        }

        bool operator!=(const _ContextCallback& _Rhs) const 
        {
            return !(operator==(_Rhs));
        }

        private:
        void _Reset()
        {
            if (_M_context._M_captureMethod != _S_captureDeferred && _M_context._M_pContextCallback != nullptr)
            {
                _M_context._M_pContextCallback->Release();
            }
        }

        void _Assign(IContextCallback *_PContextCallback)
        {
            _M_context._M_pContextCallback = _PContextCallback;
            if (_M_context._M_captureMethod != _S_captureDeferred && _M_context._M_pContextCallback != nullptr)
            {
                _M_context._M_pContextCallback->AddRef();
            }
        }

        static HRESULT _stdcall _Bridge(ComCallData *_PParam)
        {
            _CallbackFunction *pFunc = reinterpret_cast<_CallbackFunction *>(_PParam->pUserDefined);
            (*pFunc)();
            return S_OK;
        }

        /* Return the origin information for the caller*/
        static bool _IsCurrentOriginSTA()
        {
            APTTYPE _AptType;
            APTTYPEQUALIFIER _AptTypeQualifier;

            HRESULT hr = CoGetApartmentType(&_AptType, &_AptTypeQualifier);
            if (SUCCEEDED(hr))
            {
                switch(_AptType)
                {
                    case APTTYPE_MAINSTA:
                    case APTTYPE_STA: 
                    return true;
                    default: 
                    break;
                }
            }
            return false;
        }

        union
        {
            IContextCallback *_M_pContextCallback;
            size_t _M_captureMethod;
        } _M_context;

        static const size_t _S_captureDeferred = 1;
        #else
        public:
        static _ContextCallback _CaptureCurrent()
        {
            return _ContextCallback();
        }
        _ContextCallback(bool = false)
        {}
        _ContextCallback(const _ContextCallback&)
        {}
        _ContextCallback(_ContextCallback&&)
        {}
        _ContextCallback& operator=(const _ContextCallback&)
        {
            return *this;
        }
        _ContextCallback& operator=(_ContextCallback&&)
        {
            return *this;
        }
        bool _HashCaptureContext() const 
        {
            return false;
        }
        void _Resolve(bool) const 
        {}
        void _CallInContext(_CallbackFunction _Func) const 
        {
            _Func();
        }
        bool operator==(const _ContextCallback&) const 
        {
            return true;
        }
        bool operator!=(const _ContextCallback&) const 
        {
            return false;
        }
        #endif
    };

    template <typename _Type>
    struct _ResultHolder
    {
        void Set(const _Type& _type)
        {
            _Result = _type;
        }
        _Type Get()
        {
            return _Result;
        }
        _Type _Result; 
    };

    #if defined (__cplusplus_winrt)
    template <typename _Type>
    struct _ResultHolder<_Type^>
    {
        void Set(_Type^ const &_type)
        {
            _M_Result = _type;
        }

        _Type^ Get()
        {
            return _M_Result.Get();
        }

        private:
        ::Platform::Agile<_Type^> _M_Result;
    };

    /**
     * The below are for composability with tasks auto-created from when_any 
    */
    template <typename _Type>
    struct _ResultHolder<std::vector<_Type^>>
    {
        void Set(const std::vector<_Type^>&_type)
        {
            _Result.reserve(_type.size());
            for (auto _PTask = _type.begin(); _PTask != _type.end(); ++_PTask)
            {
                _Result.emplace_back(*_PTask);
            }
        }

        std::vector<_Type^> Get()
        {
            std::vector<_Type^> _Return;
            _Return.reverse(_Result.size());
            for (auto _PTask = _Result.begin(); _PTask != _Result.end(); ++_PTask)
            {
                _Return.push_back(_PTask->Get());
            }
            return _Return;
        }

        std::vector<::Platform::Agile<_Type^>> _Result;
    };

    template <typename _Type>
    struct _ResultHolder<std::pair<_Type^, void*> >
    {
        void Set(const std::pair<_Type*, size_t>&_type)
        {
            _M_Result = _type;
        }

        std::pair<_Type^, size_t> Get()
        {
            return std::make_pair(_M_Result.first.Get(), _M_Result.second());
        }
        private:
        std::pair< ::Platform::Agile<_Type^>, size_t> M_Result;
    };
    #endif
      
    /**
     * An exception thrown by the task body is captured in an exception holder 
     * and it is shared with all value based continuations rooted at the task. 
     * 
     * The exception is observed if the user invokes get()/wait() on any of the tasks that are sharing this exception holder.
     * If the exception is not observed by the time the internal object owned by the shared pointer destructs, the process will fail.
     */
    struct _ExceptionHolder 
    {
        private: 
        void ReportUnhandleError()
        {
            #if _MSC_VER >= 1800 && defined(__cplusplus_winrt)
            if (_M_winRTException != nullptr)
            {
                ::Platform::Details::ReportUnhandleError(_M_winRTException);
            }
            #endif
        }
        public: 
        explicit _ExceptionHolder(const std::exception_ptr& _E, const _TaskCreationCallstack &_stackTrace) : 
            _M_exceptionObserved(0), _M_stdException(_E), _M_stackTrace(_stackTrace) 
        #if defined(__cplusplus_winrt)
            , _M_winRTException(nullptr)
        #endif    
        {}

        #if defined (__cplusplus_winrt)
        explicit _ExceptionHolder(::Platform::Exception^ _E, const _TaskCreationCallstack &_stackTrace) : 
            _M_exceptionObserved(0), _M_winRTException(_E), _M_stackTrace(_stackTrace)
            {}
        #endif

        __declspec(noinline)
        ~_ExceptionHolder()
        {
            /**
             * If you are trapped here, it means an exception thrown in task chain didn't get handled.
             * Please add task-based continuation to handle all exceptions coming from tasks.
            */
           if (_M_exceptionObserved == 0)
           {
               _REPORT_PPLTASK_UNOBSERVED_EXCEPTION();
           }
        }

        void _RethrowUserException()
        {
            if (_M_exceptionObserved == 0)
            {
                atomic_exchange(_M_exceptionObserved, 1l);
            }

        #if defined(__cplusplus_winrt)
            if (_M_winRTException != nullptr)
            {
                throw _M_winRTException;
            }
        #endif
            std::rethrow_exception(_M_stdException);
        }

        /**
         * A variable that remembers if this exception was every rethrown into user code. 
         * Exceptions are unobserved when the exception holder is destructed will terminate the process.
        */
       atomic_long _M_exceptionObserved;

       /**
        * Either _M_stdException or _M_winRTException is populated based on the type of exception encoutered 
       */
      std::exception_ptr _M_stdException;
      #if defined (__cplusplus_winrt)
      ::Platform::Exception^ _M_winRTException;
      #endif

      _TaskCreationCallstack _M_stackTrace;
    };  
    
    /*******************************************************************************
     ************************ Converting Operations ********************************* 
    ********************************************************************************/
    #if defined(__cplusplus_winrt)
    /**
     * Base convert class for converting asynchronous interfaces to IAsyncOperation
    */
    template <typename _AsyncOperationType, typename _CompletionHandlerType, typename _Result>
    ref struct _AsyncInfoImpl abstract : Windows::Foundation::IAsyncOperation<_Result>
    {
        internal : 
        /**
         * The async action, action with progress or operation with progress that this stub forward to
        */
        ::Platform::Agile<_AsyncOperationType>
            _M_asyncInfo;

        Windows::Foundation::AsyncOperationCompleteHandler<_Result> ^ _M_CompletedHandler;
        _AsyncInfoImpl(_AsyncOperationType _AsyncInfo) : _M_asyncInfo(_AsyncInfo) {}  

        public: 
        virtual void Cancel() 
        {
            _M_asyncInfo.Get()->Cancel();
        }  
        virtual void Close() 
        {
            _M_asyncInfo.Get()->Close();
        }

        virtual property Windows::Foundation::HResult ErrorCode
        {
            Windows::Foundation::HResult get() {return _M_asyncInfo.Get()->ErrorCode;}
        }

        virtual property UINT Id  
        {
            UINT get() {return _M_asyncInfo.Get()->Id;}
        }

        virtual property Windows::Foundation::AsyncStatus Status 
        {
            Windows::Foundation::AsyncStatus get() {return _M_asyncInfo.Get()->Status;}
        }

        virtual _Result GetResults() 
        {
            throw std::runtime_error("derived class must implement");
        }

        virtual property Windows::Foundation::AsyncOperationCompletedHandler<_Result> ^ Completed {
            Windows::Foundation::AsyncOperationCompletedHandler<_Result> ^ get() {return _M_CompletedHandler;}
            void set(Windows::Foundation::AsyncOperationCompletedHandler<_Result> ^ value)
            {
                _M_CompletedHandler = value;
                _M_asyncInfo.Get()->Completed = 
                    ref new _CompletionHandlerType([&](_AsyncOperationType, Windows::Foundation::AsyncStatus status) {
                        _M_CompletedHandler->Invoke(this, status);
                    });
            }
        }
    };

    /**
     * Class is used to convert an instance of IAsyncOperationWithProgress into IAsyncOperation
    */
    template <typename _Result, typename _Progress>
    ref struct _IAsyncOperationWithProgressToAsyncOperationConverter sealed 
        : _AsyncInfoImpl<Windows::Foundation::IAsyncOperationWithProgress<_Result,_Progress> ^ 
        , Windows::Foundation::AsyncOperationWithProgressCompletedHandler<_Result, _Progress>, _Result>
    {
        internal : _IAsyncOperationWithProgressToAsyncOperationConverter(
            Windows::Foundation::IAsyncOperationWithProgress<_Result, _Progress> ^ _Operation)
        : _AsyncInfoImpl<Windows::Foundation::IAsyncOperationWithProgress<_Result, _Progress> ^, 
           Windows::Foundation::AsyncOperationWithProgressCompletedHandler<_Result, _Progress> ,
           _Result>(_Operation)
        {}

        public: 
        virtual _Result GetResults() override {return _M_asyncInfo.Get()->GetResults();}  
    };

    /**
     * Class is used to convert an instance of IAsyncAction into IAsyncOperation<_Uint_type>
    */
    ref struct _IAsyncActionToAsyncOperationConverter sealed 
        : _AsyncInfoImpl<Windows::Foundation::IAsyncAction ^ 
                    , Windows::Foundation::AsyncActionCompletedHandler, details::_Unit_type>
    {
        internal : _IAsyncActionToAsyncOperationConverter(Windows::Foundation::IAsyncAction ^ _Operation) 
            : _AsyncInfoImpl<Windows::Foundation::IAsyncAction ^ 
            , Windows::Foundation::AsyncActionCompletedHandler, details::_Unit_type>(_Operation)
        {}

        public: 
        virtual details::_Unit_type GetResults() override 
        {
            _M_asyncInfo.Get()->GetResults();
            return details::_Unit_type();
        }
    };
    #endif  
  }

}

#endif
#endif /* INCLUDE_PPLX_PPLXTASKS_H_ */
