#include "typedefs.h"
#include "registers.h"
#include "config.h"
#include "utils.h"
#include "Ui.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <math.h>

// The following values can be calibrated in runtime
#define DEFAULT_PW_MIN				CYCLES_PER_MS*5/4
#define DEFAULT_PW_RANGE			CYCLES_PER_MS/2
#define DEFAULT_REVERSE				1
#define DEFAULT_STEER_LOW_BAND		DEFAULT_PW_RANGE/6
#define DEFAULT_STEER_HIGH_BAND		DEFAULT_PW_RANGE-DEFAULT_STEER_LOW_BAND
#define DEFAULT_THROTTLE_LOW_BAND	0
#define DEFAULT_THROTTLE_HIGH_BAND	DEFAULT_PW_RANGE/2

/* Clock frequency consideration
At 9.6MHz:
	16-bit overflow: 1.75 seconds
	32-bit overflow: 31.81 hours
	Clock step/(9.6MHz/256) = 0.026ms
				1ms = 37.5 steps
				2ms = 75 steps
At 16MHz:
	16-bit overflow: 1.05 seconds
	32-bit overflow: 19.09 hours
	Clock step/(16MHz/256) = 0.016ms
				1ms = 62.5 steps
				2ms = 125 steps
*/
uint16_t clk; // 1/CYCLES_PER_MS step clock
uint8_t clk_100ms; // ~100ms step clock

struct Config_t Config = {
	.front_duty = 127, .rear_duty = 32, .brake_duty = 255, .blinkers = 1, .lights = !0
};

struct RxChConfig_t RxChConfig[2] = {
	{&GPORT(RX_PORT), RX_CH1, DEFAULT_PW_MIN, DEFAULT_PW_RANGE, DEFAULT_REVERSE, DEFAULT_STEER_LOW_BAND, DEFAULT_STEER_HIGH_BAND},
	{&GPORT(RX_PORT), RX_CH2, DEFAULT_PW_MIN, DEFAULT_PW_RANGE, DEFAULT_REVERSE, DEFAULT_THROTTLE_LOW_BAND, DEFAULT_THROTTLE_HIGH_BAND},
};

struct RxCh_t RxCh[2];

extern struct Ui_t Ui;


int main(void) {
	#ifdef OSCCAL_VALUE
	OSCCAL = OSCCAL_VALUE;
	#endif

	// Read Config from EEPROM
	// TODO

	// Set receiver channel pins as input
	_MMIO_BYTE(RxChConfig[STEERING].ddrx) &= ~_BV(RxChConfig[STEERING].pxn);
	_MMIO_BYTE(RxChConfig[THROTTLE].ddrx) &= ~_BV(RxChConfig[THROTTLE].pxn);
	
	// Set TIMERn: Fast PWM Mode, TOP=0xFF, No prescaler, TOV enabled, OC0A enabled, OC0B enabled
	TCCRnA = _BV(COMnA1) | _BV(COMnB1) | _BV(WGMn1) | _BV(WGMn0);
	TCCRnB = _BV(CSn0);
	TIMSKn = _BV(TOIEn);
	OCRnA = Config.front_duty;
	OCRnB = Config.rear_duty;
	
	// PCINT_CH1 and PCINT_CH2 enabled
	PCICR_REG = _BV(PCIE_BIT);
	PCMSK_REG = _BV(PCINT_CH1_BIT) | _BV(PCINT_CH2_BIT);
	
	sei();

	while (1) {
		for (uint8_t i=0; i<sizeof(RxCh)/sizeof(RxCh[0]); i++) {
			RxCh[i].value = clamp(RxCh[i].value_raw, RxChConfig[i].pw_min, RxChConfig[i].pw_min + RxChConfig[i].pw_range) \
							- RxChConfig[i].pw_min;
			RxCh[i].value = RxChConfig[i].reverse ? RxChConfig[i].pw_range - RxCh[i].value : RxCh[i].value;

			uint8_t A1 = RxCh[i].value <= (RxChConfig[i].low_band-HYSTERESIS);
			uint8_t A2 = RxCh[i].value >= (RxChConfig[i].low_band+HYSTERESIS);
			uint8_t B = RxCh[i].band != LEFT;
			uint8_t C1 = RxCh[i].value >= (RxChConfig[i].high_band+HYSTERESIS);
			uint8_t C2 = RxCh[i].value <= (RxChConfig[i].high_band-HYSTERESIS);
			uint8_t D = RxCh[i].band != RIGHT;
			if ((A1 && B) || (C1 && D) || (A2 && C2 && !(B && D)) ) {
					if (A1) {
						RxCh[i].band = LOW_BAND;
					} else if (C1) {
						RxCh[i].band = HIGH_BAND;
					} else {
						RxCh[i].band = MIDDLE_BAND;
					}
					RxCh[i].band_start_time = clk_100ms;
					RxCh[i].band_duration = 0;
			} else if (RxCh[i].band_duration <= 254) {
				RxCh[i].band_duration = clk_100ms - RxCh[i].band_start_time;
			}
		}
	
		switch (Ui.page) {
			case UI_PAGE_MAIN:
				ui_page_main();
				break;
			case UI_PAGE_CFG_LIGHTS_DUTY:
				ui_page_cfg_duty();
				break;
			case UI_PAGE_CONFIGURATION:
			case UI_PAGE_CFG_CHREVERSE:
				ui_page_generic_list();
				break;
			case UI_PAGE_CFG_CALIBRATION:
				ui_page_cfg_calibration();
				break;
		}
	}
}

ISR(TIMERn_OVF_vect) {
	if (clk++ << (16-(uint8_t)(log2(CYCLES_PER_MS*100)+0.5)) == 0)
		clk_100ms++;
}

ISR(PCINT_vect) {
	// Start timing on rising edge of PWM
	// Calculate value on falling edge of PWM
	for (uint8_t i=0; i<sizeof(RxCh)/sizeof(RxCh[0]); i++) {
		uint8_t new_state = bit_is_set_bool(_MMIO_BYTE(RxChConfig[i].pinx), RxChConfig[i].pxn);
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