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
#include <flash.h>
#include "typedefs.h"
#include "storage.h"
#define FLASH_ERASED 0xFF
#define FLASH_EMPTY  0x00

// Number of flash pages to save my state. (Each page is 1KBytes)
// This number must be > 1 and a power-of-two
#define N_PAGES 2
static rom near StorageKey *current_page;
static rom near StorageKey *key_ptr[MAX_KEYS];
static unsigned char curr_page_nr;

static const unsigned char key_packet_sz[MAX_KEYS] = {16, 16, 16, 4};
static char packet_buf[16];

// Pages of flash memory to store Relaixed state
// Allocated to a page-aligned address in the linker script
#pragma romdata my_storage
static rom StorageKey storage_area[N_PAGES*FLASH_ERASE_BLOCK];
#pragma romdata

static unsigned long flash_new_page(void);

void storage_init(void)
{
	rom near StorageKey *p;
	StorageKey key;
	unsigned char i;
	unsigned int j;

	// select current active flash page
	// JvE: Need to update this 'if' in case N_PAGES > 2...
	if (storage_area[0] == FLASH_ERASED && storage_area[FLASH_ERASE_BLOCK] != FLASH_ERASED)
		curr_page_nr = 1;
	else
		curr_page_nr = 0;
	current_page = &storage_area[curr_page_nr * FLASH_ERASE_BLOCK];

	if (current_page[0] == FLASH_EMPTY)
	{
		// set to 0 by linker, erase to 0xFF
		EraseFlash((unsigned long)current_page, (unsigned long)(current_page + FLASH_ERASE_BLOCK - 1));
	}

	// fetch last written packet for each key (each type) from flash page
	for (i=0; i<MAX_KEYS; i++)
		key_ptr[i] = 0;

	for (j = 0;
		 j < FLASH_ERASE_BLOCK && (key = current_page[j]) != FLASH_ERASED;
		 j += key_packet_sz[key])
	{
		key_ptr[key] = current_page + j;
	}

	// key_ptr[0] is reserved to point to the next available location to store new data.
	// (Actual key numbers used as index in this table are always > 0.)
	// When over-flowing into a new page, assume it is erased already...
	if (j < FLASH_ERASE_BLOCK)
		key_ptr[0] = current_page + j;
	else
		key_ptr[0] = curr_page_nr ? storage_area : storage_area + FLASH_ERASE_BLOCK;
}

// Storing data into flash, must be in units of 16-bit words and to aligned address.
void flash_store(StorageKey key, const char *p)
{
	unsigned char i, n;
	WORD data;
	unsigned long rom_addr = (unsigned long)key_ptr[0];
	// The addrs in 'key_ptr' are multiples of 2 by construction.
	
	n = key_packet_sz[key];

	if ((rom_addr & (FLASH_ERASE_BLOCK-1)) + n >= FLASH_ERASE_BLOCK)
		rom_addr = flash_new_page();

	for (i=0; i<n; i += 2)
	{
		LSB(data) = *p++;
		MSB(data) = *p++;

		WriteWordFlash(rom_addr, data._word);
		rom_addr += 2;
	}
	// administrate the availability of this last packet of type 'key':
	key_ptr[key] = key_ptr[0];

	// and update the pointer to free space:
	key_ptr[0] = (rom near StorageKey *)rom_addr;
}

void flash_load(StorageKey key, char *p)
{
	unsigned char i, n;
	rom near StorageKey *rom_p;

	n = key_packet_sz[key];
	rom_p = key_ptr[key];
	
	if (rom_p)
	{
		ReadFlash((unsigned long)rom_p, (unsigned int) n, (unsigned char *)p);
	} else
	{
		*p++ = key;
		for (i = 1; i < n; i++)
			*p++ = (byte)0;
	}
}

// Open-up a new Flash page for storing a new key item.
// Return the addr of the free location in the new page
static unsigned long flash_new_page(void)
{
	unsigned char new_page_nr;
	rom near StorageKey *new_page;
	StorageKey key;

	// select new page
	new_page_nr = (curr_page_nr + 1) & (N_PAGES - 1); // N_PAGES is power-of-two
	new_page = storage_area + new_page_nr * FLASH_ERASE_BLOCK;
	
	// Erase new flash page if not yet clean
	if (new_page[0] != FLASH_ERASED)
		EraseFlash((unsigned long)new_page, (unsigned long)(new_page + FLASH_ERASE_BLOCK - 1));

	// Init new page with known key states (key 0 is dummy)
	key_ptr[0] = new_page;	
	for (key=1; key<MAX_KEYS; key++)
	{
		if (!key_ptr[key]) continue;

		flash_load(key, packet_buf);
		flash_store(key, packet_buf); // Note: this increments key_ptr[0]
	}

	// new page is initialised. Now erase old page so that it is not selected at a next reboot
	EraseFlash((unsigned long)current_page, (unsigned long)(current_page + FLASH_ERASE_BLOCK - 1));

	curr_page_nr = new_page_nr;
	current_page = new_page;

	return (unsigned long) key_ptr[0];
}
