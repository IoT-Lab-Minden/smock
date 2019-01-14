#include <stdint.h>
#include <string.h> // ?
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_adc.h"
#include "grlib/grlib.h"
#include "grlib/widget.h"
#include "grlib/keyboard.h"
#include "driverlib/fpu.h"
#include "driverlib/pin_map.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/rom.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/udma.h"
#include "driverlib/uart.h"
#include "usblib/usblib.h"

#include "usblib/usbhid.h"
#include "usblib/usb-ids.h"
#include "usblib/usbcdc.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdcomp.h"
#include "usblib/device/usbdhid.h" //?
#include "usblib/device/usbdhidkeyb.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "drivers/buttons.h" // <-----
#include "usb/usb_structs.h"
#include "usb/USBSerialDevice.h"
#include "usb/USBKeyboardDevice.h"
#include "usb/USBCompDevice.h"
#include "energia/SPI.h"
#include "energia/Wire.h"
#include "rfid/MFRC522.h"
#include "rfid/MFRC522Constants.h"
#include "energia/Stream.h"

#define TARGET_IS_BLIZZARD_RB1

using namespace usbdevice;

/**
 * \var const int START_DELAY
 *
 * Delay before configuring the usb-device
 */
const int START_DELAY = 1000;

/**
 * \var const int WAIT_FOR_HOST_KONFIGURATION
 *
 * Delay before main-loop, so windows has time to detect the deviceDelay waited before the.
 */
const int WAIT_FOR_HOST_KONFIGURATION = 500;

/**
 * \var const int WAIT_LOG_IN
 *
 * Delay after logging in the single user.
 */
volatile const int WAIT_LOG_IN = 400;

/**
 * \var const int WAIT_LOG_IN_LONG
 *
 * Delay after logging in the multi user.
 */
volatile const int WAIT_LOG_IN_LONG = 3000;

/**
 * \var const int ENTER_DELAY
 *
 * Short delay after pressing enter.
 */
const int ENTER_DELAY = 350;

/**
 * \var const uint8_t LINUX
 *
 * Code for a LINUX System
 */
const uint8_t LINUX = 'L';

const uint8_t LOG_WINDOWS = 'b';

/**
 * Command code which could be received by the host.
 */
typedef enum : uint8_t {
	SINGLE_USER = '1',
	ASK_FOR_PW = 'p',
	OS_SYSTEM = 'o',
	READ_UID = 'u',
	HOST_STATUS = 'l',
	USER_QUANTITY = 'q',
	END_REQUEST = '!',
} CommandCode;

/**
 * States the device can have.
 */
typedef enum : uint8_t {
	START,
	LOCKED,
	WAIT_FOR_PW,
	UNLOCKED,
	VALIDATE_HOST_LOCKED,
} State;

/**
 * \var int8_t currentUid[]
 *
 * Currently logged UID.
 */
uint8_t currentUid[] = {0,0,0,0};

/**
 * \var uint32_t cardMissing
 *
 * Counter increased when currentUid is missing.
 */
uint32_t cardMissing = 0;

/**
 * \var uint32_t ui32TxCount
 *
 * TxCounter to find out if new data was transmitted
 */
uint32_t ui32TxCount;

/**
 * \var uint8_t os
 *
 * The code of the operating system of the host.
 */
uint8_t os = 0;

/**
 * \var uint8_t singleUser
 *
 * The code of the single user.
 */
uint8_t singleUser = 0;

/**
 * \var const int SYSTICKS_PER_SECOND
 *
 * The system tick timer period.
 */
const int SYSTICKS_PER_SECOND = 100;

/**
 * \fn void configureUSB()
 *
 * Configures the USB pins.
 *
 * \return void
 */
void configureUSB();

/**
 * \fn delay(int ms);
 * \brief Delays in ms given time.
 *
 * \param ms Time to wait in ms
 *
 * \return void
 */
void delay(int ms);

/**
 * \fn uint32_t ControlHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue, void *pvMsgData)
 * \brief Handles the default USB-Events of the serial device.
 */
