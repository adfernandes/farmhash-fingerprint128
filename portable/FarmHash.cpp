// FarmHash, by Geoff Pike
//
// https://github.com/google/farmhash
//
// Provides a 128-bit hash 'Fingerprint128' equivalent to 'CityHash128 (v1.1.1)',
// which itself is equivalent to Google's 'FarmHash (v1.1)' as per GitHub repo.
//
// Copyright (c) 2014 Google, Inc.
//
// Numerous Modifications and Optimizations
// Copyright (c) 2015 Andrew Fernandes <andrew@fernandes.org>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// --------------------------------------------------------------------
//
#include "FarmHash.hpp"

#include <cstring>
#include <algorithm>

// ---------------------------------------------------------------------

namespace {

    // Give hints to the optimizer (even though humans are notoriously bad at doing so).

    #if defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER)

        inline bool IsLikely  ( const bool x ) { return __builtin_expect(x, true ); }
        inline bool IsUnlikely( const bool x ) { return __builtin_expect(x, false); }

    #else

        inline bool IsLikely  ( const bool x ) { return x; }
        inline bool IsUnlikely( const bool x ) { return x; }

    #endif

    // Byte-order independent fetching.

    inline uint64_t Fetch64(const uint8_t *p) {
        uint64_t result;
        std::memcpy(&result, p, sizeof(result));
        return uint64_in_little_endian_order(result);
    }

    inline uint32_t Fetch32(const uint8_t *p) {
        uint32_t result;
        std::memcpy(&result, p, sizeof(result));
        return uint32_in_little_endian_order(result);
    }

    // Cyclical rotation of unsigned integers.

    #if defined(_MSC_VER)

     // inline uint32_t RotateRight32(uint32_t value, int shift) { return _rotr  (value, shift); }
        inline uint64_t RotateRight64(uint64_t value, int shift) { return _rotr64(value, shift); }

    #else // modern compilers automatically get this right, with no 'undefined' behaviour (https://goo.gl/ZsFpuz)

     // inline uint32_t RotateRight32(uint32_t value, unsigned shift) { shift &= 31; return (value >> shift) | (value << (32 - shift)); }
        inline uint64_t RotateRight64(uint64_t value, unsigned shift) { shift &= 63; return (value >> shift) | (value << (64 - shift)); }

    #endif

}

namespace FarmHash {

    namespace {

        // Some primes between 2^63 and 2^64 for various uses.

        const uint64_t k0 = 0xc3a5c85c97cb3127ULL;
        const uint64_t k1 = 0xb492b66fbe98f273ULL;
        const uint64_t k2 = 0x9ae16a3b2f90404fULL;

        // Murmur-inspired hashing and suboperations.

        inline uint64_t Hash128to64(UInt128 x) {
            const uint64_t kMul = 0x9ddfea08eb382d69ULL;
            uint64_t a = (UInt128Low64(x) ^ UInt128High64(x)) * kMul;
            a ^= (a >> 47);
            uint64_t b = (UInt128High64(x) ^ a) * kMul;
            b ^= (b >> 47);
            b *= kMul;
            return b;
        }

        inline uint64_t ShiftMix(uint64_t value) {
            return value ^ (value >> 47);
        }

        inline uint64_t HashLen16(uint64_t u, uint64_t v) {
            return Hash128to64(UInt128(u, v));
        }

        inline uint64_t HashLen16(uint64_t u, uint64_t v, uint64_t mul) {
            uint64_t a = (u ^ v) * mul;
            a ^= (a >> 47);
            uint64_t b = (v ^ a) * mul;
            b ^= (b >> 47);
            b *= mul;
            return b;
        }

        inline uint64_t HashLen0to16(const uint8_t *s, size_t len) {
            if (len >= 8) {
                uint64_t mul = k2 + len * 2;
                uint64_t a = Fetch64(s) + k2;
                uint64_t b = Fetch64(s + len - 8);
                uint64_t c = RotateRight64(b, 37) * mul + a;
                uint64_t d = (RotateRight64(a, 25) + b) * mul;
                return HashLen16(c, d, mul);
            }
            if (len >= 4) {
                uint64_t mul = k2 + len * 2;
                uint64_t a = Fetch32(s);
                return HashLen16(len + (a << 3), Fetch32(s + len - 4), mul);
            }
            if (len > 0) {
                uint8_t a = s[0];
                uint8_t b = s[len >> 1];
                uint8_t c = s[len - 1];
                uint32_t y = static_cast<uint32_t>(a) + (static_cast<uint32_t>(b) << 8);
                uint32_t z = len + (static_cast<uint32_t>(c) << 2);
                return ShiftMix(y * k2 ^ z * k0) * k2;
            }
            return k2;
        }

        // Return a 16-byte hash for 48 bytes.  Quick and dirty.
        // Callers do best to use "random-looking" value for a and b.

        inline std::pair<uint64_t,uint64_t> WeakHashLen32WithSeeds(uint64_t w, uint64_t x, uint64_t y, uint64_t z, uint64_t a, uint64_t b) {
            a += w;
            b = RotateRight64(b + a + z, 21);
            uint64_t c = a;
            a += x;
            a += y;
            b += RotateRight64(a, 44);
            return std::make_pair(a + z, b + c);
        }

        // Return a 16-byte hash for s[0] ... s[31], a, and b.  Quick and dirty.

