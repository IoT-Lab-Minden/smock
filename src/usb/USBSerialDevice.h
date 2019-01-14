/**
 * \file USBSerialDevice.h
 * \brief Contains the declaration of the methods and attributes needed for the keyboard device.
 *
 *  The serial device enables a serial communication with the host.
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
#include "USBSerialConstants.h"

namespace usbdevice {

	/**
	 * \class USBSerialDevice
	 * \brief Device for serial communication.
	 *
	 * A device that enables a serial communication with the host. It provides methods to
	 * configure the connection, to write in the transmit buffer and read from the
	 * receive buffer.
	 */
	class USBSerialDevice {
	private:
		/**
		 * The serial device
		 */
		tUSBDCDCDevice g_sCDCDevice;

		/**
		 * Transmit buffer
		 */
		tUSBBuffer g_sTxBuffer;
		uint8_t g_pui8USBTxBuffer[USB_BUFFER_SIZE];

		/**
		 * Receive buffer
		 */
		tUSBBuffer g_sRxBuffer;
		uint8_t g_pui8USBRxBuffer[USB_BUFFER_SIZE];

		/**
		 * Transmit actions counter
		 */
		uint32_t g_ui32UARTTxCount;

		/**
		 * Receive actions counter
		 */
		uint32_t g_ui32UARTRxCount;

		/**
		 * Indicates if the usb communication was already configured
		 */
		bool g_bUSBConfigured;

		/**
		 * Indicates if a break has been send
		 */
		bool g_bSendingBreak;

		/**
		 * Communication flags
		 */
		uint32_t g_ui32Flags;

		/**
		 * Receive Buffer accessible from user realized as queue.
		 */
		uint8_t ui8ReceiveBuffer[USB_BUFFER_SIZE];
		uint32_t ui32ReceiveBufferStart;
		uint32_t ui32ReceiveBufferEnd;

		/**
		 * Status code
		 */
		char *g_pcStatus;

		/**
		 * Callback to handle general USB events like connecting or disconnecting
		 */
		static tUSBCallback controlHandler;

		/**
		 * Callback to handle receive events.
		 */
		static tUSBCallback rxHandler;

		/**
		 * Callback to handle transmit events.
		 */
		static tUSBCallback txHandler;

		/**
		 * \brief Constructor which builds the object.
		 * \param controlHandler The callback function that should be set controlHandler.
		 * \param rxHandler The callback function that should be set rxHandler
		 * \param txHandler The callback function that should be set txHandler
		 *
		 * The constructor initializes the g_sCDCDevice of the object and initializes the
		 * buffers.
		 */
		USBSerialDevice(tUSBCallback controlHandler, tUSBCallback rxHandler, tUSBCallback txHandler);

	public:

		/**
		 * \fn USBSerialDevice *getInstance()
		 * \brief Returns the instance of the USBSerialDevice
		 *
		 * The function creates the object the first time it is called and return the pointer
		 * to the object. The object is created on the heap. Later calls only return the
		 * pointer to this object.
		 *
		 * \return - the instance of serial device
		 */
		static USBSerialDevice *getInstance();

		/**
		 * \fn void registerControlHandler(tUSBCallback cHandler)
		 * \brief Registers a new Callback function for the control handler.
		 *
		 * \param cHandler New callback method
		 *
		 * The handler handles general USB events like connecting or disconnecting events.
		 * A new handler can only be set before the device is referenced the first time.
		 * When the device is referenced the first time, the composite device struct is
		 * initialized with the current callback method by calling the constructor of the
		 * composite device. After that there is no possibility to change the callback
		 * method. It can only be one function registered.
		 *
		 * \return void
		 */
		static void registerControlHandler(tUSBCallback cHandler);

		/**
		 * \fn void registerRxHandler(tUSBCallback rHandler)
		 * \brief Registers a new Callback function for the receive handler.
		 *
		 * \param rHandler New callback method
		 *
		 * The handler handles receive events.
		 * A new handler can only be set before the device is referenced the first time.
		 * When the device is referenced the first time, the composite device struct is
		 * initialized with the current callback method by calling the constructor of the
		 * composite device. After that there is no possibility to change the callback
		 * method. It can only be one function registered.
		 *
		 * \return void
		 */
		static void registerRxHandler(tUSBCallback rHandler);

		/**
		 * \fn void registerTxHandler(tUSBCallback tHandler)
		 * \brief Registers a new Callback function for the transmit handler.
		 *
		 * \param tHandler New callback method
		 *
		 * The handler handles transmit events.
		 * A new handler can only be set before the device is referenced the first time.
		 * When the device is referenced the first time, the composite device struct is
		 * initialized with the current callback method by calling the constructor of the
		 * composite device. After that there is no possibility to change the callback
		 * method. It can only be one function registered.
		 *
		 * \return void
		 */
		static void registerTxHandler(tUSBCallback tHandler);

		/**
		 * \fn void initBuffer()
		 * \brief Initializes the transmit and receive buffers.
		 *
		 * Initializes the rx and tx buffers by calling USBBufferInit() for both buffers.
		 *
		 * \return void
		 */
		void initBuffer();

		/**
		 * \fn void flushReceiveBuffer()
		 * \brief Flushes the receive buffer which is accessible from the user.
		 *
		 * The receive buffer is flush by writing 0 to each slot of the buffer.
		 */
		void flushReceiveBuffer();

		/**
		 * \fn tUSBDCDCDevice *getSerialDevice()
		 * \brief Getter for the g_sCDCDevice attribute.
		 *
		 * Returns the g_sCDCDevice.
		 *
		 * \return tUSBDCDCDevice - the struct standing for the keyboard device.
		 */
		tUSBDCDCDevice *getSerialDevice();

		/**
		 * \fn tUSBBuffer *getRxBuffer()
		 * \brief Getter for the rxBuffer.
		 *
		 * Returns the rxBuffer.
		 *
		 * \return tUSBBuffer - pointer to the buffer.
		 */
		tUSBBuffer *getRxBuffer();

		/**
		 * \fn tUSBBuffer *getTxBuffer()
		 * \brief Getter for the txBuffer.
		 *
		 * Returns the rxBuffer.
		 *
		 * \return tUSBBuffer - pointer to the buffer.
		 */
		tUSBBuffer *getTxBuffer();

		/**
		 * \fn void connected()
		 * \brief Called when new connection appeared.
		 *
		 * Configures new connection by flushing the rxBuffer and txBuffer and by dis- and
		 * enabling the master interrupt. Set g_bUSBConfigured to true and status to
		 * connected.
		 *
		 * \return void
		 */
		void connected();

		/**
		 * \fn void disconnected()
		 * \brief Called when a conncection is closed.
		 *
		 * Dis- and enables the master interrupt. Set g_bUSBConfigured to false and status to
		 * disconnected.
		 *
		 * \return void
		 */
		void disconnected();

		/**
		 * \fn void write(uint8_t *data, uint32_t length)
		 * \brief Writes data in transmit buffer.
		 *
		 * \param data Pointer to the data that shell be transmit.
		 * \param length Length of the data that shell be transmit.
		 *
		 * Calls USBBufferWrite() to write the data into the txBuffer.
		 *
		 * \return void
		 */
		void write(uint8_t *data, uint32_t length);

		/**
		 * \fn uint8_t popReceiveBuffer()
		 * \brief Returns top of queue
		 *
		 * \return uint8_t Returns the first byte of receive queue
		 */
		uint8_t popReceiveBuffer();

		/**
		 * \fn int pushReceiveBuffer(uint8_t data)
		 * \brief Adds add the end of the queue
		 *
		 * \param data The data that should be added to the queue
		 *
		 * The data is added to the end of the queue. If the queue is
		 * full, the new data discarded.
		 *
		 * \return int - 0 if the buffer is full. 1 otherwise.
		 */
		int pushReceiveBuffer(uint8_t data);

		/**
		 * \fn uint32_t getReceiveBufferLength()
		 * \brief Getter for the length of the user accessible reciveBuffer.
		 *
		 * Returns the length of the user accessible reciveBuffer.
		 *
		 * \return uint32_t - length of the buffer.
		 */
		uint32_t getReceiveBufferLength();

		/**
		 * \fn uint32_t getRxEventCount()
		 * \brief Getter for the number of receive events.
		 *
		 * Returns the number of receive events.
		 *
		 * \return uint32_t - number of receive events.
		 */
		uint32_t getRxEventCount();

		/**
		 * \fn bool commandStatusUpdated()
		 * \brief Indicates weather the COMMAND_STATUS_UPDATE flag is that
		 *
		 * Returns the command status update flag as bool.
		 *
		 * \return bool - command status update flag as bool.
		 */
		bool commandStatusUpdated();

		/**
		 * \fn void resetCommandStatusFlag()
		 * \brief Resets command status flag
		 *
		 * Command status flag is set to 0.
		 *
		 * \return void
		 */
		void resetCommandStatusFlag();

		/**
		 * \fn void readBuffer()
		 * \brief Writes data from the rxBuffer into the receive buffer.
		 *
		 * Reads the rx Buffer and writes the data into the receive buffer. After this the
		 * rx buffer gets flushed by calling USBPrimeTransmitt().
		 *
		 * \return void
		 */
		void readBuffer();

		/**
		 * \fn void USBUARTPrimeTransmit()
		 *
		 * Take as many bytes from the transmit buffer as we have space for and move
		 * them into the USB UART's transmit FIFO. Afterwards the rx buffer is flushed.
		 */
		void USBUARTPrimeTransmit();

		/**
		 * \fn void CheckForSerialStateChange(const tUSBDCDCDevice *psDevice, int32_t i32Errors)
		 * \brief Checks the error flags and reacts accordingly.
		 *
		 * \param psDevice - the serial device struct
		 * \param i32Errors - error flags
		 *
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
		 *
		 * \return void
		 */
		void CheckForSerialStateChange(const tUSBDCDCDevice *psDevice, int32_t i32Errors);

		/**
		 * \fn void SetControlLineState(uint16_t ui16State)
		 * \brief Set the state of the RS232 RTS and DTR signals.
		 *
		 * \param ui16State State informations
		 *
		 * \return void
		 */
		void SetControlLineState(uint16_t ui16State);

		/**
		 * \fn bool SetLineCoding(tLineCoding *psLineCoding)
		 * \brief Set the communication parameters to use on the UART.
		 *
		 * \param psLineCoding Contains the informations for the line coding which should be set
		 *
		 * \return bool - true when setting of the line coding was successfull, false otherwise.
		 */
		bool SetLineCoding(tLineCoding *psLineCoding);

		/**
		 * \fn void GetLineCoding(tLineCoding *psLineCoding)
		 * \brief Get the communication parameters in use on the UART.
		 *
		 * \param psLineCoding Contains the informations for the line coding
		 *
		 * \return void
		 */
		void GetLineCoding(tLineCoding *psLineCoding);

		/**
		 * \fn void SendBreak(bool bSend)
		 * \brief Sets or clears a break condition.
		 *
		 * \param bSend Indicating weather to set or clear break condition
		 *
		 * This function sets or clears a break condition on the redirected UART RX
		 * line. A break is started when the function is called with bSend set to
		 * true and persists until the function is called again with bSend set
		 * to false.
		 *
		 * \return void
		 */
		void SendBreak(bool bSend);
	};

} /* namespace usbdevice */

#endif /* USBSERIALDEVICE_H_ */
