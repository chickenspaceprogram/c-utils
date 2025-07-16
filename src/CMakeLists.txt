# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# SPDX-License-Identifier: MPL-2.0

cmake_minimum_required(VERSION 3.21)

if (C_UTILS_NONPORTABLE)
	set(C_UTILS_NONPORTABLE_FILES
		nonport_include/cu/hashmap.h
		nonport_include/cu/minheap.h
		nonport_include/include/cu/sync.h
		nonport_include/cu/pool.h
		nonport_include/cu/task.h
		sync.c
		pool.c
		task.c
	)
endif()

add_library(CUtils
	alloc.c
	arena.c
	allocators.c
	include/cu/alloc.h
	include/cu/allocators.h
	include/cu/deque.h
	include/cu/intmanip.h
	include/cu/vector.h
	${C_UTILS_NONPORTABLE_FILES}
)
target_include_directories(CUtils PUBLIC include)
target_compile_features(CUtils PUBLIC c_std_23) # headers use typeof()

if (C_UTILS_NONPORTABLE)
	target_include_directories(CUtils PUBLIC nonport_include)
endif()

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


add_subdirectory(tests)
