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

// high-priority interrupt vector:
#pragma code app_isr_hi = ISR_HI
void app_interrupt_at_high_vector(void)
{
	_asm goto app_isr_high _endasm	
}

// low-priority interrupt vector:
#pragma code app_isr_lo = ISR_LO
void app_interrupt_at_low_vector(void)
{
	_asm goto app_isr_low _endasm
}

#pragma code
void main(void)
{
	unsigned int i;
	
	init();
	
	
	while (1)
	{
		for (i=0; i<64000; i++)
			;
		
		if (display_cnt < 0x80)
			display_set( DIGIT_C, 0);
		else
		    display_set( DIGIT_minus, 1);

		check_usb_power();
	}
}

void check_usb_power(void)
{
	static BOOL prev_usb_bus_sense = 0;
	
	if (HasUSB != prev_usb_bus_sense)
	{
		PIR2bits.USBIF = 1; // enter USB code through interrupt
		prev_usb_bus_sense = HasUSB;
	}	
}	

// Pick-up low-priority interrupts
#pragma interruptlow app_isr_low
void app_isr_low(void)
{
	if (PIE2bits.USBIE && PIR2bits.USBIF)
	{
		PIR2bits.USBIF = 0;
	}	
}

// Pick-up high-priority interrupts
#pragma interrupt app_isr_high
void app_isr_high(void)
{
	if (PIE3bits.TMR4IE && PIR3bits.TMR4IF)
		display_isr();
}

static void init(void)
{
	// PORTA: all inputs, no interrupt-on-change facility :-(...
	PORTA = 0; TRISA = 0xFF; // A all inputs
	ADCON0 = 0; ANCON0 = 0xFF; ANCON1 = 0x1F; // disable AD converter, all digital IO
	
	// PORTB: use all-input config, use open-drain output style
	// The I2C pins must also be configured in input mode.
	PORTB = 0; TRISB = 0xFF; // B all inputs
	INTCON2bits.RBPU = 0; // use weak pull-up for RB67
	
	// PORTC: all input except 7
	PORTC = 0x80; TRISC = 0x7F;
	
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
	PIE3bits.TMR4IE = 1;
	
	// Globally enable interrupts
	RCONbits.IPEN = 1;
	INTCONbits.GIEH = 1;
	INTCONbits.GIEL = 1;
}	