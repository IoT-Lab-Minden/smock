/*
 * USBCompDevice.h
 *
 *  Created on: 07.12.2018
 *      Author: malte
 */

#ifndef USBCOMPDEVICE_H_
#define USBCOMPDEVICE_H_
#include <stdint.h>
#include "usblib/usblib.h"
#include "usblib/usbhid.h"
#include "usblib/usb-ids.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdcomp.h"
#include "usblib/device/usbdhid.h"
#include "USBSerialDevice.h"
#include "USBKeyboardDevice.h"
#include "usb_structs.h"
#include "USBCompConstants.h"

namespace usbdevice {

	class USBCompDevice {
	private:

		/**
		 * The memory allocation for the composite USB device descriptors.
		 */
		uint8_t g_pui8DescriptorData[DESCRIPTOR_DATA_SIZE];

		/**
		 * The array of devices supported by this composite device.
		 */
		tCompositeEntry g_psCompDevices[NUM_DEVICES];

		/**
		 * Allocate the Device Data for the top level composite device class.
		 */
		tUSBDCompositeDevice g_sCompDevice;

		static tUSBCallback eventHandler;

	protected:
		USBCompDevice(tUSBCallback eventHandler);

	public:
		static USBCompDevice *getInstance();

		static void registerEventHandler(tUSBCallback eventHandler);

		void init();

	};

} /* namespace usbdevice */

#endif /* USBCOMPDEVICE_H_ */
