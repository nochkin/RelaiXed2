/******************************************************************************
 * Module to control the 2-digit 7-segment display
 *
 * Copyright 2010  Jos van Eijndhoven
 *****************************************************************************/
#include "typedefs.h"

extern volatile byte display_cnt; // increments on every display refresh (is at 160Hz)

#define DIGIT_A     10
#define DIGIT_B     11
#define DIGIT_C     12
#define DIGIT_D     13
#define DIGIT_E     14
#define DIGIT_F     15
#define DIGIT_minus 16
#define DIGIT_P     17
#define DIGIT_dark  18
#define DIGIT_U     19
#define DIGIT_r     20
#define DIGIT_c     21

extern void display_isr(void);
extern void display_set(byte digit_hi, byte digit_lo);
extern void display_set_alt( byte digit_hi, byte digit_lo,
							byte duration);

