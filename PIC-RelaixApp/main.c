/****************************************************************************************
	This file is part of the Relaixed firmware.
    The Relaixed firmware is intended to control a DIY audio premaplifier.
    Copyright 2011 Jos van Eijndhoven, The Netherlands

    The Relaixed firmware is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The Relaixed firmware is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this firmware.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************************/

/****************************************************************************************
	This source code can be compiled with the free Microchip MPLAB C18 'LITE' C-compiler.
	The set of source files can be handled as project with the free Microchip MPLAB IDE.

	This file contains the 'main' top function, the interrupt-service-routines, and
    the code to set the start-up state of the targeted PIC18F25J50 microcontroller.
****************************************************************************************/

#pragma config WDTEN = OFF          //WDT disabled (enabled by SWDTEN bit)
#pragma config PLLDIV = 3           //Divide by 3 (12 MHz oscillator input)
#pragma config STVREN = ON          //stack overflow/underflow reset enabled
#pragma config XINST = OFF          //Extended instruction set disabled
#pragma config CPUDIV = OSC1        //No CPU system clock divide: Fosc=48MHz
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
#pragma config WPFP = PAGE_7        //Write Protect Program Flash Page 0
#pragma config WPEND = PAGE_0       //Start protection at page 0
#pragma config WPCFG = OFF          //Write/Erase last page protect Disabled
#pragma config WPDIS = OFF          //WPFP[5:0], WPEND, and WPCFG bits ignored


#include <p18cxxx.h>
#include <stdio.h>
#include <i2c.h>
#include "typedefs.h"                   
#include "io_cfg.h"
#include "display.h"
#include "usb_io.h"
#include "storage.h"
#include "amp_state.h"
#include "relays.h"
#include "ir_receiver.h"
#include "dac_cntl.h"

// Forward declarations
void app_isr_high(void);
void app_isr_low(void);
void main(void);
void check_usb_power(char);
static void init(void);

#ifdef __DEBUG
// After recent edits, I guess it is OK to have NO_BOOTLOADER always defined,
// that makes development/debugging easier...
#define NO_BOOTLOADER
#endif


// Normal operation. Built hex image should be inserted by bootloader
#define _RESET 0x2000
#define ISR_HI 0x2008
#define ISR_LO 0x2018

// JvE: pragmas for bootloader addressing made:
// the bootloader passes interrupt events to these locations.
// according to MPASM&MPLINK manual, chapter 13.5
// The 'reset' jumps into the real 'main', not a device init function.
extern void _startup (void); // See c018i.c in your C18 compiler dir
#pragma code app_start = _RESET
void _reset(void)
{
	_asm goto _startup _endasm	
}

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

#ifdef NO_BOOTLOADER
// the 3 functions below are a dummy bootloader replacement
// if a real bootloader is present, it overrides this code
//#pragma code boot_reset = 0x0000
//void boot_reset(void)
//{
//	_asm goto _startup _endasm
//}

#pragma code boot_isr_hi=0x0008
void boot_isr_high(void)
{
	_asm goto app_interrupt_at_high_vector _endasm	
}
#pragma code boot_isr_lo = 0x0018
void boot_isr_low(void)
{
	_asm goto app_interrupt_at_low_vector _endasm
}
#endif

#pragma code
// back to normal code allocation

static BOOL prev_usb_bus_sense;
static unsigned int volume_tick, usb_tick, chan_tick, power_tick, flash_tick, ir_tick;
static unsigned char dac_lock_tick;
static char ir_received_ok, ir_speedup;

static char vol_usb_msg[] = { 'V', '0', '0', '0', '0'};

