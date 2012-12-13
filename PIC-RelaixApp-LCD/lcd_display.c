/******************************************************************************
 * Module to control an "EA DOGM132x-5" graphic LCD display
 *
 * Copyright 2012  Jos van Eijndhoven
 *****************************************************************************/
#include "io_cfg.h"
#include "lcd_display.h"

/* forward declarations */
#define lcd_send_cmd(cmd)	{LCD_A0 = 0; lcd_send_byte(cmd);}
#define lcd_send_data(data)	{LCD_A0 = 1; lcd_send_byte(data);}
static void lcd_send_byte( byte data);
static void lcd_set_addr(byte row_addr, byte col_addr);
#define LCD_reset		0xe2
#define LCD_displayOn	0xaf
#define LCD_displayOff	0xae
#define LCD_displayNormal	0xa6
#define LCD_pageAddr	0xb0
#define LCD_colAddrLo	0x00
#define LCD_colAddrHi	0x10
#define LCD_startline0	0x40
#define LCD_adcNormal	0xa0
#define LCD_commonOut	0xc0
#define LCD_bias		0xa2
#define LCD_powerCntl	0x2F
#define LCD_booster		0xf8
#define LCD_voltageReg	0x23
#define LCD_contrast	0x81
#define LCD_indicator	0xac

/* By default there is no LCD display, only a two-digit 7-segment LED display */
byte has_lcd_display = 0;

/* Check for presence of LCD.
 * The LCD is connected into the left 7-segment socket
 * Note that at this 'init' time, the timer for display L/R multiplex is not running.
 * Also, assume that Port B and C, which normally drive the LEDs in open-collector mode,
 * were preset in the 'input' state.
 */
void lcd_display_init(void)
{
	int i;
	LEDright = 0; // power the left LCD, it connects to the left 7-seg socket.
	if (LCD_Sense)
	{ // normal LED: read-back from LED shows high
		has_lcd_display = 0;
	} else
	{ // read a pull-down value of at least one of both lines,
      // made by a short in the LCD connection.
	  // Which one of the lines, or both, grounded indicates the type of LCD panel.
		has_lcd_display = 1;
		LEDright = 0; // power the left LCD, it connects to the left 7-seg socket.
		mLED_1_On(); // turn into output
		mLED_2_On(); // turn into output
		mLED_3_On(); // turn into output
		mLED_4_On(); // turn into output
		mLED_5_On(); // turn into output
		mLED_6_On(); // turn into output
		LCD_RSTb = 0; // low active
		LCD_Vdd  = 1;
		LCD_CSb  = 1;
		LCD_A0   = 0;
		LCD_SCL  = 1;
		LCD_SI   = 1;

		for (i=0; i<10000; i++)
			;
		LCD_RSTb = 1; // take out of reset after a little pause

		for (i=0; i<100; i++)
			;
		LCD_CSb = 0;
		lcd_send_cmd( LCD_startline0);
		lcd_send_cmd( LCD_adcNormal);
		lcd_send_cmd( LCD_commonOut);
		lcd_send_cmd( LCD_displayNormal);
		lcd_send_cmd( LCD_bias);
		lcd_send_cmd( LCD_powerCntl);
		lcd_send_cmd( LCD_booster); lcd_send_cmd( 0x00);
		lcd_send_cmd( LCD_voltageReg);
		lcd_send_cmd( LCD_contrast); lcd_send_cmd( 0x1f);
		lcd_send_cmd( LCD_indicator); lcd_send_cmd( 0x00);
		lcd_send_cmd( LCD_displayOn);
		LCD_CSb = 1;
	}
}

void lcd_display_mute(void)
{
}

void lcd_display_nolock(void)
{
}

void lcd_display_volume(byte msdigit, byte lsdigit)
{
	if (!has_lcd_display)
		return;

	LCD_CSb = 0;
	lcd_set_addr(0, 0);
	lcd_send_data( 0x3f);
	lcd_send_data( msdigit);
	lcd_send_data( lsdigit);
	lcd_send_data( 0xc3);
	LCD_CSb = 1;
}

static void lcd_send_byte( byte data)
{
	short i;

	for (i=0; i<8; )
	{
		LCD_SCL = 0;
		LCD_SI  = 0 != (data & 0x80); // set 'SI' output data bit, MSB first
		data <<= 1;
		i++;
		LCD_SCL = 1; // create a few cycles delay between setting data and clock rising edge
	}		
}

static void lcd_set_addr(byte row_addr, byte col_addr)
{
	lcd_send_cmd( LCD_pageAddr  | row_addr);
	lcd_send_cmd( LCD_colAddrLo | (col_addr & 0x0f));
	lcd_send_cmd( LCD_colAddrHi | (col_addr >> 4));
}


