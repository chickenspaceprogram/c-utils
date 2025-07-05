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

find_package(Threads)
if (Threads_FOUND)
	if (CMAKE_USE_PTHREADS_INIT)
		add_compile_definitions(USE_PTHREAD)
	elseif(CMAKE_USE_WIN32_INIT)
		add_compile_definitions(USE_WIN32)
	else()
		message(FATAL_ERROR "CUtils requires a suitable threading library; CMake failed to find one.")
	endif()
endif()

target_include_directories(CUtils PUBLIC include)
target_compile_features(CUtils PUBLIC c_std_23) # headers use typeof()
target_link_libraries(CUtils PUBLIC Threads::Threads)

add_subdirectory(tests)
