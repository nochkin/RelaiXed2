#include <p18cxxx.h>
#include "typedefs.h"                   
#include "io_cfg.h"                     

// JvE: pragmas for bootloader addressing made
// according to MPASM&MPLINK manual, chapter 13.5

extern void _startup(void);

#pragma code _RESET_INTERRRUPT_VECTOR = 0x002000
// hmmm.. gives error: #pragma code _entry_scn = 0x002000
// unfortunately, _entry_scn is mapped to 0x0000 in c018i.c
void _reset(void)
{
	_asm goto _startup _endasm
}

#pragma code _HIGH_INTERRUPT_VECTOR = 0x002008
void _high_ISR(void)
{
	mLED_4_On();
}
	
#pragma code _LOW_INTERRUPT_VECTOR = 0x002018
void _low_ISR(void)
{
	mLED_4_On();
}

#pragma code //usercode=0x002000
void main(void)
{
	static word cnt = 0;
	mInitAllLEDs();

	while (1)
	{
		if (cnt == 0)
		{
			cnt = 60000;
			if (mLED_1)
			{
				mLED_2_On();
				mLED_1_Off();
			} else if (mLED_2)
			{
				mLED_3_On();
				mLED_2_Off();
			} else if (mLED_3)
			{
				mLED_4_On();
				mLED_3_Off();
			} else if (mLED_4)
			{
				mLED_1_On();
				mLED_4_Off();
			} else
				mLED_1_On();
		} else
			cnt--;		
	}	
}	