#ifndef REGISTERS_H
#define REGISTERS_H

#include "config.h"
#include <avr/interrupt.h>

// Register utils
#define _EXPAND(pre, var, post) (pre ## var ## post) // needed because '##' inhibits macro expansion

// Constants
#define CYCLES_PER_MS	((F_CPU/100/256+5)/10)
#define HYSTERESIS		1

// RxChConfig macros
#define ddrx portx-1
#define pinx portx-2

// I/O register macros
#define GPORT(p)	_GPORT(p)
#define _GPORT(p)	_EXPAND(PORT,p,)
#define GDDR(p)		_GDDR(p)
#define _GDDR(p)	_EXPAND(DDR,p,)
#define GPIN(p)		_GPIN(p)
#define _GPIN(p)	_EXPAND(PIN,p,)

#define LEFT_RIGHT_MASK		(_BV(LIGHTS_LEFT) | _BV(LIGHTS_RIGHT))
#define FRONT_REAR_MASK		(_BV(LIGHTS_FRONT) | _BV(LIGHTS_REAR))
#define ALL_MASK			(LEFT_RIGHT_MASK | FRONT_REAR_MASK)

// Timer register
#define TCCRnA		_TCCRnA(TIMER_NUMBER)
#define _TCCRnA(n)	_EXPAND(TCCR,n,A)
#define COMnA0		_COMnA0(TIMER_NUMBER)
#define _COMnA0(n)	_EXPAND(COM,n,A0)
#define COMnA1		_COMnA1(TIMER_NUMBER)
#define _COMnA1(n)	_EXPAND(COM,n,A1)
#define COMnB0		_COMnB0(TIMER_NUMBER)
#define _COMnB0(n)	_EXPAND(COM,n,B0)
#define COMnB1		_COMnB1(TIMER_NUMBER)
#define _COMnB1(n)	_EXPAND(COM,n,B1)
#define WGMn0		_WGMn0(TIMER_NUMBER)
#define _WGMn0(n)	_EXPAND(WGM,n,0)
#define WGMn1		_WGMn1(TIMER_NUMBER)
#define _WGMn1(n)	_EXPAND(WGM,n,1)

#define TCCRnB		_TCCRnB(TIMER_NUMBER)
#define _TCCRnB(n)	_EXPAND(TCCR,n,B)
#define WGMn2		_WGMn2(TIMER_NUMBER)
#define _WGMn2(n)	_EXPAND(WGM,n,2)
#define CSn2		_CSn2(TIMER_NUMBER)
#define _CSn2(n)	_EXPAND(CS,n,2)
#define CSn1		_CSn1(TIMER_NUMBER)
#define _CSn1(n)	_EXPAND(CS,n,1)
#define CSn0		_CSn0(TIMER_NUMBER)
#define _CSn0(n)	_EXPAND(CS,n,0)

#if defined(TIMSK)
	#define TIMSKn		TIMSK
#else
	#define TIMSKn		_TIMSKn(TIMER_NUMBER)
	#define _TIMSKn(n)	_EXPAND(TIMSK,n,)
#endif

#define TOIEn		_TOIEn(TIMER_NUMBER)
#define _TOIEn(n)	_EXPAND(TOIE,n,)

#define OCRnA		_OCRnA(TIMER_NUMBER)
#define _OCRnA(n)	_EXPAND(OCR,n,A)
#define OCRnB		_OCRnB(TIMER_NUMBER)
#define _OCRnB(n)	_EXPAND(OCR,n,B)

// Interrupts
#if defined(TIM0_OVF_vect)
	#define TIMERn_OVF_vect		_TIMERn_OVF_vect(TIMER_NUMBER)
	#define _TIMERn_OVF_vect(n) _EXPAND(TIM,n,_OVF_vect)
#elif defined(TIMER0_OVF_vect)
	#define TIMERn_OVF_vect		_TIMERn_OVF_vect(TIMER_NUMBER)
	#define _TIMERn_OVF_vect(n) _EXPAND(TIMER,n,_OVF_vect)
#else
	#error Could not determine the correct timer overflow vector to use.
#endif

#if defined(GIMSK)
	#define PCICR_REG	GIMSK
	#define PCIE_BIT	PCIE
#elif defined(PCICR)
	#define PCICR_REG	PCICR
#endif

#ifdef PCMSK
	#define PCMSK_REG	PCMSK
#endif

#if PCINT_CH1 >= 0 & PCINT_CH2 >= 0 & PCINT_CH1 < 8 & PCINT_CH2 < 8
	#ifdef PCICR
	#define PCIE_BIT	PCIE0
	#endif

	#ifdef PCMSK0
	#define PCMSK_REG	PCMSK0
	#endif

	#define PCINT_CH1_BIT	PCINT_CH1-0
	#define PCINT_CH2_BIT	PCINT_CH2-0
	
	#define PCINT_vect	PCINT0_vect
#elif PCINT_CH1 >= 8 & PCINT_CH2 >= 8 && PCINT_CH1 < 16 & PCINT_CH2 < 16
	#ifdef PCICR
	#define PCIE_BIT	PCIE1
	#endif
	
	#ifdef PCMSK1
	#define PCMSK_REG	PCMSK1
	#endif

	#define PCINT_CH1_BIT	PCINT_CH1-8
	#define PCINT_CH2_BIT	PCINT_CH2-8

	#define PCINT_vect	PCINT1_vect
#elif PCINT_CH1 >= 16 & PCINT_CH2 >= 16 && PCINT_CH1 < 24 & PCINT_CH2 < 24
	#ifdef PCICR
	#define PCIE_BIT	PCIE2
	#endif
	
	#ifdef PCMSK2
	#define PCMSK_REG	PCMSK2
	#endif

	#define PCINT_CH1_BIT	PCINT_CH1-16
	#define PCINT_CH2_BIT	PCINT_CH2-16

	#define PCINT_vect	PCINT2_vect
#else
	#error PCINT_CH1 and PCINT_CH2 in `config.h` must be in the same PCMSK register
#endif

#endif //REGISTERS_H