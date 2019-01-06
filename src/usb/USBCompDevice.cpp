/**
 * \file USBCompDevice.cpp
 * \brief Contains the definitions of the methods for the composite device.
 *
 * This composite device is realized by a singelton pattern because the micro controller
 * is only able to have one usb device active at once. The device allows a keyboard device
 * and a serial device to be active at the same time through the same port.
 */

#include "USBCompDevice.h"
#include "usb_structs.h"
#include "usblib/device/usbdcdc.h"
#include "usblib/device/usbdcomp.h"

namespace usbdevice {

	tUSBCallback USBCompDevice::eventHandler = nullptr;

	void USBCompDevice::init() {
		USBDHIDKeyboardCompositeInit(0, USBKeyboardDevice::getInstance()->getKeyboardDevice(), &g_psCompDevices[0]);
		USBDCDCCompositeInit(0, USBSerialDevice::getInstance()->getSerialDevice(), &g_psCompDevices[1]);

		USBDCompositeInit(0, &g_sCompDevice, DESCRIPTOR_DATA_SIZE, g_pui8DescriptorData);
	}

	void USBCompDevice::registerEventHandler(tUSBCallback handler) {
		eventHandler = handler;
	}

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
