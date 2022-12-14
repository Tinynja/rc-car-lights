TARGET=RC_Car_Lights_$(MCU)

-include Makefile.presets

#MCU and programmer
MCU?=atmega328p
PROGRAMMER?=arduino
BAUD?=
PORT?=

#Folder structure
BIN_DIR=bin
LIB_DIR=lib
SRC_DIR?=src
OBJ_DIR=obj
BUILD_DIR=build


#Libraries
LIBS?=$(filter-out %.a,$(wildcard $(LIB_DIR)/*))
LIBS_A=$(LIBS:$(LIB_DIR)/%=$(LIB_DIR)/lib%.a)
INCLUDES=$(addprefix -I,$(LIBS))

CFLAGS=-mmcu=$(MCU) $(INCLUDES) -Os -flto -funsigned-char -funsigned-bitfields -ffunction-sections -fdata-sections -Wall -Wno-int-conversion -Wno-int-to-pointer-cast -Wno-comment

AVRDUDEFLAGS=-p $(MCU) -c $(PROGRAMMER) -P $(PORT) $(if $(BAUD),-b $(BAUD)) $(if $(NO-CHIP-ERASE),-D) 

ARFLAGS=--plugin "C:\Users\Amine\AppData\Local\Programs\avr-gcc\libexec\gcc\avr\12.1.0\liblto_plugin.dll"

SRC_LIST=$(wildcard $(SRC_DIR)/*.c)
OBJ_LIST=$(SRC_LIST:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEP_LIST=$(SRC_LIST:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.d)

VARDEP=$(BUILD_DIR)/$(1).$($(1)).VARDEP

ifeq ($(OS),Windows_NT)
	RM=del /s /q /f
	RMDIR=rmdir /s /q
	MKDIR=mkdir
	TOUCH=type nul >
	FixPath=$(subst /,\,$1)
else
	RM=rm -rf
	RMDIR=$(RM)
	MKDIR=mkdir -p
	TOUCH=touch
	FixPath=$1
endif

.PHONY: all
all: $(addprefix $(BIN_DIR)/,$(TARGET).elf $(TARGET).eep $(TARGET).hex EEPROM_initializer_$(MCU).hex)

#Target build rules
build: $(BIN_DIR)/$(TARGET).elf

$(BIN_DIR)/$(TARGET).elf: $(OBJ_LIST) $(LIBS_A) $(call VARDEP,MCU) | $(BIN_DIR)/ $(BUILD_DIR)/
	avr-gcc $(CFLAGS) -dumpdir $(BUILD_DIR) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map -Wl,--cref -Wl,--print-memory-usage $(filter %.o,$^) $(filter %.a,$^) -o $@

$(BIN_DIR)/$(TARGET).hex: $(BIN_DIR)/$(TARGET).elf
	avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures -O ihex $< $@

$(BIN_DIR)/$(TARGET).eep: $(BIN_DIR)/$(TARGET).elf
	avr-objcopy -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0 --no-change-warnings -O ihex $< $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c Makefile $(call VARDEP,MCU) | $(OBJ_DIR)/
	avr-gcc $(CFLAGS) -MMD -c $< -o $@
	@avr-gcc $(CFLAGS) -S $< -o $(@:%.o=%.s)

%/:
	@$(MKDIR) $(call FixPath,$@)

#EEPROM_initializer rules
.PHONY: eeprom-initializer
eeprom-initializer: $(BIN_DIR)/EEPROM_initializer_$(MCU).hex

$(BIN_DIR)/EEPROM_initializer_$(MCU).hex: $(BUILD_DIR)/EEPROM_initializer_$(MCU).elf | $(BIN_DIR)/
	avr-objcopy $< $@

$(BUILD_DIR)/EEPROM_initializer_$(MCU).elf: $(OBJ_DIR)/EEPROM_initializer.o $(OBJ_DIR)/eeprom.o $(LIBS_A) | $(BUILD_DIR)/
	avr-gcc $(CFLAGS) -dumpdir $(BUILD_DIR) -Wl,-Map=$(BUILD_DIR)/EEPROM_initializer.map -Wl,--cref -Wl,--print-memory-usage $^ -o $@

$(OBJ_DIR)/EEPROM_initializer.o: $(SRC_DIR)/EEPROM_initializer/EEPROM_initializer.c Makefile | $(OBJ_DIR)/
	avr-gcc $(CFLAGS) -Wno-unused-variable -MMD -c $< -o $@

$(OBJ_DIR)/eeprom.o: $(OBJ_DIR)/eeprom.bin
	avr-ld -r -b binary $< -o $@

$(OBJ_DIR)/eeprom.bin: $(BIN_DIR)/$(TARGET).eep Makefile | $(OBJ_DIR)/
	avr-objcopy -I ihex -O binary $< $@


#Static library build rules
$(LIB_DIR)/lib%.a: $(OBJ_DIR)/%/ $(LIB_DIR)/%/*.c $(LIB_DIR)/%/*.h Makefile 
	cd $< && avr-gcc $(CFLAGS) -c $(addprefix ../../,$(filter %.c,$^))
	avr-ar $(ARFLAGS) rcs $@ $(patsubst $(LIB_DIR)/%.c,$(OBJ_DIR)/%.o,$(filter %.c,$^))

#Variable dependencies
$(BUILD_DIR)/%.VARDEP: | $(BUILD_DIR)/
	$(RM) $(call FixPath,$(BUILD_DIR)/*.VARDEP) 2> nul
	$(TOUCH) $(call FixPath,$@)


.PHONY: program
program: program-flash

.PHONY: program-all
program-all: program-flash program-eeprom

.PHONY: program-flash
program-flash: $(BIN_DIR)/$(TARGET).hex
	avrdude $(AVRDUDEFLAGS) -U flash:w:$<:i

.PHONY: program-eeprom
program-eeprom: $(BIN_DIR)/$(TARGET).eep
	avrdude $(AVRDUDEFLAGS) -U eeprom:w:$<:i

.PHONY: program-eeprom-initializer
program-eeprom-initializer: $(BIN_DIR)/EEPROM_initializer_$(MCU).hex
	avrdude $(AVRDUDEFLAGS) -U flash:w:$<:e

.PHONY: terminal
terminal:
	avrdude $(AVRDUDEFLAGS) -t

	
ifeq (,$(filter clean,$(MAKECMDGOALS)))
-include $(DEP_LIST)
endif

.PHONY: clean
clean:
	-@$(RMDIR) $(call FixPath,$(BIN_DIR) $(OBJ_DIR) $(BUILD_DIR)) 2> nul
	-@$(RM) $(call FixPath,$(LIB_DIR)/*.a) 2> nul