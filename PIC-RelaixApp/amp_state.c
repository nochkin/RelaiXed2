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
 
volatile char volume_incr; // modified from isr
static char volume_incr_carry;

volatile char channel_incr;
volatile char power_incr;

static char power;

static union store_volume
{
	struct
	{
		StorageKey key;
		byte	_volume;
		byte	_balance;
		byte	_channel;
	};
	unsigned int words[2];
} StoreVolume;

#define master_volume	StoreVolume._volume
#define channel			StoreVolume._channel
#define balance			StoreVolume._balance

// Input selection channels: numbered 0 .. 5
#define NCHANNELS 6

void amp_state_init(void)
{
	volume_incr = 0;
    volume_incr_carry = 0;
	power_incr = 0;
	channel_incr = 0;
	channel = 0;
	power = 0;
}	
 
void volume_update(void)
{
	byte vol_div_10, vol_by_10, vol_mod_10;
	char vol_msg[] = {'V', '+', '0'};
	char vol_incr_abs;

	if (power < 2)
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

	if (master_volume > 64)
		master_volume = 0; // weird, flash_load error?

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

	while (channel > 6)
		channel -= 6;
	if (channel == 0)
		channel = 1;

	set_relays(0x00, power, channel, master_volume, master_volume);

	display_set_alt( DIGIT_C, channel, 2); // repeat channel-display twice

	usb_write( chan_msg, (byte)3); // three-char message
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
		display_set( DIGIT_dark, DIGIT_dark);
		// and follow immediatly with analog power shutdown

        // set volume-state down AFTER power-state down,
        // otherwise an interrupt-flash-tick might store a (wrong) 0 volume to flash
		power = 0;
		power_incr = 0;
		master_volume = 0; 
		volume_incr_carry = 0;
		set_relays(0x00, 0x00, 0x00, 0x00, 0x00);
	} else if (power_incr > 0 && power == 0)
	{
		power = 1;
		display_set( 0x00, 0x00);
		set_relays(0x00, power, 0x00, 0x00, 0x00);
		// Enable analog power, stay in mute
		// Later, from the 'power_tick' counter, power will be incremented to 2
	} else if (power_incr > 0 && power == 1)
	{
		power = 2;
		flash_load(KeyVolume, &StoreVolume.key);
		if (master_volume > 64)
			master_volume = 0;
		volume_incr = 0;
		channel_incr = 0;
		volume_update();
		channel_update();

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