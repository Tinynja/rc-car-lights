#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

uint32_t clk=0;
uint8_t timer=0;
uint8_t value_raw=0;

struct {
    uint8_t value: 7;
    uint8_t state: 1;
} myStruct;

int main(void) {
    uart_init(BAUD_CALC(250000));

    TCCR0A = _BV(COM0A1) | _BV(WGM01) | _BV(WGM00);
    TCCR0B = _BV(CS00);
    TIMSK0 = _BV(TOIE0);
    
    DDRB = _BV(PB0);
	GIMSK = _BV(PCIE);
    PCMSK = _BV(PCINT2);

    sei();
    while (1) {
		if (value_raw < 80) {
            ATOMIC_BLOCK(ATOMIC_FORCEON)
            {
                myStruct.value = 0;
            }
		} else {
            ATOMIC_BLOCK(ATOMIC_FORCEON)
            {
                myStruct.value = 127;
            }
		}
        OCR0A = myStruct.value;
    }
}

ISR(TIM0_OVF_vect) {
    clk++;
}

ISR(PCINT0_vect) {
    uint8_t new_state = bit_is_set(PINB, PB2) >> PB2;
    if (!myStruct.state && new_state) {
        timer = clk;
    } else if (myStruct.state && !new_state) {
        value_raw = clk-timer;
    }
    myStruct.state = new_state;
}