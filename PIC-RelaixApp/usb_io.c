/******************************************************************************
 * USB typedefs copied from ../PIC-bootload/usbmmap.h
 * Data struct definition of BufferDescriptor as shared with USB hardware engine
 *****************************************************************************/
#include <p18cxxx.h>
#include "typedefs.h"
 
typedef union _BD_STAT
{
    byte _byte;
    struct{
        unsigned BC8:1;
        unsigned BC9:1;
        unsigned BSTALL:1;              //Buffer Stall Enable
        unsigned DTSEN:1;               //Data Toggle Synch Enable
        unsigned INCDIS:1;              //Address Increment Disable
        unsigned KEN:1;                 //BD Keep Enable
        unsigned DTS:1;                 //Data Toggle Synch Value
        unsigned UOWN:1;                //USB Ownership
    };
    struct{
        unsigned BC8:1;
        unsigned BC9:1;
        unsigned PID0:1;
        unsigned PID1:1;
        unsigned PID2:1;
        unsigned PID3:1;
        unsigned :1;
        unsigned UOWN:1;
    };
    struct{
        unsigned :2;
        unsigned PID:4;                 //Packet Identifier
        unsigned :2;
    };
} BD_STAT;                              //Buffer Descriptor Status Register

typedef union _BDT
{
    struct
    {
        BD_STAT Stat;
        byte Cnt;
        byte ADRL;                      //Buffer Address Low
        byte ADRH;                      //Buffer Address High
    };
    struct
    {
        unsigned :8;
        unsigned :8;
        byte* ADR;                      //Buffer Address
    };
} BDT;                                  //Buffer Descriptor Table

/* Buffer Descriptor Status Register Initialization Parameters */
#define _BSTALL     0x04                //Buffer Stall enable
#define _DTSEN      0x08                //Data Toggle Synch enable
#define _INCDIS     0x10                //Address increment disable
#define _KEN        0x20                //SIE keeps buff descriptors enable
#define _DAT0       0x00                //DATA0 packet expected next
#define _DAT1       0x40                //DATA1 packet expected next
#define _DTSMASK    0x40                //DTS Mask
#define _USIE       0x80                //SIE owns buffer
#define _UCPU       0x00                //CPU owns buffer

/******************************************************************************
 * Basic settings copied from ../PIC-bootload/usbcfg.h
 * Addresses are read from map file of bootloader build result
 *****************************************************************************/
 
#define HID_BD_OUT              ep1Bo
#define HID_INT_OUT_EP_SIZE     64
#define HID_BD_IN               ep1Bi
#define HID_INT_IN_EP_SIZE      64

volatile far BDT ep1Bi =  0x00040c;
volatile far BDT ep1Bo =  0x000408;
#define  hid_report_in  (HID_BD_IN.ADR)
#define  hid_report_out (HID_BD_OUT.ADR)

/******************************************************************************
 * Macro:           void mUSBBufferReady(buffer_dsc)
 *
 * PreCondition:    IN Endpoint: Buffer is loaded and ready to be sent.
 *                  OUT Endpoint: Buffer is free to be written to by SIE.
 *
 * Input:           byte buffer_dsc: Root name of the buffer descriptor group.
 *                  i.e. ep0Bo, ep1Bi, ... Declared in usbmmap.c
 *                  Names can be remapped for readability, see examples in
 *                  usbcfg.h (#define HID_BD_OUT      ep1Bo)
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This macro should be called each time after:
 *                  1. A non-EP0 IN endpoint buffer is populated with data.
 *                  2. A non-EP0 OUT endpoint buffer is read.
 *                  This macro turns the buffer ownership to SIE for servicing.
 *                  It also toggles the DTS bit for synchronization.
 *
 * Note:            None
 *****************************************************************************/
#define mUSBBufferReady(buffer_dsc)                                         \
{                                                                           \
    buffer_dsc.Stat._byte &= _DTSMASK;          /* Save only DTS bit */     \
    buffer_dsc.Stat.DTS = !buffer_dsc.Stat.DTS; /* Toggle DTS bit    */     \
    buffer_dsc.Stat._byte |= _USIE|_DTSEN;      /* Turn ownership to SIE */ \
}

/******************************************************************************
 * Macro:           (bit) mHIDRxIsBusy(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This macro is used to check if HID OUT endpoint is
 *                  busy (owned by SIE) or not.
 *                  Typical Usage: if(mHIDRxIsBusy())
 *
 * Note:            None
 *****************************************************************************/
#define mHIDRxIsBusy()              HID_BD_OUT.Stat.UOWN

/******************************************************************************
 * Macro:           (bit) mHIDTxIsBusy(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This macro is used to check if HID IN endpoint is
 *                  busy (owned by SIE) or not.
 *                  Typical Usage: if(mHIDTxIsBusy())
 *
 * Note:            None
 *****************************************************************************/
#define mHIDTxIsBusy()              HID_BD_IN.Stat.UOWN


