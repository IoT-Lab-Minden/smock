/*
 * RFIDReader.h
 *
 *  Created on: 14.12.2018
 *      Author: malte
 */

#ifndef RFIDREADER_H_
#define RFIDREADER_H_
#include <stdint.h>
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include "RFIDReaderConstants.h"

namespace rfid_reader {
	const uint8_t IRQIN_PIN = GPIO_PIN_6;
	const uint32_t IRQIN_PIN_BASE = GPIO_PORTA_BASE;

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
