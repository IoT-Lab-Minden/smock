/*
 * USBSerialDevice.cpp
 *
 *  Created on: 06.12.2018
 *      Author: malte
 */

#include "USBSerialDevice.h"
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_uart.h"
#include "driverlib/rom.h"
#include "driverlib/usb.h"
#include "usblib/usblib.h"
#include "usblib/usbcdc.h"
#include "usblib/usb-ids.h"
#include "usblib/usbhid.h"
#include "driverlib/uart.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdcdc.h"
#include "usblib/device/usbdhid.h"
#include "usblib/device/usbdhidkeyb.h"
#include "usb_structs.h"

namespace usbdevice {

	tUSBCallback USBSerialDevice::controlHandler = 0;

	tUSBCallback USBSerialDevice::rxHandler = 0;

	tUSBCallback USBSerialDevice::txHandler = 0;

	void USBSerialDevice::registerControlHandler(tUSBCallback cHandler) {
		controlHandler = cHandler;
	}

	void USBSerialDevice::registerRxHandler(tUSBCallback rHandler) {
		rxHandler = rHandler;
	}

	void USBSerialDevice::registerTxHandler(tUSBCallback tHandler) {
		txHandler = tHandler;
	}

	USBSerialDevice *USBSerialDevice::getInstance() {
		static USBSerialDevice instance(controlHandler, rxHandler, txHandler);
		return &instance;
	}

	USBSerialDevice::USBSerialDevice(tUSBCallback controlHandler, tUSBCallback rxHandler, tUSBCallback txHandler) :
	g_sCDCDevice( {
				USB_VID_TI_1CBE,
				USB_PID_SERIAL,
				0,
				USB_CONF_ATTR_SELF_PWR,
				controlHandler,
				(void *)&g_sCDCDevice,
				USBBufferEventCallback,
				(void *)&g_sRxBuffer,
				USBBufferEventCallback,
				(void *)&g_sTxBuffer,
				0,
				0,
			}),
	g_sTxBuffer ( {
				true,
				txHandler,
				(void *)&g_sCDCDevice,
				USBDCDCPacketWrite,
				USBDCDCTxPacketAvailable,
				(void *)&g_sCDCDevice,
				g_pui8USBTxBuffer,
				UART_BUFFER_SIZE,
			}),
	g_sRxBuffer ( {
				false,
				rxHandler,
				(void *)&g_sCDCDevice,
				USBDCDCPacketRead,
				USBDCDCRxPacketAvailable,
				(void *)&g_sCDCDevice,
				g_pui8USBRxBuffer,
				UART_BUFFER_SIZE,
			}),
	g_bSendingBreak(false)
	{}


	tUSBDCDCDevice *USBSerialDevice::getSerialDevice() {
		return &g_sCDCDevice;
	}

	tUSBBuffer *USBSerialDevice::getRxBuffer() {
		return &g_sRxBuffer;
	}

	tUSBBuffer *USBSerialDevice::getTxBuffer() {
		return &g_sTxBuffer;
	}

	void USBSerialDevice::connected() {
		g_bUSBConfigured = true;
		USBBufferFlush(&g_sTxBuffer);
		USBBufferFlush(&g_sRxBuffer);

		uint32_t ui32IntsOff = ROM_IntMasterDisable();
		g_pcStatus = "Connected";
		g_ui32Flags |= COMMAND_STATUS_UPDATE;
		if (!ui32IntsOff) {
			ROM_IntMasterEnable();
		}
	}

	void USBSerialDevice::disconnected() {
		g_bUSBConfigured = false;
		uint32_t ui32IntsOff = ROM_IntMasterDisable();
		g_pcStatus = "Disconnected";
		g_ui32Flags |= COMMAND_STATUS_UPDATE;
		if (!ui32IntsOff) {
			ROM_IntMasterEnable();
		}
	}

	void USBSerialDevice::write(uint8_t *data, uint32_t length) {
		USBBufferWrite((tUSBBuffer *) &g_sTxBuffer, data, length);
	}


	uint8_t *USBSerialDevice::getReceiveBuffer() {
		return ui8ReceiveBuffer;
	}

	uint32_t USBSerialDevice::getReceiveBufferEnd() {
		return ui32ReceiveBufferEnd;
	}

	uint32_t USBSerialDevice::getRxEventCount() {
		return g_ui32UARTRxCount;
	}

	bool USBSerialDevice::commandStatusUpdated() {
		return g_ui32Flags & COMMAND_STATUS_UPDATE;
	}

	void USBSerialDevice::resetCommandStatusFlag() {
		g_ui32Flags &= ~COMMAND_STATUS_UPDATE;
	}

	void USBSerialDevice::initBuffer() {
		USBBufferInit(&g_sTxBuffer);
		USBBufferInit(&g_sRxBuffer);
	}

