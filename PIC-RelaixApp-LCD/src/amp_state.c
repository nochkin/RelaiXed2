/****************************************************************************************
	This file is part of the Relaixed firmware.
    The Relaixed firmware is intended to control a DIY audio premaplifier.
    Copyright 2011 Jos van Eijndhoven, The Netherlands

    The Relaixed firmware is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The Relaixed firmware is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this firmware.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************************/
/*
 * Code to keep track of amplifier state variables
 */
 
#include "typedefs.h"
#include <p18cxxx.h>
#include "io_cfg.h"
#include "amp_state.h"
#include "usb_io.h"
#include "display.h"
#include "relays.h"
#include "storage.h"
#include "dac_cntl.h"
#include "lcd.h"
#include "bmp.h"
#include "defines.h"
 
volatile char volume_incr; // modified from isr
static char volume_incr_carry;

volatile char channel_incr;
volatile char power_incr;
volatile char balance_incr;
static char balance_incr_carry;

static char power, muted;
static byte n_channels, analog_channel;


static union store_volume
{
	struct
	{
		StorageKey key;
		byte	_volume;
		char	_balance; // signed
		byte	_channel;
	};
	unsigned int words[2];
} StoreVolume;

#define master_volume	StoreVolume._volume
#define channel			StoreVolume._channel
#define master_balance	StoreVolume._balance

// Input selection channels: numbered 0 .. 5
#define NCHANNELS 6

void amp_state_init(void)
{
	volume_incr = 0;
    volume_incr_carry = 0;
	balance_incr = 0;
    balance_incr_carry = 0;
	power_incr = 0;
	channel_incr = 0;
	channel = 0;
	power = 0;
}	

static void set_volume_balance_relays(void)
{
	char balance_left, balance_right;
	char volume_left, volume_right;

	if (master_volume == 0 || channel == 0 || power == 0 || muted != 0)
	{
		// Don't let balance-computations result in non-zero volume
		set_relays(0x00, power, channel, 0x00, 0x00);
		return;
	}

	// Hmm... Microchip's C compiler wrongly implements signed >>
	balance_right = (master_balance >> 1) | (master_balance & 0x80);
	balance_left  = balance_right - master_balance;
	volume_right  = (char)master_volume + balance_right;
	volume_left   = (char)master_volume + balance_left;

	if (volume_right > 64) volume_right = 64;
	if (volume_left  > 64) volume_left  = 64;
	if (volume_right <  0) volume_right = 0;
	if (volume_left  <  0) volume_left  = 0;


	set_relays(0x00, power, analog_channel, volume_left, volume_right);
}

void volume_display(char override)
{
	byte vol_div_10, vol_by_10, vol_mod_10;
#ifdef VOLUME_DB_LCD_DISPLAY
	byte attenuation;
#endif

	//char msg[2]; 

	if (muted)
	{
		if (!has_lcd_display)
			display_set( 0x00, 0x00, override);
		else {
			//lcd_display_volume(0x00, 0x00);		
			lcd_display_mute(muted);
		}
		return;
	}
	else if (dac_status() == DAC_NOLOCK)
	{
		if (!has_lcd_display)
			display_set( DIGIT_n, DIGIT_o, override);
		return;
	}

#ifdef VOLUME_DB_LCD_DISPLAY
	attenuation = 64 - master_volume;
	vol_div_10 = 0;
	for (vol_by_10 = 10; vol_by_10 <= attenuation; vol_by_10 += 10)
		vol_div_10++;
	vol_mod_10 = attenuation - vol_by_10 + 10;
#else
	vol_div_10 = 0;
	for (vol_by_10 = 10; vol_by_10 <= master_volume; vol_by_10 += 10)
		vol_div_10++;
	vol_mod_10 = master_volume - vol_by_10 + 10;
#endif
	if (!has_lcd_display)
		display_set( vol_div_10, vol_mod_10, override);
	else
		lcd_display_volume(vol_div_10, vol_mod_10);
}

