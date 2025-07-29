# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# SPDX-License-Identifier: MPL-2.0

cmake_minimum_required(VERSION 3.21)
project(CUtils C)

# this file does kinda suck but it also does work and i can't be bothered to make it decent

option(C_UTILS_USE_ASAN "Links c-utils and any of its tests with the ASAN library.")
option(C_UTILS_DEBUG "Builds a debug version of c-utils and adds extra GCC/Clang-specific compiler flags.")
option(C_UTILS_CPPCHECK "Uses the cppcheck tool to statically analyze source code when building.")
option(C_UTILS_CLANG_TIDY "Uses the clang-tidy tool to statically analyze source code when building.")
option(C_UTILS_TESTS "Builds c-utils's unit tests.")

option(C_UTILS_NONPORTABLE_AUTO 
	"Automatically turns on or turns off nonportable features depending on platform. This is primarily intended for c-utils project developers."
)
option(C_UTILS_NONPORTABLE "Enables nonportable c-utils features.")
set(CMAKE_C_STANDARD 23)

if (C_UTILS_NONPORTABLE_AUTO)
	if (MSVC)
		set(C_UTILS_NONPORTABLE OFF)
	else()
		set(C_UTILS_NONPORTABLE ON)
	endif()
endif()

if(C_UTILS_TESTS)
	include(CTest)
endif()


if(C_UTILS_USE_ASAN)
	message(NOTICE "Option `C_UTILS_USE_ASAN` specified. Linking ASAN and setting `C_UTILS_DEBUG`.")
	set(CMAKE_CXX_FLAGS "-fsanitize=address -fsanitize-recover=address -fsanitize=undefined -fsanitize=leak")
	set(C_UTILS_DEBUG ON)
endif()

if (C_UTILS_DBGFLAGS)
	message(NOTICE "Option `C_UTILS_DBGFLAGS` specified. Adding extra compile flags.")
	set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -Werror")
endif()

if (C_UTILS_CPPCHECK)
	find_program(CPPCHECK cppcheck)
	if (CPPCHECK)
		message(NOTICE "Found cppcheck!")
		list(APPEND CMAKE_CXX_CPPCHECK
			"--enable=all"
			"--inconclusive"
			"--force"
			"--std=c11"
		)
	else()
		message(WARNING "Failed to find cppcheck. Continuing...")
	endif()
endif()

if (C_UTILS_CLANG_TIDY)
	find_program(CLANG_TIDY clang-tidy)
	if (CLANG_TIDY)
		message(NOTICE "Found clang-tidy!")
		list(APPEND CMAKE_C_CLANG_TIDY
			"--use-color"
		)
		set(CMAKE_C_CLANG_TIDY ${CLANG_TIDY})
	else()
		message(NOTICE "clang-tidy not found and not used.")
	endif()
endif()



add_subdirectory(src)
