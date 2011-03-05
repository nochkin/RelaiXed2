/*
 * Code to keep track of amplifier state variables
 */
 
#include "typedefs.h"
#include <p18cxxx.h>
#include "io_cfg.h"
#include "amp_state.h"
#include "usb_io.h"
#include "display.h"
 
volatile char volume_incr; // modified from isr
static char master_volume;

void amp_state_init(void)
{
	volume_incr = 0;
	master_volume = 0;
}	
 
void volume_update(void)
{
	byte vol_div_10, vol_by_10, vol_mod_10;
	char vol_msg[] = {'V', '+', '0'};
	char vol_incr_abs = (volume_incr >= 0) ? volume_incr : -volume_incr;
    vol_msg[2] = (vol_incr_abs <= 9) ? '0' + vol_incr_abs : '*';
	if (volume_incr < 0) vol_msg[1] = '-';	
	
	master_volume += volume_incr;
	volume_incr = 0; // small risc of losing an interrupt result here

	if (master_volume < 0) master_volume = 0;
	else if (master_volume > 64) master_volume = 64;
	
	vol_div_10 = 0;
	for (vol_by_10 = 10; vol_by_10 <= master_volume; vol_by_10 += 10)
		vol_div_10++;
	vol_mod_10 = master_volume - vol_by_10 + 10;

	display_set( vol_div_10, vol_mod_10);

	usb_write( vol_msg, (byte)3);
}	