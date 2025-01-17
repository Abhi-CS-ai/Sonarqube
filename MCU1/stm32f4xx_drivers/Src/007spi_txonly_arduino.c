/*
 * 007spi_txonly_arduino.c
 *
 *  Created on: 27 de jun de 2020
 *      Author: tiago
 */


/* SPI2 PINS
 * PB14 -- > MISO
 * PB15 --> MOSI
 * PB13 -- > SCLK
 * PB12 --> NSS
 * ALT function mode: 5
 */

#include "../Inc/stm32f407xx_gpio_driver.h"
#include "../Inc/stm32f4xx_spi_driver.h"
#include <string.h>

void delay(void){
	for(uint32_t i=0;i<500000/2;i++);
}

void SPI2_GPIOInits(void){
	GPIO_Handle_t SPIPins;

	SPIPins.pGPIOx = GPIOB;
	SPIPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	SPIPins.GPIO_PinConfig.GPIO_PinAltFunMode = 5;
	SPIPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	SPIPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
	SPIPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

	// SCLK
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
	GPIO_Init(&SPIPins);

	//MOSI
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_15;
	GPIO_Init(&SPIPins);

	//MISO
	//SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_14;
	//GPIO_Init(&SPIPins);

	//NSS
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
	GPIO_Init(&SPIPins);
}

void SPI2_Inits(void){

	SPI_Handle_t SPI2handle;

	SPI2handle.pSPIx = SPI2;
	SPI2handle.SPIConfig.SPI_BusConfig = SPI_BUS_CONFIG_FD;
	SPI2handle.SPIConfig.SPI_DeviceMode = SPI_DEVICE_MODE_MASTER;
	//generates sclk of 8MHz
	SPI2handle.SPIConfig.SPI_SclkSpeed = SPI_SCLK_SPEED_DIV2;
	SPI2handle.SPIConfig.SPI_DFF = SPI_DFF_8BITS;
	SPI2handle.SPIConfig.SPI_CPOL = SPI_CPOL_LOW;
	SPI2handle.SPIConfig.SPI_CPHA = SPI_CPHA_LOW;
	// software slave management enable for NSS pin
	SPI2handle.SPIConfig.SPI_SSM = SPI_SSM_DI;

	SPI_Init(&SPI2handle);
}

void GPIO_ButtonInit(){

	GPIO_Handle_t GPIOBtn;
	// INPUT = read the state of an electrical signal
	GPIOBtn.pGPIOx 								= 	GPIOA;
	GPIOBtn.GPIO_PinConfig.GPIO_PinNumber 		=	GPIO_PIN_NO_0;
	GPIOBtn.GPIO_PinConfig.GPIO_PinMode 		= 	GPIO_MODE_IN;
	GPIOBtn.GPIO_PinConfig.GPIO_PinSpeed		= 	GPIO_SPEED_FAST;
	//GpioLed.GPIO_PinConfig.GPIO_PinOPType		=	GPIO_OP_TYPE_OD; because it is an input
	// there is an internal pull-down register in the board button
	GPIOBtn.GPIO_PinConfig.GPIO_PinPuPdControl	=	GPIO_NO_PUPD;

	GPIO_Init(&GPIOBtn);
}

int main(void){

	char user_data[] = "Hello world";

	GPIO_ButtonInit();

	// this function is used to initialized the GPIO pins to behave as SPI2 pins
	SPI2_GPIOInits();
	//this function is used to initialize the SPI2 peripheral parameters
	SPI2_Inits();

	/*	SSM = 0 --> hardware manegment
	 * (SSM = 0, SSOE = 1) --> NSS output enable
	 *	(SSM = 0, SSOE = 0) --> NSS output disable
	 *	SPE = 1 --> NSS is HIGH
	 *	SPE = 0 --> NSS is LOW
	 */
	SPI_SSOEConfig(SPI2, ENABLE);

	while(1){

		// wait until the button is pressed
		while( !GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0) );

		// to avoid debucing
		delay();

		//Enable SPI2 peripheral
		SPI_PeripheralControl(SPI2, ENABLE);

		//first send length information, because the arduino doesn't know how many data it will receive
		uint8_t dataLen = strlen(user_data);
		SPI_SendData(SPI2, &dataLen, 1);

		//send data
		SPI_SendData(SPI2, (uint8_t*)user_data, strlen(user_data));

		//let's confirm SPI is not busy
		while( SPI_GetFlagStatus(SPI2, SPI_BUSY_FLAG) );

		//Disable SPI2 peripheral
		SPI_PeripheralControl(SPI2, DISABLE);
	}

	return 0;
}
