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
#include "usblib/device/usbdhidmouse.h"
#include "usblib/device/usbdhidkeyb.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "usb_structs.h"
#include "global_defs.h"

static int32_t g_adcPing[NUM_CHANNELS];
static int32_t g_adcPong[NUM_CHANNELS];
static volatile uint32_t g_ui32DMAErrCount = 0;
static volatile int g_skip_samples = 0;
#pragma DATA_ALIGN(g_filterBufferImag, 16)
static int16_t g_filterBufferImag[FILTER_BUFFER_SIZE] = { 0 };
#pragma DATA_ALIGN(g_filterBufferReal, 16)
static int16_t g_filterBufferReal[FILTER_BUFFER_SIZE] = { 0 };
static const int16_t *g_filter_ptr = 0;
static const int16_t *g_r_ptr = 0;
static const int16_t *g_i_ptr = 0;
static int32_t g_count = 0;
static int32_t g_sum_real = 0;
static int32_t g_sum_imag = 0;

static int16_t *g_filterImagInPtr = 0;
static int16_t *g_filterRealInPtr = 0;

static int16_t *g_adcInPtr = 0;
static int16_t *g_adcOutPtr = 0;
static int16_t g_adcBuffer[ADC_BUFFER_SIZE];

extern const int16_t g_filter[FILTER_SIZE];


static int32_t g_fftBuffer[FFT_BIN_SIZE];
static int32_t g_fftInvBuffer[FFT_BIN_SIZE];
static int32_t g_fftMultiplyBuffer[FFT_BIN_SIZE];

extern const int16_t g_fftSignalA[FFT_BIN_SIZE*2];
extern const int16_t g_fftSignalB[FFT_BIN_SIZE*2];
extern const int16_t g_fftSignalC[FFT_BIN_SIZE*2];
extern const int16_t g_fftSignalD[FFT_BIN_SIZE*2];
extern const int16_t g_test_signal[FFT_BIN_SIZE * 8];
//static const int16_t *g_test_ptr;

const uint8_t g_message[MESSAGE_LENGTH] = { 'A', 'B' };
uint8_t g_message_last_char = 0;
uint8_t g_message_index = 0;
const char g_spooky_message[] = "spooky";
int g_spooky_message_index = 0;
int g_spooky_last_tick = 0;
unsigned int g_keyTick = 0;
bool g_keyDown = false;

// uDMA control table aligned to 1024-byte boundary
#pragma DATA_ALIGN(pui8ControlTable, 1024)
uint8_t pui8ControlTable[1024];

void ConfigureUART(void);
void InitADC00(void);

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
SysTickIntHandler(void)
{
    g_ui32SysTickCount++;
}

//*****************************************************************************
//
// Handles all of the generic USB events.
//
//*****************************************************************************
uint32_t
USBEventHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgParam,
                void *pvMsgData)
{
    //
    // Infor the UI code of the state change.
    if(ui32Event == USB_EVENT_CONNECTED)
    {
        UARTprintf("Host connected.\n");
    }
    else if(ui32Event == USB_EVENT_DISCONNECTED)
    {
        UARTprintf("Host disconnected.\n");
    }
    else if(ui32Event == USB_EVENT_SUSPEND)
    {
        UARTprintf("Host suspend.\n");
    }
    else if(ui32Event == USB_EVENT_RESUME)
    {
        UARTprintf("Host resume.\n");
    }
    return(0);
}


/*
 * main.c
 */
