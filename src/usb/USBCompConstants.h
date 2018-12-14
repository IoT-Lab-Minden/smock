/*
 * USBCompConstants.h
 *
 *  Created on: 10.12.2018
 *      Author: malte
 */

#ifndef USBCOMPCONSTANTS_H_
#define USBCOMPCONSTANTS_H_
#include <stdint.h>

namespace usbdevice {
	const uint8_t DESCRIPTOR_DATA_SIZE = (COMPOSITE_DHID_SIZE + COMPOSITE_DCDC_SIZE);
	const uint8_t NUM_DEVICES = 2;

}



#endif /* USBCOMPCONSTANTS_H_ */
