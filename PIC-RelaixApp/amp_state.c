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
 
volatile char volume_incr; // modified from isr
static char master_volume;
static char volume_incr_carry;

volatile char channel_incr;
volatile char power_down_button;
static byte channel;

static char power;

// Input selection channels: numbered 0 .. 5
#define NCHANNELS 6

void amp_state_init(void)
{
	volume_incr = 0;
    volume_incr_carry = 0;
	master_volume = 0;
	power_down_button = 0;
	channel = 1;
	power = 1;
}	
 
void volume_update(void)
{
	byte vol_div_10, vol_by_10, vol_mod_10;
	char vol_msg[] = {'V', '+', '0'};
	char vol_incr_abs;

	if (!power)
		return;

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

	set_relays(0x00, power, channel, master_volume, master_volume);
	
	vol_div_10 = 0;
	for (vol_by_10 = 10; vol_by_10 <= master_volume; vol_by_10 += 10)
		vol_div_10++;
	vol_mod_10 = master_volume - vol_by_10 + 10;

	display_set( vol_div_10, vol_mod_10);

	usb_write( vol_msg, (byte)3); // three-char message
}

void channel_update(void)
{
	char chan_msg[] = {'C', '+', '0'};
	char chan_incr_abs;

	if (!power)
	{
		// first enable power, don't modify channel
		power_update();
		channel_incr = 0;
	}

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

	while (channel > 6)
		channel -= 6;

	set_relays(0x00, power, channel, master_volume, master_volume);

	display_set_alt( DIGIT_C, channel, 2); // repeat channel-display twice

	usb_write( chan_msg, (byte)3); // three-char message
}

void power_update(void)
{
	char chan_msg[] = {'P', '0'};

	if (power_down_button && power)
	{
		master_volume = 0;
		volume_incr_carry = 0;
		set_relays(0x00, power, channel, master_volume, master_volume);
		display_set( DIGIT_dark, DIGIT_dark);
		power = 0;
		power_down_button = 0;
		set_relays(0x00, power, channel, master_volume, master_volume);
	} else if (!power)
	{
		power = 1;
		set_relays(0x00, power, channel, master_volume, master_volume);
	}

	chan_msg[1] = power ? '1' : '0';
	usb_write( chan_msg, (byte)2); // two-char message
}