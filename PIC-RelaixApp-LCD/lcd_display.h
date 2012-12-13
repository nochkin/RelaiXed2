/******************************************************************************
 * Module to control an "EA DOGM132x-5" graphic LCD display
 *
 * Copyright 2012  Jos van Eijndhoven
 *****************************************************************************/
#include "typedefs.h"

extern byte has_lcd_display;
extern void lcd_display_init(void);
extern void lcd_display_mute(void);
extern void lcd_display_nolock(void);
extern void lcd_display_volume(byte msdigit, byte lsdigit);
