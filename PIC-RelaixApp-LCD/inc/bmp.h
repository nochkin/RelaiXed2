/******************************************************************************
 * BMP to display on the DOGL128-6E LCD Display
 * (ie. splash screen)
 * Copyright 2012  Nicola Zandona
 *****************************************************************************/


#include "typedefs.h"
#include "macros.h"
#include "lcd.h"

#ifndef BMP_H
#define BMP_H
//#pragma romdata bmp



#define LCD_BMP_SPLASH_X_SIZE LCD_COL_MAX
#define LCD_BMP_SPLASH_Y_SIZE LCD_ROW_MAX
#define LCD_BMP_SPLASH_ARRAY_Y_SIZE_IN_BYTES 8
static far unsigned char rom Realixed2Splash_bmp[] = 
{
	/*
	height = 64
	width = 128
	array_x_size_in_byte = 128
	array_y_size_in_byte = 8
	*/

	/* Character Data - Index: 0 */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x07,0x00,0x00,
	0x00,0x00,0x00,0x00,0xF0,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0xFC,0x01,0x00,0x00,0x00,0x00,0x00,0x80,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0x0F,0x00,0x00,0x00,
	0x00,0x00,0x00,0xFC,0x03,0x00,0x00,0x00,0x00,0x00,0x80,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0xE0,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0xFC,0x7B,0x00,0x00,0x00,0x00,
	0x00,0x00,0x7F,0xF8,0x00,0x00,0x00,0x00,0x00,0xE0,0x1F,0xF8,0x01,0x00,0x00,0x00,0x00,0xF8,0x03,0x9C,0x03,0x00,0x00,0x00,0x00,0xF8,0x00,0x1C,0x07,0x00,0x00,0x00,
	0x00,0x1C,0x00,0x0C,0x0E,0x00,0x00,0x00,0x00,0x1C,0x00,0x0C,0x1C,0x00,0x00,0x00,0x00,0x0E,0x00,0x0E,0x38,0x00,0x00,0x00,0x00,0x06,0x00,0x0E,0x70,0x00,0x00,0x00,
	0x00,0x06,0x00,0x06,0xE0,0x00,0x00,0x00,0x00,0x06,0x00,0x06,0xC0,0x01,0x00,0x00,0x00,0x06,0x00,0x07,0x80,0x03,0x00,0x00,0x00,0x06,0x00,0x03,0x00,0x07,0x00,0x00,
	0x00,0x06,0x00,0x03,0x00,0x06,0x00,0x00,0x00,0x06,0x80,0x03,0x00,0x04,0x00,0x00,0x00,0x06,0x80,0x01,0x00,0x00,0x00,0x00,0x00,0x06,0xC0,0x01,0x00,0x00,0x00,0x00,
	0x00,0x06,0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x0E,0x3F,0x00,0xFC,0x01,0x00,0x00,0x00,0xFC,0x0F,0x00,0xFE,0x03,0x00,0x00,
	0x00,0xF8,0x03,0x00,0xFF,0x07,0x00,0x00,0x00,0xF0,0x00,0x00,0x63,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x63,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x63,0x06,0x00,0x00,
	0x00,0x00,0x00,0x00,0x63,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x63,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x3E,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x1E,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x07,0x00,0x00,
	0x00,0x00,0x00,0x80,0xFF,0x07,0x00,0x00,0x00,0x00,0x00,0xFF,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0xE3,0x07,0x00,0x00,
	0x00,0x00,0x00,0x00,0x63,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x63,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x63,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x63,0x06,0x00,0x00,
	0x00,0x00,0x00,0x00,0xE3,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x00,0x00,
	0x00,0x00,0x00,0x00,0xF0,0x07,0x00,0x00,0x00,0x00,0x00,0x60,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x1E,0x07,0x00,0x00,
	0x00,0x00,0x00,0x00,0xFC,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0xB8,0x03,0x00,0x00,
	0x00,0x00,0x00,0x00,0x1E,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x04,0x00,0x00,0x00,0x00,0x00,0x80,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x01,0x00,0x00,0x00,
	0x00,0x00,0x00,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1C,0xFC,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x07,0x00,0x00,
	0x00,0x00,0x00,0x00,0xFF,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x63,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x63,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x63,0x06,0x00,0x00,
	0x00,0x00,0x00,0x00,0x63,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x63,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x7C,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x3C,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF8,0x03,0x00,0x00,
	0x00,0x00,0x00,0x00,0xFC,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x06,0x00,0x00,
	0x00,0x00,0x00,0x00,0x0C,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x8C,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x07,0x00,0x00,0x00,0x00,0x00,0xF0,0x7F,0x00,0x00,0x00,
	0x00,0x00,0x00,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};


