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
#include "usb/global_defs.h"
#include "usb/USBSerialDevice.h"
#include "usb/USBKeyboardDevice.h"
#include "usb/USBCompDevice.h"
#include "rfid/RFIDReader.h"
#include "energia/SPI.h"
#include "energia/Wire.h"
#include "rfid/MFRC522.h"
#include "rfid/MFRC522Constants.h"
#include "energia/Stream.h"

#define TARGET_IS_BLIZZARD_RB1

using namespace usbdevice;

const int START_DELAY = 1000;
const int WAIT_FOR_HOST_KONFIGURATION = 500;
const int ENTER_DELAY = 350;

const char ENTER = UNICODE_RETURN;
const char password[] = "Frotheim257";

const uint8_t ASK_FOR_PW = '1';
const uint8_t OS_SYSTEM = '3';
const uint8_t HOST_STATUS = '4';
const uint8_t END_REQUEST = '!';
const uint8_t WIN = 'w';
const uint8_t LINUX = 'l';

typedef enum : uint8_t {
	START,
	LOCKED,
	WAIT_FOR_PW,
	UNLOCKED,
	VALIDATE_HOST_LOCKED,
} State;

uint8_t currentUid[] = {0,0,0,0};
uint32_t cardMissing = 20;

uint32_t ui32TxCount;
uint32_t ui32RxCount;

uint8_t os = 0;

/**
 * The system tick timer period.
 */
const int SYSTICKS_PER_SECOND = 100;

/**
 * Configure the USB pins
 */
void configureUSB();

void setUpI2C();

void delay(int ms);

uint32_t ControlHandler(void *pvCBData, uint32_t ui32Event,
		uint32_t ui32MsgValue, void *pvMsgData);

uint32_t USBEventHandler(void *pvCBData, uint32_t ui32Event,
		uint32_t ui32MsgParam, void *pvMsgData);

uint32_t USBKeyboardHandler(void *pvCBData, uint32_t ui32Event,
		uint32_t ui32MsgParam, void *pvMsgData);

uint32_t TxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,
		void *pvMsgData);

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
	/*rfid_reader::RFIDReader reader;
	reader.wakeUpCall();*/

	uint8_t ui8ButtonsChanged, ui8Buttons;
	while (true) {
		switch (state) {
		case START:
			if (ui32RxCount != USBSerialDevice::getInstance()->getRxEventCount() && USBSerialDevice::getInstance()->getReceiveBuffer()[0] == OS_SYSTEM) {
				os = USBSerialDevice::getInstance()->getReceiveBuffer()[1];
				// Ask for state
				uint8_t command[] = {HOST_STATUS, END_REQUEST};
				USBSerialDevice::getInstance()->write(command, sizeof(command));
				state = LOCKED;
			}
			break;
		case LOCKED:
			// TODO: art entprellen dazu?
			if (reader.isNewCardPresent()) {
				if (reader.readCardSerial()) {
					for (uint8_t i = 0; i < sizeof(currentUid); i++) {
						currentUid[i] = reader.uid.uiduint8_t[i];
					}
					uint8_t command[] = {ASK_FOR_PW, END_REQUEST};
					USBSerialDevice::getInstance()->write(command, sizeof(command));
					state = WAIT_FOR_PW;
				}
			}
			break;
		case WAIT_FOR_PW:
			if (ui32RxCount != USBSerialDevice::getInstance()->getRxEventCount()) {
				if (USBSerialDevice::getInstance()->getReceiveBuffer()[0] == HOST_STATUS) {
					for (uint8_t i = 0; i < sizeof(currentUid); i++) {
						currentUid[i] = 0;
					}
					state = LOCKED;
				} else if (USBSerialDevice::getInstance()->getReceiveBuffer()[0] == ASK_FOR_PW) {
					char pw[USBSerialDevice::getInstance()->getReceiveBufferEnd() - 1];
					for (int i = 1; i < USBSerialDevice::getInstance()->getReceiveBufferEnd(); i++) {
						pw[i] = USBSerialDevice::getInstance()->getReceiveBuffer()[1];
					}
					if (os != LINUX) {
						USBKeyboardDevice::getInstance()->USBWriteString(&ENTER, 1);
						delay(ENTER_DELAY);
					}
					USBKeyboardDevice::getInstance()->USBWriteString(pw, sizeof(pw));
					USBKeyboardDevice::getInstance()->USBWriteString(&ENTER, 1);
					state = UNLOCKED;
				}
			}
		case UNLOCKED:
			if (reader.isNewCardPresent()) {
				if (reader.readCardSerial()) {
					if (currentUid[0] == reader.uid.uiduint8_t[0] &&
							currentUid[1] == reader.uid.uiduint8_t[1] &&
							currentUid[2] == reader.uid.uiduint8_t[2] &&
							currentUid[3] == reader.uid.uiduint8_t[3]) {
						cardMissing = 0;
					} else {
						cardMissing++;
					}
				} else {
					if (cardMissing <= 50) {
						cardMissing++;
					}
				}
				if (cardMissing == 50) {
					for (uint8_t i = 0; i < sizeof(currentUid); i++) {
						currentUid[i] = 0;
					}
					uint8_t command[] = {HOST_STATUS, END_REQUEST};
					USBSerialDevice::getInstance()->write(command, sizeof(command));
					state = VALIDATE_HOST_LOCKED;
				}
			}
			break;
		case VALIDATE_HOST_LOCKED:
			if (ui32RxCount != USBSerialDevice::getInstance()->getRxEventCount() && USBSerialDevice::getInstance()->getReceiveBuffer()[0] == HOST_STATUS) {
				if (USBSerialDevice::getInstance()->getReceiveBuffer()[1] == '1') {
					USBKeyboardDevice::getInstance()->USBPressKeyCombination(HID_KEYB_LEFT_CTRL | HID_KEYB_LEFT_ALT, &DEL, 1);
					delay(ENTER_DELAY);
					USBKeyboardDevice::getInstance()->USBWriteString(&ENTER, 1);
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

	ui32RxCount = 0;
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
