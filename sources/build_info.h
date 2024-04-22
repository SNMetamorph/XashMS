/*
build_info.h - routines for getting information about build host OS, architecture and VCS state
Copyright (C) 2022 SNMetamorph

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#pragma once
#include "build.h"

namespace BuildInfo
{
	// Returns name of target operating system.
	constexpr const char *GetPlatform()
	{
#if BUILD_WIN32
		return "win32";
#elif BUILD_ANDROID
		return "android";
#elif BUILD_LINUX
		return "linux";
#elif BUILD_APPLE
		return "apple";
#elif BUILD_FREEBSD
		return "freebsd";
#elif BUILD_NETBSD
		return "netbsd";
#elif BUILD_OPENBSD
		return "openbsd";
#elif BUILD_EMSCRIPTEN
		return "emscripten";
#elif BUILD_DOS4GW
		return "dos4gw";
#elif BUILD_HAIKU
		return "haiku";
#elif BUILD_SERENITY
		return "serenityos";
#elif BUILD_IRIX
		return "irix";
#else
#error "Place your operating system name here! If this is a mistake, try to fix conditions above and report a bug"
#endif
		return "unknown";
	}

	// Returns current name of the architecture without any spaces.
	constexpr const char *GetArchitecture()
	{
#if BUILD_AMD64
		return "amd64";
#elif BUILD_X86
		return "i386";
#elif BUILD_ARM && BUILD_64BIT
		return "arm64";
#elif BUILD_ARM
		return "armv"
	#if BUILD_ARM == 8
		"8_32" // for those who (mis)using 32-bit OS on 64-bit CPU
	#elif BUILD_ARM == 7
		"7"
	#elif BUILD_ARM == 6
		"6"
	#elif BUILD_ARM == 5
		"5"
	#elif BUILD_ARM == 4
		"4"
	#endif

	#if BUILD_ARM_HARDFP
		"hf"
	#else
		"l"
	#endif
	;
#elif BUILD_MIPS && BUILD_BIG_ENDIAN
		return "mips"
	#if BUILD_64BIT
		"64"
	#endif
	#if BUILD_LITTLE_ENDIAN
		"el"
	#endif
	;
#elif BUILD_RISCV
		return "riscv"
	#if BUILD_64BIT
		"64"
	#else
		"32"
	#endif
	#if BUILD_RISCV_SINGLEFP
		"d"
	#elif BUILD_RISCV_DOUBLEFP
		"f"
	#endif
	;
#elif BUILD_JS
		return "javascript";
#elif BUILD_E2K
		return "e2k";
#else
#error "Place your architecture name here! If this is a mistake, try to fix conditions above and report a bug"
#endif
		return "unknown";
	}

	// Returns a short hash of current commit in VCS as string
	constexpr const char *GetCommitHash()
	{
#ifdef BUILD_COMMIT_HASH
		return BUILD_COMMIT_HASH;
#else
		return "notset";
#endif
	}

	// Returns string with name of current branch in VCS.
	constexpr const char *GetBranchName()
	{
#ifdef BUILD_BRANCH_NAME
		return BUILD_BRANCH_NAME;
#else
		return "notset";
#endif
	}

	// Returns build host machine date when program was built.
	constexpr const char *GetDate()
	{
		return __DATE__;
	}
};
