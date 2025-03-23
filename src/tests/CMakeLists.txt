
# Copyright 2024, 2025 Athena Boose

# This file is part of c-utils.

# c-utils is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option)
# any later version.

# c-utils is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
# for more details.

# You should have received a copy of the GNU General Public License
# along with c-utils. If not, see <https://www.gnu.org/licenses/>. 

cmake_minimum_required(VERSION 3.15)

add_executable(test-arena arena.c)
target_link_libraries(test-arena PRIVATE c-utils)

add_test(
    NAME test-arena
    COMMAND $<TARGET_FILE:test-arena>
)
