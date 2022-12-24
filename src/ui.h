#ifndef UI_H
#define UI_H

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