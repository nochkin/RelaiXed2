/******************************************************************************
 * Module to control the DOGL128-6E LCD Display
 *
 * Copyright 2012  Nicola Zandona (based on a Jos van Eijndhoven original idea)
 *****************************************************************************/
 

#include <spi.h>
#include <stdio.h>
#include <string.h>
#include "typedefs.h"
#include "lcd.h"
#include "io_cfg.h"
#include "font_5x8.h"
#include "bmp.h"
#include "fonts.h"
#include "defines.h"


void lcd_write_command (unsigned char command);
void lcd_write_data (unsigned char command);
void lcd_hw_reset (void);
void init_spi2(void);
unsigned char WriteSPI2(unsigned char data_out);
byte lcd_write_gfx(byte x_pos, byte y_pos, byte x_size, byte y_size, far rom unsigned char* data, byte array_y_size);
byte lcd_clear_gfx(byte x_pos, byte y_pos, byte x_size, byte y_size);

#define LCD_POWER_PIN		LATCbits.LATC7
#define	LCD_POWER_DIR 		TRISCbits.TRISC7

#define LCD_A0_PIN(m)  		{TRISCbits.TRISC1 = (m)? 1: 0; PORTCbits.RC1 = (m)? 0:0;}
#define LCD_A0_DIR			TRISCbits.TRISC1

#define LCD_RESET_PIN(m)  		{TRISBbits.TRISB3 = (m)? 1: 0; PORTBbits.RB3 = (m)? 1:0;}
#define LCD_RESET_DIR		TRISBbits.TRISB3

#define LCD_CS_PIN(m) 			{TRISCbits.TRISC2 = (m)? 1: 0; PORTCbits.RC2 = (m)? 1:0;}
#define LCD_CS_DIR			TRISCbits.TRISC2

#define LCD_SENSE_PIN  		PORTCbits.RC0

#define LCD_BACKLIGHT_PIN  	LATBbits.LATB2	//PORTBbits.RB2
#define LCD_BACKLIGHT_DIR 	TRISBbits.TRISB2




#pragma udata large_udata
unsigned char ram LCD_shadow[LCD_PAGE_MAX][LCD_COL_MAX];
#pragma udata

/* exported globals */
// flag to singanl where a lcd or a 7-seg display is connected
volatile byte has_lcd_display = 0;

// flag to signal if the shadow memory can be copied into the LCD device or if it is still being written
byte lcd_wr_ack;

// flag to signal whether to shadow has been modified or not
byte memory_tainted;

/* Check for presence of LCD.
 * The LCD is connected into the left 7-segment socket
 * Note that at this 'init' time, the timer for display L/R multiplex is not running.
 * Also, assume that Port B and C, which normally drive the LEDs in open-collector mode,
 * were preset in the 'input' state.
 */
void detect_display(void)
{
	LEDright = 0; // power the left LCD, it connects to the left 7-seg socket.
	PORTCbits.RC7 = 0; 
	
	// LCD_SENSE_PIN is already input [set during init() ]
	if (LCD_SENSE_PIN)	{ 
		// normal LED: read-back from LED shows high
		has_lcd_display = 0;
	} 
	else { 
		// LCD display	
		has_lcd_display = 1;
	}
}


void power_off_display() {
	// NZ: please note that after an power down , the LCD must be re-initialized.
	LEDright = 1; 
}


void power_lcd_display(byte power) {
	lcd_write_command(LCD_POWER_CTRL(power));
}

void power_lcd_backlight(byte power) {
	switch (power) {
		case LCD_BACKLIGHT_OFF:
			break;

		case LCD_BACKLIGHT_FULL:
		default:
			break;

		case LCD_BACKLIGHT_DIMMED:
			break;
	}
}


