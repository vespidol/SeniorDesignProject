/*
 * Game.h
 *
 * Created: 12/3/2020 17:50:11
 *  Author: vespi
 */ 


#ifndef GAME_H_
#define GAME_H_

#include <asf.h>
#include <stdlib.h>
#include "spi_driver.h"
#include "timer_driver.h"
#include "LEDMatrix_driver.h"
#include "uart_driver.h"
#include <time.h>
#include <stdio.h>


/*===================DEFINES===================*/
#define SW1 PIO_PER_P28
#define SW2 PIO_PER_P8
#define SW3 PIO_PER_P11
#define SW4 PIO_PER_P14

#define XPOS_FIFO 0
#define YPOS_FIFO 1

#define CHARACTER_WIDTH 3
#define CHARACTER_HEIGHT 3
#define CHARACTERBULLETS 4
#define ENEMY_WIDTH 4
#define ENEMY_HEIGHT 4
#define NUMBEROFENEMIES 9
#define MAXENEMYBULLETS 9

#define WIN 1
#define LOSE 0
#define RUNNING 2

#define STACKSIZE 512

/*==================================STRUCTs==================================*/
typedef struct {
	int16_t xpos;
	int16_t ypos;
	uint16_t color;	
	uint16_t life;
	bool alive;
}Player_t;

typedef struct{
	int16_t xpos;
	int16_t ypos;
	uint16_t color;
	bool alive;
}bullet_t;



/*=====================================THREADS===================================*/
void StartGameThread(void *p);
void UpdateCharacter(void *p);
void UpdateEnemy(void *p);
void BulletThread(void *p);
void UpdateMatrix(void *p);
void WinGame(void *p);
void LoseGame(void *p);
//void ResumeHomeScreen(void *p);
void IdleTask(void *p); //Set priority to 0 which is lowest

/*=====================================FIFO===================================*/

/*
 * Initializes One to One FIFO Struct
 */
int InitFIFO(uint32_t FIFOIndex);

/*
 * Reads FIFO
 *  - Waits until CurrentSize semaphore is greater than zero
 *  - Gets data and increments the head ptr (wraps if necessary)
 * Param "FIFOChoice": chooses which buffer we want to read from
 * Returns: uint32_t Data from FIFO
 */
int32_t readFIFO(uint32_t FIFO);

/*
 * Writes to FIFO
 *  Writes data to Tail of the buffer if the buffer is not full
 *  Increments tail (wraps if ncessary)
 *  Param "FIFOChoice": chooses which buffer we want to read from
 *        "Data': Data being put into FIFO
 *  Returns: error code for full buffer if unable to write
 */
int writeFIFO(uint32_t FIFO, int32_t data);





#endif /* GAME_H_ */