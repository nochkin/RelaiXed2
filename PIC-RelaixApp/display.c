/*********************************************************************
 * Take care of 2-digit 7-segment display
 *
 * Main acess function is 'display()', which is called from ISR of Timer4.
 ********************************************************************/
#include "typedefs.h"
#include "io_cfg.h"
#include "usb_io.h"

/* Exported globals */
byte display_cnt; // increments on every display refresh (is at 160Hz)

/* Local vars */
static byte segment_hi, segment_lo;

/* segment-table: bits [3:0] are for PORTC[3:0],
 * bits [6:4] are moved to PORTB[2:0].
 */
static rom char segment_table[] = {
	8, 94, 34, 66, 84, 65, 1, 90, 0, 64, /* '0' - '9' */
	16, 5, 41, 6, 33, 49, /* 'A' - 'F' */
	119 /* '-' */,
	48 /* 'P' */,
	127 /* ' ' */
};

void display_isr(void)
{
	byte segment;
	char s[1] = {'a'};
	display_cnt++;

	/* first switch LEDs off (TRIS to '1') */
	TRISB |= 0x0F;
	TRISC |= 0x07;
	
	/* switch LED select */
	LEDright = (display_cnt & 0x01);
	segment = LEDright ? segment_lo : segment_hi;
	
	/* drive LED segments (selected TRIS bits to '0') */
	TRISB &= segment & 0x0F;
	TRISC &= (segment >> 4);
	PORTB &= 0xF0;
	PORTC &= 0xF8;
	
	// signal some activity to USB
	if (display_cnt == 0)
		usb_write(s, 1);
	
	// reset interrupt flag: await new TMR4 wrap-around
	PIR3bits.TMR4IF = 0;
}
	
void display_set(byte digit_hi, byte digit_lo)
{
	segment_hi = segment_table[digit_hi];
	segment_lo = segment_table[digit_lo];
}	

