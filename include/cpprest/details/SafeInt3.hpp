/*
 * SafeInt3.hpp
 *
 *  Created on: Oct 12, 2018
 *      Student (MIG Virtual Developer): Tung Dang
 */

#ifndef INCLUDE_CPPREST_DETAILS_SAFEINT3_HPP_
#define INCLUDE_CPPREST_DETAILS_SAFEINT3_HPP_

// Option Compiler
#define VISUAL_STUDIO_COMPILER 0
#define CLANG_COMPILER 1
#define GCC_COMPILER 2
#define UNKNOWN_COMPILER -1

#if defined __clang__
#define SAFEINT_COMPILER CLANG_COMPILER
#elif defined __GNUC__
#define SAFEINT_COMPILER GCC_COMPILER
#elif defined _MSC_VER
#define SAFEINT_COMPILER VISUAL_STUDIO_COMPILER
#else
#define SAFEINT_COMPILER UNKNOWN_COMPILER
#endif

#if SAFEINT_COMPILER == VISUAL_STUDIO_COMPILER
#pragma warning( push )
#pragma warning( disable:4987 4820 4987 4820 )

#endif

#include <cstddef>
#include <cstdlib>

#if SAFEINT_COMPILER == VISUAL_STUDIO_COMPILER && defined _M_AMD64
#include <intrin.h>
#define SAFEINT_USE_INTRINSICS 1
#else
#define SAFEINT_USE_INTRINSICS 0
#endif

#if SAFEINT_COMPILER == VISUAL_STUDIO_COMPILER
#pragma warning(pop)
#endif

#if SAFEINT_COMPILER == GCC_COMPILER || SAFEINT_COMPILER == CLANG_COMPILER
#define NEEDS_INT_DEFINED
#if !defined NULL
#define NULL 0
#endif

#if SAFEINT_COMPILER == GCC_COMPILER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif

#include <stdint.h>

#if SAFEINT_COMPILER == CLANG_COMPILER
#if __has_feature(cxx_nullptr)
#define NEEDS_NULLPTR_DEFINED 0
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++11-long-long"
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif
#endif

#if !defined NEEDS_NULLPTR_DEFINED
#if SAFEINT_COMPILER == VISUAL_STUDIO_COMPILER

#if (_MSC_VER < 1600)
#define NEEDS_NULLPTR_DEFINED 1
#else
#define NEEDS_NULLPTR_DEFINED 0
#endif

#else

#if __cplusplus >= 201103L
#define NEEDS_NULLPTR_DEFINED 0
#else
#define NEEDS_NULLPTR_DEFINED 1
#endif

#endif
#endif

#if NEEDS_NULLPTR_DEFINED
#define nullptr NULL
#endif

#ifndef C_ASSERT
#define C_ASSERT(e) typedef char __C_ASSERT__[(e)?1:-1]
#endif
C_ASSERT(-1 == static_cast<int>(0xffffffff));

#ifdef NEEDS_INT_DEFINED
#define __int8 char
#define __int16 short
#define __int32 int
#define __int64 long long
#endif

namespace msl {
  namespace safeint3 {
    enum SafeIntError {
      SafeIntNoError = 0,
      SafeIntArithmeticOverflow,
      SafeIntDivideByZero
    };
  }
}

#if defined SAFEINT_REMOVE_NOTHROW
#define SAFEINT_NOTHROW
#else
#define SAFEINT_NOTHROW throw()
#endif

namespace msl
{

namespace safeint3
{

#if !defined SAFEINT_ASSERT
#include <assert.h>
#define SAFEINT_ASSERT(x) assert(x)
#endif

#if defined SAFEINT_ASSERT_ON_EXCEPTION
inline void SafeIntExceptionAssert() SAFEINT_NOTHROW {SAFEINT_ASSERT(false);}
#else
inline void SafeIntExceptionAssert() SAFEINT_NOTHROW {}
#endif

#if SAFEINT_COMPILER == GCC_COMPILER || SAFEINT_COMPILER == CLANG_COMPILER
#define SAFEINT_NORETURN __attribute__((noreturn))
#define SAFEINT_STDCALL
#define SAFEINT_VISIBLE __attribute__((__visibility__("default")))
#define SAFEINT_WEAK __attribute__((weak))
#else
#define SAFEINT_NORETURN __declspec(noreturn)
#define SAFEINT_STDCALL __stdcall
#define SAFEINT_VISIBLE
#define SAFEINT_WEAK
#endif

class SAFEINT_VISIBLE SafeIntException
{
public:
  SafeIntException() SAFEINT_NOTHROW { m_code = SafeIntNoError;}
  SafeIntException( SafeIntError code ) SAFEINT_NOTHROW
  {
    m_code = code;
  }
  SafeIntError m_code;
};

namespace SafeIntInternal
{
  template<typename E>class SafeIntExceptionHandler;
  template<>class SafeIntExceptionHandler<SafeIntException>
  {
  public:
    static SAFEINT_NORETURN void SAFEINT_STDCALL SafeIntOnOverflow()
    {
      SafeIntExceptionAssert();
      throw SafeIntException(SafeIntArithmeticOverflow);
    }

    static SAFEINT_NORETURN void SAFEINT_STDCALL SafeIntOnDivZero()
    {
      SafeIntExceptionAssert();
      throw SafeIntException(SafeIntDivideByZero);
    }
  };

#if !defined _CRT_SECURE_INVALID_PARAMETER
#include <stdlib.h>
#define _CRT_SECURE_INVALID_PARAMETER(msg) abort()
#endif

class SafeInt_InvalidParameter
{
public:
  static SAFEINT_NORETURN void SafeIntOnOverflow() SAFEINT_NOTHROW
  {
    SafeIntExceptionAssert();
    _CRT_SECURE_INVALID_PARAMETER("SafeInt Arithmetic Overflow");
  }

  static SAFEINT_NORETURN void SafeIntOnDivZero() SAFEINT_NOTHROW
  {
    SafeIntExceptionAssert();
    _CRT_SECURE_INVALID_PARAMETER("SafeInt Divide By Zero");
  }
};

#if defined _WINDOWS_
class SafeIntWin32ExceptionHandler
{
public:
  static SAFEINT_NORETURN void SAFEINT_STDCALL SafeIntOnOverflow() SAFEINT_NOTHROW
  {
    SafeIntExceptionAssert();
    RaiseException(static_cast<DWORD>(EXCEPTION_INT_OVERFLOW), EXCEPTION_NONCONTINUABLE,0,0);
  }

  static SAFEINT_NORETURN void SAFEINT_STDCALL SafeIntOnDivZero() SAFEINT_NOTHROW
  {
    SafeIntExceptionAssert();
    RaiseException(static_cast<DWORD>(EXCEPTION_INT_DIVIDE_BY_ZERO), EXCEPTION_NONCONTINUABLE,0,0);
  }
};
#endif
}

// Both of these cross-platform support

typedef SafeIntInternal::SafeIntExceptionHandler <SafeIntException> CPlusPlusExceptionHandler;
typedef SafeIntInternal::SafeInt_InvalidParameter InvalidParameterExceptionHandler;

#if defined _WINDOWS_
typedef SafeIntInternal::SafeIntWin32ExceptionHandler Win32ExceptionHandler;
#endif

#if defined VISUAL_STUDIO_SAFEINT_COMPAT
typedef CPlusPlusExceptionHandler SafeIntErrorPolicy_SafeIntException;
typedef InvalidParameterExceptionHandler SafeIntErrorPolicy_InvalidParameter;
#endif

#if !defined SafeIntDefaultExceptionHandler
    #if defined SAFEINT_RAISE_EXCEPTION
        #if !defined _WINDOWS_
        #error Include windows.h in order to use Win32 exceptions
        #endif

