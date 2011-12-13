/*
 * Interface for keeping track of amplifier state variables
 */

/* volume_incr is modified from isr */
extern volatile char volume_incr;
extern volatile char channel_incr;
extern volatile char power_incr;
extern volatile char balance_incr;

extern void amp_state_init(void);
extern void volume_display(char override);
extern void volume_update(void);
extern void balance_update(void);
extern void channel_update(void);
extern void power_update(void);
extern void flash_volume_channel(void);
extern char power_state(void);
extern void channel_set( unsigned char new_ch);
extern void mute(void);
extern void unmute(void);