void config_lcd(void)
{
	mLED_1_On(); // turn into output
	mLED_2_On(); // turn into output
//	mLED_3_On(); // turn into output
//	mLED_4_On(); // turn into output
//	mLED_5_On(); // turn into output
	mLED_6_On(); // turn into output
//	mLED_7_On(); // turn into output

	LCD_A0_DIR			= 1;
	LCD_RESET_DIR		= 1;
	LCD_CS_DIR			= 1;

	// setup the remappable peripheral outputs:
	// Clear IOLOCK
	EECON2 = 0x55;
	EECON2 = 0xAA;
	PPSCONbits.IOLOCK = 0;
	RPOR3 = 9; // seg(b) -> RB0 -> RP3 -> SDO2 (SPI2 Data Out)
	RPOR4 = 10; // seg(f) -> RB1 -> RP4 -> SCK2 (SPI2 CLK)
	// Set IOLOCK
	EECON2 = 0x55;
	EECON2 = 0xAA;
	PPSCONbits.IOLOCK = 1;

	// init SPI2
	init_spi2();

	// control backlight LCD
	LCD_BACKLIGHT_PIN = 1; //NZ: useless for the moment....
	
	// reset LCD
	lcd_hw_reset();

	// init display
	LCD_RESET_PIN(1);
	LCD_CS_PIN(0);
	LCD_A0_PIN(0);
	WriteSPI2(LCD_START_LINE_ADDR(0));
	WriteSPI2(LCD_ADC_SELECT(LCD_ADC_REVERSE));
	WriteSPI2(LCD_COM_SCAN_DIR(LCD_COM_NORMAL_DIR));
	WriteSPI2(LCD_ORIENTATION(LCD_ORIENTATION_NORMAL));
	WriteSPI2(LCD_SET_BIAS(LCD_BIAS_1_9));
	WriteSPI2(LCD_PWR_CTRL(LCD_PWR_BOOSTER_ON | LCD_PWR_REGULATOR_ON | LCD_PWR_FOLLOWER_ON));
	WriteSPI2(LCD_BOOSTER_RATIO_MODE);
	WriteSPI2(LCD_BOOSTER_RATIO_SET_VALUE(LCD_BOOSTER_2X_3X_4X));
	WriteSPI2(LCD_V0_VOLT_REGULATOR(7)); 		//from data sheet. meaning?
	WriteSPI2(LCD_ELECTR_VOL_MODE_SET);
	WriteSPI2(LCD_ELECTR_VOL_SET_VALUE(0x16)); 	//from data sheet. meaning?
	WriteSPI2(LCD_STATIC_INDICATOR_MODE);
	WriteSPI2(LCD_STATIC_INDICATOR_SET(LCD_STATIC_INDICATOR_OFF));
	//WriteSPI2(LCD_POWER_CTRL(LCD_POWER_ON));
	LCD_CS_PIN(1);

	// clear lcd memory
	clear_lcd();

	memory_tainted = 0;
}

void lcd_write_command(unsigned char command)
{
	// chip select
	LCD_CS_PIN(0)
	// set A0 to 0 (command)
	LCD_A0_PIN(0);
	// write the command thru the SPI
	WriteSPI2(command);
	// set A0 to 1 (next will be data)
	LCD_A0_PIN(1);
	// clear CS
	LCD_CS_PIN(1);
} 

void lcd_write_data(unsigned char data)
{
	// chip select
	LCD_CS_PIN(0);
	// set A0 to 1 (data)
	LCD_A0_PIN(1);
	// write the data thru the SPI
	WriteSPI2(data);
	// clear CS
	LCD_CS_PIN(1);
} 

void lcd_hw_reset (void)
{
	int i;
	// reset LCDF
	LCD_RESET_PIN(0);
	// delay
	for (i=0; i<100; i++);
	LCD_RESET_PIN(1);
	for (i=0; i<100; i++);
} 


void init_spi2(void) {
	SSP2STAT = 0x40;        // Set SMP=0 and CKE=1. Notes: The lower 6 bit is read only
	SSP2CON1 = 0x20;        // Enable SPI Master with Fosc/4
	//SSP2CON1 = 0x30;        // Enable SPI Master with Fosc/4
}


