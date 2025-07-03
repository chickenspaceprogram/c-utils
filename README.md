# c-utils

## A pile of C utilities

## Notes

This library hasn't been designed with an eye towards future ABI compatibility.
Properly guaranteeing that would probably invalidate the macro-based generics
approach used here.

This is a fine tradeoff for me, as this library is mostly intended for use in
my personal projects; I'm going to always end up always statically linking this
and I recommend you do the same.

Because of the necessity for static linking (and the fact that strong-copyleft
library licenses are a bit rude), I've opted for using the MPL here instead of
LGPL/GPL.

Furthermore, this library won't compile on MSVC.
It uses statement-expressions, which are a GCC/Clang extension and unsupported
by MSVC.
Statement-expressions are the only way to have "nice" C generics, so they're
unavoidable in my opinion.

## Documentation

The header files have been commented a bit; the code is macro-hell and kinda
unreadable but you should be able to figure out how to interface with it.

Check out the `src/tests` directory for the test cases; there are examples
for most of the functions in this library there.

Please report any bugs you encounter!

At time of writing, `rc.h` and `arc.h` haven't been tested so there are likely
bugs there. I haven't needed either and quite frankly can't be bothered.

## Features

- Generic allocator API
- Datastructures:
    - HashMap
    - Vector/Arraylist
    - Deque
    - Min-heap
    - Reference-counting pointer (atomic and non-atomic)
