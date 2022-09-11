#pragma once

#if !defined(likely)
#    define likely(x) (__builtin_expect(!!(x), 1))
#endif
#if !defined(unlikely)
#    define unlikely(x) (__builtin_expect(!!(x), 0))
#endif

// static uint8_t BITS[9] = {0, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};