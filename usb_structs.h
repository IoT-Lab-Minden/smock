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

#include "usblib/usbcdc.h"
#include "usblib/device/usbdcdc.h"
#include "usblib/device/usbdhidkeyb.h"
#include "usblib/device/usbdcomp.h"

// Globals used by both classes.
extern volatile uint32_t g_ui32USBFlags;
extern volatile uint32_t g_ui32SysTickCount;
extern volatile uint8_t g_ui8Buttons;

/**
 * The HID keyboard device initialization and customization structures.
 */

// The flags used by this application for the g_ulFlags value.
const uint8_t FLAG_MOVE_UPDATE = 0;
const uint8_t FLAG_CONNECTED = 1;
const uint8_t FLAG_LED_ACTIVITY = 2;
const uint8_t FLAG_MOVE_MOUSE = 3;
const uint8_t FLAG_COMMAND_RECEIVED = 4;
const uint8_t FLAG_SUSPENDED = 5;

extern const uint8_t g_pui8LangDescriptor[];
extern const uint8_t g_pui8ManufacturerString[];
extern const uint8_t g_pui8ProductString[];
extern const uint8_t g_pui8SerialNumberString[];
extern const uint8_t g_pui8ControlInterfaceString[];
extern const uint8_t g_pui8ConfigString[];
extern const uint8_t * const g_pui8StringDescriptors[];
extern const unsigned NUM_STRING_DESCRIPTORS;
#endif
