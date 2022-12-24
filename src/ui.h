#ifndef UI_H
#define UI_H


// The mask serves two purposes: a mask, and a state (enabled/disabled)
#define SUBSYS_BRAKES			0
#define SUBSYS_BRAKES_MASK		1
#define SUBSYS_BLINKERS			2
#define SUBSYS_BLINKERS_MASK	3
#define SUBSYS_HAZARDS			4
#define SUBSYS_HAZARDS_MASK		5
#define SUBSYS_STAGING			6

#define COMBO2(a1,a2)		((a1 << 2) | (a2))
#define COMBO4(a1,a2,a3,a4)	((a1 << 6) | (a2 << 4) | (a3 << 2) | (a4))

#define COMBO_LIGHTS	COMBO2(LEFT,LEFT)
#define COMBO_HAZARDS	COMBO2(RIGHT,RIGHT)
#define COMBO_CONFIG	COMBO4(LEFT,RIGHT,LEFT,RIGHT)

#define COMBO_MIN_DURATION	4 // in 100ms
#define COMBO_MAX_DURATION	8 // in 100ms
#define COMBO_MAX_PAUSE		2 // in 100ms

#define UI_MAIN				0
#define UI_CONFIGURATION	1
#define UI_CFG_FRONT		2
#define UI_CFG_REAR			3
#define UI_CFG_BRAKE		4
#define UI_CFG_BLINKERS		5
#define UI_CFG_CHREVERSE	6
#define UI_CFG_CALIBRATION	7

struct Ui_t {
	uint8_t page;
	uint8_t timer;
	uint8_t combo;
	uint8_t flag;
};

void ui_main();

void ui_configuration();

void ui_cfg_front();

void ui_cfg_rear();

void ui_cfg_brake();

void ui_cfg_blinkers();

void ui_cfg_chreverse();

void ui_cfg_calibration();


#endif //UI_H