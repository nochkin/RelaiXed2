/******************************************************************************
 * USB typedefs copied from ../PIC-bootload/usbmmap.h
 *****************************************************************************/
 
#ifndef USB_IO_H
#define USB_IO_H

#include "typedefs.h"

#define HID_INT_OUT_EP_SIZE     64
#define HID_INT_IN_EP_SIZE      64

extern void usb_write(const char *buffer, byte len);
extern byte usb_read(char *buffer, byte len);
extern byte usb_state(void);
void byte2hex( char *dest, unsigned char data);

#define hex(i) (i + (i<=9 ? '0' : 'a'-10))


#endif //USB_IO_H