#include "typedefs.h"
#include "registers.h"
#include "ui.h"
#include <avr/io.h>

extern uint8_t clk_100ms;
extern struct Config_t Config;
extern struct RxChConfig_t RxChConfig[2];
extern struct RxCh_t RxCh[2];

struct Ui_t Ui;

volatile uint8_t Lights[SUBSYS_STAGING+1];
/*
All steering action combinations involve "moving out of the middle then going back to middle".
Therefore it is sufficient to simply encode an action combination as a series of left/right movements (e.g. LRLR)
Left/Right data is given by the RxCh.band variable, and this data requires 2 bits per movement.
Therefore in a byte we could store 4 movements.
*/

void ui_main() {
	/*
	Each subsystem (lights, blinkers, brakes, hazards) has its own set of rules it wants
	to follow. In total, there are 16 combinations of every subsystem state, with 11 unique
	solutions (see "extras/Light subsystems.xlsx"). I think hardcoding every combination in
	a switch case would take too much Flash space, so we proceed differently.
	Each subsystem is given a priority (1-highest to 4-lowest), and they take turn in
	setting/clearing the bits of a copy of the DDRx register as they please. Higher
	priority subsystems overwrite the bits of lower priority subsystems.
	Once everysubsystem has passed, the copy is written in the real DDRx register.
	Subsystem priority:
		1. hazards
		2. blinkers
		3. brakes
		4. lights
	*/
	if (RxCh[STEERING].band != CENTER) {
		// Combo detection
		if ((Ui.flag == 0) && RxCh[STEERING].band_duration > COMBO_MIN_DURATION) {
			Ui.combo = (Ui.combo << 2) | RxCh[STEERING].band;
			Ui.flag = 1;

			// Blinkers
			if (Config.blinkers && !Lights[SUBSYS_HAZARDS_MASK]) {
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
		if (Ui.flag == 1) {
			Lights[SUBSYS_BLINKERS_MASK] = 0;
			Lights[SUBSYS_STAGING] = 0;
		}

		// Combo detection
		Ui.flag = 0;
		if ((Ui.combo & 0xF) == COMBO_LIGHTS)  {
			// TODO: Write lights to EEPROM
			Config.lights = !Config.lights;
			Lights[SUBSYS_STAGING] = 0;
			Ui.combo = Ui.combo << 2;
		} else if ((Ui.combo & 0xF) == COMBO_HAZARDS) {
			Lights[SUBSYS_HAZARDS] = Config.lights * (LEFT_RIGHT_MASK | _BV(LIGHTS_FRONT));
			Lights[SUBSYS_HAZARDS_MASK] = !Lights[SUBSYS_HAZARDS_MASK];
			Lights[SUBSYS_STAGING] = 0;
			Ui.timer = clk_100ms + COMBO_MIN_DURATION;
			Ui.combo = Ui.combo << 2;
		} else if (Ui.combo == COMBO_CONFIG) {
			Ui.combo = Ui.combo << 2;
		}
	}

	if (clk_100ms == Ui.timer) {
		Lights[SUBSYS_BLINKERS] = ~Lights[SUBSYS_BLINKERS];
		Lights[SUBSYS_HAZARDS] = ~Lights[SUBSYS_HAZARDS];
		Lights[SUBSYS_STAGING] = 0;
		Ui.timer += COMBO_MIN_DURATION;
	}

	// Throttle channel
	if (RxCh[THROTTLE].band_duration == 0 || Lights[SUBSYS_STAGING] == 0) { // not ideal, because executed continuously for the whole 100ms
		switch (RxCh[THROTTLE].band) {
			case BRAKE:
				OCRnB = Config.brake_duty;
				Lights[SUBSYS_BRAKES] = LEFT_RIGHT_MASK | (Config.lights << LIGHTS_FRONT);
				Lights[SUBSYS_BRAKES_MASK] = LEFT_RIGHT_MASK | _BV(LIGHTS_FRONT);
				Lights[SUBSYS_STAGING] = 0;
				Lights[SUBSYS_HAZARDS_MASK] = Lights[SUBSYS_HAZARDS_MASK] ? _BV(LIGHTS_FRONT) : 0;
				break;
			default:
				OCRnB = Config.rear_duty;
				Lights[SUBSYS_BRAKES_MASK] = 0;
				Lights[SUBSYS_STAGING] = 0;
				Lights[SUBSYS_HAZARDS_MASK] = Lights[SUBSYS_HAZARDS_MASK] ? LEFT_RIGHT_MASK : 0;
		}
	}

	if (Lights[SUBSYS_STAGING] == 0) {
		Lights[SUBSYS_STAGING] = (Config.lights ? LEFT_RIGHT_MASK : 0) | FRONT_REAR_MASK;
		for (uint8_t i=0; i<sizeof(Lights)/sizeof(Lights[0])-1; i += 2) {
			Lights[SUBSYS_STAGING] = (Lights[SUBSYS_STAGING] & ~Lights[i+1]) | (Lights[i] & Lights[i+1]);
		}
		GDDR(LIGHTS_PORT) = Lights[SUBSYS_STAGING];
	}
}

void ui_configuration() {
	;
}

void ui_cfg_front() {
	;
}

void ui_cfg_rear() {
	;
}

void ui_cfg_brake() {
	;
}

void ui_cfg_blinkers() {
	;
}

void ui_cfg_chreverse() {
	;
}

void ui_cfg_calibration() {
	;
}