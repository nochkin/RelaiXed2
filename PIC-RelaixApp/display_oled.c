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
 * Use an OLED 2 lines x 16 character display.
 * It has an SSD1311 controller, compatible with the '1602' commands.
 * It is driven through an I2C connection.
 *
 * The display is at address 0x3c
 * That is fortunate, since our mcp23017 relay driver is at 0x40-0x46 for the Relaixed2
 * and at address 0x48-0x4e for the RelaixedSE (relaixedPassive).
 ********************************************************************/

#include <stdint.h>
#include <plib/i2c.h>
#include <stdio.h>
#include "display_oled.h"
#include "io_cfg.h"
#include "usb_io.h"
#include "amp_state.h"

//#define OLED_addr 0x3c
#define OLED_addr 0x78
#define OLED_CGRAMADDR 0x40
#define OLED_DDRAMADDR 0x80
#define OLED_Command_Mode 0x80
#define OLED_Data_Mode 0x40

uint8_t has_oled_display = 0;

static void oled_setup(void);
static void oled_sendcommand(uint8_t command);
static void oled_sendData(uint8_t data);
static void oled_cursor(uint8_t col, uint8_t row);
static void createChar(uint8_t location, const uint8_t charmap[]);
static void oled_cmd_data( uint8_t cmd, uint8_t n, const uint8_t data[]);

extern unsigned char myWriteI2C(unsigned char data_out);

// Glyphs used to build big chars:
// Each of these 8 glyphs will be stored in the display as custom char
static const uint8_t bigcharGlyphs[8][8]= {
#define GlyphLT 0
    {0x07, 0x0f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f},
#define GlyphUB 1
    {0x1f, 0x1f, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00},
#define GlyphRT 2
    {0x1c, 0x1e, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f},
#define GlyphLL 3
    {0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x0f, 0x07},
#define GlyphLB 4
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x1f, 0x1f},
#define GlyphLR 5
    {0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1e, 0x1c},
#define GlyphUM 6
    {0x1f, 0x1f, 0x1f, 0x00, 0x00, 0x00, 0x1f, 0x1f},
#define GlyphLM 7
    {0x1f, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x1f, 0x1f}
};
// Big char defines:
// 10 chars, each defined out of 2 rows and 3 columns of glyphs:
static const uint8_t bigchars[10][6] = {
    // 0:
    {GlyphLT, GlyphUB, GlyphRT, GlyphLL, GlyphLB, GlyphLR},
    // 1:
    //{GlyphUB, GlyphRT,     ' ',     ' ',    0x1f,     ' '},
    {    ' ',    0x1f,    ' ',     ' ',    0x1f,     ' '},
    // 2:
    {GlyphUM, GlyphUM, GlyphRT, GlyphLL, GlyphLM, GlyphLM},
    // 3:
    {GlyphUM, GlyphUM, GlyphRT, GlyphLM, GlyphLM, GlyphLR},
    // 4:
    {GlyphLL, GlyphLB, GlyphRT,     ' ',     ' ',    0x1f},
    // 5:
    {   0x1f, GlyphUM, GlyphUM, GlyphLM, GlyphLM, GlyphLR},
    // 6:
    {GlyphLT, GlyphUM, GlyphUM, GlyphLL, GlyphLM, GlyphLR},
    // 7:
    {GlyphUB, GlyphUB, GlyphRT,     ' ', GlyphLT,     ' '},
    // 8:
    {GlyphLT, GlyphUM, GlyphRT, GlyphLL, GlyphLM, GlyphLR},
    // 9:
    {GlyphLT, GlyphUM, GlyphRT,     ' ',     ' ',    0x1f}
};

// return 1 if oled display is detected
uint8_t display_oled_init(void)
{
    uint8_t j;
    uint16_t i;
    int8_t err = 0;
    char oled_msg[5] = {'O', 'L', 'E', 'D', '?'};
    has_oled_display = 0;
    
    // Check for OLED display, try twice..
    for (j=0; j<2; j++) {
        StartI2C();
        err = myWriteI2C(OLED_addr);
        StopI2C();
        SelectA = 1;

        for (i=0; i<50; i++)
            ;

        if (!err) {
            has_oled_display = 1;
            break;
        }
    }

    if (has_oled_display) {
        oled_msg[4] = '!';
        oled_setup();

        for (j=0; j<8; j++)
            createChar( j, bigcharGlyphs[j]);
    }

    usb_write(oled_msg, (uint8_t) 5);

    return has_oled_display;
}

#if 0
void display_oled_string(uint8_t row, uint8_t col, const char *string)
{
    char c;
    uint16_t i;
    uint8_t inx = col + (row ? 0x40 : 0);

    StartI2C();
    myWriteI2C(OLED_addr);
    myWriteI2C(OLED_Command_Mode);
    myWriteI2C(OLED_DDRAMADDR | inx); // set cursor
    myWriteI2C(OLED_Data_Mode);

    while(c = *string++)
        myWriteI2C(c);

    StopI2C();
    SelectA = 1;
    for (i=0; i<50; i++)
        ;
}
#endif

