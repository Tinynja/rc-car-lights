#include "typedefs.h"
#include "registers.h"
#include "utils.h"
#include "ui.h"
#include <avr/io.h>
#include <avr/eeprom.h>

extern uint8_t clk_100ms;
extern struct Config_t Config;
extern struct Config_t ConfigEE EEMEM;
extern struct RxChConfig_t RxChConfig[2];
extern struct RxChConfig_t RxChConfigEE[2] EEMEM;
extern struct RxCh_t RxCh[2];

struct Ui_t Ui = {
	.combo = COMBO_LIGHTS
};

volatile uint8_t Lights[SUBSYS_STAGING+1];
/*
All steering action combinations involve "moving out of the middle then going back to middle".
Therefore it is sufficient to simply encode an action combination as a series of left/right movements (e.g. LRLR)
Left/Right data is given by the RxCh.band variable, and this data requires 2 bits per movement.
Therefore in a byte we could store 4 movements.
*/



void ui_page_main() {
	/*
	Each subsystem (lights, blinkers, brakes, hazards) has its own set of rules it wants
	to follow. In total, there are 16 combinations of every subsystem state, with 11 unique
	solutions (see "extras/Light subsystems.xlsx"). I think hardcoding every combination in
	a switch case would take too much Flash space, so we proceed differently.
	Each subsystem is given a priority (1-highest to 4-lowest), and they take turn in
	setting/clearing the bits of a staging copy of the DDRx register as they please. Higher
	priority subsystems overwrite the bits of lower priority subsystems.
	Once everysubsystem has passed, the staging copy is written in the real DDRx register.
	Subsystem priority:
		1. hazards
		2. blinkers
		3. brakes
		4. lights
	*/
	if (RxCh[STEERING].band != CENTER) {
		// Combo detection
		if (Ui.flag == 0 && RxCh[STEERING].band_duration > COMBO_MIN_DURATION) {
			Ui.combo = (Ui.combo << 2) | RxCh[STEERING].band;
			Ui.flag = 1;

			// Blinkers
			if (Config.blinkers && !Ui.hazards) {
				Lights[SUBSYS_BLINKERS] = Lights[SUBSYS_STAGING];
				Lights[SUBSYS_BLINKERS_MASK] = RxCh[STEERING].band == LEFT ? _BV(LIGHTS_LEFT) : _BV(LIGHTS_RIGHT);
				Ui.timer = clk_100ms;
			}
		} else if ((Ui.combo & 0x3) && RxCh[STEERING].band_duration > COMBO_MAX_DURATION) {
			Ui.combo = Ui.combo << 2;
		}
	} else if ((Ui.combo & 0x3) && RxCh[STEERING].band_duration > COMBO_MAX_PAUSE) {
		Ui.combo = Ui.combo << 2;
	} else {
		// Blinkers
		Lights[SUBSYS_BLINKERS_MASK] = 0;

		// Combo detection
		Ui.flag = 0;
		if ((Ui.combo & 0xF) == COMBO_LIGHTS) {
			Config.lights = !Config.lights;
			Lights[SUBSYS_BRAKES] = LEFT_RIGHT_MASK | (Config.lights << LIGHTS_FRONT);
			Ui.combo = Ui.combo << 2;
			eeprom_update_byte(&ConfigEE.lights, Config.lights);
		} else if ((Ui.combo & 0xF) == COMBO_HAZARDS) {
			Lights[SUBSYS_BLINKERS] = -Config.lights;
			Ui.hazards = !Ui.hazards;
			Ui.timer = clk_100ms + COMBO_MIN_DURATION;
			Ui.combo = Ui.combo << 2;
		} else if (Ui.combo == COMBO_CONFIG) {
			ui_pageinit_configuration();
		}
	}

	if (clk_100ms == Ui.timer) {
		Lights[SUBSYS_BLINKERS] = ~Lights[SUBSYS_BLINKERS];
		Ui.timer += COMBO_MIN_DURATION;
	}

	// Throttle channel
	switch (RxCh[THROTTLE].band) {
		case BRAKE:
			OCRnB = Config.brake_duty;
			Lights[SUBSYS_BRAKES_MASK] = LEFT_RIGHT_MASK | _BV(LIGHTS_FRONT);
			Lights[SUBSYS_BLINKERS_MASK] = Ui.hazards ? _BV(LIGHTS_FRONT) : Lights[SUBSYS_BLINKERS_MASK];
			break;
		default:
			OCRnB = Config.rear_duty;
			Lights[SUBSYS_BRAKES_MASK] = 0;
			Lights[SUBSYS_BLINKERS_MASK] = Ui.hazards ? LEFT_RIGHT_MASK : Lights[SUBSYS_BLINKERS_MASK];
	}

	// Ugly but uses less flash than for loop
	Lights[SUBSYS_STAGING] = (((((Config.lights ? LEFT_RIGHT_MASK : 0) | FRONT_REAR_MASK) \
								& ~Lights[SUBSYS_BRAKES_MASK]) | (Lights[SUBSYS_BRAKES] & Lights[SUBSYS_BRAKES_MASK])) \
								& ~Lights[SUBSYS_BLINKERS_MASK]) | (Lights[SUBSYS_BLINKERS] & Lights[SUBSYS_BLINKERS_MASK]);
	GDDR(LIGHTS_PORT) = Lights[SUBSYS_STAGING];

}