unsigned char WriteSPI2(unsigned char data_out) {
	unsigned char TempVar;  
	
	TempVar = SSP2BUF;           // Clears BF
	PIR3bits.SSP2IF = 0;         // Clear interrupt flag
	SSP2CON1bits.WCOL = 0;			//Clear any previous write collision
	SSP2BUF = data_out;          // write byte to SSP2BUF register
	if ( SSP2CON1 & 0x80 )       // test if write collision occurred
		return ( -1 );              // if WCOL bit is set return negative #
	else
	 	// while( !SSP2STATbits.BF ); // wait until bus cycle complete 
		while(!PIR3bits.SSP2IF); //wait until bus cycle complete
	return ( 0 );                // if WCOL bit is not set return non-negative#
}

void clear_lcd(void)
{
	int page, col;
	for (page = 0; page<LCD_PAGE_MAX; page++) {
		lcd_write_command(LCD_SET_PAGE_ADDR(page)); //page
		lcd_write_command(LCD_SET_COL_MSB(0));		// begin of the page
		lcd_write_command(LCD_SET_COL_LSB(0));
		for (col=0; col<LCD_COL_MAX; col++)
		{
			lcd_write_data(0x00);							//LCD auto increments col address in RAM ==> loop on cols but without "col" index in arg
			LCD_shadow[page][col] = 0x00;
		}
	}
	memory_tainted = 1;
}


void isr_lcd_refresh(void)
{
	int page, col;

	if (SHADOW_MEMORY_IS_LOCKED)	
		return;

	if (!memory_tainted)
		return;
	
	for (page = 0; page<LCD_PAGE_MAX; page++) {
		lcd_write_command(LCD_SET_PAGE_ADDR(page)); //page
		lcd_write_command(LCD_SET_COL_MSB(0));		// begin of the page
		lcd_write_command(LCD_SET_COL_LSB(0));
		for (col=0; col<LCD_COL_MAX; col++)
			lcd_write_data(LCD_shadow[page][col]);		//LCD auto increments col address in RAM ==> loop on cols but without "col" index in arg
	}
	
	memory_tainted = 0;
}



byte lcd_clear_gfx(byte x_pos, byte y_pos, byte x_size, byte y_size) {
	
	byte x;

	for (x=x_pos; x < (x_pos+x_size); x++) {
	
		lcd_write_gfx(x, y_pos, 1, y_size, _emptycol, 8);
	}

	

}


