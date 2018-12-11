/*
 * Copyright (c) 2010 by Cristian Maglie <c.maglie@bug.st>
 * SPI Master library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ssi.h"
#include "inc/hw_types.h"
#include "driverlib/rom.h"
#include "driverlib/ssi.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom_map.h"
#include "SPI.h"

#define SSIBASE g_ulSSIBase[SSIModule]
#define NOT_ACTIVE 0xA

/* variants
   stellarpad - LM4F120H5QR, TM4C123GH6PM, aka TARGET_IS_BLIZZARD_RB1
    i base  port
    0 SSI0 PA
    1 SSI1 PF
    2 SSI2 PB
    3 SSI3 PD
*/

static const unsigned long g_ulSSIBase[] = {
    SSI0_BASE, SSI1_BASE, SSI2_BASE, SSI3_BASE
};

//*****************************************************************************
//
// The list of SSI peripherals.
//
//*****************************************************************************
static const unsigned long g_ulSSIPeriph[] = {
    SYSCTL_PERIPH_SSI0, SYSCTL_PERIPH_SSI1, SYSCTL_PERIPH_SSI2, SYSCTL_PERIPH_SSI3
};

//*****************************************************************************
//
// The list of SSI gpio configurations.
//
//*****************************************************************************
static const unsigned long g_ulSSIConfig[][4] = {
    {GPIO_PA2_SSI0CLK, GPIO_PA3_SSI0FSS, GPIO_PA4_SSI0RX, GPIO_PA5_SSI0TX},
    {GPIO_PF2_SSI1CLK, GPIO_PF3_SSI1FSS, GPIO_PF0_SSI1RX, GPIO_PF1_SSI1TX},
    {GPIO_PB4_SSI2CLK, GPIO_PB5_SSI2FSS, GPIO_PB6_SSI2RX, GPIO_PB7_SSI2TX},
    {GPIO_PD0_SSI3CLK, GPIO_PD1_SSI3FSS, GPIO_PD2_SSI3RX, GPIO_PD3_SSI3TX},
};

//*****************************************************************************
//
// The list of SSI gpio port bases.
//
//*****************************************************************************
static const unsigned long g_ulSSIPort[] = {
    GPIO_PORTA_BASE, GPIO_PORTF_BASE, GPIO_PORTB_BASE, GPIO_PORTD_BASE
};

//*****************************************************************************
//
// The list of SSI gpio configurations.
//
//*****************************************************************************
static const unsigned long g_ulSSIPins[] = {
	GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5,
	GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
	GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7,
	GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3
};

SPIClass::SPIClass(void) {
	SSIModule = 2;
	SSIBitOrder = LSBFIRST;
}

SPIClass::SPIClass(uint8_t module) {
	SSIModule = module;
	SSIBitOrder = MSBFIRST;
}

void SPIClass::begin() {
	unsigned long initialData = 0;

    if(SSIModule == NOT_ACTIVE) {
        SSIModule = BOOST_PACK_SPI;
    }

	ROM_SysCtlPeripheralEnable(g_ulSSIPeriph[SSIModule]);
	ROM_SSIDisable(SSIBASE);
	ROM_GPIOPinConfigure(g_ulSSIConfig[SSIModule][0]);
	ROM_GPIOPinConfigure(g_ulSSIConfig[SSIModule][1]);
	ROM_GPIOPinConfigure(g_ulSSIConfig[SSIModule][2]);
	ROM_GPIOPinConfigure(g_ulSSIConfig[SSIModule][3]);
	ROM_GPIOPinTypeSSI(g_ulSSIPort[SSIModule], g_ulSSIPins[SSIModule]);


	/*
	  Polarity Phase        Mode
	     0 	   0   SSI_FRF_MOTO_MODE_0
	     0     1   SSI_FRF_MOTO_MODE_1
	     1     0   SSI_FRF_MOTO_MODE_2
	     1     1   SSI_FRF_MOTO_MODE_3
	*/

	/*
	 * Default to
	 * System Clock, SPI_MODE_0, MASTER,
	 * 1MHz bit rate, and 8 bit data
	*/
	ROM_SSIClockSourceSet(SSIBASE, SSI_CLOCK_SYSTEM);

	ROM_SSIConfigSetExpClk(SSIBASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 1000000, 8);

	ROM_SSIEnable(SSIBASE);

	//clear out any initial data that might be present in the RX FIFO
	while(ROM_SSIDataGetNonBlocking((uint32_t)SSIBASE, (uint32_t *)&initialData));
}

void SPIClass::end() {
	ROM_SSIDisable(SSIBASE);
}

void SPIClass::setBitOrder(uint8_t ssPin, uint8_t bitOrder) {
	SSIBitOrder = bitOrder;
}

void SPIClass::setBitOrder(uint8_t bitOrder) {
	SSIBitOrder = bitOrder;
}

void SPIClass::setDataMode(uint8_t mode) {
	HWREG(SSIBASE + SSI_O_CR0) &= ~(SSI_CR0_SPO | SSI_CR0_SPH);
	HWREG(SSIBASE + SSI_O_CR0) |= mode;
}

void SPIClass::setClockDivider(uint8_t divider){
  //value must be even
  HWREG(SSIBASE + SSI_O_CPSR) = divider;
}

uint8_t SPIClass::transfer(uint8_t data) {
	unsigned long rxtxData;

	rxtxData = data;
	if(SSIBitOrder == LSBFIRST) {
		__rbit(rxtxData);	// reverse order of 32 bits
		__rev(rxtxData);	// reverse order of bytes to get original bits into lowest byte
	}
	ROM_SSIDataPut(SSIBASE, (uint8_t) rxtxData);

	while(ROM_SSIBusy(SSIBASE));

	ROM_SSIDataGet((uint32_t) SSIBASE,(uint32_t *) &rxtxData);
	if(SSIBitOrder == LSBFIRST) {
		__rbit(rxtxData);	// reverse order of 32 bits
		__rev(rxtxData);	// reverse order of bytes to get original bits into lowest byte
	}

	return (uint8_t) rxtxData;
}

void SPIClass::setModule(uint8_t module) {
	SSIModule = module;
	begin();
}

SPIClass SPI;
