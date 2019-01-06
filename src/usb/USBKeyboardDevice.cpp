/**
 * \file USBKeyboardDevice.cpp
 * \brief Contains the definitions of the methods for the keyboard device.
 *
 * The keyboard device is realized by a singelton pattern because the micro controller is
 * only able to have one keyboard device active at the same time. The device is a human
 * interface device and simulates a keyboard.
 */

#include "USBKeyboardDevice.h"
#include <stdint.h>
#include <stdbool.h>
#include "grlib/keyboard.h"
#include "usblib/usblib.h"
#include "usblib/usbhid.h"
#include "usblib/usb-ids.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdhid.h"
#include "usblib/device/usbdhidkeyb.h"
#include "driverlib/rom.h"
#include "usb_structs.h"

namespace usbdevice {

	tUSBCallback USBKeyboardDevice::keyboardHandler = nullptr;

	void USBKeyboardDevice::registerKeyboardHandler(tUSBCallback handler) {
		keyboardHandler = handler;
	}

	// Anlegen des Objektes auf dem Heap hat nicht funktioniert??
	USBKeyboardDevice *USBKeyboardDevice::getInstance() {
		static USBKeyboardDevice instance(keyboardHandler);
		return &instance;
	}

	USBKeyboardDevice::USBKeyboardDevice(tUSBCallback keyboardHandler) :
	g_sKeyboardDevice( {
				USB_VID_TI_1CBE,
				USB_PID_KEYBOARD,
				250,
				USB_CONF_ATTR_BUS_PWR,
				keyboardHandler,
				(void *)&g_sKeyboardDevice,
				0,
				0
			})
	{}


	tUSBDHIDKeyboardDevice *USBKeyboardDevice::getKeyboardDevice() {
		return &g_sKeyboardDevice;
	}

	void USBKeyboardDevice::USBKeyboardInit(void) {
	    // Clear out the special key variable.
	    g_sKeyboardState.ui8Special = 0;
	}

	void USBKeyboardDevice::releaseSpecialKey() {
		USBDHIDKeyboardKeyStateChange(&g_sKeyboardDevice, 0, g_sKeyboardState.ui8Special, false);
		g_sKeyboardState.ui8Special = 0;
	}

	bool USBKeyboardDevice::IsKeyboardPending(void) {
	    return g_sKeyboardState.pending;
	}

	void USBKeyboardDevice::USBKeyboardUpdate(uint8_t ui8Modifiers, uint8_t ui8Key, bool bPressed)
	{
	    uint8_t ui8Usage;

	    // Move these to a-z because USB HID does not recognize unshifted values,
	    // it uses the SHIFT modifier to change the case.
	    if ((ui8Key >= 'A') && (ui8Key <= 'Z'))
	    {
	        ui8Key |= 0x20;

	        if (bPressed)
	        {
	            ui8Modifiers |= HID_KEYB_LEFT_SHIFT;
	        }
	    }

	    // Get the usage code for this character.
	    ui8Usage = GetUsageCode(ui8Key, false);

	    // Check if this was a "special" key because USB HID handles these separately.
	    if ((ui8Usage == HID_KEYB_USAGE_CAPSLOCK)
	            || (ui8Usage == HID_KEYB_USAGE_SCROLLOCK)
	            || (ui8Usage == HID_KEYB_USAGE_NUMLOCK))
	    {
	        // If there was already a special key pressed, then force it to be released.
	        if (g_sKeyboardState.ui8Special)
	        {
	            USBDHIDKeyboardKeyStateChange(&g_sKeyboardDevice, ui8Modifiers,
	                                          g_sKeyboardState.ui8Special, false);
	        }

	        // Save the new special key.
	        g_sKeyboardState.ui8Special = ui8Usage;
	    }

	    // If there was not an unshifted value for this character then look for a shifted version of the character.
	    if (ui8Usage == 0)
	    {
	        // Get the shifted value and set the shift modifier.
	        ui8Usage = GetUsageCode(ui8Key, true);

	        if (bPressed)
	        {
	            ui8Modifiers |= HID_KEYB_LEFT_SHIFT;
	        }
	    }

	    // If a valid usage code was found then pass the key along to the USB library.
	    if (ui8Usage)
	    {
	        USBDHIDKeyboardKeyStateChange(&g_sKeyboardDevice, ui8Modifiers,
	                                      ui8Usage, bPressed);
	        g_sKeyboardState.pending = true;
	    }
	}

