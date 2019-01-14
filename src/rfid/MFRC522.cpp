/**
 * \file MFRC522.cpp
 * \brief Contains the definitions of the methods for MFRC522.
 */

#include "MFRC522.h"
#include "../energia/SPI.h"
#include "driverlib/rom.h"
#include "MFRC522Constants.h"
#include "../energia/avr/pgmspace.h"

namespace rfid_reader {

	MFRC522::MFRC522() {
		SPI.begin();
		SPI.setModule(0);

	    ROM_GPIOPinTypeGPIOOutput(SS_PIN_BASE, SS_PIN);
	    ROM_GPIOPinWrite(SS_PIN_BASE, SS_PIN, SS_PIN);

	    ROM_GPIOPinTypeGPIOOutput(RST_PIN_BASE, RST_PIN);
	    init();
	}

	void MFRC522::init() {
	    if (ROM_GPIOPinRead(RST_PIN_BASE, RST_PIN)) {
		    ROM_GPIOPinWrite(RST_PIN_BASE, RST_PIN, RST_PIN);
		    delay(50);
	    } else {
	    	softReset();
	    }
		writeRegister(TModeReg, 0x80);			// TAuto=1; timer starts automatically at the end of the transmission in all communication modes at all speeds
		writeRegister(TPrescalerReg, 0xA9);		// TPreScaler = TModeReg[3..0]:TPrescalerReg, ie 0x0A9 = 169 => f_timer=40kHz, ie a timer period of 25ms.
		writeRegister(TReloadRegH, 0x03);		// Reload timer with 0x3E8 = 1000, ie 25ms before timeout.
		writeRegister(TReloadRegL, 0xE8);

		writeRegister(TxASKReg, 0x40);		// Default 0x00. Force a 100 % ASK modulation independent of the ModGsPReg register setting
		writeRegister(ModeReg, 0x3D);		// Default 0x3F. Set the preset value for the CRC coprocessor for the CalcCRC command to 0x6363 (ISO 14443-3 part 6.2.4)
		antennaOn();						// Enable the antenna driver pins TX1 and TX2 (they were disabled by the reset)
	}

	void MFRC522::writeRegister(uint8_t reg, uint8_t value) {
		// select device
	    ROM_GPIOPinWrite(SS_PIN_BASE, SS_PIN, 0);
		SPI.transfer(reg & 0x7E);							// write command address format:  r(1)/w(0) a5 a4 a3 a2 a1 RFU(0)  with a5 as address page and a4 - a1 as register address
		SPI.transfer(value);
	    ROM_GPIOPinWrite(SS_PIN_BASE, SS_PIN, SS_PIN);
	}

	void MFRC522::writeRegister(uint8_t reg, uint8_t count, uint8_t *values) {
		ROM_GPIOPinWrite(SS_PIN_BASE, SS_PIN, 0);
		SPI.transfer(reg & 0x7E);
		for (uint8_t index = 0; index < count; index++) {
			SPI.transfer(values[index]);
		}
	    ROM_GPIOPinWrite(SS_PIN_BASE, SS_PIN, SS_PIN);
	}

	uint8_t MFRC522::readRegister(uint8_t reg) {
		uint8_t value;
	    ROM_GPIOPinWrite(SS_PIN_BASE, SS_PIN, 0);
		SPI.transfer(0x80 | (reg & 0x7E));
		value = SPI.transfer(0);
	    ROM_GPIOPinWrite(SS_PIN_BASE, SS_PIN, SS_PIN);
		return value;
	}

