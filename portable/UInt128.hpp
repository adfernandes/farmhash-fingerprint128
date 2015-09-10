#ifndef UINT128_HPP
#define UINT128_HPP

// ---------------------------------------------------------------------
// Standard header files.

#include <cstdint>
#include <cstdlib>

#include <utility>

// ---------------------------------------------------------------------
#if !defined(UInt128)

typedef std::pair<uint64_t,uint64_t> UInt128;

inline UInt128 AsUInt128(uint64_t lo, uint64_t hi) { return UInt128(lo, hi); }

inline uint64_t UInt128Low64 (const UInt128 &x) { return x.first;  }
inline uint64_t UInt128High64(const UInt128 &x) { return x.second; }

static_assert( sizeof(UInt128) == 16, "the 'UInt128' type must be packed for compatibility with the 'uint8_t[16]' type" );

#endif // defined(UInt128)
// ---------------------------------------------------------------------
// Done.

#endif // ! UINT128_HPP