	void USBSerialDevice::readBuffer() {
		ui32ReceiveBufferEnd = USBBufferRead((tUSBBuffer *) &g_sRxBuffer, ui8ReceiveBuffer, UART_BUFFER_SIZE);
		g_ui32UARTRxCount += ui32ReceiveBufferEnd;
		USBUARTPrimeTransmit();
	}

	void USBSerialDevice::USBUARTPrimeTransmit()
	{
		uint32_t ui32Read;
		uint8_t ui8Char;

		//
		// If we are currently sending a break condition, don't receive any
		// more data. We will resume transmission once the break is turned off.
		//
		if (g_bSendingBreak)
		{
			return;
		}

		//
		// If there is space in the UART FIFO, try to read some characters
		// from the receive buffer to fill it again.
		//
		while (ROM_UARTSpaceAvail(USB_UART_BASE))
		{
			//
			// Get a character from the buffer.
			//
			ui32Read = USBBufferRead((tUSBBuffer *) &g_sRxBuffer, &ui8Char, 1);
			//
			// Did we get a character?
			//
			if (ui32Read)
			{
				//
				// Place the character in the UART transmit FIFO.
				//
				ROM_UARTCharPutNonBlocking(USB_UART_BASE, ui8Char);

				//
				// Update our count of bytes transmitted via the UART.
				//
				g_ui32UARTTxCount++;
			}
			else
			{
				//
				// We ran out of characters so exit the function.
				//
				return;
			}
		}
	}

	void USBSerialDevice::CheckForSerialStateChange(const tUSBDCDCDevice *psDevice,
			int32_t i32Errors)
	{
		uint16_t ui16SerialState;

		//
		// Clear our USB serial state.  Since we are faking the handshakes, always
		// set the TXCARRIER (DSR) and RXCARRIER (DCD) bits.
		//
		ui16SerialState = USB_CDC_SERIAL_STATE_TXCARRIER |
		USB_CDC_SERIAL_STATE_RXCARRIER;

		//
		// Are any error bits set?
		//
		if (i32Errors)
		{
			//
			// At least one error is being notified so translate from our hardware
			// error bits into the correct state markers for the USB notification.
			//
			if (i32Errors & UART_DR_OE)
			{
				ui16SerialState |= USB_CDC_SERIAL_STATE_OVERRUN;
			}

			if (i32Errors & UART_DR_PE)
			{
				ui16SerialState |= USB_CDC_SERIAL_STATE_PARITY;
			}

			if (i32Errors & UART_DR_FE)
			{
				ui16SerialState |= USB_CDC_SERIAL_STATE_FRAMING;
			}

			if (i32Errors & UART_DR_BE)
			{
				ui16SerialState |= USB_CDC_SERIAL_STATE_BREAK;
			}

			// Call the CDC driver to notify the state change.
			USBDCDCSerialStateChange((void *) psDevice, ui16SerialState);
		}
	}

	void USBSerialDevice::SetControlLineState(uint16_t ui16State)
	{
		// TODO: If configured with GPIOs controlling the handshake lines,
		// set them appropriately depending upon the flags passed in the wValue
		// field of the request structure passed.
	}

