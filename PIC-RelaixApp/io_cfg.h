/*********************************************************************
 *
 *   I/O Pin declarations for the PIC18F25J50 in the RelaiXed front PCB
 *
 *********************************************************************
 * FileName:        io_cfg.c
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
 * This code is copyrighted by Jos van Eijndhoven, Oct 1, 2010.
 * Commercial application or exploitation is not allowed without
 * written consent from the author.
 *
 * File Version  Date		Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 1.0			 10/01/2010	Original Version.  
 ********************************************************************/

// Note that I/O directions, to be set by *TRIS* bits,
// are already initialized by the RelaiXed  bootloader.

#define mLED_1              TRISBbits.TRISB2
#define mLED_2              TRISBbits.TRISB0
#define mLED_3              TRISCbits.TRISC1
#define mLED_4              TRISCbits.TRISC0
#define mLED_5              TRISCbits.TRISC2
#define mLED_6              TRISBbits.TRISB1
#define mLED_7              TRISBbits.TRISB3

#define mLED_1_On()         {mLED_1 = 0; PORTBbits.RB2 = 0;}
#define mLED_2_On()         {mLED_2 = 0; PORTBbits.RB0 = 0;}
#define mLED_3_On()         {mLED_3 = 0; PORTCbits.RC1 = 0;}
#define mLED_4_On()         {mLED_4 = 0; PORTCbits.RC0 = 0;}
#define mLED_5_On()         {mLED_5 = 0; PORTCbits.RC2 = 0;}
#define mLED_6_On()         {mLED_6 = 0; PORTBbits.RB1 = 0;}
#define mLED_7_On()         {mLED_7 = 0; PORTBbits.RB3 = 0;}

#define mLED_1_Off()        mLED_1 = 1;
#define mLED_2_Off()        mLED_2 = 1;
#define mLED_3_Off()        mLED_3 = 1;
#define mLED_4_Off()        mLED_4 = 1;
#define mLED_5_Off()        mLED_5 = 1;
#define mLED_6_Off()        mLED_6 = 1;
#define mLED_7_Off()        mLED_7 = 1;

#define IRserial PORTAbits.RA0
#define VolA     PORTAbits.RA1
#define VolB     PORTAbits.RA2
#define SelectA  PORTAbits.RA3
#define SelectB  PORTAbits.RA5

#define HasUSB   PORTCbits.RC6
#define LEDright PORTCbits.RC7

// RA6&7 are in use by the xtal oscillator
// RC4&5 are reserved for the USB port
// RB6&7 are reserved for the ICSP (Debug/Program) port
// RB4&5 are reserved for the I2C interface to the relay board