int main(void)
{
    //C:\ti\TivaWare_C_Series-2.1.1.71\examples\boards\dk-tm4c129x\usb_dev_chid\usb_dev_chid.c

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


    // configure dma
    ROM_SysCtlPeripheralClockGating(true);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    ROM_SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UDMA);
    ROM_IntEnable(INT_UDMAERR);
    ROM_uDMAEnable();
    ROM_uDMAControlBaseSet(pui8ControlTable);

    // configure A/D converter, use AIN4 PD3
    g_filterImagInPtr = &g_filterBufferImag[0];
    g_filterRealInPtr = &g_filterBufferReal[0];
    g_adcInPtr = g_adcBuffer;
    g_adcOutPtr = g_adcBuffer;
    //g_test_ptr = g_test_signal + 256;

    InitADC00();

    ConfigureUART();

    g_message_index = 0;
    uint8_t last_led = 0;


    USBStackModeSet(0, eUSBModeForceDevice, 0);

    USBKeyboardInit();

    USBDHIDKeyboardCompositeInit(0, &g_sKeyboardDevice, &g_psCompDevices[1]);

    USBDCompositeInit(0, &g_sCompDevice, DESCRIPTOR_DATA_SIZE,
                      g_pui8DescriptorData);

    UARTprintf("Spooky keyboard online.\n");

    IntMasterEnable();

    int test_count = 0;
    while(true)
    {


        int sample_count = ((g_adcInPtr - g_adcOutPtr + ADC_BUFFER_SIZE) % ADC_BUFFER_SIZE) / 2;
        if(sample_count >= FFT_BIN_SIZE)
        {
            if(test_count > 2)
            {
                test_count = 0;
            }
            test_count++;
            int i;
            int16_t *ptr_inData;

            //
            // test the letter A
            //
            ptr_inData = (int16_t *)g_fftBuffer;
            int16_t *ptr_inModel = (int16_t *)g_fftSignalA;
            int16_t *ptr_out = (int16_t *)g_fftMultiplyBuffer;
            for(i=FFT_BIN_SIZE;i>0;i--)
            {
                int a = *ptr_inData++;
                int b = *ptr_inData++;
                int c = *ptr_inModel++;
                int d = *ptr_inModel++;
                int ac = a * c;
                int bd = b * d;

                int re = ac - bd;
                int im = (a + b) * (c + d) - ac - bd;

                *ptr_out++ = re >> MULTIPLY_DOWN_SHIFT;
                *ptr_out++ = im >> MULTIPLY_DOWN_SHIFT;
            }

            // find peak
            int16_t *ptr_inv = (int16_t *)g_fftInvBuffer;
            int32_t maxA = 0;
            int32_t maxA_index = 0;
            for(i=FFT_BIN_SIZE;i>0;i--)
            {
                int re = *ptr_inv++;
                int im = *ptr_inv++;
                int val = re * re + im * im;
                if(val > maxA)
                {
                    maxA = val;
                    maxA_index = FFT_BIN_SIZE - i;
                }
            }

            //
            // test the letter B
            //
            ptr_inData = (int16_t *)g_fftBuffer;
            ptr_inModel = (int16_t *)g_fftSignalB;
            ptr_out = (int16_t *)g_fftMultiplyBuffer;
            for(i=FFT_BIN_SIZE;i>0;i--)
            {
                int a = *ptr_inData++;
                int b = *ptr_inData++;
                int c = *ptr_inModel++;
                int d = *ptr_inModel++;
                int ac = a * c;
                int bd = b * d;

                int re = ac - bd;
                int im = (a + b) * (c + d) - ac - bd;

                *ptr_out++ = re >> MULTIPLY_DOWN_SHIFT;
                *ptr_out++ = im >> MULTIPLY_DOWN_SHIFT;
            }

            // find peak
            ptr_inv = (int16_t *)g_fftInvBuffer;
            int32_t maxB = 0;
            int32_t maxB_index = 0;
            for(i=FFT_BIN_SIZE;i>0;i--)
            {
                int re = *ptr_inv++;
                int im = *ptr_inv++;
                int val = re * re + im * im;
                if(val > maxB)
                {
                    maxB = val;
                    maxB_index = FFT_BIN_SIZE - i;
                }
            }

            //
            // test the letter C
            //
            ptr_inData = (int16_t *)g_fftBuffer;
            ptr_inModel = (int16_t *)g_fftSignalC;
            ptr_out = (int16_t *)g_fftMultiplyBuffer;
            for(i=FFT_BIN_SIZE;i>0;i--)
            {
                int a = *ptr_inData++;
                int b = *ptr_inData++;
                int c = *ptr_inModel++;
                int d = *ptr_inModel++;
                int ac = a * c;
                int bd = b * d;

                int re = ac - bd;
                int im = (a + b) * (c + d) - ac - bd;

                *ptr_out++ = re >> MULTIPLY_DOWN_SHIFT;
                *ptr_out++ = im >> MULTIPLY_DOWN_SHIFT;
            }

            // find peak
            ptr_inv = (int16_t *)g_fftInvBuffer;
            int32_t maxC = 0;
            int32_t maxC_index = 0;
            for(i=FFT_BIN_SIZE;i>0;i--)
            {
                int re = *ptr_inv++;
                int im = *ptr_inv++;
                int val = re * re + im * im;
                if(val > maxC)
                {
                    maxC = val;
                    maxC_index = FFT_BIN_SIZE - i;
                }
            }

            //
            // test the letter D
            //
            ptr_inData = (int16_t *)g_fftBuffer;
            ptr_inModel = (int16_t *)g_fftSignalD;
            ptr_out = (int16_t *)g_fftMultiplyBuffer;
            for(i=FFT_BIN_SIZE;i>0;i--)
            {
                int a = *ptr_inData++;
                int b = *ptr_inData++;
                int c = *ptr_inModel++;
                int d = *ptr_inModel++;
                int ac = a * c;
                int bd = b * d;

                int re = ac - bd;
                int im = (a + b) * (c + d) - ac - bd;

                *ptr_out++ = re >> MULTIPLY_DOWN_SHIFT;
                *ptr_out++ = im >> MULTIPLY_DOWN_SHIFT;
            }

            // find peak
            ptr_inv = (int16_t *)g_fftInvBuffer;
            int32_t maxD = 0;
            int32_t maxD_index = 0;
            for(i=FFT_BIN_SIZE;i>0;i--)
            {
                int re = *ptr_inv++;
                int im = *ptr_inv++;
                int val = re * re + im * im;
                if(val > maxD)
                {
                    maxD = val;
                    maxD_index = FFT_BIN_SIZE - i;
                }
            }

            char found_char;
            int low_maxA = maxA >> THRESHOLD_SHIFT;
            int low_maxB = maxB >> THRESHOLD_SHIFT;
            int low_maxC = maxC >> THRESHOLD_SHIFT;
            int low_maxD = maxD >> THRESHOLD_SHIFT;
            int max_index;
            if((low_maxA > maxB) && (low_maxA > maxC) && (low_maxA > maxD))
            {
                found_char = 'A';
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, last_led = (GPIO_PIN_2 | GPIO_PIN_3));
                max_index = maxA_index;
            }
            else if((low_maxB > maxA) && (low_maxB > maxC) && (low_maxB > maxD))
            {
                found_char = 'B';
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, last_led = GPIO_PIN_3);
                max_index = maxB_index;
            }
            else if((low_maxC > maxA) && (low_maxC > maxB) && (low_maxC > maxD))
            {
                found_char = 'C';
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, last_led = GPIO_PIN_2);
                max_index = maxC_index;
            }
            else if((low_maxD > maxA) && (low_maxD > maxB) && (low_maxD > maxC))
            {
                found_char = 'D';
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, last_led = GPIO_PIN_1);
                max_index = maxD_index;
            }
            else
            {
                found_char = '_';
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, last_led = 0);

                if((maxA > maxB) && (maxA > maxC) && (maxA > maxD))
                    max_index = maxA_index;
                else if((maxB > maxC) && (maxB > maxD))
                    max_index = maxB_index;
                else if((maxC > maxD))
                    max_index = maxC_index;
                else
                    max_index = maxD_index;
            }

            int skip = FFT_BIN_SIZE - max_index;
            if(skip > FFT_BIN_SIZE - 4)
                skip = 0;
            g_skip_samples = skip * 4;

            UARTprintf("%c %d %d %d %d %d\n", found_char, maxA, maxB, maxC, maxD, max_index);

            if(g_message_last_char != found_char)
            {
                if(g_message[g_message_index] == found_char)
                {
                    g_message_index++;
                    if(g_message_index >= MESSAGE_LENGTH)
                    {
                        UARTprintf("spooky!");
                        g_spooky_message_index = 0;
                    }
                }
                else
                {
                    if(g_message[0] == found_char)
                        g_message_index = 1;
                    else
                        g_message_index = 0;
                }
            }

            g_adcOutPtr += FFT_BIN_SIZE * 2;
            if(g_adcOutPtr >= &g_adcBuffer[ADC_BUFFER_SIZE])
            {
                g_adcOutPtr = g_adcBuffer;
            }
        }

        unsigned int tick = ROM_SysTickValueGet();
        if((g_spooky_message_index == 0) && (g_keyTick == 0))
        {
            g_keyTick = g_ui32SysTickCount;
        }
        if(g_keyTick != 0)
        {
            int diff = g_keyTick - g_ui32SysTickCount;
            if(diff <= 0)
            {
                if(g_keyDown)
                {
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, last_led);
                    UARTprintf("key up %c %d %d\n ", g_spooky_message[g_spooky_message_index], (int)IsKeyboardPending(), g_ui32SysTickCount);
                    USBKeyboardUpdate(0, g_spooky_message[g_spooky_message_index], false);
                    g_spooky_message_index++;
                    g_keyTick = (g_ui32SysTickCount + 0x1) | 1;
                    g_keyDown = false;
                }
                else if(g_spooky_message_index < sizeof(g_spooky_message))
                {
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_1 | GPIO_PIN_3);
                    UARTprintf("key down %c %d %d\n ", g_spooky_message[g_spooky_message_index], (int)IsKeyboardPending(), g_ui32SysTickCount);
                    USBKeyboardUpdate(0, g_spooky_message[g_spooky_message_index], true);
                    g_keyTick = (g_ui32SysTickCount + 0x2) | 1;
                    g_keyDown = true;
                }
                else
                {
                    g_keyTick = 0;
                }
            }
        }

    }
}