	bool USBSerialDevice::SetLineCoding(tLineCoding *psLineCoding)
	{
		uint32_t ui32Config;
		bool bRetcode;

		//
		// Assume everything is OK until we detect any problem.
		//
		bRetcode = true;

		//
		// Word length.  For invalid values, the default is to set 8 bits per
		// character and return an error.
		//
		switch (psLineCoding->ui8Databits)
		{
			case 5:
			{
				ui32Config = UART_CONFIG_WLEN_5;
				break;
			}

			case 6:
			{
				ui32Config = UART_CONFIG_WLEN_6;
				break;
			}

			case 7:
			{
				ui32Config = UART_CONFIG_WLEN_7;
				break;
			}

			case 8:
			{
				ui32Config = UART_CONFIG_WLEN_8;
				break;
			}

			default:
			{
				ui32Config = UART_CONFIG_WLEN_8;
				bRetcode = false;
				break;
			}
		}

		//
		// Parity.  For any invalid values, we set no parity and return an error.
		//
		switch (psLineCoding->ui8Parity)
		{
			case USB_CDC_PARITY_NONE:
			{
				ui32Config |= UART_CONFIG_PAR_NONE;
				break;
			}

			case USB_CDC_PARITY_ODD:
			{
				ui32Config |= UART_CONFIG_PAR_ODD;
				break;
			}

			case USB_CDC_PARITY_EVEN:
			{
				ui32Config |= UART_CONFIG_PAR_EVEN;
				break;
			}

			case USB_CDC_PARITY_MARK:
			{
				ui32Config |= UART_CONFIG_PAR_ONE;
				break;
			}

			case USB_CDC_PARITY_SPACE:
			{
				ui32Config |= UART_CONFIG_PAR_ZERO;
				break;
			}

			default:
			{
				ui32Config |= UART_CONFIG_PAR_NONE;
				bRetcode = false;
				break;
			}
		}

		//
		// Stop bits.  Our hardware only supports 1 or 2 stop bits whereas CDC
		// allows the host to select 1.5 stop bits.  If passed 1.5 (or any other
		// invalid or unsupported value of ui8Stop, we set up for 1 stop bit but
		// return an error in case the caller needs to Stall or otherwise report
		// this back to the host.
		//
		switch (psLineCoding->ui8Stop)
		{
			//
			// One stop bit requested.
			//
			case USB_CDC_STOP_BITS_1:
			{
				ui32Config |= UART_CONFIG_STOP_ONE;
				break;
			}

			//
			// Two stop bits requested.
			//
			case USB_CDC_STOP_BITS_2:
			{
				ui32Config |= UART_CONFIG_STOP_TWO;
				break;
			}

			//
			// Other cases are either invalid values of ui8Stop or values that we
			// cannot support so set 1 stop bit but return an error.
			//
			default:
			{
				ui32Config |= UART_CONFIG_STOP_ONE;
				bRetcode = false;
				break;
			}
		}

		//
		// Set the UART mode appropriately.
		//
		ROM_UARTConfigSetExpClk(USB_UART_BASE, ROM_SysCtlClockGet(),
				psLineCoding->ui32Rate, ui32Config);

		//
		// Let the caller know if we had a problem or not.
		//
		return (bRetcode);
	}

	void USBSerialDevice::GetLineCoding(tLineCoding *psLineCoding)
	{
		uint32_t ui32Config;
		uint32_t ui32Rate;

		//
		// Get the current line coding set in the UART.
		//
		ROM_UARTConfigGetExpClk(USB_UART_BASE, ROM_SysCtlClockGet(), &ui32Rate,
				&ui32Config);
		psLineCoding->ui32Rate = ui32Rate;

		//
		// Translate the configuration word length field into the format expected
		// by the host.
		//
		switch (ui32Config & UART_CONFIG_WLEN_MASK)
		{
			case UART_CONFIG_WLEN_8:
			{
				psLineCoding->ui8Databits = 8;
				break;
			}

			case UART_CONFIG_WLEN_7:
			{
				psLineCoding->ui8Databits = 7;
				break;
			}

			case UART_CONFIG_WLEN_6:
			{
				psLineCoding->ui8Databits = 6;
				break;
			}

			case UART_CONFIG_WLEN_5:
			{
				psLineCoding->ui8Databits = 5;
				break;
			}
		}

		//
		// Translate the configuration parity field into the format expected
		// by the host.
		//
		switch (ui32Config & UART_CONFIG_PAR_MASK)
		{
			case UART_CONFIG_PAR_NONE:
			{
				psLineCoding->ui8Parity = USB_CDC_PARITY_NONE;
				break;
			}

			case UART_CONFIG_PAR_ODD:
			{
				psLineCoding->ui8Parity = USB_CDC_PARITY_ODD;
				break;
			}

			case UART_CONFIG_PAR_EVEN:
			{
				psLineCoding->ui8Parity = USB_CDC_PARITY_EVEN;
				break;
			}

			case UART_CONFIG_PAR_ONE:
			{
				psLineCoding->ui8Parity = USB_CDC_PARITY_MARK;
				break;
			}

			case UART_CONFIG_PAR_ZERO:
			{
				psLineCoding->ui8Parity = USB_CDC_PARITY_SPACE;
				break;
			}
		}

		//
		// Translate the configuration stop bits field into the format expected
		// by the host.
		//
		switch (ui32Config & UART_CONFIG_STOP_MASK)
		{
			case UART_CONFIG_STOP_ONE:
			{
				psLineCoding->ui8Stop = USB_CDC_STOP_BITS_1;
				break;
			}

			case UART_CONFIG_STOP_TWO:
			{
				psLineCoding->ui8Stop = USB_CDC_STOP_BITS_2;
				break;
			}
		}
	}

	void USBSerialDevice::SendBreak(bool bSend)
	{
		//
		// Are we being asked to start or stop the break condition?
		//
		if (!bSend)
		{
			//
			// Remove the break condition on the line.
			//
			ROM_UARTBreakCtl(USB_UART_BASE, false);
			g_bSendingBreak = false;
		}
		else
		{
			//
			// Start sending a break condition on the line.
			//
			ROM_UARTBreakCtl(USB_UART_BASE, true);
			g_bSendingBreak = true;
		}
	}

} /* namespace usbdevice */
