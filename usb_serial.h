/*
 * usb_serial.h
 *
 *  Created on: 01.12.2018
 *      Author: malte
 */

#ifndef USB_SERIAL_H_
#define USB_SERIAL_H_

#define DEFAULT_BIT_RATE        115200
#define DEFAULT_UART_CONFIG     (UART_CONFIG_WLEN_8 | UART_CONFIG_PAR_NONE | \
                                 UART_CONFIG_STOP_ONE)

#define USB_UART_BASE           UART0_BASE
#define USB_UART_PERIPH         SYSCTL_PERIPH_UART0
#define USB_UART_INT            INT_UART0

#define TX_GPIO_BASE            GPIO_PORTA_BASE
#define TX_GPIO_PERIPH          SYSCTL_PERIPH_GPIOA
#define TX_GPIO_PIN             GPIO_PIN_1

#define RX_GPIO_BASE            GPIO_PORTA_BASE
#define RX_GPIO_PERIPH          SYSCTL_PERIPH_GPIOA
#define RX_GPIO_PIN             GPIO_PIN_0

#define COMMAND_PACKET_RECEIVED 0x00000001
#define COMMAND_STATUS_UPDATE   0x00000002

extern bool g_bUSBConfigured;
extern bool g_bSendingBreak;

extern uint32_t g_ui32UARTTxCount;
extern uint32_t g_ui32UARTRxCount;

extern uint8_t ui8ReceiveBuffer[];
extern uint32_t ui32ReceiveBufferEnd;

extern uint32_t g_ui32Flag;
extern char *g_pcStatus;

/**
 * Handles CDC driver notifications related to the receive channel (data from
 * the USB host).
 *
 * \param ui32CBData is the client-supplied callback data value for this channel.
 * \param ui32Event identifies the event we are being notified about.
 * \param ui32MsgValue is an event-specific value.
 * \param pvMsgData is an event-specific pointer.
 *
 * This function is called by the CDC driver to notify us of any events
 * related to operation of the receive data channel (the OUT channel carrying
 * data from the USB host).
 *
 * \return The return value is event-specific.
 */
uint32_t RxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,
                   void *pvMsgData);

/**
 * Handles CDC driver notifications related to the transmit channel (data to
 * the USB host).
 *
 * \param ui32CBData is the client-supplied callback pointer for this channel.
 * \param ui32Event identifies the event we are being notified about.
 * \param ui32MsgValue is an event-specific value.
 * \param pvMsgData is an event-specific pointer.
 *
 * This function is called by the CDC driver to notify us of any events
 * related to operation of the transmit data channel (the IN channel carrying
 * data to the USB host).
 *
 * \return The return value is event-specific.
 */
uint32_t TxHandler(void *pvi32CBData, uint32_t ui32Event, uint32_t ui32MsgValue,
                   void *pvMsgData);

/**
 * Handles CDC driver notifications related to control and setup of the device.
 *
 * \param pvCBData is the client-supplied callback pointer for this channel.
 * \param ui32Event identifies the event we are being notified about.
 * \param ui32MsgValue is an event-specific value.
 * \param pvMsgData is an event-specific pointer.
 *
 * This function is called by the CDC driver to perform control-related
 * operations on behalf of the USB host.  These functions include setting
 * and querying the serial communication parameters, setting handshake line
 * states and sending break conditions.
 *
 * \return The return value is event-specific.
 */
uint32_t ControlHandler(void *pvCBData, uint32_t ui32Event,
                        uint32_t ui32MsgValue, void *pvMsgData);

/**
 * Take as many bytes from the transmit buffer as we have space for and move
 * them into the USB UART's transmit FIFO.
 */
void USBUARTPrimeTransmit(uint32_t ui32Base);

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
void CheckForSerialStateChange(const tUSBDCDCDevice *psDevice,
                               int32_t i32Errors);

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

#endif /* USB_SERIAL_H_ */
