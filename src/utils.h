#ifndef UTILS_H
#define UTILS_H

uint8_t clamp(uint8_t value, uint8_t min, uint8_t max);

// Utils
#define bit_is_set_bool(sfr, bit) (bit_is_set(sfr, bit) >> bit)
#define bit_is_clear_bool(sfr, bit) (!(bit_is_set(sfr, bit) >> bit))
#define _EXPAND(pre, var, post) (pre ## var ## post) // needed because '##' inhibits macro expansion

// Compile-time base-2 logarithm-ish
// Taken from: https://stackoverflow.com/questions/27581671/how-to-compute-log-with-the-preprocessor
#define NEEDS_BIT(N, B)     (((unsigned long)N >> B) > 0)

#define LOG(N) \
        (NEEDS_BIT(N,  0) + NEEDS_BIT(N,  1) + \
         NEEDS_BIT(N,  2) + NEEDS_BIT(N,  3) + \
         NEEDS_BIT(N,  4) + NEEDS_BIT(N,  5) + \
         NEEDS_BIT(N,  6) + NEEDS_BIT(N,  7) + \
         NEEDS_BIT(N,  8) + NEEDS_BIT(N,  9) + \
         NEEDS_BIT(N, 10) + NEEDS_BIT(N, 11) + \
         NEEDS_BIT(N, 12) + NEEDS_BIT(N, 13) + \
         NEEDS_BIT(N, 14) + NEEDS_BIT(N, 15) + \
         NEEDS_BIT(N, 16) + NEEDS_BIT(N, 17) + \
         NEEDS_BIT(N, 18) + NEEDS_BIT(N, 19) + \
         NEEDS_BIT(N, 20) + NEEDS_BIT(N, 21) + \
         NEEDS_BIT(N, 22) + NEEDS_BIT(N, 23) + \
         NEEDS_BIT(N, 24) + NEEDS_BIT(N, 25) + \
         NEEDS_BIT(N, 26) + NEEDS_BIT(N, 27) + \
         NEEDS_BIT(N, 28) + NEEDS_BIT(N, 29) + \
         NEEDS_BIT(N, 30) + NEEDS_BIT(N, 31))

#endif //UTILS_H