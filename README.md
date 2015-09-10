# Read Me

An implementation of `Fingerprint128` from Google's [`FarmHash`](https://github.com/google/farmhash) version `1.1` suite, which is equivalent to Google's [`CityHash128`](https://github.com/google/cityhash) version `1.1.1`.

## Testing

The `test` directory builds for `x86` and `x86_64` targets on Apple's MacOS. It ensures that the `portable` implementation produces identical results to Google's reference `farmhash` implementation on **both** 32-bit and 64-bit architectures.

Note that the implementation should build cleanly on both 32-bit and 64-bit architectures, and give identical results on both big-endian and litte-endian systems. (Note that big-endian systems have not been tested yet, however!)
