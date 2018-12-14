/*
 * pplxtasks.h
 * https://github.com/PowerShell/cpprestsdk/blob/dev/Release/include/pplx/pplxtasks.h
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

     
     #endif
  }  

}

#endif

#endif /* INCLUDE_PPLX_PPLXTASKS_H_ */
