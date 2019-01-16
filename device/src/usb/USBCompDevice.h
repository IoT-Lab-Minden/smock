/**
 * \file USBCompDevice.h
 * \brief Contains the declaration of the methods and attributes needed for the composite device.
 *
 * This device allows a keyboard device and a serial device to be active at the same time
 * through the same port.
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

	/**
	 * \class USBCompDevice
	 * \brief Device consisting of a serial device and a keyboard device.
	 *
	 * The composite device allows a keyboard device and a serial device to be active
	 * at the same time through the same usb port.
	 * The device is realized by a singelton pattern because the micro controller
	 * is only able to have one usb device active on the port at once.
	 */
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

		/**
		 * Handles default usb events
		 */
		static tUSBCallback eventHandler;

		/**
		 * \brief Constructor which builds the object.
		 * \param eventHandler The callback function that should be set.
		 *
		 * The constructor initializes the g_sCompDevice of the object.
		 */
		USBCompDevice(tUSBCallback eventHandler);

	public:

		/**
		 * \fn USBCompDevice *getInstance()
		 * \brief Returns the instance of the USBCompdevice
		 *
		 * The function creates the object the first time it is called and return the pointer
		 * to the object. The object is created on the heap. Later calls only return the
		 * pointer to this object.
		 *
		 * \return - the instance of composite device
		 */
		static USBCompDevice *getInstance();

		/**
		 * \fn void registerEventHandler(tUSBCallback eventHandler)
		 * \brief Registers a new Callback function for the envent handler.
		 *
		 * \param eventHandler New callback method.
		 *
		 * A new handler can only be set before the device is referenced the first time.
		 * When the device is referenced the first time, the composite device struct is
		 * initialized with the current callback method by calling the constructor of the
		 * composite device. After that there is no possibility to change the callback
		 * method. It can only be one function registered.
		 *
		 * \return void
		 */
		static void registerEventHandler(tUSBCallback eventHandler);

		/**
		 * \fn void init()
		 * \brief Initialized the composite device
		 *
		 * The device is initialized by initializing both sub-devices as part of the
		 * composite device followed by initializing the composite device itself.
		 *
		 * \return void
		 */
		void init();

	};

} /* namespace usbdevice */

#endif /* USBCOMPDEVICE_H_ */
