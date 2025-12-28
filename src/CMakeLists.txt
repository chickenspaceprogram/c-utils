# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# SPDX-License-Identifier: MPL-2.0

cmake_minimum_required(VERSION 3.25)

add_library(CUtils STATIC
	alloc.c
	arena.c
	rand.c
	siphash.c
	hashmap.c
	bitmanip.c
	string.c
	../include/cu/alloc.h
	../include/cu/arena.h
	../include/cu/bitmanip.h
	../include/cu/dbgassert.h
	../include/cu/hashmap.h
	../include/cu/rand.h
	../include/cu/siphash.h
	../include/cu/string.h
)
target_include_directories(CUtils PUBLIC ../include)

include(CheckSymbolExists)
check_symbol_exists("memset_explicit" "string.h" HAVE_MEMSET_EXPLICIT)
check_symbol_exists("explicit_bzero" "string.h" HAVE_EXPLICIT_BZERO)
check_symbol_exists("SecureZeroMemory" "Windows.h" HAVE_SECURE_ZERO_MEMORY)
check_symbol_exists("memset_s" "string.h" HAVE_MEMSET_S)
check_symbol_exists("stdc_bit_ceil" "stdbit.h" HAVE_STDC_BIT_CEIL)
check_symbol_exists("ckd_mul" "stdckdint.h" HAVE_CKD_MUL)

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

# the following suck, but since they are just detecting C23 features idc
if(HAVE_STDC_BIT_CEIL)
	# probably have all of stdbit
	target_compile_definitions(CUtils PUBLIC CU_HAVE_STDBIT)
endif()
if(HAVE_CKD_MUL)
	# probably have all of ckdint
	target_compile_definitions(CUtils PUBLIC CU_HAVE_CKDINT)
endif()

if (EXISTS "/dev/urandom")
	target_compile_definitions(CUtils PRIVATE CUTILS_HAVE_URANDOM)
endif()
check_symbol_exists("getentropy" "unistd.h" HAVE_GETENTROPY)
check_symbol_exists("arc4random_buf" "stdlib.h" HAVE_ARC4RANDOM)

if (HAVE_GETENTROPY)
	target_compile_definitions(CUtils PRIVATE CU_HAVE_GETENTROPY)
endif()

if (HAVE_ARC4RANDOM)
	target_compile_definitions(CUtils PRIVATE CU_HAVE_ARC4RANDOM)
endif()

if (WIN32)
	target_compile_definitions(CUtils PRIVATE CU_HAVE_BCRYPT)
	target_link_libraries(CUtils PRIVATE bcrypt)
endif()

if (NOT HAVE_URANDOM AND NOT HAVE_GETENTROPY AND NOT HAVE_ARC4RANDOM AND NOT WIN32)
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
