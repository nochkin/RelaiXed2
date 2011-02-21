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


#include <p18cxxx.h>
#include "typedefs.h"                   
#include "io_cfg.h"
#include "display.h"
#include "usb_io.h"
#include "amp_state.h"

// Forward declarations
void app_isr_high(void);
void app_isr_low(void);
void main(void);
void check_usb_power(void);
static void init(void);               

#ifdef __DEBUG
// Don't use my bootloader USB stack: the MPLAB debugger doesn't understand that.
// This debug image can be uploaded by the debugger/programmer
#define ISR_HI 0x0008
#define ISR_LO 0x0018
#else
// Normal operation. Built hex image should be inserted by bootloader
#define ISR_HI 0x2008
#define ISR_LO 0x2018

// JvE: pragmas for bootloader addressing made
// according to MPASM&MPLINK manual, chapter 13.5
// The 'reset' jumps into the real 'main', not a device init function.
#pragma code app_start=0x2000
void app_start(void)
{
	_asm goto main _endasm	
}
#endif

// high-priority interrupt vector: (global interrupt vector if IPEN is clear)
#pragma code app_isr_hi = ISR_HI
void app_interrupt_at_high_vector(void)
{
	_asm goto app_isr_high _endasm	
}

// low-priority interrupt vector: (not used if IPEN is clear)
#pragma code app_isr_lo = ISR_LO
void app_interrupt_at_low_vector(void)
{
#ifdef UseIPEN
	_asm goto app_isr_low _endasm
#else
	_asm goto app_isr_high _endasm	
#endif
}

BOOL prev_usb_bus_sense;

#pragma code
void main(void)
{
	unsigned int i;
	char usb_char;
	const char init_msg[] = {'I', 'N', 'I', 'T'};
	
	init();
	display_cnt = 0;
	display_set( 0x00, 0x00);
	display_set_alt( 0x00, 0x00, 0x00);
	amp_state_init();

	// Globally enable interrupts
#ifdef UseIPEN
	INTCONbits.GIEH = 1;
	INTCONbits.GIEL = 1;
#else
	INTCONbits.PEIE = 1;
	INTCONbits.GIE = 1;
#endif

	// (re-)launch USB activity
	prev_usb_bus_sense = 0;
	check_usb_power();
	usb_write( init_msg, (byte)4);
	
	while (1)
	{
		for (i=0; i<64000; i++)
			;
			
		usb_char = usb_state();
				
		display_set( DIGIT_C, usb_char);

		if (INTCON3bits.INT2IF)
			volume_update();
		
		/* some I/O to check repeatedly, for absence of interrupt-on-change */
		check_usb_power();
	}
}

void check_usb_power(void)
{
	if (HasUSB != prev_usb_bus_sense)
	{
		PIR2bits.USBIF = 1; // enter USB code through interrupt
		prev_usb_bus_sense = HasUSB;
	}	
}	

// Pick-up high-priority interrupts (or global interrupts if IPEN is clear)
#pragma interrupt app_isr_high
void app_isr_high(void)
{
	if (PIE3bits.TMR4IE && PIR3bits.TMR4IF)
	{
		display_isr();
		PIR3bits.TMR4IF = 0;
	}
#ifdef UseIPEN
}

// Pick-up low-priority interrupts

#pragma interruptlow app_isr_low
void app_isr_low(void)
{
	// The PIC hardware will clear the 'GIEL' bit upon entering this isr.
	// With our boot-loader in place, that will happen already there.
	// The bootloader will take care of the USB interrupt PIR2bits.USBIF.
#endif

	if (PIE3bits.TMR4IE && PIR3bits.TMR4IF)
	{
		display_isr();
		PIR3bits.TMR4IF = 0;
	}
	if (INTCON3bits.INT2IE && INTCON3bits.INT2IF)
	{   // edge on VolA input: check edge-direction of VolA against VolB value:
		if (INTCON2bits.INTEDG2 == VolB)
			volume_incr -= 1;
		else
			volume_incr += 1;
		
		// do not yet clear INTCON3bits.INT2IF: wait some cycles to reduce sensitivity
		// on spikes. Clear later in volume-processing code
		//INTCON3bits.INT2IF = 0;
		INTCON3bits.INT2IE = 0; // return from isr with volume interrupt disabled
	}
 	// The 'return from interruptlow' instruction will set the GIEL bit again	
}

static void init(void)
{
	// Reset some things, which aren't reset from USB command
	RCONbits.IPEN = IPENvalue; // enable interrupt priority mechanism
	INTCON = 0; // disable all interrupts for now
	INTCON2 = 0xFF;
	INTCON3 = 0;
	PIE1 = 0;
	PIE2 = 0;
	PIE3 = 0;
	PIR1 = 0;
 	PIR2 &= 0x10; // don't touch pending USB interrupt flag
	PIR3 = 0;
	IPR1 = 0;
	IPR2 = 0;
	IPR3 = 0;

#ifndef __DEBUG
	PIE2bits.USBIE = 1;  // allow USB interrupts
#endif

	// PORTA: all inputs, no interrupt-on-change facility :-(...
	PORTA = 0; TRISA = 0xFF; // A all inputs
	ADCON0 = 0; ANCON0 = 0xFF; ANCON1 = 0x1F; // disable AD converter, all digital IO
	
	
	// PORTB: use all-input config, use open-drain output style
	// The I2C pins must also be configured in input mode.
	PORTB = 0; TRISB = 0xFF; // B all inputs
	INTCON2bits.RBPU = 0; // use weak pull-up for RB67
	
	// PORTC: all input except 7
	PORTC = 0x80; TRISC = 0x7F;
	
	// setup the remappable peripheral inputs:
	// Clear IOLOCK
	EECON2 = 0x55;
	EECON2 = 0xAA;
	PPSCONbits.IOLOCK = 0;
	RPINR1 = 0; // IRserial -> RA0 -> RP0 -> INT1
	RPINR2 = 1; // VolA     -> RA1 -> RP1 -> INT2
	RPINR3 = 2; // SelectB  -> RA5 -> RP2 -> INT3
	// Set IOLOCK
	EECON2 = 0x55;
	EECON2 = 0xAA;
	PPSCONbits.IOLOCK = 1;
	
	// setup I2C peripheral
	SSP1CON1 = 0x28; // enable I2C master mode
	SSP1ADD = 0x63; // 100kHz bitrate from 40MHz system clock
	
	// Timer0 OK for use, 8-bit prescale, 8- or 16-bit counter, sets TMR0IF
	// Timer1: enabling it locks pins RC0 and RC1 to a special-function input, cannot do that.
	// Timer2: 4-prescale, 4-bit post-scale, 8-bit counter&comparator, sets TMR2IF
	// Timer3: 3-bit prescale, 16-bit counter, allows cascading with TMR1 or TMR2, sets TMR1IF(!)
	// Timer4: 4-bit prescale, 8-bit count&compare, 4=bit postscale, sets TMR4IF
	
	// setup Timer4 for display refresh: 2^16 downscale from Fosc/4 is 153Hz
	// hmm... measure as 2x91.5509 Hz
	T4CON = 0xFF; // timer4 on, 16x prescale, 16x postscale
	IPR3bits.TMR4IP = 1; // high priority interrupt
	//IPR3bits.TMR4IP = 0; // low priority interrupt
	PIE3bits.TMR4IE = 1;
	
	// PortA (through PPS) interrupt enables
	INTCON2bits.INTEDG2 = 1; //!VolA;
	INTCON3bits.INT2IF = 0;
	INTCON3bits.INT2IP = 0;
	INTCON3bits.INT2IE = 1; // VolA
}	