void main(void)
{
	unsigned int i;
	char usb_char, err;
	const char init_msg[] = {'I', 'N', 'I', 'T'};
	extern FILE *stdout = _H_USER;  // redirect stdout to USB
	
	init();
	storage_init();
	
	display_cnt = 0;
    volume_tick = 0;
	chan_tick = 0;
	usb_tick = 0;
    ir_tick = 0;
	ir_speedup = 20;
    dac_lock_tick = 0;
	display_set_alt( 0x00, 0x00, 0x00);
	display_set( 0x00, 0x00, 1);
	ir_receiver_init();
	err = relay_boards_init();
	set_relays(0x00, 0x00, 0x00, 0x00, 0x00);
	amp_state_init();

	if (err)
		display_set_alt( DIGIT_E, 0x01, 3);

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
	usb_write( init_msg, (byte)4);

	// The above 'set_relays' enabled the power relay for the analog supply.
	power_tick = 150;

	// Set a timer to later undo the mute and activate last volume setting.
	// wait some time for stabilization before enabling all other interrupts
	while (power_tick > 0)
		; // gets decreased on timer interrupts, 183Hz

	// power==0 now, from amp_state_init().
	// incr power now quickly to 1, and later to 2.
	power_incr = 1;

	INTCON3bits.INT1IF = 0;
	INTCON3bits.INT1IE = 1;
	INTCON3bits.INT2IF = 0;
	INTCON3bits.INT2IE = 1;
	INTCON3bits.INT3IF = 0;
	INTCON3bits.INT3IE = 1;

    // Check if a DAC is present in this Relaixed, if so initialize.
    // This check was delayed to allow DAC power-up, otherwise its i2c interface stays in reset
	dac_init();

	while (1)
	{
		if (volume_incr)
			volume_update();

		if (balance_incr > 1 || balance_incr < -1)
			// suppress a single tick, might have been by accident
			balance_update();

		if (channel_incr)
			channel_update();

		if (power_incr)
		{
			if (flash_tick && power_incr < 0)
			{
				// quickly save recent volume/balance update
				flash_tick = 0;
				flash_volume_channel();
			} else if (power_incr > 0 && power_state() == 0)
			{
				// if we move power_state from 0 to 1, we surely want to go later to 2
				power_tick = 500;
			}

			if (power_incr > 0)
				dac_init();  // check (again) for presence of DAC: it needs time to get out of reset
			power_update();
		}

		if (ir_received_ok)
		{
			ir_received_ok = 0;
			ir_handle_code();
			if (volume_incr)
			{
				vol_usb_msg[0] = 'V';
				// when 'volume' keeps pressed, the volume-tick-speed goes up
				if(ir_speedup <= 49)
					ir_speedup += 4;
			} else
			{
				vol_usb_msg[0] = 'v';
				ir_speedup = 20;
			}
			byte2hex(vol_usb_msg+1, ir_tick);
			byte2hex(vol_usb_msg+3, ir_speedup);

			if (power_incr)
				ir_tick = 100; // increase the default 20 to 100 on power on/off
			else
				flash_tick = 400;

			usb_write(vol_usb_msg, 5);
		}

		if (flash_tick == 1)
		{
			flash_tick = 0;
			flash_volume_channel();
		}

		if (dac_status() >= DAC_NOLOCK && dac_lock_tick == 0)
		{
			dac_check_lock();
			dac_lock_tick = 45; // check lock 4x per secnd
		}

		/* some I/O to check repeatedly, for absence of interrupt-on-change */
		check_usb_power(err);
	}
}

void check_usb_power(char err)
{
	if (HasUSB == prev_usb_bus_sense)
		usb_tick = 0;
	else
		usb_tick++;

	if (usb_tick == 0xFF)
	{
		prev_usb_bus_sense = HasUSB;
		if (!err) // don't conceal earlier error on display
			display_set_alt( DIGIT_U, (HasUSB ? 1 : 0), 2);
		PIR2bits.USBIF = 1; // enter USB code through interrupt
	}
}	