uint32_t ControlHandler(void *pvCBData, uint32_t ui32Event,
		uint32_t ui32MsgValue, void *pvMsgData);

/**
 * \fn uint32_t USBEventHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue, void *pvMsgData)
 * \brief Handles the default USB-Events of the keyboard device.
 */
uint32_t USBEventHandler(void *pvCBData, uint32_t ui32Event,
		uint32_t ui32MsgParam, void *pvMsgData);

/**
 * \fn uint32_t USBKeyboardHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue, void *pvMsgData)
 * \brief Handles the keyboard events of the keyboard device.
 */
uint32_t USBKeyboardHandler(void *pvCBData, uint32_t ui32Event,
		uint32_t ui32MsgParam, void *pvMsgData);

/**
 * \fn uint32_t TxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue, void *pvMsgData)
 * \brief Handles the transmit events of the serial device.
 */
uint32_t TxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,
		void *pvMsgData);

/**
 * \fn uint32_t RxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue, void *pvMsgData)
 * \brief Handles the receive events of the serial device.
 */
uint32_t RxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,
		void *pvMsgData);

/*
 * main.c
 *
 * To use Serial Communication connect to com port (COM9) with baud 115200
 * ser = serial.Serial("COM9", 115200)
 * ser.write("w")
 * print ser.read() // -> 'w'
 */
