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

void app_isr_high(void);
void app_isr_low(void);
void main(void);
void check_usb_power(void);
static void init(void);               

// JvE: pragmas for bootloader addressing made
// according to MPASM&MPLINK manual, chapter 13.5

#pragma code app_start=0x2000
void app_start(void)
{
	_asm goto main _endasm	
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

#pragma code
void main(void)
{
	unsigned int i;
	byte j;
	
	init();
	
	j = 0;
	
	while (1)
	{
		for (i=0; i<64000; i++)
			;
		if (++j > 2)
			j = 0;
			
		switch (j)
		{
			case 0: mLED_1_On(); mLED_4_Off(); break;
			case 1: mLED_7_On(); mLED_1_Off(); break;
			case 2: mLED_4_On(); mLED_7_Off();
		}	
		check_usb_power();
	}
}

void check_usb_power(void)
{
	static BOOL prev_usb_bus_sense = 0;
	
	if (UCONbits.USBEN)
		mLED_2_On()
	else
		mLED_2_Off()

	if (HasUSB != prev_usb_bus_sense)
	{
		PIR2bits.USBIF = 1; // enter USB code through interrupt
		prev_usb_bus_sense = HasUSB;
	}	
}	

// we do not expect to receive interrupts in this app...
#pragma interruptlow app_isr_low
void app_isr_low(void)
{
}

#pragma interrupt app_isr_high
void app_isr_high(void)
{
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
}	