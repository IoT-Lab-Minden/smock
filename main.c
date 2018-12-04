#include <stdint.h>
#include <string.h>
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
#include "usblib/device/usbdhid.h"
#include "usblib/device/usbdhidkeyb.h"
#include "usblib/device/usbdcomp.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "drivers/buttons.h"
#include "usb_structs.h"
#include "global_defs.h"
#include "usb_keyboard.h"
#include "usb_serial.h"

const int START_DELAY = 1000;
const int WAIT_FOR_HOST_KONFIGURATION = 500;
const int ENTER_DELAY = 280;

const char ENTER = UNICODE_RETURN;
const char password[] = "PASSWORD";

uint32_t ui32TxCount;
uint32_t ui32RxCount;
extern uint32_t g_ui32Flags;

/**
 * The system tick timer period.
 */
const int SYSTICKS_PER_SECOND = 100;

/**
 * The current system tick count.
 */
volatile uint32_t g_ui32SysTickCount;

/**
 * Configure the USB pins
 */
void configureUSB();

void delay(int ms);

/**
 * This is the interrupt handler for the SysTick interrupt.
 */
void SysTickIntHandler(void)
{
    g_ui32SysTickCount++;
}

/*
 * main.c
 *
 * To use Serial Communication connect to com port (COM9) with baud 115200
 * ser = serial.Serial("COM9", 115200)
 * ser.write("w")
 * print ser.read() // -> 'w'
 */
int main(void)
{
    volatile uint32_t ui32Loop;
    volatile uint32_t ui32Loop1;

    delay(START_DELAY);

    configureUSB();

    ButtonsInit();

    IntMasterEnable();

    delay(WAIT_FOR_HOST_KONFIGURATION);

    uint8_t ui8ButtonsChanged, ui8Buttons;
    while (true)
    {
        //
        // See if the buttons updated.
        //
        ButtonsPoll(&ui8ButtonsChanged, &ui8Buttons);

        // TODO: Wartezeit sollte über Variable einstellbar sein
        // TODO: Handle caps
        if (ui8ButtonsChanged && (ui8Buttons & RIGHT_BUTTON))
        {
            USBWriteString(&ENTER, 1);
            delay(ENTER_DELAY);
            USBWriteString(password, sizeof(password) - 1);
            USBWriteString(&ENTER, 1);
        }

        if (ui8ButtonsChanged && (ui8Buttons & LEFT_BUTTON))
        {
            USBPressKeyCombination(HID_KEYB_LEFT_CTRL | HID_KEYB_LEFT_ALT, &DEL, 1);
            delay(ENTER_DELAY);
            USBWriteString(&ENTER, 1);

        }


        if (g_ui32Flags & COMMAND_STATUS_UPDATE)
        {
            //
            // Clear the command flag
            //
            ROM_IntMasterDisable();
            g_ui32Flags &= ~COMMAND_STATUS_UPDATE;
            ROM_IntMasterEnable();
        }
        if (ui32RxCount != g_ui32UARTRxCount)
        {
            //
            // Turn on the Blue LED.
            //
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);

            //
            // Delay for a bit.
            //
            SysCtlDelay(ROM_SysCtlClockGet() / 3 / 20);

            //
            // Turn off the Blue LED.
            //
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);

            //
            // Write back received bytes
            //
            USBBufferWrite((tUSBBuffer *) &g_sTxBuffer,
                           (uint8_t *) &ui8ReceiveBuffer, ui32ReceiveBufferEnd);
            //
            // Take a snapshot of the latest receive count.
            //
            ui32RxCount = g_ui32UARTRxCount;

        }
    }
}

void configureUSB()
{
    ROM_FPULazyStackingEnable();

    ROM_SysCtlClockSet(
            SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN
                    | SYSCTL_XTAL_16MHZ);
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
    DEFAULT_BIT_RATE,
                            DEFAULT_UART_CONFIG);
    ROM_UARTFIFOLevelSet(USB_UART_BASE, UART_FIFO_TX4_8, UART_FIFO_RX4_8);

    // Configure and enable UART interrupts.
    ROM_UARTIntClear(USB_UART_BASE, ROM_UARTIntStatus(USB_UART_BASE, false));
    ROM_UARTIntEnable(USB_UART_BASE, (UART_INT_OE | UART_INT_BE | UART_INT_PE |
    UART_INT_FE | UART_INT_RT | UART_INT_TX | UART_INT_RX));

    // Initialize the transmit and receive buffers.
    USBBufferInit(&g_sTxBuffer);
    USBBufferInit(&g_sRxBuffer);

    USBStackModeSet(0, eUSBModeForceDevice, 0);

    // USBDCDCInit(0, &g_sCDCDevice);
    // USBDHIDKeyboardInit(0, &g_sKeyboardDevice);

    USBDHIDKeyboardCompositeInit(0, &g_sKeyboardDevice, &g_psCompDevices[0]);
    USBDCDCCompositeInit(0, &g_sCDCDevice, &g_psCompDevices[1]);

    USBDCompositeInit(0, &g_sCompDevice, DESCRIPTOR_DATA_SIZE,
                      g_pui8DescriptorData);

    ui32RxCount = 0;
    ui32TxCount = 0;

    ROM_IntEnable(USB_UART_INT);
}

void delay(int ms) {
    uint32_t one_ms = (ROM_SysCtlClockGet() / 3) / 1000;
    SysCtlDelay(one_ms * ms);
}

