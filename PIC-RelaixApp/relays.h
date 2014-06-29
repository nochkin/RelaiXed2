/*********************************************************************
 * Send state-updates to relay board(s) through an I2C connection
 * Also detect connected relay boards.
 ********************************************************************/
#include <stdint.h>

extern char relay_boards_init(void);
extern  void set_relays(uint8_t power, uint8_t channel, uint8_t vol_l, uint8_t vol_r);
#ifndef M_RELAY
extern uint8_t relayBoardType;
#endif

// Relaixed2 or -SMD with XLR audio and balance control
#define RELAIXED_XLR 0
// Relaixed single-ended passive board from March 2014
// leave 3 lsb bits for sub-versions...
#define RELAIXED_SE  8
#define isRelaixedXLR (RELAIXED_XLR == relayBoardType)
#define isRelaixedSE  (RELAIXED_SE  == relayBoardType)