/******************************************************************************
 * Module to receive and decode various types of IR commands
 *
 * Copyright 2011  Jos van Eijndhoven
 *****************************************************************************/

#include "typedefs.h"
#include "io_cfg.h"
#include "usb_io.h"
#include "ir_receiver.h"
#include "amp_state.h"


/***************************
 * This code is designed to decode three different IR control protocols:
 * The old/original Philips RC5, RC5X, the newer RC6, and the old Sony SIRC protocol
 * For a good overview of these, See http://www.sbprojects.com/knowledge/ir/ir.htm
 *
 * Timer0 is used for measuring delay between successive edges on the IR input.
 * The decoding is based on receiving an interrupt on each edge, and saving a
 * bit-value, where the bit-value obviously depends on the protocol,
 * which in turn is decided from the delay values.
 * TMR0 (in 8-bit mode) increments every 21.33usec.
 * At TMR0 wrap-around (5.5msec) a time-out on the IR bitstream occurred.
 *
 * If a complete 'frame' (IR bitstream for one command) has been received,
 * it is further processed/decoded in the main loop, outside of any isr.
 *
 ******************************************************************************/

typedef enum
{
	IR_unknown = 0,
	IR_rc5,
	IR_rc6,
    IR_sirc,
    IR_error
} IRprotocol;

static unsigned char nbits, nedges, nbits_ok, final_nbits;
static unsigned short ircode, final_ircode;
static IRprotocol protocol, final_protocol;

static near rom void (*protocol_handler[5])(auto byte);

// returns delay since previous call (reset) in units of Fosc/4 /256 = 21.3 usec
static byte tmr_delay(void)
{
	byte delay = TMR0L;
	TMR0L = 0; // reset timer0 and its prescaler
	nedges++;

	return delay;
}


// ir_tmr_isr is triggered when no new edge is received in reasonable time
// This terminates IR protocol reception, even if incomplete
// returns true (1) if a valid IR sequence was received
char usb_tmr_msg[] = {'I', 'R', ':', '0', '0', '0', '0'};
char ir_tmr_isr(void)
{
	char ok;

	// capture received frame in separate variables (maybe new IR bits would flow in)
	final_ircode = ircode;
	final_nbits = nbits;
	final_protocol = protocol;
	ok = final_protocol != IR_error && final_protocol != IR_unknown && nbits_ok;

	if (ok)
	{
		byte2hex( usb_tmr_msg+3, (byte)(ircode>>8));
		byte2hex( usb_tmr_msg+5, (byte)(ircode&0x00ff));
	} else
	{
		byte2hex( usb_tmr_msg+3, (byte)protocol);
		byte2hex( usb_tmr_msg+5, (byte)nbits);
		usb_tmr_msg[3] = '?';
	}
	usb_write( usb_tmr_msg, (byte)7);

	// reset to prepare for reception of next IR frame
	ircode = 0;
	nbits = 0;
	nedges = 0;
    nbits_ok = 0;
	protocol = IR_unknown;

	return ok;
}

/////////////////////////////// RC 5 ////////////////////////////////////////////
// RC5 protocol:
// T is 900us, is 42 counts
// Frame is: one,one,toggle,11xdata (5-bit address and final 6-bit command)
// 1 is sequence dark,light:  2x 900us is 2x42 counts
// 0 is sequence light,dark:  2x 900us is 2x42 counts
// 
//
// keycodes: (6 LSB bits)
// numeric 0-9 : 0x00 - 0x09
//    vol-  vol+ : 0x11 0x10
//     ch-   ch+ : 0x21 0x20
//   mute  audio : 0x0d 0xcb
//    stop  play : 0x6d 0x6e // RC5X
//         power : 0x0c
//    left right : 0x55 0x56 // RC5X
//       down up : 0x51 0x50 // RC5X
static char rc_got_half_period;
static char rc_toggle;

static void rc5_receive(auto byte delay)
{
	char new_half_delay = delay < 63; //  < 1.5 T

	if (rc_got_half_period == new_half_delay)
	{
		// OK, got two short (half-bit) delays, now reached center of bit-period
		// OR, OK, saw long (full-bit) delay just after center of earlier bit
		rc_got_half_period = 0;
		nbits++;
		ircode <<= 1;
		nbits_ok = (nbits == 12); // we didn't count the 2 S bits, but included the toggle bit
		if (!INTCON2bits.INTEDG1) // falling edge, next-half-bit is light
			ircode |= 1;
	}
	else if (!rc_got_half_period && new_half_delay)
	{
		rc_got_half_period = 1; // just reached start of new bit period: await next edge
	} else
		protocol = IR_error;
}

/////////////////////////////// RC 6 ////////////////////////////////////////////
// RC6 protocol:
// T = 444 us, is 21 counts
// frame is: leader,one,zero,zero,zero,toggle,16xdata
// leader is: 6T light, 2T dark (is 125+42)
// one is: T light, T dark      (is 21+21)
// zero is: T dark, T light
// toggle is: 2T light, 2T dark (or vice versa, switches after key release)
// First 8 data is device address, next 8 data is key code, MSB first
// frame repetition time is 240T is 106.7ms
// frame separation is  240T-58T is 80.9ms
//
// keycodes: 0-9 : 0x00 - 0x09
//    vol-  vol+ : 0x5a 0x5b  (l,r)
//     ch-   ch+ : 0x59 0x58  (d,u)
//   pause audio : 0x30 0x4e
//    stop  play : 0x31 0x2c
//         power : 0x0c
//       menu ok : 0x54 0x5c
static void rc6_receive(auto byte delay)
{
	char new_half_delay;

	if (nbits == 4 || nbits == 5)
		new_half_delay = delay < 53; // 2.5 x T
	else
		new_half_delay = delay < 30; // 1.5 x T

	if (rc_got_half_period == new_half_delay)
	{
		// OK, got two short (half-bit) delays, now reached center of bit-period
		// OR, OK, saw long (full-bit) delay just after center of earlier bit
		rc_got_half_period = 0;
		nbits++;
		ircode <<= 1;
		nbits_ok = (nbits == 21); // we have also counted bits in the 'leader'
		if (INTCON2bits.INTEDG1) // rising edge, next-half-bit is dark
			ircode |= 1;
		if (nbits == 5)
			rc_toggle = ((byte)ircode) & 0x01; // keep toggle, otherwise it is lost (shifted out)
	}
	else if (!rc_got_half_period && new_half_delay)
	{
		rc_got_half_period = 1; // just reached start of new bit period: await next edge
	} else
		protocol = IR_error;
}

