/*
 * Switch_driver.c
 *
 * Created: 11/29/2020 01:15:45
 *  Author: vespi
 */ 

#include "switch_driver.h"

void switch_init(){	
	//Enable clock for PIOA controller
	REG_PMC_PCER0 |= PMC_PCER0_PID11;
		
	//Set pushbutton switches as controllable by PIO controller
	REG_PIOA_PER |= SW1 | SW2 | SW3 | SW4;
	//Output disable register (set as input for button)
	REG_PIOA_ODR |= SW1 | SW2 | SW3 | SW4;
	//Disable pull down
	REG_PIOA_PPDDR |= SW1 | SW2 | SW3 | SW4;
	//Disable internal pull up resistor
	REG_PIOA_PUDR |= SW1 | SW2 | SW3 | SW4;
	//Enable glitch filter on buttons (debounce)
	REG_PIOA_IFER |= SW1 | SW2 | SW3 | SW4;
	//Read ISR so that it clears any interrupt flags that might be there
	uint32_t temp = REG_PIOA_ISR;
	//Set interrupt to be an event driven interrupt
	REG_PIOA_AIMER |= SW1 | SW2 | SW3 | SW4;
	//Set interrupt source to be an edge-detection event
	REG_PIOA_ESR |= (SW1 | SW2 | SW3 | SW4);
	//Set the interrupt to be on a falling edge event
	REG_PIOA_FELLSR |= (SW1 | SW2 | SW3 | SW4);
	//enable input change interrupt on PA24
	REG_PIOA_IER |= SW1 | SW2 | SW3 | SW4;
	//enable PIOA interrupts
	NVIC_EnableIRQ(PIOA_IRQn);
}