static void oled_cmd_data(uint8_t cmd, uint8_t n, const uint8_t *data) {
    uint8_t i;

    StartI2C();
    myWriteI2C(OLED_addr);
    myWriteI2C(OLED_Command_Mode);
    myWriteI2C(cmd);

    if (n > 0) {
        myWriteI2C(OLED_Data_Mode);
        for (i=0; i<n; i++)
            myWriteI2C(data[i]);
    }
    StopI2C();
    SelectA = 1;
    for (i=0; i<50; i++)
        ;
}

void display_oled_chars(uint8_t row, uint8_t col, uint8_t len, const uint8_t chars[])
{
    uint8_t inx = col + (row ? 0x40 : 0);

    oled_cmd_data(OLED_DDRAMADDR | inx, len, chars);
}

void display_oled_bigchar(uint8_t col, uint8_t charnum) {
    if (charnum >= 10)
        return;

    display_oled_chars(0, col, 3, &bigchars[charnum][0]);
    display_oled_chars(1, col, 3, &bigchars[charnum][3]);
}

void display_oled_sleep(void) {
    display_oled_chars(0,0,16,"                ");
    display_oled_chars(1,0,16,"               .");
}

void display_oled_channel(uint8_t channel)
{
    static char chstr[2] = {'c', '-'};

    if (channel)
        chstr[1] = '0'+ channel;
    else
        chstr[1] = '-';
    display_oled_chars(0,14,2,chstr);
}

void display_oled_volume(uint8_t upperdigit, uint8_t lowdigit)
{
    display_oled_bigchar( 0, upperdigit);
    display_oled_bigchar( 4, lowdigit);
}

void display_oled_mute(void) {
    display_oled_chars(1,11,5,"muted");
}

void display_oled_unmute(void) {
    display_oled_chars(1,11,5,"     ");
}

static void createChar(uint8_t location, const uint8_t charmap[]) {
    oled_cmd_data(OLED_CGRAMADDR | (location << 3), 8, charmap);
}

static void oled_sendcommand(uint8_t command)
{
    uint16_t i;

    StartI2C();
    myWriteI2C(OLED_addr);
    myWriteI2C(OLED_Command_Mode);
    myWriteI2C(command);
    StopI2C();
    SelectA = 1;

    for (i=0; i<50; i++)
        ;
}

// Initialize oled display.
// Copied from example in https://github.com/gadjet/1602-OLED-Arduino-Library/blob/master/OLedI2C.cpp
static void oled_setup() {
    uint16_t i;
    uint8_t arg;

    oled_sendcommand(0x2A); // **** Set "RE"=1	00101010B

    //oled_sendcommand(0x71);
    //oled_sendcommand(0x5C); // JvE: needs repair, byte after 0x71 should be data mode!
    arg = 0x5c;
    oled_cmd_data(0x71, 1, &arg); // Enable internal Vdd regulator

    oled_sendcommand(0x28);

    oled_sendcommand(0x08); // **** Set Sleep Mode On
    oled_sendcommand(0x2A); // **** Set "RE"=1	00101010B
    oled_sendcommand(0x79); // **** Set "SD"=1	01111001B

    oled_sendcommand(0xD5);
    oled_sendcommand(0x70);
    oled_sendcommand(0x78); // **** Set "SD"=0  01111000B

    oled_sendcommand(0x08); // **** Set 5-dot, 3 or 4 line(0x09), 1 or 2 line(0x08)

    oled_sendcommand(0x06); // **** Set Com31-->Com0  Seg0-->Seg99
    arg = 0;
    oled_cmd_data(0x72, 1, &arg); // **** Set ROM A and 8 CGRAM

    // **** Set OLED Characterization *** //
    oled_sendcommand(0x2A); // **** Set "RE"=1
    oled_sendcommand(0x79); // **** Set "SD"=1

    // **** CGROM/CGRAM Management *** //
    // JvE: following 2 lines were wrong, moved up (SD=0) and data mode
    //oled_sendcommand(0x72); // **** Set ROM
    //oled_sendcommand(0x00); // **** Set ROM A and 8 CGRAM


    oled_sendcommand(0xDA); // **** Set Seg Pins HW Config
    oled_sendcommand(0x10);

    oled_sendcommand(0x81); // **** Set Contrast
    oled_sendcommand(0xFF);

    oled_sendcommand(0xDB); // **** Set VCOM deselect level
    oled_sendcommand(0x30); // **** VCC x 0.83

    oled_sendcommand(0xDC); // **** Set gpio - turn EN for 15V generator on.
    oled_sendcommand(0x03);

    oled_sendcommand(0x78); // **** Exiting Set OLED Characterization
    oled_sendcommand(0x28);
    oled_sendcommand(0x2A);
    //oled_sendcommand(0x05); 	// **** Set Entry Mode
    oled_sendcommand(0x06); // **** Set Entry Mode
    oled_sendcommand(0x08);
    oled_sendcommand(0x28); // **** Set "IS"=0 , "RE" =0 //28
    oled_sendcommand(0x01); // **** Clear display
    //oled_sendcommand(0x80); // **** Set DDRAM Address to 0x80 (line 1 start)

    for (i=0; i<10000; i++)
        ;

    oled_sendcommand(0x0C); // **** Turn on Display
}

