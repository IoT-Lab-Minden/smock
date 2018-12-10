/*
 * USBSerialConstants.h
 *
 *  Created on: 10.12.2018
 *      Author: malte
 */

#ifndef USBSERIALCONSTANTS_H_
#define USBSERIALCONSTANTS_H_

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
}



#endif /* USBSERIALCONSTANTS_H_ */
