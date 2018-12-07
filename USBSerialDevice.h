/*
 * USBSerialDevice.h
 *
 *  Created on: 06.12.2018
 *      Author: malte
 */

#ifndef USBSERIALDEVICE_H_
#define USBSERIALDEVICE_H_

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"

#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"

#include "usblib/usblib.h"
#include "usblib/usbcdc.h"

#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdcdc.h"

namespace usbdevice {
	const uint32_t DEFAULT_BIT_RATE = 115200;
	const uint32_t DEFAULT_UART_CONFIG = (UART_CONFIG_WLEN_8 | UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE);

	const uint32_t USB_UART_BASE = UART0_BASE;
	const uint32_t USB_UART_PERIPH = SYSCTL_PERIPH_UART0;
	const uint32_t USB_UART_INT = INT_UART0;

	const uint32_t TX_GPIO_BASE = GPIO_PORTA_BASE;
	const uint32_t TX_GPIO_PERIPH = SYSCTL_PERIPH_GPIOA;
	const uint32_t TX_GPIO_PIN = GPIO_PIN_1;

	const uint32_t RX_GPIO_BASE = GPIO_PORTA_BASE;
	const uint32_t RX_GPIO_PERIPH = SYSCTL_PERIPH_GPIOA;
	const uint32_t RX_GPIO_PIN = GPIO_PIN_0;

	const uint32_t COMMAND_PACKET_RECEIVED = 0x00000001;
	const uint32_t COMMAND_STATUS_UPDATE = 0x00000002;
	const uint32_t UART_BUFFER_SIZE = 256;

	class USBSerialDevice {

	private:
		tUSBDCDCDevice g_sCDCDevice;

		tUSBBuffer g_sTxBuffer;
		uint8_t g_pui8USBTxBuffer[UART_BUFFER_SIZE];

		tUSBBuffer g_sRxBuffer;
		uint8_t g_pui8USBRxBuffer[UART_BUFFER_SIZE];

		uint32_t g_ui32UARTTxCount;
		uint32_t g_ui32UARTRxCount;

		bool g_bUSBConfigured;
		bool g_bSendingBreak;

		uint32_t g_ui32Flags;

		uint8_t ui8ReceiveBuffer[UART_BUFFER_SIZE];
		uint32_t ui32ReceiveBufferEnd;

		char *g_pcStatus;

		static tUSBCallback controlHandler;

		static tUSBCallback rxHandler;

		static tUSBCallback txHandler;

		USBSerialDevice(tUSBCallback controlHandler, tUSBCallback rxHandler, tUSBCallback txHandler);

	public:
		static USBSerialDevice *getInstance();

		static void registerControlHandler(tUSBCallback cHandler);

		static void registerRxHandler(tUSBCallback rHandler);

		static void registerTxHandler(tUSBCallback tHandler);

		void initBuffer();

		tUSBDCDCDevice *getSerialDevice();

		tUSBBuffer *getRxBuffer();

		tUSBBuffer *getTxBuffer();

		void connected();

		void disconnected();

		void write(uint8_t *data, uint32_t length);

		uint8_t *getReceiveBuffer();

		uint32_t getReceiveBufferEnd();

		uint32_t getRxEventCount();

		bool commandStatusUpdated();

		void resetCommandStatusFlag();

		/**
		 * Read the rx Buffer and writes the data into the receive buffer. After this the rx buffer gets flushed
		 */
		void readBuffer();

		/**
		 * Take as many bytes from the transmit buffer as we have space for and move
		 * them into the USB UART's transmit FIFO.
		 */
		void USBUARTPrimeTransmit();

		/**
		 * This function is called whenever serial data is received from the UART.
		 * It is passed the accumulated error flags from each character received in
		 * this interrupt and determines from them whether or not an interrupt
		 * notification to the host is required.
		 *
		 * If a notification is required and the control interrupt endpoint is idle,
		 * we send the notification immediately.  If the endpoint is not idle, we
		 * accumulate the errors in a global variable which will be checked on
		 * completion of the previous notification and used to send a second one
		 * if necessary.
		 */
		void CheckForSerialStateChange(const tUSBDCDCDevice *psDevice, int32_t i32Errors);

		/**
		 * Set the state of the RS232 RTS and DTR signals.
		 */
		void SetControlLineState(uint16_t ui16State);

		/**
		 * Set the communication parameters to use on the UART.
		 */
		bool SetLineCoding(tLineCoding *psLineCoding);

		/**
		 * Get the communication parameters in use on the UART.
		 */
		void GetLineCoding(tLineCoding *psLineCoding);

		/**
		 * This function sets or clears a break condition on the redirected UART RX
		 * line.  A break is started when the function is called with \e bSend set to
		 * \b true and persists until the function is called again with \e bSend set
		 * to \b false.
		 */
		void SendBreak(bool bSend);
	};

} /* namespace usbdevice */

#endif /* USBSERIALDEVICE_H_ */
