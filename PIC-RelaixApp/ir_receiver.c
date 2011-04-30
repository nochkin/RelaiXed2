/******************************************************************************
 * Module to receive and decode various types of IR commands
 *
 * Copyright 2011  Jos van Eijndhoven
 *****************************************************************************/

#include "typedefs.h"
#include "io_cfg.h"
#include "usb_io.h"
#include "ir_receiver.h"

// Timer0 OK for use, 8-bit prescale, 8- or 16-bit counter, sets TMR0IF
// 12MHz / (prescale=256) =  46.875Hz or 21.3333 usec
// 16-bit timer -> overflow after 1.4 sec.
// 8-bit timer -> overflow after 5.5 msec.

typedef enum
{
	IR_unknown = 0,
	IR_rc5,
	IR_rc6,
    IR_sirc
} IRprotocol;

static unsigned char ircode, nbits, nedges;
static IRprotocol protocol;

// returns delay since previous call (reset) in units of Fosc/4 /256 = 21.3 usec
static byte tmr_delay(void)
{
	byte delay = TMR0L;
	TMR0L = 0; // reset timer0 and its prescaler
	nedges++;

	return delay;
}

void ir_receiver_init(void)
{
	ircode = 0;
	nbits = 0;
	nedges = 0;
}

// ir_tmr_isr is triggered when no new edge is received in reasonable time
// This terminates IR protocol reception, even if incomplete
void ir_tmr_isr(void)
{
	char usb_msg[] = {'I', 'R', '.', '0', '0'};
	byte2hex( usb_msg+3, nedges);
	// observe for SIRC: edges=26, RC5:edges=24, RC6: edges alternate between 36 and 38
	ircode = 0;
	nbits = 0;
	nedges = 0;

	usb_write( usb_msg, (byte)5);
}

static char usb_msg[] = {'I', 'R', '=', '0', '1', '2', '3'};

// New edge is detected on IR receiver input
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
			usb_msg[2] = '5';
		}
		// Philips RC6 starts 2664usec low, 888usec high (nominal 125 and 42)
		else if (first_low >= 105 && first_low <= 145 &&
	             delay >= 35 && delay <= 50) // RC6 signal
		{
			protocol = IR_rc6;
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
		break;
	}
	
	return; // just needed to set the tmr base
}
