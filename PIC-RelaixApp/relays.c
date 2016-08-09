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
/*********************************************************************
 * Send state-updates to relay board(s) through an I2C connection
 * Also detect connected relay boards.
 * 
 * On an unreliable I2C connection, protocol errors could arise.
 * It seems that this (Microchip) implementation of SW-drivers of its
 * HW I2C module can achieve various kinds of deadlock on such errors.
 * An I2C protocol deadlock can hang-up the entire PIC.
 * The low-level I2C driver code should better be rewritten to resolve this!
 ********************************************************************/
#define M_RELAY

#include <stdint.h>
#include <plib/i2c.h>
#include <stdio.h>
#include "io_cfg.h"
#include "usb_io.h"
#include "amp_state.h"
#include "relays.h"
#include "display.h"

// BoardId on I2C bus, as addr bits in I2C. values are ((0..7)<<1);
static uint8_t relayBoardId   = 0;
uint8_t relayBoardType = RELAIXED_XLR;

// Local version, adapted from the provided one in C18/pmc_common/i2c/i2c1writ.c
uint8_t myWriteI2C(unsigned char data_out) {
    SSP1BUF = data_out; // write single uint8_t to SSPBUF
    if (SSP1CON1bits.WCOL) // test if write collision occurred
    {
        return ( -1); // if WCOL bit is set return negative #
    }
    //else if (((SSP1CON1 & 0x0F) == 0x08) || ((SSP1CON1 & 0x0F) == 0x0B)) //master mode only
    {
        while (SSP1STATbits.BF); // wait until write cycle is complete
        IdleI2C(); // wait for bus idle before continuing
        if (SSP1CON2bits.ACKSTAT) // test for ACK condition received
        {
            return ( -2); // return NACK
        }
        else return ( 0); //return ACK
    }
    //return -2;
}

static uint8_t myReadI2C( void ) {
  if( ((SSP1CON1&0x0F)==0x08) || ((SSP1CON1&0x0F)==0x0B) )	//master mode only
    SSP1CON2bits.RCEN = 1;           // enable master for 1 byte reception
  while ( !SSP1STATbits.BF );      // wait until byte received  
  return ( SSP1BUF );              // return with read byte 
}

// return 1-byte register read.
// perform initial write-cycle to set register index, then do read-cycle.
uint8_t i2c_read( uint8_t chip_addr, uint8_t regno) {
  char data;
  
  StartI2C();
  myWriteI2C(chip_addr & 0xfe); // lsb 0 is for write
  myWriteI2C(regno); // set reg index pointer

  RestartI2C();
  myWriteI2C(chip_addr | 0x01); // lsb 1 is for eead
  data = myReadI2C();
  StopI2C();

  return data;
}

// test presence of chip addr, return 0 if found
uint8_t i2c_probe( uint8_t chip_addr) {
	uint8_t err;
	
	StartI2C();
    err = myWriteI2C(chip_addr & 0xfe); // address MCP23017 on board with addr/id 0
    StopI2C();

	return err;
}

// write single byte to i2c device register, return 0 if OK
// assume lsb of chip_addr is 0 for write
uint8_t i2c_write1( uint8_t chip_addr, uint8_t regno, uint8_t data) {
  char err = 0;
  
  StartI2C();
  err = myWriteI2C(chip_addr) ||
        myWriteI2C(regno) || // set reg index pointer
        myWriteI2C(data);
  StopI2C();

  return err;
}

// write two bytes to consequtive i2c device registers, return 0 if OK
// assume lsb of chip_addr is 0 for write
uint8_t i2c_write2( uint8_t chip_addr, uint8_t regno, uint8_t data0, uint8_t data1) {
  char err = 0;
  
  StartI2C();
  err = myWriteI2C(chip_addr) ||
        myWriteI2C(regno) || // set reg index pointer
        myWriteI2C(data0) ||
        myWriteI2C(data1);
  StopI2C();

  return err;
}

static void mcp23017_init(uint8_t board_id) {
	// board_id is 0..7, based on mcp23017 address jumper setting
	unsigned char i2c_addr = 0x40 | (board_id << 1);
	// Set init values for MCP23017 output pins
    uint8_t portA_init = 0;
    if (isRelaixedSEx)
        portA_init = 0xff; // unbuffered drive to relays, low active

	//  set initial value for GPIOA and B
	i2c_write2( i2c_addr, 0x12, portA_init, 0x00);

    // Set both port A and B as all-output
	i2c_write2( i2c_addr, 0x00, 0x00, 0x00);
}

