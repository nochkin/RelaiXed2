/**********************************************************************
 * Provide a non-volatile storage mechanism,
 * A] to save state like volume and channel when power is switched-off
 * B] to save IR receiver protocol choices
 *********************************************************************/
#define MAX_KEYS 5

typedef enum
{
	KeyFill = 0,
	KeyIR,
	KeyLevels,
	KeyVolume
} StorageKey;

extern void storage_init(void);
extern void flash_store(StorageKey key, const unsigned int *w);
extern void flash_load(StorageKey key, char *p);
