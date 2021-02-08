/*
 * timer_driver.c
 *
 * Created: 11/17/2020 23:40:52
 *  Author: vespi
 */ 
#include "timer_driver.h"

void timer_init(){
	//Enable TC0 interrupt in the NVIC
	NVIC_EnableIRQ(TC0_IRQn);
	
	//Enable TC0 peripheral clock in the PMC;
	REG_PMC_PCER0 |= PMC_PCER0_PID23;
	
	//Set TC0 clock to the internal master clock/2
	REG_TC0_CMR0 |= TC_CMR_TCCLKS_TIMER_CLOCK1; //48Mhz/2 = 24Mhz
	
	//Enable counter overflow interrupt
	//REG_TC0_IER0 |= TC_IER_COVFS;
	//Compare resets counter and clock
	REG_TC0_CMR0 |= TC_CMR_CPCTRG;
	//Enable RC Compare Interrupt
	REG_TC0_IER0 |= TC_IER_CPCS;
	
	//Enable TC clock
	REG_TC0_CCR0 |= TC_CCR_CLKEN;
	
	REG_TC0_RC0 = 65535;
	/*
	The counter is held in a 16bit reg REG_TC_CV0 
	and will increment until it hits 65535, which will
	trigger an interrupt.
	If clock is running at 10MHz
	(1second/10MHz * 65535) = it will trigger every 6.5535ms
	*/
}

void timer_start(){
	//Once TC0 is setup, reset the counter and start the count
	REG_TC0_CCR0 |= TC_CCR_SWTRG;
}

