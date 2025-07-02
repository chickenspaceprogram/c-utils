# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.14)

add_library(rudp_utils
	alloc.c
	include/rudp/utils/hashmap.h
	include/rudp/utils/intmanip.h
	include/rudp/utils/vector.h
	include/rudp/utils/alloc.h
	include/rudp/utils/minheap.h
	include/rudp/utils/rc.h
	include/rudp/utils/deque.h
)

target_include_directories(rudp_utils PUBLIC include)

add_subdirectory(tests)
