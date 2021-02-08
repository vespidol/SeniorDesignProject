/*
 * uart_driver.c
 *
 * Created: 12/1/2020 03:13:58
 *  Author: vespi
 */ 

#include "uart_driver.h"

void uart_init(){
	/*UART SETUP*/
	//Enable UART0 Clock
	REG_PMC_PCER0 |= PMC_PCER0_PID8;
	//Disable PIOA control on pins PA9 & PA10
	REG_PIOA_PDR |= PIO_PDR_P9 | PIO_PDR_P10;
	//Enable peripheral (A) on pins PA9 & PA10
	PIOA->PIO_ABCDSR[0] &= ~(PIO_PDR_P9 | PIO_PDR_P10);
	PIOA->PIO_ABCDSR[1] &= ~(PIO_PDR_P9 | PIO_PDR_P10);
	//Configure Baud Rate
	//fcpu/16*BR -> 120,000,000/(16*9600) = 781
	REG_UART0_BRGR |= 781;
	//Parity
	REG_UART0_MR |= UART_MR_PAR_NO;
	//Set to Normal mode by default
	//enable transmit/receive
	REG_UART0_CR |= UART_CR_TXEN;
	REG_UART0_CR |= UART_CR_RXEN;
	//enable interrupt on receive
	REG_UART0_IER |= UART_IER_RXRDY;
	//Enable UART0 Interrupts
	NVIC_EnableIRQ(UART0_IRQn);
}

void transmit_byte(uint8_t data){
	//wait for ready
	while (!(REG_UART0_SR & UART_SR_TXRDY));
	while (!(REG_UART0_SR & UART_SR_TXEMPTY));
	REG_UART0_THR |= data;
}

uint8_t read_byte(){
	return REG_UART0_RHR;
}