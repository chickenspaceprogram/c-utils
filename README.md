# c-utils

c-utils (usually called just `cu`) is a lightweight sort of compatibility layer
on top of the standard C library. It is effectively "stuff that should be in
libc" and "stuff that is in libc but isn't available everywhere."

It's primarily intended for Unix systems, but it runs on Windows if compiled
with GCC/Clang. Testing is not exhaustive, but it should compile on recent
GCC/Clang versions, and is intended to neatly fail with a compile error on
compilers that it doesn't support; I've put some effort to avoid subtle
nonportable errors.

## Notes

This library hasn't been designed with an eye towards future ABI compatibility.
Most structs have public definitions and there is no mechanism to check library
versions.

This is a fine tradeoff for me, as this library is mostly intended for use in
my personal projects; I'm going to always end up always statically linking this
and compiling it along with my other projects; I recommend you do the same.

With the exception of bugfixes it's intended that any changes that could break
dependent code will also change function names and will result in compile
errors in that dependent code.

## Documentation

Documentation isn't great, headers should have enough information to be useful
though. At some point I'll figure out Doxygen.

Check out the `src/tests` directory for the test cases; there are examples
for most of the functions in this library there.

Please report any bugs you encounter!

## Features

- Generic allocator interface
- A couple types of arenas
- A system CSPRNG interface
- A secure hash function for use in hashmaps
- A hashmap implemented with quadratic probing (chaining hashmap planned also)
- Bit manipulation and overflow checking functions
- An assert macro that asserts in both release and debug builds
- A halfhearted string library
