/*
 * Interface for keeping track of amplifier state variables
 */

/* volume_incr is modified from isr */
extern volatile char volume_incr;
extern volatile char channel_incr;

extern void amp_state_init(void);
extern void volume_update(void);
extern void channel_update(void);
