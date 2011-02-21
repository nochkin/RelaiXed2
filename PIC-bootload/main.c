/*********************************************************************
 *
 *   Microchip USB HID Bootloader v1.01 for PIC18F46J50 Family Devices
 *
 *********************************************************************
 * FileName:        main.c
 * Dependencies:    See INCLUDES section below
 * Processor:       PIC18
 * Compiler:        C18 3.22+
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the “Company”) for its PICmicro® Microcontroller is intended and
 * supplied to you, the Company’s customer, for use solely and
 * exclusively on Microchip PICmicro Microcontroller products. The
 * software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 * File Version  Date		Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 1.0			 04/09/2008	Started from MCHPFSUSB v1.3 HID Mouse
 *							demo project.  Main modifications include
 *							changes to HID report descriptor, and
 *							replacement of the "user_mouse.c" file
 *							with the contents contained in the
 *							Boot46J50Family.c file.
 * 1.01			01/23/2009	Minor modifications.  Added firmware only
 *							entry point (goto 0x001C)
 * 1.02			07/06/2009  Slight change to report descriptor.
 ********************************************************************/

/*********************************************************************
IMPORTANT NOTE: This code is currently configured to work with the
PIC18F46J50 FS USB Demo Board.  It can be readily adapted to
work with other members of the PIC18F46J50 Family of USB microcontrollers 
as well (PIC18F24J50/25J50/26J50/44J50/45J50/46J50).

To do so, modify the linker script for the appropriate FILES includes,
and the new memory ranges (assuming a different memory size device), and
click "Configure --> Select Device" and select the proper
microcontroller.  Also double check to verify that the io_cfg.h and
usbcfg.h are properly configured to match your desired application
platform.

It is also recommended to configure the default I/O pin usage in this code.
See the InitializeSystem() function.

This code is meant to be compiled with the C18 compiler version 3.22+
with all optimizations turned on.  If some of the optimizations are not
enabled, the total code size may grow to exceed the 0x000-0xFFF memory
region this code is designed to occupy.  In this case, linker errors will
occur.  If this happens, the vector remapping in the _entry() function 
will have to be modified, as will the application firmware projects that 
may get programmed using this bootloader firmware (to have an entry
point higher than 0x1000).  Additionally, the linker script in this
project will have to be modified to make the BootPage section larger.
*********************************************************************/


//----------------------------------------------------
//Usage tips for this HID USB bootloader firwmare
//----------------------------------------------------

//To enter this bootloader firmware, hold the RB2 I/O pin low at power
//up or after a reset.  Alternatively, application firmware may enter
//the bootloader firmware by clearing the INTCON<GIE> bit and then
//executing an "_asm goto 0x001C _endasm" instruction.

//If a high priority interrupt occurs, the PC will jump to 0x1008
//If a low priority interrupt occurs, the PC will jump to 0x1018

//If RB2 is high at power up/after reset, this code will jump to
//the application firmware, instead of staying in this bootloader firmware.
//The start of the application firmware should be at 0x1000
//In other words, when developing the application firmware which will be
//programmed with this bootloader, place the following in the code, if
//it is a C18 based project:

//extern void _startup (void);    // See c018i.c in your C18 compiler dir
//#pragma code AppFirmwareStartLocation = 0x1000
//void _reset (void)
//{
//    _asm goto _startup _endasm
//}

//Build the application project with a linker script that marks
//the address range 0x000-0xFFF as "PROTECTED".  This is the program
//memory region that this bootloader is currently configured to occupy.

//Although the bootloader can re-program the program memory page that
//contains the configuration bits (the last page of implemented flash)
//it is not always preferrable to do so in case a user attempts to
//program a hex file with configuration bit settings that are not compatible
//with USB operation.  This would prevent further entry into the bootloader.
//If the bootloader will not be used to program the configuration
//words page, the application firmware's linker script should mark
//the entire page as PROTECTED.



/** I N C L U D E S **********************************************************/
#include <p18cxxx.h>
#include "typedefs.h"                   
#include "usb.h"                         
#include "io_cfg.h"                     
#include "Boot46J50Family.h"

