/*********************************************************************
 *
 * Microchip USB C18 Firmware -  HID Bootloader Version 1.0 for PIC18F46J50 Family Devices
 *
 *********************************************************************
 * FileName:        Boot46J50Family.h
 * Dependencies:    See INCLUDES section below
 * Processor:       PIC18
 * Compiler:        C18 2.30.01+
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
 * File version         Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 1.0					04/09/2008	Original
 ********************************************************************/
#ifndef BOOT46J50FAMILY_H
#define BOOT46J50FAMILY_H

#define ProgramMemStart	0x002000 //Beginning of application program memory
                        // JvE modified from 0x001000 to 0x002000
                        //(not occupied by bootloader). 

/** P U B L I C  P R O T O T Y P E S *****************************************/
extern void UserInit(void);
extern void ProcessIO(void);
extern void ProcessBootLoad(void);


#endif //BOOT46J50FAMILY_H
