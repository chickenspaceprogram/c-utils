# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.


.PHONY: rel dbg asan wipe clean make test memcheck

DEBUG=-DCMAKE_BUILD_TYPE=Debug
RELEASE=-DCMAKE_BUILD_TYPE=Release
STATIC_TOOLS=-DC_UTILS_CLANG_TIDY=ON -DC_UTILS_CPPCHECK=ON
TESTS=-DC_UTILS_TESTS=ON
ASAN=-DC_UTILS_USE_ASAN=ON
SENTINEL=build_sentinel
THREADS=-j8
BUILD_DIR=build

# this is a makefile i have to shorten the commands I have to type

make: $(SENTINEL)
	: 'build'
	cmake --build $(BUILD_DIR) $(THREADS)

rel: wipe
	: 'rel'
	cmake -B $(BUILD_DIR) $(RELEASE) $(TESTS) $(STATIC_TOOLS)
	touch $(SENTINEL)
	cmake --build $(BUILD_DIR) $(THREADS)

dbg: wipe
	: 'dbg'
	cmake -B $(BUILD_DIR) $(DEBUG) $(TESTS) $(STATIC_TOOLS)
	touch $(SENTINEL)
	cmake --build $(BUILD_DIR) $(THREADS)

asan: wipe
	: 'asan'
	cmake -B $(BUILD_DIR) $(DEBUG) $(TESTS) $(STATIC_TOOLS) $(ASAN)
	touch $(SENTINEL)
	cmake --build $(BUILD_DIR) $(THREADS)

test: make
	ctest --test-dir $(BUILD_DIR) --output-on-failure $(THREADS)

memcheck: make
	ctest --test-dir $(BUILD_DIR) -T memcheck $(THREADS)

wipe:
	: 'wipe'
	rm -rf build
	rm -f $(SENTINEL)

clean:
	: 'clean'
	cmake --build $(BUILD_DIR) --target clean

$(SENTINEL):
	: '$(SENTINEL)'
	cmake -B $(BUILD_DIR) $(DEBUG) $(TESTS) $(STATIC_TOOLS)
	touch $(SENTINEL)


