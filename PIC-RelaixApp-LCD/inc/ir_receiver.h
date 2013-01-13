/******************************************************************************
 * Module to receive and decode various types of IR commands
 *
 * Copyright 2011  Jos van Eijndhoven
 *****************************************************************************/

extern void ir_receiver_init(void);
extern char ir_tmr_isr(void);
extern void ir_receiver_isr(void);
extern void ir_handle_code(void);
