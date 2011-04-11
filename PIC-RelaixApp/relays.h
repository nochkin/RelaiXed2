/*********************************************************************
 * Send state-updates to relay board(s) through an I2C connection
 * Also detect connected relay boards.
 ********************************************************************/

extern void relay_boards_init(void);
extern void set_relays(byte board_id, byte power,
					byte channel, byte vol_l, byte vol_r);