int main(void) {
	State state = START;

	USBSerialDevice::registerControlHandler(ControlHandler);
	USBSerialDevice::registerRxHandler(RxHandler);
	USBSerialDevice::registerTxHandler(TxHandler);

	USBKeyboardDevice::registerKeyboardHandler(USBKeyboardHandler);

	USBCompDevice::registerEventHandler(USBEventHandler);

	USBKeyboardDevice::getInstance();
	USBKeyboardDevice::getInstance();

	USBSerialDevice::getInstance();
	USBCompDevice::getInstance();

	delay(START_DELAY);

	configureUSB();
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

	ButtonsInit();

	IntMasterEnable();

	delay(WAIT_FOR_HOST_KONFIGURATION);
	Wire.begin();
	Wire.setModule(0);

	rfid_reader::MFRC522 reader;
	uint8_t res = reader.readRegister(rfid_reader::CommandReg);

	while (true) {
		uint8_t symbol = USBSerialDevice::getInstance()->popReceiveBuffer();
		if (symbol == READ_UID) {
			if (currentUid[0] == 0 && currentUid[1] == 0 && currentUid[2] == 0 && currentUid[3] == 0) {
				// no card present
				uint8_t command[] = {READ_UID, END_REQUEST};
				USBSerialDevice::getInstance()->write(command, sizeof(command));
			} else {
				uint8_t command[] = {READ_UID, currentUid[0], currentUid[1], currentUid[2], currentUid[3], END_REQUEST};
				USBSerialDevice::getInstance()->write(command, sizeof(command));
			}
			symbol = USBSerialDevice::getInstance()->popReceiveBuffer();
			delay(100);
		}
		if (symbol == USER_QUANTITY) {
			singleUser = USBSerialDevice::getInstance()->popReceiveBuffer();
			symbol = USBSerialDevice::getInstance()->popReceiveBuffer();
		}
		if (symbol == OS_SYSTEM && state != START) {
			os = USBSerialDevice::getInstance()->popReceiveBuffer();
			singleUser = USBSerialDevice::getInstance()->popReceiveBuffer();
			symbol = USBSerialDevice::getInstance()->popReceiveBuffer();
		}

		switch (state) {
		case START:
			if (symbol == OS_SYSTEM) {
				os = USBSerialDevice::getInstance()->popReceiveBuffer();
				singleUser = USBSerialDevice::getInstance()->popReceiveBuffer();
				state = LOCKED;
			}
			break;
		case LOCKED:
			if (reader.isNewCardPresent()) {
				if (reader.readCardSerial()) {
					for (volatile uint8_t i = 0; i < sizeof(currentUid); i++) {
						currentUid[i] = reader.uid.uidbyte[i];
					}
					uint8_t command[] = {ASK_FOR_PW, currentUid[0], currentUid[1], currentUid[2], currentUid[3], END_REQUEST};
					USBSerialDevice::getInstance()->write(command, sizeof(command));
					state = WAIT_FOR_PW;
				}
			}
			break;
		case WAIT_FOR_PW:
			if (symbol == HOST_STATUS) {
				USBSerialDevice::getInstance()->popReceiveBuffer();
				for (volatile uint8_t i = 0; i < sizeof(currentUid); i++) {
					currentUid[i] = 0;
				}
				state = LOCKED;
			} else if (symbol == ASK_FOR_PW) {
				char pw[UART_BUFFER_SIZE / 2];
				uint32_t length = 0;
				uint8_t noEnd = 1;
				for (length = 0; length < UART_BUFFER_SIZE && noEnd; length++) {
					pw[length] = USBSerialDevice::getInstance()->popReceiveBuffer();
					if (pw[length] == '\n') {
						noEnd = 0;
					}
				}

				if (os != LINUX && singleUser == SINGLE_USER) {
					USBKeyboardDevice::getInstance()->USBWriteString(&ENTER, 1);
					delay(ENTER_DELAY);
				}
				if (singleUser != SINGLE_USER) {
					char username[UART_BUFFER_SIZE / 2];
					uint32_t lengthU = 0;
					uint8_t noEnd = 1;
					for (lengthU = 0; lengthU < UART_BUFFER_SIZE && noEnd; lengthU++) {
						username[lengthU] = USBSerialDevice::getInstance()->popReceiveBuffer();
						if (username[lengthU] == '\n') {
							noEnd = 0;
						}
					}
					USBKeyboardDevice::getInstance()->USBWriteString(username, lengthU);
					if (os == LINUX) {
						USBKeyboardDevice::getInstance()->USBWriteString(&ENTER, 1);
						delay(ENTER_DELAY);
					} else {
						USBKeyboardDevice::getInstance()->USBWriteString(&KEY_TAB, 1);
					}
				}
				USBKeyboardDevice::getInstance()->USBWriteString(pw, length);
				USBKeyboardDevice::getInstance()->USBWriteString(&ENTER, 1);
				if (singleUser == SINGLE_USER) {
					delay(WAIT_LOG_IN);
				} else {
					delay(WAIT_LOG_IN_LONG);
				}
				state = UNLOCKED;
			}
			break;
		case UNLOCKED:
			if (reader.isNewCardPresent()) {
				if (reader.readCardSerial()) {
					if (currentUid[0] == reader.uid.uidbyte[0] && currentUid[1] == reader.uid.uidbyte[1] &&
							currentUid[2] == reader.uid.uidbyte[2] && currentUid[3] == reader.uid.uidbyte[3] &&
							reader.uid.uidbyte[0] + reader.uid.uidbyte[1] + reader.uid.uidbyte[2] + reader.uid.uidbyte[3] > 0) {
						cardMissing = 0;
					} else {
						cardMissing++;
					}
				}
			} else {
				if (cardMissing <= 30) {
					cardMissing++;
				}
			}
			if (cardMissing == 30) {
				for (uint8_t i = 0; i < sizeof(currentUid); i++) {
					currentUid[i] = 0;
				}
				uint8_t command[] = {HOST_STATUS, END_REQUEST};
				USBSerialDevice::getInstance()->write(command, sizeof(command));
				cardMissing = 0;
				state = VALIDATE_HOST_LOCKED;
			}
			break;
		case VALIDATE_HOST_LOCKED:
			if (symbol == HOST_STATUS) {
				if (USBSerialDevice::getInstance()->popReceiveBuffer() != '0') {
					if (os != LINUX) {
						USBKeyboardDevice::getInstance()->USBPressKeyCombination(HID_KEYB_LEFT_CTRL | HID_KEYB_LEFT_ALT, &DEL, 1);
						delay(ENTER_DELAY);
						if (singleUser == SINGLE_USER) {
							USBKeyboardDevice::getInstance()->USBWriteString(&ENTER, 1);
						} else {
							uint8_t b = USBKeyboardDevice::getInstance()->GetUsageCode(LOG_WINDOWS, false);
							USBKeyboardDevice::getInstance()->USBPressKeyCombination(HID_KEYB_LEFT_ALT, &b, 1);
						}
					} else {
						uint8_t key;
						if (singleUser == SINGLE_USER) {
							key = USBKeyboardDevice::getInstance()->GetUsageCode('l', false);
						} else {
							key = USBKeyboardDevice::getInstance()->GetUsageCode('u', false);
						}
						USBKeyboardDevice::getInstance()->USBPressKeyCombination(HID_KEYB_LEFT_CTRL | HID_KEYB_LEFT_ALT, &key, 1);
					}
				}
				state = LOCKED;
			}
			break;
		}
	}
}

