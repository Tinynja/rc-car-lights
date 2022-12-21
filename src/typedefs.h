#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <avr/io.h>

struct Config_t {
	uint8_t front_duty;
	uint8_t rear_duty;
	uint8_t brake_duty;
	uint8_t blinkers;
};

struct RxChConfig_t {
	uint8_t portx; // WARNING: Change size if using an MCU with I/O addresses higher 0x3F
	uint8_t pbx;
	uint8_t pw_min; // minimum pulse width
	uint8_t pw_range; // pulse width range
	uint8_t reverse; // channel reversing
	uint8_t low_band;
	uint8_t high_band;
};

struct RxCh_t {
	uint8_t rising_edge_time;
	uint8_t value_raw; // could be removed if flash space is limited
	uint8_t value;
	uint8_t state;
	uint8_t band;
	uint8_t band_start_time; // in ~0.1s unit
	uint8_t band_duration; // could be removed if flash space is limited
};

#endif //TYPEDEFS_H