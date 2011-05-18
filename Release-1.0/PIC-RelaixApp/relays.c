/*********************************************************************
 * Send state-updates to relay board(s) through an I2C connection
 * Also detect connected relay boards.
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

void set_relays(byte board_id, byte power, byte channel, byte vol_l, byte vol_r)
{
	// 'power' off is 0, non-zero is power on.
	// channel is 1..6 for input selection, 0 for mute
	// vol_[rl] ranges from 0..64
	byte chip_addr;
	byte i2c_portA, i2c_portB;
	char i2c_msg[9] = {'I','2','C',':','0','0',',','0','0'};

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
	i2c_portB = (vol_r << 6) | vol_l;
	i2c_portA = (channel << 4) | (vol_r >> 2);
	if (power)
		i2c_portA |= 0x80;

	/////////////////// Perform I2C transmission /////////////////////
	chip_addr = 0x40 | (board_id << 1);
	StartI2C();
	writeI2C(chip_addr);
	writeI2C(0x12); // addr of GPIOA register
	writeI2C(i2c_portA);
	writeI2C(i2c_portB);
	StopI2C();

	// vsprintf( i2c_msg, "I2C:%02x,%02x", i2c_portA, i2c_portB); why not OK??
	i2c_msg[5] = '0' + (i2c_portA & 0x0f);
	i2c_msg[4] = '0' + ((i2c_portA >> 4) & 0x0f);
	i2c_msg[8] = '0' + (i2c_portB & 0x0f);
	i2c_msg[7] = '0' + ((i2c_portB >> 4) & 0x0f);
	usb_write( i2c_msg, (byte)9);
}
	