#include "../config.h"
#include <avr/io.h>
#include <avr/eeprom.h>

extern char _binary_obj_eeprom_bin_start[];
extern char _binary_obj_eeprom_bin_end[];

int main(void) {
	volatile char PROTECT_THIS_BLOCK_FROM_OPTIMIZATIONS[3] = "##";
	eeprom_update_block(_binary_obj_eeprom_bin_start, 0, (_binary_obj_eeprom_bin_end - _binary_obj_eeprom_bin_start));
}