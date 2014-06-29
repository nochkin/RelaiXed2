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
/**********************************************************************
 * Provide a non-volatile storage mechanism,
 * A] to save state like volume and channel when power is switched-off
 * B] to save IR receiver protocol choices
 *
 * Unfortunately, this device does not provide EEPROM storage.
 * We must use flash memory, wich can be erased in 1K pages only
 * Also use larger memory to avoid too frequent overwriting.
 * (For this PIC, only 10K times overwritable...)
 *********************************************************************/

/*
 * Key 0: white-space filler, used to fill last bytres to end-of-page
 * Key 1: IR protocol & key ids, 1 header byte + 15 data bytes
 * Key 2: input channel volumes: 1 header byte, 7 volumes
 *        & board volumes: 1 header byte, 7 volumes
 * Key 3: 1 head byte, +3: master volume, channel select, LR-balance
 */
#include <stdint.h>
#include <plib/flash.h>
#include "storage.h"

// flash device properties
#define FLASH_ERASED 0xFF
#define FLASH_EMPTY  0x00
#define FLASH_PAGESIZE 1024
#if FLASH_ERASE_BLOCK > FLASH_PAGESIZE
  error in flash configuration!
#endif

// Number of flash pages to save my state. (Each page is 1KBytes)
// This number must be > 1 and a power-of-two
#define N_PAGES 2
// pointers into flash memory space, not directly writable
static const StorageKey *current_page;
static const StorageKey *key_ptr[MAX_KEYS];
static unsigned char curr_page_nr;

static const unsigned char key_packet_sz[MAX_KEYS] = {16, 4, 16, 4};
static union
{
	char chars[16];
	uint16_t words[8];
} packet_buf;

// Pages of flash memory to store Relaixed state
// Allocated to a 1K page-aligned address
const StorageKey storage_area[N_PAGES*FLASH_PAGESIZE] @ 0x7400;

static unsigned long flash_new_page(void);

// find pointers into flash space that store Relaixed state variables
void storage_init(void)
{
	StorageKey key;
	unsigned char i, n;
	unsigned int j;

	// select current active flash page
	// JvE: Need to update this 'if' in case N_PAGES > 2...
	key = storage_area[FLASH_PAGESIZE]; // start of page 1
	if (key == FLASH_EMPTY || key == FLASH_ERASED)
		curr_page_nr = 0; // try to append to (partially filled) page 0
	else
		curr_page_nr = 1;
	current_page = &storage_area[curr_page_nr * FLASH_PAGESIZE];
	key = current_page[0]; //first item in currrent page

	if (key == FLASH_EMPTY)
	{
		// set to 0 by linker, erase to 0xFF
		EraseFlash((unsigned long)current_page, (unsigned long)(current_page + FLASH_PAGESIZE - 1));
		EECON1bits.WREN = 0;
	}

	// fetch last written packet for each key (each type) from flash page
	for (i=0; i<MAX_KEYS; i++)
		key_ptr[i] = 0;

	for (j = 0;
		 j < FLASH_PAGESIZE && (key = current_page[j]) != FLASH_ERASED;
		 j += n)
	{
		if (key > 0 && key < MAX_KEYS)
		{
			key_ptr[key] = current_page + j;
			n = key_packet_sz[key];
		} else
			n = 4; // hm... got error, weird key value 
	}

	// key_ptr[0] is reserved to point to the next available location to store new data.
	// (Actual key numbers used as index in this table are always > 0.)
	// When over-flowing into a new page, assume it is erased already...
	if (j < FLASH_PAGESIZE)
		key_ptr[0] = current_page + j;
	else
		key_ptr[0] = curr_page_nr ? storage_area : storage_area + FLASH_PAGESIZE;
}

// Storing data into flash, must be in units of 16-bit words and to aligned address.
static void flash_store_page(StorageKey key, const unsigned int *w)
{
	unsigned char i, n, n_words;
	unsigned long rom_addr = (unsigned long)key_ptr[0];
	// The addrs in 'key_ptr' are multiples of 2 by construction.
	
	n = key_packet_sz[key];
	n_words = n >> 1;

        // will fit: avoid recursive call
	// if ((rom_addr & (FLASH_PAGESIZE-1)) + n >= FLASH_PAGESIZE)
	//	rom_addr = flash_new_page();

	for (i = 0; i < n_words; i++)
	{
		WriteWordFlash(rom_addr, *w++);
		rom_addr += 2;
	}
	EECON1bits.WREN = 0;

	// administrate the availability of this last packet of type 'key':
	key_ptr[key] = key_ptr[0];

	// and update the pointer to free space:
	key_ptr[0] = (const StorageKey *)rom_addr;
}

void flash_store(StorageKey key, const unsigned int *w)
{
	unsigned char i, n, n_words;
	unsigned long rom_addr = (unsigned long)key_ptr[0];
	// The addrs in 'key_ptr' are multiples of 2 by construction.

	n = key_packet_sz[key];

	if ((rom_addr & (FLASH_PAGESIZE-1)) + n >= FLASH_PAGESIZE)
		rom_addr = flash_new_page();

        flash_store_page(key, w);
}

void flash_load(StorageKey key, char *p)
{
	unsigned char i, n;
	const StorageKey *rom_p;

	n = key_packet_sz[key];
	rom_p = key_ptr[key];
	
	if (rom_p)
	{
		ReadFlash((unsigned long)rom_p, (unsigned int) n, (unsigned char *)p);
	} else
	{
		*p++ = key;
		for (i = 1; i < n; i++)
			*p++ = (uint8_t)0;
	}
}

// Open-up a new Flash page for storing a new key item.
// Return the addr of the free location in the new page
static unsigned long flash_new_page(void)
{
	unsigned char new_page_nr;
	const StorageKey *new_page;
	StorageKey key;

	// select new page
	new_page_nr = (curr_page_nr + 1) & (N_PAGES - 1); // N_PAGES is power-of-two
	new_page = storage_area + new_page_nr * FLASH_PAGESIZE;
	
	// Erase new flash page if not yet clean
	if (new_page[0] != FLASH_ERASED)
		EraseFlash((unsigned long)new_page, (unsigned long)(new_page + FLASH_PAGESIZE - 1));

	// Init new page with known key states (key 0 is dummy)
	key_ptr[0] = new_page;	
	for (key=1; key<MAX_KEYS; key++)
	{
		if (!key_ptr[key]) continue;

		flash_load(key, packet_buf.chars);
		flash_store_page(key, packet_buf.words); // Note: this increments key_ptr[0]
	}

	// new page is initialised. Now erase old page so that it is not selected at a next reboot
	EraseFlash((unsigned long)current_page, (unsigned long)(current_page + FLASH_PAGESIZE - 1));
	EECON1bits.WREN = 0;

	curr_page_nr = new_page_nr;
	current_page = new_page;

	return (unsigned long) key_ptr[0];
}