#define LCD_BMP_INPUT_STRING_X_SIZE 44
#define LCD_BMP_INPUT_STRING_Y_SIZE 19
#define LCD_BMP_INPUT_STRING_ARRAY_Y_SIZE_IN_BYTES 3
static far unsigned char rom input_string[] = 
{
	/* 'I' */ /* 4 cols */
	0xFF,0x7F,0x00,0xFF,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	/* 'n' */ /* 11 cols */
	0xE0,0x7F,0x00,0xE0,0x7F,0x00,0x30,0x00,0x00,0x30,0x00,0x00,0x30,0x00,0x00,0x30,0x00,0x00,0x70,0x00,0x00,0xE0,0x7F,0x00,0xC0,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	/* 'p' */ /* 11 cols */
	0xE0,0xFF,0x07,0xF0,0xFF,0x07,0x30,0x60,0x00,0x30,0x60,0x00,0x30,0x60,0x00,0x30,0x60,0x00,0x70,0x70,0x00,0xE0,0x3F,0x00,0xC0,0x1F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	/* 'u' */ /* 11 cols */
	0xF0,0x1F,0x00,0xF0,0x3F,0x00,0x00,0x70,0x00,0x00,0x60,0x00,0x00,0x60,0x00,0x00,0x60,0x00,0x00,0x60,0x00,0xF0,0x3F,0x00,0xF0,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	/* 't' */ /* 7 cols */
	0x30,0x00,0x00,0x30,0x00,0x00,0xFE,0x3F,0x00,0xFE,0x7F,0x00,0x30,0x60,0x00,0x30,0x60,0x00,0x30,0x60,0x00
	//,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};


#define LCD_BMP_DB_X_SIZE 26
#define LCD_BMP_DB_Y_SIZE 20
#define LCD_BMP_DB_ARRAY_Y_SIZE_IN_BYTES 3
static far unsigned char rom dB[] = 
{

	/* Character Data - Index: 0 */
	0x00,0xFF,0x01,0x80,0xFF,0x03,0xC0,0x01,0x07,0xE0,0x00,0x0E,0x60,0x00,0x0C,0x60,0x00,0x0C,0x60,0x00,0x0C,0x60,0x00,0x0C,0x60,0x00,0x0C,0xFF,
	0xFF,0x0F,0xFF,0xFF,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x0F,0xFF,0xFF,0x0F,0x03,0x06,0x0C,0x03,0x06,0x0C,0x03,0x06,0x0C,0x03,0x06,
	0x0C,0x03,0x06,0x0C,0x03,0x06,0x0C,0x03,0x06,0x0C,0x06,0x0F,0x06,0x06,0x0F,0x07,0xFC,0xF9,0x03,0xF8,0xF0,0x01
};

static far unsigned char rom _emptycol[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };



#define LCD_BMP_MUTE_X_SIZE 26
#define LCD_BMP_MUTE_Y_SIZE 20
#define LCD_BMP_MUTE_ARRAY_Y_SIZE_IN_BYTES 3
static far unsigned char rom Mute[] = 
{
	/* Character Data - Index: 0 */
	0x80,0xFF,0x01,0x00,0x01,0x00,0x00,0x02,0x00,0x00,0x04,0x00,0x00,0x04,0x00,0x00,0x02,0x00,0x00,0x01,0x00,0x80,0xFF,0x01,0x00,0x00,0x00,0x00,
	0xFC,0x00,0x00,0x00,0x01,0x00,0x00,0x01,0x00,0x00,0x01,0x00,0xFC,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0x00,0x04,0x00,0x00,0xFC,0x01,0x00,0x04,
	0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0xFC,0x01,0x00,0x24,0x01,0x00,0x24,0x01,0x00,0x24,0x01,0x00,0x04,0x01
};

#define LCD_BMP_CH_VERT_X_SIZE 25
#define LCD_BMP_CH_VERT_Y_SIZE 49
#define LCD_BMP_CH_VERT_ARRAY_Y_SIZE_IN_BYTES 7
static far unsigned char rom CH_vert[] = 
{
	/* Character Data - Index: 0 */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
	0x39,0x39,0x39,0x39,0x39,0x39,0x01,0x45,0x45,0x45,0x45,0x45,0x45,0x01,0x45,0x45,0x45,0x45,0x45,0x45,0x01,0x45,0x45,0x45,0x45,0x45,0x45,0x01,
	0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x7D,0x7D,0x7D,0x7D,0x7D,0x7D,0x01,0x11,0x11,0x11,0x11,0x11,0x11,0x01,0x11,0x11,0x11,0x11,0x11,0x11,0x01,
	0x7D,0x7D,0x7D,0x7D,0x7D,0x7D,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x49,0x45,0x1D,0x5D,0x39,0x01,0x49,0x65,0x45,0x11,0x55,0x55,0x01,
	0x7D,0x55,0x55,0x11,0x55,0x55,0x01,0x41,0x49,0x6D,0x7D,0x65,0x21,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x01
};




#define LCD_BMP_BALANCE_X_SIZE		47
#define LCD_BMP_BALANCE_Y_SIZE		15
#define LCD_BMP_BALANCE_ARRAY_Y_SIZE_IN_BYTES 2
static far unsigned char rom BalanceBmp[] = 
{

	/* Character Data - Index: 0 */
	0x00,0x00,0x00,0x08,0x00,0x08,0x00,0x00,0x00,0x26,0x00,0x49,0x70,0x49,0x10,0x49,0x10,0x3E,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,
	0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x3E,0x10,0x51,0x70,0x49,0x10,0x45,
	0x10,0x3E,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,
	0x10,0x00,0x10,0x26,0x10,0x49,0x70,0x49,0x00,0x49,0x00,0x3E,0x00,0x00,0x00,0x00
};

#define LCD_BMP_BALANCE_IDX_X_SIZE		1
#define LCD_BMP_BALANCE_IDX_Y_SIZE		2
#define LCD_BMP_BALANCE_IDX_ARRAY_Y_SIZE_IN_BYTES 1

static far unsigned char rom BalanceIndex[] = 
{
	0xFF
};


//#pragma romdata 
#endif

