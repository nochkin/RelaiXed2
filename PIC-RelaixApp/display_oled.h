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
//extern void display_oled_string(uint8_t row, uint8_t col, const char *string);
extern void display_oled_sleep(void);
extern void display_oled_chars(uint8_t row, uint8_t col, uint8_t len, const uint8_t chars[]);
extern void display_oled_bigchar(uint8_t col, uint8_t charnum);

extern void display_oled_channel(uint8_t channel);
extern void display_oled_volume(uint8_t upperdigit, uint8_t lowdigit);
extern void display_oled_mute(void);
extern void display_oled_unmute(void);

#endif	/* DISPLAY_OLED_H */

