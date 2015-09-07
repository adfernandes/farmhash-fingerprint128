# To Do

* Make sure the 32-bit variants make sense with the SSE and AVX options. (Does any 32-bit mode even have AVX extensions?)
* Compile with `GCC` or `clang` on Windows, Linux, and Darwin.
* Change the namespace wrapping for true processor independence and dispatch. Right now only a few of the functions use `NAMESPACE_FOR_HASH_FUNCTIONS`, unfortunately.
