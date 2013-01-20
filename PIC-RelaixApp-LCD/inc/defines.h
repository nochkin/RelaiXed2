/******************************************************************************
 * Defines to control Relaixed2 behavior
 * Copyright 2012  Nicola Zandona
 *****************************************************************************/


#include "typedefs.h"
#include "macros.h"


#ifndef DEFINES_H
#define DEFINES_H

// when defined, Relaixed removes the leading zero when displaying 2-digits values on display (LCD only)
#define VOLUME_REMOVE_LEADING_ZERO

// when defined, Relaixed shows volume as dB attenuation (from 0dB to -inf)
#define VOLUME_DB_LCD_DISPLAY

// when define, backligh is PWM controlled
#define LCD_BACKLIGHT_DIMMER

#endif