        #define SafeIntDefaultExceptionHandler Win32ExceptionHandler
    #elif defined SAFEINT_FAILFAST
        #define SafeIntDefaultExceptionHandler InvalidParameterExceptionHandler
    #else
        #define SafeIntDefaultExceptionHandler CPlusPlusExceptionHandler
		#if !defined SAFEINT_EXCEPTION_HANDLER_CPP
		#define SAFEINT_EXCEPTION_HANDLER_CPP 1
		#endif
    #endif
#endif

#if !defined SAFEINT_EXCEPTION_HANDLER_CPP
#define SAFEINT_EXCEPTION_HANDLER_CPP 0
#endif

#if SAFEINT_EXCEPTION_HANDLER_CPP
#define SAFEINT_CPP_THROW
#else
#define SAFEINT_CPP_THROW SAFEINT_NOTHROW
#endif

/*
Turn out
We can fool the Compile into not seeing Compile-time constants
*/
template<int method>class CompileConst;
template<> class CompileConst<true> {public: static bool Value() SAFEINT_NOTHROW {return true;}};
template<> class CompileConst<false> {public: static bool Value() SAFEINT_NOTHROW {return false;}};

/*--------------------------------------------------------------------------------------------------------------------------------------*/

/*
With following template, we are not allowed to cast a float to an enum.
In this case, if we happen to assign an enum to a SafeInt of some type, it will not compile
*/
template<typename T> class NumericType;
#if defined _LIBCPP_TYPE_TRAITS || defined _TYPE_TRAITS_
template<>class NumericType<bool> {public: enum { isBool = true, isFloat = false, isInt = false };};
template<typename T>class NumericType
{
public:
  enum
  {
    isBool = false;
    isFloat = std::is_floating_point<T>::value,
    isInt = std::is_integral<T>::value || std::is_enum<T>::value
  };
};

#else

template<>class NumericType<bool>          {public: enum {isBool = true, isFloat = false, isInt = false};};
template<>class NumericType<char>          {public: enum {isBool = false, isFloat = false, isInt = true};};
template<>class NumericType<unsigned char> {public: enum {isBool = false, isFloat = false, isInt = true};};
template<>class NumericType<signed char>   {public: enum {isBool = false, isFloat = false, isInt = true};};
template<>class NumericType<short>         {public: enum {isBool = false, isFloat = false, isInt = true};};
template<>class NumericType<unsigned short>{public: enum {isBool = false, isFloat = false, isInt = true};};
#if defined SAFEINT_USE_WCHAR_T || defined _NATIVE_WCHAR_T_DEFINED
template <> class NumericType<wchar_t>     {public: enum {isBool = false, isFloat = false, isInt = true};};
#endif
template<>class NumericType<int>           {public: enum {isBool = false, isFloat = false, isInt = true};};
template<>class NumericType<unsigned int>  {public: enum {isBool = false, isFloat = false, isInt = true};};
template<>class NumericType<long>          {public: enum {isBool = false, isFloat = false, isInt = true};};
template<>class NumericType<unsigned long> {public: enum {isBool = false, isFloat = false, isInt = true};};
template<>class NumericType<__int64>       {public: enum {isBool = false, isFloat = false, isInt = true};};
template<>class NumericType<unsigned __int64>{public: enum {isBool = false, isFloat = false, isInt = true};};
template<>class NumericType<float>         {public: enum {isBool = false, isFloat = true, isInt = false};};
template<>class NumericType<double>        {public: enum {isBool = false, isFloat = true, isInt = false};};
template<>class NumericType<long double>   {public: enum {isBool = false, isFloat = true, isInt = false};};

template<typename T>class NumericType
{
public:
  enum
  {
    isBool = false,
    isFloat = false,
    inInt = static_cast<int>(static_cast<T>(0)) == 0
  };
};
#endif

/*--------------------------------------------------------------------------------------------------------------------------------------*/

// Use the following template to avoid compile-time const truncation warning

template<int fSigned, int bits> class SafeIntMinMax;
template<> class SafeIntMinMax<true,8>
{
public:
  const static signed __int8 min = (-0x7f - 1);
  const static signed __int8 max = 0x7f;
};
template<> class SafeIntMinMax<true,16>
{
public:
  const static __int16 min = (-0x7fff - 1);
  const static __int16 max = 0x7fff;
};
template<> class SafeIntMinMax<true,32>
{
public:
  const static __int32 min = (-0x7fffffff -1);
  const static __int32 max = 0x7fffffff;
};
template<> class SafeIntMinMax<true,64>
{
public:
  const static __int64 min = static_cast<__int64>(0x8000000000000000LL);
  const static __int64 max = 0x7fffffffffffffffLL;
};

template<> class SafeIntMinMax<false, 8>
{
public:
  const static unsigned __int8 min = 0;
  const static unsigned __int8 max = 0xff;
};
template<> class SafeIntMinMax<false, 16>
{
public:
  const static unsigned __int16 min = 0;
  const static unsigned __int16 max = 0xffff;
};
template<> class SafeIntMinMax<false, 32>
{
public:
  const static unsigned __int32 min = 0;
  const static unsigned __int32 max = 0xffffffff;
};
template<> class SafeIntMinMax<false, 64>
{
public:
  const static unsigned __int64 min = 0;
  const static unsigned __int64 max = 0xffffffffffffffffULL;
};

template<typename T>class IntTraits
{
public:
  C_ASSERT(NumericType<T>::isInt);
  enum
  {
    isSigned = ((T)(-1) < 0),
    is64Bit = (sizeof(T) == 8),
    is32Bit = (sizeof(T) == 4),
    is16Bit = (sizeof(T) == 2),
    is8Bit  = (sizeof(T) == 1),
    isLT32Bit = (sizeof(T) < 4),
    isLT64Bit = (sizeof(T) < 8),
    isInt8  = (sizeof(T) == 1 && isSigned),
    isUint8 = (sizeof(T) == 1 && !isSigned),
    isInt16  = (sizeof(T) == 2 && isSigned),
    isUint16 = (sizeof(T) == 2 && !isSigned),
    isInt32  = (sizeof(T) == 4 && isSigned),
    isUint32 = (sizeof(T) == 4 && !isSigned),
    isInt64  = (sizeof(T) == 8 && isSigned),
    isUint64 = (sizeof(T) == 8 && !isSigned),
    bitCount = (sizeof(T)*8),
    isBool   = ((T)2 == (T)1)
  };

  const static T maxInt = static_cast<T>(SafeIntMinMax<isSigned,bitCount>::max);
  const static T minInt = static_cast<T>(SafeIntMinMax<isSigned,bitCount>::min);
};

template<typename T>
const T IntTraits<T>::maxInt;
template<typename T>
const T IntTraits<T>::minInt;

/*--------------------------------------------------------------------------------------------------------------------------------------*/

template<typename T, typename U>class SafeIntCompare
{
public:
  enum
  {
    isBothSigned = (IntTraits<T>::isSigned && IntTraits<U>::isSigned),
    isBothUnSigned = (!IntTraits<T>::isSigned && !IntTraits<U>::isSigned),
    isLikeSigned = ((bool)(IntTraits<T>::isSigned) == (bool)(IntTraits<U>::isSigned)),
    isCastOK = ((isLikeSigned && sizeof(T) >= sizeof(U)) ||
                (IntTraits<T>::isSigned && sizeof(T) > sizeof(U))),
    isBothLT32Bit = (IntTraits<T>::isLT32Bit && IntTraits<U>::isLT32Bit),
    isBothLT64Bit = (IntTraits<T>::isLT64Bit && IntTraits<U>::isLT64Bit)
    };
};

template<typename T, typename U>  class IntRegion
{
public:
  enum
  {
        IntZone_UintLT32_UintLT32 = SafeIntCompare< T,U >::isBothUnsigned && SafeIntCompare< T,U >::isBothLT32Bit,
        IntZone_Uint32_UintLT64   = SafeIntCompare< T,U >::isBothUnsigned && IntTraits< T >::is32Bit && IntTraits< U >::isLT64Bit,
        IntZone_UintLT32_Uint32   = SafeIntCompare< T,U >::isBothUnsigned && IntTraits< T >::isLT32Bit && IntTraits< U >::is32Bit,
        IntZone_Uint64_Uint       = SafeIntCompare< T,U >::isBothUnsigned && IntTraits< T >::is64Bit,
        IntZone_UintLT64_Uint64    = SafeIntCompare< T,U >::isBothUnsigned && IntTraits< T >::isLT64Bit && IntTraits< U >::is64Bit,

        IntZone_UintLT32_IntLT32  = !IntTraits< T >::isSigned && IntTraits< U >::isSigned && SafeIntCompare< T,U >::isBothLT32Bit,
        IntZone_Uint32_IntLT64    = IntTraits< T >::isUint32 && IntTraits< U >::isSigned && IntTraits< U >::isLT64Bit,
        IntZone_UintLT32_Int32    = !IntTraits< T >::isSigned && IntTraits< T >::isLT32Bit && IntTraits< U >::isInt32,
        IntZone_Uint64_Int        = IntTraits< T >::isUint64 && IntTraits< U >::isSigned && IntTraits< U >::isLT64Bit,
        IntZone_UintLT64_Int64    = !IntTraits< T >::isSigned && IntTraits< T >::isLT64Bit && IntTraits< U >::isInt64,
        IntZone_Uint64_Int64      = IntTraits< T >::isUint64 && IntTraits< U >::isInt64,

        IntZone_IntLT32_IntLT32   = SafeIntCompare< T,U >::isBothSigned && SafeIntCompare< T, U >::isBothLT32Bit,
        IntZone_Int32_IntLT64     = SafeIntCompare< T,U >::isBothSigned && IntTraits< T >::is32Bit && IntTraits< U >::isLT64Bit,
        IntZone_IntLT32_Int32     = SafeIntCompare< T,U >::isBothSigned && IntTraits< T >::isLT32Bit && IntTraits< U >::is32Bit,
        IntZone_Int64_Int64       = SafeIntCompare< T,U >::isBothSigned && IntTraits< T >::isInt64 && IntTraits< U >::isInt64,
        IntZone_Int64_Int         = SafeIntCompare< T,U >::isBothSigned && IntTraits< T >::is64Bit && IntTraits< U >::isLT64Bit,
        IntZone_IntLT64_Int64     = SafeIntCompare< T,U >::isBothSigned && IntTraits< T >::isLT64Bit && IntTraits< U >::is64Bit,

        IntZone_IntLT32_UintLT32  = IntTraits< T >::isSigned && !IntTraits< U >::isSigned && SafeIntCompare< T,U >::isBothLT32Bit,
        IntZone_Int32_UintLT32    = IntTraits< T >::isInt32 && !IntTraits< U >::isSigned && IntTraits< U >::isLT32Bit,
        IntZone_IntLT64_Uint32    = IntTraits< T >::isSigned && IntTraits< T >::isLT64Bit && IntTraits< U >::isUint32,
        IntZone_Int64_UintLT64    = IntTraits< T >::isInt64 && !IntTraits< U >::isSigned && IntTraits< U >::isLT64Bit,
        IntZone_Int_Uint64        = IntTraits< T >::isSigned && IntTraits< U >::isUint64 && IntTraits< T >::isLT64Bit,
        IntZone_Int64_Uint64 = IntTraits< T >::isInt64 && IntTraits< U >::isUint64
  };
};

/*--------------------------------------------------------------------------------------------------------------------------------------*/

/*
The non-throwing version are for use by the helper functions that return success and failure
Some of the non-throwing functions are not used, but are maintained for completeness
*/
enum AbsMethod
{
  AbsMethodInt,
  AbsMethodInt64,
  AbsMethodNoop
};

template <typename T>
class GetAbsMethod
{
public:
  enum
  {
    method = IntTraits<T>::isLT64Bit && IntTraits<T>::isSigned ? AbsMethodInt :
             IntTraits<T>::isInt64 ? AbsMethodInt64 : AbsMethodNoop
  };
};

template<typename T, int > class AbsValueHelper;
template<typename T> class AbsValueHelper <T,AbsMethodInt>
{
public:
	static unsigned __int32 Abs(T t) SAFEINT_NOTHROW
	{
		SAFEINT_ASSERT(t < 0);
		return ~(unsigned __int32)t + 1;
	}
};

template<typename T> class AbsValueHelper<T,AbsMethodInt64>
{
public:
	static unsigned __int64 Abs(T t) SAFEINT_NOTHROW
	{
		SAFEINT_ASSERT(t < 0);
		return ~(unsigned __int64)t + 1;
	}
};

template<typename T> class AbsValueHelper<T,AbsMethodNoop>
{
public:
	static T Abs(T t) SAFEINT_NOTHROW
	{
		SAFEINT_ASSERT(false);
		return t;
	}
};

/*--------------------------------------------------------------------------------------------------------------------------------------*/

/*
 * Negation will normally upcast to int
 * For example -(unsigned short)0xffff == (int)0xffff0001
 */
template<typename T, bool> class NegationHelper;
template<typename T> class NegationHelper <T,true>
{
public:
	template<typename E>
	static T NegativeThrow(T t) SAFEINT_CPP_THROW
	{
		if (t != IntTraits<T>::minInt)
		{
			return -t;
		}
		E::SafeIntOnOverflow();
	}

	static bool Negative(T t, T& ret) SAFEINT_NOTHROW
	{
		if (t != IntTraits<T>::minInt)
		{
			ret = -t;
			return true;
		}
		return false;
    }
};

template <typename T> class SignedNegation;
template<>
class SignedNegation <signed __int32>
{
public:
	static signed __int32 Value(unsigned __int64 in) SAFEINT_NOTHROW
	{
		return (signed __int32)(~(unsigned __int32)in + 1);
	}
	static signed __int32 Value(unsigned __int32 in) SAFEINT_NOTHROW
	{
		return (signed __int32)(~in + 1);
	}

};

template <>
class SignedNegation <signed __int64>
{
public:
	static signed __int64 Value(unsigned __int64 in) SAFEINT_NOTHROW
	{
		return (signed __int64)(~in + 1);
	}
};

template <typename T> class NegationHelper <T, false>
{
public:
  template<typename E>
  static T NegativeThrow(T t) SAFEINT_CPP_THROW
  {
    #if defined SAFEINT_DISALLOW_UNSIGNED_NEGATION
    C_ASSERT(sizeof(T) == 0);
    #endif

    #if SAFEINT_COMPILER == VISUAL_STUDIO_COMPILER
    #pragma warning(push)
    #pragma warning(disable:4146)
    #endif
    return (T)-t;

    #if SAFEINT_COMPILER == VISUAL_STUDIO_COMPILER
    #pragma warning(pop)
    #endif
  }

  static bool Negative(T t, T& ret) SAFEINT_NOTHROW
  {
    if (IntTraits<T>::isLT32Bit)
    {
      SAFEINT_ASSERT(false);
    }
    #if defined SAFEINT_DISALLOW_UNSIGNED_NEGATION
    C_ASSERT(sizeof(T) == 0);
    #endif
    ret = -t;
    return true;
  }
};

/*--------------------------------------------------------------------------------------------------------------------------------------*/

// Core logic to determine casting behavior
enum CastMethod {
	CastOK = 0,
	CastCheckLTZero,
	CastCheckGTMax,
	CastCheckSafeIntMinMaxUnsigned,
	CastCheckSafeIntMinMaxSigned,
	CastToFloat,
	CastFromFloat,
	CastToBool,
	CastFromBool,
};

template <typename ToType, typename FromType>
class GetCastMethod
{
public:
	enum {
		method = (IntTraits<FromType>::isBool && !IntTraits<ToType>::isBool) ? CastFromBool:
				(!IntTraits<FromType>::isBool && IntTraits<ToType>::isBool) ? CastToBool:
				(SafeIntCompare<ToType,FromType>::isCastOK) ? CastOK:
				((IntTraits<ToType>::isSigned && !IntTraits<FromType>::isSigned && sizeof(FromType) >= sizeof(ToType)) ||
				 (SafeIntCompare<ToType, FromType>::isBothUnSigned && sizeof(FromType) > sizeof(ToType))) ? CastCheckGTMax:
				(!IntTraits<ToType>::isSigned && IntTraits<FromType>::isSigned &&
						sizeof(ToType) >= sizeof(FromType)) ? CastCheckLTZero:
				(!IntTraits<ToType>::isSigned) ? CastCheckSafeIntMinMaxUnsigned : CastCheckSafeIntMinMaxSigned
	};
};

template <typename FromType> class GetCastMethod <float, FromType>
{
public:
	enum {
		method = CastOK
	};
};

template <typename FromType> class GetCastMethod <double, FromType>
{
public:
	enum {
		method = CastOK
	};
};

template <typename FromType> class GetCastMethod <long double, FromType>
{
public:
	enum {
		method = CastOK
	};
};

template <typename ToType> class GetCastMethod<ToType, float>
{
public:
	enum {
		method = CastFromFloat
	};
};

template <typename ToType> class GetCastMethod<ToType, double>
{
public:
	enum {
		method = CastFromFloat
	};
};

template <typename ToType> class GetCastMethod<ToType, long double>
{
public:
	enum {
		method = CastFromFloat
	};
};

template <typename T, typename U, int> class SafeCastHelper;
template <typename T, typename U> class SafeCastHelper <T,U,CastOK>
{
public:
	static bool Cast(U u, T& t) SAFEINT_NOTHROW
	{
		t = (T)u;
		return true;
	}

	template <typename E>
	static void CastThrow(U u, T& t) SAFEINT_CPP_THROW
	{
		t = (T)u;
	}
};

template <typename T, typename U> class SafeCastHelper <T, U, CastFromFloat>
{
public:
	static bool Cast(U u, T& t) SAFEINT_NOTHROW
	{
		if (u <= (U)IntTraits<T>::maxInt && u >= (U)IntTraits<T>::minInt)
		{
			t = (T)u;
			return true;
		}
		return false;
	}

	template <typename E>
	static void CastThrow(U u, T& t) SAFEINT_CPP_THROW
	{
		if (u <= (U)IntTraits<T>::maxInt && u >= (U)IntTraits<T>::minInt)
		{
			t = (T)u;
			return;
		}
		E::SafeIntOnOverflow();
	}
};

template <typename T> class SafeCastHelper <T, bool, CastFromBool>
{
public:
	static bool Cast(bool b, T& t) SAFEINT_NOTHROW
	{
		t = (T)(b ? 1 : 0);
		return true;
	}

	template <typename E>
	static void CastThrow(bool b, T& t) SAFEINT_CPP_THROW
	{
		t = (T)(b ? 1 : 0);
	}
};

template <typename T> class SafeCastHelper <bool, T, CastToBool>
{
public:
	static bool Cast(T t, bool& b) SAFEINT_NOTHROW
	{
		b = !!t;
		return true;
	}

	template <typename E>
	static void CastThrow(T t, bool& b) SAFEINT_CPP_THROW
	{
		b = !!t;
	}
};

template <typename T, typename U> class SafeCastHelper <T, U, CastCheckLTZero>
{
public:
	static bool Cast(U u, T& t) SAFEINT_NOTHROW
	{
		if (u < 0)
		{
			return false;
		}
		t = (T)u;
		return true;
	}

	template<typename E>
	static void CastThrow(U u, T& t) SAFEINT_CPP_THROW
	{
		if (u < 0)
		{
			E::SafeIntOnOverflow();
		}
		t = (T)u;
	}
};

template <typename T, typename U> class SafeCastHelper <T, U, CastCheckGTMax>
{
public:
	static bool Cast(U u, T& t) SAFEINT_NOTHROW
	{
		if (u > (U)IntTraits<T>::maxInt)
		{
			return false;
		}
		t = (T)u;
		return true;
	}
	template <typename E>
	static void CastThrow(U u, T&t) SAFEINT_CPP_THROW
	{
		if (u > (U)IntTraits<T>::maxInt)
			E::SafeIntOnOverflow();
		t = (T)u;
	}
};

template <typename T, typename U> class SafeCastHelper <T, U, CastCheckSafeIntMinMaxUnsigned>
{
public:
	static bool Cast(U u, T& t) SAFEINT_NOTHROW
	{
		if (u > IntTraits<T>::maxInt || u < 0)
			return false;
		t = (T)u;
		return true;
	}

	template <typename E>
	static void CastThrow(U u, T& t) SAFEINT_CPP_THROW
	{
		if (u > IntTraits<T>::maxInt || u < 0)
			E::SafeIntOnOverflow();
		t = (T)u;
	}
};

template <typename T, typename U> class SafeCastHelper <T, U, CastCheckSafeIntMinMaxSigned>
{
public:
	static bool Cast(U u, T& t) SAFEINT_NOTHROW
	{
		if (u > IntTraits<T>::maxInt || u < IntTraits<T>::minInt)
			return false;
		t = (T)u;
		return true;
	}

	template <typename E>
	static void CastThrow(U u, T& t) SAFEINT_CPP_THROW
	{
		if (u > IntTraits<T>::maxInt || u < IntTraits<T>::minInt)
			E::SafeIntOnOverflow();
		t = (T)u;
	}
};

/*--------------------------------------------------------------------------------------------------------------------------------------*/

/*
 * Some cases of some integer conversion operations
 * For example:
 *             signed char a = -1 = 0xff
 *             unsigned int b = 0xffffffff
 * We need a value-preserving cast
 * and we are essentially testing (unsigned int)a < b == false
 */

enum ComparisonMethod {
	ComparisonMethod_Ok = 0,
	ComparisonMethod_CastInt,
	ComparisonMethod_CastInt64,
	ComparisonMethod_UnsignedT,
	ComparisonMethod_UnsignedU
};

template<typename T, typename U>
class ValidComparison
{
public:
  enum
  {
    #ifdef ANSI_CONVERSIONS
    method = ComparisonMethod_Ok
    #else
    method = ((SafeIntCompare<T,U>::isLikeSigned) ? ComparisonMethod_Ok:
             ((IntTraits<T>::isSigned && sizeof(T) < 8 && sizeof(U) < 4) ||
              (IntTraits<U>::isSigned && sizeof(T) < 4 && sizeof(U) < 8)) ? ComparisonMethod_CastInt :
             ((IntTraits<T>::isSigned && sizeof(U) < 8) ||
              (IntTraits<U>::isSigned && sizeof(T) < 8)) ? ComparisonMethod_CastInt64 :
             (!IntTraits<T>::isSigned) ? ComparisonMethod_UnsignedT :
                                         ComparisonMethod_UnsignedU
    )
    #endif
  };
};

template <typename T, typename U, int state> class EqualityTest;
template <typename T, typename U> class EqualityTest<T, U, ComparisonMethod_Ok>
{
public:
	static bool IsEquals(const T t, const U u) SAFEINT_NOTHROW
	{
		return (t == u);
	}
};

template <typename T, typename U> class EqualityTest<T, U, ComparisonMethod_CastInt>
{
public:
	static bool IsEquals(const T t, const U u) SAFEINT_NOTHROW
	{
		return ((int)t == (int)u);
	}
};

template <typename T, typename U> class EqualityTest<T, U, ComparisonMethod_CastInt64>
{
public:
	static bool IsEquals(const T t, const U u) SAFEINT_NOTHROW
	{
		return ((__int64)t == (__int64)u);
	}
};

template <typename T, typename U> class EqualityTest<T, U, ComparisonMethod_UnsignedT>
{
public:
	// one operand is 32 or 64bit unsigned, and the other is signed and the same size or smaller
	static bool IsEquals(const T t, const U u) SAFEINT_NOTHROW
	{
		if (u < 0)
			return false;
		return (t == (T)u);
	}
};

template <typename T, typename U> class EqualityTest<T, U, ComparisonMethod_UnsignedU>
{
public:
	static bool IsEquals(const T t, const U u) SAFEINT_NOTHROW
	{
		if (t < 0)
			return false;
		return ((U)t == u);
	}
};

template <typename T, typename U, int state> class GreaterThanTest;
template <typename T, typename U> class GreaterThanTest <T, U, ComparisonMethod_Ok>
{
public:
	static bool GreaterThan(const T t, const U u) SAFEINT_NOTHROW
	{
		return (t > u);
	}
};

template <typename T, typename U> class GreaterThanTest <T, U, ComparisonMethod_CastInt>
{
public:
	static bool GreaterThan(const T t, const U u) SAFEINT_NOTHROW
	{
		return ((int)t > (int)u);
	}
};

template <typename T, typename U> class GreaterThanTest <T, U, ComparisonMethod_CastInt64>
{
public:
	static bool GreaterThan(const T t, const U u) SAFEINT_NOTHROW
	{
		return ((__int64)t > (__int64)u);
	}
};

template <typename T, typename U> class GreaterThanTest <T, U, ComparisonMethod_UnsignedT>
{
public:
	static bool GreaterThan(const T t, const U u) SAFEINT_NOTHROW
	{
		if (u < 0)
			return true;
		return (t > (T)u);
	}
};

template <typename T, typename U> class GreaterThanTest <T, U, ComparisonMethod_UnsignedU>
{
public:
	static bool GreaterThan(const T t, const U u) SAFEINT_NOTHROW
	{
		if (t > 0)
			return true;
		return ((U)t > u);
	}
};

template <typename T, typename U, int method> class ModulusHelper;
template <typename T, typename U> class ModulusHelper<T, U, ComparisonMethod_Ok>
{
public:
	static SafeIntError Modulus(const T&t, const U& u, T& result) SAFEINT_NOTHROW
	{
		if (u == 0)
			return SafeIntDivideByZero;

		if (CompileConst <IntTraits<U>::isSigned>::Value())
		{
			if (u == (U)-1)
			{
				result = 0;
				return SafeIntNoError;
			}
		}
		result = (T)(t % u);
		return SafeIntNoError;
	}

	template <typename E>
	static void ModulusThrow(const T& t, const U& u, T& result) SAFEINT_CPP_THROW
	{
		if (u == 0)
			E::SafeIntOnDivZero();
		if (CompileConst <IntTraits<U>::isSigned>::Value())
		{
			if (u == (U)-1)
			{
				result = 0;
				return;
			}
		}
		result = (T)(t % u);
	}
};

template <typename T, typename U> class ModulusHelper <T, U, ComparisonMethod_CastInt>
{
public:
	static SafeIntError Modulus(const T& t, const U& u, T& result) SAFEINT_NOTHROW
	{
		if (u == 0)
		{
			return SafeIntDivideByZero;
		}
		if (CompileConst<IntTraits<U>::isSigned>::Value())
		{
			if (u == (U)-1)
			{
				result = 0;
				return SafeIntNoError;
			}
		}
		result = (T)(t % u);
		return SafeIntNoError;
	}

	template<typename E>
	static void ModulusThrow(const T& t, const U& u, T& result) SAFEINT_CPP_THROW
	{
		if (u == 0)
			E::SafeIntOnDivZero();
		if (CompileConst<IntTraits<U>::isSigned>::Value())
		{
			if (u == (u)-1)
			{
				result = 0;
				return;
			}
		}
		result = (T)(t % u);
	}
};

template <typename T, typename U> class ModulusHelper <T, U, ComparisonMethod_CastInt64>
{
public:
	static SafeIntError Modulus(const T& t, const U& u, T& result) SAFEINT_NOTHROW
	{
		if (u == 0)
		{
			return SafeIntDivideByZero;
		}
		if (CompileConst<IntTraits<U>::isSigned>::Value())
		{
			if (u == (U)-1)
			{
				result = 0;
				return SafeIntNoError;
			}
		}
		result = (T)((__int64)t % (__int64)u);
		return SafeIntNoError;
	}

	template<typename E>
	static void ModulusThrow(const T& t, const U& u, T& result) SAFEINT_CPP_THROW
	{
		if (u == 0)
			E::SafeIntOnDivZero();
		if (CompileConst<IntTraits<U>::isSigned>::Value())
		{
			if (u == (u)-1)
			{
				result = 0;
				return;
			}
		}
		result = (T)((__int64)t % (__int64)u);
	}
};

template <typename T, typename U> class ModulusHelper<T, U, ComparisonMethod_UnsignedT>
{
public:
	static SafeIntError Modulus(const T& t, const U& u, T& result) SAFEINT_NOTHROW
	{
		if (u == 0)
			return SafeIntDivideByZero;
		if (u < 0)
			result = (T)(t % AbsValueHelper<U, GetAbsMethod<U>::method>::Abs(u));
		else
			result = (T)(t % u);
		return SafeIntNoError;
	}

	template < typename E >
	static void ModulusThrow( const T& t, const U& u, T& result ) SAFEINT_CPP_THROW
	{
		if (u == 0)
			E::SafeIntOnDivZero();
		if (u < 0)
			result = (T)(t % AbsValueHelper<U, GetAbsMethod<U>::method>::Abs(u));
		else
			result = (T)(t % u);
	}
};

template <typename T, typename U> class ModulusHelper<T, U, ComparisonMethod_UnsignedU>
{
public:
	static SafeIntError Modulus(const T& t, const U& u, T& result) SAFEINT_NOTHROW
	{
		if (u == 0)
			return SafeIntDivideByZero;
		if (t < 0)
			result = (T)(~(AbsValueHelper<T,GetAbsMethod<T>::method>::Abs(t) % u) + 1);
		else
			result = (T)((T)t % u);
		return SafeIntNoError;
	}

	template < typename E >
	static void ModulusThrow( const T& t, const U& u, T& result ) SAFEINT_CPP_THROW
	{
		if (u == 0)
			E::SafeIntOnDivZero();
		if (u < 0)
			result = (T)(~(AbsValueHelper<T,GetAbsMethod<T>::method>::Abs(t) % u) + 1);
		else
			result = (T)((T)t % u);
	}
};

/*--------------------------------------------------------------------------------------------------------------------------------------*/

enum MultiplicationState {
	MultiplicationState_CastInt = 0,
	MultiplicationState_CastInt64,      // One or both signed, smaller than 64-bit
	MultiplicationState_CastUint,       // Both are unsigned, smaller than 32-bit
	MultiplicationState_CastUint64,     // Both are unsigned, both 32-bit or smaller
	MultiplicationState_Uint64Uint,     // Both are unsigned, lhs 64-bit, rhs 32-bit or smaller
	MultiplicationState_Uint64Uint64,   // Both are unsigned int64
	MultiplicationState_Uint64Int,      // lhs is unsigned int64, rhs int 32
	MultiplicationState_Uint64Int64,
	MultiplicationState_UintUint64,     // Both are unsigned, lhs 32-bit or smaller, rhs 64-bit
	MultiplicationState_UintInt64,
	MultiplicationState_Int64Uint,
	MultiplicationState_Int64Int64,
	MultiplicationState_Int64Int,
	MultiplicationState_IntUint64,
	MultiplicationState_IntInt64,
	MultiplicationState_Int64Uint64,
	MultiplicationState_Error
};

template <typename T, typename U>
class MultiplicationMethod
{
public:
	enum {
		method = (IntRegion<T,U>::IntZone_UintLT32_UintLT32 ? MultiplicationState_CastUint :
				 (IntRegion<T,U>::IntZone_Uint32_UintLT64 ||
				  IntRegion<T,U>::IntZone_UintLT32_Uint32) ? MultiplicationState_CastUint64 :
				  SafeIntCompare<T,U>::isBothUnSigned &&
				  IntTraits<T>::isUint64 && IntTraits<U>::isUint64 ? MultiplicationState_Uint64Uint64 :
				  (IntRegion<T,U>::IntZone_Uint64_Uint) ? MultiplicationState_Uint64Uint :
				  (IntRegion<T,U>::IntZone_UintLT64_Uint64) ? MultiplicationState_UintUint64 :
				  (IntRegion< T,U >::IntZone_UintLT32_IntLT32)  ? MultiplicationState_CastInt :
				  (IntRegion< T,U >::IntZone_Uint32_IntLT64 ||
				  IntRegion< T,U >::IntZone_UintLT32_Int32)    ? MultiplicationState_CastInt64 :
				  (IntRegion< T,U >::IntZone_Uint64_Int)        ? MultiplicationState_Uint64Int :
				  (IntRegion< T,U >::IntZone_UintLT64_Int64)    ? MultiplicationState_UintInt64 :
                  (IntRegion< T,U >::IntZone_Uint64_Int64)      ? MultiplicationState_Uint64Int64 :
				  (IntRegion< T,U >::IntZone_IntLT32_IntLT32)   ? MultiplicationState_CastInt :
	              (IntRegion< T,U >::IntZone_Int32_IntLT64 ||
			      IntRegion< T,U >::IntZone_IntLT32_Int32)     ? MultiplicationState_CastInt64 :
				  (IntRegion< T,U >::IntZone_Int64_Int64)       ? MultiplicationState_Int64Int64 :
				  (IntRegion< T,U >::IntZone_Int64_Int)         ? MultiplicationState_Int64Int :
				  (IntRegion< T,U >::IntZone_IntLT64_Int64)     ? MultiplicationState_IntInt64 :
				  (IntRegion< T,U >::IntZone_IntLT32_UintLT32)  ? MultiplicationState_CastInt :
				  (IntRegion< T,U >::IntZone_Int32_UintLT32 ||
			       IntRegion< T,U >::IntZone_IntLT64_Uint32)    ? MultiplicationState_CastInt64 :
				  (IntRegion< T,U >::IntZone_Int64_UintLT64)    ? MultiplicationState_Int64Uint :
				  (IntRegion< T,U >::IntZone_Int_Uint64)        ? MultiplicationState_IntUint64 :
				  (IntRegion< T,U >::IntZone_Int64_Uint64       ? MultiplicationState_Int64Uint64 :
				   MultiplicationState_Error ) )
	};
};

template <typename T, typename U, int state> class MultiplicationHelper;
template <typename T, typename U> class MultiplicationHelper<T, U, MultiplicationState_CastInt>
{
public:
	static bool Multiply(const T& t, const U& u, T& ret) SAFEINT_NOTHROW
	{
		int tmp = t * u;
		if (tmp > IntTraits<T>::maxInt || tmp < IntTraits<T>::minInt)
		{
			return false;
		}
		ret = (T)tmp;
		return true;
	}

	template <typename E>
	static void MultiplyThrow(const T& t, const U& u, T& ret) SAFEINT_CPP_THROW
	{
		int tmp = t * u;
		if (tmp > IntTraits<T>::maxInt || tmp < IntTraits<T>::minInt)
			E::SafeIntOnOverflow();
		ret = (T)tmp;
	}
};

template <typename T, typename U> class MultiplicationHelper<T, U, MultiplicationState_CastUint>
{
public:
	static bool Multiply(const T& t, const U& u, T& ret) SAFEINT_NOTHROW
	{
		unsigned int tmp = (unsigned int)(t * u);
		if (tmp > IntTraits<T>::maxInt)
			return false;
		ret = (T)tmp;
		return true;
	}

	template < typename E >
	static void MultiplyThrow( const T& t, const U& u, T& ret ) SAFEINT_CPP_THROW
	{
	    unsigned int tmp = (unsigned int)( t * u );
	    if( tmp > IntTraits< T >::maxInt )
	        E::SafeIntOnOverflow();
	    ret = (T)tmp;
	}
};

template <typename T, typename U> class MultiplicationHelper<T, U, MultiplicationState_CastInt64>
{
	static bool Multiply(const T& t, const U& u, T& ret) SAFEINT_NOTHROW
	{
		__int64 tmp = (__int64)t * (__int64)u;
		if (tmp > (__int64)IntTraits<T>::maxInt || tmp < (__int64)IntTraits<T>::minInt)
		{
			return false;
		}
		ret = (T)tmp;
		return true;
	}

	template < typename E >
	static void MultiplyThrow( const T& t, const U& u, T& ret ) SAFEINT_CPP_THROW
	{
	    __int64 tmp = (__int64)t * (__int64)u;
	    if(tmp > (__int64)IntTraits< T >::maxInt || tmp < (__int64)IntTraits< T >::minInt)
	       E::SafeIntOnOverflow();\
	    ret = (T)tmp;
	}
};

template <typename T, typename U> class MultiplicationHelper<T, U, MultiplicationState_CastUint64>
{
	static bool Multiply(const T& t, const U& u, T& ret) SAFEINT_NOTHROW
	{
		unsigned __int64 tmp = (unsigned __int64)t * (unsigned __int64)u;
		if (tmp > (unsigned __int64)IntTraits<T>::maxInt)
			return false;
		ret = (T)tmp;
		return true;
    }

	template < typename E >
	static void MultiplyThrow( const T& t, const U& u, T& ret ) SAFEINT_CPP_THROW
	{
	    unsigned __int64 tmp = (unsigned __int64)t * (unsigned __int64)u;
	    if(tmp > (unsigned __int64)IntTraits< T >::maxInt)
	       E::SafeIntOnOverflow();
	    ret = (T)tmp;
	}
};

template <typename T, typename U> class LargeIntRegMultiply;
#if SAFEINT_USE_INTRINSICS
inline bool IntrinsicMultiplyUint64(const unsigned __int64& a, const unsigned __int64& b,
                                    unsigned __int64* pRet) SAFEINT_NOTHROW
{
  unsigned __int64 ulHigh = 0;
  *pRet = _umul128(a,b,&ulHigh);
  return ulHigh == 0;
}

inline bool IntrinsicMultiplyInt64(const signed __int64& a, const signed __int64& b,
                                   signed __int64* pRet) SAFEINT_NOTHROW
{
  __int64 llHigh = 0;
  *pRet = _mul128(a,b,&llHigh);
  if ((a^b) < 0)
  {
    if (llHigh == -1 && *pRet < 0 || llHigh == 0 && *pRet == 0)
    {
      return true;
    }
  }
  else
  {
    if (llHigh == 0 && (unsigned __int64)*pRet <= IntTraits<signed __int64>::maxInt)
    {
      return true;
    }
  }
  return false;
}
#endif

template<> class LargeIntRegMultiply<unsigned __int64, unsigned __int64>
{
public:
  static bool RegMultiply (const unsigned __int64&& a, const unsigned __int64& b, unsigned __int64* pRet) SAFEINT_NOTHROW
  {
    #if SAFEINT_USE_INTRINSICS
    return IntrinsicMultiplyUint64(a,b,pRet);
    #else
    /*
     * Let consider that a*b can be broken up into:
     * (aHigh * 2^32 + aLow) * (bHigh * 2^32 + blow)
     * => (aHigh * bHigh * 2^64) + (aLow * bHigh * 2^32) + (aHigh * bLow * 2^32) + (aLow * bLow)
     */
    unsigned __int32 aHigh, aLow, bHigh, bLow;
    aHigh = (unsigned __int32)(a >> 32);
    aLow = (unsigned __int32)a;
    bHigh = (unsigned __int32)(b >> 32);
    bLow = (unsigned __int32)b;

    *pRet = 0;
    if (aHigh == 0)
    {
    	if (bHigh != 0)
    	{
    		*pRet = (unsigned __int64)aLow * (unsigned __int64)bHigh;
    	}
    }
    else if (bHigh == 0)
	{
    	if (aHigh != 0)
    	{
    		*pRet = (unsigned __int64)aHigh * (unsigned __int64)bLow;
    	}
	}
    else
    {
    	return false;
    }

    if (*pRet != 0)
    {
    	unsigned __int64 tmp;
    	if ((unsigned __int32)(*pRet >> 32) != 0)
    	{
    		return false;
    	}

    	*pRet <<= 32;
    	tmp = (unsigned __int64)aLow * (unsigned __int64)bLow;
    	*pRet += tmp;

    	if (*pRet < tmp)
    		return false;

    	return true;
    }

    *pRet = (unsigned __int64)aLow * (unsigned __int64)bLow;
    return true;

    #endif
  }

  template < typename E >
      static void RegMultiplyThrow( const unsigned __int64& a, const unsigned __int64& b, unsigned __int64* pRet ) SAFEINT_CPP_THROW
      {
  #if SAFEINT_USE_INTRINSICS
          if( !IntrinsicMultiplyUint64( a, b, pRet ) )
              E::SafeIntOnOverflow();
  #else
          unsigned __int32 aHigh, aLow, bHigh, bLow;

          aHigh = (unsigned __int32)(a >> 32);
          aLow  = (unsigned __int32)a;
          bHigh = (unsigned __int32)(b >> 32);
          bLow  = (unsigned __int32)b;

          *pRet = 0;

          if(aHigh == 0)
          {
              if(bHigh != 0)
              {
                  *pRet = (unsigned __int64)aLow * (unsigned __int64)bHigh;
              }
          }
          else if(bHigh == 0)
          {
              if(aHigh != 0)
              {
                  *pRet = (unsigned __int64)aHigh * (unsigned __int64)bLow;
              }
          }
          else
          {
              E::SafeIntOnOverflow();
          }

          if(*pRet != 0)
          {
              unsigned __int64 tmp;

              if((unsigned __int32)(*pRet >> 32) != 0)
                  E::SafeIntOnOverflow();

              *pRet <<= 32;
              tmp = (unsigned __int64)aLow * (unsigned __int64)bLow;
              *pRet += tmp;

              if(*pRet < tmp)
                  E::SafeIntOnOverflow();

              return;
          }

          *pRet = (unsigned __int64)aLow * (unsigned __int64)bLow;
  #endif
  }
};

template<> class LargeIntRegMultiply<unsigned __int64, unsigned __int32>
{
public:
  static bool RegMultiply(const unsigned __int64& a, unsigned __int32 b,
                          unsigned __int64* pRet) SAFEINT_NOTHROW
  {
    #if SAFEINT_USE_INTRINSICS
    return IntrinsicMultiplyUint64(a, (unsigned __int64)b, pRet);
    #else
    unsigned __int32 aHigh, aLow;
    /*
     * Let consider a*b can be broken up into:
     * (aHigh * 2^32 + aLow) * b
     * => (aHigh * b * 2^32) + (aLow * b)
     */
    aHigh = (unsigned __int32)(a >> 32);
    aLow = (unsigned __int32)a;

    *pRet = 0;
    if (aHigh != 0)
    {
    	*pRet = (unsigned __int64)aHigh * (unsigned __int64)b;
    	unsigned __int64 tmp;
    	if ((unsigned __int32)(*pRet >> 32) != 0)
    	{
    		return false;
    	}
    	*pRet <<= 32;
    	tmp = (unsigned __int64)aLow * (unsigned __int64)b;
    	*pRet += tmp;
    	if (*pRet < tmp)
    		return false;
    	return true;
    }
    *pRet = (unsigned __int64)aLow * (unsigned __int64)b;
    return true;
    #endif
  }

  template < typename E >
      static void RegMultiplyThrow( const unsigned __int64& a, unsigned __int32 b, unsigned __int64* pRet ) SAFEINT_CPP_THROW
      {
  #if SAFEINT_USE_INTRINSICS
          if( !IntrinsicMultiplyUint64( a, (unsigned __int64)b, pRet ) )
              E::SafeIntOnOverflow();
  #else
          unsigned __int32 aHigh, aLow;

          // Consider that a*b can be broken up into:
          // (aHigh * 2^32 + aLow) * b
          // => (aHigh * b * 2^32) + (aLow * b)

          aHigh = (unsigned __int32)(a >> 32);
          aLow  = (unsigned __int32)a;

          *pRet = 0;

          if(aHigh != 0)
          {
              *pRet = (unsigned __int64)aHigh * (unsigned __int64)b;

              unsigned __int64 tmp;

              if((unsigned __int32)(*pRet >> 32) != 0)
                  E::SafeIntOnOverflow();

              *pRet <<= 32;
              tmp = (unsigned __int64)aLow * (unsigned __int64)b;
              *pRet += tmp;

              if(*pRet < tmp)
                  E::SafeIntOnOverflow();

              return;
          }

          *pRet = (unsigned __int64)aLow * (unsigned __int64)b;
          return;
  #endif
  }
};

template<> class LargeIntRegMultiply<unsigned __int64, signed __int32>
{
public:
	static bool RegMultiply(const unsigned __int64& a, signed __int32 b, unsigned __int64* pRet) SAFEINT_NOTHROW
	{
		if (b < 0 && a != 0)
		{
			return false;
		}
    #if SAFEINT_USE_INTRINSICS
    return IntrinsicMultiplyUint64(a, (unsigned __int64)b, pRet);
    #else
    return LargeIntRegMultiply<unsigned __int64, unsigned __int32>::RegMultiply(a, (unsigned __int32)b, pRet);
    #endif
	}

  template < typename E >
    static void RegMultiplyThrow( const unsigned __int64& a, signed __int32 b, unsigned __int64* pRet ) SAFEINT_CPP_THROW
    {
        if( b < 0 && a != 0 )
            E::SafeIntOnOverflow();

#if SAFEINT_USE_INTRINSICS
        if( !IntrinsicMultiplyUint64( a, (unsigned __int64)b, pRet ) )
            E::SafeIntOnOverflow();
#else
        LargeIntRegMultiply< unsigned __int64, unsigned __int32 >::template RegMultiplyThrow< E >( a, (unsigned __int32)b, pRet );
#endif
}
};

template<> class LargeIntRegMultiply<unsigned __int64, signed __int64>
{
public:
	static bool RegMultiply( const unsigned __int64& a, signed __int64 b, unsigned __int64* pRet ) SAFEINT_NOTHROW
	    {
	        if( b < 0 && a != 0 )
	            return false;

	#if SAFEINT_USE_INTRINSICS
	        return IntrinsicMultiplyUint64( a, (unsigned __int64)b, pRet );
	#else
	        // return LargeIntRegMultiply< unsigned __int64, unsigned __int64 >::RegMultiply(a, (unsigned __int64)b, pRet);
	#endif
	}

  template < typename E >
    static void RegMultiplyThrow( const unsigned __int64& a, signed __int64 b, unsigned __int64* pRet ) SAFEINT_CPP_THROW
    {
        if( b < 0 && a != 0 )
            E::SafeIntOnOverflow();

#if SAFEINT_USE_INTRINSICS
        if( !IntrinsicMultiplyUint64( a, (unsigned __int64)b, pRet ) )
            E::SafeIntOnOverflow();
#else
        LargeIntRegMultiply< unsigned __int64, unsigned __int64 >::template RegMultiplyThrow< E >( a, (unsigned __int64)b, pRet );
#endif
}
};

template<> class LargeIntRegMultiply<signed __int32, unsigned __int64>
{
public:
	static bool RegMultiply(signed __int32 a, const unsigned __int64& b, signed __int32* pRet) SAFEINT_NOTHROW
	{
		/*
		 * Let consider a*b can be broken up into
		 * (aHigh * 2^32 + aLow) * (bHigh * 2^32 + bLow)
		 * => (aHigh * bHigh * 2^64) + (aLow * bHigh * 2^32) + (aHigh * bLow * 2^32) + (aLow * bLow)
		 */
		unsigned __int32 bHigh, bLow;
		bool fIsNegative = false;
		bHigh = (unsigned __int32)(b >> 32);
		bLow = (unsigned __int32)b;

		*pRet = 0;
		if (bHigh != 0 && a != 0)
			return false;

		if (a < 0)
		{
			a = (signed __int32)AbsValueHelper<signed __int32, GetAbsMethod<signed __int32>::method>::Abs(a);
			fIsNegative = true;
		}

		unsigned __int64 tmp = (unsigned __int32)a * (unsigned __int64)bLow;
		if (!fIsNegative)
		{
			if (tmp <= (unsigned __int64)IntTraits<signed __int32>::maxInt)
			{
				*pRet = (signed __int32)tmp;
				return true;
			}
		}
		else
		{
			if (tmp <= (unsigned __int64)IntTraits<signed __int32>::maxInt+1)
			{
				*pRet = SignedNegation< signed __int32 >::Value( tmp );
				return true;
			}
		}
		return false;
	}

	template < typename E >
	    static void RegMultiplyThrow( signed __int32 a, const unsigned __int64& b, signed __int32* pRet ) SAFEINT_CPP_THROW
	    {
	        unsigned __int32 bHigh, bLow;
	        bool fIsNegative = false;

	        // Consider that a*b can be broken up into:
	        // (aHigh * 2^32 + aLow) * (bHigh * 2^32 + bLow)
	        // => (aHigh * bHigh * 2^64) + (aLow * bHigh * 2^32) + (aHigh * bLow * 2^32) + (aLow * bLow)

	        bHigh = (unsigned __int32)(b >> 32);
	        bLow  = (unsigned __int32)b;

	        *pRet = 0;

	        if(bHigh != 0 && a != 0)
	            E::SafeIntOnOverflow();

	        if( a < 0 )
	        {
	            a = (signed __int32)AbsValueHelper< signed __int32, GetAbsMethod< signed __int32 >::method >::Abs(a);
	            fIsNegative = true;
	        }

	        unsigned __int64 tmp = (unsigned __int32)a * (unsigned __int64)bLow;

	        if( !fIsNegative )
	        {
	            if( tmp <= (unsigned __int64)IntTraits< signed __int32 >::maxInt )
	            {
	                *pRet = (signed __int32)tmp;
	                return;
	            }
	        }
	        else
	        {
	            if( tmp <= (unsigned __int64)IntTraits< signed __int32 >::maxInt+1 )
	            {
	                *pRet = SignedNegation< signed __int32 >::Value( tmp );
	                return;
	            }
	        }

	        E::SafeIntOnOverflow();
	}
};

template<> class LargeIntRegMultiply<unsigned __int32, unsigned __int64>
{
public:
	static bool RegMultiply(unsigned __int32 a, const unsigned __int64& b, unsigned __int32* pRet) SAFEINT_NOTHROW
	{
		/*
		 * Consider that a*b can be broken up into
		 * (bHigh * 2^32 + bLow) * a
		 * => (bHigh * a * 2^32) + (bLow * a)
		 * In this case, the result must fit into 32-bits
		 */
		if ((unsigned __int32)(b >> 32) != 0 && a != 0)
		{
			return false;
		}

		unsigned __int64 tmp = b * (unsigned __int64)a;
		if ((unsigned __int32)(tmp >> 32) != 0)
			return false;

		*pRet = (unsigned __int32)tmp;
		return true;
	}

	template < typename E >
	static void RegMultiplyThrow( unsigned __int32 a, const unsigned __int64& b, unsigned __int32* pRet ) SAFEINT_CPP_THROW
	{
	        if( (unsigned __int32)(b >> 32) != 0 && a != 0 )
	            E::SafeIntOnOverflow();

	        unsigned __int64 tmp = b * (unsigned __int64)a;

	        if( (unsigned __int32)(tmp >> 32) != 0 ) // overflow
	            E::SafeIntOnOverflow();

	        *pRet = (unsigned __int32)tmp;
	}
};

template<> class LargeIntRegMultiply<unsigned __int32, signed __int64>
{
	static bool RegMultiply(unsigned __int32 a, const signed __int64& b, unsigned __int32* pRet) SAFEINT_NOTHROW
	{
		if (b < 0 && a != 0)
			return false;
		return LargeIntRegMultiply<unsigned __int32, unsigned __int64>::RegMultiply(a, (unsigned __int64)b, pRet);
	}

	template < typename E >
	static void RegMultiplyThrow( unsigned __int32 a, const signed __int64& b, unsigned __int32* pRet ) SAFEINT_CPP_THROW
	{
	        if( b < 0 && a != 0 )
	            E::SafeIntOnOverflow();

	        LargeIntRegMultiply< unsigned __int32, unsigned __int64 >::template RegMultiplyThrow< E >( a, (unsigned __int64)b, pRet );
	}
};

template <> class LargeIntRegMultiply <signed __int64, signed __int64>
{
public:
	static bool RegMultiply(const signed __int64& a, const signed __int64& b, signed __int64* pRet) SAFEINT_NOTHROW
	{
#if SAFEINT_USE_INTRINSICS
		return IntrinsicMultiplyInt64(a,b,pRet);
#else
		bool aNegative = false;
		bool bNegative = false;

		unsigned __int64 tmp;
		__int64 a1 = a;
		__int64 b1 = b;

		if (a1 < 0)
		{
			aNegative = true;
			a1 = (signed __int64)AbsValueHelper<signed __int64, GetAbsMethod<signed __int64>::method>::Abs(a1);
		}

		if (b1 < 0)
		{
			bNegative = true;
			b1 = (signed __int64)AbsValueHelper<signed __int64, GetAbsMethod<signed __int64>::method>::Abs(b1);
		}

		if (LargeIntRegMultiply<unsigned __int64, unsigned __int64>::RegMultiply((unsigned __int64)a1,
				(unsigned __int64)b1, &tmp))
		{
			if (aNegative ^ bNegative)
			{
				if (tmp <= (unsigned __int64)IntTraits<signed __int64>::minInt)
				{
					*pRet = SignedNegation<signed __int64>::Value(tmp);
					return true;
				}
			}
			else
			{
				if (tmp <= (unsigned __int64)IntTraits<signed __int64>::maxInt)
				{
					*pRet = (signed __int64)tmp;
					return true;
				}
			}
		}
		return false;
#endif
	}

	template < typename E >
	static void RegMultiplyThrow( const signed __int64& a, const signed __int64& b, signed __int64* pRet ) SAFEINT_CPP_THROW
	{
	#if SAFEINT_USE_INTRINSICS
	        if( !IntrinsicMultiplyInt64( a, b, pRet ) )
	            E::SafeIntOnOverflow();
	#else
	        bool aNegative = false;
	        bool bNegative = false;

	        unsigned __int64 tmp;
	        __int64 a1 = a;
	        __int64 b1 = b;

	        if( a1 < 0 )
	        {
	            aNegative = true;
	            a1 = (signed __int64)AbsValueHelper< signed __int64, GetAbsMethod< signed __int64 >::method >::Abs(a1);
	        }

	        if( b1 < 0 )
	        {
	            bNegative = true;
	            b1 = (signed __int64)AbsValueHelper< signed __int64, GetAbsMethod< signed __int64 >::method >::Abs(b1);
	        }

	        LargeIntRegMultiply< unsigned __int64, unsigned __int64 >::template RegMultiplyThrow< E >( (unsigned __int64)a1, (unsigned __int64)b1, &tmp );

	        // The unsigned multiplication didn't overflow or we'd be in the exception handler
	        if( aNegative ^ bNegative )
	        {
	            // Result must be negative
	            if( tmp <= (unsigned __int64)IntTraits< signed __int64 >::minInt )
	            {
	                *pRet = SignedNegation< signed __int64 >::Value( tmp );
	                return;
	            }
	        }
	        else
	        {
	            // Result must be positive
	            if( tmp <= (unsigned __int64)IntTraits< signed __int64 >::maxInt )
	            {
	                *pRet = (signed __int64)tmp;
	                return;
	            }
	        }

	        E::SafeIntOnOverflow();
	#endif
	}
};

template <> class LargeIntRegMultiply <signed __int64, unsigned __int32>
{
	static bool RegMultiply(const signed __int64& a, unsigned __int32 b, signed __int64* pRet) SAFEINT_NOTHROW
    {
#if SAFEINT_USE_INTRINSICS
		return IntrinsicMultiplyInt64(a,b,pRet);
#else
		bool aNegative = false;
		unsigned __int64 tmp;
		__int64 a1 = a;

		if (a1 < 0)
		{
			aNegative = true;
			a1 = (signed __int64)AbsValueHelper<signed __int64, GetAbsMethod<signed __int64>::method>::Abs(a1);
		}

		if (LargeIntRegMultiply<unsigned __int64, unsigned __int32>::RegMultiply((unsigned __int64)a1, b, &tmp))
		{
			if (aNegative)
			{
				if (tmp <= (unsigned __int64)IntTraits<signed __int64>::minInt)
				{
					*pRet = SignedNegation<signed __int64>::Value(tmp);
					return true;
				}
			}
			else
			{
				if (tmp <= (unsigned __int64)IntTraits<signed __int64>::maxInt)
				{
					*pRet = (signed __int64)tmp;
					return true;
				}
			}
		}

		return false;
#endif
    }

	template < typename E >
	static void RegMultiplyThrow( const signed __int64& a, unsigned __int32 b, signed __int64* pRet ) SAFEINT_CPP_THROW
	{
	#if SAFEINT_USE_INTRINSICS
	        if( !IntrinsicMultiplyInt64( a, (signed __int64)b, pRet ) )
	            E::SafeIntOnOverflow();
	#else
	        bool aNegative = false;
	        unsigned __int64 tmp;
	        __int64 a1 = a;

	        if( a1 < 0 )
	        {
	            aNegative = true;
	            a1 = (signed __int64)AbsValueHelper< signed __int64, GetAbsMethod< signed __int64 >::method >::Abs(a1);
	        }

	        LargeIntRegMultiply< unsigned __int64, unsigned __int32 >::template RegMultiplyThrow< E >( (unsigned __int64)a1, b, &tmp );

	        // The unsigned multiplication didn't overflow
	        if( aNegative )
	        {
	            // Result must be negative
	            if( tmp <= (unsigned __int64)IntTraits< signed __int64 >::minInt )
	            {
	                *pRet = SignedNegation< signed __int64 >::Value( tmp );
	                return;
	            }
	        }
	        else
	        {
	            // Result must be positive
	            if( tmp <= (unsigned __int64)IntTraits< signed __int64 >::maxInt )
	            {
	                *pRet = (signed __int64)tmp;
	                return;
	            }
	        }

	        E::SafeIntOnOverflow();
	#endif
	}
};

template <> class LargeIntRegMultiply <signed __int64, signed __int32>
{
public:
    static bool RegMultiply( const signed __int64& a, signed __int32 b, signed __int64* pRet ) SAFEINT_NOTHROW
    {
#if SAFEINT_USE_INTRINSICS
        return IntrinsicMultiplyInt64( a, (signed __int64)b, pRet );
#else
        bool aNegative = false;
        bool bNegative = false;

        unsigned __int64 tmp;
        __int64 a1 = a;
        __int64 b1 = b;

        if( a1 < 0 )
        {
            aNegative = true;
            a1 = (signed __int64)AbsValueHelper< signed __int64, GetAbsMethod< signed __int64 >::method >::Abs(a1);
        }

        if( b1 < 0 )
        {
            bNegative = true;
            b1 = (signed __int64)AbsValueHelper< signed __int64, GetAbsMethod< signed __int64 >::method >::Abs(b1);
        }

        if( LargeIntRegMultiply< unsigned __int64, unsigned __int32 >::RegMultiply( (unsigned __int64)a1, (unsigned __int32)b1, &tmp ) )
        {
            // The unsigned multiplication didn't overflow
            if( aNegative ^ bNegative )
            {
                // Result must be negative
                if( tmp <= (unsigned __int64)IntTraits< signed __int64 >::minInt )
                {
                    *pRet = SignedNegation< signed __int64 >::Value( tmp );
                    return true;
                }
            }
            else
            {
                // Result must be positive
                if( tmp <= (unsigned __int64)IntTraits< signed __int64 >::maxInt )
                {
                    *pRet = (signed __int64)tmp;
                    return true;
                }
            }
        }

        return false;
#endif
    }

    template < typename E >
    static void RegMultiplyThrow( signed __int64 a, signed __int32 b, signed __int64* pRet ) SAFEINT_CPP_THROW
    {
#if SAFEINT_USE_INTRINSICS
        if( !IntrinsicMultiplyInt64( a, (signed __int64)b, pRet ) )
            E::SafeIntOnOverflow();
#else
        bool aNegative = false;
        bool bNegative = false;

        unsigned __int64 tmp;

        if( a < 0 )
        {
            aNegative = true;
            a = (signed __int64)AbsValueHelper< signed __int64, GetAbsMethod< signed __int64 >::method >::Abs(a);
        }

        if( b < 0 )
        {
            bNegative = true;
            b = (signed __int32)AbsValueHelper< signed __int32, GetAbsMethod< signed __int32 >::method >::Abs(b);
        }

        LargeIntRegMultiply< unsigned __int64, unsigned __int32 >::template RegMultiplyThrow< E >( (unsigned __int64)a, (unsigned __int32)b, &tmp );

        // The unsigned multiplication didn't overflow
        if( aNegative ^ bNegative )
        {
            // Result must be negative
            if( tmp <= (unsigned __int64)IntTraits< signed __int64 >::minInt )
            {
                *pRet = SignedNegation< signed __int64 >::Value( tmp );
                return;
            }
        }
        else
        {
            // Result must be positive
            if( tmp <= (unsigned __int64)IntTraits< signed __int64 >::maxInt )
            {
                *pRet = (signed __int64)tmp;
                return;
            }
        }

        E::SafeIntOnOverflow();
#endif
}
};

template <> class LargeIntRegMultiply <signed __int32, signed __int64>
{
public:
    static bool RegMultiply( signed __int32 a, const signed __int64& b, signed __int32* pRet ) SAFEINT_NOTHROW
    {
#if SAFEINT_USE_INTRINSICS
        __int64 tmp;

        if( IntrinsicMultiplyInt64( a, b, &tmp ) )
        {
            if( tmp > IntTraits< signed __int32 >::maxInt ||
                tmp < IntTraits< signed __int32 >::minInt )
            {
                return false;
            }

            *pRet = (__int32)tmp;
            return true;
        }
        return false;
#else
        bool aNegative = false;
        bool bNegative = false;

        unsigned __int32 tmp;
        __int64 b1 = b;

        if( a < 0 )
        {
            aNegative = true;
            a = (signed __int32)AbsValueHelper< signed __int32, GetAbsMethod< signed __int32 >::method >::Abs(a);
        }

        if( b1 < 0 )
        {
            bNegative = true;
            b1 = (signed __int64)AbsValueHelper< signed __int64, GetAbsMethod< signed __int64 >::method >::Abs(b1);
        }

        if( LargeIntRegMultiply< unsigned __int32, unsigned __int64 >::RegMultiply( (unsigned __int32)a, (unsigned __int64)b1, &tmp ) )
        {
            // The unsigned multiplication didn't overflow
            if( aNegative ^ bNegative )
            {
                // Result must be negative
                if( tmp <= (unsigned __int32)IntTraits< signed __int32 >::minInt )
                {
                    *pRet = SignedNegation< signed __int32 >::Value( tmp );
                    return true;
                }
            }
            else
            {
                // Result must be positive
                if( tmp <= (unsigned __int32)IntTraits< signed __int32 >::maxInt )
                {
                    *pRet = (signed __int32)tmp;
                    return true;
                }
            }
        }

        return false;
#endif
    }

    template < typename E >
    static void RegMultiplyThrow( signed __int32 a, const signed __int64& b, signed __int32* pRet ) SAFEINT_CPP_THROW
    {
#if SAFEINT_USE_INTRINSICS
        __int64 tmp;

        if( IntrinsicMultiplyInt64( a, b, &tmp ) )
        {
            if( tmp > IntTraits< signed __int32 >::maxInt ||
                tmp < IntTraits< signed __int32 >::minInt )
            {
                E::SafeIntOnOverflow();
            }

            *pRet = (__int32)tmp;
            return;
        }
        E::SafeIntOnOverflow();
#else
        bool aNegative = false;
        bool bNegative = false;

        unsigned __int32 tmp;
        signed __int64 b2 = b;

        if( a < 0 )
        {
            aNegative = true;
            a = (signed __int32)AbsValueHelper< signed __int32, GetAbsMethod< signed __int32 >::method >::Abs(a);
        }

        if( b < 0 )
        {
            bNegative = true;
            b2 = (signed __int64)AbsValueHelper< signed __int64, GetAbsMethod< signed __int64 >::method >::Abs(b2);
        }

        LargeIntRegMultiply< unsigned __int32, unsigned __int64 >::template RegMultiplyThrow< E >( (unsigned __int32)a, (unsigned __int64)b2, &tmp );

        // The unsigned multiplication didn't overflow
        if( aNegative ^ bNegative )
        {
            // Result must be negative
            if( tmp <= (unsigned __int32)IntTraits< signed __int32 >::minInt )
            {
                *pRet = SignedNegation< signed __int32 >::Value( tmp );
                return;
            }
        }
        else
        {
            // Result must be positive
            if( tmp <= (unsigned __int32)IntTraits< signed __int32 >::maxInt )
            {
                *pRet = (signed __int32)tmp;
                return;
            }
        }

        E::SafeIntOnOverflow();
#endif
}
};

template <> class LargeIntRegMultiply <signed __int64, unsigned __int64>
{
public:
    // Leave this one as-is - will call unsigned intrinsic internally
    static bool RegMultiply( const signed __int64& a, const unsigned __int64& b, signed __int64* pRet ) SAFEINT_NOTHROW
    {
        bool aNegative = false;

        unsigned __int64 tmp;
        __int64 a1 = a;

        if( a1 < 0 )
        {
            aNegative = true;
            a1 = (signed __int64)AbsValueHelper< signed __int64, GetAbsMethod< signed __int64 >::method >::Abs(a1);
        }

        if( LargeIntRegMultiply< unsigned __int64, unsigned __int64 >::RegMultiply( (unsigned __int64)a1, (unsigned __int64)b, &tmp ) )
        {
            // The unsigned multiplication didn't overflow
            if( aNegative )
            {
                // Result must be negative
                if( tmp <= (unsigned __int64)IntTraits< signed __int64 >::minInt )
                {
                    *pRet = SignedNegation< signed __int64 >::Value( tmp );
                    return true;
                }
            }
            else
            {
                // Result must be positive
                if( tmp <= (unsigned __int64)IntTraits< signed __int64 >::maxInt )
                {
                    *pRet = (signed __int64)tmp;
                    return true;
                }
            }
        }

        return false;
    }

    template < typename E >
    static void RegMultiplyThrow( const signed __int64& a, const unsigned __int64& b, signed __int64* pRet ) SAFEINT_CPP_THROW
    {
        bool aNegative = false;
        unsigned __int64 tmp;
        __int64 a1 = a;

        if( a1 < 0 )
        {
            aNegative = true;
            a1 = (signed __int64)AbsValueHelper< signed __int64, GetAbsMethod< signed __int64 >::method >::Abs(a1);
        }

        if( LargeIntRegMultiply< unsigned __int64, unsigned __int64 >::RegMultiply( (unsigned __int64)a1, (unsigned __int64)b, &tmp ) )
        {
            // The unsigned multiplication didn't overflow
            if( aNegative )
            {
                // Result must be negative
                if( tmp <= (unsigned __int64)IntTraits< signed __int64 >::minInt )
                {
                    *pRet = SignedNegation< signed __int64 >::Value( tmp );
                    return;
                }
            }
            else
            {
                // Result must be positive
                if( tmp <= (unsigned __int64)IntTraits< signed __int64 >::maxInt )
                {
                    *pRet = (signed __int64)tmp;
                    return;
                }
            }
        }

        E::SafeIntOnOverflow();
}
};

/*
 * Properly transition types.
 * The methods need __int64, __int32,
 * but variables being passed to us could be long long, long int, or long, depeding on the compiler
 * Microsoft compiler knows that long long is the same type as __int64, but gcc does not
 */

template < typename T, typename U > class MultiplicationHelper< T, U, MultiplicationState_Uint64Uint64>
{
public:
	static bool Multiply(const T& t, const U& u, T& ret) SAFEINT_NOTHROW
	{
		C_ASSERT(IntTraits<T>::isUint64 && IntTraits<U>::isUint64);
		unsigned __int64 t1 = t;
		unsigned __int64 u1 = u;
		return LargeIntRegMultiply< unsigned __int64, unsigned __int64 >::RegMultiply(t1,u1,reinterpret_cast<unsigned __int64*>(&ret));
	}

	template < typename E >
	static void MultiplyThrow(const unsigned __int64& t, const unsigned __int64& u, T& ret) SAFEINT_CPP_THROW
	{
		C_ASSERT(IntTraits<T>::isUint64 && IntTraits<U>::isUint64);
		unsigned __int64 t1 = t;
		unsigned __int64 u1 = u;
		LargeIntRegMultiply< unsigned __int64, unsigned __int64>::template RegMultiplyThrow<E>(t1,u1,reinterpret_cast<unsigned __int64*>(&ret));
	}
};

template < typename T, typename U > class MultiplicationHelper< T, U, MultiplicationState_Uint64Uint >
{
public:
    // T is unsigned __int64
    // U is any unsigned int 32-bit or less
    static bool Multiply( const T& t, const U& u, T& ret ) SAFEINT_NOTHROW
    {
        C_ASSERT( IntTraits<T>::isUint64 );
        unsigned __int64 t1 = t;
        return LargeIntRegMultiply< unsigned __int64, unsigned __int32 >::RegMultiply( t1, (unsigned __int32)u, reinterpret_cast<unsigned __int64*>(&ret) );
    }

    template < typename E >
    static void MultiplyThrow( const T& t, const U& u, T& ret ) SAFEINT_CPP_THROW
    {
        C_ASSERT( IntTraits<T>::isUint64 );
        unsigned __int64 t1 = t;
        LargeIntRegMultiply< unsigned __int64, unsigned __int32 >::template RegMultiplyThrow< E >( t1, (unsigned __int32)u, reinterpret_cast<unsigned __int64*>(&ret) );
    }
};

// converse of the previous function
template < typename T, typename U > class MultiplicationHelper< T, U, MultiplicationState_UintUint64 >
{
public:
    // T is any unsigned int up to 32-bit
    // U is unsigned __int64
    static bool Multiply(const T& t, const U& u, T& ret) SAFEINT_NOTHROW
    {
        C_ASSERT( IntTraits<U>::isUint64 );
        unsigned __int64 u1 = u;
        unsigned __int32 tmp;

        if( LargeIntRegMultiply< unsigned __int32, unsigned __int64 >::RegMultiply( t, u1, &tmp ) &&
            SafeCastHelper< T, unsigned __int32, GetCastMethod< T, unsigned __int32 >::method >::Cast(tmp, ret) )
        {
            return true;
        }

        return false;
    }

    template < typename E >
    static void MultiplyThrow(const T& t, const U& u, T& ret) SAFEINT_CPP_THROW
    {
        C_ASSERT( IntTraits<U>::isUint64 );
        unsigned __int64 u1 = u;
        unsigned __int32 tmp;

        LargeIntRegMultiply< unsigned __int32, unsigned __int64 >::template RegMultiplyThrow< E >( t, u1, &tmp );
        SafeCastHelper< T, unsigned __int32, GetCastMethod< T, unsigned __int32 >::method >::template CastThrow< E >(tmp, ret);
    }
};

template < typename T, typename U > class MultiplicationHelper< T, U, MultiplicationState_Uint64Int >
{
public:
    // T is unsigned __int64
    // U is any signed int, up to 64-bit
    static bool Multiply(const T& t, const U& u, T& ret) SAFEINT_NOTHROW
    {
        C_ASSERT( IntTraits<T>::isUint64 );
        unsigned __int64 t1 = t;
        return LargeIntRegMultiply< unsigned __int64, signed __int32 >::RegMultiply(t1, (signed __int32)u, reinterpret_cast< unsigned __int64* >(&ret));
    }

    template < typename E >
    static void MultiplyThrow(const T& t, const U& u, T& ret) SAFEINT_CPP_THROW
    {
        C_ASSERT( IntTraits<T>::isUint64 );
        unsigned __int64 t1 = t;
        LargeIntRegMultiply< unsigned __int64, signed __int32 >::template RegMultiplyThrow< E >(t1, (signed __int32)u, reinterpret_cast< unsigned __int64* >(&ret));
    }
};

template < typename T, typename U > class MultiplicationHelper< T, U, MultiplicationState_Uint64Int64 >
{
public:
    // T is unsigned __int64
    // U is __int64
    static bool Multiply(const T& t, const U& u, T& ret) SAFEINT_NOTHROW
    {
        C_ASSERT( IntTraits<T>::isUint64 && IntTraits<U>::isInt64 );
        unsigned __int64 t1 = t;
        __int64          u1 = u;
        return LargeIntRegMultiply< unsigned __int64, __int64 >::RegMultiply(t1, u1, reinterpret_cast< unsigned __int64* >(&ret));
    }

    template < typename E >
    static void MultiplyThrow(const T& t, const U& u, T& ret) SAFEINT_CPP_THROW
    {
        C_ASSERT( IntTraits<T>::isUint64 && IntTraits<U>::isInt64 );
        unsigned __int64 t1 = t;
        __int64          u1 = u;
        LargeIntRegMultiply< unsigned __int64, __int64 >::template RegMultiplyThrow< E >(t1, u1, reinterpret_cast< unsigned __int64* >(&ret));
    }
};

template < typename T, typename U > class MultiplicationHelper< T, U, MultiplicationState_UintInt64 >
{
public:
    // T is unsigned up to 32-bit
    // U is __int64
    static bool Multiply(const T& t, const U& u, T& ret) SAFEINT_NOTHROW
    {
        C_ASSERT( IntTraits<U>::isInt64 );
        __int64          u1 = u;
        unsigned __int32 tmp;

        if( LargeIntRegMultiply< unsigned __int32, __int64 >::RegMultiply( (unsigned __int32)t, u1, &tmp ) &&
            SafeCastHelper< T, unsigned __int32, GetCastMethod< T, unsigned __int32 >::method >::Cast(tmp, ret) )
        {
            return true;
        }

        return false;
    }

    template < typename E >
    static void MultiplyThrow(const T& t, const U& u, T& ret) SAFEINT_CPP_THROW
    {
        C_ASSERT( IntTraits<U>::isInt64 );
        __int64          u1 = u;
        unsigned __int32 tmp;

        LargeIntRegMultiply< unsigned __int32, __int64 >::template RegMultiplyThrow< E >( (unsigned __int32)t, u1, &tmp );
        SafeCastHelper< T, unsigned __int32, GetCastMethod< T, unsigned __int32 >::method >::template CastThrow< E >(tmp, ret);
    }
};

template < typename T, typename U > class MultiplicationHelper< T, U, MultiplicationState_Int64Uint >
{
public:
    // T is __int64
    // U is unsigned up to 32-bit
    static bool Multiply( const T& t, const U& u, T& ret ) SAFEINT_NOTHROW
    {
        C_ASSERT( IntTraits<T>::isInt64 );
        __int64          t1 = t;
        return LargeIntRegMultiply< __int64, unsigned __int32 >::RegMultiply( t1, (unsigned __int32)u, reinterpret_cast< __int64* >(&ret) );
    }

    template < typename E >
    static void MultiplyThrow( const T& t, const U& u, T& ret ) SAFEINT_CPP_THROW
    {
        C_ASSERT( IntTraits<T>::isInt64 );
        __int64          t1 = t;
        LargeIntRegMultiply< __int64, unsigned __int32 >::template RegMultiplyThrow< E >( t1, (unsigned __int32)u, reinterpret_cast< __int64* >(&ret) );
    }
};

template < typename T, typename U > class MultiplicationHelper< T, U, MultiplicationState_Int64Int64 >
{
public:
    // T, U are __int64
    static bool Multiply( const T& t, const U& u, T& ret ) SAFEINT_NOTHROW
    {
        C_ASSERT( IntTraits<T>::isInt64 && IntTraits<U>::isInt64 );
        __int64          t1 = t;
        __int64          u1 = u;
        return LargeIntRegMultiply< __int64, __int64 >::RegMultiply( t1, u1, reinterpret_cast< __int64* >(&ret) );
    }

    template < typename E >
    static void MultiplyThrow( const T& t, const U& u, T& ret ) SAFEINT_CPP_THROW
    {
        C_ASSERT( IntTraits<T>::isInt64 && IntTraits<U>::isInt64 );
        __int64          t1 = t;
        __int64          u1 = u;
        LargeIntRegMultiply< __int64, __int64 >::template RegMultiplyThrow< E >( t1, u1, reinterpret_cast< __int64* >(&ret));
    }
};

template < typename T, typename U > class MultiplicationHelper< T, U, MultiplicationState_Int64Int >
{
public:
    // T is __int64
    // U is signed up to 32-bit
    static bool Multiply( const T& t, U u, T& ret ) SAFEINT_NOTHROW
    {
        C_ASSERT( IntTraits<T>::isInt64 );
        __int64          t1 = t;
        return LargeIntRegMultiply< __int64, __int32 >::RegMultiply( t1, (__int32)u, reinterpret_cast< __int64* >(&ret));
    }

    template < typename E >
    static void MultiplyThrow( const __int64& t, U u, T& ret ) SAFEINT_CPP_THROW
    {
        C_ASSERT( IntTraits<T>::isInt64 );
        __int64          t1 = t;
        LargeIntRegMultiply< __int64, __int32 >::template RegMultiplyThrow< E >(t1, (__int32)u, reinterpret_cast< __int64* >(&ret));
    }
};

template < typename T, typename U > class MultiplicationHelper< T, U, MultiplicationState_IntUint64 >
{
public:
    // T is signed up to 32-bit
    // U is unsigned __int64
    static bool Multiply(T t, const U& u, T& ret) SAFEINT_NOTHROW
    {
        C_ASSERT( IntTraits<U>::isUint64 );
        unsigned __int64 u1 = u;
        __int32 tmp;

        if( LargeIntRegMultiply< __int32, unsigned __int64 >::RegMultiply( (__int32)t, u1, &tmp ) &&
            SafeCastHelper< T, __int32, GetCastMethod< T, __int32 >::method >::Cast( tmp, ret ) )
        {
            return true;
        }

        return false;
    }

    template < typename E >
    static void MultiplyThrow(T t, const unsigned __int64& u, T& ret) SAFEINT_CPP_THROW
    {
        C_ASSERT( IntTraits<U>::isUint64 );
        unsigned __int64 u1 = u;
        __int32 tmp;

        LargeIntRegMultiply< __int32, unsigned __int64 >::template RegMultiplyThrow< E >( (__int32)t, u1, &tmp );
        SafeCastHelper< T, __int32, GetCastMethod< T, __int32 >::method >::template CastThrow< E >( tmp, ret );
    }
};

template < typename T, typename U > class MultiplicationHelper< T, U, MultiplicationState_Int64Uint64>
{
public:
    // T is __int64
    // U is unsigned __int64
    static bool Multiply( const T& t, const U& u, T& ret ) SAFEINT_NOTHROW
    {
        C_ASSERT( IntTraits<T>::isInt64 && IntTraits<U>::isUint64 );
        __int64          t1 = t;
        unsigned __int64 u1 = u;
        return LargeIntRegMultiply< __int64, unsigned __int64 >::RegMultiply( t1, u1, reinterpret_cast< __int64* >(&ret));
    }

    template < typename E >
    static void MultiplyThrow( const __int64& t, const unsigned __int64& u, T& ret ) SAFEINT_CPP_THROW
    {
        C_ASSERT( IntTraits<T>::isInt64 && IntTraits<U>::isUint64 );
        __int64          t1 = t;
        unsigned __int64 u1 = u;
        LargeIntRegMultiply< __int64, unsigned __int64 >::template RegMultiplyThrow< E >( t1, u1, reinterpret_cast< __int64* >(&ret) );
    }
};

template < typename T, typename U > class MultiplicationHelper< T, U, MultiplicationState_IntInt64>
{
public:
    // T is signed, up to 32-bit
    // U is __int64
    static bool Multiply( T t, const U& u, T& ret ) SAFEINT_NOTHROW
    {
        C_ASSERT( IntTraits<U>::isInt64 );
        __int64 u1 = u;
        __int32 tmp;

        if( LargeIntRegMultiply< __int32, __int64 >::RegMultiply( (__int32)t, u1, &tmp ) &&
            SafeCastHelper< T, __int32, GetCastMethod< T, __int32 >::method >::Cast( tmp, ret ) )
        {
            return true;
        }

        return false;
    }

    template < typename E >
    static void MultiplyThrow(T t, const U& u, T& ret) SAFEINT_CPP_THROW
    {
        C_ASSERT( IntTraits<U>::isInt64 );
        __int64 u1 = u;
        __int32 tmp;

        LargeIntRegMultiply< __int32, __int64 >::template RegMultiplyThrow< E >( (__int32)t, u1, &tmp );
        SafeCastHelper< T, __int32, GetCastMethod< T, __int32 >::method >::template CastThrow< E >( tmp, ret );
    }
};

/*--------------------------------------------------------------------------------------------------------------------------------------*/

enum DivisionState {
	DivisionState_OK,
	DivisionState_UnsignedSigned,
	DivisionState_SignedUnsigned32,
	DivisionState_SignedUnsigned64,
	DivisionState_SignedUnsigned,
	DivisionState_SignedSigned,
};

template <typename T, typename U> class DivisionMethod
{
public:
	enum
	{
		method = (SafeIntCompare<T,U>::isBothUnSigned ? DivisionState_OK :
				 (!IntTraits<T>::isSigned && IntTraits<U>::isSigned) ? DivisionState_UnsignedSigned :
			     (IntTraits<T>::isSigned && IntTraits<U>::isUint32
			    		 && IntTraits<T>::isLT64Bit) ? DivisionState_SignedUnsigned32 :
			     (IntTraits<T>::isSigned && IntTraits<U>::isUint64) ? DivisionState_SignedUnsigned64 :
			     (IntTraits<T>::isSigned && IntTraits<U>::isSigned) ? DivisionState_SignedUnsigned :
			    		 DivisionState_SignedSigned)
	};
};

template <typename T, typename U, int state> class DivisionHelper;
template <typename T, typename U> class DivisionHelper<T,U,DivisionState_OK>
{
public:
	static SafeIntError Divide(const T& t, const U& u, T& result) SAFEINT_NOTHROW
	{
		if (u == 0)
			return SafeIntDivideByZero;
		if (t == 0)
		{
			result = 0;
			return SafeIntNoError;
		}

		result = (T)(t/u);
		return SafeIntNoError;
	}

	template < typename E >
	static void DivideThrow( const T& t, const U& u, T& result ) SAFEINT_CPP_THROW
	{
	        if( u == 0 )
	            E::SafeIntOnDivZero();

	        if( t == 0 )
	        {
	            result = 0;
	            return;
	        }

	        result = (T)( t/u );
	}
};

template <typename T, typename U> class DivisionHelper <T, U, DivisionState_UnsignedSigned>
{
public:
	static SafeIntError Divide(const T& t, const U& u, T& result) SAFEINT_NOTHROW
	{
		if (u == 0)
			return SafeIntDivideByZero;
		if (t == 0)
		{
			result = 0;
			return SafeIntNoError;
		}

		if (u > 0)
		{
			result = (T)(t/u);
			return SafeIntNoError;
		}

		if (AbsValueHelper<U, GetAbsMethod<U>::method>::Abs(u) > t)
		{
			result = 0;
			return SafeIntNoError;
		}
		return SafeIntArithmeticOverflow;
	}

	template < typename E >
	static void DivideThrow( const T& t, const U& u, T& result ) SAFEINT_CPP_THROW
	{

	        if( u == 0 )
	            E::SafeIntOnDivZero();

	        if( t == 0 )
	        {
	            result = 0;
	            return;
	        }

	        if( u > 0 )
	        {
	            result = (T)( t/u );
	            return;
	        }

	        // it is always an error to try and divide an unsigned number by a negative signed number
	        // unless u is bigger than t
	        if( AbsValueHelper< U, GetAbsMethod< U >::method >::Abs( u ) > t )
	        {
	            result = 0;
	            return;
	        }

	        E::SafeIntOnOverflow();
	}
};

template <typename T, typename U> class DivisionHelper <T, U, DivisionState_SignedUnsigned32>
{
public:
	static SafeIntError Divide(const T& t, const U& u, T& result) SAFEINT_NOTHROW
	{
		if (u == 0)
			return SafeIntDivideByZero;

		if (t == 0)
		{
			result = 0;
			return SafeIntNoError;
		}

		if (t > 0)
			result = (T)(t/u);
		else
			result = (T)((__int64)t/(__int64)u);

		return SafeIntNoError;
	}

	template < typename E >
	static void DivideThrow( const T& t, const U& u, T& result ) SAFEINT_CPP_THROW
	{
	        if( u == 0 )
	        {
	            E::SafeIntOnDivZero();
	        }

	        if( t == 0 )
	        {
	            result = 0;
	            return;
	        }

	        if( t > 0 )
	            result = (T)( t/u );
	        else
	            result = (T)( (__int64)t/(__int64)u );
	}
};

template <typename T, typename U> class DivisionHelper<T, U, DivisionState_SignedUnsigned64>
{
public:
	static SafeIntError Divide(const T& t, const unsigned __int64& u, T& result) SAFEINT_NOTHROW
	{
		C_ASSERT(IntTraits<U>::isUint64);

		if (u == 0)
			return SafeIntDivideByZero;

		if (t == 0)
		{
			result = 0;
			return SafeIntNoError;
		}

		if (u <= (unsigned __int64)IntTraits<T>::maxInt)
		{
			if (CompileConst< sizeof( T ) < sizeof( __int64 )>::Value())
				result = (T)((int)t/(int)u);
			else
				result = (T)((__int64)t/(__int64)u);
		}
		else if (t == IntTraits<T>::minInt && u == (unsigned __int64)IntTraits<T>::minInt)
		{
			result = -1;
		}
		else
		{
			result = 0;
		}
		return SafeIntNoError;
	}

	template < typename E >
	static void DivideThrow( const T& t, const unsigned __int64& u, T& result ) SAFEINT_CPP_THROW
	{
	        C_ASSERT( IntTraits< U >::isUint64 );

	        if( u == 0 )
	        {
	            E::SafeIntOnDivZero();
	        }

	        if( t == 0 )
	        {
	            result = 0;
	            return;
	        }

	        if( u <= (unsigned __int64)IntTraits< T >::maxInt )
	        {
	            // Else u can safely be cast to T
	            if( CompileConst< sizeof( T ) < sizeof( __int64 ) >::Value() )
	                result = (T)( (int)t/(int)u );
	            else
	                result = (T)((__int64)t/(__int64)u);
	        }
	        else // Corner case
	        if( t == IntTraits< T >::minInt && u == (unsigned __int64)IntTraits< T >::minInt )
	        {
	            // Min int divided by it's own magnitude is -1
	            result = -1;
	        }
	        else
	        {
	            result = 0;
	        }
	}
};

template <typename T, typename U> class DivisionHelper <T, U, DivisionState_SignedUnsigned>
{
public:
    // T is any signed, U is unsigned and smaller than 32-bit
    // In this case, standard operator casting is correct
    static SafeIntError Divide( const T& t, const U& u, T& result ) SAFEINT_NOTHROW
    {
        if( u == 0 )
        {
            return SafeIntDivideByZero;
        }

        if( t == 0 )
        {
            result = 0;
            return SafeIntNoError;
        }

        result = (T)( t/u );
        return SafeIntNoError;
    }

    template < typename E >
    static void DivideThrow( const T& t, const U& u, T& result ) SAFEINT_CPP_THROW
    {
        if( u == 0 )
        {
            E::SafeIntOnDivZero();
        }

        if( t == 0 )
        {
            result = 0;
            return;
        }

        result = (T)( t/u );
    }
};

template <typename T, typename U> class DivisionHelper<T, U, DivisionState_SignedSigned>
{
public:
	static SafeIntError Divide (const T& t, const U& u, T& result) SAFEINT_NOTHROW
	{
		if (u == 0)
			return SafeIntDivideByZero;

		if (t == 0)
		{
			result = 0;
			return SafeIntNoError;
		}

		if (t == IntTraits<T>::minInt && u == (U)-1)
		{
			return SafeIntArithmeticOverflow;
		}
		result = (T)(t/u);
		return SafeIntNoError;
	}

	template < typename E >
	static void DivideThrow( const T& t, const U& u, T& result ) SAFEINT_CPP_THROW
	{
	        if(u == 0)
	        {
	            E::SafeIntOnDivZero();
	        }

	        if( t == 0 )
	        {
	            result = 0;
	            return;
	        }

	        // Must test for corner case
	        if( t == IntTraits< T >::minInt && u == (U)-1 )
	            E::SafeIntOnOverflow();

	        result = (T)( t/u );
	}
};

/*--------------------------------------------------------------------------------------------------------------------------------------*/

enum AdditionState
{
    AdditionState_CastIntCheckMax,
    AdditionState_CastUintCheckOverflow,
    AdditionState_CastUintCheckOverflowMax,
    AdditionState_CastUint64CheckOverflow,
    AdditionState_CastUint64CheckOverflowMax,
    AdditionState_CastIntCheckSafeIntMinMax,
    AdditionState_CastInt64CheckSafeIntMinMax,
    AdditionState_CastInt64CheckMax,
    AdditionState_CastUint64CheckSafeIntMinMax,
    AdditionState_CastUint64CheckSafeIntMinMax2,
    AdditionState_CastInt64CheckOverflow,
    AdditionState_CastInt64CheckOverflowSafeIntMinMax,
    AdditionState_CastInt64CheckOverflowMax,
    AdditionState_ManualCheckInt64Uint64,
    AdditionState_ManualCheck,
    AdditionState_Error
};

template< typename T, typename U >
class AdditionMethod
{
public:
    enum
    {
                 //unsigned-unsigned
        method = (IntRegion< T,U >::IntZone_UintLT32_UintLT32  ? AdditionState_CastIntCheckMax :
                 (IntRegion< T,U >::IntZone_Uint32_UintLT64)   ? AdditionState_CastUintCheckOverflow :
                 (IntRegion< T,U >::IntZone_UintLT32_Uint32)   ? AdditionState_CastUintCheckOverflowMax :
                 (IntRegion< T,U >::IntZone_Uint64_Uint)       ? AdditionState_CastUint64CheckOverflow :
                 (IntRegion< T,U >::IntZone_UintLT64_Uint64)   ? AdditionState_CastUint64CheckOverflowMax :
                 //unsigned-signed
                 (IntRegion< T,U >::IntZone_UintLT32_IntLT32)  ? AdditionState_CastIntCheckSafeIntMinMax :
                 (IntRegion< T,U >::IntZone_Uint32_IntLT64 ||
                  IntRegion< T,U >::IntZone_UintLT32_Int32)    ? AdditionState_CastInt64CheckSafeIntMinMax :
                 (IntRegion< T,U >::IntZone_Uint64_Int ||
                  IntRegion< T,U >::IntZone_Uint64_Int64)      ? AdditionState_CastUint64CheckSafeIntMinMax :
                 (IntRegion< T,U >::IntZone_UintLT64_Int64)    ? AdditionState_CastUint64CheckSafeIntMinMax2 :
                 //signed-signed
                 (IntRegion< T,U >::IntZone_IntLT32_IntLT32)   ? AdditionState_CastIntCheckSafeIntMinMax :
                 (IntRegion< T,U >::IntZone_Int32_IntLT64 ||
                  IntRegion< T,U >::IntZone_IntLT32_Int32)     ? AdditionState_CastInt64CheckSafeIntMinMax :
                 (IntRegion< T,U >::IntZone_Int64_Int ||
                  IntRegion< T,U >::IntZone_Int64_Int64)       ? AdditionState_CastInt64CheckOverflow :
                 (IntRegion< T,U >::IntZone_IntLT64_Int64)     ? AdditionState_CastInt64CheckOverflowSafeIntMinMax :
                 //signed-unsigned
                 (IntRegion< T,U >::IntZone_IntLT32_UintLT32)  ? AdditionState_CastIntCheckMax :
                 (IntRegion< T,U >::IntZone_Int32_UintLT32 ||
                  IntRegion< T,U >::IntZone_IntLT64_Uint32)    ? AdditionState_CastInt64CheckMax :
                 (IntRegion< T,U >::IntZone_Int64_UintLT64)    ? AdditionState_CastInt64CheckOverflowMax :
                 (IntRegion< T,U >::IntZone_Int64_Uint64)      ? AdditionState_ManualCheckInt64Uint64 :
                 (IntRegion< T,U >::IntZone_Int_Uint64)        ? AdditionState_ManualCheck :
                  AdditionState_Error)
    };
};

template <typename T, typename U, int method> class AdditionHelper;
template <typename T, typename U> class AdditionHelper <T, U, AdditionState_CastIntCheckMax>
{
public:
	static bool Addition(const T& lhs, const U& rhs, T& result) SAFEINT_NOTHROW
	{
		__int32 tmp = lhs + rhs;

		if (tmp <= (__int32)IntTraits<T>::maxInt)
		{
			result = (T)tmp;
			return true;
		}
		return false;
	}

	template < typename E >
	static void AdditionThrow( const T& lhs, const U& rhs, T& result ) SAFEINT_CPP_THROW
	{
	        //16-bit or less unsigned addition
	        __int32 tmp = lhs + rhs;

	        if( tmp <= (__int32)IntTraits< T >::maxInt )
	        {
	            result = (T)tmp;
	            return;
	        }

	        E::SafeIntOnOverflow();
	}
};

template <typename T, typename U> class AdditionHelper <T, U, AdditionState_CastUintCheckOverflow >
{
public:
	static bool Addition(const T& lhs, const U& rhs, T& result) SAFEINT_NOTHROW
	{
		unsigned __int32 tmp = (unsigned __int32)lhs + (unsigned __int32)rhs;
		if (tmp >= lhs)
		{
			result = (T)tmp;
			return true;
		}
		return false;
	}

	template < typename E >
	static void AdditionThrow( const T& lhs, const U& rhs, T& result ) SAFEINT_CPP_THROW
	{
	        // 32-bit or less - both are unsigned
	        unsigned __int32 tmp = (unsigned __int32)lhs + (unsigned __int32)rhs;

	        //we added didn't get smaller
	        if( tmp >= lhs )
	        {
	            result = (T)tmp;
	            return;
	        }
	        E::SafeIntOnOverflow();
	}
};

template <typename T, typename U> class AdditionHelper <T, U, AdditionState_CastUintCheckOverflowMax >
{
public:
	static bool Addition(const T& lhs, const U& rhs, T& result) SAFEINT_NOTHROW
	{
		unsigned __int32 tmp = (unsigned __int32)lhs + (unsigned __int32)rhs;
		if (tmp >= lhs && tmp <= IntTraits<T>::maxInt)
		{
			result = (T)tmp;
			return true;
		}
		return false;
	}

	template < typename E >
	static void AdditionThrow( const T& lhs, const U& rhs, T& result ) SAFEINT_CPP_THROW
	{
	        // 32-bit or less - both are unsigned
	        unsigned __int32 tmp = (unsigned __int32)lhs + (unsigned __int32)rhs;

	        //we added didn't get smaller
	        if( tmp >= lhs && tmp <= IntTraits<T>::maxInt)
	        {
	            result = (T)tmp;
	            return;
	        }
	        E::SafeIntOnOverflow();
	}
};

template <typename T, typename U> class AdditionHelper <T, U, AdditionState_CastUint64CheckOverflow >
{
public:
	static bool Addition(const T& lhs, const U& rhs, T& result) SAFEINT_NOTHROW
	{
		unsigned __int64 tmp = (unsigned __int64)lhs + (unsigned __int64)rhs;
		if (tmp >= lhs)
		{
			result = (T)tmp;
			return true;
		}
		return false;
	}

	template < typename E >
	static void AdditionThrow( const T& lhs, const U& rhs, T& result ) SAFEINT_CPP_THROW
	{
	        // 32-bit or less - both are unsigned
	        unsigned __int64 tmp = (unsigned __int64)lhs + (unsigned __int64)rhs;

	        //we added didn't get smaller
	        if( tmp >= lhs )
	        {
	            result = (T)tmp;
	            return;
	        }
	        E::SafeIntOnOverflow();
	}
};

template <typename T, typename U> class AdditionHelper <T, U, AdditionState_CastUint64CheckOverflowMax >
{
public:
	static bool Addition(const T& lhs, const U& rhs, T& result) SAFEINT_NOTHROW
	{
		unsigned __int64 tmp = (unsigned __int64)lhs + (unsigned __int64)rhs;
		if (tmp >= lhs && tmp <= IntTraits<T>::maxInt)
		{
			result = (T)tmp;
			return true;
		}
		return false;
	}

	template < typename E >
	static void AdditionThrow( const T& lhs, const U& rhs, T& result ) SAFEINT_CPP_THROW
	{
	        // 32-bit or less - both are unsigned
	        unsigned __int64 tmp = (unsigned __int64)lhs + (unsigned __int64)rhs;

	        //we added didn't get smaller
	        if( tmp >= lhs && tmp <= IntTraits<T>::maxInt)
	        {
	            result = (T)tmp;
	            return;
	        }
	        E::SafeIntOnOverflow();
	}
};

template <typename T, typename U> class AdditionHelper <T, U, AdditionState_CastIntCheckSafeIntMinMax>
{
public:
	static bool Addition(const T& lhs, const U& rhs, T& result) SAFEINT_NOTHROW
	{
		__int32 tmp = lhs + rhs;

		if (tmp <= (__int32)IntTraits<T>::maxInt && tmp >= (__int32)IntTraits<T>::minInt)
		{
			result = (T)tmp;
			return true;
		}
		return false;
	}

	template < typename E >
	static void AdditionThrow( const T& lhs, const U& rhs, T& result ) SAFEINT_CPP_THROW
	{
	        //16-bit or less unsigned addition
	        __int32 tmp = lhs + rhs;

	        if( tmp <= (__int32)IntTraits< T >::maxInt && tmp >= (__int32)IntTraits<T>::minInt)
	        {
	            result = (T)tmp;
	            return;
	        }

	        E::SafeIntOnOverflow();
	}
};

template <typename T, typename U> class AdditionHelper <T, U, AdditionState_CastInt64CheckSafeIntMinMax>
{
public:
	static bool Addition(const T& lhs, const U& rhs, T& result) SAFEINT_NOTHROW
	{
		__int64 tmp = (__int64)lhs + (__int64)rhs;

		if (tmp <= (__int64)IntTraits<T>::maxInt && tmp >= (__int64)IntTraits<T>::minInt)
		{
			result = (T)tmp;
			return true;
		}
		return false;
	}

	template < typename E >
	static void AdditionThrow( const T& lhs, const U& rhs, T& result ) SAFEINT_CPP_THROW
	{
	        //16-bit or less unsigned addition
	        __int64 tmp = (__int64)lhs + (__int64)rhs;

	        if( tmp <= (__int64)IntTraits< T >::maxInt && tmp >= (__int64)IntTraits<T>::minInt)
	        {
	            result = (T)tmp;
	            return;
	        }

	        E::SafeIntOnOverflow();
	}
};

template <typename T, typename U> class AdditionHelper <T, U, AdditionState_CastInt64CheckMax>
{
public:
	static bool Addition(const T& lhs, const U& rhs, T& result) SAFEINT_NOTHROW
	{
		__int64 tmp = (__int64)lhs + (__int64)rhs;

		if (tmp <= (__int64)IntTraits<T>::maxInt)
		{
			result = (T)tmp;
			return true;
		}
		return false;
	}

	template < typename E >
	static void AdditionThrow( const T& lhs, const U& rhs, T& result ) SAFEINT_CPP_THROW
	{
	        //16-bit or less unsigned addition
	        __int64 tmp = (__int64)lhs + (__int64)rhs;

	        if( tmp <= (__int64)IntTraits< T >::maxInt)
	        {
	            result = (T)tmp;
	            return;
	        }

	        E::SafeIntOnOverflow();
	}
};

template <typename T, typename U> class AdditionHelper <T, U, AdditionState_CastUint64CheckSafeIntMinMax>
{
public:
	static bool Addition(const T& lhs, const U& rhs, T& result) SAFEINT_NOTHROW
	{
		unsigned __int64 tmp;

		if (rhs < 0)
		{
			tmp = AbsValueHelper<U, GetAbsMethod<U>::method>::Abs(rhs);
			if (tmp <= lhs)
			{
				result = lhs - tmp;
				return true;
			}
		}
		else
		{
			tmp = (unsigned __int64)lhs + (unsigned __int64)rhs;
			if (tmp >= lhs)
			{
				result = (T)tmp;
				return true;
			}
		}
		return false;
	}

	template < typename E >
	static void AdditionThrow( const T& lhs, const U& rhs, T& result ) SAFEINT_CPP_THROW
	{
	        // lhs is unsigned __int64, rhs signed
	        unsigned __int64 tmp;

	        if( rhs < 0 )
	        {
	            // So we're effectively subtracting
	            tmp = AbsValueHelper< U, GetAbsMethod< U >::method >::Abs( rhs );

	            if( tmp <= lhs )
	            {
	                result = lhs - tmp;
	                return;
	            }
	        }
	        else
	        {
	            // now we know that rhs can be safely cast into an unsigned __int64
	            tmp = (unsigned __int64)lhs + (unsigned __int64)rhs;

	            // We added and it did not become smaller
	            if( tmp >= lhs )
	            {
	                result = (T)tmp;
	                return;
	            }
	        }

	        E::SafeIntOnOverflow();
	}
};

template <typename T, typename U> class AdditionHelper <T, U, AdditionState_CastUint64CheckSafeIntMinMax2>
{
public:
	static bool Addition(const T& lhs, const U& rhs, T& result) SAFEINT_NOTHROW
	{

		if (rhs < 0)
		{
			if (lhs >= ~(unsigned __int64)(rhs) + 1)
			{
				result = (T)(lhs + rhs);
				return true;
			}
		}
		else
		{
			unsigned __int64 tmp = (unsigned __int64)lhs + (unsigned __int64)rhs;
			if (tmp <= IntTraits<T>::maxInt)
			{
				result = (T)tmp;
				return true;
			}
		}
		return false;
	}

	template < typename E >
	static void AdditionThrow( const T& lhs, const U& rhs, T& result ) SAFEINT_CPP_THROW
	{
		if (rhs < 0)
				{
					if (lhs >= ~(unsigned __int64)(rhs) + 1)
					{
						result = (T)(lhs + rhs);
						return;
					}
				}
				else
				{
					unsigned __int64 tmp = (unsigned __int64)lhs + (unsigned __int64)rhs;
					if (tmp <= IntTraits<T>::maxInt)
					{
						result = (T)tmp;
						return;
					}
				}

	        E::SafeIntOnOverflow();
	}
};

template <typename T, typename U> class AdditionHelper <T, U, AdditionState_CastInt64CheckOverflow>
{
public:
	static bool Addition(const T& lhs, const U& rhs, T& result) SAFEINT_NOTHROW
	{
		__int64 tmp = (__int64)((unsigned __int64)lhs + (unsigned __int64)rhs);
		if (lhs >= 0)
		{
			if (rhs >= 0 && tmp < lhs)
			{
				return false;
			}
		}
		else
		{
			if (rhs < 0 && tmp > lhs)
				return false;
		}
		result = (T)tmp;
		return true;
	}

	template < typename E >
	static void AdditionThrow( const T& lhs, const U& rhs, T& result ) SAFEINT_CPP_THROW
	{

	        __int64 tmp = (__int64)((unsigned __int64)lhs + (unsigned __int64)rhs);

	        if( lhs >= 0 )
	        {
	            if( rhs >= 0 && tmp < lhs )
	                E::SafeIntOnOverflow();
	        }
	        else
	        {
	            if( rhs < 0 && tmp > lhs )
	                E::SafeIntOnOverflow();
	        }

	        result = (T)tmp;
	}
};

template <typename T, typename U> class AdditionHelper <T, U, AdditionState_CastInt64CheckOverflowSafeIntMinMax>
{
public:
	static bool Addition(const T& lhs, const U& rhs, T& result) SAFEINT_NOTHROW
	{
		__int64 tmp;
		if (AdditionHelper<__int64,__int64, AdditionState_CastInt64CheckOverflow>::Addition((__int64)lhs,(__int64)rhs, tmp) &&
			tmp <= IntTraits<T>::maxInt && tmp >= IntTraits<T>::minInt)
		{
			result = (T)tmp;
			return true;
		}
		return false;
	}

	template <typename E>
	static void AdditionThrow(const T& lhs, const U& rhs, T& result) SAFEINT_CPP_THROW
	{
		__int64 tmp;
		AdditionHelper<__int64,__int64, AdditionState_CastInt64CheckOverflow>::Addition((__int64)lhs,(__int64)rhs, tmp);
		if (tmp <= IntTraits<T>::maxInt && tmp >= IntTraits<T>::minInt)
		{
			result = (T)tmp;
			return;
		}
		E::SafeIntOnOverflow();
	}
};

template <typename T, typename U> class AdditionHelper<T, U, AdditionState_CastInt64CheckOverflowMax>
{
public:
	static bool Addition(const T& lhs, const U& rhs, T& result) SAFEINT_NOTHROW
	{
		unsigned __int64 tmp = (unsigned __int64)lhs + (unsigned __int64)rhs;
		if ((__int64)tmp >= lhs)
		{
			result = (T)(__int64)tmp;
			return true;
		}
		return false;
	}

	template <typename E>
	static void AdditionThrow(const T& lhs, const U& rhs, T& result) SAFEINT_CPP_THROW
	{
		unsigned __int64 tmp = (unsigned __int64)lhs + (unsigned __int64)rhs;
	    if ((__int64)tmp >= lhs)
		{
			result = (T)(__int64)tmp;
			return;
		}
	    E::SafeIntOnOverflow();
	}
};

template <typename T, typename U> class AdditionHelper<T, U, AdditionState_ManualCheckInt64Uint64>
{
public:
	static bool Addition(const __int64& lhs, const unsigned __int64& rhs, __int64& result) SAFEINT_NOTHROW
	{
		C_ASSERT(IntTraits<T>::isInt64 && IntTraits<U>::isUint64);
		unsigned __int64 tmp = (unsigned __int64)lhs + rhs;
		if ((__int64)tmp >= lhs)
		{
			result = (__int64)tmp;
			return true;
		}
		return false;
	}

	template <typename E>
	static void AdditionThrow(const __int64& lhs, const unsigned __int64& rhs, T& result) SAFEINT_CPP_THROW
	{
		C_ASSERT(IntTraits<T>::isInt64 && IntTraits<U>::isUint64);
		unsigned __int64 tmp = (unsigned __int64)lhs + rhs;
		if ((__int64)tmp >= lhs)
		{
			result = (__int64)tmp;
			return;
		}
	    E::SafeIntOnOverflow();
	}
};

template <typename T, typename U> class AdditionHelper <T, U, AdditionState_ManualCheck>
{
public:
	static bool Addition (const T& lhs, const U& rhs, T& result) SAFEINT_NOTHROW
	{
		if ((unsigned __int32)(rhs >> 32) == 0)
		{
			unsigned __int32 tmp = (unsigned __int32)rhs + (unsigned __int32)lhs;
			if ((__int32)tmp >= lhs && SafeCastHelper<T, __int32, GetCastMethod<T, __int32>::method>::Cast((__int32)tmp, result))
				return true;
		}
		return false;
	}

	template <typename E>
	static void AdditionThrow(const T& lhs, const U& rhs, T& result) SAFEINT_CPP_THROW
	{
		if ((unsigned __int32)(rhs >> 32) == 0)
		{
			unsigned __int32 tmp = (unsigned __int32)rhs + (unsigned __int32)lhs;
			if ((__int32)tmp >= lhs && SafeCastHelper<T, __int32, GetCastMethod<T, __int32>::method>::Cast((__int32)tmp, result))
				return;
		}
		E::SafeIntOnOverflow();
	}
};

/*--------------------------------------------------------------------------------------------------------------------------------------*/

enum SubtractionState
{
    SubtractionState_BothUnsigned,
    SubtractionState_CastIntCheckSafeIntMinMax,
    SubtractionState_CastIntCheckMin,
    SubtractionState_CastInt64CheckSafeIntMinMax,
    SubtractionState_CastInt64CheckMin,
    SubtractionState_Uint64Int,
    SubtractionState_UintInt64,
    SubtractionState_Int64Int,
    SubtractionState_IntInt64,
    SubtractionState_Int64Uint,
    SubtractionState_IntUint64,
    SubtractionState_Int64Uint64,
    // states for SubtractionMethod2
    SubtractionState_BothUnsigned2,
    SubtractionState_CastIntCheckSafeIntMinMax2,
    SubtractionState_CastInt64CheckSafeIntMinMax2,
    SubtractionState_Uint64Int2,
    SubtractionState_UintInt642,
    SubtractionState_Int64Int2,
    SubtractionState_IntInt642,
    SubtractionState_Int64Uint2,
    SubtractionState_IntUint642,
    SubtractionState_Int64Uint642,
    SubtractionState_Error
};

template < typename T, typename U > class SubtractionMethod
{
public:
    enum
    {
                 // unsigned-unsigned
        method = ((IntRegion< T,U >::IntZone_UintLT32_UintLT32 ||
                 (IntRegion< T,U >::IntZone_Uint32_UintLT64)   ||
                 (IntRegion< T,U >::IntZone_UintLT32_Uint32)   ||
                 (IntRegion< T,U >::IntZone_Uint64_Uint)       ||
                 (IntRegion< T,U >::IntZone_UintLT64_Uint64))      ? SubtractionState_BothUnsigned :
                 // unsigned-signed
                 (IntRegion< T,U >::IntZone_UintLT32_IntLT32)      ? SubtractionState_CastIntCheckSafeIntMinMax :
                 (IntRegion< T,U >::IntZone_Uint32_IntLT64 ||
                  IntRegion< T,U >::IntZone_UintLT32_Int32)        ? SubtractionState_CastInt64CheckSafeIntMinMax :
                 (IntRegion< T,U >::IntZone_Uint64_Int ||
                  IntRegion< T,U >::IntZone_Uint64_Int64)          ? SubtractionState_Uint64Int :
                 (IntRegion< T,U >::IntZone_UintLT64_Int64)        ? SubtractionState_UintInt64 :
                 // signed-signed
                 (IntRegion< T,U >::IntZone_IntLT32_IntLT32)       ? SubtractionState_CastIntCheckSafeIntMinMax :
                 (IntRegion< T,U >::IntZone_Int32_IntLT64 ||
                  IntRegion< T,U >::IntZone_IntLT32_Int32)         ? SubtractionState_CastInt64CheckSafeIntMinMax :
                 (IntRegion< T,U >::IntZone_Int64_Int ||
                  IntRegion< T,U >::IntZone_Int64_Int64)           ? SubtractionState_Int64Int :
                 (IntRegion< T,U >::IntZone_IntLT64_Int64)         ? SubtractionState_IntInt64 :
                 // signed-unsigned
                 (IntRegion< T,U >::IntZone_IntLT32_UintLT32)      ? SubtractionState_CastIntCheckMin :
                 (IntRegion< T,U >::IntZone_Int32_UintLT32 ||
                  IntRegion< T,U >::IntZone_IntLT64_Uint32)        ? SubtractionState_CastInt64CheckMin :
                 (IntRegion< T,U >::IntZone_Int64_UintLT64)        ? SubtractionState_Int64Uint :
                 (IntRegion< T,U >::IntZone_Int_Uint64)            ? SubtractionState_IntUint64 :
                 (IntRegion< T,U >::IntZone_Int64_Uint64)          ? SubtractionState_Int64Uint64 :
                  SubtractionState_Error)
    };
};

template < typename T, typename U > class SubtractionMethod2
{
public:
    enum
    {
                 // unsigned-unsigned
        method = ((IntRegion< T,U >::IntZone_UintLT32_UintLT32 ||
                 (IntRegion< T,U >::IntZone_Uint32_UintLT64)   ||
                 (IntRegion< T,U >::IntZone_UintLT32_Uint32)   ||
                 (IntRegion< T,U >::IntZone_Uint64_Uint)       ||
                 (IntRegion< T,U >::IntZone_UintLT64_Uint64))     ? SubtractionState_BothUnsigned2 :
                 // unsigned-signed
                 (IntRegion< T,U >::IntZone_UintLT32_IntLT32)     ? SubtractionState_CastIntCheckSafeIntMinMax2 :
                 (IntRegion< T,U >::IntZone_Uint32_IntLT64 ||
                  IntRegion< T,U >::IntZone_UintLT32_Int32)       ? SubtractionState_CastInt64CheckSafeIntMinMax2 :
                 (IntRegion< T,U >::IntZone_Uint64_Int ||
                  IntRegion< T,U >::IntZone_Uint64_Int64)         ? SubtractionState_Uint64Int2 :
                 (IntRegion< T,U >::IntZone_UintLT64_Int64)       ? SubtractionState_UintInt642 :
                 // signed-signed
                 (IntRegion< T,U >::IntZone_IntLT32_IntLT32)      ? SubtractionState_CastIntCheckSafeIntMinMax2 :
                 (IntRegion< T,U >::IntZone_Int32_IntLT64 ||
                  IntRegion< T,U >::IntZone_IntLT32_Int32)        ? SubtractionState_CastInt64CheckSafeIntMinMax2 :
                 (IntRegion< T,U >::IntZone_Int64_Int ||
                  IntRegion< T,U >::IntZone_Int64_Int64)          ? SubtractionState_Int64Int2 :
                 (IntRegion< T,U >::IntZone_IntLT64_Int64)        ? SubtractionState_IntInt642 :
                 // signed-unsigned
                 (IntRegion< T,U >::IntZone_IntLT32_UintLT32)     ? SubtractionState_CastIntCheckSafeIntMinMax2 :
                 (IntRegion< T,U >::IntZone_Int32_UintLT32 ||
                  IntRegion< T,U >::IntZone_IntLT64_Uint32)       ? SubtractionState_CastInt64CheckSafeIntMinMax2 :
                 (IntRegion< T,U >::IntZone_Int64_UintLT64)       ? SubtractionState_Int64Uint2 :
                 (IntRegion< T,U >::IntZone_Int_Uint64)           ? SubtractionState_IntUint642 :
                 (IntRegion< T,U >::IntZone_Int64_Uint64)         ? SubtractionState_Int64Uint642 :
                                                                    SubtractionState_Error)
    };
};

template <typename T, typename U, int method> class SubtractionHelper;
template <typename T, typename U> class SubtractionHelper <T, U, SubtractionState_BothUnsigned>
{
public:
	static bool Subtract(const T& lhs, const U& rhs, T& result) SAFEINT_NOTHROW
	{
		if (rhs <= lhs)
		{
			result = (T)(lhs-rhs);
			return true;
		}
		return false;
	}

	template < typename E >
	static void SubtractThrow( const T& lhs, const U& rhs, T& result ) SAFEINT_CPP_THROW
	{
	        // both are unsigned - easy case
	        if( rhs <= lhs )
	        {
	            result = (T)( lhs - rhs );
	            return;
	        }

	        E::SafeIntOnOverflow();
	}
};

template <typename T, typename U> class SubtractionHelper <T, U, SubtractionState_BothUnsigned2>
{
public:
	static bool Subtract(const T& lhs, const U& rhs, U& result) SAFEINT_NOTHROW
	{
		if (rhs <= lhs)
		{
			T tmp = (T)(lhs - rhs);
			return SafeCastHelper<U, T, GetCastMethod<U,T>::method>::Cast(tmp,result);
		}
		return false;
	}

	template < typename E >
	static void SubtractThrow( const T& lhs, const U& rhs, U& result ) SAFEINT_CPP_THROW
	{
	        // both are unsigned - easy case
	        if( rhs <= lhs )
	        {
	            T tmp = (T)(lhs - rhs);
	            SafeCastHelper< U, T, GetCastMethod<U, T>::method >::template CastThrow<E>( tmp, result);
	            return;
	        }
	        E::SafeIntOnOverflow();
	}
};

template <typename T, typename U> class SubtractionHelper <T, U, SubtractionState_CastIntCheckSafeIntMinMax>
{
public:
	static bool Subtract(const T& lhs, const U& rhs, T& result) SAFEINT_NOTHROW
	{
		if (rhs <= lhs)
		{
			__int32 tmp = (lhs - rhs);
			if (SafeCastHelper<T, __int32, GetCastMethod<T, __int32>::method>::Cast(tmp,result))
			{
				result = (T)tmp;
				return true;
			}
		}
		return false;
	}

	template < typename E >
	static void SubtractThrow( const T& lhs, const U& rhs, U& result ) SAFEINT_CPP_THROW
	{
	       __int32 tmp = (lhs - rhs);
	       SafeCastHelper< T, __int32, GetCastMethod<T, __int32>::method >::template CastThrow<E>( tmp, result);
	}
};

template <typename U, typename T> class SubtractionHelper <U, T, SubtractionState_CastIntCheckSafeIntMinMax2>
{
public:
	static bool Subtract(const U& lhs, const T& rhs, T& result) SAFEINT_NOTHROW
	{
			__int32 tmp = (lhs - rhs);
			return SafeCastHelper<T, __int32, GetCastMethod<T, __int32>::method>::Cast(tmp,result);
	}

	template < typename E >
	static void SubtractThrow( const U& lhs, const T& rhs, T& result ) SAFEINT_CPP_THROW
	{
	       __int32 tmp = (lhs - rhs);
	       SafeCastHelper< T, __int32, GetCastMethod<T, __int32>::method >::template CastThrow<E>( tmp, result);
	}
};

template <typename T, typename U> class SubtractionHelper<T, U, SubtractionState_CastIntCheckMin>
{
public:
	static bool Subtract(const T& lhs, const U& rhs, T& result) SAFEINT_NOTHROW
	{
		__int32 tmp = lhs - rhs;
		if (tmp >= (__int32)IntTraits<T>::minInt)
		{
			result = (T)tmp;
			return true;
		}
		return false;
	}

	template <typename E>
	static void SubtractThrow(const T& lhs, const U& rhs, T& result) SAFEINT_CPP_THROW
	{
		        __int32 tmp = lhs - rhs;
				if (tmp >= (__int32)IntTraits<T>::minInt)
				{
					result = (T)tmp;
					return;
				}
				E::SafeIntOnOverflow();
	}
};

template <typename T, typename U> class SubtractionHelper <T, U, SubtractionState_CastInt64CheckSafeIntMinMax>
{
public:
	static bool Subtract(const T& lhs, const U& rhs, T& result) SAFEINT_NOTHROW
	{
			__int64 tmp = (__int64)lhs - (__int64)rhs;
			return SafeCastHelper<T, __int64, GetCastMethod<T, __int64>::method>::Cast(tmp,result);
	}

	template < typename E >
	static void SubtractThrow( const T& lhs, const U& rhs, T& result ) SAFEINT_CPP_THROW
	{
		   __int64 tmp = (__int64)lhs - (__int64)rhs;
	       SafeCastHelper< T, __int64, GetCastMethod<T, __int64>::method >::template CastThrow<E>( tmp, result);
	}
};

template <typename U, typename T> class SubtractionHelper <U, T, SubtractionState_CastInt64CheckSafeIntMinMax2>
{
public:
	static bool Subtract(const U& lhs, const T& rhs, T& result) SAFEINT_NOTHROW
	{
		   __int64 tmp = (__int64)lhs - (__int64)rhs;
		   return SafeCastHelper<T, __int64, GetCastMethod<T, __int64>::method>::Cast(tmp,result);
	}

	template < typename E >
	static void SubtractThrow( const U& lhs, const T& rhs, T& result ) SAFEINT_CPP_THROW
	{
		   __int64 tmp = (__int64)lhs - (__int64)rhs;
		   SafeCastHelper< T, __int64, GetCastMethod<T, __int64>::method >::template CastThrow<E>( tmp, result);
	}
};

template <typename T, typename U> class SubtractionHelper<T, U, SubtractionState_CastInt64CheckMin>
{
public:
	static bool Subtract(const T& lhs, const U& rhs, T& result) SAFEINT_NOTHROW
	{
		__int64 tmp = (__int64)lhs - (__int64)rhs;
		if (tmp >= (__int64)IntTraits<T>::minInt)
		{
			result = (T)tmp;
			return true;
		}
		return false;
	}

	template <typename E>
	static void SubtractThrow(const T& lhs, const U& rhs, T& result) SAFEINT_CPP_THROW
	{
		        __int64 tmp = (__int64)lhs - (__int64)rhs;
				if (tmp >= (__int64)IntTraits<T>::minInt)
				{
					result = (T)tmp;
					return;
				}
				E::SafeIntOnOverflow();
	}
};

template <typename T, typename U> class SubtractionHelper <T, U, SubtractionState_Uint64Int>
{
public:
	static bool Subtract(const T& lhs, const U& rhs, T& result) SAFEINT_NOTHROW
	{
		if (rhs >= 0)
		{
			if ((unsigned __int64)rhs <= lhs)
			{
				result = (T)(lhs - (unsigned __int64)rhs);
				return true;
			}
		}
		else
		{
			T tmp = lhs;
			result = lhs + AbsValueHelper<U, GetAbsMethod< U >::method>::Abs(rhs);
			if (result > tmp)
				return true;
		}
		return false;
	}

	template < typename E >
	static void SubtractThrow( const T& lhs, const U& rhs, T& result ) SAFEINT_CPP_THROW
	{
	        if( rhs >= 0 )
	        {
	            if( (unsigned __int64)rhs <= lhs )
	            {
	                result = (T)( lhs - (unsigned __int64)rhs );
	                return;
	            }
	        }
	        else
	        {
	            T tmp = lhs;
	            result = lhs + AbsValueHelper< U, GetAbsMethod< U >::method >::Abs( rhs );

	            if(result >= tmp)
	                return;
	        }

	        E::SafeIntOnOverflow();
	}
};

template <typename U, typename T> class SubtractionHelper <U, T, SubtractionState_Uint64Int2>
{
public:
	static bool Subtract(const U& lhs, const T& rhs, T& result) SAFEINT_NOTHROW
	{
		if (rhs < 0)
		{
			unsigned __int64 tmp;
			tmp = lhs + (unsigned __int64)AbsValueHelper<T, GetAbsMethod<T>::method>::Abs(rhs);
			if (tmp >= lhs && tmp <= IntTraits<T>::maxInt)
			{
				result = (T)tmp;
				return true;
			}
		}
		else if ((unsigned __int64)rhs > lhs)
		{
			/*
			 * Result is negative
			 * implies that lhs must fit into T, and result cannot overflow
			 * Allow us to drop to 32-bit math, which faster on a 32-bit system
			 */
			result = (T)lhs - (T)rhs;
			return true;
		}
		else
		{
			unsigned __int64 tmp = (unsigned __int64)lhs - (unsigned __int64)rhs;
			if (tmp <= IntTraits<T>::maxInt)
			{
				result = (T)tmp;
				return true;
			}
		}
		return false;
	}

	template < typename E >
	static void SubtractThrow( const U& lhs, const T& rhs, T& result ) SAFEINT_CPP_THROW
	{
	        // U is unsigned __int64, T is signed
	        if( rhs < 0 )
	        {
	            // treat this as addition
	            unsigned __int64 tmp;

	            tmp = lhs + (unsigned __int64)AbsValueHelper< T, GetAbsMethod< T >::method >::Abs( rhs );

	            // must check for addition overflow and max
	            if( tmp >= lhs && tmp <= IntTraits< T >::maxInt )
	            {
	                result = (T)tmp;
	                return;
	            }
	        }
	        else if( (unsigned __int64)rhs > lhs ) // now both are positive, so comparison always works
	        {
	            // result is negative
	            // implies that lhs must fit into T, and result cannot overflow
	            // Also allows us to drop to 32-bit math, which is faster on a 32-bit system
	            result = (T)lhs - (T)rhs;
	            return;
	        }
	        else
	        {
	            // result is positive
	            unsigned __int64 tmp = (unsigned __int64)lhs - (unsigned __int64)rhs;

	            if( tmp <= IntTraits< T >::maxInt )
	            {
	                result = (T)tmp;
	                return;
	            }
	        }

	        E::SafeIntOnOverflow();
	}
};

template <typename T, typename U> class SubtractionHelper <T, U, SubtractionState_UintInt64>
{
public:
	static bool Subtract(const T& lhs, const U& rhs, T& result) SAFEINT_NOTHROW
	{
		if (rhs >= 0)
		{
			if ((unsigned __int64)rhs <= lhs)
			{
				result = (T)lhs - (T)rhs;
				return true;
			}
		}
		else
		{
			unsigned __int64 tmp = lhs + ~(unsigned __int64)rhs + 1;
			if (tmp <= IntTraits<T>::maxInt)
			{
				result = (T)tmp;
				return true;
			}
		}
		return false;
	}

	template < typename E >
	static void SubtractThrow( const T& lhs, const U& rhs, T& result ) SAFEINT_CPP_THROW
	{
	        // lhs is an unsigned int32 or smaller, rhs signed __int64
	        // must first see if rhs is positive or negative
	        if( rhs >= 0 )
	        {
	            if( (unsigned __int64)rhs <= lhs )
	            {
	                result = (T)( lhs - (T)rhs );
	                return;
	            }
	        }
	        else
	        {
	            // we're now effectively adding
	            // since lhs is 32-bit, and rhs cannot exceed 2^63
	            // this addition cannot overflow
	            unsigned __int64 tmp = lhs + ~(unsigned __int64)( rhs ) + 1; // negation safe

	            // but we could exceed MaxInt
	            if(tmp <= IntTraits< T >::maxInt)
	            {
	                result = (T)tmp;
	                return;
	            }
	        }

	        E::SafeIntOnOverflow();
	}
};

template <typename U, typename T> class SubtractionHelper <U, T, SubtractionState_UintInt642>
{
public:
	static bool Subtract(const U& lhs, const T& rhs, T& result) SAFEINT_NOTHROW
	{
		if (rhs >= 0)
		{
		    result = (T)((__int64)lhs - rhs);
		    return true;
		}
		else
		{
			unsigned __int64 tmp = (unsigned __int64)lhs + (unsigned __int64)(-rhs);
			if (tmp <= (unsigned __int64)IntTraits<T>::maxInt)
			{
				result = (T)tmp;
				return true;
			}
		}
		return false;
	}

	template < typename E >
	static void SubtractThrow( const U& lhs, const T& rhs, T& result ) SAFEINT_CPP_THROW
	{
	        // U unsigned 32-bit or less, T __int64
	        if( rhs >= 0 )
	        {
	            // overflow not possible
	            result = (T)( (__int64)lhs - rhs );
	            return;
	        }
	        else
	        {
	            // we effectively have an addition
	            // which cannot overflow internally
	            unsigned __int64 tmp = (unsigned __int64)lhs + (unsigned __int64)( -rhs );

	            if( tmp <= (unsigned __int64)IntTraits< T >::maxInt )
	            {
	                result = (T)tmp;
	                return;
	            }
	        }

	        E::SafeIntOnOverflow();
	}
};

template <typename T, typename U> class SubtractionHelper <T, U, SubtractionState_Int64Int>
{
public:
	static bool Subtract(const T& lhs, const U& rhs, T& result) SAFEINT_NOTHROW
	{
		__int64 tmp = (__int64)((unsigned __int64)lhs - (unsigned __int64)rhs);
		if ((lhs >= 0 && rhs < 0 && tmp < lhs) ||
			(rhs >= 0 && tmp > lhs))
		{
			return false;
		}

		result = (T)tmp;
		return true;
	}

	template <typename E>
	static void SubtractionThrow(const T& lhs, const U& rhs, T& result) SAFEINT_CPP_THROW
	{
		__int64 tmp = (__int64)((unsigned __int64)lhs - (unsigned __int64)rhs);
		if ((lhs >= 0 && rhs < 0 && tmp < lhs) ||
			(rhs >= 0 && tmp > lhs))
		{
			E::SafeIntOnOverflow();
		}
		result = (T)tmp;
	}
};

template <typename U, typename T> class SubtractionHelper <U, T, SubtractionState_Int64Int2>
{
public:
	static bool Subtract(const U& lhs, const T& rhs, T& result) SAFEINT_NOTHROW
	{
		__int64 tmp = lhs - rhs;
		if (lhs >= 0)
		{
			if ((IntTraits<T>::isLT64Bit && tmp > IntTraits<T>::maxInt) ||
				(rhs < 0 && tmp < lhs))
			{
				return false;
			}
		}
		else
		{
			if ((IntTraits<T>::isLT64Bit && tmp < IntTraits<T>::minInt) ||
				(rhs >= 0 && tmp > lhs))
			{
				return false;
			}
		}

		result = (T)tmp;
		return true;
	}

	template < typename E >
	static void SubtractThrow( const U& lhs, const T& rhs, T& result ) SAFEINT_CPP_THROW
	{
	        __int64 tmp = lhs - rhs;

	        if( lhs >= 0 )
	        {
	            if( ( CompileConst< IntTraits< T >::isLT64Bit >::Value() && tmp > IntTraits< T >::maxInt ) ||
	                ( rhs < 0 && tmp < lhs ) )
	            {
	                E::SafeIntOnOverflow();
	            }
	        }
	        else
	        {
	            // lhs negative
	            if( ( CompileConst< IntTraits< T >::isLT64Bit >::Value() && tmp < IntTraits< T >::minInt) ||
	                ( rhs >=0 && tmp > lhs ) )
	            {
	                E::SafeIntOnOverflow();
	            }
	        }

	        result = (T)tmp;
	}
};

template <typename T, typename U> class SubtractionHelper <T, U, SubtractionState_IntInt64>
{
public:
	static bool Subtract(const T& lhs, const U& rhs, T& result) SAFEINT_NOTHROW
	{
		__int64 tmp = (__int64)((unsigned __int64)lhs - (unsigned __int64)rhs);
		if (lhs >= 0)
		{
			if (rhs >= 0)
			{
				if (tmp >= IntTraits<T>::minInt)
				{
					result = (T)tmp;
					return true;
				}
			}
			else
			{
				if (tmp >= lhs && tmp <= IntTraits<T>::maxInt)
				{
					result = (T)tmp;
					return true;
				}
			}
		}
		else
		{
			if (rhs >= 0)
			{
				if (tmp <= lhs && tmp >= IntTraits<T>::minInt)
				{
					result = (T)tmp;
					return true;
				}
			}
			else
			{
				if (tmp <= IntTraits<T>::maxInt)
				{
					result = (T)tmp;
					return true;
				}
			}
		}
		return false;
	}

	template < typename E >
	static void SubtractThrow( const T& lhs, const U& rhs, T& result ) SAFEINT_CPP_THROW
    {
	        __int64 tmp = (__int64)((unsigned __int64)lhs - (unsigned __int64)rhs);

	        if( lhs >= 0 )
	        {
	            // first case
	            if( rhs >= 0 )
	            {
	                if( tmp >= IntTraits< T >::minInt )
	                {
	                    result = (T)tmp;
	                    return;
	                }
	            }
	            else
	            {
	                // second case
	                if( tmp >= lhs && tmp <= IntTraits< T >::maxInt )
	                {
	                    result = (T)tmp;
	                    return;
	                }
	            }
	        }
	        else
	        {
	            // lhs < 0
	            // third case
	            if( rhs >= 0 )
	            {
	                if( tmp <= lhs && tmp >= IntTraits< T >::minInt )
	                {
	                    result = (T)tmp;
	                    return;
	                }
	            }
	            else
	            {
	                // fourth case
	                if( tmp <= IntTraits< T >::maxInt )
	                {
	                    result = (T)tmp;
	                    return;
	                }
	            }
	        }

	        E::SafeIntOnOverflow();
	}
};

template <typename U, typename T> class SubtractionHelper <U, T, SubtractionState_IntInt642>
{
public:
	static bool Subtract(const U& lhs, const T& rhs, T& result) SAFEINT_NOTHROW
	{
		__int64 tmp = (__int64)lhs - rhs;
		if ( (lhs > 0 && rhs < 0 && tmp < lhs) || (rhs > 0 && tmp > lhs))
			return false;
		result = (T)tmp;
		return true;
	}

	template < typename E >
	static void SubtractThrow( const U& lhs, const T& rhs, T& result ) SAFEINT_CPP_THROW
	{
	        __int64 tmp = (__int64)lhs - rhs;

	        if( ( lhs >= 0 && rhs < 0 && tmp < lhs ) ||
	            ( rhs > 0 && tmp > lhs ) )
	        {
	            E::SafeIntOnOverflow();
	            //else OK
	        }

	        result = (T)tmp;
	}
};

template <typename T, typename U> class SubtractionHelper <T, U, SubtractionState_Int64Uint>
{
public:
	static bool Subtract(const T& lhs, const U& rhs, T& result) SAFEINT_NOTHROW
	{
		unsigned __int64 tmp = (unsigned __int64)lhs - (unsigned __int64)rhs;
		if ((__int64)tmp <= lhs)
		{
			result = (T)(__int64)tmp;
			return true;
		}
		return false;
	}

	template < typename E >
	static void SubtractThrow( const T& lhs, const U& rhs, T& result ) SAFEINT_CPP_THROW
	{
	        // lhs is a 64-bit int, rhs unsigned int32 or smaller
	        // perform test as unsigned to prevent unwanted optimizations
	        unsigned __int64 tmp = (unsigned __int64)lhs - (unsigned __int64)rhs;

	        if( (__int64)tmp <= lhs )
	        {
	            result = (T)tmp;
	            return;
	        }

	        E::SafeIntOnOverflow();
	}
};

template <typename U, typename T> class SubtractionHelper <U, T, SubtractionState_Int64Uint2>
{
public:
	static bool Subtract(const U& lhs, const T& rhs, T& result) SAFEINT_NOTHROW
	{
		unsigned __int64 tmp = (unsigned __int64)lhs - (unsigned __int64)rhs;
		if ((__int64)tmp <= IntTraits<T>::maxInt && (__int64)tmp >= IntTraits<T>::minInt)
		{
			result = (T)(__int64)tmp;
			return true;
		}
		return false;
	}

	template < typename E >
	static void SubtractThrow( const U& lhs, const T& rhs, T& result ) SAFEINT_CPP_THROW
	{
	        // lhs is a 64-bit int, rhs unsigned int32 or smaller
	        // perform test as unsigned to prevent unwanted optimizations
	        unsigned __int64 tmp = (unsigned __int64)lhs - (unsigned __int64)rhs;

	        if ((__int64)tmp <= IntTraits<T>::maxInt && (__int64)tmp >= IntTraits<T>::minInt)
	        {
	        	result = (T)(__int64)tmp;
	        	return;
	        }

	        E::SafeIntOnOverflow();
	}
};

template <typename T, typename U> class SubtractionHelper <T, U, SubtractionState_IntUint64>
{
public:
	static bool Subtract(const T& lhs, const U& rhs, T& result) SAFEINT_NOTHROW
	{
		const unsigned __int64 AbsMinIntT = (unsigned __int64)IntTraits<T>::maxInt + 1;
		if (lhs < 0)
		{
			if (rhs <= AbsMinIntT - AbsValueHelper<T, GetAbsMethod<T>::method>::Abs(lhs))
			{
				result = (T)(lhs - rhs);
				return true;
			}
		}
		else
		{
			if (rhs <= AbsMinIntT + (unsigned __int64)lhs)
			{
				result = (T)(lhs - rhs);
				return true;
			}
		}
		return false;
	}

	template < typename E >
	static void SubtractThrow( const T& lhs, const U& rhs, T& result ) SAFEINT_CPP_THROW
	{
	        // lhs is any signed int, rhs unsigned int64
	        // check against available range

	        // We need the absolute value of IntTraits< T >::minInt
	        // This will give it to us without extraneous compiler warnings
	        const unsigned __int64 AbsMinIntT = (unsigned __int64)IntTraits< T >::maxInt + 1;

	        if( lhs < 0 )
	        {
	            if( rhs <= AbsMinIntT - AbsValueHelper< T, GetAbsMethod< T >::method >::Abs( lhs ) )
	            {
	                result = (T)( lhs - rhs );
	                return;
	            }
	        }
	        else
	        {
	            if( rhs <= AbsMinIntT + (unsigned __int64)lhs )
	            {
	                result = (T)( lhs - rhs );
	                return;
	            }
	        }

	        E::SafeIntOnOverflow();
	}
};

template <typename U, typename T> class SubtractionHelper <U, T, SubtractionState_IntUint642>
{
public:
	static bool Subtract(const U& lhs, const T& rhs, T& result) SAFEINT_NOTHROW
	{
		if (lhs >= 0 && (T)lhs >= rhs)
		{
			result = (T)((U)lhs - (U)rhs);
			return true;
		}
		return false;
	}

	template < typename E >
	static void SubtractThrow( const U& lhs, const T& rhs, T& result ) SAFEINT_CPP_THROW
	{
		if (lhs >= 0 && (T)lhs >= rhs)
		{
			result = (T)((U)lhs - (U)rhs);
			return;
		}
	        E::SafeIntOnOverflow();
	}
};

template <typename T, typename U> class SubtractionHelper <T, U, SubtractionState_Int64Uint64>
{
public:
	static bool Subtract(const __int64& lhs, const unsigned __int64& rhs, __int64& result) SAFEINT_NOTHROW
	{
		C_ASSERT(IntTraits<T>::isInt64 && IntTraits<U>::isUint64);
		unsigned __int64 tmp = (unsigned __int64)lhs - rhs;
		if ((__int64)tmp <= lhs)
		{
			result = (__int64)tmp;
			return true;
		}
		return false;
	}

	template <typename E>
	static void SubtractThrow(const __int64& lhs, const unsigned __int64& rhs, T& result) SAFEINT_CPP_THROW
	{
		C_ASSERT(IntTraits<T>::isInt64 && IntTraits<U>::isUint64);
	    unsigned __int64 tmp = (unsigned __int64)lhs - rhs;
		if ((__int64)tmp <= lhs)
		{
			result = (__int64)tmp;
			return;
		}
		E::SafeIntOnOverflow();
	}
};

template <typename U, typename T> class SubtractionHelper<U, T, SubtractionState_Int64Uint642>
{
public:
	static bool Subtract(const __int64& lhs, const unsigned __int64& rhs, T& result) SAFEINT_NOTHROW
	{
		C_ASSERT(IntTraits<T>::isInt64 && IntTraits<U>::isUint64);
		if (lhs >= 0 && (unsigned __int64)lhs >= rhs)
		{
			result = (unsigned __int64)lhs - rhs;
			return true;
		}
		return false;
	}

	template <typename E>
	static void SubtractThrow(const __int64& lhs, const unsigned __int64& rhs, T& result) SAFEINT_CPP_THROW
	{
		C_ASSERT(IntTraits<T>::isInt64 && IntTraits<U>::isUint64);
		if (lhs >= 0 && (unsigned __int64)lhs >= rhs)
		{
			result = (unsigned __int64)lhs - rhs;
			return;
		}
		E::SafeIntOnOverflow();
	}
};

/*--------------------------------------------------------------------------------------------------------------------------------------*/

enum BinaryState {
	BinaryState_OK,
	BinaryState_Int8,
	BinaryState_Int16,
	BinaryState_Int32
};

template <typename T, typename U> class BinaryMethod
{
public:
	enum {
		method = ( sizeof( T ) <= sizeof( U ) ||
		         SafeIntCompare< T, U >::isBothUnsigned ||
		         !IntTraits< U >::isSigned )          ? BinaryState_OK :
		         IntTraits< U >::isInt8               ? BinaryState_Int8 :
		         IntTraits< U >::isInt16              ? BinaryState_Int16
		                                              : BinaryState_Int32
	};
};

#ifdef SAFEINT_DISABLE_BINARY_ASSERT
#define BinaryAssert(x)
#else
#define BinaryAssert(x) SAFEINT_ASSERT(x)
#endif

template <typename T, typename U, int method> class BinaryAndHelper;
template <typename T, typename U> class BinaryAndHelper <T, U, BinaryState_OK>
{
public:
	static T And(T lhs, U rhs) SAFEINT_NOTHROW
	{
		return (T)(lhs & rhs);
	}
};

template <typename T, typename U> class BinaryAndHelper<T, U, BinaryState_Int8>
{
public:
	static T And(T lhs, U rhs) SAFEINT_NOTHROW
	{
		BinaryAssert((lhs & rhs) == (lhs & (unsigned __int8)rhs));
		return (T)(lhs & (unsigned __int8)rhs);
	}
};

template <typename T, typename U> class BinaryAndHelper<T, U, BinaryState_Int16>
{
public:
	static T And(T lhs, U rhs) SAFEINT_NOTHROW
	{
		BinaryAssert((lhs & rhs) == (lhs & (unsigned __int16)rhs));
		return (T)(lhs & (unsigned __int16)rhs);
	}
};

template <typename T, typename U> class BinaryAndHelper<T, U, BinaryState_Int32>
{
public:
	static T And(T lhs, U rhs) SAFEINT_NOTHROW
	{
		BinaryAssert((lhs & rhs) == (lhs & (unsigned __int32)rhs));
		return (T)(lhs & (unsigned __int32)rhs);
	}
};


template <typename T, typename U, int method> class BinaryOrHelper;
template <typename T, typename U> class BinaryOrHelper <T, U, BinaryState_OK>
{
public:
	static T Or(T lhs, U rhs) SAFEINT_NOTHROW
	{
		return (T)(lhs | rhs);
	}
};

template <typename T, typename U> class BinaryOrHelper <T, U, BinaryState_Int8>
{
public:
	static T Or(T lhs, U rhs) SAFEINT_NOTHROW
	{
		BinaryAssert((lhs | rhs) == (lhs | (unsigned __int8)rhs));
		return (T)(lhs | (unsigned __int8)rhs);
	}
};

template <typename T, typename U> class BinaryOrHelper <T, U, BinaryState_Int16>
{
public:
	static T Or(T lhs, U rhs) SAFEINT_NOTHROW
	{
		BinaryAssert((lhs | rhs) == (lhs | (unsigned __int16)rhs));
		return (T)(lhs | (unsigned __int16)rhs);
	}
};

template <typename T, typename U> class BinaryOrHelper <T, U, BinaryState_Int32>
{
public:
	static T Or(T lhs, U rhs) SAFEINT_NOTHROW
	{
		BinaryAssert((lhs | rhs) == (lhs | (unsigned __int32)rhs));
		return (T)(lhs | (unsigned __int32)rhs);
	}
};

template <typename T, typename U, int method> class BinaryXorHelper;
template <typename T, typename U> class BinaryXorHelper<T, U, BinaryState_OK>
{
public:
	static T Xor(T lhs, U rhs) SAFEINT_NOTHROW {return (T)(lhs ^ rhs);}
};

template <typename T, typename U> class BinaryXorHelper<T, U, BinaryState_Int8>
{
	static T Xor(T lhs, U rhs) SAFEINT_NOTHROW
	{
		BinaryAssert((lhs ^ rhs) == (lhs ^ (unsigned __int8)rhs));
		return (T)(lhs ^ (unsigned __int8)rhs);
	}
};

template <typename T, typename U> class BinaryXorHelper<T, U, BinaryState_Int16>
{
	static T Xor(T lhs, U rhs) SAFEINT_NOTHROW
	{
		BinaryAssert((lhs ^ rhs) == (lhs ^ (unsigned __int16)rhs));
		return (T)(lhs ^ (unsigned __int16)rhs);
	}
};

template <typename T, typename U> class BinaryXorHelper<T, U, BinaryState_Int32>
{
	static T Xor(T lhs, U rhs) SAFEINT_NOTHROW
	{
		BinaryAssert((lhs ^ rhs) == (lhs ^ (unsigned __int32)rhs));
		return (T)(lhs ^ (unsigned __int32)rhs);
	}
};

/*--------------------------------------------------------------------------------------------------------------------------------------*/

template <typename T, typename U>
inline bool SafeCast(const T From, U& To) SAFEINT_NOTHROW
{
	return SafeCastHelper<U, T, GetCastMethod<U, T>::method>::Cast(From, To);
}

template <typename T, typename U>
inline bool SafeEquals(const T t, const U u) SAFEINT_NOTHROW
{
	return EqualityTest<T, U, ValidComparison<T, U>::method>::IsEquals(t,u);
}

template <typename T, typename U>
inline bool SafeNoEquals(const T t, const U u) SAFEINT_NOTHROW
{
	return !EqualityTest<T, U, ValidComparison<T, U>::method>::IsEquals(t,u);
}

template <typename T, typename U>
inline bool SafeGreaterThan(const T t, const U u) SAFEINT_NOTHROW
{
	return GreaterThanTest<T, U, ValidComparison<T,U>::method>::GreaterThan(t,u);
}

template <typename T, typename U>
inline bool SafeGreaterThanEquals(const T t, const U u) SAFEINT_NOTHROW
{
	return !GreaterThanTest<T, U, ValidComparison<T,U>::method>::GreaterThan(t,u);
}

template <typename T, typename U>
inline bool SafeLessThan(const T t, const U u) SAFEINT_NOTHROW
{
	return GreaterThanTest<U, T, ValidComparison<U,T>::method>::GreaterThan(u,t);
}

template <typename T, typename U>
inline bool SafeLessThanEquals(const T t, const U u) SAFEINT_NOTHROW
{
	return !GreaterThanTest<T, U, ValidComparison<T,U>::method>::GreaterThan(t,u);
}

template <typename T, typename U>
inline bool SafeModulus(const T& t, const U& u, T& result) SAFEINT_NOTHROW
{
	return (ModulusHelper<T, U, ValidComparison<T, U>::method>::Modulus(t,u,result) == SafeIntNoError);
}

template <typename T, typename U>
inline bool SafeMultiply(T t, U u, T& result) SAFEINT_NOTHROW
{
	return MultiplicationHelper<T, U, MultiplicationMethod<T, U>::method>::Multiply(t,u,result);
}

template <typename T, typename U>
inline bool SafeDivide(T t, U u, T& result) SAFEINT_NOTHROW
{
	return (DivisionHelper<T, U, DivisionMethod<T, U>::method>::Divide(t, u, result) == SafeIntNoError);
}

template <typename T, typename U>
inline bool SafeAdd(T t, U u, T& result) SAFEINT_NOTHROW
{
	return AdditionHelper<T, U, AdditionMethod<T,U>::method>::Addition(t,u,result);
}

template <typename T, typename U>
inline bool SafeSubtract(T t, U u, T& result) SAFEINT_NOTHROW
{
	return SubtractionHelper<T, U, SubtractionMethod<T,U>::method>::Subtract(t,u.result);
}

/*--------------------------------------------------------------------------------------------------------------------------------------*/
template <typename T, typename E = SafeIntDefaultExceptionHandler> class SafeInt
{
private:
	T m_int;

	template <typename U>
	static SafeInt<T,E>SafeTtoI(U* input) SAFEINT_CPP_THROW
	{
		U* tmp = input;
		SafeInt<T,E> s;
		bool negative = false;

		if ( input == nullptr || input[0] == 0)
			E::SafeIntOnOverflow();

		switch (*tmp)
		{
		case '-':
			tmp++;
			negative = true;
			break;
		case '+':
			tmp++;
			break;
		}

		while (*tmp != 0)
		{
			if (*tmp < '0' || *tmp > '9')
				break;
			if ((T)s != 0)
				s *= (T)10;
			if (!negative)
				s += (T)(*tmp - '0');
			else
				s -= (T)(*tmp - '0');

			tmp++;
		}
		return 0;
	}
public:
	SafeInt() SAFEINT_NOTHROW
	{
		C_ASSERT(NumericType<T>::inInt);
		m_int = 0;
	}

	SafeInt(const T& i) SAFEINT_NOTHROW
	{
		C_ASSERT(NumericType<T>::inInt);
		m_int = i;
	}

	SafeInt(bool b) SAFEINT_NOTHROW
	{
		C_ASSERT(NumericType<T>::inInt);
		m_int = (T)(b ? 1 : 0);
	}

	template <typename U>
	SafeInt(const SafeInt<U,E>&u) SAFEINT_CPP_THROW
	{
		C_ASSERT(NumericType<T>::inInt);
		*this = SafeInt<T,E>((U)u);
	}

	template <typename U>
	SafeInt(const U& i) SAFEINT_CPP_THROW
	{
		C_ASSERT(NumericType<T>::inInt);
		SafeCastHelper<T, U, GetCastMethod<T,U>::method>::template CastThrow<E>(i,m_int);
	}

	/*
	 * Start overloading operators
	 * assignment operator
	 * constructions exist for all int types and will ensure safety
	 */

	template <typename U>
	SafeInt<T, E>& operator = (const U& rhs) SAFEINT_CPP_THROW
	{
		*this = SafeInt<T,E>(rhs);
		return *this;
	}

	SafeInt<T, E>& operator =(const T& rhs) SAFEINT_NOTHROW
	{
		m_int = rhs;
		return *this;
	}

	template <typename U>
	SafeInt<T, E>& operator = (const SafeInt<U,E>& rhs) SAFEINT_CPP_THROW
	{
		SafeCastHelper<T,U,GetCastMethod<T,U>::method>::template CastThrow<E> (rhs.Ref(), m_int);
		return *this;
	}

	SafeInt<T,E>& operator = (const SafeInt<T,E>& rhs) SAFEINT_NOTHROW
	{
		m_int = rhs.m_int;
		return *this;
	}

	operator bool() const SAFEINT_NOTHROW
	{
		return !m_int;
	}

	operator char() const SAFEINT_CPP_THROW
	{
		char val;
		SafeCastHelper<char,T,GetCastMethod<char,T>::method>::template CastThrow<E> (m_int,val);
		return val;
	}

	operator signed char() const SAFEINT_CPP_THROW
	{
		signed char val;
		SafeCastHelper<signed char, T, GetCastMethod<signed char, T>::method>::template CastThrow<E> (m_int, val);
		return val;
	}

	operator unsigned char() const SAFEINT_CPP_THROW
	{
		unsigned char val;
		SafeCastHelper<unsigned char, T, GetCastMethod<unsigned char, T>::method>::template CastThrow<E> (m_int, val);
		return val;
	}

	operator __int16() const SAFEINT_CPP_THROW
	{
		__int16 val;
		SafeCastHelper<__int16, T, GetCastMethod<__int16, T>::method>::template CastThrow<E> (m_int, val);
		return val;
	}

	operator unsigned __int16() const SAFEINT_CPP_THROW
	{
		unsigned __int16 val;
		SafeCastHelper<unsigned __int16, T, GetCastMethod<unsigned __int16, T>::method>::template CastThrow<E> (m_int, val);
		return val;
	}

	operator __int32() const SAFEINT_CPP_THROW
	{
		__int32 val;
		SafeCastHelper<__int32, T, GetCastMethod<__int32,T>::method>::template CastThrow<E> (m_int, val);
		return val;
	}

	operator unsigned __int32() const SAFEINT_CPP_THROW
	{
		unsigned __int32 val;
		SafeCastHelper<unsigned __int32, T, GetCastMethod<unsigned __int32,T>::method>::template CastThrow<E> (m_int, val);
		return val;
	}

	operator long() const SAFEINT_CPP_THROW
	{
		long val;
		SafeCastHelper<long, T, GetCastMethod<long, T>::method>::template CastThrow<E> (m_int, val);
		return val;
	}

	operator unsigned long() const SAFEINT_CPP_THROW
	{
		unsigned long val;
		SafeCastHelper<unsigned long, T, GetCastMethod<unsigned long, T>::method>::template CastThrow<E> (m_int, val);
		return val;
	}

	operator __int64() const SAFEINT_CPP_THROW
	{
		__int64 val;
		SafeCastHelper<__int64, T, GetCastMethod<__int64, T>::method>::template CastThrow<E> (m_int, val);
		return val;
	}

	operator unsigned __int64() const SAFEINT_CPP_THROW
	{
		unsigned __int64 val;
		SafeCastHelper<unsigned __int64, T, GetCastMethod<unsigned __int64, T>::method>::template CastThrow<E> (m_int, val);
		return val;
	}

#if defined SAFEINT_USE_WCHAR_T || defined _NATIVE_WCHAR_T_DEFINED
operator wchar_t() const SAFEINT_CPP_THROW
{
  wchar_t val;
  SafeCastHelper <wchar_t, T, GetCastMethod<wchar_t, T>::method>::template CastThrow<E> (m_int, val);
  return val;
}
#endif

#ifdef SIZE_T_CAST_NEEDED
operator size_t() const SAFEINT_CPP_THROW
{
  size_t val;
  SafeCastHelper <size_t, T, GetCastMethod<size_t, T>::method>::template CastThrow<E> (m_int, val);
  return val;
}
#endif

    operator float() const SAFEINT_CPP_THROW
    {
    	float val;
    	SafeCastHelper<float, T, GetCastMethod<float,T>::method>::template CastThrow<E> (m_int, val);
    	return val;
    }

    operator double() const SAFEINT_CPP_THROW
    {
    	double val;
    	SafeCastHelper<double, T, GetCastMethod<double,T>::method>::template CastThrow<E> (m_int, val);
    	return val;
    }

    operator long double() const SAFEINT_CPP_THROW
    {
    	long double val;
    	SafeCastHelper<long double, T, GetCastMethod<long double, T>::method>::template CastThrow<E> (m_int, val);
    	return val;
    }

    T *Ptr() SAFEINT_NOTHROW {return &m_int;}
    const T *Ptr() const SAFEINT_NOTHROW {return &m_int;}
    const T &Ptr() const SAFEINT_NOTHROW {return m_int;}

    /*
     * SafeInt <T,E>::Ptr() is inconvenient, use the overload operator &
     * Allow you to more easily pass a SafeInt into things like ReadFile
     * This is unsafe things
     */

    T *operator &() SAFEINT_NOTHROW {return &m_int;}
    const T *operator &() const SAFEINT_NOTHROW {return &m_int;}

    bool operator !() const SAFEINT_NOTHROW {return (!m_int) ? true : false;}

    const SafeInt<T,E>& operator +() const SAFEINT_NOTHROW {return *this;}

    const SafeInt<T,E>& operator -() const SAFEINT_CPP_THROW
    {
    	return SafeInt<T,E>(NegationHelper<T, IntTraits<T>::isSigned>::template NegativeThrow<E>(m_int));
    }

    SafeInt<T, E>& operator ++() SAFEINT_CPP_THROW
    {
    	if (m_int != IntTraits<T>::maxInt)
    	{
    		++m_int;
    		return *this;
    	}
    	E::SafeIntOnOverflow();
    }

    SafeInt<T, E>& operator --() SAFEINT_CPP_THROW
    {
    	if (m_int != IntTraits<T>::minInt)
    	{
    		--m_int;
    		return *this;
    	}
    	E::SafeIntOnOverflow();
    }

    SafeInt<T, E> operator ++(int) SAFEINT_CPP_THROW
    {
    	if (m_int != IntTraits<T>::maxInt)
    	{
    		SafeInt<T,E> tmp(m_int);
    		m_int++;
    		return tmp;
    	}
    	E::SafeIntOnOverflow();
    }

    SafeInt<T, E> operator--(int) SAFEINT_CPP_THROW
    {
    	if (m_int != IntTraits<T>::minInt)
    	{
    		SafeInt<T,E> tmp(m_int);
    		m_int--;
    	}
    	E::SafeIntOnOverflow();
    }

    SafeInt<T,E> operator ~() const SAFEINT_NOTHROW
    {
    	return SafeInt<T, E> ((T)~m_int);
    }

    template <typename U>
    SafeInt <T, E> operator %(U rhs) const SAFEINT_CPP_THROW
	{
    	T result;
    	ModulusHelper<T, U, ValidComparison<T,U>::method>::template ModulusThrow<E> (m_int, rhs, result);
    	return SafeInt<T,E>(result);
	}

    SafeInt<T, E> operator %(SafeInt<T,E>rhs) const SAFEINT_CPP_THROW
    {
    	T result;
    	ModulusHelper<T,T,ValidComparison<T,T>::method>::template ModulusThrow<E>(m_int, rhs, result);
    	return SafeInt<T,E>(result);
    }

    template <typename U>
    SafeInt <T,E>& operator %=(U rhs) SAFEINT_CPP_THROW
	{
    	ModulusHelper<T, U, ValidComparison<T,U>::method>::template ModulusThrow<E>(m_int, rhs, m_int);
    	return *this;
	}

    template <typename U>
    SafeInt <T, E>& operator %=(SafeInt<U,E> rhs) SAFEINT_CPP_THROW
	{
    	ModulusHelper<T, U, ValidComparison<T,U>::method>::template ModulusThrow<E>(m_int, (U)rhs, m_int);
    	return *this;
	}

    template <typename U>
    SafeInt<T, E> operator *(U rhs) const SAFEINT_CPP_THROW
	{
    	T ret(0);
        MultiplicationHelper<T, U, MultiplicationMethod<T,U>::method>::template MultiplyThrow<E> (m_int, rhs, ret);
        return SafeInt<T,E>(ret);
	}

    SafeInt<T, E> operator *(SafeInt<T,E>rhs) const SAFEINT_CPP_THROW
    {
    	T ret(0);
    	MultiplicationHelper<T, T, MultiplicationMethod<T,T>::method>::template MultiplyThrow<E> (m_int, (T)rhs, ret);
    	return SafeInt<T,E>(ret);
    }

    SafeInt<T,E>& operator *=(SafeInt<T,E>rhs) SAFEINT_CPP_THROW
    {
    	MultiplicationHelper <T, T, MultiplicationMethod<T,T>::method>::template MultiplyThrow<E> (m_int, (T)rhs, m_int);
    	return *this;
    }

    template <typename U>
    SafeInt<T,E>& operator *=(U rhs) SAFEINT_CPP_THROW
	{
    	MultiplicationHelper<T, U, MultiplicationMethod<T, U>::method>::template MultiplyThrow<E> (m_int, rhs, m_int);
    	return *this;
	}

    template <typename U>
    SafeInt<T,E>& operator *=(SafeInt<U, E> rhs) SAFEINT_CPP_THROW
	{
    	MultiplicationHelper<T, U, MultiplicationMethod<T, U>::method>::template MultiplyThrow<E> (m_int, rhs.Ref(), m_int);
    	return *this;
	}

    template <typename U>
    SafeInt<T, E> operator /(U rhs) const SAFEINT_CPP_THROW
	{
    	T ret(0);
    	DivisionHelper<T, U, DivisionMethod<T, U>::method>::template DivideThrow<E> (m_int, rhs, ret);
    	return SafeInt<T,E>(ret);
	}

    SafeInt<T,E> operator /(SafeInt<T,E> rhs) const SAFEINT_CPP_THROW
    {
    	T ret(0);
    	DivisionHelper<T, T, DivisionMethod<T, T>::method>::template DivideThrow<E> (m_int, (T)rhs, ret);
    	return SafeInt<T,E>(ret);
    }

    SafeInt<T,E>& operator /=(SafeInt<T,E> rhs) const SAFEINT_CPP_THROW
    {
    	DivisionHelper<T, T, DivisionMethod<T, T>::method>::template DivideThrow<E> (m_int, (T)rhs, m_int);
    	return *this;
    }

    template <typename U>
    SafeInt<T,E>& operator /=(U rhs) SAFEINT_CPP_THROW
    {
    	DivisionHelper<T, U, DivisionMethod<T, U>::method>::template DivideThrow<E> (m_int, rhs, m_int);
        return *this;
    }

    template <typename U>
    SafeInt<T,E>& operator /=(SafeInt<U, E> rhs) SAFEINT_CPP_THROW
    {
    	DivisionHelper<T, U, DivisionMethod<T, U>::method>::template DivideThrow<E> (m_int, (U)rhs, m_int);
        return *this;
    }

    SafeInt<T, E> operator +(SafeInt<T, E>rhs) const SAFEINT_CPP_THROW
    {
    	T ret(0);
    	AdditionHelper <T, T, AdditionMethod<T, T>::method>::template AdditionThrow<E>(m_int, (T)rhs, ret);
    	return SafeInt<T, E>(ret);
    }

    template <typename U>
    SafeInt<T, E> operator +(U rhs) const SAFEINT_CPP_THROW
	{
    	T ret(0);
    	AdditionHelper <T, U, AdditionMethod<T, U>::method>::template AdditionThrow<E>(m_int, rhs, ret);
    	return SafeInt<T,E>(ret);
	}

    SafeInt<T, E>& operator +=(SafeInt<T, E>rhs) const SAFEINT_CPP_THROW
    {
        AdditionHelper <T, T, AdditionMethod<T, T>::method>::template AdditionThrow<E>(m_int, (T)rhs, m_int);
        return *this;
    }

    template <typename U>
    SafeInt<T, E>& operator +=(U rhs) const SAFEINT_CPP_THROW
    {
        AdditionHelper <T, U, AdditionMethod<T, U>::method>::template AdditionThrow<E>(m_int, rhs, m_int);
        return *this;
    }

    template <typename U>
    SafeInt<T, E>& operator +=(SafeInt<U,E> rhs) const SAFEINT_CPP_THROW
    {
        AdditionHelper <T, U, AdditionMethod<T, U>::method>::template AdditionThrow<E>(m_int, (U)rhs, m_int);
        return *this;
    }

    template <typename U>
    SafeInt<T, E> operator -(U rhs) const SAFEINT_CPP_THROW
	{
    	T ret(0);
    	SubtractionHelper< T, U, SubtractionMethod< T, U >::method >::template SubtractThrow< E >( m_int, rhs, ret );
    	return SafeInt<T, E>(ret);
	}

    SafeInt<T, E> operator -(SafeInt<T,E> rhs) const SAFEINT_CPP_THROW
    {
        T ret(0);
        SubtractionHelper< T, T, SubtractionMethod< T, T >::method >::template SubtractThrow< E >( m_int, (T)rhs, ret );
        return SafeInt<T, E>(ret);
    }

    SafeInt<T, E>& operator -=(SafeInt<T,E> rhs) const SAFEINT_CPP_THROW
    {
        SubtractionHelper< T, T, SubtractionMethod< T, T >::method >::template SubtractThrow< E >( m_int, (T)rhs, m_int );
        return *this;
    }

    template <typename U>
    SafeInt<T, E>& operator -=(U rhs) const SAFEINT_CPP_THROW
    {
        SubtractionHelper< T, U, SubtractionMethod< T, U >::method >::template SubtractThrow< E >( m_int, rhs, m_int );
        return *this;
    }

    template <typename U>
    SafeInt<T, E>& operator -=(SafeInt<U, E> rhs) const SAFEINT_CPP_THROW
    {
        SubtractionHelper< T, U, SubtractionMethod< T, U >::method >::template SubtractThrow< E >( m_int, rhs, m_int );
        return *this;
    }

#ifdef SAFEINT_DISABLE_SHIFT_ASSERT
#define ShiftAssert(x)
#else
#define ShiftAssert(x) SAFEINT_ASSERT(x)
#endif

    template <typename U>
    SafeInt <T, E> operator << (U bits) const SAFEINT_NOTHROW
	{
    	ShiftAssert(!IntTraits<U>::isSigned || bits >= 0);
    	ShiftAssert(bits < (int)IntTraits<T>::bitCount);

    	return SafeInt<T, E>((T)(m_int << bits));
	}

    template <typename U>
    SafeInt<T, E> operator << (SafeInt<U, E> bits) const SAFEINT_NOTHROW
	{
    	ShiftAssert(!IntTraits<U>::isSigned || bits >= 0);
    	ShiftAssert(bits < (int)IntTraits<T>::bitCount);

    	return SafeInt<T, E>((T)(m_int << (U)bits));
	}

    template <typename U>
    SafeInt<T, E> operator <<= (U bits) SAFEINT_NOTHROW
	{
    	ShiftAssert(!IntTraits<U>::isSigned || bits >= 0);
    	ShiftAssert(bits < (int)IntTraits<T>::bitCount);

    	m_int <<= bits;
    	return *this;
	}

    template <typename U>
    SafeInt<T, E>& operator <<= (U bits) SAFEINT_NOTHROW
    {
        ShiftAssert(!IntTraits<U>::isSigned || bits >= 0);
        ShiftAssert(bits < (int)IntTraits<T>::bitCount);

        m_int <<= bits;
        return *this;
    }

    template <typename U>
    SafeInt<T, E>& operator <<= (SafeInt<U, E> bits) SAFEINT_NOTHROW
    {
        ShiftAssert(!IntTraits<U>::isSigned || bits >= 0);
        ShiftAssert(bits < (int)IntTraits<T>::bitCount);

        m_int <<= (U)bits;
        return *this;
    }

    template <typename U>
    SafeInt <T, E> operator >> (U bits) const SAFEINT_NOTHROW
    {
        ShiftAssert(!IntTraits<U>::isSigned || bits >= 0);
        ShiftAssert(bits < (int)IntTraits<T>::bitCount);

        return SafeInt<T, E>((T)(m_int >> bits));
    }

    template <typename U>
    SafeInt<T, E> operator >> (SafeInt<U, E> bits) const SAFEINT_NOTHROW
    {
        ShiftAssert(!IntTraits<U>::isSigned || bits >= 0);
        ShiftAssert(bits < (int)IntTraits<T>::bitCount);

        return SafeInt<T, E>((T)(m_int >> (U)bits));
    }

    template <typename U>
    SafeInt<T, E>& operator >>= (U bits) SAFEINT_NOTHROW
    {
        ShiftAssert(!IntTraits<U>::isSigned || bits >= 0);
        ShiftAssert(bits < (int)IntTraits<T>::bitCount);

        m_int >>= bits;
        return *this;
    }

    template <typename U>
    SafeInt<T, E>& operator >>= (SafeInt<U, E> bits) SAFEINT_NOTHROW
    {
        ShiftAssert(!IntTraits<U>::isSigned || bits >= 0);
        ShiftAssert(bits < (int)IntTraits<T>::bitCount);

        m_int >>= (U)bits;
        return *this;
    }

    SafeInt<T, E> operator &(SafeInt<T, E>rhs) const SAFEINT_NOTHROW
    {
    	return SafeInt<T, E>(m_int & (T)rhs);
    }

    template <typename U>
    SafeInt<T, E> operator &(U rhs) const SAFEINT_NOTHROW
	{
    	return SafeInt<T, E>(BinaryAndHelper<T, U, BinaryMethod<T, U>::method>::And(m_int, rhs));
	}

    SafeInt<T, E>& operator &=(SafeInt<T,E> rhs) SAFEINT_NOTHROW
    {
    	m_int &= (T)rhs;
    	return *this;
    }

    template <typename U>
    SafeInt<T, E>& operator &=(U rhs) SAFEINT_NOTHROW
    {
        m_int = (BinaryAndHelper<T, U, BinaryMethod<T, U>::method>::And(m_int, rhs));
        return *this;
    }

    template <typename U>
    SafeInt<T, E>& operator &=(SafeInt<U, E> rhs) SAFEINT_NOTHROW
    {
        m_int = (BinaryAndHelper<T, U, BinaryMethod<T, U>::method>::And(m_int, (U)rhs));
        return *this;
    }

    SafeInt<T, E> operator ^(SafeInt<T, E>rhs) const SAFEINT_NOTHROW
    {
        return SafeInt<T, E>((T)(m_int ^ (T)rhs));
    }

    template <typename U>
    SafeInt<T, E> operator ^(U rhs) const SAFEINT_NOTHROW
    {
        return SafeInt<T, E>(BinaryXorHelper<T, U, BinaryMethod<T, U>::method>::Xor(m_int, rhs));
    }

    SafeInt<T, E>& operator ^=(SafeInt<T,E> rhs) SAFEINT_NOTHROW
    {
        m_int ^= (T)rhs;
        return *this;
    }

    template <typename U>
    SafeInt<T, E>& operator ^=(U rhs) SAFEINT_NOTHROW
    {
        m_int = (BinaryXorHelper<T, U, BinaryMethod<T, U>::method>::Xor(m_int, rhs));
        return *this;
    }

    template <typename U>
    SafeInt<T, E>& operator ^=(SafeInt<U, E> rhs) SAFEINT_NOTHROW
    {
        m_int = (BinaryXorHelper<T, U, BinaryMethod<T, U>::method>::Xor(m_int, (U)rhs));
        return *this;
    }

    SafeInt<T, E> operator | (SafeInt<T, E> rhs) const SAFEINT_NOTHROW
    {
    	return SafeInt<T, E>((T)(m_int | (T)rhs));
    }

    template <typename U>
    SafeInt<T, E> operator | (U rhs) const SAFEINT_NOTHROW
    {
        return SafeInt<T, E>(BinaryOrHelper<T, U, BinaryMethod<T, U>::method>::Or(m_int, rhs));
    }

    SafeInt<T, E>& operator |= (SafeInt<T,E> rhs) SAFEINT_NOTHROW
    {
        m_int |= (T)rhs;
        return *this;
    }

    template <typename U>
    SafeInt<T, E>& operator |= (U rhs) SAFEINT_NOTHROW
    {
        m_int = (BinaryOrHelper<T, U, BinaryMethod<T, U>::method>::Or(m_int, rhs));
        return *this;
    }

    template <typename U>
    SafeInt<T, E>& operator |= (SafeInt<U, E> rhs) SAFEINT_NOTHROW
    {
        m_int = (BinaryOrHelper<T, U, BinaryMethod<T, U>::method>::Or(m_int, (U)rhs));
        return *this;
    }

    SafeInt<T, E> Min(SafeInt<T,E>test, const T floor = IntTraits<T>::minInt) const SAFEINT_NOTHROW
    {
    	T tmp = test < m_int ? (T)test : m_int;
    	return tmp < floor ? floor : tmp;
    }

    SafeInt<T, E> Max(SafeInt<T,E>test, const T upper = IntTraits<T>::maxInt) const SAFEINT_NOTHROW
    {
    	T tmp = test > m_int ? (T)test : m_int;
    	return tmp > upper ? upper : tmp;
    }

    void Swap (SafeInt<T, E>& with) SAFEINT_NOTHROW
    {
    	T temp(m_int);
    	m_int = with.m_int;
    	with.m_int = temp;
    }

    static SafeInt<T, E> SafeAtoI(const char* input) SAFEINT_CPP_THROW
    {
    	return SafeTtoI(input);
    }

    static SafeInt<T, E> SafeWtoI(const wchar_t* input)
	{
    	return SafeTtoI(input);
	}

    enum alignBits
    {
            align2 = 1,
            align4 = 2,
            align8 = 3,
            align16 = 4,
            align32 = 5,
            align64 = 6,
            align128 = 7,
            align256 = 8
    };
    
    template <alignBits bits>
    const SafeInt<T,E>& Align() SAFEINT_CPP_THROW
    {
    	if (m_int == 0)
    		return *this;
    	const T AlignValue = ((T)1 << bits) - 1;
    	m_int = (T)((m_int + AlignValue) & ~AlignValue);
    	
    	if (m_int <= 0)
    		E::SafeIntOnOverflow();
    	
    	return *this;
    }
    
    const SafeInt<T,E>& Align2() {return Align<align2>();}
    const SafeInt<T,E>& Align4() {return Align<align4>();}
    const SafeInt<T,E>& Align8() {return Align<align8>();}
    const SafeInt<T,E>& Align16() {return Align<align16>();}
    const SafeInt<T,E>& Align32() {return Align<align32>();}
    const SafeInt<T,E>& Align64() {return Align<align64>();}
    
};


/*--------------------------------------------------------------------------------------------------------------------------------------*/


}
}

#endif /* INCLUDE_CPPREST_DETAILS_SAFEINT3_HPP_ */
