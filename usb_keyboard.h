/*
 * usb_keyboard.h
 *
 *  Created on: 01.12.2018
 *      Author: malte
 */
#ifndef USB_KEYBOARD_H
#define USB_KEYBOARD_H

#define HASHTAG 0x32

void USBKeyboardMain(void);

/**
 * Handle basic initialization of the USB keyboard.
 */
void USBKeyboardInit(void);

/**
 * Returns the current keyboard state
 */
bool IsKeyboardPending(void);

/**
 * Called by the UI interface to update the USB keyboard.
 *
 * \param ui8Modifiers is the set of key modifiers.
 * \param ui8Key is ASCII character to look up.
 * \param bPressed indicates if this is a press or release event.
 *
 * This function is used to update a key that has been pressed based on the
 * ASCII character that is passed in the \e ui8Key parameter.   The \e bPressed
 * parameter is \b true if the key was pressed and \b false if the key was
 * released.
 *
 * \return None.
 */
void USBKeyboardUpdate(uint8_t ui8Modifiers, uint8_t ui8UsageCode,
                       bool bPressed);

/**
 * Handle the callbacks from the USB library's HID keyboard layer.
 */
uint32_t USBKeyboardHandler(void *pvCBData, uint32_t ui32Event,
                            uint32_t ui32MsgData, void *pvMsgData);

/**
 * Handles all of the generic USB events.
 */
uint32_t USBEventHandler(void *pvCBData, uint32_t ui32Event,
                         uint32_t ui32MsgValue, void *pvMsgData);

/**
 * Calls USBKeyboardUpdate for every char in the string
 */
void USBWriteString(const char *string, int length);
#endif
