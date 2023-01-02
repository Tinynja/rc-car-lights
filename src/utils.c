#include <avr/io.h>

uint8_t clamp(uint8_t value, uint8_t min, uint8_t max) {
	if (value < min) {
		return min;
	} else if (value > max) {
		return max;
	} else {
		return value;
	}
}

uint16_t normalize_duty(uint16_t value, uint16_t range) {
	unsigned short last_raw_value = range/6u;
    if (value <= last_raw_value) {
        return value;
    } else {
		return (value-last_raw_value)*(255-last_raw_value)/(range-last_raw_value)+last_raw_value;
    }
}