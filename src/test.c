#define F_CPU 16000000UL

#include <avr/io.h>
// #include "config.h"

// #define CYCLES_PER_MS F_CPU/1000/256

// volatile uint8_t *myPINB = &PINB;

#define ddrx portx-1
#define pinx portx-2

struct {
    uint8_t portx: 6;
    uint8_t b: 2;
} myStruct = {0x38, 0};

int main(void) {
    DDRB = _BV(PB0);
    while (1) {
        _MMIO_BYTE(myStruct.pinx) = _BV(PB0);
    }
}