//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void ConfigureUART(void)
{
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


void InitADC00(void)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    ROM_GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_3); // AIN4

    ROM_ADCHardwareOversampleConfigure(ADC0_BASE, HW_AVERAGE);
    ROM_ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_ALWAYS, 0);
    ROM_ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH4);
    ROM_ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ADC_CTL_CH4);
    ROM_ADCSequenceStepConfigure(ADC0_BASE, 0, 2, ADC_CTL_CH4);
    ROM_ADCSequenceStepConfigure(ADC0_BASE, 0, 3, ADC_CTL_CH4);
    ROM_ADCSequenceStepConfigure(ADC0_BASE, 0, 4, ADC_CTL_CH4);
    ROM_ADCSequenceStepConfigure(ADC0_BASE, 0, 5, ADC_CTL_CH4);
    ROM_ADCSequenceStepConfigure(ADC0_BASE, 0, 6, ADC_CTL_CH4);
    ROM_ADCSequenceStepConfigure(ADC0_BASE, 0, 7, ADC_CTL_CH4|ADC_CTL_IE|ADC_CTL_END);
    ADCSequenceDMAEnable(ADC0_BASE, 0);

    // adc 0
    ROM_uDMAChannelAttributeDisable(UDMA_CHANNEL_ADC0,
                                    UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
                                    UDMA_ATTR_HIGH_PRIORITY |
                                    UDMA_ATTR_REQMASK);

    ROM_uDMAChannelControlSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT,
                            UDMA_SIZE_32 | UDMA_SRC_INC_NONE | UDMA_DST_INC_32 |
                              UDMA_ARB_8);

    ROM_uDMAChannelControlSet(UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT,
                            UDMA_SIZE_32 | UDMA_SRC_INC_NONE | UDMA_DST_INC_32 |
                              UDMA_ARB_8);

    ROM_uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT,
                               UDMA_MODE_PINGPONG,
                               (void *) (ADC0_BASE + ADC_O_SSFIFO0),
                               (void *)&g_adcPing[0], NUM_CHANNELS);

    ROM_uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT,
                               UDMA_MODE_PINGPONG,
                               (void *) (ADC0_BASE + ADC_O_SSFIFO0),
                               (void *)&g_adcPong[0], NUM_CHANNELS);
    // both
    ROM_uDMAChannelEnable(UDMA_CHANNEL_ADC0);

    ROM_ADCIntClear(ADC0_BASE, 0);
    ROM_ADCIntEnable(ADC0_BASE, 0);
    ROM_IntEnable(INT_ADC0SS0);

    ADCSequenceEnable(ADC0_BASE, 0);
}


