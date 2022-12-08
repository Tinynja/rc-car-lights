BOARD ?= 
COM ?= 
PROGRAMMER ?= 
BAUD ?=

VPATH = build
CFLAGS = 

bin/blink.elf: blink.cpp
	avr-gcc -Wall -g -mmcu=attiny13 -O blink.cpp -dumpdir build/ -save-temps -Wl,--print-memory-usage -o bin/blink.elf

.PHONY: upload
upload:
	avrdude -p t13 -c ft232r_custom -P COM8 -b 76800 -U flash:w:bin/blink.elf:e

.PHONY: all
all: bin/blink.elf upload