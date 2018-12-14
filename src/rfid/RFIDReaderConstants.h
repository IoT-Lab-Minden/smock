/*
 * RFIDReaderConstants.h
 *
 *  Created on: 14.12.2018
 *      Author: malte
 */

#ifndef RFIDREADERCONSTANTS_H_
#define RFIDREADERCONSTANTS_H_

#include <stdint.h>

namespace rfid_reader {

	const uint8_t CONTROL_BYTE_COMMAND = 0x00;
	const uint8_t CONTROL_BYTE_POLLING = 0x03;
	const uint8_t CONTROL_BYTE_READING = 0x02;

	const uint8_t POLLING_END = 0x08;

	const uint8_t COMMAND_PROTOCOL = 0x02;
	const uint8_t COMMAND_ECHO = 0x55;

	const uint8_t COMMAND_PROTOCOL_ISO15693 = 0x01;
}



#endif /* RFIDREADERCONSTANTS_H_ */
