#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef PRAGMA_ONCE
#pragma once
#endif

#ifdef _MSC_VER
	#ifndef PRAGMA_ONCE
	#define PRAGMA_ONCE
	#pragma once
	#endif
	#define CC_MSVC
#elif __GNUC__
	#define CC_GCC_MAIN
	#define CC_GCC_UNIX
#elif __clang__
	#define CC_CLANG
#elif __MINGW32__
	#define CC_GCC_MAIN
	#define CC_GCC_WIN32
#endif

#ifdef _cplusplus
	#define BEGIN_C extern "C" {
	#define END_C }
	#define BEGIN_CPP
	#define END_CPP
#else
	#define BEGIN_C
	#define END_C
	#define BEGIN_CPP extern "C++" {
	#define END_CPP }
#endif


#ifndef CC_MSVC
#undef 	BEGIN_CPP
#undef 	END_CPP
#define BEGIN_CPP
#define END_CPP
#endif

#ifdef MCVS

	#define EXPORT __declspec(dllexport)
	#define IMPORT __declspec(dllimport)
	
#endif

#define GLOBAL_EXPORT EXPORT

#ifdef CC_GCC_MAIN

	#define SINGLE_EXPORT
	#define SINGLE_IMPORT
	
#else
	
	#define BEGIN_EXPORT
	#define SINGLE_EXPORT EXPORT
	#define END_EXPORT
	
	#define BEGIN_IMPORT
	#define SINGLE_IMPORT IMPORT
	#define END_IMPORT
	
#endif

#ifdef _LINUX
#include <alloca.h>
#endif // _LINUX

#define ALIGN_VALUE( val, alignment ) ( ( val + alignment - 1 ) & ~( alignment - 1 ) ) //  need macro for constant expression

#define  stackalloc( _size ) _alloca( ALIGN_VALUE( _size, 16 ) )
#define  stackfree( _p )   0

#ifdef __cplusplus
	template <typename T >
	
	int NULLED( T type )
	{
		if ( type == nullprt || !type )
			return true;
		return false;
	}
#endif

#endif //PLATFORM_H