/** C O N F I G U R A T I O N ************************************************/
// Note: For a complete list of the available config pragmas and their values, 
// see the compiler documentation, and/or click "Help --> Topics..." and then 
// select "PIC18 Config Settings" in the Language Tools section.

#if defined(PIC18F46J50_PIM)
     #pragma config WDTEN = OFF          //WDT disabled (enabled by SWDTEN bit)
     #pragma config PLLDIV = 3           //Divide by 3 (12 MHz oscillator input)
     #pragma config STVREN = ON            //stack overflow/underflow reset enabled
     #pragma config XINST = OFF          //Extended instruction set disabled
     #pragma config CPUDIV = OSC1        //No CPU system clock divide
     #pragma config CP0 = OFF            //Program memory is not code-protected
     #pragma config OSC = HSPLL          //HS oscillator, PLL enabled, HSPLL used by USB
     #pragma config T1DIG = OFF          //Sec Osc clock source may not be selected, unless T1OSCEN = 1
     #pragma config LPT1OSC = OFF        //high power Timer1 mode
     #pragma config FCMEN = OFF          //Fail-Safe Clock Monitor disabled
     #pragma config IESO = OFF           //Two-Speed Start-up disabled
     #pragma config WDTPS = 32768        //1:32768
     #pragma config DSWDTOSC = INTOSCREF //DSWDT uses INTOSC/INTRC as clock
     #pragma config RTCOSC = T1OSCREF    //RTCC uses T1OSC/T1CKI as clock
     #pragma config DSBOREN = OFF        //Zero-Power BOR disabled in Deep Sleep
     #pragma config DSWDTEN = OFF        //Disabled
     #pragma config DSWDTPS = 8192       //1:8,192 (8.5 seconds)
     #pragma config IOL1WAY = OFF        //IOLOCK bit can be set and cleared
     #pragma config MSSP7B_EN = MSK7     //7 Bit address masking
     #pragma config WPFP = PAGE_1        //Write Protect Program Flash Page 0
     #pragma config WPEND = PAGE_0       //Start protection at page 0
     #pragma config WPCFG = OFF          //Write/Erase last page protect Disabled
     #pragma config WPDIS = OFF          //WPFP[5:0], WPEND, and WPCFG bits ignored 
//If using the YOUR_BOARD hardware platform (see usbcfg.h), uncomment below and add pragmas
#elif defined(RELAIXED2)
	 //Add the configuration pragmas here for your hardware platform
     #pragma config WDTEN = OFF          //WDT disabled (enabled by SWDTEN bit)
     #pragma config PLLDIV = 3           //Divide by 3 (12 MHz oscillator input)
     #pragma config STVREN = ON          //stack overflow/underflow reset enabled
     #pragma config XINST = OFF          //Extended instruction set disabled
     #pragma config CPUDIV = OSC1        //No CPU system clock divide
     #pragma config CP0 = OFF            //Program memory is not code-protected
     #pragma config OSC = HSPLL          //HS oscillator, PLL enabled, HSPLL used by USB
     #pragma config T1DIG = OFF          //Sec Osc clock source may not be selected, unless T1OSCEN = 1
     #pragma config LPT1OSC = OFF        //high power Timer1 mode
     #pragma config FCMEN = OFF          //Fail-Safe Clock Monitor disabled
     #pragma config IESO = OFF           //Two-Speed Start-up disabled
     #pragma config WDTPS = 32768        //1:32768
     #pragma config DSWDTOSC = INTOSCREF //DSWDT uses INTOSC/INTRC as clock
     #pragma config RTCOSC = T1OSCREF    //RTCC uses T1OSC/T1CKI as clock
     #pragma config DSBOREN = OFF        //Zero-Power BOR disabled in Deep Sleep
     #pragma config DSWDTEN = OFF        //Disabled
     #pragma config DSWDTPS = 8192       //1:8,192 (8.5 seconds)
     #pragma config IOL1WAY = OFF        //IOLOCK bit can be set and cleared
     #pragma config MSSP7B_EN = MSK7     //7 Bit address masking
     #pragma config WPFP = PAGE_1        //Write Protect Program Flash Page 0
     #pragma config WPEND = PAGE_0       //Start protection at page 0
     #pragma config WPCFG = OFF          //Write/Erase last page protect Disabled
     #pragma config WPDIS = OFF          //WPFP[5:0], WPEND, and WPCFG bits ignored 

