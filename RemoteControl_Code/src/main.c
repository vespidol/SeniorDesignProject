/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#include <asf.h>
#include "switch_driver.h"
#include "joystick_driver.h"
#include "uart_driver.h"

uint8_t what_pressed;
uint16_t Joystick_X, Joystick_Y;
uint8_t global_flag;

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */
	board_init();
	sysclk_init(); //Clock is set to 48 MHz (To change use conf_clock.h)
	WDT->WDT_MR = WDT_MR_WDDIS;  // disable watchdog
	
	/* Initialize Drivers */
	switch_init();
	joystick_init();
	uart_init();
	
	uint8_t xtop, xlow, ytop, ylow;	
	    
	what_pressed = 0;
	global_flag = 0;
	while(1){
		delay_ms(300);
		GetJoystickCoordinates(&Joystick_X, &Joystick_Y);
		xlow = Joystick_X;
		xtop = (Joystick_X >> 8);
		ylow = Joystick_Y;
		ytop = (Joystick_Y >> 8);	
		global_flag = 1; //Set high so button interrupt doesn't transmit
		//transmit_byte(xtop); //For some reason sending top byte first doesnt work?
		transmit_byte(xlow);
		delay_ms(50);
		//transmit_byte(xlow);
		transmit_byte(xtop);
		//transmit_byte(ytop);
		//transmit_byte(ylow);
		global_flag = 0; //go low so button can interrupt

		//what_pressed++;
		
	}
}


//when PIOA interrupt is triggered this block of code will run
void PIOA_Handler(void) {
	// reading PIOA_ISR will clear interrupt flags
	uint32_t status = REG_PIOA_ISR;
	
	if(global_flag == 0){
		if ((status & SW1) >= 1){ //Falling edge detected on SW1
			what_pressed = 0x40;
			transmit_byte(what_pressed);
		}
		else if ((status & SW2) >= 1){
			what_pressed = 0x80;
			transmit_byte(what_pressed);
		}
		else if ((status & SW3) >= 1){
			what_pressed = 0xC0;
			transmit_byte(what_pressed);
		}
		else if ((status & SW4) >= 1){
			what_pressed = 0xF0;
			transmit_byte(what_pressed);
		}
	}
}

