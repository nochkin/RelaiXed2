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
 * Detect whether this Relaixed board connects to a 4-input DAC
 * Control the DAC input selection together with the relaixed inputs.
 * Read-back DAC lock info (spdif presence) to show in the display.
 ********************************************************************/
#include <plib/i2c.h>
#include <stdio.h>
#include "amp_state.h"
#include "display.h"
#include "usb_io.h"
#include "dac_cntl.h"
#include "io_cfg.h"

static byte dac_state = DAC_ABSENT;

byte dac_status( void)
{
  return dac_state;
}

// Local version, adapted from the provided one in C18/pmc_common/i2c/i2c1writ.c
static unsigned char writeI2C( unsigned char data_out )
{
    SSP1BUF = data_out;           // write single byte to SSPBUF
    if ( SSP1CON1bits.WCOL )      // test if write collision occurred
    {
        SelectA = 0;
        return -1;              // if WCOL bit is set return negative #
    }
    else if( ((SSP1CON1&0x0F)==0x08) || ((SSP1CON1&0x0F)==0x0B) )	//master mode only
    {
        while( SSP1STATbits.BF );   // wait until write cycle is complete
	    IdleI2C();                 // ensure module is idle
	    if ( SSP1CON2bits.ACKSTAT ) // test for ACK condition received
            {
                SelectA = 0;
                return  -2;			// return NACK
            }
            else return  0;              //return ACK
    }
    return -2;
}

static unsigned char readI2C( void )
{
  if( ((SSP1CON1&0x0F)==0x08) || ((SSP1CON1&0x0F)==0x0B) )	//master mode only
    SSP1CON2bits.RCEN = 1;           // enable master for 1 byte reception
  while ( !SSP1STATbits.BF );      // wait until byte received  
  return ( SSP1BUF );              // return with read byte 
}

static char dac_send(byte regaddr, byte data)
{
	char err;
    char chip_addr = 0x4e; // lsb is 0 for write

	StartI2C();
	err = writeI2C(chip_addr) || // opcode
	      writeI2C(regaddr) || // addr of mcp23008 register
	      writeI2C(data);      // into selected reg
	StopI2C();
        SelectA = 1;


    return err;
}

static char dac_receive( void)
{
  char data;
  char chip_addr = 0x4f; // lsb is 1 for read

  StartI2C();
  writeI2C(chip_addr);
  data = readI2C();
  StopI2C();
  SelectA = 1;

  return data;
}

/* Is a DAC connected to the Relaixed i2c bus? */
/* on OK, we end with setting reg-addr to GPIO */
void dac_init(void)
{
	char err;
    char dac_msg[] = {'D', 'A', 'C', '-'};

	err = dac_send(0x00, 0xe0) || // IODIR: upper 3 bits are input
          dac_send(0x05, 0x20) || // IOCON: no addr auto-increment
          dac_send(0x09, 0x01);   // GPIO:  set output lsb

	if (err)
		dac_msg[3] = '-';
	else
    {
		dac_msg[3] = '!';
		dac_state = DAC_INACTIVE;
    }
	usb_write( dac_msg, (byte)4); // three-char message
}

/* Choose DAC input: 0 .. 3. A value >=4 switches DAC off. */
/* set reg-addr to GPIO for status read */
void dac_set_channel( byte sel)
{
	char chan;
	// chan is a one-hot decoding of 'chan',
    // with 3 lsb bits only.

	switch (sel)
	{ case 0: chan = 0x0e; break;
	  case 1: chan = 0x0d; break;
	  case 2: chan = 0x0b; break;
	  case 3: chan = 0x07; break;
	  default:  chan = 0x0f;
	}
	dac_send(0x09, chan);
    dac_state = (chan == 0x0f) ? DAC_INACTIVE : DAC_NOLOCK;
}

/* called repeatedly when state is NOLOCK or LOCKED */
void dac_check_lock( void)
{
	char dac_io;

	if (power_state() < 2)
	{
		dac_state = DAC_INACTIVE;
		return;
	} 

    dac_io = dac_receive();

	if (dac_io & 0x40) // nolockLED (hmm.. now slipLED), active high on no-lock
	{
		dac_state = DAC_NOLOCK;
		volume_display(0); // set no-lock indicator in display
	} else
	{
		dac_state = DAC_LOCKED;
		volume_display(0); // set volume in display
	}
}