#else
	#error Not a supported board (yet), make sure the proper board is selected in usbcfg.h, and if so, set configuration bits in __FILE__, line __LINE__
#endif

/** V A R I A B L E S ********************************************************/
#pragma udata
word led_count;
unsigned int pll_startup_counter;	//Used for software delay while pll is starting up


/** P R I V A T E  P R O T O T Y P E S ***************************************/
static void InitializeSystem(void);
void USBTasks(void);
void BlinkUSBStatus(void);

//externs
extern void LongDelay(void);


/** D E C L A R A T I O N S **************************************************/
#pragma code
void main(void)
{   
    InitializeSystem();
    USBTasks(); // check for first USB stuff
    
    UIE = 0x3f;          // allow sensible USB interrupts
	PIE2bits.USBIE  = 1; // allow USB interrupts
#ifdef UseIPEN
	INTCONbits.GIEL = 1; // allow all low-priority interrupts (among which the USB interrupts)
	INTCONbits.GIEH = 1; // also allow high-priority interrupts (prerequisite for low-priority ints)
#else
	INTCONbits.GIE = 1;  // Allow all interrupts
	INTCONbits.PEIE = 1; // including the USB device interrupts
#endif
	_asm
		goto ProgramMemStart			// Assume the user app has its own main loop.
	_endasm
	// we will never return here
}//end main


/******************************************************************************
 * Function:        static void InitializeSystem(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        InitializeSystem is a centralize initialization routine.
 *                  All required USB initialization routines are called from
 *                  here.
 *
 *                  User application initialization routine should also be
 *                  called from here.                  
 *
 * Note:            None
 *****************************************************************************/
