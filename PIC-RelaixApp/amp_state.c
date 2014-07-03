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

#include <stdint.h>
#include <p18cxxx.h>
#include "io_cfg.h"
#include "amp_state.h"
#include "usb_io.h"
#include "display.h"
#include "relays.h"
#include "storage.h"
#include "dac_cntl.h"

volatile int8_t volume_incr; // modified from isr
static int8_t volume_incr_carry;

volatile int8_t channel_incr;
volatile int8_t power_incr;
volatile int8_t balance_incr;
static int8_t balance_incr_carry;

static int8_t power, muted;
static uint8_t n_channels, analog_channel;

static union store_volume {

    struct {
        uint8_t key;
        uint8_t _volume;
        int8_t _balance; // signed
        uint8_t _channel;
    };
    uint16_t words[2];
} StoreVolume;

#define master_volume	StoreVolume._volume
#define channel		StoreVolume._channel
#define master_balance	StoreVolume._balance

void amp_state_init(void) {
    volume_incr = 0;
    volume_incr_carry = 0;
    balance_incr = 0;
    balance_incr_carry = 0;
    power_incr = 0;
    channel_incr = 0;
    channel = 0;
    power = 0;
}

static uint8_t avoid_volume_tick(int8_t *tmp_l, int8_t *tmp_r, int8_t vol_l, int8_t vol_r) {
    static int8_t prev_l = 0;
    static int8_t prev_r = 0;
    uint8_t min_l, min_r, bad;

    min_l = prev_l & vol_l;
    min_r = prev_r & vol_r;

    bad = ((min_l != prev_l) && (min_l != vol_l)) ||
            ((min_r != prev_r) && (min_r != vol_r));

    prev_l = vol_l;
    prev_r = vol_r;
    *tmp_l = min_l;
    *tmp_r = min_r;

    return bad;
}

static void set_volume_balance_relays(void) {
    int8_t balance_left, balance_right;
    int8_t volume_left, volume_right;
    int8_t tmp_l, tmp_r;

    if (master_volume == 0 || channel == 0 || power == 0 || muted != 0) {
        // Don't let balance-computations result in non-zero volume
        set_relays(power, analog_channel, 0x00, 0x00);
        return;
    }

    if (isRelaixedXLR) {
        balance_right = master_balance >> 1;
        balance_left = balance_right - master_balance;
        volume_right = (int8_t) master_volume + balance_right;
        volume_left = (int8_t) master_volume + balance_left;

        if (volume_right > 64) volume_right = 64;
        if (volume_left > 64) volume_left = 64;
        if (volume_right < 0) volume_right = 0;
        if (volume_left < 0) volume_left = 0;
    } else {
         // the relaixedSE does not implement balance control
        volume_right = (int8_t) master_volume;
        volume_left = (int8_t) master_volume;
    }

	// volume relays have minimum volume (max attenuation) when driven all-0,
	// which is in the user-interface as volume==1. volume==0 has also the input switched-off
	if (volume_right > 0)
		volume_right--;
	if (volume_left > 0)
		volume_left--;
	
    if (avoid_volume_tick(&tmp_l, &tmp_r, volume_left, volume_right)) {
        uint8_t start_cnt;
        set_relays(power, analog_channel, tmp_l, tmp_r);

        start_cnt = display_cnt;
        while (display_cnt != (uint8_t)(start_cnt + 3))
            ; // wait for about 15 msec.
    }
    set_relays(power, analog_channel, volume_left, volume_right);
}

void volume_display(uint8_t override) {
    uint8_t vol_div_10, vol_by_10, vol_mod_10;

    if (muted) {
        display_set(0x00, 0x00, override);
        return;
    } else if (dac_status() == DAC_NOLOCK) {
        display_set(DIGIT_n, DIGIT_o, override);
        return;
    }

    vol_div_10 = 0;
    for (vol_by_10 = 10; vol_by_10 <= master_volume; vol_by_10 += 10)
        vol_div_10++;
    vol_mod_10 = master_volume - vol_by_10 + 10;

    display_set(vol_div_10, vol_mod_10, override);
}

void volume_update(void) {
    uint8_t vol_div_10, vol_by_10, vol_mod_10;
    char vol_msg[] = {'V', '+', '0'};
    int8_t vol_incr_abs;

    if (power < 2)
        return;

    if (muted) {
        muted = 0;
        volume_incr = 0; // first unmute before changing volume setting
    }

    volume_incr_carry += volume_incr;
    volume_incr = 0; // quick copy-and-reset: avoid intermediate interrupts

    vol_incr_abs = volume_incr_carry;
    if (volume_incr_carry < 0) {
        vol_incr_abs = -volume_incr_carry;
        vol_msg[1] = '-';
    }
    vol_msg[2] = (vol_incr_abs <= 9) ? '0' + vol_incr_abs : '*';

    // Our volume rotary switch giver two pulses per notch,
    // but we want to incr volume with 1 per notch.
    // In stead of simply div by 2, we build-in a hysteresis,
    // giving more stable behavior in case of switch spikes.
    // So, repeated +1,-1 will not cause relay activity.
    while (volume_incr_carry >= 2) {
        if (master_volume < 64)
            master_volume++;

        volume_incr_carry -= 2;
    }

    while (volume_incr_carry <= -2) {
        if (master_volume > 0)
            master_volume--;

        volume_incr_carry += 2;
    }

    if (master_volume > 64)
        master_volume = 0; // weird, flash_load error?

    set_volume_balance_relays();
    volume_display(1);
    usb_write(vol_msg, (uint8_t) 3); // three-char message
}

