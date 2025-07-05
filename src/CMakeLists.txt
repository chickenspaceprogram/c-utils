# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# SPDX-License-Identifier: MPL-2.0

cmake_minimum_required(VERSION 3.21)

add_library(CUtils
	alloc.c
	arena.c
	sync.c
	include/cu/alloc.h
	include/cu/arc.h
	include/cu/arena.h
	include/cu/deque.h
	include/cu/hashmap.h
	include/cu/intmanip.h
	include/cu/minheap.h
	include/cu/rc.h
	include/cu/sync.h
	include/cu/threadpool.h
	include/cu/vector.h
)

include(CheckSymbolExists)
check_symbol_exists("__STDC_NO_THREADS__", "threads.h", NO_C11_THREADS)
check_symbol_exists("__STDC_NO_ATOMICS__", "stdatomic.h", NO_C11_ATOMICS)

if(NO_C11_THREADS)
	message(SEND_ERROR "Your C library lacks C11 threads.")
endif()

if(NO_C11_ATOMICS)
	message(SEND_ERROR "Your C library lacks C11 atomics.")
endif()

target_include_directories(CUtils PUBLIC include)
target_compile_features(CUtils PUBLIC c_std_23) # headers use typeof()

add_subdirectory(tests)