// Pick-up high-priority interrupts (or global interrupts if IPEN is clear)
#pragma interrupt app_isr_high
void app_isr_high(void)
{
	// Timer4 wrap-around timer event, issued at 183Hz rate
	// Used for display multiplexing, but also for various local time counters
	if (PIE3bits.TMR4IE && PIR3bits.TMR4IF)
	{
		display_isr();

		if (volume_tick)
			volume_tick--;

		if (ir_tick)
			ir_tick--;

		if (chan_tick)
		{
			chan_tick--;
			if (chan_tick == 0 && INTCON2bits.INTEDG3 && power_state() == 2)
				// waited long for rising edge: do power-down
				power_incr = -1;
		}

		if (power_tick)
		{
			power_tick--;
			if (power_tick == 0)
			{
				power_incr = 1;
				// INTCON3bits.INT3IE set only after reaching here??
			}
		}

		if (flash_tick > 1)
			flash_tick--;

		if (dac_lock_tick)
			dac_lock_tick--;

		PIR3bits.TMR4IF = 0;
	}

	if (INTCON3bits.INT1IE && INTCON3bits.INT1IF)
	{   // edge on IR-receiver input
		// ir_tick, set high, needs to count down to ir_speedup, to accept a next volume command
		// From ir_speedup to 0 it counts down further, allowing as 'continuation' an increase in 'ir_speedup'.
		// The baseline value of '20' for ir-speedup covers the interval of about 100msec pause
		// between successive ir_commands in RC5 when the button maintains pressed
		if (((char)ir_tick) - ir_speedup <= 0)
		{	// the tick counter is to prevent a too high rate of processing volume/channel updates
			ir_receiver_isr();
		}

		INTCON3bits.INT1IF = 0;
		INTCON2bits.INTEDG1 = !IRserial;
	}

	if (INTCONbits.TMR0IE && INTCONbits.TMR0IF)
	{	// IR receiver timer expires: about 5 ms after end of IR pulse-train
		// Might also be a an unexpected termination on a bad signal reception
		ir_received_ok = ir_tmr_isr();
		if (ir_tick == 0)
			ir_speedup = 20; // reset accelleration

		if (ir_received_ok)
			ir_tick = 65; // wait-time to process a next IR signal train; avoid unwanted/fast command repeats

		INTCONbits.TMR0IE = 0; // do this interrupt only once after IR pulse train.
		INTCON2bits.INTEDG1 = 0; // a new pulse-train should start with a neg-edge.
	}

	if (INTCON3bits.INT2IE && INTCON3bits.INT2IF)
	{   // edge on VolA input: check edge-direction of VolA against VolB value:
		if (volume_tick == 0)
		{
			if (INTCON2bits.INTEDG3 == 1) //channel-button is pressed
			{
				if (INTCON2bits.INTEDG2 == VolB)
					balance_incr -= 1;
				else
					balance_incr += 1;
				
				if (balance_incr > 1 || balance_incr < -1)
					chan_tick = 0; // Suppress channel or power action on button release
			} else
			{
				if (INTCON2bits.INTEDG2 == VolB)
					volume_incr -= 1;
				else
					volume_incr += 1;
			}
		
        	volume_tick = 2; // create delay
			flash_tick = 400;
		}
		INTCON2bits.INTEDG2 = !VolA;
		INTCON3bits.INT2IF = 0;
	}

	if (INTCON3bits.INT3IE && INTCON3bits.INT3IF)
	{   // activity on channel-select push-button
		if (INTCON2bits.INTEDG3 && chan_tick < 252)
		{
			// rising edge on channel input (release of SelectB)
			if (chan_tick == 0)
			{
				// did already do power-down a while ago
				// or knob-turning created this into just balance adjust
			}
			else
			{ // quick release: do next channel
				if (power_state() == 2)
				{
					channel_incr = 1;
					flash_tick = 300;
				}
			}
			chan_tick = 3;
			INTCON2bits.INTEDG3 = 0; // look for falling edge
		} else if (!INTCON2bits.INTEDG3 && chan_tick == 0)
		{
			// falling edge on channel input (SelectB)
                        if (power_state() == 0)
			{
                            // launch power-up sequence
                            power_incr = 1;
                            chan_tick = 3;
			} else
                            chan_tick = 255; // Count duration of push button
			INTCON2bits.INTEDG3 = 1; // look for rising edge
		}
		INTCON3bits.INT3IF = 0;
	}

	// Power-supply drop on digital power: quickly mute audio
	if (PIE2bits.LVDIE && PIR2bits.LVDIF)
	{
		if (power_state() == 2)
		{
			power_incr = -1;
			PIE2bits.LVDIE = 0; // we go power-down, prevent interrupt to repeat
		}
		PIR2bits.LVDIF = 0;
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

 	// The 'return from interruptlow' instruction will set the GIEL bit again
}

static void init(void)
{
	// Reset some things, just to be sure
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

#ifdef NO_BOOTLOADER
	PIE2bits.USBIE = 0;
	PIR2bits.USBIF = 0;
#else
	PIE2bits.USBIE = 1;  // allow USB interrupts
#endif

        OSCTUNE = 0x40; // enable the 96MHz PLL to create the 48MHz system clock

	// PORTA: all inputs, no interrupt-on-change facility :-(...
	PORTA = 0; TRISA = 0xFF; // A all inputs
	ADCON0 = 0; ANCON0 = 0xFF; ANCON1 = 0x1F; // disable AD converter, all digital IO
	//CMCON = 0x07; // disable analog comparators on PortA
	//CM1CON = 0x07;
	
	
	// PORTB: use all-input config, use open-drain output style
	// The I2C pins must also be configured in input mode.
	PORTB = 0; TRISB = 0xFF; // B all inputs
	INTCON2bits.RBPU = 0; // use weak pull-up for RB67
	
	// PORTC: all input
	// note: portc.7 toggles run-time from input to output for 'opendrain' behavior
	PORTC = 0x00; TRISC = 0xFF;
	
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
	
	// setup Timer4 for display refresh: 2^16 downscale from Fosc/4 is 183Hz
	T4CON = 0xFF; // timer4 on, 16x prescale, 16x postscale
	IPR3bits.TMR4IP = 1; // high priority interrupt
	PIE3bits.TMR4IE = 1;

	// setup Timer0 for InfraRed protocol reception
	INTCON2bits.TMR0IP = 1; // high priority interrupt
	INTCONbits.TMR0IE = 0; // for now, enabled on actual IR reception
	T0CON = 0xc7; // Timer0 on, 8-bit mode, 256x prescale, on Fosc/4: 183Hz= 5.4msec timer interrupt
	
	// Volume rotary interrupt enable (on PortA, through PPS)
	INTCON2bits.INTEDG2 = !VolA;
	//INTCON3bits.INT2IF = 0;
	INTCON3bits.INT2IP = 1;
	INTCON3bits.INT2IE = 0; // just wait a while

	// SelectB input: interrupt on falling edge for channel select increment
	INTCON2bits.INTEDG3 = 0; //falling
	//INTCON3bits.INT3IF = 0;
	INTCON2bits.INT3IP = 1;
	INTCON3bits.INT3IE = 0; // just wait a while

	// IRreceiver input interrupt
	INTCON2bits.INTEDG1 = 0; //falling
	INTCON3bits.INT1IP = 1;
	INTCON3bits.INT1IE = 0; // just wait a while
	
	// I2C master to drive relay board(s)
    // adapted from OpenI2C in C18/pmc_common library
  	SSP1STAT &= 0x3F;               // power on state 
  	SSP1CON1 = 0x00;                // power on state
  	SSP1CON2 = 0x00;                // power on state
  	SSP1CON1 |= MASTER;           	// select serial mode 
  	SSP1STAT |= 0;                	// slew rate on/off 

  	//TRISBbits.TRISB5 = 1;         // SDA is input
  	//TRISBbits.TRISB4 = 1;			// SCL is input
  	SSP1CON1bits.SSPEN = 1;         // enable synchronous serial port

	// Set-up HVLD module: mute audio when power-supply unexpectedly drops
	// Programmed HVLD value '1001' corrsponds to 2.6V (range 2.47 - 2.73)
    // Take a small safety margin on that voltage level,
    // choosing it too low, can cause a too late audio mute.
    // (in time is as long as the audio opamps have reasonable supply voltage)
	HLVDCON = 0x79;
	PIR2bits.LVDIF = 0;
	IPR2bits.LVDIP = 1;
	PIE2bits.LVDIE = 0; //Enable only after power has stabelized
}	