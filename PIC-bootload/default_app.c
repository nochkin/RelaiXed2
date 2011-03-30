/******************************************************************************
 Default (dummy) application above bootloader
 Its 'sleep' will get interrupted for USB events.
 
 Jos van Eijndhoven
 September 2010
 *****************************************************************************/
#include <p18cxxx.h>
#include "typedefs.h"                   
#include "io_cfg.h"
#include "usb.h"

void app_isr_high(void);
void app_isr_low(void);
void app_main(void);
void check_usb_power(void);
void try_USB_log(void);
extern void HIDTxReport(char *buffer, byte len);

#pragma code app_start=0x2000
void app_start(void)
{
	_asm goto app_main _endasm	
}

// high-priority interrupt vector: (or global vector if IPEN is clear)
#pragma code app_isr_hi=0x2008
void app_interrupt_at_high_vector(void)
{
	_asm goto app_isr_high _endasm	
}

// low-priority interrupt vector: (unused if IPEN is clear)
#pragma code app_isr_lo=0x2018
void app_interrupt_at_low_vector(void)
{
	_asm goto app_isr_low _endasm
}

#pragma code page=0x2020
void app_main(void)
{
	unsigned int i;
	byte j;

	// Test to see if interrupts to an application still work properly:
	// setup Timer4 for display refresh: 2^16 downscale from Fosc/4 is 153Hz
	T4CON = 0xFF; // timer4 on, 16x prescale, 16x postscale
	IPR3bits.TMR4IP = 1; // high priority interrupt
	PIE3bits.TMR4IE = 1;

	while(1)
	{
		for (i=0; i<64000U; i++)
			;
		j += 1;
		if (j > 5) j = 0;
		
		switch (j)
		{
			case 0: mLED_6_On(); mLED_4_Off(); break;
			case 1: mLED_1_On(); mLED_5_Off(); break;
			case 2: mLED_2_On(); mLED_6_Off(); break;
			case 3: mLED_3_On(); mLED_1_Off(); break;
			case 4: mLED_4_On(); mLED_2_Off(); break;
			case 5: mLED_5_On(); mLED_3_Off();
		}
		
		if (j==0 && UCONbits.USBEN && mGetLogMode)
			try_USB_log();
		
		/* show USB status */
		if (UCONbits.USBEN)
		{
			if (mGetLogMode)
				mLED_7_Toggle()
			else
				mLED_7_On()
		} else
			mLED_7_Off()

		check_usb_power();
	}
}

void try_USB_log( void)
{
	char command[64] = {0x09 /* command = LOG_DEVICE */ ,
	                    2 /* length of payload string */,
	                    'a', '.',
	                    0 /* padding */};
	                    
	if (!mHIDTxIsBusy())
		HIDTxReport(command, 64);
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
}

#define LEDright PORTCbits.RC7

#pragma interrupt app_isr_high
void app_isr_high(void)
{
	if (PIE3bits.TMR4IE && PIR3bits.TMR4IF)
	{   // multiplex both 7-segment displays
		LEDright = !LEDright;
		PIR3bits.TMR4IF = 0;
	}
}
