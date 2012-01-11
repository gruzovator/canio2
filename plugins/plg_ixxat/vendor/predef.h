/*****************************************************************************
 IXXAT Automation GmbH
******************************************************************************

 File    : PREDEF.H
 Summary : Compiler specific declarations.

 Date    : 2003-10-05
 Author  : Hartmut Heim

 Compiler: MSVC, Borland C, LabWindows/CVI

 Remarks : This file is shared between user and kernel mode components.
           Request a kernel mode developer before changing the contents
           of this file.

******************************************************************************
 all rights reserved
*****************************************************************************/

#ifndef _PREDEF_H_
#define _PREDEF_H_

/*****************************************************************************
 * compiler declarations
 ****************************************************************************/

#undef __INT8   //  8 bit integer
#undef __INT16  // 16 bit integer
#undef __INT32  // 32 bit integer
#undef __INT64  // 64 bit integer
#undef __SINT8  //  8 bit signed integer
#undef __SINT16 // 16 bit signed integer
#undef __SINT32 // 32 bit signed integer
#undef __SINT64 // 64 bit signed integer
#undef __UINT8  //  8 bit unsigned integer
#undef __UINT16 // 16 bit unsigned integer
#undef __UINT32 // 32 bit unsigned integer
#undef __UINT64 // 64 bit unsigned integer

//
// Microsoft C/C++ compiler
//
#ifdef _MSC_VER
#define MICROSOFT_C _MSC_VER
#if (_MSC_VER >= 1300)
#define __INT8            __int8
#define __INT16           __int16
#define __INT32           __int32
#else
#define __INT8              char
#define __INT16             short
#define __INT32             int
#endif
#define __INT64           __int64
#define __SINT8  signed   __INT8
#define __SINT16 signed   __INT16
#define __SINT32 signed   __INT32
#define __SINT64 signed   __INT64
#define __UINT8  unsigned __INT8
#define __UINT16 unsigned __INT16
#define __UINT32 unsigned __INT32
#define __UINT64 unsigned __INT64
#endif

//
// Borland C/C++ compiler
//
#ifdef __BORLANDC__
#define BORLAND_C __BORLANDC__
#define __INT8            __int8
#define __INT16           __int16
#define __INT32           __int32
#define __INT64           __int64
#define __SINT8  signed   __INT8
#define __SINT16 signed   __INT16
#define __SINT32 signed   __INT32
#define __SINT64 signed   __INT64
#define __UINT8  unsigned __INT8
#define __UINT16 unsigned __INT16
#define __UINT32 unsigned __INT32
#define __UINT64 unsigned __INT64
#endif

//
// National Instruments LabWindows/CVI compiler
//
#ifdef _CVI_
#define NILABWIN_C _CVI_
#define __INT8              char
#define __INT16             short
#define __INT32             int
#define __SINT8  signed   __INT8
#define __SINT16 signed   __INT16
#define __SINT32 signed   __INT32
#define __UINT8  unsigned __INT8
#define __UINT16 unsigned __INT16
#define __UINT32 unsigned __INT32
#if (_CVI_ > 600)
#define __INT64           __int64
#define __SINT64 signed   __INT64
#define __UINT64 unsigned __INT64
#else
#define __INT64           double
#define __SINT64          double
#define __UINT64          double
#endif
#endif


/*****************************************************************************
 * machine types
 ****************************************************************************/

