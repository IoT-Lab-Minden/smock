/*
 * RFIDReader.cpp
 *
 *  Created on: 14.12.2018
 *      Author: malte
 */

#include "RFIDReader.h"
#include "SPI.h"
#include "driverlib/rom.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"

namespace rfid_reader {

	RFIDReader::RFIDReader() {
		// Set pin to output
	    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_5);
	    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_3);
	    // Set pin to high
		ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3, GPIO_PIN_3);

		SPI.begin();
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
		while (result & 0x11111000 != POLLING_END);

		communicationPause();

		// read result
		SPI.transfer(CONTROL_BYTE_READING);
		uint8_t resultCode = SPI.transfer(0x00);
		uint8_t length = SPI.transfer(0x00);

		toggleSlaveActive();
		return resultCode;
	}

	void RFIDReader::wakeUpCall() {
		ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3, 0);
		delay(100);
		ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3, GPIO_PIN_3);
		delay(100);

		ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, GPIO_PIN_5);
	}

	uint8_t RFIDReader::singleEcho() {
		toggleSlaveActive();

		SPI.transfer(CONTROL_BYTE_COMMAND);
		SPI.transfer(COMMAND_ECHO);

		communicationPause();

		SPI.transfer(CONTROL_BYTE_READING);
		uint8_t response = SPI.transfer(0x00);

		toggleSlaveActive();
		return response;
	}


	void RFIDReader::communicationPause() {
		toggleSlaveActive();
		delay(10);
		toggleSlaveActive();
	}

	void RFIDReader::delay(int ms) {
		uint32_t one_ms = (ROM_SysCtlClockGet() / 3) / 1000;
		SysCtlDelay(one_ms * ms);
	}

	void RFIDReader::toggleSlaveActive() {
		static uint8_t active = 0;
		if (active) {
			ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, 0);
			active = 1;
		} else {
			ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, GPIO_PIN_5);
			active = 0;
		}
	}

} /* namespace rfid_reader */
