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
#include "USBKeyboardConstants.h"

namespace usbdevice {

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

} /* namespace usbdevice */

#endif /* USBKEYBOARDDEVICE_H_ */
