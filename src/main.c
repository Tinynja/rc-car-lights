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
#define DEFAULT_STEER_LOW_BAND		DEFAULT_PW_RANGE*1/3
#define DEFAULT_STEER_HIGH_BAND		DEFAULT_PW_RANGE*2/3
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
			case UI_CONFIGURATION:
				ui_configuration();
				break;
			case UI_CFG_FRONT:
				ui_cfg_front();
				break;
			case UI_CFG_REAR:
				ui_cfg_rear();
				break;
			case UI_CFG_BRAKE:
				ui_cfg_brake();
				break;
			case UI_CFG_BLINKERS:
				ui_cfg_blinkers();
				break;
			case UI_CFG_CHREVERSE:
				ui_cfg_chreverse();
				break;
			case UI_CFG_CALIBRATION:
				ui_cfg_calibration();
				break;
			default:
				ui_main();
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

uint8_t clamp(uint8_t value, uint8_t min, uint8_t max) {
	if (value < min) {
		return min;
	} else if (value > max) {
		return max;
	} else {
		return value;
	}
}

// uint8_t multiply_fraction_repeatedly(uint8_t a, uint8_t b_num, uint8_t b_den, uint8_t n) {
//     // Multiply number *a* by fraction *b_num/b_den*, *n* times,
//     // round to neareast unit each time
//     // WARNING: choose b_num wisely so that 2*a*b_num does not exceed sizeof(int)
//     for (uint8_t i = 1; i <= n; i++)
//         // (2*___+1)/2 rounds to unit without using floats
//         a = (2*a*b_num/b_den+1)/2;
//     return a;
// }

// uint8_t normalize_pwm(uint8_t pwm) {
//     if (pwm <= 19) {
//         return pwm;
//     } else if (pwm <= 27) {
//         return multiply_fraction_repeatedly(20, 34, 29, pwm-19);
//     } else {
//         return multiply_fraction_repeatedly(255, 29, 34, 35-pwm);
//     }
// }