byte lcd_write_gfx(byte x_pos, byte y_pos, byte x_size, byte y_size, far rom unsigned char* data, byte array_y_size_in_byte)
{
	byte error = 0x00;
	byte page_start, page_end;
	byte page_offset_top, page_offset_bottom;
	byte col_cnt, page_cnt;
	int i;

	byte data_col_idx;

	unsigned char mask;
	unsigned char mask2;
	unsigned char tmp[8];
	unsigned char tmp2;
	unsigned char carry;

	memory_tainted = 1;

	if ((x_pos + x_size - 1)> LCD_COL_MAX)
		error |= 0x01; 
	if ((y_pos + y_size) > LCD_ROW_MAX)
		error |= 0x02;

	// shadow ram indexes (target)
	page_start = y_pos>>3;					// page start index
	page_end = MIN((y_pos + y_size -1)>>3,7);		// page end index
	page_offset_top = y_pos & 0x07;				// shift inside page/byte for shadow
	page_offset_bottom = (y_pos + y_size) & 0x07;				// shift inside page/byte for shadow
	
	
	for (col_cnt=x_pos; col_cnt<(x_pos+x_size); col_cnt++)
	{
		data_col_idx = col_cnt-x_pos;

		// copy a whole column into temp 
		for (i=0; i<8 && (i+page_start)<LCD_PAGE_MAX; i++)
			tmp[i+page_start] = data[((int)data_col_idx*(int)array_y_size_in_byte) + i];
			
		
		// shift "down"
		page_offset_top = page_offset_top;
		for (i=page_start; i<=page_end; i++) {
			if (i==page_start) {
				// tmp[i] 							   7654 3xxx
				// i.e page_offset_top = 2 
				carry = tmp[i] >> (8-page_offset_top);		// xxxx xx76
				mask = (1<<(page_offset_top))-1;			// 0000 0011
				carry &= mask;						// 0000 0076
				tmp[i] <<= page_offset_top;					// 543x xxxx
			}
			else if (i==page_end) {
				// tmp[i] 							   GHFE DCBA
				tmp[i] <<= page_offset_top;					// FEDC BAxx
				tmp[i] &= ~mask;					// FEDC BA00
				// carry set below					   0000 00hg
				// mask set above                      0000 0011
				tmp[i] |= carry;					// FEDC BAhg
				// data_offset_bottom = 3
				mask = (1<<(page_offset_bottom==0?8:page_offset_bottom))-1;	// 0000 0111
				tmp[i] &= mask;						// 0000 0Ahg
			}
			else if ((i > page_start) && (i < page_end)) {
				// mask set above                      0000 0011
				tmp2 = carry;						// 0000 0076
				// tmp[i] 							   hgfe dcba
				carry = tmp[i] >> (8-page_offset_top);		// xxxx xxhg
				carry &= mask;						// 0000 00hg
				tmp[i] <<= page_offset_top;					// fedc baxx
				tmp[i] &= ~mask;					// fedc ba00
				tmp[i] |= tmp2;						// fedc ba76
			}
			
			if (page_start == page_end) {
				tmp[i] <<= page_offset_top;
				mask = ~((1<<page_offset_top)-1);
				mask2 = ~(1<<page_offset_bottom)-1;
				mask = ~(mask ^ mask2); // X-NOR
				tmp[i] &= mask;
			}
		}
	
			
		for (page_cnt=/*0*/page_start; page_cnt<=page_end/*7*/; page_cnt++)
		{
			/*if (page_cnt < page_start) {
				continue;
			}
			else */
			if (page_cnt == page_start) {
				mask = (1<<page_offset_top)-1;
			}
			else if ((page_cnt > page_start) && (page_cnt < page_end)) {
				mask = 0x00;
			}
			else if (page_cnt == page_end) {
				mask = ~((1<<(page_offset_bottom==0?8:page_offset_bottom))-1);
			}
			/*
			else // page_cnt > page_end
				continue;
			*/
			
			if (page_start == page_end) {
				mask = (1<<page_offset_top)-1;
				mask2 = (1<<page_offset_bottom)-1;
				mask = (mask ^ mask2); // X-OR
			}

			LCD_shadow[page_cnt][col_cnt] &= mask;
			tmp[page_cnt] &= ~mask;
			LCD_shadow[page_cnt][col_cnt] |= tmp[page_cnt];
		}
	}
	return error;
}


