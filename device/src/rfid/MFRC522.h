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
		 * \fn void writeRegister(PCD_Register reg, uint8_t value)
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
		void writeRegister(PCD_Register reg, uint8_t value);

		/**
		 * \fn void writeRegister(PCD_Register reg, uint8_t count, uint8_t *values)
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
		void writeRegister(PCD_Register reg, uint8_t count, uint8_t *values);

		/**
		 * \fn uint8_t readRegister(PCD_Register reg)
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
		uint8_t readRegister(PCD_Register reg);

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
		 * \fn void setRegisterBitMask(PCD_Register reg, uint8_t mask)
		 * \brief Applys a mask to a given register.
		 *
		 * \param reg Address of the Register the mask should be applied to
		 * \param mask Mask that should be applied
		 *
		 * The address needs to be given in the format x,p1,p0,a3,a2,a1,a0,x.
		 * x: The first stands for a read operation and is set to 1 the second is reserved
		 * for future use. and is set to 0
		 * p1-0: Addressing the page.
		 * a3-0: Address of the register.
		 *
		 * First reads the value of the register followed by writing the value, and-linked
		 * to the mask, back into the register.
		 * \return void
		 */
		void setRegisterBitMask(PCD_Register reg, uint8_t mask);

		/**
		 * \fn void clearRegisterBitMask(PCD_Register reg, uint8_t mask)
		 * \brief Applys a mask to a given register.
		 *
		 * \param reg Address of the Register the mask should be applied to
		 * \param mask Mask that should be applied
		 *
		 * The address needs to be given in the format x,p1,p0,a3,a2,a1,a0,x.
		 * x: The first stands for a read operation and is set to 1 the second is reserved
		 * for future use. and is set to 0
		 * p1-0: Addressing the page.
		 * a3-0: Address of the register.
		 *
		 * First reads the value of the register followed by writing the value, and-linked
		 * to the negated mask, back into the register.
		 * \return void
		 */
		void clearRegisterBitMask(PCD_Register reg, uint8_t mask);

		/**
		 * \fn void init()
		 * \brief Initializes the MFRC522 chip.
		 *
		 * Carries out a soft reset if this has not already been done and writes default
		 * start values into the register of the MFRC522 chip.
		 *
		 * \return void
		 */
		void init();

		/**
		 * \fn void antennaOn()
		 * \brief Turns the antenna on
		 *
		 * Turns the antenna on by enabling pins TX1 and TX2.
		 * After a reset these pins are disabled.
		 *
		 * \return void
		 */
		void antennaOn();

		/**
		 * \fn void antennaOff()
		 * \brief Turns the antenna off
		 *
		 *  Turns the antenna off by disabling pins TX1 and TX2.
		 *
		 * \return void
		 */
		void antennaOff();

		/**
		 * \fn uint8_t performSelfTest()
		 * \brief Performs a self-test of the MFRC522
		 *
		 * Performs a self-test following the steps in the documentation 19.1:
		 * 	1. Perform a soft reset.
		 * 	2. Clear the internal buffer by writing 25 bytes of 00h and perform the Config Command
		 * 	3. Enable the Selftest by writing the value 09h to the register AutoTestReg.
		 * 	4. Write 00h to the FIFO.
		 * 	5. Start the Selftest with the CalcCRC Command.
		 * 	6. The Selftest will be performed.
		 * 	7. When the Selftest is finished, the FIFO contains the following bytes.
		 *
		 * \return uint8_t - 0 in case the test was aborted.
		 */
		uint8_t performSelfTest();

		/**
		 * \fn uint8_t isNewCardPresent()
		 * \brief Indicates weather a new card is present.
		 *
		 * Returns true if a PICC responds to PICC_CMD_REQA.
		 * Only "new" cards in state IDLE are invited. Sleeping cards in state HALT are ignored.
		 *
		 * \return uint8_t - 1 when a new card is present
		 */
		uint8_t isNewCardPresent();

		/**
		 * \fn uint8_t readCardSerial()
		 * \brief Reads the UID
		 *
		 * Simple wrapper around PICC_Select.
		 * Remember to call PICC_IsNewCardPresent(), PICC_RequestA() or PICC_WakeupA() first.
		 * The read UID is available in the class variable uid.
		 *
		 * \return uint8_t - 0 if a UID could be read.
		 */
		uint8_t readCardSerial();

		/**
		 * \fn StatusCode select_PICC(Uid *uid, uint8_t validBits)
		 * \brief Transmits SELECT/ANTICOLLISION commands to select a single PICC.
		 *
		 * \param *uid Pointer to Uid struct
		 * \param validBits The number of known UID bits supplied in *uid. Default value is 0. If set you must also supply uid->size.
		 *
		 * Before calling this function the PICCs must be placed in the READY(*) state by calling PICC_RequestA() or PICC_WakeupA().
		 * On success:
		 * 	- The chosen PICC is in state ACTIVE(*) and all other PICCs have returned to state IDLE/HALT. (Figure 7 of the ISO/IEC 14443-3 draft.)
		 * 	- The UID size and value of the chosen PICC is returned in *uid along with the SAK.
		 *
		 * \return StatusCode - STATUS_OK on success, STATUS_??? otherwise.
		 */
		StatusCode select_PICC(Uid *uid, uint8_t validBits = 0);

		/**
		 * \fn StatusCode calculateCRC(uint8_t *data, uint8_t length, uint8_t *result)
		 * \brief Calculate a CRC_A.
		 *
		 * \param *data Pointer to the data to transfer to the FIFO for CRC calculation.
		 * \param length The number of bytes to transfers
		 * \param *reslut Pointer to the result buffer, the output is written into.
		 *
		 * Use the CRC coprocessor in the MFRC522 to calculate a CRC_A.
		 *
		 * \return StatusCode - STATUS_OK on success, STATUS_??? otherwise.
		 */
		StatusCode calculateCRC(uint8_t *data, uint8_t length, uint8_t *result);

		/**
		 * \fn StatusCode REQA_or_WUPA(uint8_t command, uint8_t *bufferATQA, uint8_t *bufferSize)
		 * \brief Transmits REQA or WUPA commands.
		 *
		 * \param command The command to send - PICC_CMD_REQA or PICC_CMD_WUPA
		 * \param *bufferATQA The buffer to store the answer to the request in
		 * \param *bufferSize Buffer size, at least two bytes.
		 *
		 * Clears the collision register before transceive the data.
		 *
		 * \return StatusCode - STATUS_OK on success, STATUS_??? otherwise.
		 */
		StatusCode REQA_or_WUPA(uint8_t command, uint8_t *bufferATQA, uint8_t *bufferSize);

		/**
		 * \fn StatusCode transceiveData(uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint8_t *backLen, uint8_t *validBits, uint8_t rxAlign, bool checkCRC)
		 * \brief Executes the Transceive command.
		 *
		 * \param *sendData Pointer to the data to transfer to the FIFO.
		 * \param sendLen Number of bytes to transfer to the FIFO.
		 * \param *backData nullptr or pointer to buffer if data should be read back after executing the command.
		 * \param *backLen In: Max number of bytes to write to *backData. Out: The number of bytes returned.
		 * \param *validBits The number of valid bits in the last byte. 0 for 8 valid bits. Default nullptr.
		 * \param rxAlign Defines the bit position in backData[0] for the first bit received. Default 0.
		 * \param checkCRC True => The last two bytes of the response is assumed to be a CRC_A that must be validated.
		 *
		 * Calls communicateWithPICC with a PCD_Transceive Command.
		 * CRC validation can only be done if backData and backLen are specified.
		 *
		 * \return StatusCode - STATUS_OK on success, STATUS_??? otherwise.
		 */
		StatusCode transceiveData(uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint8_t *backLen, uint8_t *validBits = nullptr, uint8_t rxAlign = 0, bool checkCRC = false);

		/**
		 * \fn StatusCode communicateWithPICC(uint8_t command, uint8_t waitIRq, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint8_t *backLen, uint8_t *validBits, uint8_t rxAlign, bool checkCRC)
		 * \brief Single communication protocol.
		 *
		 * Transfers data to the MFRC522 FIFO, executes a command, waits for completion and transfers data back from the FIFO.
		 * CRC validation can only be done if backData and backLen are specified.
 	 	 *
		 * \return StatusCode - STATUS_OK on success, STATUS_??? otherwise.
		 */
		StatusCode communicateWithPICC(uint8_t command, uint8_t waitIRq, uint8_t *sendData, uint8_t sendLen, uint8_t *backData = nullptr, uint8_t *backLen = nullptr, uint8_t *validBits = nullptr, uint8_t rxAlign = 0, bool checkCRC = false);

		/**
		 * \var uid
		 *
		 * The uid currently read by the MRFC522.
		 */
		Uid uid;
	};

} /* namespace rfid_reader */

#endif /* SRC_RFID_MFRC522_H_ */
