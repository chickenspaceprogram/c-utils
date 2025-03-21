
# Copyright 2024, 2025 Athena Boose

# This file is part of badsh.

# badsh is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option)
# any later version.

# badsh is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
# for more details.

# You should have received a copy of the GNU General Public License
# along with badsh. If not, see <https://www.gnu.org/licenses/>. 

# Copyright 2024, 2025 Athena Boose

cmake_minimum_required(VERSION 3.15)
project(
    badsh 
    VERSION 0.1
    DESCRIPTION "The worst POSIX-compliant shell possible."
    LANGUAGES C
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)
set(C_UTILS_INCLUDES
    "${PROJECT_SOURCE_DIR}/include/c-utils/arena.h"
    "${PROJECT_SOURCE_DIR}/include/c-utils/string.h"
)

include(CTest)

add_subdirectory(src)