	void MFRC522::readRegister(PCD_Register reg, uint8_t count, uint8_t *values, uint8_t rxAlign) {
		if (count == 0) {
			return;
		}
		uint8_t address = 0x80 | (reg & 0x7E);
		uint8_t index = 0;

	    ROM_GPIOPinWrite(SS_PIN_BASE, SS_PIN, 0);
		count--;
		SPI.transfer(address);
		while (index < count) {
			if (index == 0 && rxAlign) {
				uint8_t mask = 0;
				for (uint8_t i = rxAlign; i <= 7; i++) {
					mask |= (1 << i);
				}
				// Read value and tell that we want to read the same address again.
				uint8_t value = SPI.transfer(address);
				// Apply mask to both current value of values[0] and the new data in value.
				values[0] = (values[index] & ~mask) | (value & mask);
			}
			else { // Normal case
				values[index] = SPI.transfer(address);	// Read value and tell that we want to read the same address again.
			}
			index++;
		}
		values[index] = SPI.transfer(0);
	    ROM_GPIOPinWrite(SS_PIN_BASE, SS_PIN, SS_PIN);
	}

	void MFRC522::setRegisterBitMask(uint8_t reg, uint8_t mask) {
		uint8_t tmp;
		tmp = readRegister(reg);
		writeRegister(reg, tmp | mask);
	}

	void MFRC522::clearRegisterBitMask(uint8_t reg, uint8_t mask) {
		uint8_t tmp;
		tmp = readRegister(reg);
		writeRegister(reg, tmp & (~mask));
	}

	void MFRC522::softReset() {
		writeRegister(CommandReg, PCD_SoftReset);
		delay(50);
		while (readRegister(CommandReg) & (1<<4));
	}

	void MFRC522::antennaOn() {
		uint8_t value = readRegister(TxControlReg);
		if ((value & 0x03) != 0x03) {
			writeRegister(TxControlReg, value | 0x03);
		}
	}

	void MFRC522::antennaOff() {
		clearRegisterBitMask(TxControlReg, 0x03);
	}

	uint8_t MFRC522::performSelfTest() {
		// 1. Perform a soft reset.
		softReset();

		// 2. Clear the internal buffer by writing 25 uint8_ts of 00h
		uint8_t ZEROES[25] = {0x00};
		setRegisterBitMask(FIFOLevelReg, 0x80);	// flush the FIFO buffer
		writeRegister(FIFODataReg, 25, ZEROES);	// write 25 uint8_ts of 00h to FIFO
		writeRegister(CommandReg, PCD_Mem);		// transfer to internal buffer

		// 3. Enable self-test
		writeRegister(AutoTestReg, 0x09);

		// 4. Write 00h to FIFO buffer
		writeRegister(FIFODataReg, 0x00);

		// 5. Start self-test by issuing the CalcCRC command
		writeRegister(CommandReg, PCD_CalcCRC);

		// 6. Wait for self-test to complete
		word i;
		uint8_t n;
		for (i = 0; i < 0xFF; i++) {
			n = readRegister(DivIrqReg);	// DivIrqReg[7..0] bits are: Set2 reserved reserved MfinActIRq reserved CRCIRq reserved reserved
		}
		writeRegister(CommandReg, PCD_Idle);		// Stop calculating CRC for new content in the FIFO.

		// 7. Read out resulting 64 uint8_ts from the FIFO buffer.
		uint8_t result[64];
		readRegister(FIFODataReg, 64, result, 0);

		// Auto self-test done
		// Reset AutoTestReg register to be 0 again. Required for normal operation.
		writeRegister(AutoTestReg, 0x00);

		// Determine firmware version (see section 9.3.4.8 in spec)
		uint8_t version = readRegister(VersionReg);

		// Pick the appropriate reference values
		const uint8_t *reference;
		switch (version) {
			case 0x88:	// Fudan Semiconductor FM17522 clone
				reference = FM17522_firmware_reference;
				break;
			case 0x90:	// Version 0.0
				reference = MFRC522_firmware_referenceV0_0;
				break;
			case 0x91:	// Version 1.0
				reference = MFRC522_firmware_referenceV1_0;
				break;
			case 0x92:	// Version 2.0
				reference = MFRC522_firmware_referenceV2_0;
				break;
			default:	// Unknown version
				return 0; // abort test
		}

		// Verify that the results match up to our expectations
		for (i = 0; i < 64; i++) {
			if (result[i] != pgm_read_uint8_t(&(reference[i]))) {
				return 0;
			}
		}

		// Test passed; all is good.
		return 1;
	}

