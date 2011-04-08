/*********************************************************************
 * Take care of 2-digit 7-segment display
 *
 * Main acess function is 'display()', which is called from ISR of Timer4.
 ********************************************************************/
#include "typedefs.h"
#include "io_cfg.h"

/* Exported globals */
byte display_cnt; // increments on every display refresh (is at 160Hz)

/* Local vars */
static byte segment_data[4];
static byte alt_display_cnt;

/* segment-table: bits [3:0] are for PORTB[3:0],
 * bits [6:4] are moved to PORTC[2:0].
 */
static rom near char segment_table[] = {
	8, 94, 34, 66, 84, 65, 1, 90, 0, 64, /* '0' - '9' */
	16, 5, 41, 6, 33, 49, /* 'A' - 'F' */
	119 /* '-' */,
	48  /* 'P' */,
	127 /* ' ' */,
	12  /* 'U' */
};

// Display refresh, called from isr on TMR4 wraparound, at 183Hz rate
// We toggle 'LEDright' (digit select), giving multiplex frequency of 91.5Hz.
void display_isr(void)
{
	byte segment, inx;
	display_cnt++;

	/* choose which segment to display */
	inx = 0;
	if (display_cnt & 0x01)
		inx++; // for even cnts, show lower (right) segment

	if (alt_display_cnt != 0)// Don't alternate... && (display_cnt & 0x80))
	{
		inx += 2;
		if (alt_display_cnt != 0xff && display_cnt == 0x80)
			alt_display_cnt--;
	}

	/* first switch LEDs off (TRIS to '1') */
	TRISB |= 0x0F;
	TRISC |= 0x07;
	
	/* switch LED select */
	LEDright = !(display_cnt & 0x01); // set IO port segment select

	segment = segment_data[inx];
	
	/* drive LED segments (selected TRIS bits to '0') */
	TRISB &= segment | 0xF0;
	TRISC &= (segment >> 4) | 0xF8;
	PORTB &= 0xF0;
	PORTC &= 0xF8;
}
	
void display_set(byte digit_hi, byte digit_lo)
{
	segment_data[0] = segment_table[digit_lo];
	segment_data[1] = segment_table[digit_hi];
	alt_display_cnt = 0;
}	

void display_set_alt( byte digit_hi, byte digit_lo, byte duration)
{
	segment_data[2] = segment_table[digit_lo];
	segment_data[3] = segment_table[digit_hi];
	alt_display_cnt = duration;
}
