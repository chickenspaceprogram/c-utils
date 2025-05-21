# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.15)

add_executable(test-mem test-mem.c)
target_link_libraries(test-mem PRIVATE c-utils)

add_test(
    NAME test-mem
    COMMAND $<TARGET_FILE:test-mem>
)

#add_executable(test-string test-string.c)
#target_link_libraries(test-string PRIVATE c-utils)

#add_test(
#    NAME test-string
#    COMMAND $<TARGET_FILE:test-string>
#)