void ui_pageinit_configuration() {
	GDDR(LIGHTS_PORT) = FRONT_REAR_MASK;
	OCRnA = OCRnB = 255;
	Ui.timer = clk_100ms+1;
	Ui.flag = 0;
	Ui.index = Ui.blink_index = 1;
	Ui.max_index = UI_CFG_IDX_MAIN;
	Ui.page = UI_PAGE_CONFIGURATION;
}

void ui_page_generic_list() {
	if (clk_100ms == Ui.timer) {
		if (--Ui.blink_index) {
			GDDR(LIGHTS_PORT) ^= LEFT_RIGHT_MASK;
			Ui.timer += LIST_BLINK_DURATION;
		} else {
			GDDR(LIGHTS_PORT) &= ~LEFT_RIGHT_MASK;
			Ui.timer += LIST_PAUSE_DURATION;
			Ui.blink_index = Ui.index << 1;
		}
	}
	
	switch (RxCh[STEERING].band) {
		case LEFT:
			if (Ui.flag == 0) {
				Ui.flag = 1;
				Ui.index = ++Ui.index == (Ui.max_index+1) ? 1 : Ui.index;
				Ui.blink_index = 1;
				Ui.timer = clk_100ms;
			}
			break;
		case RIGHT:
			Ui.flag = 2;
			break;
		default:
			switch (Ui.page) {
				case UI_PAGE_CONFIGURATION:
					ui_list_configuration();
					break;
				case UI_PAGE_CFG_CHREVERSE:
					ui_list_cfg_chreverse();
			}
	}
}

void ui_list_configuration() {
	if (Ui.flag == 2) {
		if (Ui.index <= 3) {
			Ui.flag = Ui.index;
			Ui.page = UI_PAGE_CFG_LIGHTS_DUTY;
			GDDR(LIGHTS_PORT) = ALL_MASK;
			Ui.timer = clk_100ms + CFG_DUTY_HOLD_DURATION;
		} else {
			Ui.flag = 0;
		}
		switch (Ui.index) {
			case UI_CFG_IDX_FRONT_DUTY:
				Ui.light_register = &OCRnA;
				break;
			case UI_CFG_IDX_REAR_DUTY:
			case UI_CFG_IDX_BRAKE_DUTY:
				Ui.light_register = &OCRnB;
				break;
			case UI_CFG_IDX_BLINKERS:
				Config.blinkers = !Config.blinkers;
				GDDR(LIGHTS_PORT) |= LEFT_RIGHT_MASK;
				Ui.index = Ui.blink_index = 1;
				Ui.timer = clk_100ms + CFG_CONFIRMATION_DURATION;
				break;
			case UI_CFG_IDX_CHREVERSE:
				Ui.page = UI_PAGE_CFG_CHREVERSE;
				Ui.index = Ui.blink_index = 1;
				Ui.max_index = sizeof(RxChConfig)/sizeof(RxChConfig[0]);
				break;
			case UI_CFG_IDX_CALIBRATION:
				GDDR(LIGHTS_PORT) &= ~LEFT_RIGHT_MASK;
				Ui.page = UI_PAGE_CFG_CALIBRATION;
				Ui.timer = clk_100ms+30;
				Ui.flag = 0;
				break;
			case UI_CFG_IDX_MAIN:
				// TODO: SAVE CONFIG TO EEPROM
				OCRnA = Config.front_duty;
				Ui.page = UI_PAGE_MAIN;
				Ui.combo = Ui.hazards = 0;
				eeprom_update_block(&Config, &ConfigEE, sizeof(Config));
		}
	} else {
		Ui.flag = 0;
	}
}

