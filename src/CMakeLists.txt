# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# SPDX-License-Identifier: MPL-2.0

cmake_minimum_required(VERSION 3.25)

if (C_UTILS_NONPORTABLE)
	set(C_UTILS_NONPORTABLE_FILES
		nonport_include/cu/hashmap.h
		nonport_include/cu/minheap.h
		nonport_include/cu/arena.h
		arena.c # ms doesn't support max_align_t for some unknown reason ;-;
	)
endif()

add_library(CUtils STATIC
	alloc.c
	allocators.c
	sync.c
	rand.c
	tblhash.c
	include/cu/alloc.h
	include/cu/allocators.h
	include/cu/deque.h
	include/cu/intmanip.h
	include/cu/vector.h
	include/cu/sync.h
	include/cu/tblhash.h
	include/cu/rand.h
	${C_UTILS_NONPORTABLE_FILES}
)
target_include_directories(CUtils PUBLIC include)

if (C_UTILS_NONPORTABLE)
	target_include_directories(CUtils PUBLIC nonport_include)
	target_compile_definitions(CUtils PUBLIC CU_NONPORTABLE)
endif()

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

try_compile(HAVE_TYPEOF SOURCES "${PROJECT_SOURCE_DIR}/src/typeof_test.c" COMPILE_DEFINITIONS "-DCU_TYPEOF=typeof" C_STANDARD 23)
try_compile(HAVE_DUNDER_TYPEOF SOURCES "${PROJECT_SOURCE_DIR}/src/typeof_test.c" COMPILE_DEFINITIONS "-DCU_TYPEOF=__typeof__" C_STANDARD 23)
if (HAVE_TYPEOF)
	target_compile_definitions(CUtils PUBLIC CU_TYPEOF=typeof)
elseif(HAVE_DUNDER_TYPEOF)
	target_compile_definitions(CUtils PUBLIC CU_TYPEOF=__typeof__)
else()
	message(SEND_ERROR "c-utils requires typeof() to compile")
endif()

include(CheckIncludeFile)
check_include_file(threads.h HAVE_THREADS_H)

check_symbol_exists("__STDC_NO_THREADS__" "threads.h" NO_C11_THREADS)
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

if (EXISTS "/dev/urandom")
	target_compile_definitions(CUtils PRIVATE CUTILS_HAVE_URANDOM)
endif()

check_symbol_exists("getentropy" "unistd.h" HAVE_GETENTROPY)
if (HAVE_GETENTROPY)
	target_compile_definitions(CUtils PRIVATE CUTILS_HAVE_GETENTROPY)
endif()

check_symbol_exists("arc4random_buf" "stdlib.h" HAVE_ARC4RANDOM)
if (HAVE_ARC4RANDOM)
	target_compile_definitions(CUtils PRIVATE CUTILS_HAVE_ARC4RANDOM)
endif()

check_symbol_exists("BCryptGenRandom" "Windows.h" HAVE_BCRYPT)
if (HAVE_BCRYPTGENRANDOM)
	target_compile_definitions(CUtils PRIVATE CUTILS_HAVE_BCRYPT)
endif()

if (NOT HAVE_URANDOM AND NOT HAVE_GETENTROPY AND NOT HAVE_ARC4RANDOM AND NOT HAVE_BCRYPT)
message(FATAL_ERROR "Failed to find a suitable random number generation library.")
endif() 




function(c_utils_make_test testfilename) # can take libs to link as an optional arg
if (C_UTILS_TESTS)
	get_filename_component(EXENAME ${testfilename} NAME_WLE)
	add_executable(${EXENAME} ${testfilename})
	target_link_libraries(${EXENAME} ${ARGN})
	add_test(
		NAME ${EXENAME}
		COMMAND $<TARGET_FILE:${EXENAME}>
	)
endif()
endfunction()

add_subdirectory(tests)
