/**
 * \file USBKeyboardDevice.h
 * \brief Contains the declaration of the methods and attributes needed for the keyboard device.
 *
 * The device allows the micro controller to simulate a keyboard.
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
#include "USBKeyboardConstants.h"

namespace usbdevice {

	/**
	 * \class USBKeyboardDevice
	 * \brief Device simulating a keyboard
	 *
	 * A human interface device that simulates a keyboard. The class provides several methods
	 * to initialize the device and to send key (combinations) to the host.
	 * The keyboard device is realized by a singelton pattern because the micro controller is only
	 * able to have one keyboard device active at the same time.
	 */
	class USBKeyboardDevice {
	private:
		/**
		 * The Keyboard device
		 */
		tUSBDHIDKeyboardDevice g_sKeyboardDevice;


		/**
		 * Callback function which is called when an event is raised by the host.
		 */
		static tUSBCallback keyboardHandler;

		/**
		 * \brief Constructor which builds the object.
		 * \param keyboardHandler The callback function that should be set.
		 *
		 * The constructor initializes the g_sKeyboardDevice of the object.
		 */
		USBKeyboardDevice(tUSBCallback keyboardHandler);

	public:

		/**
		 * \fn uint8_t GetUsageCode(char cKey, bool bShifted)
		 * \brief Method to get the usage code of a char.
		 *
		 * \param cKey Char to get the key from
		 * \param bShifted Indicates weather the char is a shifted (big) char.
		 *
		 * Method to get the usage code of a char. (Keyboard code standing for this char)
		 *
		 * \return uint8_t - Usage code of this char.
		 */
		uint8_t GetUsageCode(char cKey, bool bShifted);

		/**
		 * \fn USBKeyboardDevice *getInstance()
		 * \brief Returns the instance of the USBKeyboardDevice
		 *
		 * The function creates the object the first time it is called and return the pointer
		 * to the object. The object is created on the heap. Later calls only return the
		 * pointer to this object.
		 *
		 * \return - the instance of keyboard device
		 */
		static USBKeyboardDevice *getInstance();

		/**
		 * \fn void registerKeyboardHandler(tUSBCallback keyboardHandler)
		 * \brief Registers a new Callback function for the keyboard handler.
		 *
		 * \param keyboardHandler New callback method
		 *
		 * The handler is called when an event is raised by the host.
		 * A new handler can only be set before the device is referenced the first time.
		 * When the device is referenced the first time, the composite device struct is
		 * initialized with the current callback method by calling the constructor of the
		 * composite device. After that there is no possibility to change the callback
		 * method. It can only be one function registered.
		 *
		 * \return void
		 */
		static void registerKeyboardHandler(tUSBCallback keyboardHandler);

		/**
		 * \fn tUSBDHIDKeyboardDevice *getKeyboardDevice()
		 * \brief Getter for the g_sKeyboardDevice attribute
		 *
		 * Returns the g_sKeyboardDevice.
		 *
		 * \return tUSBDHIDKeyboardDevice - the struct standing for the keyboard device.
		 */
		tUSBDHIDKeyboardDevice *getKeyboardDevice();

		/**
		 * \fn void USBKeyboardInit()
		 * \brief Handle basic initialization of the USB keyboard.
		 *
		 * The device is initialized by clearing out the special key variable.
		 *
		 * \return void
		 */
		void USBKeyboardInit();

		/**
		 * \fn void releaseSpecialKey()
		 * \brief Release the currently pressed special key.
		 *
		 * The currently pressed special key stored in g_sKeyboardState is releades by calling the
		 * USBDHIDKeyboardKeyStateChange() function. The key is set to 0 afterwards.
		 *
		 * \return void
		 */
		void releaseSpecialKey();

		/**
		 * \fn bool IsKeyboardPending(void)
		 * \brief Returns the current keyboard state.
		 *
		 * Returns the current keyboard state stored in g_sKeyboardState.
		 *
		 * \return bool - The current keyboard state
		 */
		bool IsKeyboardPending();

		/**
		 * \fn void USBKeyboardUpdate(uint8_t ui8Modifiers, uint8_t ui8UsageCode, bool bPressed)
		 * \brief Called by the UI interface to update the USB keyboard.
		 *
		 * \param ui8Modifiers The set of key modifiers.
		 * \param ui8UsageCode The ASCII character to look up.
		 * \param bPressed Indicates if this is a press or release event.
		 *
		 * This function is used to update a key that has been pressed based on the
		 * ASCII character that is passed in the ui8UsageCode parameter. The bPressed
		 * parameter is true if the key was pressed and false if the key was
		 * released.
		 *
		 * \return void
		 */
		void USBKeyboardUpdate(uint8_t ui8Modifiers, uint8_t ui8UsageCode,
				bool bPressed);

		/**
		 * \fn void USBWriteString(const char *string, int length)
		 * \brief Every key representation of the ASCII characters is pressed and released.
		 *
		 * \param string String of ASCII characters representing the key that should be pressed.
		 * \param length length of string array.
		 *
		 * Calls USBKeyboardUpdate for every char in the string to press the key. Before the next
		 * char is pressed, the key before that is released.
		 *
		 * \return void
		 */
		void USBWriteString(const char *string, int length);

		/**
		 * \fn void USBPressKeyCombination(const uint8_t modifiers, const uint8_t *string, int length)
		 * \brief Sends a single key combination to the host.
		 *
		 * \param modifiers special keys like "Alt" or "Del".
		 * \param string String of ASCII characters representing the key that should be pressed.
		 * \param length length of string array.
		 *
		 * Presses all keys at the same time by calling USBKeyboardUpdate. After all keys have been
		 * pressed they are all released.
		 *
		 * \return void
		 */
		void USBPressKeyCombination(const uint8_t modifiers, const uint8_t *string, int length);
	};

} /* namespace usbdevice */

#endif /* USBKEYBOARDDEVICE_H_ */