/******************************************************************************
 * Function:        void HIDTxReport(char *buffer, byte len)
 *
 * PreCondition:    mHIDTxIsBusy() must return false.
 *
 *                  Value of 'len' must be equal to or smaller than
 *                  HID_INT_IN_EP_SIZE
 *                  For an interrupt endpoint, the largest buffer size is
 *                  64 bytes.
 *
 * Input:           buffer  : Pointer to the starting location of data bytes
 *                  len     : Number of bytes to be transferred
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Use this macro to transfer data located in data memory.
 *
 *                  Remember: mHIDTxIsBusy() must return false before user
 *                  can call this function.
 *                  Unexpected behavior will occur if this function is called
 *                  when mHIDTxIsBusy() == 0
 *
 *                  Typical Usage:
 *                  if(!mHIDTxIsBusy())
 *                      HIDTxReport(buffer, 3);
 *
 * Note:            None
 *****************************************************************************/
static void HIDTxReport(char *buffer, byte len)
{
	byte i;
	
    /*
     * Value of len should be equal to or smaller than HID_INT_IN_EP_SIZE.
     * This check forces the value of len to meet the precondition.
     */
	if(len > HID_INT_IN_EP_SIZE)
	    len = HID_INT_IN_EP_SIZE;

   /*
    * Copy data from user's buffer to dual-ram buffer
    */
    for (i = 0; i < len; i++)
    	hid_report_in[i] = buffer[i];

    HID_BD_IN.Cnt = len;
    mUSBBufferReady(HID_BD_IN);

}//end HIDTxReport


/******************************************************************************
 * Function:        byte HIDRxReport(char *buffer, byte len)
 *
 * PreCondition:    Value of input argument 'len' should be smaller than the
 *                  maximum endpoint size responsible for receiving report
 *                  data from USB host for HID class.
 *                  Input argument 'buffer' should point to a buffer area that
 *                  is bigger or equal to the size specified by 'len'.
 *
 * Input:           buffer  : Pointer to where received bytes are to be stored
 *                  len     : The number of bytes expected.
 *
 * Output:          The number of bytes copied to buffer.
 *
 * Side Effects:    Publicly accessible variable hid_rpt_rx_len is updated
 *                  with the number of bytes copied to buffer.
 *                  Once HIDRxReport is called, subsequent retrieval of
 *                  hid_rpt_rx_len can be done by calling macro
 *                  mHIDGetRptRxLength().
 *
 * Overview:        HIDRxReport copies a string of bytes received through
 *                  USB HID OUT endpoint to a user's specified location. 
 *                  It is a non-blocking function. It does not wait
 *                  for data if there is no data available. Instead it returns
 *                  '0' to notify the caller that there is no data available.
 *
 * Note:            If the actual number of bytes received is larger than the
 *                  number of bytes expected (len), only the expected number
 *                  of bytes specified will be copied to buffer.
 *                  If the actual number of bytes received is smaller than the
 *                  number of bytes expected (len), only the actual number
 *                  of bytes received will be copied to buffer.
 *****************************************************************************/

static byte HIDRxReport(char *buffer, byte len)
{
    byte hid_rpt_rx_len = 0;  // JvE: added '= 0'
    
    if(!mHIDRxIsBusy())
    {
        /*
         * Adjust the expected number of bytes to equal
         * the actual number of bytes received.
         */
        if(len > HID_BD_OUT.Cnt)
            len = HID_BD_OUT.Cnt;
        
        /*
         * Copy data from dual-ram buffer to user's buffer
         */
        for(hid_rpt_rx_len = 0; hid_rpt_rx_len < len; hid_rpt_rx_len++)
            buffer[hid_rpt_rx_len] = hid_report_out[hid_rpt_rx_len];

        /*
         * Prepare dual-ram buffer for next OUT transaction
         */
        HID_BD_OUT.Cnt = sizeof(hid_report_out);
        mUSBBufferReady(HID_BD_OUT);
    }//end if
    
    return hid_rpt_rx_len;
    
}//end HIDRxReport

/******************************************************************************
 * Finally my own read/write functions....
 *****************************************************************************/

byte usb_read(char *buffer, byte len)
{
	unsigned short i;
	
	if (!UCONbits.USBEN)
		return 0;
	
	for (i=0; mHIDRxIsBusy() && i < 1000; i++)
		; // wait a limited time for buffer access
		// Why !!$#$#!# did Microchip not support USB pingpong buffers in their firmware?
		
    return HIDRxReport(buffer, len);
}

void usb_write( char *buffer, byte len)
{
	unsigned short i;
	
	if (!UCONbits.USBEN)
		return;

	for (i=0; mHIDTxIsBusy() && i < 1000; i++)
		; // wait a limited time for buffer access
		// Why !!$#$#!# did Microchip not support USB pingpong buffers in their firmware?
	
	if(!mHIDTxIsBusy())
 		HIDTxReport(buffer, len);
}	