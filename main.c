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
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "drivers/buttons.h"
#include "usb_structs.h"
#include "global_defs.h"
#include "usb_keyboard.h"

const char ENTER = UNICODE_RETURN;
const char password[] = "PASSWORD";

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

/**
 * This is the interrupt handler for the SysTick interrupt.
 */
void SysTickIntHandler(void) {
    g_ui32SysTickCount++;
}

/*
 * main.c
 */
int main(void) {
    volatile uint32_t ui32Loop;
    volatile uint32_t ui32Loop1;

    configureUSB();

    ButtonsInit();

    IntMasterEnable();

    // wait for the host to set up the usb device(maybe longer?)
    for(ui32Loop = 0; ui32Loop < 20; ui32Loop++) {
        for(ui32Loop1 = 0; ui32Loop1 < 200000; ui32Loop1++) {
        }
    }

    uint8_t ui8ButtonsChanged, ui8Buttons;
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

void configureUSB() {
    ROM_FPULazyStackingEnable();

    ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
    uint32_t ui32SysClock = ROM_SysCtlClockGet();

    ROM_SysTickPeriodSet(ui32SysClock / SYSTICKS_PER_SECOND);
    ROM_SysTickIntEnable();
    ROM_SysTickEnable();

    // configure USB pins
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    ROM_GPIOPinTypeUSBAnalog(GPIO_PORTD_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    USBStackModeSet(0, eUSBModeForceDevice, 0);

    USBDHIDKeyboardInit(0, &g_sKeyboardDevice);
}

