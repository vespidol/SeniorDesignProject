/*
 * IncFile1.h
 *
 * Created: 11/16/2020 01:31:50
 *  Author: vespi
 */ 


#ifndef SPI_DRIVER_H_
#define SPI_DRIVER_H_

#include "sam.h"

void spi_init();
void spi_setMode(uint8_t mode);
void spi_write(uint8_t data);


#endif /* SPI_DRIVER_H_ */