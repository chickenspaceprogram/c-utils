# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.15)

add_subdirectory(tests)

set(C_UTILS_INCLUDES
    "${PROJECT_SOURCE_DIR}/include/c-utils/mem.h"
    "${PROJECT_SOURCE_DIR}/include/c-utils/string.h"
)

add_library(c-utils 
	${C_UTILS_INCLUDES}
	mem.c
#	string.c
)

target_include_directories(c-utils PUBLIC "${PROJECT_SOURCE_DIR}/include")