void balance_update(void) {
    if (power < 2 || muted || isRelaixedSE)
        return;

    // Similar carefull updating as for volume...
    balance_incr_carry += balance_incr;
    balance_incr = 0; // quick copy-and-reset: avoid intermediate interrupts

    while (balance_incr_carry >= 2) {
        if (master_balance < 9)
            master_balance++;

        balance_incr_carry -= 2;
    }

    while (balance_incr_carry <= -2) {
        if (master_balance > -9)
            master_balance--;

        balance_incr_carry += 2;
    }

    set_volume_balance_relays();

    if (master_balance >= 0) {
        display_set_alt(DIGIT_dark, master_balance, 2);
    } else {
        display_set_alt(DIGIT_minus, -master_balance, 2);
    }
}

void mute(void) {
    if (power != 2)
        return;

    muted = 1;
    set_relays(power, 0x00, 0x00, 0x00);
    display_set(0x00, 0x00, 1);
}

void unmute(void) {
    if (power != 2)
        return;

    volume_incr = 0;
    volume_update();
}

void channel_update(void) {
    char chan_msg[] = {'C', '+', '0', '=', '0'};
    int8_t chan_incr_abs;
    uint8_t dac_channel;
    uint8_t old_channel;
    uint8_t start_time;

    if (power < 2)
        return;

    old_channel = analog_channel;
    if (channel_incr >= 0) {
        chan_incr_abs = channel_incr;
    } else {
        chan_incr_abs = -channel_incr;
        chan_msg[1] = '-';
    }
    chan_msg[2] = (chan_incr_abs <= 9) ? '0' + chan_incr_abs : '*';

    channel += channel_incr;
    channel_incr = 0;

    n_channels = isRelaixedXLR ? 6 : 4; // default relaixed
    if (dac_status() != DAC_ABSENT)
        n_channels += 3; // 4 digital inputs into one analog input

    while (channel > n_channels)
        channel -= n_channels;
    if (channel == 0)
        channel = n_channels;

    // if we have a dac, it is a 4-input dac connected to analog input channel 4
    // the dac digital inputs are numbered 1-4 in the user interface
    if (dac_status() != DAC_ABSENT) {
        analog_channel = (channel <= 4) ? 4 :
                (channel <= 7) ? channel - 4 :
                channel - 3;
        dac_channel = (channel <= 4) ? channel - 1 : 4;
        dac_set_channel(dac_channel);
    } else
        analog_channel = channel;
    chan_msg[4] = '0' + analog_channel;
    usb_write(chan_msg, (uint8_t) 5);

    if (old_channel != analog_channel) {
        chan_msg[1] = 'm';
        chan_msg[2] = 'u';
        chan_msg[3] = 't';
        chan_msg[4] = 'e';
        usb_write(chan_msg, (uint8_t) 5);
        // first: do max attenuation om current input channel
        set_relays(power, old_channel, 0x00, 0x00);
        //sleep 15msec;
        start_time = display_cnt;
        while (display_cnt != (uint8_t)(start_time + 3))
            ; // display_cnt increments at 183 Hz
        // second: switch input channel while holding attenuation.
        set_relays(power, analog_channel, 0x00, 0x00);
        // Let new input DC levels settle...
        //sleep 100msec;
        while (display_cnt != (uint8_t)(start_time + 20))
            ;
    }
    // Finally re-establish volume on new input channel
    set_volume_balance_relays();
    volume_display(0);
    display_set_alt(DIGIT_C, channel, 2); // repeat channel-display twice
}

// channel_set with absolute channel number as argument, called from ir_receiver only

void channel_set(uint8_t new_ch) {
    if (dac_status() == DAC_ABSENT && new_ch > 6)
        return; // silently ignore unsupported button

    channel_incr = new_ch - channel;
    channel_update();
}

void power_update(void) {
    // maintain different power states:
    // 0: audio is muted, analog power is off, minimal stand-by power
    // 1: analog power is switched-on, audio is muted, timer running to enter next power level
    //    In relaixedSE: soft-power is on with series damping resistors
    // 2: analog power is full on, audio is on

    char chan_msg[] = {'P', '0'};

    if (power_incr == -1 && power != 0) {
        // first do audio mute...
        set_relays(power, 0x00, 0x00, 0x00);
        display_set(DIGIT_dark, DIGIT_dark, 1);
        // and follow immediatly with analog power shutdown

        // set volume-state down AFTER power-state down,
        // otherwise an interrupt-flash-tick might store a (wrong) 0 volume to flash
        power = 0;
        master_volume = 0;
        volume_incr_carry = 0;
        set_relays(0x00, 0x00, 0x00, 0x00);
    } else if (power_incr > 0 && power == 0) {
        power = 1;
        display_set(0x00, 0x00, 1);
        set_relays(power, 0x00, 0x00, 0x00);
        // Enable analog power, stay in mute
        // Later, from the 'power_tick' counter, power will be incremented to 2
    } else if (power_incr > 0 && power == 1) {
        power = 2;
        flash_load(KeyVolume, &StoreVolume.key);
        volume_incr = 0;
        channel_incr = 0;
        balance_incr = 0;
        muted = 0;
        volume_update(); // shows volume in display
        channel_update(); // shows channel in temporary display

        PIR2bits.LVDIF = 0;
        PIE2bits.LVDIE = 1; // watch power-supply level for drops
    }

    power_incr = 0;
    chan_msg[1] = '0' + power;
    usb_write(chan_msg, (uint8_t) 2); // two-char message
}

void flash_volume_channel(void) {
    char flash_msg[] = {'F', 'v'};

    if (power != 2)
        return; // Only save volume in power-up steady-state

    StoreVolume.key = KeyVolume; // Just to be sure, suppress flash errors
    flash_store(KeyVolume, StoreVolume.words);
    usb_write(flash_msg, (uint8_t) 2); // two-char message
}

uint8_t power_state(void) {
    return power;
}