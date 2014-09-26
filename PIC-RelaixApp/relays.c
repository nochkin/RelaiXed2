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

typedef union _WORD {
    uint16_t _word;
    struct {
        uint8_t v[2];
    };
} WORD;
#define WORD_LSB(a)      ((a).v[0])
#define WORD_MSB(a)      ((a).v[1])

// BoardId on I2C bus, as addr bits in I2C. values are ((0..7)<<1);
static uint8_t relayBoardId   = 0;
uint8_t relayBoardType = RELAIXED_XLR;

// Local version, adapted from the provided one in C18/pmc_common library

unsigned char myWriteI2C(unsigned char data_out) {
    SSP1BUF = data_out; // write single uint8_t to SSPBUF
    if (SSP1CON1bits.WCOL) // test if write collision occurred
    {
        SelectA = 0;
        return ( -1); // if WCOL bit is set return negative #
    }
    //else if (((SSP1CON1 & 0x0F) == 0x08) || ((SSP1CON1 & 0x0F) == 0x0B)) //master mode only
    {
        while (SSP1STATbits.BF); // wait until write cycle is complete
        IdleI2C(); // wait for bus idle before continuing
        if (SSP1CON2bits.ACKSTAT) // test for ACK condition received
        {
            SelectA = 0;
            return ( -2); // return NACK
        }
        else return ( 0); //return ACK
    }
    //return -2;
}

// Configure mode and initial conditions in the
// I2C receiver (MCP23017) in the relay board(s)
char relay_boards_init(void) {
    char err = 0;
    uint8_t i, portA_init;
    char i2c_msg[9] = {'B', 'o', 'a', 'r', 'd', ' ', 'E', 'R', 'R'};

    // Check for relay board type, try twice..
    for (i=0; i<2; i++) {
        StartI2C();
        err = myWriteI2C(0x40); // address MCP23017 on board with addr/id 0
        StopI2C();
        SelectA = 1;
        if (!err) {
            relayBoardType = RELAIXED_XLR;
            relayBoardId = 0;
            break;
        }
        StartI2C();
        err = myWriteI2C(0x48); // address MCP23017 on board with A2==1
        StopI2C();
        SelectA = 1;
        if (!err) {
            relayBoardType = RELAIXED_SE;
            relayBoardId = 8;
            break;
        }
    }
    if (err) {
        usb_write(i2c_msg, (uint8_t) 9);
        return err;
    }

    // Leave IOCON reg is default (=0) state: has auto address increment

    // Set init values for MCP23017 output pins
    portA_init = 0;
    if (isRelaixedSE)
        portA_init = 0xff; // unbuffered drive to relays, low active

    StartI2C();
    myWriteI2C(0x40 | relayBoardId) || // address MCP23017
    myWriteI2C(0x12) || // addr of GPIOA register
    myWriteI2C(portA_init) || // to GPA[0..7]
    myWriteI2C(0); // to GPB[0..7]
    StopI2C();

    // Set both port A and B as all-output
    StartI2C();
    myWriteI2C(0x40 | relayBoardId) || // address MCP23017 on board with addr/id 0
    myWriteI2C(0x00) || // select on-chip register addr 0;
    myWriteI2C(0x00) || // IODIRA = 0 (all output)
    myWriteI2C(0x00); // IODIRB = 0 (all output)
    StopI2C();

    SelectA = 1;
    byte2hex(i2c_msg + 6, relayBoardId);
    usb_write(i2c_msg, (uint8_t) 8);
    return 0;
}

void mcp23017_output(uint8_t chip_addr, WORD data) {
    StartI2C();
    myWriteI2C(chip_addr);
    myWriteI2C(0x12); // addr of GPIOA register
    myWriteI2C(WORD_MSB(data)); // to GPA[0..7]
    myWriteI2C(WORD_LSB(data)); // to GPB[0..7]
    StopI2C();
    SelectA = 1;
}

void set_relays(uint8_t power, uint8_t channel, uint8_t vol_l, uint8_t vol_r) {
    // 'power' off is 0, non-zero is power on.
    // In relaixedSE: power==1: temp 'soft power' state with series resistors
    //                power==2: full power on.
    // channel is 1..6 for input selection, 0 for mute
    // vol_[rl] ranges from 0..64
    uint8_t chip_addr;
    WORD i2c_data;
    char i2c_msg[9] = {'I', '2', 'C', ':', '0', '0', ',', '0', '0'};

    if (!power)
        channel = 0; // channel==0 means 'mute'.

    if (!channel) {
        vol_l = 0;
        vol_r = 0;
    }

    // encode new state in 16-bit word for relay board state
    if (isRelaixedXLR) {
        WORD_LSB(i2c_data) = (vol_r << 6) | vol_l; // PortB
        WORD_MSB(i2c_data) = (channel << 4) | (vol_r >> 2); // PortA
        if (power)
            WORD_MSB(i2c_data) |= 0x80;
    } else {
        // isRelaixedSE: relaixedPassive
        uint8_t tmpA;
        WORD_LSB(i2c_data) = vol_l;
        if (power > 1) // full power on, major power relay
            WORD_LSB(i2c_data) |= 0x40;
        tmpA = 0xff; // portA is low-active!
        if (power > 0 && channel > 0 && channel <= 4)
            tmpA = ~(1 << (channel-1)); // decoded channel, active low
        if (power > 0)
            tmpA &= 0x3F; // double line to soft-power-relay, active low
        WORD_MSB(i2c_data) = tmpA;
    }

    /////////////////// Perform I2C transmission /////////////////////
    // Need bus-reset of device-reset here, to recover from lockup on erroneous bus transfers??
    chip_addr = 0x40 | relayBoardId;
    mcp23017_output(chip_addr, i2c_data);

    //////////////////// Log output to USB if logging is on /////////////////
    // vsprintf( i2c_msg, "I2C:%02x,%02x", i2c_portA, i2c_portB); why not OK??
    byte2hex(i2c_msg + 4, WORD_MSB(i2c_data));
    byte2hex(i2c_msg + 7, WORD_LSB(i2c_data));
    usb_write(i2c_msg, (uint8_t) 9);
}