#if defined(MICROSOFT_C)
#  if !defined(_M_IX86_) && defined(_M_IX86)
#    define _M_BITS 32
#    define _M_IX86_ _M_IX86
#  elif !defined(_M_MIPS_) && defined(_M_MRX000)
#    define _M_BITS 32
#    define _M_MIPS_ _M_MRX000
#  elif !defined(_M_ALPHA_) && defined(_M_ALPHA)
#    define _M_BITS 32
#    define _M_ALPHA_ _M_ALPHA
#  elif !defined(_M_PPC_) && defined(_M_PPC)
#    define _M_BITS 32
#    define _M_PPC_ _M_PPC
#  elif !defined(_M_M68K_) && defined(_M_M68K)
#    define _M_BITS 32
#    define _M_M68K_ _M_M68K
#  elif !defined(_M_MPPC_) && defined(_M_MPPC)
#    define _M_BITS 32
#    define _M_MPPC_ _M_MPPC
#  elif !defined(_M_IA64_) && defined(_M_IA64)
#    define _M_BITS 64
#    define _M_IA64_ _M_IA64
#  elif !defined(_M_X64_) && defined(_M_X64)
#    define _M_BITS 64
#    define _M_X64_ _M_X64
#  elif !defined(_M_AMD64_) && defined(_M_AMD64)
#    define _M_BITS 64
#    define _M_AMD64_ _M_AMD64
#  endif
#elif defined(BORLAND_C)
#  if !defined(_M_IX86_) && defined(_M_IX86)
#    define _M_BITS 32
#    define _M_IX86_ _M_IX86
#  endif
#elif defined(NILABWIN_C)
#  if !defined(_M_IX86_) && defined(_M_IX86)
#    define _M_BITS 32
#    define _M_IX86_ _M_IX86
#  endif
#endif

#ifndef _M_BITS
#error "Target architecture not defined"
#endif

/*****************************************************************************
 * general definitions
 ****************************************************************************/

#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif
#endif

#if (_MSC_VER >= 1300)
# if (_M_BITS == 32)
#  define ALIGN __declspec(align(4))
# elif (_M_BITS == 64)
#  define ALIGN __declspec(align(8))
# else
#  define ALIGN __declspec(align(1))
# endif
#else
# define ALIGN
#endif

#ifndef ALIGN1
# if (_MSC_VER >= 1300)
#  define ALIGN1 __declspec(align(1))
# else
#  define ALIGN1
# endif
#endif

#ifndef ALIGN2
# if (_MSC_VER >= 1300)
#  define ALIGN2 __declspec(align(2))
# else
#  define ALIGN2
# endif
#endif

#ifndef ALIGN4
# if (_MSC_VER >= 1300)
#  define ALIGN4 __declspec(align(4))
# else
#  define ALIGN4
# endif
#endif

#ifndef ALIGN8
# if (_MSC_VER >= 1300)
#  define ALIGN8 __declspec(align(8))
# else
#  define ALIGN8
# endif
#endif

#ifndef ALIGN16
# if (_MSC_VER >= 1300)
#  define ALIGN16 __declspec(align(16))
# else
#  define ALIGN16
# endif
#endif

#ifndef ALIGN32
# if (_MSC_VER >= 1300)
#  define ALIGN32 __declspec(align(32))
# else
#  define ALIGN32
# endif
#endif

#ifndef DECLSPEC_ALLOC
# if (_MSC_VER >= 1100)
#  define DECLSPEC_ALLOC(x) __declspec(allocate(x))
# else
#  define DECLSPEC_ALLOC(x)
# endif
#endif

#ifndef DECLSPEC_NAKED
# if defined (_M_IX86) && ((_MSC_VER >= 1100) || (__BORLANDC__ >= 0x550))
#  define DECLSPEC_NAKED __declspec(naked)
#  define NAKED          DECLSPEC_NAKED
# else
#  define DECLSPEC_NAKED
#  define NAKED
# endif
#endif

#ifndef DECLSPEC_SELECTANY
# if (_MSC_VER >= 1100) || (__BORLANDC__ >= 0x550)
#  define DECLSPEC_SELECTANY __declspec(selectany)
# else
#  define DECLSPEC_SELECTANY
# endif
#endif

#if (_MSC_FULL_VER >= 14002050)
# ifndef DECLSPEC_NOALIAS
#  define DECLSPEC_NOALIAS __declspec(noalias)
# endif
# ifndef DECLSPEC_RESTRICT
#  define DECLSPEC_RESTRICT __declspec(restrict)
# endif
#else
# ifndef DECLSPEC_NOALIAS
#  define DECLSPEC_NOALIAS
# endif
# ifndef DECLSPEC_RESTRICT
#  define DECLSPEC_RESTRICT
# endif
#endif

#define NOALIAS  DECLSPEC_NOALIAS
#define RESTRICT DECLSPEC_RESTRICT

#ifndef SCALL
#define SCALL __stdcall
#endif

#ifndef CCALL
#define CCALL __cdecl
#endif

#ifndef FCALL
#define FCALL __fastcall
#endif


#endif //_PREDEF_H_