	void USBKeyboardDevice::USBWriteString(const char *string, int length) {
	    static unsigned int g_keyTick = 0;
	    static bool g_keyDown = false;

	    uint8_t running = 1;
	    int index = 0;

	    while (running) {
	        unsigned int tick = ROM_SysTickValueGet();
	        if((index == 0) && (g_keyTick == 0)) {
	            g_keyTick = g_ui32SysTickCount;
	        }
	        if(g_keyTick != 0) {
	            int diff = g_keyTick - g_ui32SysTickCount;
	            if(diff <= 0) {
	                if(g_keyDown) {
	                    //release button
	                    USBKeyboardUpdate(0, string[index], false);
	                    index++;
	                    g_keyTick = (g_ui32SysTickCount + 0x1) | 1;
	                    g_keyDown = false;
	                }
	                else if(index < length) {
	                    // press button
	                    USBKeyboardUpdate(0, string[index], true);
	                    g_keyTick = (g_ui32SysTickCount + 0x2) | 1;
	                    g_keyDown = true;
	                }
	                else {
	                    g_keyTick = 0;
	                }
	            }
	        }
	        if (index >= length) {
	            running = 0;
	        }
	    }
	}

	void USBKeyboardDevice::USBPressKeyCombination(const uint8_t modifiers, const uint8_t *string, int length) {
	    static unsigned int g_keyTick = 0;

	    uint8_t running = 1;
	    int index = 0;

	    while (running) {
	        unsigned int tick = ROM_SysTickValueGet();
	        if((index == 0) && (g_keyTick == 0)) {
	            g_keyTick = g_ui32SysTickCount;
	        }
	        if(g_keyTick != 0) {
	            int diff = g_keyTick - g_ui32SysTickCount;
	            if(diff <= 0) {
	                if(index < length) {
	                    // press button
	                    uint8_t usage = string[index];
	                    USBDHIDKeyboardKeyStateChange(&g_sKeyboardDevice, modifiers,
	                                                       usage, true);
	                    index++;
	                    g_keyTick = (g_ui32SysTickCount + 0x2) | 1;
	                }
	                else if(index < length * 2 && index >= length) {
	                    // release button
	                    uint8_t usage = string[index - length];
	                    USBDHIDKeyboardKeyStateChange(&g_sKeyboardDevice, 0,
	                                                       usage, false);
	                    index++;
	                    g_keyTick = (g_ui32SysTickCount + 0x1) | 1;
	                }
	                else {
	                    g_keyTick = 0;
	                }
	            }
	        }
	        if (index >= length * 2) {
	            running = 0;
	        }
	    }
	}


	uint8_t USBKeyboardDevice::GetUsageCode(char cKey, bool bShifted)
	{
	    int32_t i32Idx, i32Entries;
	    const sUsageEntry *pUsageTable;

	    if (bShifted)
	    {
	        i32Entries = ui32NumUsageCodesShift;
	        pUsageTable = g_pcUsageCodesShift;
	    }
	    else
	    {
	        i32Entries = ui32NumUsageCodes;
	        pUsageTable = g_pcUsageCodes;
	    }

	    for (i32Idx = 0; i32Idx < i32Entries; i32Idx++)
	    {
	        if (pUsageTable[i32Idx].cChar == cKey)
	        {
	            return (pUsageTable[i32Idx].cUsage);
	        }
	    }
	    return (0);
	}


} /* namespace usbdevice */