void lcd_display_volume(byte msdigit, byte lsdigit)
{
	// lock shadow memory for write
	LOCK_SHADOW_MEMORY;

#ifdef VOLUME_DB_LCD_DISPLAY
	// display 'minus' symbol
	lcd_write_gfx(LCD_VOLUME_MINUS_X_POS, LCD_VOLUME_MINUS_Y_POS, LCD_VOLUME_MINUS_X_SIZE, LCD_VOLUME_MINUS_Y_SIZE, minus32, LCD_VOLUME_MINUS_ARRAY_Y_SIZE_IN_BYTES);

	// display 'dB' symbol
	lcd_write_gfx(LCD_VOLUME_DB_SYMBOL_X_POS, LCD_VOLUME_DB_SYMBOL_Y_POS, LCD_VOLUME_DB_SYMBOL_X_SIZE, LCD_VOLUME_DB_SYMBOL_Y_SIZE, dB, LCD_VOLUME_DB_SYMBOL_ARRAY_Y_SIZE_IN_BYTES);

	// display infinity symbol
	if ((msdigit == 6) && (lsdigit == 4)) {
		lcd_write_gfx(LCD_VOLUME_1ST_DIGIT_X_POS, LCD_VOLUME_1ST_DIGIT_Y_POS, LCD_FONT_INFINITY_X_SIZE, LCD_FONT_INFINITY_Y_SIZE, infinity, LCD_FONT_INFINITY_ARRAY_Y_SIZE_IN_BYTES);
	}
	else {
		// display ms digit
#ifdef VOLUME_REMOVE_LEADING_ZERO
		if (msdigit != 0)
			lcd_write_gfx(LCD_VOLUME_1ST_DIGIT_X_POS, LCD_VOLUME_1ST_DIGIT_Y_POS, LCD_VOLUME_1ST_DIGIT_X_SIZE, LCD_VOLUME_1ST_DIGIT_Y_SIZE, NeoSansIntel32+((int)msdigit*(int)LCD_VOLUME_1ST_DIGIT_X_SIZE*(int)LCD_VOLUME_1ST_DIGIT_ARRAY_Y_SIZE_IN_BYTES), LCD_VOLUME_1ST_DIGIT_ARRAY_Y_SIZE_IN_BYTES);
		else 
			lcd_clear_gfx(LCD_VOLUME_1ST_DIGIT_X_POS, LCD_VOLUME_1ST_DIGIT_Y_POS, LCD_VOLUME_1ST_DIGIT_X_SIZE, LCD_VOLUME_1ST_DIGIT_Y_SIZE);
#else
		lcd_write_gfx(LCD_VOLUME_1ST_DIGIT_X_POS, LCD_VOLUME_1ST_DIGIT_Y_POS, LCD_VOLUME_1ST_DIGIT_X_SIZE, LCD_VOLUME_1ST_DIGIT_Y_SIZE, NeoSansIntel32+((int)msdigit*(int)LCD_VOLUME_1ST_DIGIT_X_SIZE*(int)LCD_VOLUME_1ST_DIGIT_ARRAY_Y_SIZE_IN_BYTES), LCD_VOLUME_1ST_DIGIT_ARRAY_Y_SIZE_IN_BYTES);
#endif
		// clear 2 cols
		lcd_clear_gfx(LCD_VOLUME_1ST_DIGIT_X_POS+LCD_VOLUME_1ST_DIGIT_X_SIZE, LCD_VOLUME_1ST_DIGIT_Y_POS, LCD_VOLUME_2ND_DIGIT_X_POS-(LCD_VOLUME_1ST_DIGIT_X_POS+LCD_VOLUME_1ST_DIGIT_X_SIZE), LCD_VOLUME_1ST_DIGIT_Y_SIZE);

		// display ls digit
		lcd_write_gfx(LCD_VOLUME_2ND_DIGIT_X_POS, LCD_VOLUME_2ND_DIGIT_Y_POS, LCD_VOLUME_2ND_DIGIT_X_SIZE, LCD_VOLUME_2ND_DIGIT_Y_SIZE, NeoSansIntel32+((int)lsdigit*(int)LCD_VOLUME_2ND_DIGIT_X_SIZE*(int)LCD_VOLUME_2ND_DIGIT_ARRAY_Y_SIZE_IN_BYTES), LCD_VOLUME_2ND_DIGIT_ARRAY_Y_SIZE_IN_BYTES);
	}
#else

	// display ms digit
#ifdef VOLUME_REMOVE_LEADING_ZERO
	if (msdigit != 0)
		lcd_write_gfx(LCD_VOLUME_1ST_DIGIT_X_POS, LCD_VOLUME_1ST_DIGIT_Y_POS, LCD_VOLUME_1ST_DIGIT_X_SIZE, LCD_VOLUME_1ST_DIGIT_Y_SIZE, NeoSansIntel32+((int)msdigit*(int)LCD_VOLUME_1ST_DIGIT_X_SIZE*(int)LCD_VOLUME_1ST_DIGIT_ARRAY_Y_SIZE_IN_BYTES), LCD_VOLUME_1ST_DIGIT_ARRAY_Y_SIZE_IN_BYTES);
	else 
		lcd_clear_gfx(LCD_VOLUME_1ST_DIGIT_X_POS, LCD_VOLUME_1ST_DIGIT_Y_POS, LCD_VOLUME_1ST_DIGIT_X_SIZE, LCD_VOLUME_1ST_DIGIT_Y_SIZE);
#else
	lcd_write_gfx(LCD_VOLUME_1ST_DIGIT_X_POS, LCD_VOLUME_1ST_DIGIT_Y_POS, LCD_VOLUME_1ST_DIGIT_X_SIZE, LCD_VOLUME_1ST_DIGIT_Y_SIZE, NeoSansIntel32+((int)msdigit*(int)LCD_VOLUME_1ST_DIGIT_X_SIZE*(int)LCD_VOLUME_1ST_DIGIT_ARRAY_Y_SIZE_IN_BYTES), LCD_VOLUME_1ST_DIGIT_ARRAY_Y_SIZE_IN_BYTES);
#endif

	// display ls digit
	lcd_write_gfx(LCD_VOLUME_2ND_DIGIT_X_POS, LCD_VOLUME_2ND_DIGIT_Y_POS, LCD_VOLUME_2ND_DIGIT_X_SIZE, LCD_VOLUME_2ND_DIGIT_Y_SIZE, NeoSansIntel32+((int)lsdigit*(int)LCD_VOLUME_2ND_DIGIT_X_SIZE*(int)LCD_VOLUME_2ND_DIGIT_ARRAY_Y_SIZE_IN_BYTES), LCD_VOLUME_2ND_DIGIT_ARRAY_Y_SIZE_IN_BYTES);

#endif



	// release shadow memory for write
	RELEASE_SHADOW_MEMORY;
}

