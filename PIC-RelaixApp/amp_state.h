/*
 * Interface for keeping track of amplifier state variables
 */
#include <stdint.h>

/* volume_incr is modified from isr */
extern volatile int8_t volume_incr;
extern volatile int8_t channel_incr;
extern volatile int8_t power_incr;
extern volatile int8_t balance_incr;

extern void amp_state_init(void);
extern void volume_display(uint8_t override);
extern void volume_update(void);
extern void balance_update(void);
extern void channel_update(void);
extern void power_update(void);
extern void flash_volume_channel(void);
extern uint8_t power_state(void);
extern void channel_set( uint8_t new_ch);
extern void mute(void);
extern void unmute(void);