void ui_list_cfg_chreverse() {
	if (Ui.flag == 2) {
		RxChConfig[Ui.index-1].reverse = !RxChConfig[Ui.index-1].reverse;
		ui_pageinit_configuration();
		GDDR(LIGHTS_PORT) |= LEFT_RIGHT_MASK;
		Ui.timer = clk_100ms + CFG_CONFIRMATION_DURATION;
	}
	Ui.flag = 0;
}

void ui_page_cfg_duty() {
	if (Ui.flag && clk_100ms != Ui.timer) {
		*(uint8_t*) Ui.light_register = normalize_duty(RxCh[STEERING].value, RxChConfig[STEERING].pw_range);
		if (RxCh[STEERING].value < Ui.last_value-1 || RxCh[STEERING].value > Ui.last_value+1) {
			Ui.last_value = RxCh[STEERING].value;
			Ui.timer = clk_100ms + CFG_DUTY_HOLD_DURATION;
		}
	} else {
		GDDR(LIGHTS_PORT) &= ~LEFT_RIGHT_MASK;
		switch (Ui.flag) {
			case UI_CFG_IDX_FRONT_DUTY:
				Config.front_duty = *(uint8_t*) Ui.light_register;
				break;
			case UI_CFG_IDX_REAR_DUTY:
				Config.rear_duty = *(uint8_t*) Ui.light_register;
				break;
			case UI_CFG_IDX_BRAKE_DUTY:
				Config.brake_duty = *(uint8_t*) Ui.light_register;
		}
		Ui.flag = 0;
		if (RxCh[STEERING].band == CENTER) {
			ui_pageinit_configuration();
		}
	}
}

void ui_page_cfg_calibration() {
	if (Ui.flag < 6) {
		if (clk_100ms == Ui.timer) {
			GDDR(LIGHTS_PORT) ^= LEFT_RIGHT_MASK;
			Ui.timer += Ui.flag & 1 ? 30 : 5;

			// Ugly but uses less flash than for loop
			switch (Ui.flag) {
				case 0:
					RxChConfig[STEERING].pw_min = RxCh[STEERING].value_raw;
					RxChConfig[THROTTLE].pw_min = RxCh[THROTTLE].value_raw;
					break;
				case 2:
					if (RxCh[STEERING].value_raw > RxChConfig[STEERING].pw_min) {
						RxChConfig[STEERING].pw_range = RxCh[STEERING].value_raw - RxChConfig[STEERING].pw_min;
					} else {
						RxChConfig[STEERING].pw_range = RxChConfig[STEERING].pw_min - RxCh[STEERING].value_raw;
						RxChConfig[STEERING].pw_min = RxCh[STEERING].value_raw;
					}
					if (RxCh[THROTTLE].value_raw > RxChConfig[THROTTLE].pw_min) {
						RxChConfig[THROTTLE].pw_range = RxCh[THROTTLE].value_raw - RxChConfig[THROTTLE].pw_min;
					} else {
						RxChConfig[THROTTLE].pw_range = RxChConfig[THROTTLE].pw_min - RxCh[THROTTLE].value_raw;
						RxChConfig[THROTTLE].pw_min = RxCh[THROTTLE].value_raw;
					}
			}

			Ui.flag++;
		}
	} else {
		RxChConfig[STEERING].low_band = RxChConfig[STEERING].pw_range/8u;
		RxChConfig[STEERING].high_band = RxChConfig[STEERING].pw_range - RxChConfig[STEERING].low_band;
		RxChConfig[THROTTLE].low_band = 0;
		RxChConfig[THROTTLE].high_band = RxCh[THROTTLE].value + RxChConfig[THROTTLE].pw_range/16u;
		
		ui_pageinit_configuration();
		eeprom_update_block(&RxChConfig, &RxChConfigEE, sizeof(RxChConfig));
	}
}