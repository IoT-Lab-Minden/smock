/*
 * USBKeyboardDevice.h
 *
 *  Created on: 06.12.2018
 *      Author: malte
 */

#ifndef USBKEYBOARDDEVICE_H_
#define USBKEYBOARDDEVICE_H_

#include <stdbool.h>
#include <stdint.h>
#include "driverlib/gpio.h"
#include "grlib/grlib.h"
#include "grlib/widget.h"
#include "grlib/keyboard.h"
#include "driverlib/rom.h"
#include "usblib/usblib.h"
#include "usblib/usbhid.h"
#include "usblib/usbcdc.h"
#include "usblib/usb-ids.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdcomp.h"
#include "usblib/device/usbdhid.h"
#include "usblib/device/usbdhidmouse.h"
#include "usblib/device/usbdhidkeyb.h"
#include "usb_structs.h"
#include "global_defs.h"

namespace usbdevice {
	const uint8_t HASHTAG = 0x32;
	const uint8_t HOME = 0x24;
	const uint8_t DEL = 0x4c;

	/**
	 * Global USB keyboard state.
	 */
	struct {
		// Holds a pending special key press for the Caps Lock, Scroll Lock, or
		// Num Lock keys.
		uint8_t ui8Special;
		uint8_t pending;
	}g_sKeyboardState;

	/**
	 * The look up table entries for usage codes.
	 */
	typedef struct {
		char cChar;
		char cUsage;
	} sUsageEntry;

	class USBKeyboardDevice {
	private:
		tUSBDHIDKeyboardDevice g_sKeyboardDevice;

		uint8_t GetUsageCode(char cKey, bool bShifted);

		static tUSBCallback keyboardHandler;


		USBKeyboardDevice(tUSBCallback keyboardHandler);

	public:
		static USBKeyboardDevice *getInstance();

		static void registerKeyboardHandler(tUSBCallback keyboardHandler);

		tUSBDHIDKeyboardDevice *getKeyboardDevice();
		/**
		 * Handle basic initialization of the USB keyboard.
		 */
		void USBKeyboardInit(void);

		/**
		 * Release the currently pressed special key
		 */
		void releaseSpecialKey();

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
		 * Calls USBKeyboardUpdate for every char in the string
		 */
		void USBWriteString(const char *string, int length);

		/**
		 * Presses all key at the same time
		 */
		void USBPressKeyCombination(const uint8_t modifiers, const uint8_t *string, int length);
	};

	/**
	 * The un-shifted HID usage codes used by the graphical keyboard.
	 */
	const sUsageEntry g_pcUsageCodes[] = { {'q', HID_KEYB_USAGE_Q}, {
			'w', HID_KEYB_USAGE_W},
		{	'e', HID_KEYB_USAGE_E}, {
			'r', HID_KEYB_USAGE_R},
		{	't', HID_KEYB_USAGE_T}, {
			'y', HID_KEYB_USAGE_Y},
		{	'u', HID_KEYB_USAGE_U}, {
			'i', HID_KEYB_USAGE_I},
		{	'o', HID_KEYB_USAGE_O}, {
			'p', HID_KEYB_USAGE_P},
		{	'a', HID_KEYB_USAGE_A}, {
			's', HID_KEYB_USAGE_S},
		{	'd', HID_KEYB_USAGE_D}, {
			'f', HID_KEYB_USAGE_F},
		{	'g', HID_KEYB_USAGE_G}, {
			'h', HID_KEYB_USAGE_H},
		{	'j', HID_KEYB_USAGE_J}, {
			'k', HID_KEYB_USAGE_K},
		{	'l', HID_KEYB_USAGE_L}, {
			'z', HID_KEYB_USAGE_Z},
		{	'x', HID_KEYB_USAGE_X}, {
			'c', HID_KEYB_USAGE_C},
		{	'v', HID_KEYB_USAGE_V}, {
			'b', HID_KEYB_USAGE_B},
		{	'n', HID_KEYB_USAGE_N}, {
			'm', HID_KEYB_USAGE_M},
		{	'0', HID_KEYB_USAGE_0}, {
			'1', HID_KEYB_USAGE_1},
		{	'2', HID_KEYB_USAGE_2}, {
			'3', HID_KEYB_USAGE_3},
		{	'4', HID_KEYB_USAGE_4}, {
			'5', HID_KEYB_USAGE_5},
		{	'6', HID_KEYB_USAGE_6}, {
			'7', HID_KEYB_USAGE_7},
		{	'8', HID_KEYB_USAGE_8}, {
			'9', HID_KEYB_USAGE_9},
		{	'-', HID_KEYB_USAGE_MINUS},
		{	'=', HID_KEYB_USAGE_EQUAL}, {
			'\'',
			HID_KEYB_USAGE_FQUOTE},
		{	'[', HID_KEYB_USAGE_LBRACKET},
		{	']', HID_KEYB_USAGE_RBRACKET},
		{	';', HID_KEYB_USAGE_SEMICOLON},
		{	' ', HID_KEYB_USAGE_SPACE},
		{	'/', HID_KEYB_USAGE_FSLASH}, {
			'\\',
			HID_KEYB_USAGE_BSLASH},
		{	'.', HID_KEYB_USAGE_PERIOD},
		{	',', HID_KEYB_USAGE_COMMA}, {
			UI_CAPS_LOCK,
			HID_KEYB_USAGE_CAPSLOCK},
		{	UI_SCROLL_LOCK,
			HID_KEYB_USAGE_SCROLLOCK},
		{	UI_NUM_LOCK,
			HID_KEYB_USAGE_NUMLOCK},
		{	UNICODE_BACKSPACE,
			HID_KEYB_USAGE_BACKSPACE},
		{	UNICODE_RETURN,
			HID_KEYB_USAGE_ENTER},};

	const uint32_t ui32NumUsageCodes = sizeof(g_pcUsageCodes)
	/ sizeof(sUsageEntry);

	/**
	 * The shifted HID usage codes that are used by the graphical keyboard.
	 */
	const sUsageEntry g_pcUsageCodesShift[] = {
		{	')', HID_KEYB_USAGE_0}, {'!', HID_KEYB_USAGE_1},
		{	'@', HID_KEYB_USAGE_2}, {'#', HID_KEYB_USAGE_3},
		{	'$', HID_KEYB_USAGE_4}, {'%', HID_KEYB_USAGE_5},
		{	'^', HID_KEYB_USAGE_6}, {'&', HID_KEYB_USAGE_7},
		{	'*', HID_KEYB_USAGE_8}, {'(', HID_KEYB_USAGE_9}, {
			'?', HID_KEYB_USAGE_FSLASH},
		{	'+', HID_KEYB_USAGE_EQUAL}, {':', HID_KEYB_USAGE_SEMICOLON}, {
			'_', HID_KEYB_USAGE_MINUS},
		{	'~', HID_KEYB_USAGE_BQUOTE}, {'|', HID_KEYB_USAGE_BSLASH}, {
			'\"', HID_KEYB_USAGE_FQUOTE},};

	const uint32_t ui32NumUsageCodesShift = sizeof(g_pcUsageCodes)
	/ sizeof(sUsageEntry);

} /* namespace usbdevice */

#endif /* USBKEYBOARDDEVICE_H_ */
