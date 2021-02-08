#include "spi_driver.h"


void spi_init(){
	//Set for 12-bit transfer
	
	//enable peripheral clock
	REG_PMC_PCER0 |= PMC_PCER0_PID21;
	//set spi master mode
	REG_SPI_MR |= SPI_MR_MSTR;
	//set fixed peripheral select(peripheral chosen in SP_MR.PCS instead of SPI_THR.PCS)
	REG_SPI_MR &= ~SPI_MR_PS;
	//set polarity and clock phase
	spi_setMode(3);
	//set clock generator (1 = peripheral clock rate), otherwise a divisor
	//SCBR = fperipheral clock / SPCK Bit Rate -> 48MHz/12 = 4MHz
	SPI->SPI_CSR[0] |= SPI_CSR_SCBR(12);
	//chip select rises as soon as the last transfer is achieved
	SPI->SPI_CSR[0] &= ~SPI_CSR_CSAAT;
	SPI->SPI_CSR[0] |= SPI_CSR_DLYBS(1);
	SPI->SPI_CSR[0] |= SPI_CSR_BITS_16_BIT;
	//give peripheral control of pins (Chip select pins are optional)
	REG_PIOA_PDR |= PIO_PDR_P11; //NPCS0/NSS
	REG_PIOA_PDR |= PIO_PDR_P13; //MOSI
	REG_PIOA_PDR |= PIO_PDR_P14; //SPCK
	//Set the Peripheral chip select to NPCS0
	REG_SPI_MR |= SPI_MR_PCS(0b1110);
	//enable SPI
	REG_SPI_CR |= SPI_CR_SPIEN;
}

void spi_setMode(uint8_t mode){	
	/*
    Mode		CPOL	NCPHA
    Mode0		0		1
    Mode1		0		0
    Mode2		1		1
    Mode3		1		0
    */
    if (mode == 0){
        SPI->SPI_CSR[0] &= ~SPI_CSR_CPOL;
        SPI->SPI_CSR[0] |= SPI_CSR_NCPHA;
    }
    else if (mode == 1){
        SPI->SPI_CSR[0] &= ~SPI_CSR_CPOL;
        SPI->SPI_CSR[0] &= ~SPI_CSR_NCPHA;
    }
    else if (mode == 2){
        SPI->SPI_CSR[0] |= SPI_CSR_CPOL;
        SPI->SPI_CSR[0] |= SPI_CSR_NCPHA;
    }
    else if (mode == 3){
        SPI->SPI_CSR[0] |= SPI_CSR_CPOL;
        SPI->SPI_CSR[0] &= ~SPI_CSR_NCPHA;
    }
}

void spi_write(uint8_t data){
	uint8_t readData;
	uint16_t tempData;
	//chip select automatically Low when transfer starts
	
	tempData = 0x0F00 | data;
	tempData = tempData << 4;
	//data = data >> 4;   //Shift data to the right 4 bits
	//data = data | 0xF0; //In the Form 0xFX (X is any digit)
	//wait for transmit register to be empty
	while (!(REG_SPI_SR & SPI_SR_TDRE));	
	//send data to transmit register
	REG_SPI_TDR |= tempData;	
	//wait for received data to be ready to be read
	while (!(REG_SPI_SR & SPI_SR_RDRF));
	//Read data to clear the shift register	
	readData = REG_SPI_RDR;
}