void configureUSB() {
	ROM_FPULazyStackingEnable();

	ROM_SysCtlClockSet(
	SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
	uint32_t ui32SysClock = ROM_SysCtlClockGet();

	ROM_SysTickPeriodSet(ui32SysClock / SYSTICKS_PER_SECOND);
	ROM_SysTickIntEnable();
	ROM_SysTickEnable();

	// configure USB pins
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	ROM_GPIOPinTypeUSBAnalog(GPIO_PORTD_BASE, GPIO_PIN_4 | GPIO_PIN_5);

	// Enable the UART that we will be redirecting.
	ROM_SysCtlPeripheralEnable(USB_UART_PERIPH);

	// Enable and configure the UART RX and TX pins
	ROM_SysCtlPeripheralEnable(TX_GPIO_PERIPH);
	ROM_SysCtlPeripheralEnable(RX_GPIO_PERIPH);
	ROM_GPIOPinTypeUART(TX_GPIO_BASE, TX_GPIO_PIN);
	ROM_GPIOPinTypeUART(RX_GPIO_BASE, RX_GPIO_PIN);

	// Set the default UART configuration.
	ROM_UARTConfigSetExpClk(USB_UART_BASE, ROM_SysCtlClockGet(),
			DEFAULT_BIT_RATE, DEFAULT_UART_CONFIG);
	ROM_UARTFIFOLevelSet(USB_UART_BASE, UART_FIFO_TX4_8, UART_FIFO_RX4_8);

	// Configure and enable UART interrupts.
	ROM_UARTIntClear(USB_UART_BASE, ROM_UARTIntStatus(USB_UART_BASE, false));
	ROM_UARTIntEnable(USB_UART_BASE, (UART_INT_OE | UART_INT_BE | UART_INT_PE |
	UART_INT_FE | UART_INT_RT | UART_INT_TX | UART_INT_RX));

	// Initialize the transmit and receive buffers.
	USBSerialDevice::getInstance()->initBuffer();

	USBStackModeSet(0, eUSBModeForceDevice, 0);

	// USBDCDCInit(0, &g_sCDCDevice);
	// USBDHIDKeyboardInit(0, &g_sKeyboardDevice);

	USBCompDevice::getInstance()->init();

	ui32TxCount = 0;

	ROM_IntEnable(USB_UART_INT);
}

void delay(int ms) {
	uint32_t one_ms = (ROM_SysCtlClockGet() / 3) / 1000;
	SysCtlDelay(one_ms * ms);
}

uint32_t ControlHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue, void *pvMsgData) {

	//
	// Which event are we being asked to process?
	//
	switch (ui32Event) {
		case USB_EVENT_CONNECTED:
			USBSerialDevice::getInstance()->connected();
			break;
		case USB_EVENT_DISCONNECTED:
			USBSerialDevice::getInstance()->disconnected();
			break;
		case USBD_CDC_EVENT_GET_LINE_CODING:
			USBSerialDevice::getInstance()->GetLineCoding((tLineCoding *)pvMsgData);
			break;
		case USBD_CDC_EVENT_SET_LINE_CODING:
			USBSerialDevice::getInstance()->SetLineCoding((tLineCoding *)pvMsgData);
			break;
		case USBD_CDC_EVENT_SET_CONTROL_LINE_STATE:
			USBSerialDevice::getInstance()->SetControlLineState((uint16_t) ui32MsgValue);
			break;
		case USBD_CDC_EVENT_SEND_BREAK:
			USBSerialDevice::getInstance()->SendBreak(true);
			break;
		case USBD_CDC_EVENT_CLEAR_BREAK:
			USBSerialDevice::getInstance()->SendBreak(false);
			break;
		case USB_EVENT_SUSPEND:
		case USB_EVENT_RESUME:
			break;
		default:
			break;
	}
	return (0);
}