// Configure mode and initial conditions in the
// I2C receiver (MCP23017) in the relay board(s)
char relay_boards_init(void) {
    char err = 0;
    uint8_t i, data, chip_addr;
    char i2c_msg[9] = {'B', 'o', 'a', 'r', 'd', ' ', 'E', 'R', 'R'};

    // Check for relay board type, try twice..
    for (i=0; i<2; i++) {
		chip_addr = 0x40;
		err = i2c_probe( chip_addr);
        if (!err) {
            relayBoardType = RELAIXED_XLR;
            relayBoardId = 0;
            break;
        }
		// RelaixedXLR not (yet) found, check for RelaixedPassive
		chip_addr = 0x48;
		err = i2c_probe( chip_addr);
		if (err) // RelaixedSE not found either, try again...
			continue;
        relayBoardType = RELAIXED_SE;
        relayBoardId = 8;

		// OK, found main relaixedSE board:
		// check for extra slave board at next address..
		err = i2c_probe( 0x4a); // address MCP23017 on board with A2==1, A1=0,A0=1
        if (err) { // no slave board at next addr, so just plain RelaixedSE: OK
		    err = 0;
			break;
		}
		
        relayBoardType = RELAIXED_SE2;
		// OK, found main relaixedS board and one extra slave board
		// check for 2nd extra slave board at next address..
        err = i2c_probe(0x4c); // address MCP23017 on board with A2==1, A1=1,A0=0
        if (!err) {
            relayBoardType = RELAIXED_SE3;
        }
		err = 0;
		break;
    }
    if (i == 2) {
		// could not find a proper relaixed board :-(
        usb_write(i2c_msg, (uint8_t) 9);
        return err;
    }

    // Leave IOCON reg is default (=0) state: has auto address increment
	data = i2c_read(chip_addr, 0x05); // read GPINT if BANK=0, or IOCON if BANK=1
	if (data & 0x80) // IOCON.BANK==1 ? (GPINT.7 is probably 0)
		i2c_write1( chip_addr, 0x05, 0x00); // reset IOCON: set BANK=0

	if (relayBoardType == RELAIXED_XLR)
		mcp23017_init(0);
	else {
		mcp23017_init(4);
		if (relayBoardType == RELAIXED_SE2)
			mcp23017_init(5);
		else if (relayBoardType == RELAIXED_SE3) {
			mcp23017_init(5);
			mcp23017_init(6);
		}
	}

    SelectA = 1;
    byte2hex(i2c_msg + 6, relayBoardType);
    usb_write(i2c_msg, (uint8_t) 8);
    return 0;
}

void set_relays(uint8_t power, uint8_t channel, uint8_t vol_l, uint8_t vol_r) {
    // 'power' off is 0, non-zero is power on.
    // In relaixedSE: power==1: temp 'soft power' state with series resistors
    //                power==2: full power on.
    // channel is 1..6 for input selection, 0 for mute
    // vol_[rl] ranges from 0..64
    uint8_t chip_addr;
	uint8_t relaixedse_pwr = 0;
	uint8_t portA, portB;
    char i2c_msg[9] = {'I', '2', 'C', ':', '0', '0', ',', '0', '0'};

    if (!power)
        channel = 0; // channel==0 means 'mute'.

    if (!channel) {
        vol_l = 0;
        vol_r = 0;
    }

    // encode new state in 16-bit word for relay board state
	
    if (isRelaixedXLR) {
        portB = (vol_r << 6) | vol_l; // PortB
        portA = (channel << 4) | (vol_r >> 2); // PortA
        if (power)
            portA |= 0x80;
    } else {
        // isRelaixedSE: relaixedPassive
        uint8_t tmpA;
		if (power > 1)
			relaixedse_pwr = 0x40; // full power on, major power relay
        portB = vol_l | relaixedse_pwr;
        portA = 0xff; // portA is low-active!
        if (power > 0 && channel > 0 && channel <= 6)
            // Note: default use for 2 aux pins is for channel extension
            portA = ~(1 << (channel-1)); // decoded channel, active low
        if (power > 0)
            portA &= 0x3F; // double line to soft-power-relay, active low
    }

    /////////////////// Perform I2C transmission /////////////////////
    // Need bus-reset of device-reset here, to recover from lockup on erroneous bus transfers??
    chip_addr = 0x40 | relayBoardId;
	i2c_write2( chip_addr, 0x12, portA, portB);
	
	// support for extra slave boards..
	if (isRelaixedSE2 || isRelaixedSE3) {
		chip_addr += 2;
		portB = vol_r  | relaixedse_pwr;
		i2c_write2( chip_addr, 0x12, portA, portB);
		
		if (isRelaixedSE3) {
			chip_addr += 2;
			portB = ((vol_r + vol_l)>>1) | relaixedse_pwr;
			i2c_write2( chip_addr, 0x12, portA, portB);
		}
	}

    //////////////////// Log output to USB if logging is on /////////////////
    // vsprintf( i2c_msg, "I2C:%02x,%02x", i2c_portA, i2c_portB); why not OK??
    byte2hex(i2c_msg + 4, portA);
    byte2hex(i2c_msg + 7, portB);
    usb_write(i2c_msg, (uint8_t) 9);
}
