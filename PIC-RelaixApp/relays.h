/*********************************************************************
 * Send state-updates to relay board(s) through an I2C connection
 * Also detect connected relay boards.
 ********************************************************************/
#include <stdint.h>

extern char relay_boards_init(void);
extern  void set_relays(uint8_t power, uint8_t channel, uint8_t vol_l, uint8_t vol_r);
extern uint8_t i2c_probe( uint8_t chip_addr); // returns 0 if present
extern uint8_t i2c_read( uint8_t chip_addr, uint8_t regno); // reads single (1-byte) register
extern uint8_t i2c_write1( uint8_t chip_addr, uint8_t regno, uint8_t data); // returns 0 if OK
extern uint8_t myWriteI2C(unsigned char data_out); // low-level, I had rather not exported this...
#ifndef M_RELAY
extern uint8_t relayBoardType;
#endif

// Relaixed2 or -SMD with XLR audio and balance control
#define RELAIXED_XLR 0
// Relaixed single-ended passive board from March 2014
// leave 3 lsb bits for sub-versions...
#define RELAIXED_SE  8
// RELAIXED_SE2 is RELAIXED_SE with an extra (2nd) relay board, allows balance
#define RELAIXED_SE2  9
// RELAIXED_SE3 is RELAIXED_SE with two extra relay boards, first two do balance
#define RELAIXED_SE3   10
#define isRelaixedXLR  (RELAIXED_XLR == relayBoardType)
#define isRelaixedSE   (RELAIXED_SE  == relayBoardType)
#define isRelaixedSE2  (RELAIXED_SE2  == relayBoardType)
#define isRelaixedSE3  (RELAIXED_SE3  == relayBoardType)
#define isRelaixedSEx  (RELAIXED_SE  == (relayBoardType & RELAIXED_SE))