void uDMAErrorHandler(void)
{
    uint32_t ui32Status;
    ui32Status = ROM_uDMAErrorStatusGet();
    if(ui32Status)
    {
        ROM_uDMAErrorStatusClear();
        g_ui32DMAErrCount++;
    }
}

// ADC interrupt handler. Called on completion of uDMA transfer
// reduce sample rate to 1000000/(64*4) = 3906.25
void ADC00IntHandler(void)
{
    uint32_t ui32ModeP;
    uint32_t ui32ModeA;
    int sum, i;
    int32_t *ptr;


    ROM_ADCIntClear(ADC0_BASE, 0);

    ui32ModeP = ROM_uDMAChannelModeGet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT);
    if(ui32ModeP == UDMA_MODE_STOP)
    {
        if(g_skip_samples > 0)
        {
            ROM_uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT,
                                       UDMA_MODE_PINGPONG,
                                       (void *) (ADC0_BASE + ADC_O_SSFIFO0),
                                       (void *)&g_adcPing[0], NUM_CHANNELS);
            ROM_uDMAChannelEnable(UDMA_CHANNEL_ADC0);
            g_skip_samples--;
        }
        else
        {
            int stage = (((uint32_t)g_filterRealInPtr) & 0x7) >> 2;
            if(stage == 0)
            {
                // initialise buffers and setup
                g_filter_ptr = g_filter;
                g_r_ptr = g_filterRealInPtr - 1;
                g_i_ptr = g_filterImagInPtr - 1;
                g_sum_real = 0;
                g_sum_imag = 0;
                g_count = (g_filterRealInPtr - &g_filterBufferReal[0]);
            }

            ptr = g_adcPing;

            sum = *ptr++;
            sum += *ptr++;
            sum += *ptr++;
            sum += *ptr++;
            sum -= 0x800 * 4;
            //sum = *ptr - 0x800;
            //ptr += 4;
            //sum = *g_test_ptr++;
            *g_filterImagInPtr++ = sum;

            sum = *ptr++;
            sum += *ptr++;
            sum += *ptr++;
            sum += *ptr++;
            sum -= 0x800 * 4;
            //sum = *ptr - 0x800;
            //sum = *g_test_ptr++;
            *g_filterRealInPtr++ = sum;

            ROM_uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT,
                                       UDMA_MODE_PINGPONG,
                                       (void *) (ADC0_BASE + ADC_O_SSFIFO0),
                                       (void *)&g_adcPing[0], NUM_CHANNELS);
            ROM_uDMAChannelEnable(UDMA_CHANNEL_ADC0);

            if(g_filterRealInPtr >= &g_filterBufferReal[FILTER_BUFFER_SIZE])
            {
                g_filterRealInPtr = &g_filterBufferReal[0];
                g_filterImagInPtr = &g_filterBufferImag[0];
            }

            if(g_count >= 0)
            {
                if(g_count >= FILTER_STEP_SIZE/2)
                {
                    for(i=FILTER_STEP_SIZE/2;i>0;i--)
                    {
                        g_sum_real += (*g_filter_ptr++ * *g_r_ptr--)>>1;
                        g_sum_imag += (*g_filter_ptr++ * *g_i_ptr--)>>1;
                    }
                    g_count -= FILTER_STEP_SIZE/2;
                }
                else
                {
                    for(i=g_count;i>0;i--)
                    {
                        g_sum_real += (*g_filter_ptr++ * *g_r_ptr--)>>1;
                        g_sum_imag += (*g_filter_ptr++ * *g_i_ptr--)>>1;
                    }
                    g_r_ptr = &g_filterBufferReal[FILTER_BUFFER_SIZE-1];
                    g_i_ptr = &g_filterBufferImag[FILTER_BUFFER_SIZE-1];
                    for(i=(FILTER_STEP_SIZE/2)-g_count;i>0;i--)
                    {
                        g_sum_real += (*g_filter_ptr++ * *g_r_ptr--)>>1;
                        g_sum_imag += (*g_filter_ptr++ * *g_i_ptr--)>>1;
                    }
                    g_count -= FILTER_STEP_SIZE/2;
                }
            }
            else
            {
                for(i=FILTER_STEP_SIZE/2;i>0;i--)
                {
                    g_sum_real += (*g_filter_ptr++ * *g_r_ptr--)>>1;
                    g_sum_imag += (*g_filter_ptr++ * *g_i_ptr--)>>1;
                }
            }
            if(stage == 1)
            {
                *g_adcInPtr++ = g_sum_real >> 16;
                *g_adcInPtr++ = g_sum_imag >> 16;
                if(g_adcInPtr >= &g_adcBuffer[ADC_BUFFER_SIZE])
                {
                    g_adcInPtr = g_adcBuffer;
                }
            }

            //if(g_test_ptr >= &g_test_signal[FFT_BIN_SIZE * 8])
            //{
            //  g_test_ptr = g_test_signal;
            //}
        }
    }
    else
    {
        ui32ModeA = ROM_uDMAChannelModeGet(UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT);
        if(ui32ModeA == UDMA_MODE_STOP)
        {
            if(g_skip_samples > 0)
            {
                ROM_uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT,
                                           UDMA_MODE_PINGPONG,
                                           (void *) (ADC0_BASE + ADC_O_SSFIFO0),
                                           (void *)&g_adcPong[0], NUM_CHANNELS);
                ROM_uDMAChannelEnable(UDMA_CHANNEL_ADC0);
                g_skip_samples--;
            }
            else
            {
                ptr = g_adcPong;

                sum = *ptr++;
                sum += *ptr++;
                sum += *ptr++;
                sum += *ptr++;
                sum -= 0x800 * 4;
                //sum = *ptr - 0x800;
                //ptr += 4;
                //sum = *g_test_ptr++;
                *g_filterImagInPtr++ = -sum;

                sum = *ptr++;
                sum += *ptr++;
                sum += *ptr++;
                sum += *ptr++;
                sum -= 0x800 * 4;
                //sum = *ptr - 0x800;
                //sum = *g_test_ptr++;
                *g_filterRealInPtr++ = -sum;

                ROM_uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT,
                                           UDMA_MODE_PINGPONG,
                                           (void *) (ADC0_BASE + ADC_O_SSFIFO0),
                                           (void *)&g_adcPong[0], NUM_CHANNELS);
                ROM_uDMAChannelEnable(UDMA_CHANNEL_ADC0);

                if(g_filterRealInPtr >= &g_filterBufferReal[FILTER_BUFFER_SIZE])
                {
                    g_filterRealInPtr = &g_filterBufferReal[0];
                    g_filterImagInPtr = &g_filterBufferImag[0];
                }

                //if(g_test_ptr >= &g_test_signal[FFT_BIN_SIZE * 8])
                //{
                //  g_test_ptr = g_test_signal;
                //}
            }
        }
    }
}

