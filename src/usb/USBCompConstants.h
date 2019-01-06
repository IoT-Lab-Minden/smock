/**
 * \file USBCompConstants.h
 * \brief Contains the constants needed for the Composite-Device
 *
 * Contains the constants needed for the Composite-Device
 */

#ifndef USBCOMPCONSTANTS_H_
#define USBCOMPCONSTANTS_H_
#include <stdint.h>

namespace usbdevice {
	/**
	 * \var const uint8_t DESCRIPTOR_DATA_SIZE
	 *
	 * Size of the Composite-Device-Descriptor
	 */
	const uint8_t DESCRIPTOR_DATA_SIZE = (COMPOSITE_DHID_SIZE + COMPOSITE_DCDC_SIZE);

	/**
	 * \var const uint8_t NUM_DEVICES
	 *
	 * Number of the devices of the Composite-Device
	 */
	const uint8_t NUM_DEVICES = 2;
}



#endif /* USBCOMPCONSTANTS_H_ */
