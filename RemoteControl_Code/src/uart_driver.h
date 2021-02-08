/*
 * uart_driver.h
 *
 * Created: 12/1/2020 03:13:39
 *  Author: vespi
 */ 


#ifndef UART_DRIVER_H_
#define UART_DRIVER_H_

#include <asf.h>

void uart_init();
void transmit_byte(uint8_t data);
uint8_t read_byte();

#endif /* UART_DRIVER_H_ */