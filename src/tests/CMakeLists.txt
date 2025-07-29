# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# SPDX-License-Identifier: MPL-2.0

cmake_minimum_required(VERSION 3.14)

c_utils_make_test(test_vector.c PUBLIC CUtils)
c_utils_make_test(test_deque.c PUBLIC CUtils)


if (C_UTILS_NONPORTABLE)
	c_utils_make_test(test_arena.c PUBLIC CUtils)
	c_utils_make_test(test_minheap.c PUBLIC CUtils)
	c_utils_make_test(test_hashmap.c PUBLIC CUtils)
	c_utils_make_test(test_sync.c PUBLIC CUtils)
endif()
