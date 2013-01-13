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
#include <i2c.h>
#include <stdio.h>
#include "typedefs.h"
#include "io_cfg.h"
#include "usb_io.h"
#include "amp_state.h"
#include "relays.h"

// Local version, adapted from the provided one in C18/pmc_common library
static unsigned char writeI2C( unsigned char data_out )
{
  SSP1BUF = data_out;           // write single byte to SSPBUF
  if ( SSP1CON1bits.WCOL )      // test if write collision occurred
   return ( -1 );              // if WCOL bit is set return negative #
  else if( ((SSP1CON1&0x0F)==0x08) || ((SSP1CON1&0x0F)==0x0B) )	//master mode only
  { 
	 	while( SSP1STATbits.BF );   // wait until write cycle is complete   
	    IdleI2C();                 // ensure module is idle
	    if ( SSP1CON2bits.ACKSTAT ) // test for ACK condition received
	    	 return ( -2 );			// return NACK
		else return ( 0 );              //return ACK
  }
}

// Configure mode and initial conditions in the
// I2C receiver (MCP23017) in the relay board(s)
char relay_boards_init(void)
{
	char err;
	// Leave IOCON reg is default (=0) state: has auto address increment
	// Set both port A and B as all-output
	StartI2C();
	err = writeI2C(0x40) || // address MCP23017 on board with addr/id 0
		writeI2C(0x00) || // select on-chip register addr 0;
		writeI2C(0x00) || // IODIRA = 0 (all output)
		writeI2C(0x00); // IODIRB = 0 (all output)
	StopI2C();

	return err;
}

void mcp23017_output(byte chip_addr, WORD data)
{
	StartI2C();
	writeI2C(chip_addr);
	writeI2C(0x12); // addr of GPIOA register
	writeI2C(MSB(data)); // to GPA[0..7]
	writeI2C(LSB(data)); // to GPB[0..7]
	StopI2C();
}

void set_relays(byte board_id, byte power, byte channel, byte vol_l, byte vol_r)
{
	// 'power' off is 0, non-zero is power on.
	// channel is 1..6 for input selection, 0 for mute
	// vol_[rl] ranges from 0..64
	byte chip_addr;
	WORD i2c_data, i2c_tmp;
	char i2c_msg[9] = {'I','2','C',':','0','0',',','0','0'};
    static WORD i2c_prev = 0;

	board_id = 0; // for now....

	if (!power)
		channel = 0; // channel==0 means 'mute'.

	if (!channel)
	{
		vol_l = 0;
		vol_r = 0;
	} else if (vol_l == 0 && vol_r == 0)
	{
		channel = 0;
	} else
	{
		if (vol_l) vol_l--;  // change coding of non-zero volume from 0..63
		if (vol_r) vol_r--;
	}

	// encode new state in 16-bit word for relay board state
	LSB(i2c_data) = (vol_r << 6) | vol_l;
	MSB(i2c_data) = (channel << 4) | (vol_r >> 2);
	if (power)
		MSB(i2c_data) |= 0x80;

	/////////////////// Perform I2C transmission /////////////////////
	// Need bus-reset of device-reset here, to recover from lockup on erroneous bus transfers??
	chip_addr = 0x40 | (board_id << 1);

	i2c_tmp._word = i2c_data._word & i2c_prev._word;
	if (i2c_tmp._word != i2c_data._word && i2c_tmp._word != i2c_prev._word)
	{
		// first issue an intermediate value, to prevent high-volume audio
		// bursts in the output during the relay switch time
		unsigned int count;

		mcp23017_output(chip_addr, i2c_tmp);
		for (count=0; count < 4000L; count++)
				; // wait-loop for about 5 msec. Not beautiful, using a timer would be nicer
	}
	mcp23017_output(chip_addr, i2c_data);
	i2c_prev = i2c_data;


	//////////////////// Log output to USB if logging is on /////////////////
	// vsprintf( i2c_msg, "I2C:%02x,%02x", i2c_portA, i2c_portB); why not OK??
	byte2hex(i2c_msg+4, MSB(i2c_data));
	byte2hex(i2c_msg+7, LSB(i2c_data));
	usb_write(i2c_msg, (byte)9);
}
	