/**
 * \file MFRC522.h
 * \brief Contains the declaration of the methods and attributes needed for the MFRC522.
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

	extern const uint8_t SS_PIN;
	extern const uint32_t SS_PIN_BASE;
	extern const uint8_t RST_PIN;
	extern const uint32_t RST_PIN_BASE;


	/**
	 * \class MFRC522
	 * \brief Handles the addressing of the MFRC522
	 *
	 * This class provide ethods to initialize and communicate with the MFRC522
	 */
	class MFRC522 {
	public:
		/**
		 * \fn MFRC522()
		 * \brief Default constructor for initialing the MFRC522
		 *
		 * Starts the SPI module, sets the PST_PIN and the SS_PIN to output and the RST_PIN
		 * to High. After that the init method is called.
		 */
		MFRC522();

		/**
		 * \fn delay(int ms)
		 * \brief Waits for a given time
		 *
		 * \param ms Milisecounds to wait.
		 *
		 * Uses the clock rate to calculate the number of ticks for one ms.
		 * This number is used to delay the given ms with SysCtlDelay().
		 *
		 * \return void
		 */
		void delay(int ms);

		/**
		 * \fn softReset()
		 * \brief Performs a soft reset on the MFRC522
		 *
		 * The soft reset is performed by writing the PCD_SoftReset command in the pcd
		 * register. The configuration data of the internal buffer remains unchanged. All
		 * registers are set to their reset values.
		 * After sending the command the system waits for its termination.
		 *
		 * \return void
		 */
		void softReset();

		/**
		 * \fn void writeRegister(uint8_t reg, uint8_t value)
		 * \brief Writes the value into the register given by its address.
		 *
		 * \param reg Adress of the register
		 * \param value The value to write in the register
		 *
		 * The address needs to be given in the format x,p1,p0,a3,a2,a1,a0,x.
		 * x: Set to 0 by the write function. The first stands for a write operation
		 * the second is reserved for future use.
		 * p1-0: Addressing the page.
		 * a3-0: Address of the register.
		 *
		 * The register is written to by writing 0 to the SS_PIN, sending the address byte
		 * followed by the data byte via SPI.
		 * Afterwards the SS_PIN is set to high again.
		 *
		 * \return void
		 */
		void writeRegister(uint8_t reg, uint8_t value);

		/**
		 * \fn void writeRegister(uint8_t reg, uint8_t count, uint8_t *values)
		 * \brief Writes multiple bytes to registers, starting at the given address
		 *
		 * \param reg Start address
		 * \param count Number of bytes that should be send
		 * \param values Data bytes
		 *
		 * The address needs to be given in the format x,p1,p0,a3,a2,a1,a0,x.
		 * x: Set to 0 by the write function. The first stands for a write operation
		 * the second is reserved for future use.
		 * p1-0: Addressing the page.
		 * a3-0: Address of the register.
		 *
		 * The register is written by writing 0 to the SS_PIN, sending the address byte
		 * followed all the data bytes via SPI.
		 * Afterwards the SS_PIN is set to high again.
		 *
		 * \return void
		 */
		void writeRegister(uint8_t reg, uint8_t count, uint8_t *values);

		/**
		 * \fn uint8_t readRegister(uint8_t reg)
		 * \brief Reads the value of one register.
		 *
		 * \param reg Address of the register
		 *
		 * The address needs to be given in the format x,p1,p0,a3,a2,a1,a0,x.
		 * x: The first stands for a read operation and is set to 1 the second is reserved
		 * for future use. and is set to 0
		 * p1-0: Addressing the page.
		 * a3-0: Address of the register.
		 *
		 * The register is read by writing 0 to the SS_PIN, sending the address byte
		 * followed by a zero byte to read the answer byte.
		 * Afterwards the SS_PIN is set to high again.
		 *
		 * \return uint8_t - The received answer.
		 */
		uint8_t readRegister(uint8_t reg);

		/**
		 * \fn void readRegister(PCD_Register reg, uint8_t count, uint8_t *values, uint8_t rxAlign = 0)
		 * \brief Reads multiple bytes from an given address.
		 *
		 * \param reg Address of the register
		 * \param count Number of bytes to read
		 * \param values Buffer to write the data into
		 * \param rxAlign Read aligning
		 *
		 * The address needs to be given in the format x,p1,p0,a3,a2,a1,a0,x.
		 * x: The first stands for a read operation and is set to 1 the second is reserved
		 * for future use. and is set to 0
		 * p1-0: Addressing the page.
		 * a3-0: Address of the register.
		 *
		 * The register is read by writing 0 to the SS_PIN, sending the address byte
		 * followed by sending address byte for every byte that should be read except the
		 * last. Sending the address indcates the reading should go on. Reading is stopped by
		 * sending 0. From sending 0 the last byte is received.
		 * Afterwards the SS_PIN is set to high again.
		 * It is possible to apply an aligning.
		 *
		 * \return void
		 */
		void readRegister(PCD_Register reg, uint8_t count, uint8_t *values, uint8_t rxAlign = 0);

		/**
		 * \fn void setRegisterBitMask(uint8_t reg, uint8_t mask)
		 * \brief Applys a mask to a given register.
		 *
		 * \param reg Addres of the Register the mask should be applied to
		 * \param mask Mask that should be applied
		 *
		 * The address needs to be given in the format x,p1,p0,a3,a2,a1,a0,x.
		 * x: The first stands for a read operation and is set to 1 the second is reserved
		 * for future use. and is set to 0
		 * p1-0: Addressing the page.
		 * a3-0: Address of the register.
		 *
		 * First reads the value of the register followed by writing the value, and linked
		 * to the mask, back into the register.
		 * \return void
		 */
		void setRegisterBitMask(uint8_t reg, uint8_t mask);
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