void volume_update(void)
{
	byte vol_div_10, vol_by_10, vol_mod_10;
	char vol_msg[] = {'V', '+', '0'};
	char vol_incr_abs;

	if (power < 2)
		return;

	if (muted)
	{
		if (has_lcd_display)
			lcd_display_mute(muted);
		muted = 0;
		volume_incr = 0; // first unmute before changing volume setting
	}

	volume_incr_carry += volume_incr;
	volume_incr = 0; // quick copy-and-reset: avoid intermediate interrupts

	vol_incr_abs = volume_incr_carry;
	if (volume_incr_carry < 0)
	{
		vol_incr_abs = -volume_incr_carry;
		vol_msg[1] = '-';
	}
    vol_msg[2] = (vol_incr_abs <= 9) ? '0' + vol_incr_abs : '*';

	// Our volume rotary switch giver two pulses per notch,
	// but we want to incr volume with 1 per notch.
    // In stead of simply div by 2, we build-in a hysteresis,
    // giving more stable behavior in case of switch spikes.
    // So, repeated +1,-1 will not cause relay activity.
	while (volume_incr_carry >= 2)
	{
		if (master_volume < 64)
			master_volume++;

		volume_incr_carry -= 2;		
	}

	while (volume_incr_carry <= -2)
	{
		if (master_volume > 0)
			master_volume--;

		volume_incr_carry += 2;		
	}

	if (master_volume > 64)
		master_volume = 0; // weird, flash_load error?

	set_volume_balance_relays();
	volume_display( 1);
	usb_write( vol_msg, (byte)3); // three-char message
}

void balance_update(void)
{
	if (power < 2 || muted)
		return;

	// Similar carefull updating as for volume...
	balance_incr_carry += balance_incr;
	balance_incr = 0; // quick copy-and-reset: avoid intermediate interrupts

	while (balance_incr_carry >= 2)
	{
		if (master_balance < 9)
			master_balance++;

		balance_incr_carry -= 2;		
	}

	while (balance_incr_carry <= -2)
	{
		if (master_balance > -9)
			master_balance--;

		balance_incr_carry += 2;		
	}

	set_volume_balance_relays();

	if (!has_lcd_display)
	{
		if (master_balance >= 0)
		{
			display_set_alt( DIGIT_dark, master_balance, 2);
		} else
		{
			display_set_alt( DIGIT_minus, -master_balance, 2);
		}
	}
	else {
		lcd_display_balance(master_balance);

	}
}

void mute(void)
{
	if (power != 2)
		return;

	muted = 1;
	set_relays(0x00, power, 0x00, 0x00, 0x00);

	if (!has_lcd_display)
		display_set( 0x00, 0x00, 1);
	else
	{
		lcd_display_mute(muted);
		//lcd_display_volume(0x00, 0x00);		
	}
}

void unmute(void)
{
	if (power != 2)
		return;

	volume_incr = 0;
	volume_update();	
}

void channel_update(void)
{
	char chan_msg[] = {'C', '+', '0'};
	char chan_incr_abs;
    byte dac_channel;

	if (power < 2)
		return;

	if (channel_incr >= 0)
	{
		chan_incr_abs = channel_incr;
	} else
	{
		chan_incr_abs = -channel_incr;
		chan_msg[1] = '-';
	}
    chan_msg[2] = (chan_incr_abs <= 9) ? '0' + chan_incr_abs : '*';

	channel += channel_incr;
	channel_incr = 0;

    n_channels = 6; // default relaixed
	if (dac_status() != DAC_ABSENT)
		n_channels = 9; // 4 digital inputs into one analog input

	while (channel > n_channels)
		channel -= n_channels;
	if (channel == 0)
		channel = n_channels;

    // if we have a dac, it is a 4-input dac connected to analog input channel 4
    // the dac digital inputs are numbered 1-4 in the user interface
    if (dac_status() != DAC_ABSENT)
	{
		analog_channel = (channel <= 4) ? 4 :
						 (channel <= 7) ? channel - 4 :
						  channel - 3;
		dac_channel = (channel <= 4) ? channel - 1 : 4;
		dac_set_channel( dac_channel);
	} else
		analog_channel = channel;

	
	set_volume_balance_relays();
	volume_display(0);

	if (!has_lcd_display)
		display_set_alt( DIGIT_C, channel, 2); // repeat channel-display twice
	else
		lcd_display_channel(channel);		
	
	usb_write( chan_msg, (byte)3); // three-char message
}

