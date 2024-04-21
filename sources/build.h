/*
build.h - compile-time build information

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/
#pragma once
#ifndef BUILD_H
#define BUILD_H

/*
All BUILD_* macros set by this header are guaranteed to have positive value
otherwise not defined.

Every macro is intended to be the unified interface for buildsystems that lack
platform & CPU detection, and a neat quick way for checks in platform code
*/

#undef BUILD_64BIT
#undef BUILD_AMD64
#undef BUILD_ANDROID
#undef BUILD_APPLE
#undef BUILD_ARM
#undef BUILD_ARM_HARDFP
#undef BUILD_ARM_SOFTFP
#undef BUILD_ARMv4
#undef BUILD_ARMv5
#undef BUILD_ARMv6
#undef BUILD_ARMv7
#undef BUILD_ARMv8
#undef BUILD_BIG_ENDIAN
#undef BUILD_DOS4GW
#undef BUILD_E2K
#undef BUILD_EMSCRIPTEN
#undef BUILD_FREEBSD
#undef BUILD_HAIKU
#undef BUILD_IOS
#undef BUILD_IRIX
#undef BUILD_JS
#undef BUILD_LINUX
#undef BUILD_LINUX_UNKNOWN
#undef BUILD_LITTLE_ENDIAN
#undef BUILD_MIPS
#undef BUILD_MOBILE_PLATFORM
#undef BUILD_NETBSD
#undef BUILD_OPENBSD
#undef BUILD_POSIX
#undef BUILD_RISCV
#undef BUILD_RISCV_DOUBLEFP
#undef BUILD_RISCV_SINGLEFP
#undef BUILD_RISCV_SOFTFP
#undef BUILD_SERENITY
#undef BUILD_WIN32
#undef BUILD_X86
#undef BUILD_NSWITCH
#undef BUILD_PSVITA

//================================================================
//
//           PLATFORM DETECTION CODE
//
//================================================================
#if defined _WIN32
	#define BUILD_WIN32 1
#elif defined __EMSCRIPTEN__
	#define BUILD_EMSCRIPTEN 1
#elif defined __WATCOMC__ && defined __DOS__
	#define BUILD_DOS4GW 1
#else // POSIX compatible
	#define BUILD_POSIX 1
	#if defined __linux__
		#if defined __ANDROID__
			#define BUILD_ANDROID 1
		#else
			#include <features.h>
			// if our system libc has features.h header
			// try to detect it to not confuse other libcs with built with glibc game libraries
			#if !defined __GLIBC__
				#define BUILD_LINUX_UNKNOWN 1
			#endif
		#endif
		#define BUILD_LINUX 1
	#elif defined __FreeBSD__
		#define BUILD_FREEBSD 1
	#elif defined __NetBSD__
		#define BUILD_NETBSD 1
	#elif defined __OpenBSD__
		#define BUILD_OPENBSD 1
	#elif defined __HAIKU__
		#define BUILD_HAIKU 1
	#elif defined __serenity__
		#define BUILD_SERENITY 1
	#elif defined __sgi
		#define BUILD_IRIX 1
	#elif defined __APPLE__
		#include <TargetConditionals.h>
		#define BUILD_APPLE 1
		#if TARGET_OS_IOS
			#define BUILD_IOS 1
		#endif // TARGET_OS_IOS
	#elif defined __SWITCH__
		#define BUILD_NSWITCH 1
	#elif defined __vita__
		#define BUILD_PSVITA 1
	#else
		#error
	#endif
#endif

#if BUILD_ANDROID || defined BUILD_IOS || defined BUILD_NSWITCH || defined BUILD_PSVITA
	#define BUILD_MOBILE_PLATFORM 1
#endif

//================================================================
//
//           ENDIANNESS DEFINES
//
//================================================================

#if !defined BUILD_ENDIANNESS
	#if defined BUILD_WIN32 || __LITTLE_ENDIAN__
		//!!! Probably all WinNT installations runs in little endian
		#define BUILD_LITTLE_ENDIAN 1
	#elif __BIG_ENDIAN__
		#define BUILD_BIG_ENDIAN 1
	#elif defined __BYTE_ORDER__ && defined __ORDER_BIG_ENDIAN__ && defined __ORDER_LITTLE_ENDIAN__ // some compilers define this
		#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
			#define BUILD_BIG_ENDIAN 1
		#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
			#define BUILD_LITTLE_ENDIAN 1
		#endif
	#else
		#include <sys/param.h>
		#if __BYTE_ORDER == __BIG_ENDIAN
			#define BUILD_BIG_ENDIAN 1
		#elif __BYTE_ORDER == __LITTLE_ENDIAN
			#define BUILD_LITTLE_ENDIAN 1
		#endif
	#endif // !BUILD_WIN32
#endif

//================================================================
//
//           CPU ARCHITECTURE DEFINES
//
//================================================================
#if defined __x86_64__ || defined _M_X64
	#define BUILD_64BIT 1
	#define BUILD_AMD64 1
#elif defined __i386__ || defined _X86_ || defined _M_IX86
	#define BUILD_X86 1
#elif defined __aarch64__ || defined _M_ARM64
	#define BUILD_64BIT 1
	#define BUILD_ARM   8
#elif defined __mips__
	#define BUILD_MIPS 1
#elif defined __EMSCRIPTEN__
	#define BUILD_JS 1
#elif defined __e2k__
	#define BUILD_64BIT 1
	#define BUILD_E2K 1
#elif defined _M_ARM // msvc
	#define BUILD_ARM 7
	#define BUILD_ARM_HARDFP 1
#elif defined __arm__
	#if __ARM_ARCH == 8 || __ARM_ARCH_8__
		#define BUILD_ARM 8
	#elif __ARM_ARCH == 7 || __ARM_ARCH_7__
		#define BUILD_ARM 7
	#elif __ARM_ARCH == 6 || __ARM_ARCH_6__ || __ARM_ARCH_6J__
		#define BUILD_ARM 6
	#elif __ARM_ARCH == 5 || __ARM_ARCH_5__
		#define BUILD_ARM 5
	#elif __ARM_ARCH == 4 || __ARM_ARCH_4__
		#define BUILD_ARM 4
	#else
		#error "Unknown ARM"
	#endif

	#if defined __SOFTFP__ || __ARM_PCS_VFP == 0
		#define BUILD_ARM_SOFTFP 1
	#else // __SOFTFP__
		#define BUILD_ARM_HARDFP 1
	#endif // __SOFTFP__
#elif defined __riscv
	#define BUILD_RISCV 1

	#if __riscv_xlen == 64
		#define BUILD_64BIT 1
	#elif __riscv_xlen != 32
		#error "Unknown RISC-V ABI"
	#endif

	#if defined __riscv_float_abi_soft
		#define BUILD_RISCV_SOFTFP 1
	#elif defined __riscv_float_abi_single
		#define BUILD_RISCV_SINGLEFP 1
	#elif defined __riscv_float_abi_double
		#define BUILD_RISCV_DOUBLEFP 1
	#else
		#error "Unknown RISC-V float ABI"
	#endif
#else
	#error "Place your architecture name here! If this is a mistake, try to fix conditions above and report a bug"
#endif

#if BUILD_ARM == 8
	#define BUILD_ARMv8 1
#elif BUILD_ARM == 7
	#define BUILD_ARMv7 1
#elif BUILD_ARM == 6
	#define BUILD_ARMv6 1
#elif BUILD_ARM == 5
	#define BUILD_ARMv5 1
#elif BUILD_ARM == 4
	#define BUILD_ARMv4 1
#endif

#endif // BUILD_H
