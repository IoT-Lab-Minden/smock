/**
 * \file USBSerialConstants.h
 * \brief Contains the constants needed for the Serial Device
 *  Created on: 10.12.2018
 *      Author: malte
 */

#ifndef USBSERIALCONSTANTS_H_
#define USBSERIALCONSTANTS_H_

namespace usbdevice {
	/**
	 * \var const uint32_t DEFAULT_BIT_RATE
	 *
	 * The default bit rate for the serial communication.
	 */
	const uint32_t DEFAULT_BIT_RATE = 115200;

	/**
	 * \var const uint32_t DEFAULT_UART_CONFIG
	 *
	 * Default configuration for UART.
	 */
	const uint32_t DEFAULT_UART_CONFIG = (UART_CONFIG_WLEN_8 | UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE);

	/**
	 * var const uint32_t USB_UART_BASE
	 *
	 * The UART base for the communication.
	 */
	const uint32_t USB_UART_BASE = UART0_BASE;

	/**
	 * \var const uint32_t USB_UART_PERIPH
	 *
	 * The peripheral UART.
	 */
	const uint32_t USB_UART_PERIPH = SYSCTL_PERIPH_UART0;

	/**
	 * \var const uint32_t USB_UART_INT
	 *
	 * The UART int.
	 */
	const uint32_t USB_UART_INT = INT_UART0;

	/**
	 * \var const uint32_t TX_GPIO_BASE
	 *
	 * Transmit GPIO base.
	 */
	const uint32_t TX_GPIO_BASE = GPIO_PORTA_BASE;

	/**
	 * \var const uint32_t TX_GPIO_PERIPH
	 *
	 * Transmit GPIO peripheral
	 */
	const uint32_t TX_GPIO_PERIPH = SYSCTL_PERIPH_GPIOA;

	/**
	 * \var const uint32_t TX_GPIO_PIN
	 *
	 * Transmit GPIO pin.
	 */
	const uint32_t TX_GPIO_PIN = GPIO_PIN_1;

	/**
	 * \var const uint32_t RX_GPIO_BASE
	 *
	 * Receive GPIO base.
	 */
	const uint32_t RX_GPIO_BASE = GPIO_PORTA_BASE;

	/**
	 * \var const uint32_t RX_GPIO_PERIPH
	 *
	 * Receive GPIO peripheral.
	 */
	const uint32_t RX_GPIO_PERIPH = SYSCTL_PERIPH_GPIOA;

	/**
	 * \var const uint32_t RX_GPIO_PIN
	 *
	 * Receive GPIO pin.
	 */
	const uint32_t RX_GPIO_PIN = GPIO_PIN_0;

	/**
	 * \var const uint32_t COMMAND_PACKET_RECEIVED
	 *
	 * Code for the packet received command.
	 */
	const uint32_t COMMAND_PACKET_RECEIVED = 0x00000001;

	/**
	 * \var const uint32_t COMMAND_STATUS_UPDATE
	 *
	 * Code for status update command.
	 */
	const uint32_t COMMAND_STATUS_UPDATE = 0x00000002;

	/**
	 * \var const uint32_t UART_BUFFER_SIZE
	 *
	 * Size of the UART Buffer.
	 */
	const uint32_t UART_BUFFER_SIZE = 256;
}



#endif /* USBSERIALCONSTANTS_H_ */