void lcd_display_mute(byte muted)
{
	// lock shadow memory for write
	LOCK_SHADOW_MEMORY;

	if (muted) {
		// display 'Mute' bmp
		lcd_write_gfx(LCD_VOLUME_MUTE_SYMBOL_X_POS, LCD_VOLUME_MUTE_SYMBOL_Y_POS, LCD_VOLUME_MUTE_SYMBOL_X_SIZE, LCD_VOLUME_MUTE_SYMBOL_Y_SIZE, Mute, LCD_BMP_MUTE_ARRAY_Y_SIZE_IN_BYTES);
	}
	else {
		// clear 'Mute' bmp
		lcd_clear_gfx(LCD_VOLUME_MUTE_SYMBOL_X_POS, LCD_VOLUME_MUTE_SYMBOL_Y_POS, LCD_VOLUME_MUTE_SYMBOL_X_SIZE, LCD_VOLUME_MUTE_SYMBOL_Y_SIZE);
	
	}

	// release shadow memory for write
	RELEASE_SHADOW_MEMORY;
}





void draw_lcd_bmp() {

	// lock shadow memory for write
	LOCK_SHADOW_MEMORY;

	// draw balance scale
	lcd_write_gfx(/*x_pos*/ LCD_BALANCE_POS_X, /*y_pos*/ LCD_BALANCE_POS_Y, /*x_size*/ LCD_BALANCE_SIZE_X, /*y_size*/ LCD_BALANCE_SIZE_Y, BalanceBmp, /*array_y_size_in_byte*/ 2 );

	// draw balance logo
	// tdb

	// draw volume scale
	// tbd

	// draw channel logo
	// tbd



	// release shadow memory for write
	RELEASE_SHADOW_MEMORY;
}

