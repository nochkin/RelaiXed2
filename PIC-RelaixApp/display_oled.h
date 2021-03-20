/* 
 * File:   display_oled.h
 * Author: jos
 *
 * Created on August 14, 2014, 11:01 PM
 */

#ifndef DISPLAY_OLED_H
#define	DISPLAY_OLED_H

#include <stdint.h>

extern uint8_t has_oled_display;
extern uint8_t display_oled_init(void);

extern void display_oled_power(uint8_t power);
extern void display_oled_channel(uint8_t channel);
extern void display_oled_volume(uint8_t upperdigit, uint8_t lowdigit);
extern void display_oled_mute(void);
extern void display_oled_unmute(void);
extern void display_oled_balance(int8_t balance);

#endif	/* DISPLAY_OLED_H */
