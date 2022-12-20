#include "config.h"
#include "macros.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/atomic.h>

// The variables corresponding to these defaults can be set in runtime
#define DEFAULT_PW_MIN		CYCLES_PER_MS*5/4
#define DEFAULT_PW_RANGE	CYCLES_PER_MS/2
#define DEFAULT_REVERSE		0
#define DEFAULT_LOW_BAND	DEFAULT_PW_RANGE*1/3
#define DEFAULT_HIGH_BAND	DEFAULT_PW_RANGE*2/3

/* Clock frequency consideration
At 9.6MHz:
	16-bit overflow: 1.75 seconds
	32-bit overflow: 31.81 hours
	Clock step: 1/(9.6MHz/256) = 0.026ms
				1ms = 37.5 steps
				2ms = 75 steps
At 16MHz:
	16-bit overflow: 1.05 seconds
	32-bit overflow: 19.09 hours
	Clock step: 1/(16MHz/256) = 0.016ms
				1ms = 62.5 steps
				2ms = 125 steps
*/
uint32_t clk;

uint8_t clamp(uint8_t value, uint8_t min, uint8_t max) {
	if (value < min) {
		return min;
	} else if (value > max) {
		return max;
	} else {
		return value;
	}
}

struct {
	uint8_t portx: 6; // WARNING: Change size if using an MCU with I/O addresses higher 0x3F
	uint8_t pbx: 3;
	uint8_t pw_min: 7; // minimum pulse width
	uint8_t pw_range: 7; // pulse width range
	uint8_t reverse: 1; // channel reversing
	uint8_t low_band: 7;
	uint8_t high_band: 7;
} RxChConfig[2] = {
	{&GPORT(RX_CH1), GBIT(RX_CH1), DEFAULT_PW_MIN, DEFAULT_PW_RANGE, 0, DEFAULT_LOW_BAND, DEFAULT_HIGH_BAND},
	{&GPORT(RX_CH2), GBIT(RX_CH2), DEFAULT_PW_MIN, DEFAULT_PW_RANGE, 0, DEFAULT_LOW_BAND, DEFAULT_HIGH_BAND},
};

struct {
	uint8_t rising_edge_time: 8;
	uint8_t value_raw: 8; // could be removed if flash space is limited
	uint8_t value: 7;
	uint8_t state: 1;
	uint8_t band: 2;
	uint8_t band_start_time: 6; // in 0.1s unit
	uint8_t band_duration: 6; // could be removed if flash space is limited
} RxCh[2];


int main(void) {
	#ifdef OSCCAL_VALUE
	OSCCAL = OSCCAL_VALUE;
	#endif

	// Set light control pins as output
	GDDR(LIGHTS_FRONT) |= _BV(GBIT(LIGHTS_FRONT));
	GDDR(LIGHTS_REAR) |= _BV(GBIT(LIGHTS_REAR));
	GDDR(LIGHTS_LEFT) |= _BV(GBIT(LIGHTS_LEFT));
	GDDR(LIGHTS_RIGHT) |= _BV(GBIT(LIGHTS_RIGHT));

	// Set receiver channel pins as input
	_MMIO_BYTE(RxChConfig[0].ddrx) &= ~_BV(RxChConfig[0].pbx);
	_MMIO_BYTE(RxChConfig[1].ddrx) &= ~_BV(RxChConfig[1].pbx);
	
	// Set TIMERn: Fast PWM Mode, TOP=0xFF, No prescaler, TOV enabled, OC0A enabled, OC0B enabled
	TCCRnA = _BV(COMnA1) | _BV(COMnB1) | _BV(WGMn1) | _BV(WGMn0);
	TCCRnB = _BV(CSn0);
	TIMSKn = _BV(TOIEn);
	
	// PCINT_CH1 and PCINT_CH2 enabled
	PCICR_REG = _BV(PCIE_BIT);
	PCMSK_REG = _BV(PCINT_CH1_BIT) | _BV(PCINT_CH2_BIT);
	
	sei();
	
	// uart_init(BAUD_CALC(250000));
	while (1) {
		// ATOMIC_BLOCK needed because RxCh.value and RxCh.state are bitfields of the same byte
		// and they are both being modified here and in the ISR
		for (uint8_t i=0; i<sizeof(RxCh)/sizeof(RxCh[0]); i++) {
			ATOMIC_BLOCK(ATOMIC_FORCEON) {
				RxCh[i].value = clamp(RxCh[i].value_raw, RxChConfig[i].pw_min, RxChConfig[i].pw_min+RxChConfig[i].pw_range) \
								- RxChConfig[i].pw_min;
			}
			if ((RxCh[i].value < (RxChConfig[i].low_band-HYSTERESIS) && RxCh[i].band != 0) || \
				(RxCh[i].value > (RxChConfig[i].high_band+HYSTERESIS) && RxCh[i].band != 2) || \
				(RxCh[i].value > (RxChConfig[i].low_band) && RxCh[i].value < (RxChConfig[i].high_band) && RxCh[i].band != 1) ) {
					if (RxCh[i].value < RxChConfig[i].low_band) {
						RxCh[i].band = 0;
					} else if (RxCh[i].value > RxChConfig[i].high_band) {
						RxCh[i].band = 2;
					} else {
						RxCh[i].band = 1;
					}
					RxCh[i].band_start_time = clk/(100*CYCLES_PER_MS);
			}
			if (RxCh[i].band_duration < 48)
				RxCh[i].band_duration = clk/(100*CYCLES_PER_MS)-RxCh[i].band_start_time;
		}

		if (RxCh[0].band == 0 && RxCh[0].band_duration < 20) {
			OCRnA = 255;
		} else if (RxCh[0].band == 2 && RxCh[0].band_duration < 20) {
			OCRnA = 40;
		} else {
			OCRnA = 0;
		}
	}
}

ISR(TIMERn_OVF_vect) {
	clk++;
}

ISR(PCINT_vect) {
	// Start timing on rising edge of PWM
	// Calculate value on falling edge of PWM
	for (uint8_t i=0; i<sizeof(RxCh)/sizeof(RxCh[0]); i++) {
		uint8_t new_state = bit_is_set_bool(_MMIO_BYTE(RxChConfig[i].pinx), RxChConfig[i].pbx);
		if (!RxCh[i].state && new_state) {
			// rising edge
			RxCh[i].rising_edge_time = clk;
		} else if (RxCh[i].state && !new_state) {
			// falling edge
			RxCh[i].value_raw = clk-RxCh[i].rising_edge_time;
		}
		RxCh[i].state = new_state;
	}
}