        inline std::pair<uint64_t,uint64_t> WeakHashLen32WithSeeds(const uint8_t *s, uint64_t a, uint64_t b) {
            return WeakHashLen32WithSeeds(Fetch64(s), Fetch64(s + 8), Fetch64(s + 16), Fetch64(s + 24), a, b);
        }

        // A subroutine for CityHash128().  Returns a decent 128-bit hash for strings
        // of any length representable in signed long.  Based on City and Murmur.

        inline UInt128 CityMurmur(const uint8_t *s, size_t len, UInt128 seed) {
            uint64_t a = UInt128Low64(seed);
            uint64_t b = UInt128High64(seed);
            uint64_t c = 0;
            uint64_t d = 0;
            signed long l = len - 16;
            if (l <= 0) {  // len <= 16
                a = ShiftMix(a * k1) * k1;
                c = b * k1 + HashLen0to16(s, len);
                d = ShiftMix(a + (len >= 8 ? Fetch64(s) : c));
            } else {  // len > 16
                c = HashLen16(Fetch64(s + len - 8) + k1, a);
                d = HashLen16(b + len, c + Fetch64(s + len - 16));
                a += d;
                do {
                    a ^= ShiftMix(Fetch64(s) * k1) * k1;
                    a *= k1;
                    b ^= a;
                    c ^= ShiftMix(Fetch64(s + 8) * k1) * k1;
                    c *= k1;
                    d ^= c;
                    s += 16;
                    l -= 16;
                } while (l > 0);
            }
            a = HashLen16(a, c);
            b = HashLen16(d, b);
            return UInt128(a ^ b, HashLen16(b, a));
        }

    }

    UInt128 CityHash128WithSeed(const uint8_t *s, size_t len, UInt128 seed) {

        // We expect len >= 128 to be the common case.

        if (IsUnlikely(len < 128)) {
            return CityMurmur(s, len, seed);
        }

        // Keep 56 bytes of state: v, w, x, y, and z.

        std::pair<uint64_t,uint64_t> v, w;
        uint64_t x = UInt128Low64(seed);
        uint64_t y = UInt128High64(seed);
        uint64_t z = len * k1;
        v.first = RotateRight64(y ^ k1, 49) * k1 + Fetch64(s);
        v.second = RotateRight64(v.first, 42) * k1 + Fetch64(s + 8);
        w.first = RotateRight64(y + z, 35) * k1 + x;
        w.second = RotateRight64(x + Fetch64(s + 88), 53) * k1;

        // This is the same inner loop as CityHash64(), manually unrolled.

        do {
            x = RotateRight64(x + y + v.first + Fetch64(s + 8), 37) * k1;
            y = RotateRight64(y + v.second + Fetch64(s + 48), 42) * k1;
            x ^= w.second;
            y += v.first + Fetch64(s + 40);
            z = RotateRight64(z + w.first, 33) * k1;
            v = WeakHashLen32WithSeeds(s, v.second * k1, x + w.first);
            w = WeakHashLen32WithSeeds(s + 32, z + w.second, y + Fetch64(s + 16));
            std::swap(z, x);
            s += 64;
            x = RotateRight64(x + y + v.first + Fetch64(s + 8), 37) * k1;
            y = RotateRight64(y + v.second + Fetch64(s + 48), 42) * k1;
            x ^= w.second;
            y += v.first + Fetch64(s + 40);
            z = RotateRight64(z + w.first, 33) * k1;
            v = WeakHashLen32WithSeeds(s, v.second * k1, x + w.first);
            w = WeakHashLen32WithSeeds(s + 32, z + w.second, y + Fetch64(s + 16));
            std::swap(z, x);
            s += 64;
            len -= 128;
        } while (IsLikely(len >= 128));

        x += RotateRight64(v.first + z, 49) * k0;
        y = y * k0 + RotateRight64(w.second, 37);
        z = z * k0 + RotateRight64(w.first, 27);
        w.first *= 9;
        v.first *= k0;

        // If 0 < len < 128, hash up to 4 chunks of 32 bytes each from the end of s.

        for (size_t tail_done = 0; tail_done < len; ) {
            tail_done += 32;
            y = RotateRight64(x + y, 42) * k0 + v.second;
            w.first += Fetch64(s + len - tail_done + 16);
            x = x * k0 + w.first;
            z += w.second + Fetch64(s + len - tail_done);
            w.second += v.first;
            v = WeakHashLen32WithSeeds(s + len - tail_done, v.first + z, v.second);
            v.first *= k0;
        }

        // At this point our 56 bytes of state should contain more than
        // enough information for a strong 128-bit hash.  We use two
        // different 56-byte-to-8-byte hashes to get a 16-byte final result.

        x = HashLen16(x, v.first);
        y = HashLen16(y + z, w.first);

        return UInt128(HashLen16(x + v.second, w.second) + y, HashLen16(x + w.second, y + v.second));

    }

    UInt128 CityHash128(const uint8_t *s, size_t len) {
        return len >= 16 ? CityHash128WithSeed(s + 16, len - 16, UInt128(Fetch64(s), Fetch64(s + 8) + k0)) : CityHash128WithSeed(s, len, UInt128(k0, k1));
    }

    UInt128 Fingerprint128(const uint8_t *s, size_t len) {
        return CityHash128(s, len);
    }

}
