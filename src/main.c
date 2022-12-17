#include "config.h"
#include "macros.h"
#include <avr/io.h>
#include <avr/interrupt.h>

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

#define CYCLES_PER_MS ((F_CPU/100/256+5)/10)
uint32_t clk = 0;

struct {
	uint8_t portx: 6; // WARNING: Change size if using an MCU with I/O addresses higher 0x3F
	uint8_t pbx: 3;
	uint8_t pw_min: 7; // minimum pulse width
	uint8_t pw_range: 7; // pulse width range
} RxChConfig[2] = {
	{&GPORT(RX_CH1), GBIT(RX_CH1), CYCLES_PER_MS*4/3, CYCLES_PER_MS/3},
	{&GPORT(RX_CH2), GBIT(RX_CH2), CYCLES_PER_MS*4/3, CYCLES_PER_MS/3},
};

struct {
	uint8_t rising_edge_time: 8;
	uint8_t value_raw: 8; // could be removed if flash space is limited
	uint8_t value: 7;
	uint8_t state: 1;
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