void lcd_display_balance(char master_balance) {

	byte master_balance_abs;
	if (master_balance>0)
		master_balance_abs = master_balance;
	else
		master_balance_abs = -master_balance;

	// lock shadow memory for write
	LOCK_SHADOW_MEMORY;

	// re-draw balance scale
	lcd_write_gfx(/*x_pos*/ LCD_BALANCE_POS_X, /*y_pos*/ LCD_BALANCE_POS_Y, /*x_size*/ LCD_BALANCE_SIZE_X, /*y_size*/ LCD_BALANCE_SIZE_Y, BalanceBmp, /*array_y_size_in_byte*/ 2 );
	
	// draw balance index
	lcd_write_gfx(/*x_pos*/ LCD_BALANCE_CENTER_X + (master_balance*2), /*y_pos*/ LCD_BALANCE_POS_Y+8, /*x_size*/ 1, /*y_size*/ 3, BalanceIndex, /*array_y_size_in_byte*/1);

	// draw balance value
	if (master_balance>0) // '+'
		lcd_write_gfx(/*x_pos*/ LCD_BALANCE_POS_X, /*y_pos*/ LCD_BALANCE_POS_Y + LCD_BALANCE_SIZE_Y + 1, /*x_size*/ 5, /*y_size*/ 8, font5x8+((0x2B-0x20)*5*1),  /*array_y_size_in_byte*/1);
	else if (master_balance<0) // '-'
		lcd_write_gfx(/*x_pos*/ LCD_BALANCE_POS_X, /*y_pos*/ LCD_BALANCE_POS_Y + LCD_BALANCE_SIZE_Y + 1, /*x_size*/ 5, /*y_size*/ 8, font5x8+((0x2D-0x20)*5*1),  /*array_y_size_in_byte*/1);
	else // well.. it should be zero then. in such a case just write a space to erase the shadow 
		lcd_write_gfx(/*x_pos*/ LCD_BALANCE_POS_X, /*y_pos*/ LCD_BALANCE_POS_Y + LCD_BALANCE_SIZE_Y + 1, /*x_size*/ 5, /*y_size*/ 8, font5x8+((0x20-0x20)*5*1),  /*array_y_size_in_byte*/1);	

	//write abs value
	lcd_write_gfx(/*x_pos*/ LCD_BALANCE_POS_X + 5 + 1, /*y_pos*/ LCD_BALANCE_POS_Y + LCD_BALANCE_SIZE_Y + 1, /*x_size*/ 5, /*y_size*/ 8, font5x8+((master_balance_abs+0x30-0x20)*5*1), /*array_y_size_in_byte*/1);
	
	// release shadow memory for write
	RELEASE_SHADOW_MEMORY;
}

void lcd_display_info(const rom char* msg) {

#if 0
	sizerom_t msg_size;
	byte chr_pos;
	byte x_pos = 0;

	clear_lcd();
	msg_size = strlenpgm(msg);
	
	for (chr_pos=0; chr_pos < msg_size; chr_pos++) {
		lcd_write_gfx(/*x_pos*/ x_pos, /*y_pos*/ 0, /*x_size*/ 5, /*y_size*/ 8, font5x8+((msg[chr_pos]-0x20)*5*1), /*array_y_size_in_byte*/1);	
		x_pos += 6;
	}
#endif
}

void lcd_display_channel(byte channel) {
	// lock shadow memory for write
	LOCK_SHADOW_MEMORY;


	// display 'input'
	lcd_write_gfx(LCD_CHANNEL_INPUT_STRING_X_POS, LCD_CHANNEL_INPUT_STRING_Y_POS, LCD_CHANNEL_INPUT_STRING_X_SIZE, LCD_CHANNEL_INPUT_STRING_Y_SIZE, input_string, LCD_CHANNEL_INPUT_STRING_ARRAY_Y_SIZE_IN_BYTES);

	// display channel id
	lcd_write_gfx(LCD_CHANNEL_ID_X_POS, LCD_CHANNEL_ID_Y_POS, LCD_CHANNEL_ID_X_SIZE, LCD_CHANNEL_ID_Y_SIZE, channelDIGIT+((channel-1)*(int)LCD_CHANNEL_ID_X_SIZE*(int)LCD_FONT_CH_ID_ARRAY_Y_SIZE_IN_BYTES ), LCD_FONT_CH_ID_ARRAY_Y_SIZE_IN_BYTES );

	// release shadow memory for write
	RELEASE_SHADOW_MEMORY;

}

void lcd_display_bmp (byte x_pos, byte y_pos, byte x_size, byte y_size, far rom unsigned char* data, byte array_y_size) {
	byte row;

	int i;

	// lock shadow memory for write
	LOCK_SHADOW_MEMORY;


	lcd_write_gfx(x_pos, y_pos, x_size, y_size, data, array_y_size);

	// release shadow memory for write
	RELEASE_SHADOW_MEMORY;
}