static void InitializeSystem(void)
{
	OSCCON = 0x60;	//Clock switch to primary clock source.  May not have been running
					//from this if the bootloader is called from the application firmware.

	
	//On the PIC18F46J50 Family of USB microcontrollers, the PLL will not power up and be enabled
	//by default, even if a PLL enabled oscillator configuration is selected (such as HS+PLL).
	//This allows the device to power up at a lower initial operating frequency, which can be
	//advantageous when powered from a source which is not gauranteed to be adequate for 48MHz
	//operation.  On these devices, user firmware needs to manually set the OSCTUNE<PLLEN> bit to
	//power up the PLL.
	#if defined(__18F24J50)||defined(__18F25J50)|| \
    	defined(__18F26J50)||defined(__18F44J50)|| \
    	defined(__18F45J50)||defined(__18F46J50) 

    OSCTUNEbits.PLLEN = 1;  //Enable the PLL and wait 2+ms until the PLL locks before enabling USB module
    pll_startup_counter = 600;
    while(pll_startup_counter--);
    //Device switches over automatically to PLL output after PLL is locked and ready.

    #else
        #error Double Click this message.  Please make sure the InitializeSystem() function correctly configures your hardware platform.  
		//Also make sure the correct board is selected in usbcfg.h.  If 
		//everything is correct, comment out the above "#error ..." line
		//to suppress the error message.
    #endif

	mInitAllLEDs();

	//USB module may have already been on if the application firmware calls the bootloader
	//without first disabling the USB module.  If this happens, need
	//to temporarily soft-detach from the host, wait a delay (allows cable capacitance
	//to discharge, and to allow host software to recognize detach), then
	//re-enable the USB module, so the host knows to re-enumerate the
	//USB device.
	if(UCONbits.USBEN == 1)
	{
		UCONbits.SUSPND = 0;
		UCON = 0;
		LongDelay();
	}	


//	The USB specifications require that USB peripheral devices must never source
//	current onto the Vbus pin.  Additionally, USB peripherals should not source
//	current on D+ or D- when the host/hub is not actively powering the Vbus line.
//	When designing a self powered (as opposed to bus powered) USB peripheral
//	device, the firmware should make sure not to turn on the USB module and D+
//	or D- pull up resistor unless Vbus is actively powered.  Therefore, the
//	firmware needs some means to detect when Vbus is being powered by the host.
//	A 5V tolerant I/O pin can be connected to Vbus (through a resistor), and
// 	can be used to detect when Vbus is high (host actively powering), or low
//	(host is shut down or otherwise not supplying power).  The USB firmware
// 	can then periodically poll this I/O pin to know when it is okay to turn on
//	the USB module/D+/D- pull up resistor.  When designing a purely bus powered
//	peripheral device, it is not possible to source current on D+ or D- when the
//	host is not actively providing power on Vbus. Therefore, implementing this
//	bus sense feature is optional.  This firmware can be made to use this bus
//	sense feature by making sure "USE_USB_BUS_SENSE_IO" has been defined in the
//	usbcfg.h file.    
    #if defined(USE_USB_BUS_SENSE_IO)
    tris_usb_bus_sense = INPUT_PIN; // See io_cfg.h
    #endif

//	If the host PC sends a GetStatus (device) request, the firmware must respond
//	and let the host know if the USB peripheral device is currently bus powered
//	or self powered.  See chapter 9 in the official USB specifications for details
//	regarding this request.  If the peripheral device is capable of being both
//	self and bus powered, it should not return a hard coded value for this request.
//	Instead, firmware should check if it is currently self or bus powered, and
//	respond accordingly.  If the hardware has been configured like demonstrated
//	on the PICDEM FS USB Demo Board, an I/O pin can be polled to determine the
//	currently selected power source.  On the PICDEM FS USB Demo Board, "RA2" 
//	is used for	this purpose.  If using this feature, make sure "USE_SELF_POWER_SENSE_IO"
//	has been defined in usbcfg.h, and that an appropriate I/O pin has been mapped
//	to it in io_cfg.h.    
    #if defined(USE_SELF_POWER_SENSE_IO)
    tris_self_power = INPUT_PIN;
    #endif
    
// JvE: Hmmm... I would like to use the IPEN mode, but it seems to me impossible
// to use that reliably, because the device has just a single-entry 'fast return stack'.
    RCONbits.IPEN  = IPEN_value; // enable the interrupt priority feature
    IPR2bits.USBIP = 0; // low priority USB interrupts
	PIE2bits.USBIE = 0; // not yet allow USB interrupts
	
	// PORTA: all inputs, no interrupt-on-change facility :-(...
	PORTA = 0; TRISA = 0xFF; // A all inputs
	ADCON0 = 0; ANCON0 = 0xFF; ANCON1 = 0x1F; // disable AD converter, all digital IO
	
	// PORTB: use all-input config, use open-drain output style
	// The I2C pins must also be configured in input mode.
	PORTB = 0; TRISB = 0xFF; // B all inputs
	INTCON2bits.RBPU = 0; // use weak pull-up for RB67
	
	// PORTC: all input except 7
	PORTC = 0x80; TRISC = 0x7F;
	
	// setup I2C peripheral (not really needed for the bootloader operation)
	SSP1CON1 = 0x28; // enable I2C master mode
	SSP1ADD = 0x63; // 100kHz bitrate from 40MHz system clock
	
   	mInitializeUSBDriver();         // See usbdrv.h

    UserInit();                     // See Boot46J50Family.c.  Initializes the bootloader firmware state machine variables.

	led_count = 0;			//Initialize variable used to toggle LEDs
}//end InitializeSystem

/******************************************************************************
 * Function:        void USBTasks(void)
 *
 * PreCondition:    InitializeSystem has been called.
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Service loop for USB tasks.
 *
 * Note:            None
 *****************************************************************************/
void USBTasks(void)
{
    /*
     * Servicing Hardware
     */
    do
    {   USBCheckBusStatus();                // Must use polling method
    } while (usb_device_state == 1 && usb_bus_sense);
    USBDriverService();              	    // Interrupt or polling method
    TXADDRL = usb_device_state;	


}// end USBTasks

/** EOF main.c ***************************************************************/
