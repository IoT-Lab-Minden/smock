//*****************************************************************************
//
// usb_structs.h - Data structures defining the composite mouse and keyboard
// USB device.
//
// Copyright (c) 2013-2015 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
//
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
//
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
//
// This is part of revision 2.1.1.71 of the DK-TM4C129X Firmware Package.
//
//*****************************************************************************

#ifndef __USB_STRUCTS_H__
#define __USB_STRUCTS_H__

#include "usblib/device/usbdcdc.h"
#include "usblib/device/usbdhidkeyb.h"

// Globals used by both classes.
extern volatile uint32_t g_ui32USBFlags;
extern volatile uint32_t g_ui32SysTickCount;
extern volatile uint8_t g_ui8Buttons;

/**
 * The HID keyboard device initialization and customization structures.
 */
extern tUSBDHIDKeyboardDevice g_sKeyboardDevice;

// The flags used by this application for the g_ulFlags value.
#define FLAG_MOVE_UPDATE       0
#define FLAG_CONNECTED         1
#define FLAG_LED_ACTIVITY      2
#define FLAG_MOVE_MOUSE        3
#define FLAG_COMMAND_RECEIVED  4
#define FLAG_SUSPENDED         5

/**
 * Size of the Rx and Tx buffer
 */
#define UART_BUFFER_SIZE 256

/**
 * Transmit buffer (from the USB perspective).
 */
extern tUSBBuffer g_sTxBuffer;
extern uint8_t g_pui8USBTxBuffer[];

/**
 * Receive buffer (from the USB perspective).
 */
extern tUSBBuffer g_sRxBuffer;
extern uint8_t g_pui8USBRxBuffer[];


/**
 * The CDC device initialization and customization structures. In this case,
 * we are using USBBuffers between the CDC device class driver and the
 * application code. The function pointers and callback data values are set
 * to insert a buffer in each of the data channels, transmit and receive.
 *
 * With the buffer in place, the CDC channel callback is set to the relevant
 * channel function and the callback data is set to point to the channel
 * instance data. The buffer, in turn, has its callback set to the application
 * function and the callback data set to our CDC instance structure.
 */
extern tUSBDCDCDevice g_sCDCDevice;

#endif