// This receiver simply ignores all subsequent incoming IR edges
static void error_receive(auto byte delay)
{
	ircode = 0xffff;
}

static char usb_msg[] = {'I', 'R', '=', '0', '1', '2', '3'};
// New edge is detected on IR receiver input: A down-edge corresponds to start of 'light'
void ir_receiver_isr(void)
{
	static byte first_low;

	byte delay = tmr_delay();

	switch (nedges)
	{
	  case 1: // just entered the first 'low', must still measure its duration
		INTCONbits.TMR0IF = 0;
		INTCONbits.TMR0IE = 1; // Enable interrupt after last edge: termination
		break;
	  case 2:
		first_low = delay;
		break;
	  case 3:
		// Philips RC5 starts 900usec low, 900usec high (nominal 42 counts)
		if (first_low >= 35 && first_low <= 50 &&
	        delay >= 35 && delay <= 50) // RC5 signal
		{
			protocol = IR_rc5;
			rc_got_half_period = 0; // now on bit-center of 'S2'
			usb_msg[2] = '5';
		}
		// Philips RC5X replaces its 2nd start bit with a command-bit
		// If that bit is 0, we start with 1800usec low, 900 or 1800usec high
		else if (first_low >= 75 && first_low <= 95 && // nominal 84
	        delay >= 35 && delay <= 95) // RC5X signal
		{
			protocol = IR_rc5;
			ircode = 1; // just saw the first (raised) command bit
			rc_got_half_period = (delay < 63); // now is up-front or at-center of the toggle bit
			if (!rc_got_half_period) // already at center of toggle, account this bit
			{
				nbits = 1; // We don't count the initial command bit for standard RC5 termination
				ircode = 3; // raise both command bit and toggle bit
			}
			usb_msg[2] = 'X';
		}
		// Philips RC6 starts 2664usec low, 888usec high (nominal 125 and 42)
		else if (first_low >= 105 && first_low <= 145 &&
	             delay >= 35 && delay <= 50) // RC6 signal
		{
			protocol = IR_rc6;
			rc_got_half_period = 1; // expect only another half period to bit-center
			usb_msg[2] = '6';
		}
		// Sony SIRC starts 2400usec low, 600usec high (nominal 113 and 28)
		else if (first_low >= 100 && first_low <= 130 &&
	             delay >= 21 && delay <= 33) // RC6 signal
		{
			protocol = IR_sirc;
			usb_msg[2] = 'S';
		}
		else
		{
			usb_msg[2] = '?';
			protocol = IR_unknown;
		}

		byte2hex( usb_msg+3, first_low);
		byte2hex( usb_msg+5, delay);
		usb_write( usb_msg, (byte)7); // I observe captured values are +/-2 accurate to nominal

		break;
	  default:
		// further bits: call the proper  '<proto>_receive()' function
		protocol_handler[protocol](delay);
		break;
	}
	
	return; // just needed to set the tmr base
}

void ir_receiver_init(void)
{
	ircode = 0;
	nbits = 0;
	nedges = 0;
    nbits_ok = 0;
	protocol = IR_unknown;

	// set-up array of function pointers, for quick decoding per protocol
	protocol_handler[IR_unknown] = error_receive;
	protocol_handler[IR_rc5] = rc5_receive; //rc5_receive;
	protocol_handler[IR_rc6] = rc6_receive;
	protocol_handler[IR_sirc] = error_receive; //rc6_receive;
	protocol_handler[IR_error] = error_receive;
}

void ir_handle_code(void)
{
	byte keycode;

	switch(final_protocol)
	{
	  case IR_rc6:
		keycode = (byte)(final_ircode & 0x00ff);
		switch (keycode)
		{
		  case 0x11:  // vol down
		  case 0x59: // arrow down
		    volume_incr = -2;
			volume_update();
			break;
		  case 0x10:  // vol up
		  case 0x58: // arrow up
		    volume_incr = 2;
			volume_update();
			break;
		  case 0x21:
		  case 0x5a: // arrow left
		    channel_incr = -1;
			channel_update();
			break;
		  case 0x20:
		  case 0x5b: // arrow right
		    channel_incr = 1;
			channel_update();
			break;
		  default:
			break;
		}
	    break;

	  case IR_rc5:
		keycode = ((byte)final_ircode) & 0x003f;
		if (final_ircode & 0x1000) // 13'th bit used for RC5X
			keycode |= 0x40; // add as 7th command bit
		switch (keycode)
		{
		  case 0x11:  // vol down
		  case 0x51:  // arrow down
		    volume_incr = -2;
			volume_update();
			break;
		  case 0x10:  // vol up
		  case 0x50:  // arrow up
		    volume_incr = 2;
			volume_update();
			break;
		  case 0x21:
		    channel_incr = -1;
			channel_update();
			break;
		  case 0x20:
		    channel_incr = 1;
			channel_update();
			break;
		  default:
			break;
		}
	    break;
	  default:
	    break;
	}
}