	void MFRC522::delay(int ms) {
		uint32_t one_ms = (ROM_SysCtlClockGet() / 3) / 1000;
		SysCtlDelay(one_ms * ms);
	}

	uint8_t MFRC522::isNewCardPresent() {
		uint8_t bufferATQA[2];
		uint8_t bufferSize = sizeof(bufferATQA);
		StatusCode result = REQA_or_WUPA(PICC_CMD_REQA, bufferATQA, &bufferSize);
		return (result == STATUS_OK || result == STATUS_COLLISION);
	}

	StatusCode MFRC522::REQA_or_WUPA(uint8_t command, uint8_t *bufferATQA, uint8_t *bufferSize) {
		uint8_t validBits;
		StatusCode status;

		if (bufferATQA == NULL || *bufferSize < 2) {
			return STATUS_NO_ROOM;
		}
		clearRegisterBitMask(CollReg, 0x80);
		validBits = 7;
		status = transceiveData(&command, 1, bufferATQA, bufferSize, &validBits);
		if (status != STATUS_OK) {
			return status;
		}
		if (*bufferSize != 2 || validBits != 0) {		// ATQA must be exactly 16 bits.
			return STATUS_ERROR;
		}
		return STATUS_OK;
	}

	StatusCode MFRC522::transceiveData(uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint8_t *backLen, uint8_t *validBits, uint8_t rxAlign, bool checkCRC) {
		uint8_t waitIRq = 0x30;		// RxIRq and IdleIRq
		return communicateWithPICC(PCD_Transceive, waitIRq, sendData, sendLen, backData, backLen, validBits, rxAlign, checkCRC);
	}

	StatusCode MFRC522::communicateWithPICC(uint8_t command, uint8_t waitIRq, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint8_t *backLen, uint8_t *validBits, uint8_t rxAlign, bool checkCRC) {
		uint8_t n, _validBits;
		unsigned int i;

		// Prepare values for BitFramingReg
		uint8_t txLastBits = validBits ? *validBits : 0;
		uint8_t bitFraming = (rxAlign << 4) + txLastBits;

		writeRegister(CommandReg, PCD_Idle);			// Stop any active command.
		writeRegister(ComIrqReg, 0x7F);					// Clear all seven interrupt request bits
		setRegisterBitMask(FIFOLevelReg, 0x80);			// FlushBuffer = 1, FIFO initialization
		writeRegister(FIFODataReg, sendLen, sendData);	// Write sendData to the FIFO
		writeRegister(BitFramingReg, bitFraming);		// Bit adjustments
		writeRegister(CommandReg, command);				// Execute the command
		if (command == PCD_Transceive) {
			setRegisterBitMask(BitFramingReg, 0x80);	// StartSend=1, transmission of data starts
		}

		i = 2000;
		while (1) {
			n = readRegister(ComIrqReg);	// ComIrqReg[7..0] bits are: Set1 TxIRq RxIRq IdleIRq HiAlertIRq LoAlertIRq ErrIRq TimerIRq
			if (n & waitIRq) {					// One of the interrupts that signal success has been set.
				break;
			}
			if (n & 0x01) {						// Timer interrupt - nothing received in 25ms
				return STATUS_TIMEOUT;
			}
			if (--i == 0) {						// The emergency break. If all other conditions fail we will eventually terminate on this one after 35.7ms. Communication with the MFRC522 might be down.
				return STATUS_TIMEOUT;
			}
		}

		// Stop now if any errors except collisions were detected.
		uint8_t errorRegValue = readRegister(ErrorReg); // ErrorReg[7..0] bits are: WrErr TempErr reserved BufferOvfl CollErr CRCErr ParityErr ProtocolErr
		if (errorRegValue & 0x13) {	 // BufferOvfl ParityErr ProtocolErr
			return STATUS_ERROR;
		}

		// If the caller wants data back, get it from the MFRC522.
		if (backData && backLen) {
			n = readRegister(FIFOLevelReg);			// Number of uint8_ts in the FIFO
			if (n > *backLen) {
				return STATUS_NO_ROOM;
			}
			*backLen = n;											// Number of uint8_ts returned
			readRegister(FIFODataReg, n, backData, rxAlign);	// Get received data from FIFO
			_validBits = readRegister(ControlReg) & 0x07;		// RxLastBits[2:0] indicates the number of valid bits in the last received uint8_t. If this value is 000b, the whole uint8_t is valid.
			if (validBits) {
				*validBits = _validBits;
			}
		}

		// Tell about collisions
		if (errorRegValue & 0x08) {		// CollErr
			return STATUS_COLLISION;
		}

		// Perform CRC_A validation if requested.
		if (backData && backLen && checkCRC) {
			// In this case a MIFARE Classic NAK is not OK.
			if (*backLen == 1 && _validBits == 4) {
				return STATUS_MIFARE_NACK;
			}
			// We need at least the CRC_A value and all 8 bits of the last uint8_t must be received.
			if (*backLen < 2 || _validBits != 0) {
				return STATUS_CRC_WRONG;
			}
			// Verify CRC_A - do our own calculation and store the control in controlBuffer.
			uint8_t controlBuffer[2];
			StatusCode status = calculateCRC(&backData[0], *backLen - 2, &controlBuffer[0]);
			if (status != STATUS_OK) {
				return status;
			}
			if ((backData[*backLen - 2] != controlBuffer[0]) || (backData[*backLen - 1] != controlBuffer[1])) {
				return STATUS_CRC_WRONG;
			}
		}

		return STATUS_OK;
	}

