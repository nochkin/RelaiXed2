/******************************************************************************
 Default (dummy) application above bootloader
 Its 'sleep' will get interrupted for USB events.
 
 Jos van Eijndhoven
 April 2010
 *****************************************************************************/
#include <p18cxxx.h>
#include "typedefs.h"                   
#include "io_cfg.h"
#include "BootPIC18NonJ.h"

extern void USBSubSystem(void);

void app_isr_high(void);
void app_isr_low(void);
void app_main(void);

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

extern byte usb_device_state;
extern byte ctrl_trf_state;
extern WORD wCount;

void UARTsendhex4( char b);
void UARTsendhex8( char b);
void UARTsendbyte( char b);

#pragma code page=0x2020
void app_main(void)
{
	unsigned int i;
    mLED_2_On();

	while(1)
	{
		//for (i=0; i<64000U; i++)
		//	;
		//{	// loop wrap-around without received interrupt
		//	PIE2bits.USBIE = 0; // not allow USB interrupts
		//	UARTsendhex8(UIR);
		//	UARTsendbyte(' ');
		//} else
		//{	// came out of isr
		//	UARTsendhex4(usb_device_state);
			//UARTsendhex4(ctrl_trf_state);
			//UARTsendhex8(UADDR);
			//UARTsendhex8(LSB(wCount));
			//UARTsendhex8(UCON);
			//UARTsendhex8(PIR2);
			//UARTsendhex8(PIE2);
			//UARTsendbyte('.');
			//UARTsendhex8(UIE);
			//UARTsendhex8(UIR);
			//UARTsendbyte(' ');
		//}
		//ClrWdt();
		//USBSubSystem();
		//Sleep(); // wait to handle (USB-)interrupt
	}	
}

void UARTinit(void)
{
	unsigned char c;
    //ANSELHbits.ANS11 = 0;	// Make RB5 digital so USART can use pin for Rx
    ANSELH      = 0;
 	UART_TRISRx = 1;		// RX
    UART_TRISTx = 0;		// TX
    TXSTA       = 0x24;     // TX enable BRGH=1
    RCSTA       = 0x90;     // Single Character RX
    SPBRG       = 0x71;
    SPBRGH      = 0x02;     // 0x0271 for 48MHz -> 19200 baud
    BAUDCON     = 0x08;     // BRG16 = 1
    c = RCREG;			    // read 
}

void UARTsendbyte( char b)
{
	while (!TXSTAbits.TRMT)
		; // previous transmit still busy
	TXREG = b;
}

void UARTsendhex4( char b)
{
	unsigned char l;
	l = b & 0x0f;
	if (l < 10)
		UARTsendbyte('0' + l);
	else
		UARTsendbyte('a' - 10 + l);
}

void UARTsendhex8( char b)
{
	unsigned char h;
	
	h = b >> 4;
	UARTsendhex4( h);
	UARTsendhex4( b);
}

#pragma interruptlow app_isr_low
void app_isr_low(void)
{
	     mLED_4_On();
}

#pragma interrupt app_isr_high
void app_isr_high(void)
{
	     mLED_4_On();
}
