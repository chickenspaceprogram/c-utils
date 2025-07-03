# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# SPDX-License-Identifier: MPL-2.0

cmake_minimum_required(VERSION 3.21)

add_library(c_utils
	alloc.c
	include/cu/hashmap.h
	include/cu/intmanip.h
	include/cu/vector.h
	include/cu/alloc.h
	include/cu/minheap.h
	include/cu/rc.h
	include/cu/arc.h
	include/cu/deque.h
)

target_include_directories(c_utils PUBLIC include)
target_compile_features(c_utils PUBLIC c_std_23) # headers use typeof()

add_subdirectory(tests)
