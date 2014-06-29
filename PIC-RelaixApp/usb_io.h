/******************************************************************************
 * USB typedefs copied from ../PIC-bootload/usbmmap.h
 *****************************************************************************/
#include <stdint.h>

#ifndef USB_IO_H
#define USB_IO_H

#define HID_INT_OUT_EP_SIZE     64
#define HID_INT_IN_EP_SIZE      64

typedef uint8_t byte;

extern void usb_write(const char *buffer, uint8_t len);
extern uint8_t usb_read(char *buffer, uint8_t len);
extern uint8_t usb_state(void);
void byte2hex( char *dest, uint8_t data);

#define hex(i) ((i) + ((i)<=9 ? '0' : 'a'-10))


#endif //USB_IO_H