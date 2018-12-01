/*
 * usb_serial.c
 *
 *  Created on: 01.12.2018
 *      Author: malte
 */
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_uart.h"
#include "driverlib/rom.h"
#include "driverlib/usb.h"
#include "usblib/usblib.h"
#include "usblib/usbcdc.h"
#include "usblib/usb-ids.h"
#include "usblib/usbhid.h"
#include "driverlib/uart.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdcdc.h"
#include "usblib/device/usbdhid.h"
#include "usblib/device/usbdhidkeyb.h"
#include "usb_serial.h"
#include "usb_structs.h"

uint32_t g_ui32UARTTxCount = 0;
uint32_t g_ui32UARTRxCount = 0;
uint8_t ui8ReceiveBuffer[UART_BUFFER_SIZE];
uint32_t ui32ReceiveBufferEnd = 0;

bool g_bUSBConfigured = false;
bool g_bSendingBreak = false;

uint32_t g_ui32Flags = 0;
char *g_pcStatus;

uint32_t RxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,
                   void *pvMsgData)
{
    uint32_t ui32Count;

    //
    // Which event are we being sent?
    //
    switch (ui32Event)
    {
    //
    // A new packet has been received.
    //
    case USB_EVENT_RX_AVAILABLE:
    {
        ui32ReceiveBufferEnd = USBBufferRead((tUSBBuffer *) &g_sRxBuffer,
                                             ui8ReceiveBuffer,
                                             UART_BUFFER_SIZE);
        g_ui32UARTRxCount += ui32ReceiveBufferEnd;
        //
        // Feed some characters into the UART TX FIFO and enable the
        // interrupt so we are told when there is more space.
        //
        USBUARTPrimeTransmit(USB_UART_BASE);
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
    case USB_EVENT_DATA_REMAINING:
    {
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
    case USB_EVENT_REQUEST_BUFFER:
    {
        return (0);
    }
    default:
        break;
    }

    return (0);
}

uint32_t TxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,
                   void *pvMsgData)
{
    //
    // Which event have we been sent?
    //
    switch (ui32Event)
    {
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

uint32_t ControlHandler(void *pvCBData, uint32_t ui32Event,
                        uint32_t ui32MsgValue, void *pvMsgData)
{
    uint32_t ui32IntsOff;

    //
    // Which event are we being asked to process?
    //
    switch (ui32Event)
    {
    //
    // We are connected to a host and communication is now possible.
    //
    case USB_EVENT_CONNECTED:
        g_bUSBConfigured = true;

        //
        // Flush our buffers.
        //
        USBBufferFlush(&g_sTxBuffer);
        USBBufferFlush(&g_sRxBuffer);

        //
        // Tell the main loop to update the display.
        //
        ui32IntsOff = ROM_IntMasterDisable();
        g_pcStatus = "Connected";
        g_ui32Flags |= COMMAND_STATUS_UPDATE;
        if (!ui32IntsOff)
        {
            ROM_IntMasterEnable();
        }
        break;

        //
        // The host has disconnected.
        //
    case USB_EVENT_DISCONNECTED:
        g_bUSBConfigured = false;
        ui32IntsOff = ROM_IntMasterDisable();
        g_pcStatus = "Disconnected";
        g_ui32Flags |= COMMAND_STATUS_UPDATE;
        if (!ui32IntsOff)
        {
            ROM_IntMasterEnable();
        }
        break;

        //
        // Return the current serial communication parameters.
        //
    case USBD_CDC_EVENT_GET_LINE_CODING:
        GetLineCoding(pvMsgData);
        break;

        //
        // Set the current serial communication parameters.
        //
    case USBD_CDC_EVENT_SET_LINE_CODING:
        SetLineCoding(pvMsgData);
        break;

        //
        // Set the current serial communication parameters.
        //
    case USBD_CDC_EVENT_SET_CONTROL_LINE_STATE:
        SetControlLineState((uint16_t) ui32MsgValue);
        break;

        //
        // Send a break condition on the serial line.
        //
    case USBD_CDC_EVENT_SEND_BREAK:
        SendBreak(true);
        break;

        //
        // Clear the break condition on the serial line.
        //
    case USBD_CDC_EVENT_CLEAR_BREAK:
        SendBreak(false);
        break;

        //
        // Ignore SUSPEND and RESUME for now.
        //
    case USB_EVENT_SUSPEND:
    case USB_EVENT_RESUME:
        break;
    default:
        break;

    }

    return (0);
}

void USBUARTPrimeTransmit(uint32_t ui32Base)
{
    uint32_t ui32Read;
    uint8_t ui8Char;

    //
    // If we are currently sending a break condition, don't receive any
    // more data. We will resume transmission once the break is turned off.
    //
    if (g_bSendingBreak)
    {
        return;
    }

    //
    // If there is space in the UART FIFO, try to read some characters
    // from the receive buffer to fill it again.
    //
    while (ROM_UARTSpaceAvail(ui32Base))
    {
        //
        // Get a character from the buffer.
        //
        ui32Read = USBBufferRead((tUSBBuffer *) &g_sRxBuffer, &ui8Char, 1);
        //
        // Did we get a character?
        //
        if (ui32Read)
        {
            //
            // Place the character in the UART transmit FIFO.
            //
            ROM_UARTCharPutNonBlocking(ui32Base, ui8Char);

            //
            // Update our count of bytes transmitted via the UART.
            //
            g_ui32UARTTxCount++;
        }
        else
        {
            //
            // We ran out of characters so exit the function.
            //
            return;
        }
    }
}

void CheckForSerialStateChange(const tUSBDCDCDevice *psDevice,
                               int32_t i32Errors)
{
    uint16_t ui16SerialState;

    //
    // Clear our USB serial state.  Since we are faking the handshakes, always
    // set the TXCARRIER (DSR) and RXCARRIER (DCD) bits.
    //
    ui16SerialState = USB_CDC_SERIAL_STATE_TXCARRIER |
    USB_CDC_SERIAL_STATE_RXCARRIER;

    //
    // Are any error bits set?
    //
    if (i32Errors)
    {
        //
        // At least one error is being notified so translate from our hardware
        // error bits into the correct state markers for the USB notification.
        //
        if (i32Errors & UART_DR_OE)
        {
            ui16SerialState |= USB_CDC_SERIAL_STATE_OVERRUN;
        }

        if (i32Errors & UART_DR_PE)
        {
            ui16SerialState |= USB_CDC_SERIAL_STATE_PARITY;
        }

        if (i32Errors & UART_DR_FE)
        {
            ui16SerialState |= USB_CDC_SERIAL_STATE_FRAMING;
        }

        if (i32Errors & UART_DR_BE)
        {
            ui16SerialState |= USB_CDC_SERIAL_STATE_BREAK;
        }

        // Call the CDC driver to notify the state change.
        USBDCDCSerialStateChange((void *) psDevice, ui16SerialState);
    }
}

void SetControlLineState(uint16_t ui16State)
{
    // TODO: If configured with GPIOs controlling the handshake lines,
    // set them appropriately depending upon the flags passed in the wValue
    // field of the request structure passed.
}

bool SetLineCoding(tLineCoding *psLineCoding)
{
    uint32_t ui32Config;
    bool bRetcode;

    //
    // Assume everything is OK until we detect any problem.
    //
    bRetcode = true;

    //
    // Word length.  For invalid values, the default is to set 8 bits per
    // character and return an error.
    //
    switch (psLineCoding->ui8Databits)
    {
    case 5:
    {
        ui32Config = UART_CONFIG_WLEN_5;
        break;
    }

    case 6:
    {
        ui32Config = UART_CONFIG_WLEN_6;
        break;
    }

    case 7:
    {
        ui32Config = UART_CONFIG_WLEN_7;
        break;
    }

    case 8:
    {
        ui32Config = UART_CONFIG_WLEN_8;
        break;
    }

    default:
    {
        ui32Config = UART_CONFIG_WLEN_8;
        bRetcode = false;
        break;
    }
    }

    //
    // Parity.  For any invalid values, we set no parity and return an error.
    //
    switch (psLineCoding->ui8Parity)
    {
    case USB_CDC_PARITY_NONE:
    {
        ui32Config |= UART_CONFIG_PAR_NONE;
        break;
    }

    case USB_CDC_PARITY_ODD:
    {
        ui32Config |= UART_CONFIG_PAR_ODD;
        break;
    }

    case USB_CDC_PARITY_EVEN:
    {
        ui32Config |= UART_CONFIG_PAR_EVEN;
        break;
    }

    case USB_CDC_PARITY_MARK:
    {
        ui32Config |= UART_CONFIG_PAR_ONE;
        break;
    }

    case USB_CDC_PARITY_SPACE:
    {
        ui32Config |= UART_CONFIG_PAR_ZERO;
        break;
    }

    default:
    {
        ui32Config |= UART_CONFIG_PAR_NONE;
        bRetcode = false;
        break;
    }
    }

    //
    // Stop bits.  Our hardware only supports 1 or 2 stop bits whereas CDC
    // allows the host to select 1.5 stop bits.  If passed 1.5 (or any other
    // invalid or unsupported value of ui8Stop, we set up for 1 stop bit but
    // return an error in case the caller needs to Stall or otherwise report
    // this back to the host.
    //
    switch (psLineCoding->ui8Stop)
    {
    //
    // One stop bit requested.
    //
    case USB_CDC_STOP_BITS_1:
    {
        ui32Config |= UART_CONFIG_STOP_ONE;
        break;
    }

        //
        // Two stop bits requested.
        //
    case USB_CDC_STOP_BITS_2:
    {
        ui32Config |= UART_CONFIG_STOP_TWO;
        break;
    }

        //
        // Other cases are either invalid values of ui8Stop or values that we
        // cannot support so set 1 stop bit but return an error.
        //
    default:
    {
        ui32Config |= UART_CONFIG_STOP_ONE;
        bRetcode = false;
        break;
    }
    }

    //
    // Set the UART mode appropriately.
    //
    ROM_UARTConfigSetExpClk(USB_UART_BASE, ROM_SysCtlClockGet(),
                            psLineCoding->ui32Rate, ui32Config);

    //
    // Let the caller know if we had a problem or not.
    //
    return (bRetcode);
}

void GetLineCoding(tLineCoding *psLineCoding)
{
    uint32_t ui32Config;
    uint32_t ui32Rate;

    //
    // Get the current line coding set in the UART.
    //
    ROM_UARTConfigGetExpClk(USB_UART_BASE, ROM_SysCtlClockGet(), &ui32Rate,
                            &ui32Config);
    psLineCoding->ui32Rate = ui32Rate;

    //
    // Translate the configuration word length field into the format expected
    // by the host.
    //
    switch (ui32Config & UART_CONFIG_WLEN_MASK)
    {
    case UART_CONFIG_WLEN_8:
    {
        psLineCoding->ui8Databits = 8;
        break;
    }

    case UART_CONFIG_WLEN_7:
    {
        psLineCoding->ui8Databits = 7;
        break;
    }

    case UART_CONFIG_WLEN_6:
    {
        psLineCoding->ui8Databits = 6;
        break;
    }

    case UART_CONFIG_WLEN_5:
    {
        psLineCoding->ui8Databits = 5;
        break;
    }
    }

    //
    // Translate the configuration parity field into the format expected
    // by the host.
    //
    switch (ui32Config & UART_CONFIG_PAR_MASK)
    {
    case UART_CONFIG_PAR_NONE:
    {
        psLineCoding->ui8Parity = USB_CDC_PARITY_NONE;
        break;
    }

    case UART_CONFIG_PAR_ODD:
    {
        psLineCoding->ui8Parity = USB_CDC_PARITY_ODD;
        break;
    }

    case UART_CONFIG_PAR_EVEN:
    {
        psLineCoding->ui8Parity = USB_CDC_PARITY_EVEN;
        break;
    }

    case UART_CONFIG_PAR_ONE:
    {
        psLineCoding->ui8Parity = USB_CDC_PARITY_MARK;
        break;
    }

    case UART_CONFIG_PAR_ZERO:
    {
        psLineCoding->ui8Parity = USB_CDC_PARITY_SPACE;
        break;
    }
    }

    //
    // Translate the configuration stop bits field into the format expected
    // by the host.
    //
    switch (ui32Config & UART_CONFIG_STOP_MASK)
    {
    case UART_CONFIG_STOP_ONE:
    {
        psLineCoding->ui8Stop = USB_CDC_STOP_BITS_1;
        break;
    }

    case UART_CONFIG_STOP_TWO:
    {
        psLineCoding->ui8Stop = USB_CDC_STOP_BITS_2;
        break;
    }
    }
}

void SendBreak(bool bSend)
{
    //
    // Are we being asked to start or stop the break condition?
    //
    if (!bSend)
    {
        //
        // Remove the break condition on the line.
        //
        ROM_UARTBreakCtl(USB_UART_BASE, false);
        g_bSendingBreak = false;
    }
    else
    {
        //
        // Start sending a break condition on the line.
        //
        ROM_UARTBreakCtl(USB_UART_BASE, true);
        g_bSendingBreak = true;
    }
}
