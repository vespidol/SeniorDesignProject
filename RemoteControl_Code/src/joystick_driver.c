/*
 * joystick_driver.c
 *
 * Created: 12/1/2020 00:04:31
 *  Author: vespi
 */ 

#include "joystick_driver.h"

void joystick_init(){
	/* Configure and enable the ADC */
	//Enable clock for ADC
	REG_PMC_PCER0|= PMC_PCER0_PID29;
	//Reset the controller
	REG_ADC_CR = ADC_CR_SWRST;
	//Reset the Mode Register
	//autocalibration
	//REG_ADC_CR |= ADC_CR_AUTOCAL;
	REG_ADC_MR = 0;
	//Enable ADC Channels 0 & 1
	REG_ADC_CHER |= ADC_CHER_CH0 | ADC_CHER_CH1;
	//Set ADC clock to 1Mhz
	REG_ADC_ACR |= ADC_ACR_IBCTL(1);
	//Clock Prescaler = (fcpu/(2*adc_fq)) -1
	//120,000,000 / (2*1,000,000) - 1 = 59
	REG_ADC_MR |= ADC_MR_PRESCAL(59) | ADC_MR_STARTUP_SUT16 | ADC_MR_USEQ;
	//User sequence for channels 0 and 1
	REG_ADC_SEQR1 |= ADC_SEQR1_USCH1(1);
	//Enable ADC interrupts for end of conversion
	//REG_ADC_IER |= ADC_IER_EOC0 | ADC_IER_EOC1;
		
	//enable ADC interrupts
	//NVIC_EnableIRQ(ADC_IRQn);
}

void GetJoystickCoordinates(uint16_t *x_coord, uint16_t *y_coord){
	
	//start a conversion
	REG_ADC_CR |= ADC_CR_START;
	
	//Wait till conversion is finished and read x,y coordinates		
	while(!((REG_ADC_ISR & ADC_IMR_EOC0) > 0));
	uint32_t status = REG_ADC_ISR;
	*x_coord = ADC->ADC_CDR[0];
	while(!((status & ADC_IMR_EOC1) > 0));
	*y_coord = ADC->ADC_CDR[1];
}

//when ADC conversion is complete it will trigger an interrupt and interrupt will end by starting a new conversion
//void ADC_Handler( void) {
////read interrupt flag status register
//uint32_t status = REG_ADC_ISR;
//uint32_t status_temp = status;
//if ((status & ADC_IMR_EOC1) > 0){ //Channel0 finished
////reading this register clears flag in ISR
//Joystick_Y = ADC->ADC_CDR[1];
//}
//if ((status_temp & ADC_IMR_EOC0) > 0){ //Channel0 finished
////reading this register clears flag in ISR
//Joystick_X = ADC->ADC_CDR[0];
//}
////start the next conversion
//REG_ADC_CR |= ADC_CR_START;
//}