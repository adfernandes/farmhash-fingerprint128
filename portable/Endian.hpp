#ifndef ENDIAN_HPP
#define ENDIAN_HPP

// ---------------------------------------------------------------------
// Determine platform endianness.

#if defined(_WIN32)
#   include <Windows.h>
#endif

#if defined(__LITTLE_ENDIAN__) || defined(_LITTLE_ENDIAN) || (defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)) || (defined(REG_DWORD_LITTLE_ENDIAN) && defined(REG_QWORD_LITTLE_ENDIAN))
#   define IS_LITTLE_ENDIAN
#elif defined(__BIG_ENDIAN__) || defined(_BIG_ENDIAN) || (defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)) || (defined(REG_DWORD_BIG_ENDIAN) && defined(REG_QWORD_BIG_ENDIAN))
#   define IS_BIG_ENDIAN
#endif

#if (defined(IS_LITTLE_ENDIAN) && defined(IS_BIG_ENDIAN)) || (!defined(IS_LITTLE_ENDIAN) && !defined(IS_BIG_ENDIAN))
#   error "error in determining platform endianness"
#endif

// ---------------------------------------------------------------------
// Cross-platform byteswapping functions.

#if (defined(bswap_32) && !defined(bswap_64)) || (!defined(bswap_32) && defined(bswap_64))

    #error "either both or neither byteswapping macros must be defined here"

#elif defined(__clang__) || \
    defined(__INTEL_COMPILER) || \
   (defined(__GNUC__) && ((__GNUC__ >= 5) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)))

    #define bswap_32(x) __builtin_bswap32(x)
    #define bswap_64(x) __builtin_bswap64(x)

#elif defined(_WIN32)

    #include <stdlib.h>

    #define bswap_32(x) _byteswap_ulong(x)
    #define bswap_64(x) _byteswap_uint64(x)

#elif defined(__APPLE__)

    #include <libkern/OSByteOrder.h>

    #define bswap_32(x) OSSwapInt32(x)
    #define bswap_64(x) OSSwapInt64(x)

#elif defined(__sun) || defined(sun)

    #include <sys/byteorder.h>

    #define bswap_32(x) BSWAP_32(x)
    #define bswap_64(x) BSWAP_64(x)

#elif defined(__FreeBSD__)

    #include <sys/endian.h>

    #define bswap_32(x) bswap32(x)
    #define bswap_64(x) bswap64(x)

#elif defined(__OpenBSD__)

    #include <sys/types.h>

    #define bswap_32(x) swap32(x)
    #define bswap_64(x) swap64(x)

#elif defined(__NetBSD__)

    #include <sys/bswap.h>

    #define bswap_32(x) bswap32(x)
    #define bswap_64(x) bswap64(x)

#else

    #include <byteswap.h>

#endif

#if !(defined(bswap_32) && defined(bswap_64))
#   error "both byteswapping macros must be defined here"
#endif

#ifdef IS_BIG_ENDIAN
    #define uint32_in_little_endian_order(x) (bswap_32(x))
    #define uint64_in_little_endian_order(x) (bswap_64(x))
#else
    #define uint32_in_little_endian_order(x) (x)
    #define uint64_in_little_endian_order(x) (x)
#endif

// ---------------------------------------------------------------------
// Done.

#endif // ! ENDIAN_HPP
