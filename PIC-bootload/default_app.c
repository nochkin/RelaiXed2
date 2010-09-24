/******************************************************************************
 Default (dummy) application above bootloader
 Its 'sleep' will get interrupted for USB events.
 
 Jos van Eijndhoven
 September 2010
 *****************************************************************************/
#include <p18cxxx.h>
#include "typedefs.h"                   
#include "io_cfg.h"

void app_isr_high(void);
void app_isr_low(void);
void app_main(void);
void check_usb_power(void);

#pragma code app_start=0x2000
void app_start(void)
{
	_asm goto app_main _endasm	
}

// high-priority interrupt vector:
#pragma code app_isr_hi=0x2008
void app_interrupt_at_high_vector(void)
{
	_asm goto app_isr_high _endasm	
}

// low-priority interrupt vector:
#pragma code app_isr_lo=0x2018
void app_interrupt_at_low_vector(void)
{
	_asm goto app_isr_low _endasm
}


#pragma code page=0x2020
void app_main(void)
{
	unsigned int i;
    mLED_7_On();

	while(1)
	{
		for (i=0; i<64000U; i++)
			UIE = 0x3F; // hack: reactivate most USB interrupts
		mLED_7_Toggle();
		check_usb_power();
	}	
}

void check_usb_power(void)
{
	static BOOL prev_usb_bus_sense = 0;
	
	if (usb_bus_sense != prev_usb_bus_sense)
	{
		PIR2bits.USBIF = 1; // enter USB code through interrupt
		prev_usb_bus_sense = usb_bus_sense;
	}	
}	

// we do not expect to receive interrupts in this app...
#pragma interruptlow app_isr_low
void app_isr_low(void)
{
	     //mLED_7_On();
}

#pragma interrupt app_isr_high
void app_isr_high(void)
{
	     //mLED_7_On();
}