uint32_t USBEventHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgParam, void *pvMsgData) {
	// Infor the UI code of the state change.
	if (ui32Event == USB_EVENT_CONNECTED) {

	} else if (ui32Event == USB_EVENT_DISCONNECTED) {

	} else if (ui32Event == USB_EVENT_SUSPEND) {

	} else if (ui32Event == USB_EVENT_RESUME) {

	}
	return (0);
}

uint32_t USBKeyboardHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgParam, void *pvMsgData) {
	//
	// Handle LED set requests.  These are the various lock key requests.
	//
	if (ui32Event == USBD_HID_KEYB_EVENT_SET_LEDS) {
		//
		// Set the state of the lock keys in the UI.
		//
		//UICapsLock(ui32MsgParam & HID_KEYB_CAPS_LOCK);
		//UIScrollLock(ui32MsgParam & HID_KEYB_SCROLL_LOCK);
		//UINumLock(ui32MsgParam & HID_KEYB_NUM_LOCK);
	} else if (ui32Event == USB_EVENT_TX_COMPLETE) {
		//
		// Any time a report is sent and there is a pending special key
		// pressed send a key release.
		//
		if (g_sKeyboardState.ui8Special) {
			USBKeyboardDevice::getInstance()->releaseSpecialKey();
		}
		g_sKeyboardState.pending = false;
	}
	return (0);
}

uint32_t RxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue, void *pvMsgData) {
	uint32_t ui32Count;

	//
	// Which event are we being sent?
	//
	switch (ui32Event) {
		//
		// A new packet has been received.
		//
		case USB_EVENT_RX_AVAILABLE: {
				USBSerialDevice::getInstance()->readBuffer();
				ROM_UARTIntEnable(USB_UART_BASE, UART_INT_TX);
				break;
			}

			//
			// We are being asked how much unprocessed data we have still to
			// process. We return 0 if the UART is currently idle or 1 if it is
			// in the process of transmitting something. The actual number of
			// bytes in the UART FIFO is not important here, merely whether or
			// not everything previously sent to us has been transmitted.
			//
		case USB_EVENT_DATA_REMAINING: {
				//
				// Get the number of bytes in the buffer and add 1 if some data
				// still has to clear the transmitter.
				//
				ui32Count = ROM_UARTBusy(USB_UART_BASE) ? 1 : 0;
				return (ui32Count);
			}

			//
			// We are being asked to provide a buffer into which the next packet
			// can be read. We do not support this mode of receiving data so let
			// the driver know by returning 0. The CDC driver should not be sending
			// this message but this is included just for illustration and
			// completeness.
			//
		case USB_EVENT_REQUEST_BUFFER: {
				return (0);
			}
		default:
			break;
	}

	return (0);
}

uint32_t TxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue, void *pvMsgData) {
	//
	// Which event have we been sent?
	//
	switch (ui32Event) {
		case USB_EVENT_TX_COMPLETE:
			//
			// Since we are using the USBBuffer, we don't need to do anything
			// here.
			//
			break;
		default:
			break;

	}
	return (0);
}
