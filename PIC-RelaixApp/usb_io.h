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


#endif //USB_IO_H