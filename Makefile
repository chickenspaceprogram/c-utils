# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# SPDX-License-Identifier: MPL-2.0

.PHONY: rel dbg asan wipe clean make test memcheck

UNIXFLAGS=-DC_UTILS_DBGFLAGS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DC_UTILS_NONPORTABLE_AUTO=ON
STATIC_TOOLS=-DC_UTILS_CLANG_TIDY=ON -DC_UTILS_CPPCHECK=ON
ASAN=-DC_UTILS_USE_ASAN=ON
TESTS=-DC_UTILS_TESTS=ON

DEBUG=-DCMAKE_BUILD_TYPE=Debug
RELWITHDEBINFO=-DCMAKE_BUILD_TYPE=RelWithDebInfo
RELEASE=-DCMAKE_BUILD_TYPE=Release 

THREADS=-j8
BUILD_DIR=build

# this is a makefile i have to shorten the commands I have to type

make: $(BUILD_DIR)
	: 'build'
	cmake --build $(BUILD_DIR) $(THREADS)

rel: wipe
	: 'rel'
	cmake -B $(BUILD_DIR) $(RELEASE) $(UNIXFLAGS) $(TESTS) $(STATIC_TOOLS)
	cmake --build $(BUILD_DIR) $(THREADS)

dbg: wipe
	: 'dbg'
	cmake -B $(BUILD_DIR) $(DEBUG) $(UNIXFLAGS) $(TESTS) $(STATIC_TOOLS)
	cmake --build $(BUILD_DIR) $(THREADS)

reldbg: wipe
	: 'rel with debug info'
	cmake -B $(BUILD_DIR) $(RELWITHDEBINFO) $(UNIXFLAGS) $(TESTS) $(STATIC_TOOLS)
	cmake --build $(BUILD_DIR) $(THREADS)

asan: wipe
	: 'asan'
	cmake -B $(BUILD_DIR) $(DEBUG) $(UNIXFLAGS) $(TESTS) $(STATIC_TOOLS) $(ASAN)
	cmake --build $(BUILD_DIR) $(THREADS)

test: make
	ctest --test-dir $(BUILD_DIR) --output-on-failure $(THREADS)

memcheck: make
	ctest --test-dir $(BUILD_DIR) -T memcheck $(THREADS)

wipe:
	: 'wipe'
	rm -rf build

clean:
	: 'clean'
	cmake --build $(BUILD_DIR) --target clean

$(BUILD_DIR):
	: '$(BUILD_DIR)'
	cmake -B $(BUILD_DIR) $(DEBUG) $(UNIXFLAGS) $(TESTS) $(STATIC_TOOLS)


