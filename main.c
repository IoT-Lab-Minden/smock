/*
Copyright(c) 2015 Graham Chow

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
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
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdcomp.h"
#include "usblib/device/usbdhid.h"
#include "usblib/device/usbdhidkeyb.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "drivers/buttons.h"
#include "usb_structs.h"
#include "global_defs.h"

const char ENTER = UNICODE_RETURN;
const char password[] = "**********";

int g_spooky_message_index = 0;
int g_spooky_last_tick = 0;
unsigned int g_keyTick = 0;
bool g_keyDown = false;

void USBWriteString(const char *string, int length);
void ConfigureUART(void);

//*****************************************************************************
//
// The system tick timer period.
//
//*****************************************************************************
#define SYSTICKS_PER_SECOND 100

//*****************************************************************************
//
// The current system tick count.
//
//*****************************************************************************
volatile uint32_t g_ui32SysTickCount;

//*****************************************************************************
//
// This is the interrupt handler for the SysTick interrupt.
//
//*****************************************************************************
void
SysTickIntHandler(void) {
    g_ui32SysTickCount++;
}

//*****************************************************************************
//
// Handles all of the generic USB events.
//
//*****************************************************************************
uint32_t
USBEventHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgParam,
                void *pvMsgData) {
    //
    // Infor the UI code of the state change.
    if(ui32Event == USB_EVENT_CONNECTED) {
        UARTprintf("Host connected.\n");
    }
    else if(ui32Event == USB_EVENT_DISCONNECTED) {
        UARTprintf("Host disconnected.\n");
    }
    else if(ui32Event == USB_EVENT_SUSPEND) {
        UARTprintf("Host suspend.\n");
    }
    else if(ui32Event == USB_EVENT_RESUME) {
        UARTprintf("Host resume.\n");
    }
    return(0);
}

/*
 * main.c
 */
int main(void) {
    volatile uint32_t ui32Loop;
    volatile uint32_t ui32Loop1;

    ROM_FPULazyStackingEnable();

    ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
    uint32_t ui32SysClock = ROM_SysCtlClockGet();

    ROM_SysTickPeriodSet(ui32SysClock / SYSTICKS_PER_SECOND);
    ROM_SysTickIntEnable();
    ROM_SysTickEnable();

    // configure USB pins
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    ROM_GPIOPinTypeUSBAnalog(GPIO_PORTD_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

    ConfigureUART();

    USBStackModeSet(0, eUSBModeForceDevice, 0);

    if (!USBDHIDKeyboardInit(0, &g_sKeyboardDevice)) {
        return 0;
    }

    ButtonsInit();

    UARTprintf("Spooky keyboard online.\n");

    IntMasterEnable();

    // wait for the host to set up the usb device(maybe longer?)
    for(ui32Loop = 0; ui32Loop < 20; ui32Loop++) {
        for(ui32Loop1 = 0; ui32Loop1 < 200000; ui32Loop1++) {
        }
    }

    int once = 1;
    uint8_t ui8ButtonsChanged, ui8Buttons;
    uint8_t locked = 0;
    char l = 'l';
    char counter = 0x00;
    while(true) {
        //
        // See if the buttons updated.
        //
        ButtonsPoll(&ui8ButtonsChanged, &ui8Buttons);

        // TODO: Wartezeit sollte über Variable einstellbar sein
        // TODO: Handle caps
        if (ui8ButtonsChanged && (ui8Buttons & RIGHT_BUTTON)) {
            USBWriteString(&ENTER, 1);
            for(ui32Loop = 0; ui32Loop < 20; ui32Loop++) {
                for(ui32Loop1 = 0; ui32Loop1 < 200000; ui32Loop1++) {
                }
            }
            USBWriteString(password, sizeof(password) - 1);
            USBWriteString(&ENTER, 1);
        }
    }
}

//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void ConfigureUART(void) {
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}


void USBWriteString(const char *string, int length) {
    uint8_t running = 1;
    uint8_t last_led = 0;
    int index = 0;

    while (running) {
        unsigned int tick = ROM_SysTickValueGet();
        if((index == 0) && (g_keyTick == 0)) {
            g_keyTick = g_ui32SysTickCount;
        }
        if(g_keyTick != 0) {
            int diff = g_keyTick - g_ui32SysTickCount;
            if(diff <= 0) {
                if(g_keyDown) {
                    //release button
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, last_led);
                    UARTprintf("key up %c %d %d\n ", string[index], (int)IsKeyboardPending(), g_ui32SysTickCount);
                    USBKeyboardUpdate(0, string[index], false);
                    index++;
                    g_keyTick = (g_ui32SysTickCount + 0x1) | 1;
                    g_keyDown = false;
                }
                else if(index < length) {
                    // press button
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_1 | GPIO_PIN_3);
                    UARTprintf("key down %c %d %d\n ", string[index], (int)IsKeyboardPending(), g_ui32SysTickCount);
                    USBKeyboardUpdate(0, string[index], true);
                    g_keyTick = (g_ui32SysTickCount + 0x2) | 1;
                    g_keyDown = true;
                }
                else {
                    g_keyTick = 0;
                }
            }
        }
        if (index >= length) {
            UARTprintf("End of the World");
            running = 0;
        }
    }
}
