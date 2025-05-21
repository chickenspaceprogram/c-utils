# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.15)
project(
    c-utils 
    VERSION 0.1
    DESCRIPTION "A pile of C utility functions and datatypes."
    LANGUAGES C
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)
set(C_UTILS_INCLUDES
    "${PROJECT_SOURCE_DIR}/include/c-utils/mem.h"
    "${PROJECT_SOURCE_DIR}/include/c-utils/string.h"
)

include(CTest)

add_subdirectory(src)


