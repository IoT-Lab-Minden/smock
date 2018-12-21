/*
 * RFIDReader.cpp
 *
 *  Created on: 14.12.2018
 *      Author: malte
 */

#include "RFIDReader.h"
#include "../energia/SPI.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"

namespace rfid_reader {

	RFIDReader::RFIDReader() {
		// Set pin to output
	    ROM_GPIOPinTypeGPIOOutput(SS_PIN_BASE, SS_PIN);
	    ROM_GPIOPinTypeGPIOOutput(IRQIN_PIN_BASE, IRQIN_PIN);
	    // Set pin to high
		ROM_GPIOPinWrite(SS_PIN_BASE, SS_PIN, SS_PIN);
		ROM_GPIOPinWrite(IRQIN_PIN_BASE, IRQIN_PIN, 0);

		SPI.begin();
		SPI.setModule(0);
		SPI.setClockDivider(SPI_CLOCK_DIV64);
	}

	uint8_t RFIDReader::setISO15693Protocol() {

		toggleSlaveActive();
		// Send command
		SPI.transfer(CONTROL_BYTE_COMMAND);	// control bits
		SPI.transfer(COMMAND_PROTOCOL); // command
		SPI.transfer(0x02); // length of data
		SPI.transfer(COMMAND_PROTOCOL_ISO15693); // protocol
		SPI.transfer(0b00011000); //protocol parameter

		communicationPause();

		// Polling
		SPI.transfer(CONTROL_BYTE_POLLING);
		uint8_t result = SPI.transfer(CONTROL_BYTE_POLLING);
		while ((result & 0x11111000) != POLLING_END);

		communicationPause();

		// read result
		SPI.transfer(CONTROL_BYTE_READING);
		uint8_t resultCode = SPI.transfer(0x00);
		uint8_t length = SPI.transfer(0x00);

		toggleSlaveActive();
		return resultCode;
	}

	void RFIDReader::wakeUpCall() {
		// Tauschen?
		delay(20);
		ROM_GPIOPinWrite(IRQIN_PIN_BASE, IRQIN_PIN, IRQIN_PIN);
		delay(20);
		ROM_GPIOPinWrite(IRQIN_PIN_BASE, IRQIN_PIN, 0);
		delay(20);
		ROM_GPIOPinWrite(IRQIN_PIN_BASE, IRQIN_PIN, IRQIN_PIN);
		delay(20);

		ROM_GPIOPinWrite(SS_PIN_BASE, SS_PIN, SS_PIN);
		ROM_GPIOPinWrite(SS_PIN_BASE, SS_PIN, 0);
		SPI.transfer(0x01);
		ROM_GPIOPinWrite(SS_PIN_BASE, SS_PIN, SS_PIN);
	}

	uint8_t RFIDReader::singleEcho() {
		ROM_GPIOPinWrite(SS_PIN_BASE, SS_PIN, 0);
		delay(10);

		volatile uint8_t result;
		SPI.transfer(CONTROL_BYTE_COMMAND);
		SPI.transfer(COMMAND_ECHO);

		/*ROM_GPIOPinWrite(SS_PIN_BASE, SS_PIN, SS_PIN);
		delay(10);
		ROM_GPIOPinWrite(SS_PIN_BASE, SS_PIN, 0);

		SPI.transfer(CONTROL_BYTE_POLLING);
		result = SPI.transfer(CONTROL_BYTE_POLLING);
		while ((result & 0x11111000) != POLLING_END) {
			result = SPI.transfer(CONTROL_BYTE_POLLING);
		}*/

		ROM_GPIOPinWrite(SS_PIN_BASE, SS_PIN, SS_PIN);
		delay(10);
		ROM_GPIOPinWrite(SS_PIN_BASE, SS_PIN, 0);

		uint8_t response;
		response = SPI.transfer(CONTROL_BYTE_READING);
		SPI.transfer(0x00);

		ROM_GPIOPinWrite(SS_PIN_BASE, SS_PIN, SS_PIN);
		return response;
	}


	void RFIDReader::communicationPause() {
		toggleSlaveActive();
		delay(10);
		toggleSlaveActive();
		delay(10);
	}

	void RFIDReader::delay(int ms) {
		uint32_t one_ms = (ROM_SysCtlClockGet() / 3) / 1000;
		SysCtlDelay(one_ms * ms);
	}

	void RFIDReader::toggleSlaveActive() {
		static uint8_t active = 0;
		if (active) {
			ROM_GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, 0);
			active = 1;
		} else {
			ROM_GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_PIN_3);
			active = 0;
		}
	}

} /* namespace rfid_reader */
