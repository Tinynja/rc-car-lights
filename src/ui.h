#ifndef UI_H
#define UI_H


// The mask serves two purposes: a mask, and a state (enabled/disabled)
#define SUBSYS_BRAKES			0
#define SUBSYS_BRAKES_MASK		1
#define SUBSYS_BLINKERS			2
#define SUBSYS_BLINKERS_MASK	3
#define SUBSYS_STAGING			4

#define COMBO2(a1,a2)		((a1 << 2) | (a2))
#define COMBO4(a1,a2,a3,a4)	((a1 << 6) | (a2 << 4) | (a3 << 2) | (a4))

#define COMBO_LIGHTS	COMBO2(LEFT,LEFT)
#define COMBO_HAZARDS	COMBO2(RIGHT,RIGHT)
#define COMBO_CONFIG	COMBO4(LEFT,RIGHT,LEFT,RIGHT)

// All in 100ms unit
#define COMBO_MIN_DURATION	3
#define COMBO_MAX_DURATION	6
#define COMBO_MAX_PAUSE		2
#define LIST_BLINK_DURATION	2
#define LIST_PAUSE_DURATION	10
#define CFG_DUTY_HOLD_DURATION	30
#define CFG_CONFIRMATION_DURATION	10

#define UI_PAGE_MAIN			0
#define UI_PAGE_CONFIGURATION	1
#define UI_PAGE_CFG_LIGHTS_DUTY	2
#define UI_PAGE_CFG_CHREVERSE	3
#define UI_PAGE_CFG_CALIBRATION	4

#define UI_CFG_IDX_FRONT_DUTY	1
#define UI_CFG_IDX_REAR_DUTY	2
#define UI_CFG_IDX_BRAKE_DUTY	3
#define UI_CFG_IDX_BLINKERS		4
#define UI_CFG_IDX_CHREVERSE	5
#define UI_CFG_IDX_CALIBRATION	6
#define UI_CFG_IDX_MAIN			7

struct Ui_t {
	uint8_t page;
	uint8_t timer;
	uint8_t flag;
	uint8_t var1; // multi-purpose variable (see defines below)
	uint8_t var2; // multi-purpose variable (see defines below)
	uint8_t var3; // multi-purpose variable (see defines below)
};
// Used in ui_page_main()
#define combo		var1
#define hazards		var2
// Used in ui_page_generic_list()
#define index		var1
#define blink_index	var2
#define max_index	var3
// User in ui_page_cfg_front()
#define light_register	var1
#define last_value		var2

void ui_page_main();

void ui_pageinit_configuration();

void ui_page_generic_list();

void ui_list_configuration();

void ui_list_cfg_chreverse();

void ui_page_cfg_duty();

void ui_page_cfg_calibration();


#endif //UI_H