// channel_set with absolute channel number as argument, called from ir_receiver only
void channel_set( unsigned char new_ch)
{
	if (dac_status() == DAC_ABSENT && new_ch > 6)
		return; // silently ignore unsupported button

	channel_incr = new_ch - channel;
	channel_update();
}

void power_update(void)
{
	// maintain different power states:
	// 0: audio is muted, analog power is off, minimal stand-by power
	// 1: analog power is switched-on, audio is muted, timer running to enter next power level
	// 2: analog power is on, audio is on

	char chan_msg[] = {'P', '0'};

	if (power_incr == -1 && power != 0)
	{
		// first do audio mute...
		set_relays(0x00, power, 0x00, 0x00, 0x00);
		if (!has_lcd_display)
			display_set( DIGIT_dark, DIGIT_dark, 1);
		else {
			// power backlight and display OFF
			power_lcd_backlight(LCD_BACKLIGHT_OFF);
			power_lcd_display(LCD_POWER_OFF);
		}
		// and follow immediatly with analog power shutdown

        // set volume-state down AFTER power-state down,
        // otherwise an interrupt-flash-tick might store a (wrong) 0 volume to flash
		power = 0;
		master_volume = 0; 
		volume_incr_carry = 0;
		set_relays(0x00, 0x00, 0x00, 0x00, 0x00);
	} else if (power_incr > 0 && power == 0)
	{
		power = 1;

		if (!has_lcd_display)
			display_set( 0x00, 0x00, 1);
		else {
			// power backlight and display ON
			power_lcd_display(LCD_POWER_ON);
			power_lcd_backlight(LCD_BACKLIGHT_FULL);
			// clear lcd
			clear_lcd();
			// display splash
			lcd_display_bmp (0, 0, LCD_BMP_SPLASH_X_SIZE, LCD_BMP_SPLASH_Y_SIZE, Realixed2Splash_bmp, LCD_BMP_SPLASH_ARRAY_Y_SIZE_IN_BYTES);		
		}

		set_relays(0x00, power, 0x00, 0x00, 0x00);
		// Enable analog power, stay in mute
		// Later, from the 'power_tick' counter, power will be incremented to 2
	} else if (power_incr > 0 && power == 1)
	{
		power = 2;
		if (has_lcd_display)
			clear_lcd(); //clear splash
		flash_load(KeyVolume, &StoreVolume.key);
		volume_incr = 0;
		channel_incr = 0;
		balance_incr = 0;
		muted = 0;
		volume_update();  // shows volume in display
		channel_update(); // shows channel in temporary display

		PIR2bits.LVDIF = 0;
		PIE2bits.LVDIE = 1; // watch power-supply level for drops
	}

	power_incr = 0;
	chan_msg[1] = '0' + power;
	usb_write( chan_msg, (byte)2); // two-char message
}

void flash_volume_channel(void)
{
	char flash_msg[] = {'F', 'v'};

	if (power != 2)
		return; // Only save volume in power-up steady-state

	StoreVolume.key = KeyVolume; // Just to be sure, suppress flash errors
	flash_store(KeyVolume, StoreVolume.words);
	usb_write( flash_msg, (byte)2); // two-char message
}

char power_state(void)
{
	return power;
}