#ifndef UTILS_H
#define UTILS_H

// Macros
#define bit_is_set_bool(sfr, bit) (bit_is_set(sfr, bit) >> bit)
#define bit_is_clear_bool(sfr, bit) (!(bit_is_set(sfr, bit) >> bit))
#define log2(value) (log(value)/log(2))

// Functions
uint8_t clamp(uint8_t value, uint8_t min, uint8_t max);
uint16_t normalize_duty(uint16_t value, uint16_t range);

#endif //UTILS_H