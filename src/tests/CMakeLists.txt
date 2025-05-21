# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.15)

add_executable(test-arena arena.c)
target_link_libraries(test-arena PRIVATE c-utils)

add_test(
    NAME test-arena
    COMMAND $<TARGET_FILE:test-arena>
)

add_executable(test-string string.c)
target_link_libraries(test-string PRIVATE c-utils)

add_test(
    NAME test-string
    COMMAND $<TARGET_FILE:test-string>
)
