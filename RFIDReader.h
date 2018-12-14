/*
 * RFIDReader.h
 *
 *  Created on: 14.12.2018
 *      Author: malte
 */

#ifndef RFIDREADER_H_
#define RFIDREADER_H_
#include <stdint.h>
#include "RFIDReaderConstants.h"

namespace rfid_reader {

	class RFIDReader {

	public:
		RFIDReader();
		uint8_t setISO15693Protocol();
		void wakeUpCall();
		uint8_t singleEcho();
	private:
		void communicationPause();
		void delay(int ms);
		void toggleSlaveActive();
	};

} /* namespace rfid_reader */

#endif /* RFIDREADER_H_ */
