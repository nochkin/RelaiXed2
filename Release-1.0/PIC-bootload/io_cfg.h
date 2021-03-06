/*********************************************************************
 *
 *                Microchip USB C18 Firmware Version 1.2
 *
 *********************************************************************
 * FileName:        io_cfg.h
 * Dependencies:    See INCLUDES section below
 * Processor:       PIC18
 * Compiler:        C18 3.11+
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the �Company�) for its PICmicro� Microcontroller is intended and
 * supplied to you, the Company�s customer, for use solely and
 * exclusively on Microchip PICmicro Microcontroller products. The
 * software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN �AS IS� CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 * File Version  Date		Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 1.0			 04/09/2008	Started from MCHPFSUSB v1.3 HID Mouse
 *							demo project.  Commented out items that
 *							are not particularly useful for the
 *							bootloader.
 ********************************************************************/

#ifndef IO_CFG_H
#define IO_CFG_H

/** I N C L U D E S *************************************************/
#include "usbcfg.h"

/** T R I S *********************************************************/
#define INPUT_PIN           1
#define OUTPUT_PIN          0

#if defined(PIC18F4550_PICDEM_FS_USB)
/** U S B ***********************************************************/
#define tris_usb_bus_sense  TRISAbits.TRISA1    // Input

#if defined(USE_USB_BUS_SENSE_IO)
#define usb_bus_sense       PORTAbits.RA1
#else
#define usb_bus_sense       1
#endif

#define tris_self_power     TRISAbits.TRISA2    // Input

#if defined(USE_SELF_POWER_SENSE_IO)
#define self_power          PORTAbits.RA2
#else
#define self_power          1
#endif

// External Transceiver Interface
#define tris_usb_vpo        TRISBbits.TRISB3    // Output
#define tris_usb_vmo        TRISBbits.TRISB2    // Output
#define tris_usb_rcv        TRISAbits.TRISA4    // Input
#define tris_usb_vp         TRISCbits.TRISC5    // Input
#define tris_usb_vm         TRISCbits.TRISC4    // Input
#define tris_usb_oe         TRISCbits.TRISC1    // Output

#define tris_usb_suspnd     TRISAbits.TRISA3    // Output

/** L E D ***********************************************************/
#define mInitAllLEDs()      LATD &= 0xF0; TRISD &= 0xF0;

#define mLED_1              LATDbits.LATD0
#define mLED_2              LATDbits.LATD1
#define mLED_3              LATDbits.LATD2
#define mLED_4              LATDbits.LATD3

#define mLED_1_On()         mLED_1 = 1;
#define mLED_2_On()         mLED_2 = 1;
#define mLED_3_On()         mLED_3 = 1;
#define mLED_4_On()         mLED_4 = 1;

#define mLED_1_Off()        mLED_1 = 0;
#define mLED_2_Off()        mLED_2 = 0;
#define mLED_3_Off()        mLED_3 = 0;
#define mLED_4_Off()        mLED_4 = 0;

#define mLED_1_Toggle()     mLED_1 = !mLED_1;
#define mLED_2_Toggle()     mLED_2 = !mLED_2;
#define mLED_3_Toggle()     mLED_3 = !mLED_3;
#define mLED_4_Toggle()     mLED_4 = !mLED_4;

/** S W I T C H *****************************************************/
#define mInitAllSwitches()  TRISBbits.TRISB4=1;TRISBbits.TRISB5=1;
#define mInitSwitch2()      TRISBbits.TRISB4=1;
#define mInitSwitch3()      TRISBbits.TRISB5=1;
#define sw2                 PORTBbits.RB4
#define sw3                 PORTBbits.RB5

/** P O T ***********************************************************/
#define mInitPOT()          TRISAbits.TRISA0=1;ADCON0=0x01;ADCON2=0x3C;

/** S P I : Chip Select Lines ***************************************/
#define tris_cs_temp_sensor TRISBbits.TRISB2    // Output
#define cs_temp_sensor      LATBbits.LATB2

#define tris_cs_sdmmc       TRISBbits.TRISB3    // Output
#define cs_sdmmc            LATBbits.LATB3

/** S D M M C *******************************************************/
#define TRIS_CARD_DETECT    TRISBbits.TRISB4    // Input
#define CARD_DETECT         PORTBbits.RB4

#define TRIS_WRITE_DETECT   TRISAbits.TRISA4    // Input
#define WRITE_DETECT        PORTAbits.RA4

/********************************************************************/
/********************************************************************/
/********************************************************************/

