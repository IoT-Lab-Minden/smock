/*
 * USBCompDevice.cpp
 *
 *  Created on: 07.12.2018
 *      Author: malte
 */

#include "USBCompDevice.h"
#include "usb_structs.h"
#include "usblib/device/usbdcdc.h"
#include "usblib/device/usbdcomp.h"

namespace usbdevice {

	tUSBCallback USBCompDevice::eventHandler = 0;

	void USBCompDevice::init() {
		USBDHIDKeyboardCompositeInit(0, USBKeyboardDevice::getInstance()->getKeyboardDevice(), &g_psCompDevices[0]);
		USBDCDCCompositeInit(0, USBSerialDevice::getInstance()->getSerialDevice(), &g_psCompDevices[1]);

		USBDCompositeInit(0, &g_sCompDevice, DESCRIPTOR_DATA_SIZE, g_pui8DescriptorData);

	}

	void USBCompDevice::registerEventHandler(tUSBCallback handler) {
		eventHandler = handler;
	}

	// Anlegen des Objektes auf dem Heap hat nicht funktioniert??
	USBCompDevice *USBCompDevice::getInstance() {
		static USBCompDevice instance(eventHandler);
		return &instance;
	}

	USBCompDevice::USBCompDevice(tUSBCallback eventHandler) : g_sCompDevice( {
				USB_VID_TI_1CBE,
				USB_PID_COMP_HID_SER,
				250,
				USB_CONF_ATTR_BUS_PWR,
				eventHandler,
				g_pui8StringDescriptors,
				NUM_STRING_DESCRIPTORS,
				NUM_DEVICES,
				g_psCompDevices,
			})
	{}

} /* namespace usbdevice */
