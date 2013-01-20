/******************************************************************************
 * Module to control the DOGL128-6E LCD Display
 *
 * Copyright 2012  Nicola Zandona
 *****************************************************************************/
#include "typedefs.h"
#include "macros.h"


#ifndef LCD_H
#define LCD_H


// LCD limits
#define LCD_PAGE_MAX	8
#define LCD_ROW_MAX		64
#define LCD_COL_MAX		128



// global vars
extern volatile byte has_lcd_display;
extern byte lcd_wr_ack;
#define LOCK_SHADOW_MEMORY lcd_wr_ack=0
#define RELEASE_SHADOW_MEMORY lcd_wr_ack=1
#define SHADOW_MEMORY_IS_LOCKED (lcd_wr_ack==0)


// exported functions
extern void detect_display(void);
extern void config_lcd(void);
extern void clear_lcd(void);
extern void power_lcd_display(byte power);
extern void power_lcd_backlight(byte power, byte brightness);
extern void isr_lcd_refresh(void);
extern void lcd_display_volume(byte msdigit, byte lsdigit);
extern void lcd_display_balance(char master_balance);
extern void lcd_display_info(const rom char* msg);
extern void lcd_display_channel(byte channel);
extern void lcd_display_bmp (byte x_pos, byte y_pos, byte x_size, byte y_size, far rom unsigned char* data, byte array_y_size);
extern void lcd_display_mute(byte muted);


// LCD panel constants & macros
#define LCD_BACKLIGHT_OFF 		0
#define LCD_BACKLIGHT_FULL  	1
#define LCD_BACKLIGHT_DIMMED  	2

#define LCD_POWER_OFF 0
#define LCD_POWER_ON 1

#define LCD_PAGE_0 0
#define LCD_PAGE_1 1
#define LCD_PAGE_2 2
#define LCD_PAGE_3 3
#define LCD_PAGE_4 4	
#define LCD_PAGE_5 5
#define LCD_PAGE_6 6
#define LCD_PAGE_7 7

#define LCD_ADC_NORMALE 0
#define LCD_ADC_REVERSE 1

#define LCD_ORIENTATION_NORMAL 0
#define LCD_ORIENTATION_REVERSE 1

#define LCD_ALL_POINTS_OFF 0
#define LCD_ALL_POINTS_ON 1

#define LCD_BIAS_1_9 0
#define LCD_BIAS_1_7 1

#define LCD_COM_NORMAL_DIR 0
#define LCD_COM_REVERSE_DIR 1

#define LCD_PWR_BOOSTER_ON 0x01
#define LCD_PWR_BOOSTER_OFF 0x00
#define LCD_PWR_REGULATOR_ON 0x02
#define LCD_PWR_REGULATOR_OFF 0x00
#define LCD_PWR_FOLLOWER_ON 0x04
#define LCD_PWR_FOLLOWER_OFF 0x00


#define LCD_BOOSTER_2X_3X_4X 0x00
#define LCD_BOOSTER_5X 0x01
#define LCD_BOOSTER_6X 0x03

#define LCD_STATIC_INDICATOR_ON 0x01
#define LCD_STATIC_INDICATOR_OFF  0x00


// lcd panel commands
#define LCD_POWER_CTRL(x) B8(10101110)| ((x)&0x01) 	//1
#define LCD_START_LINE_ADDR(x) B8(01000000)|((x)&0x3F) //2
#define LCD_SET_PAGE_ADDR(page) B8(10110000)| ((page)&0x0F) 	//3
#define LCD_SET_COL_MSB(x) B8(00010000) | ((x)&0x0F)  //4A
#define LCD_SET_COL_LSB(x) B8(00000000) | ((x)&0x0F)  //4B

#define LCD_ADC_SELECT(x) B8(10100000)|((x)&0x01)			//8
#define LCD_ORIENTATION(x) B8(101000110)|((x)&0x01) 	//9
#define LCD_ALLPOINTS(x) B8(10100100)| ((x)&0x01) 		//10
#define LCD_SET_BIAS(x) B8(10100010)| ((x)&0x01)				//11
#define LCD_RESET B8(11100010)					//14
#define LCD_COM_SCAN_DIR(x)	B8(11000000)|(((x)&0x01)<<3)		//15
#define LCD_PWR_CTRL(x) B8(00101000)| ((x)&0x07)			//16
#define LCD_V0_VOLT_REGULATOR(x) B8(00100000)|  ((x)&0x07)	//17
#define LCD_ELECTR_VOL_MODE_SET B8(10000001)				//18A
#define LCD_ELECTR_VOL_SET_VALUE(x)  B8(00000000) |  ((x)&0x3F)	//18B

#define LCD_STATIC_INDICATOR_MODE B8(10101100)	//19A
#define LCD_STATIC_INDICATOR_SET(x)  B8(00000000) |((x)&0x01)	//19B

#define LCD_BOOSTER_RATIO_MODE B8(11111000)		//20A
#define LCD_BOOSTER_RATIO_SET_VALUE(x) B8(00000000)| ((x)&0x03)		//20B
#define LCD_NOP B8(11100011)					//22



// LCD layout

// volume stuff
#define LCD_VOLUME_MINUS_X_POS 0
#define LCD_VOLUME_MINUS_Y_POS 0
#define LCD_VOLUME_MINUS_X_SIZE LCD_FONT_MINUS32_X_SIZE
#define LCD_VOLUME_MINUS_Y_SIZE LCD_FONT_MINUS32_Y_SIZE
#define LCD_VOLUME_MINUS_ARRAY_Y_SIZE_IN_BYTES LCD_FONT_MINUS32_ARRAY_Y_SIZE_IN_BYTES

