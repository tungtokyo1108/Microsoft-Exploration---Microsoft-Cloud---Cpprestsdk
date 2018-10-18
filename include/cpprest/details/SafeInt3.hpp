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
	
}
}

#endif /* INCLUDE_CPPREST_DETAILS_SAFEINT3_HPP_ */
