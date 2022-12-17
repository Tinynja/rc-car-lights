TARGET=RC_Car_Lights

#Programmer options
MCU?=attiny13
PROGRAMMER?=ft232r_custom
BAUD?=78600
PORT?=COM8

#Folder structure
BIN_DIR=bin
SRC_DIR=src
OBJ_DIR=obj
BUILD_DIR=build

CFLAGS=-mmcu=$(MCU) -Wall -Wno-int-to-pointer-cast -funsigned-char -funsigned-bitfields -ffunction-sections -fdata-sections -flto -Og

SRC_LIST=$(if $(TEST),$(SRC_DIR)/test.c,$(wildcard $(SRC_DIR)/*.c))
OBJ_LIST=$(SRC_LIST:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEP_LIST=$(SRC_LIST:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.d)

ifeq ($(OS),Windows_NT)
	RM=del /s /q /f
	FixPath=$(subst /,\,$1)
else
	RM=rm -rf
	FixPath=$1
endif


.PHONY: all
all: $(addprefix $(BIN_DIR)/,$(TARGET).elf $(TARGET).eep $(TARGET).hex)

build: $(BIN_DIR)/$(TARGET).elf

$(BIN_DIR)/$(TARGET).elf: $(OBJ_LIST)
	avr-gcc $(CFLAGS) -dumpdir $(BUILD_DIR) -Wl,--print-memory-usage -Wl,-Map=build/$(TARGET).map $^ -o $@

$(BIN_DIR)/$(TARGET).hex: $(BIN_DIR)/$(TARGET).elf
	avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures -O ihex $< $@

$(BIN_DIR)/$(TARGET).eep: $(BIN_DIR)/$(TARGET).elf
	avr-objcopy -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0 --no-change-warnings -O ihex $< $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c Makefile
	avr-gcc $(CFLAGS) -MMD -c $< -o $@

ifeq (,$(filter clean,$(MAKECMDGOALS)))
-include $(DEP_LIST)
endif


.PHONY: program
program: program-flash

.PHONY: program-all
program-all: program-flash program-eeprom

.PHONY: program-flash
program-flash: $(BIN_DIR)/$(TARGET).hex
	avrdude -p $(MCU) -c $(PROGRAMMER) -P $(PORT) -b $(BAUD) $(if $(NO-CHIP-ERASE),-D) -U flash:w:$<:i

.PHONY: program-eeprom
program-eeprom: $(BIN_DIR)/$(TARGET).eep
	avrdude -p $(MCU) -c $(PROGRAMMER) -P $(PORT) -b $(BAUD) $(if $(NO-CHIP-ERASE),-D) -U eeprom:w:$<:i

.PHONY: terminal
terminal:
	avrdude -p $(MCU) -c $(PROGRAMMER) -P $(PORT) -b $(BAUD) -t


.PHONY: clean
clean:
	$(RM) $(call FixPath,$(BIN_DIR)/* $(OBJ_DIR)/* $(BUILD_DIR)/*)