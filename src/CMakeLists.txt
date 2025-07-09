# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# SPDX-License-Identifier: MPL-2.0

cmake_minimum_required(VERSION 3.21)

add_library(CUtils
	alloc.c
	arena.c
	allocators.c
	sync.c
	include/cu/alloc.h
	include/cu/allocators.h
	include/cu/deque.h
	include/cu/hashmap.h
	include/cu/intmanip.h
	include/cu/minheap.h
	include/cu/sync.h
	include/cu/threadpool.h
	include/cu/vector.h
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

include(CheckIncludeFile)
check_include_file(threads.h HAVE_THREADS_H)
check_include_file(stdatomic.h HAVE_STDATOMIC_H)

check_symbol_exists("__STDC_NO_THREADS__" "threads.h" NO_C11_THREADS)
check_symbol_exists("__STDC_NO_ATOMICS__" "stdatomic.h" NO_C11_ATOMICS)
check_symbol_exists("pthread_mutex_timedlock" "pthread.h" HAVE_PTHREAD_MUTEX_TIMEDLOCK)

find_package(Threads)
if (Threads_FOUND AND CMAKE_USE_PTHREADS_INIT)
	set(CUTILS_HAVE_PTHREADS ON)
	target_link_libraries(CUtils PUBLIC Threads::Threads)
endif()

if(HAVE_THREADS_H AND NOT NO_C11_THREADS)
	set(CUTILS_HAVE_C11_THREADS ON)
endif()

if(NOT CUTILS_HAVE_C11_THREADS AND NOT CUTILS_HAVE_PTHREADS)
	message(SEND_ERROR "c-utils requires either C11 or POSIX threads; your system has neither")
endif()

if(CUTILS_HAVE_C11_THREADS)
	target_compile_definitions(CUtils PUBLIC CUTILS_HAVE_C11_THREADS)
endif()

if(CUTILS_HAVE_PTHREADS)
	target_compile_definitions(CUtils PUBLIC CUTILS_HAVE_PTHREADS)
endif()

if(HAVE_PTHREAD_MUTEX_TIMEDLOCK)
	target_compile_definitions(CUtils PUBLIC CUTILS_HAVE_PTHREAD_MUTEX_TIMEDLOCK)
endif()

if(NO_C11_ATOMICS OR NOT HAVE_STDATOMIC_H)
	message(SEND_ERROR "c-utils requires C11 atomics, which your system lacks")
endif()

target_include_directories(CUtils PUBLIC include)
target_compile_features(CUtils PUBLIC c_std_23) # headers use typeof()

add_subdirectory(tests)
