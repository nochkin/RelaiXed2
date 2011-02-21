/*********************************************************************
 *
 *   Interrupt Service Routines for use with Microchip's
 *   HID bootloader and USB stack
 *
 *********************************************************************
 * FileName:        isr.c
 * Processor:       PIC18F..K..
 * Compiler:        C18 Lite
 * Author:          Jos van Eijndhoven
 *
 * THIS SOFTWARE IS PROVIDED IN AN 'AS IS' CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE AUTHOR,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 * File Version  Date		Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 1.0			 07/03/2010	Original Version.  
 ********************************************************************/
#include <p18cxxx.h>
#include "usb.h"
#include "Boot46J50Family.h"
#include "io_cfg.h"

void USBSubSystem(void);
void BootLoadIsr(void);

extern byte usb_active_cfg;

// high-priority interrupt vector:
// or global interrupt vector in case IPEN is not set
#pragma code high_vector=0x08
void interrupt_at_high_vector(void)
{
#ifdef UseIPEN
	// with IPEN, high-priority is only used by the uploaded application
	_asm goto ProgramMemStart+0x0008 _endasm
#else
	_asm goto BootLoadIsr _endasm
#endif
}

// low-priority interrupt vector: (not used if IPEN is clear)
#pragma code low_vector=0x18
void interrupt_at_low_vector(void)
{
	_asm goto BootLoadIsr _endasm
}

/******************************************************************************
 * The USB subsystem handles the USB stack and the USB-related 'kernel' applications such as the bootloader
 * The 'bootload' code was originally in a while(1) loop in main,
 * but is now activated through the USBIF interrupt
 *****************************************************************************/
#pragma code
#ifdef UseIPEN
#pragma interruptlow BootLoadIsr
#else
#pragma interrupt BootLoadIsr
#endif
void BootLoadIsr(void)
{
	// The PIC hardware will clear the 'GIEL' bit upon entering this isr.
	if (PIR2bits.USBIF && PIE2bits.USBIE)
	{
		USBSubSystem();
	}
	else
	{
    	_asm goto ProgramMemStart+0x0018 _endasm
    	// return-from-isr is done in application program
 	}
 	// The 'return from interruptlow' instruction will set the GIEL bit again
}

void USBSubSystem(void)
{
	extern unsigned char BootState;
	
	PIE2bits.USBIE = 0;
	PIR2bits.USBIF = 0;        
    // Hmm.. not nice :-(
    // The USBCheckBusStatus() requires polling during USB power-up.
    // Maybe this busy-wait is acceptable because USB-attach is rare, and we are in a low-priority isr.
    do
    {   USBCheckBusStatus(); // Must use polling method
    } while (usb_device_state == ATTACHED_STATE);
        // Must use polling method
		// Hope to achieve POWERED_STATE. on fail, reach DETACHED_STATE
	
	if (usb_device_state > ATTACHED_STATE)
	{
		// UIE = 0xff; // allow all USB interrupts
    	USBDriverService(); // Interrupt or polling method
 	}   	
	else
	{
		UIE = 0x00; // disable all USB interrupts
		UIEbits.ACTVIE = 1; // but allow wake-up
		// UIR = 0x00;
 	}
 	TXADDRL = usb_device_state;	// JvE: HACK for observability
	UIE = 0x3D;
	PIE2bits.USBIE = 1; // restore
  	
	if((usb_device_state == CONFIGURED_STATE) && (UCONbits.SUSPND != 1) &&
		BootState == 0x00 && !mHIDRxIsBusy() && usb_active_cfg == 1)
	{
		// Enter here if there was data for the bootloader endpoint,
		// AND we are not already in here with merely yet another interrupt
		// usb_active_cfg == 1 means we have a connection with the PC bootload app.
		
		// Also this is not really in the style of an isr :-(
		// We will not return to the user application anymore.
		// ProcessBootLoad behaves like an application itself, and will end with a device reset.
 	    ProcessBootLoad();
 	}
}