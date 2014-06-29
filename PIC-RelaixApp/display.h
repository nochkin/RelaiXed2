/******************************************************************************
 * Module to control the 2-digit 7-segment display
 *
 * Copyright 2010  Jos van Eijndhoven
 *****************************************************************************/
#include <stdint.h>

extern volatile uint8_t display_cnt; // increments on every display refresh (is at 160Hz)

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
#define DIGIT_n		22
#define DIGIT_o		23

extern void display_isr(void);
extern void display_set(uint8_t digit_hi, uint8_t digit_lo, uint8_t override);
extern void display_set_alt( uint8_t digit_hi, uint8_t digit_lo, uint8_t duration);