	StatusCode MFRC522::calculateCRC(uint8_t *data, uint8_t length, uint8_t *result) {
		writeRegister(CommandReg, PCD_Idle);		// Stop any active command.
		writeRegister(DivIrqReg, 0x04);				// Clear the CRCIRq interrupt request bit
		setRegisterBitMask(FIFOLevelReg, 0x80);		// FlushBuffer = 1, FIFO initialization
		writeRegister(FIFODataReg, length, data);	// Write data to the FIFO
		writeRegister(CommandReg, PCD_CalcCRC);		// Start the calculation

		word i = 5000;
		uint8_t n;
		while (1) {
			n = readRegister(DivIrqReg);	// DivIrqReg[7..0] bits are: Set2 reserved reserved MfinActIRq reserved CRCIRq reserved reserved
			if (n & 0x04) {						// CRCIRq bit set - calculation done
				break;
			}
			if (--i == 0) {						// The emergency break. We will eventually terminate on this one after 89ms. Communication with the MFRC522 might be down.
				return STATUS_TIMEOUT;
			}
		}
		writeRegister(CommandReg, PCD_Idle);		// Stop calculating CRC for new content in the FIFO.

		// Transfer the result from the registers to the result buffer
		result[0] = readRegister(CRCResultRegL);
		result[1] = readRegister(CRCResultRegH);
		return STATUS_OK;
	}

	uint8_t MFRC522::readCardSerial() {
		StatusCode result = select_PICC(&uid);
		return (result == STATUS_OK);
	}