#define LCD_VOLUME_DB_SYMBOL_X_POS 75
#define LCD_VOLUME_DB_SYMBOL_Y_POS 20
#define LCD_VOLUME_DB_SYMBOL_X_SIZE LCD_BMP_DB_X_SIZE
#define LCD_VOLUME_DB_SYMBOL_Y_SIZE LCD_BMP_DB_Y_SIZE
#define LCD_VOLUME_DB_SYMBOL_ARRAY_Y_SIZE_IN_BYTES LCD_BMP_DB_ARRAY_Y_SIZE_IN_BYTES

#define LCD_VOLUME_1ST_DIGIT_X_POS 15
#define LCD_VOLUME_1ST_DIGIT_Y_POS 0
#define LCD_VOLUME_1ST_DIGIT_X_SIZE LCD_FONT_NEOSANSINTEL32_X_SIZE
#define LCD_VOLUME_1ST_DIGIT_Y_SIZE LCD_FONT_NEOSANSINTEL32_Y_SIZE
#define LCD_VOLUME_1ST_DIGIT_ARRAY_Y_SIZE_IN_BYTES LCD_FONT_NEOSANSINTEL32_ARRAY_Y_SIZE_IN_BYTES

#define LCD_VOLUME_2ND_DIGIT_X_POS 45
#define LCD_VOLUME_2ND_DIGIT_Y_POS 0
#define LCD_VOLUME_2ND_DIGIT_X_SIZE LCD_FONT_NEOSANSINTEL32_X_SIZE
#define LCD_VOLUME_2ND_DIGIT_Y_SIZE LCD_FONT_NEOSANSINTEL32_Y_SIZE
#define LCD_VOLUME_2ND_DIGIT_ARRAY_Y_SIZE_IN_BYTES LCD_FONT_NEOSANSINTEL32_ARRAY_Y_SIZE_IN_BYTES

#define LCD_VOLUME_MUTE_SYMBOL_X_POS 75
#define LCD_VOLUME_MUTE_SYMBOL_Y_POS 0
#define LCD_VOLUME_MUTE_SYMBOL_X_SIZE LCD_BMP_MUTE_X_SIZE
#define LCD_VOLUME_MUTE_SYMBOL_Y_SIZE LCD_BMP_MUTE_Y_SIZE
#define LCD_VOLUME_MUTE_SYMBOL_ARRAY_Y_SIZE_IN_BYTES LCD_BMP_MUTE_ARRAY_Y_SIZE_IN_BYTES

// channel stuff
#define LCD_CHANNEL_INPUT_STRING_X_POS 0
#define LCD_CHANNEL_INPUT_STRING_Y_POS 45
#define LCD_CHANNEL_INPUT_STRING_X_SIZE LCD_BMP_INPUT_STRING_X_SIZE
#define LCD_CHANNEL_INPUT_STRING_Y_SIZE LCD_BMP_INPUT_STRING_Y_SIZE
#define LCD_CHANNEL_INPUT_STRING_ARRAY_Y_SIZE_IN_BYTES LCD_BMP_INPUT_STRING_ARRAY_Y_SIZE_IN_BYTES

#define LCD_CHANNEL_ID_X_POS 46
#define LCD_CHANNEL_ID_Y_POS 43
#define LCD_CHANNEL_ID_X_SIZE LCD_FONT_CH_ID_X_SIZE
#define LCD_CHANNEL_ID_Y_SIZE LCD_FONT_CH_ID_Y_SIZE
#define LCD_CHANNEL_ID_ARRAY_Y_SIZE_IN_BYTES LCD_FONT_CH_ID_ARRAY_Y_SIZE_IN_BYTES 

#define LCD_CHANNEL_BMP_X_POS 103
#define LCD_CHANNEL_BMP_Y_POS 0
#define LCD_CHANNEL_BMP_X_SIZE LCD_BMP_CH_VERT_X_SIZE
#define LCD_CHANNEL_BMP_Y_SIZE LCD_BMP_CH_VERT_Y_SIZE
#define LCD_CHANNEL_BMP_ARRAY_Y_SIZE_IN_BYTES LCD_BMP_CH_VERT_ARRAY_Y_SIZE_IN_BYTES 

// balance stuff
#define LCD_BALANCE_BMP_X_POS 81
#define LCD_BALANCE_BMP_Y_POS 49
#define LCD_BALANCE_BMP_X_SIZE LCD_BMP_BALANCE_X_SIZE
#define LCD_BALANCE_BMP_Y_SIZE LCD_BMP_BALANCE_Y_SIZE
#define LCD_BALANCE_BMP_ARRAY_Y_SIZE_IN_BYTES LCD_BMP_BALANCE_ARRAY_Y_SIZE_IN_BYTES

#define LCD_BALANCE_IDX_BMP_X_SIZE LCD_BMP_BALANCE_IDX_X_SIZE
#define LCD_BALANCE_IDX_BMP_Y_SIZE LCD_BMP_BALANCE_IDX_Y_SIZE
#define LCD_BALANCE_IDX_BMP_ARRAY_Y_SIZE_IN_BYTES LCD_BMP_BALANCE_IDX_ARRAY_Y_SIZE_IN_BYTES
#define LCD_BALANCE_IDX_BMP_CENTER_X_POS 	(LCD_BALANCE_BMP_X_POS + (LCD_BALANCE_BMP_X_SIZE/2) + 1)
#define LCD_BALANCE_IDX_BMP_CENTER_Y_POS 	LCD_BALANCE_BMP_Y_POS + 2



#endif //LCD_H