#elif defined(PIC18F87J50_FS_USB_PIM)
/** U S B ***********************************************************/
// Bus sense pin is RB5 on PIC18F87J50 FS USB Plug-In Module.
// Must put jumper JP1 in R-U position
#define tris_usb_bus_sense  TRISBbits.TRISB5    // Input

#if defined(USE_USB_BUS_SENSE_IO)
#define usb_bus_sense       PORTBbits.RB5
#else
#define usb_bus_sense       1
#endif

#define self_power          0

///** L E D ***********************************************************/
#define mInitAllLEDs()      LATE &= 0xFC; TRISE &= 0xFC;

#define mLED_1              LATEbits.LATE1
#define mLED_2              LATEbits.LATE0

#define mLED_1_On()         mLED_1 = 1;
#define mLED_2_On()         mLED_2 = 1;

#define mLED_1_Off()        mLED_1 = 0;
#define mLED_2_Off()        mLED_2 = 0;

#define mLED_1_Toggle()     mLED_1 = !mLED_1;
#define mLED_2_Toggle()     mLED_2 = !mLED_2;
//
///** S W I T C H *****************************************************/
#define mInitAllSwitches()  TRISBbits.TRISB4=1;
#define mInitSwitch2()      TRISBbits.TRISB4=1;

#define sw2                 PORTBbits.RB4

/********************************************************************/
/********************************************************************/
/********************************************************************/

#elif defined(PIC18F46J50_PIM)
/** U S B ***********************************************************/
// Bus sense pin is RB5 on PIC18F46J50 FS USB Plug-In Module.
// Must put jumper JP1 in R-U position
#define tris_usb_bus_sense  TRISBbits.TRISB5    // Input

#if defined(USE_USB_BUS_SENSE_IO)
#define usb_bus_sense       PORTCbits.RC2
#else
#define usb_bus_sense       1
#endif

#define self_power          0

///** L E D ***********************************************************/
#define mInitAllLEDs()      LATE &= 0xFC; TRISE &= 0xFC;

#define mLED_1              LATEbits.LATE1
#define mLED_2              LATEbits.LATE0

#define mLED_1_On()         mLED_1 = 1;
#define mLED_2_On()         mLED_2 = 1;

#define mLED_1_Off()        mLED_1 = 0;
#define mLED_2_Off()        mLED_2 = 0;

#define mLED_1_Toggle()     mLED_1 = !mLED_1;
#define mLED_2_Toggle()     mLED_2 = !mLED_2;
//
///** S W I T C H *****************************************************/
#define mInitAllSwitches()  TRISBbits.TRISB2=1;
#define mInitSwitch2()      TRISBbits.TRISB2=1;

#define sw2                 PORTBbits.RB2
/********************************************************************/
/********************************************************************/
/********************************************************************/

//Uncomment below if using the YOUR_BOARD hardware platform
#elif defined(RELAIXED2)
//Add your hardware specific I/O pin mapping here
#define tris_usb_bus_sense  TRISCbits.TRISC6    // Input
#define usb_bus_sense       PORTCbits.RC6
#define self_power          1
#define mSetLogMode         (PIR3bits.CTMUIF = 1)
#define mClrLogMode         (PIR3bits.CTMUIF = 0)
#define mGetLogMode         PIR3bits.CTMUIF

#define UseIPEN
#define IPEN_value			1

#define LEDright            PORTCbits.RC7
#define mInitAllLEDs()      TRISB = 0xFF; TRISC = 0x7F; PORTB = 0; PORTC = 0x80; // led-drivers are 'opendrain'
// individual LEDs of right segment, in order of segmentled a, b, c, ...
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

#define mLED_1_Toggle()     {mLED_1 = !mLED_1; PORTBbits.RB2 = 0;}
#define mLED_2_Toggle()     {mLED_2 = !mLED_2; PORTBbits.RB0 = 0;}
#define mLED_3_Toggle()     {mLED_3 = !mLED_3; PORTCbits.RC1 = 0;}
#define mLED_4_Toggle()     {mLED_4 = !mLED_4; PORTCbits.RC0 = 0;}
#define mLED_5_Toggle()     {mLED_5 = !mLED_5; PORTCbits.RC2 = 0;}
#define mLED_6_Toggle()     {mLED_6 = !mLED_6; PORTBbits.RB1 = 0;}
#define mLED_7_Toggle()     {mLED_7 = !mLED_7; PORTBbits.RB3 = 0;}

#else
    #error Not a supported board (yet), add I/O pin mapping in __FILE__, line __LINE__
#endif

#endif //IO_CFG_H
