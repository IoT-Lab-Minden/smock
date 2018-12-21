/*
 * MFRC522.h
 *
 *  Created on: 19.12.2018
 *      Author: malte
 */

#ifndef SRC_RFID_MFRC522_H_
#define SRC_RFID_MFRC522_H_
#include <stdint.h>
#include "driverlib/rom.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "MFRC522Constants.h"

namespace rfid_reader {

	/*const uint8_t SS_PIN = GPIO_PIN_3;
	const uint32_t SS_PIN_BASE = GPIO_PORTA_BASE;*/
	extern const uint8_t RST_PIN;
	extern const uint32_t RST_PIN_BASE;
	extern const uint8_t WAIT_IRQ;


	class MFRC522 {
	public:
		MFRC522();
		void delay(int);
		void softReset();
		void writeRegister(uint8_t, uint8_t);
		void writeRegister(uint8_t, uint8_t, uint8_t *);
		uint8_t readRegister(uint8_t);
		void readRegister(PCD_Register reg, uint8_t count, uint8_t *values, uint8_t rxAlign = 0);
		void setRegisterBitMask(uint8_t, uint8_t);
		void clearRegisterBitMask(uint8_t, uint8_t);
		void init();
		void antennaOn();
		void antennaOff();
		uint8_t performSelfTest();
		uint8_t isNewCardPresent();
		uint8_t readCardSerial();
		StatusCode select_PICC(Uid *uid, uint8_t validBits = 0);
		StatusCode calculateCRC(uint8_t *data, uint8_t length, uint8_t *result);
		StatusCode REQA_or_WUPA(uint8_t command, uint8_t *bufferATQA, uint8_t *bufferSize);
		StatusCode transceiveData(uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint8_t *backLen, uint8_t *validBits = nullptr, uint8_t rxAlign = 0, bool checkCRC = false);
		StatusCode communicateWithPICC(uint8_t command, uint8_t waitIRq, uint8_t *sendData, uint8_t sendLen, uint8_t *backData = nullptr, uint8_t *backLen = nullptr, uint8_t *validBits = nullptr, uint8_t rxAlign = 0, bool checkCRC = false);
		Uid uid;
	};

} /* namespace rfid_reader */

#endif /* SRC_RFID_MFRC522_H_ */
