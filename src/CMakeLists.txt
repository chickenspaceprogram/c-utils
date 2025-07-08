# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# SPDX-License-Identifier: MPL-2.0

cmake_minimum_required(VERSION 3.21)

add_library(CUtils
	alloc.c
	allocators.c
	include/cu/hashmap.h
	include/cu/intmanip.h
	include/cu/vector.h
	include/cu/alloc.h
	include/cu/allocators.h
	include/cu/minheap.h
	include/cu/rc.h
	include/cu/arc.h
	include/cu/deque.h
)

include(CheckSymbolExists)
check_symbol_exists("memset_explicit" "string.h" HAVE_MEMSET_EXPLICIT)
check_symbol_exists("explicit_bzero" "string.h" HAVE_EXPLICIT_BZERO)
check_symbol_exists("SecureZeroMemory" "Windows.h" HAVE_SECURE_ZERO_MEMORY)
check_symbol_exists("memset_s" "string.h" HAVE_MEMSET_S)

if(HAVE_MEMSET_EXPLICIT)
	target_compile_definitions(CUtils PRIVATE CU_HAVE_MEMSET_EXPLICIT)
endif()

if(HAVE_EXPLICIT_BZERO)
	target_compile_definitions(CUtils PRIVATE CU_HAVE_EXPLICIT_BZERO)
endif()

if(HAVE_SECURE_ZERO_MEMORY)
	target_compile_definitions(CUtils PRIVATE CU_HAVE_SECURE_ZERO_MEMORY)
endif()

if(HAVE_MEMSET_S)
	target_compile_definitions(CUtils PRIVATE CU_HAVE_MEMSET_S)
endif()


target_include_directories(CUtils PUBLIC include)
target_compile_features(CUtils PUBLIC c_std_23) # headers use typeof()

add_subdirectory(tests)
