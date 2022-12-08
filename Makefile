TARGET=RC_Car_Lights

#Programmer options
MCU?=attiny13
PROGRAMMER?=ft232r_custom
BAUD?=78600
PORT?=COM8

#Folder structure
BIN_DIR=bin
OBJ_DIR=obj
SRC_DIR=src
BUILD_DIR=build

CFLAGS=-mmcu=$(MCU) -Wall -funsigned-char -funsigned-bitfields -ffunction-sections -fdata-sections -flto -Og

SRC_LIST=$(wildcard $(SRC_DIR)/*.c)
OBJ_LIST=$(SRC_LIST:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

ifeq ($(OS),Windows_NT)
	RM=del /s /q /f
	FixPath=$(subst /,\,$1)
else
	RM=rm -rf
	FixPath=$1
endif


.PHONY: all program program-all program-flash program-eeprom clean

all: $(addprefix $(BIN_DIR)/,$(TARGET).elf $(TARGET).eep $(TARGET).hex)

build: $(BIN_DIR)/$(TARGET).elf

$(BIN_DIR)/$(TARGET).elf: $(OBJ_LIST)
	avr-gcc $(CFLAGS) -dumpdir build/ -Wl,--print-memory-usage -Wl,-Map=build/$(TARGET).map $^ -o $@

$(BIN_DIR)/$(TARGET).hex: $(BIN_DIR)/$(TARGET).elf
	avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures -O ihex $< $@

$(BIN_DIR)/$(TARGET).eep: $(BIN_DIR)/$(TARGET).elf
	avr-objcopy -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0 --no-change-warnings -O ihex $< $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	avr-gcc $(CFLAGS) $< -c -o $@

program: program-flash

program-all: program-flash program-eeprom

program-flash: $(BIN_DIR)/$(TARGET).hex
	avrdude -p $(MCU) -c $(PROGRAMMER) -P $(PORT) -b $(BAUD) -U flash:w:$<

program-eeprom: $(BIN_DIR)/$(TARGET).eep
	avrdude -p $(MCU) -c $(PROGRAMMER) -P $(PORT) -b $(BAUD) -U eeprom:w:$<

clean:
	$(RM) $(call FixPath,$(BIN_DIR)/* $(OBJ_DIR)/* $(BUILD_DIR)/*)