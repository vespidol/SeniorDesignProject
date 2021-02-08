/*
 * MainSystem_Code.c
 *
 * Created: 11/3/2020 17:27:30
 * Author : Victor Espidol
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
#include "Game.h"


/*======================GLOBAL VARIABLES===========================*/

	
int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */
	board_init();
	sysclk_init(); //Clock is set to 120 MHz (To change use conf_clock.h)
	WDT->WDT_MR = WDT_MR_WDDIS;  // disable watchdog
	/*Initialize Peripherals/drivers for system*/
	spi_init();
	timer_init();
	matrix_init(true);
	uart_init();
	
	xTaskCreate(StartGameThread, (signed char*)"StartThread", STACKSIZE, 0, 0, 0);
	
	vTaskStartScheduler();
}

