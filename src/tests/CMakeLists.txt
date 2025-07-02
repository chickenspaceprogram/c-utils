# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.14)

make_test(test_vector.c PUBLIC rudp_utils)
make_test(test_hashmap.c PUBLIC rudp_utils)
make_test(test_minheap.c PUBLIC rudp_utils)
make_test(test_deque.c PUBLIC rudp_utils)