	StatusCode MFRC522::select_PICC(Uid *uid, uint8_t validBits) {
		uint8_t cascadeLevel = 1;
		uint8_t txLastBits;
		uint8_t *responseBuffer;
		uint8_t responseLength;

		// Sanity checks
		if (validBits > 80) {
			return STATUS_INVALID;
		}

		// Prepare MFRC522
		clearRegisterBitMask(CollReg, 0x80);		// ValuesAfterColl=1 => Bits received after collision are cleared.

		uint8_t uidComplete = 0;
		while (!uidComplete) {
			uint8_t useCascadeTag;
			uint8_t count;
			uint8_t uidIndex;
			uint8_t buffer[9];

			// Set the Cascade Level in the SEL uint8_t, find out if we need to use the Cascade Tag in uint8_t 2.
			switch (cascadeLevel) {
				case 1:
					buffer[0] = PICC_CMD_SEL_CL1;
					uidIndex = 0;
					useCascadeTag = validBits && uid->size > 4;	// When we know that the UID has more than 4 uint8_ts
					break;

				case 2:
					buffer[0] = PICC_CMD_SEL_CL2;
					uidIndex = 3;
					useCascadeTag = validBits && uid->size > 7;	// When we know that the UID has more than 7 uint8_ts
					break;

				case 3:
					buffer[0] = PICC_CMD_SEL_CL3;
					uidIndex = 6;
					useCascadeTag = 0;						// Never used in CL3.
					break;

				default:
					return STATUS_INTERNAL_ERROR;
					break;
			}

			// How many UID bits are known in this Cascade Level?
			int8_t currentLevelKnownBits = validBits - (8 * uidIndex);
			if (currentLevelKnownBits < 0) {
				currentLevelKnownBits = 0;
			}
			// Copy the known bits from uid->uidbyte[] to buffer[]
			uint8_t index = 2; // destination index in buffer[]
			if (useCascadeTag) {
				buffer[index++] = PICC_CMD_CT;
			}
			uint8_t uint8_tsToCopy = currentLevelKnownBits / 8 + (currentLevelKnownBits % 8 ? 1 : 0); // The number of uint8_ts needed to represent the known bits for this level.
			if (uint8_tsToCopy) {
				uint8_t maxuint8_ts = useCascadeTag ? 3 : 4; // Max 4 uint8_ts in each Cascade Level. Only 3 left if we use the Cascade Tag
				if (uint8_tsToCopy > maxuint8_ts) {
					uint8_tsToCopy = maxuint8_ts;
				}
				for (count = 0; count < uint8_tsToCopy; count++) {
					buffer[index++] = uid->uidbyte[uidIndex + count];
				}
			}
			// Now that the data has been copied we need to include the 8 bits in CT in currentLevelKnownBits
			if (useCascadeTag) {
				currentLevelKnownBits += 8;
			}

			// Repeat anti collision loop until we can transmit all UID bits + BCC and receive a SAK - max 32 iterations.
			uint8_t selectDone = 0;
			while (!selectDone) {
				uint8_t bufferUsed;
				// Find out how many bits and uint8_ts to send and receive.
				if (currentLevelKnownBits >= 32) { // All UID bits in this Cascade Level are known. This is a SELECT.
					//Serial.print(F("SELECT: currentLevelKnownBits=")); Serial.println(currentLevelKnownBits, DEC);
					buffer[1] = 0x70; // NVB - Number of Valid Bits: Seven whole uint8_ts
					// Calculate BCC - Block Check Character
					buffer[6] = buffer[2] ^ buffer[3] ^ buffer[4] ^ buffer[5];
					// Calculate CRC_A
					StatusCode result = calculateCRC(buffer, 7, &buffer[7]);
					if (result != STATUS_OK) {
						return result;
					}
					txLastBits		= 0; // 0 => All 8 bits are valid.
					bufferUsed		= 9;
					// Store response in the last 3 uint8_ts of buffer (BCC and CRC_A - not needed after tx)
					responseBuffer	= &buffer[6];
					responseLength	= 3;
				}
				else { // This is an ANTICOLLISION.
					//Serial.print(F("ANTICOLLISION: currentLevelKnownBits=")); Serial.println(currentLevelKnownBits, DEC);
					txLastBits		= currentLevelKnownBits % 8;
					count			= currentLevelKnownBits / 8;	// Number of whole uint8_ts in the UID part.
					index			= 2 + count;					// Number of whole uint8_ts: SEL + NVB + UIDs
					buffer[1]		= (index << 4) + txLastBits;	// NVB - Number of Valid Bits
					bufferUsed		= index + (txLastBits ? 1 : 0);
					// Store response in the unused part of buffer
					responseBuffer	= &buffer[index];
					responseLength	= sizeof(buffer) - index;
				}

				// Set bit adjustments
				uint8_t rxAlign = txLastBits;											// Having a separate variable is overkill. But it makes the next line easier to read.
				writeRegister(BitFramingReg, (rxAlign << 4) + txLastBits);	// RxAlign = BitFramingReg[6..4]. TxLastBits = BitFramingReg[2..0]

				// Transmit the buffer and receive the response.
				StatusCode result = transceiveData(buffer, bufferUsed, responseBuffer, &responseLength, &txLastBits, rxAlign);
				if (result == STATUS_COLLISION) { // More than one PICC in the field => collision.
					uint8_t valueOfCollReg = readRegister(CollReg); // CollReg[7..0] bits are: ValuesAfterColl reserved CollPosNotValid CollPos[4:0]
					if (valueOfCollReg & 0x20) { // CollPosNotValid
						return STATUS_COLLISION; // Without a valid collision position we cannot continue
					}
					uint8_t collisionPos = valueOfCollReg & 0x1F; // Values 0-31, 0 means bit 32.
					if (collisionPos == 0) {
						collisionPos = 32;
					}
					if (collisionPos <= currentLevelKnownBits) { // No progress - should not happen
						return STATUS_INTERNAL_ERROR;
					}
					// Choose the PICC with the bit set.
					currentLevelKnownBits = collisionPos;
					count			= (currentLevelKnownBits - 1) % 8; // The bit to modify
					index			= 1 + (currentLevelKnownBits / 8) + (count ? 1 : 0); // First uint8_t is index 0.
					buffer[index]	|= (1 << count);
				}
				else if (result != STATUS_OK) {
					return result;
				}
				else { // STATUS_OK
					if (currentLevelKnownBits >= 32) { // This was a SELECT.
						selectDone = 1; // No more anticollision
						// We continue below outside the while.
					}
					else { // This was an ANTICOLLISION.
						// We now have all 32 bits of the UID in this Cascade Level
						currentLevelKnownBits = 32;
						// Run loop again to do the SELECT.
					}
				}
			}

			// We do not check the CBB - it was constructed by us above.

			// Copy the found UID uint8_ts from buffer[] to uid->uidbyte[]
			index			= (buffer[2] == PICC_CMD_CT) ? 3 : 2; // source index in buffer[]
			uint8_tsToCopy		= (buffer[2] == PICC_CMD_CT) ? 3 : 4;
			for (count = 0; count < uint8_tsToCopy; count++) {
				uid->uidbyte[uidIndex + count] = buffer[index++];
			}

			// Check response SAK (Select Acknowledge)
			if (responseLength != 3 || txLastBits != 0) { // SAK must be exactly 24 bits (1 uint8_t + CRC_A).
				return STATUS_ERROR;
			}
			// Verify CRC_A - do our own calculation and store the control in buffer[2..3] - those uint8_ts are not needed anymore.
			StatusCode result = calculateCRC(responseBuffer, 1, &buffer[2]);
			if (result != STATUS_OK) {
				return result;
			}
			if ((buffer[2] != responseBuffer[1]) || (buffer[3] != responseBuffer[2])) {
				return STATUS_CRC_WRONG;
			}
			if (responseBuffer[0] & 0x04) { // Cascade bit set - UID not complete yes
				cascadeLevel++;
			}
			else {
				uidComplete = 1;
				uid->sak = responseBuffer[0];
			}
		}

		// Set correct uid->size
		uid->size = 3 * cascadeLevel + 1;

		return STATUS_OK;
	}

} /* namespace rfid_reader */
