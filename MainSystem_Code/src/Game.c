/*
 * Game.c
 *
 * Created: 12/3/2020 17:49:55
 *  Author: vespi
 */ 

#include "Game.h"


uint8_t sineWave[50] = {128,144,160,175,190,203,216,227,
	236,244,250,254,255,255,254,250,
	244,236,227,216,203,190,175,160,
	144,128,112,96,81,66,53,40,
	29,20,12,6,2,0,0,2,
	6,12,20,29,40,53,66,81,
	96,112};

uint8_t triangleWave[50] = {10,20,31,41,51,61,72,82,
	92,102,113,123,133,143,154,164,
	174,184,195,205,215,225,236,246,
	255,246,236,225,215,205,195,184,
	174,164,154,143,133,123,113,102,
	92,82,72,61,51,41,31,20,
	10,0};

Player_t MainPlayer;
bullet_t MainBullet[CHARACTERBULLETS];
Player_t Enemies[9];
bullet_t EnemyBullet[MAXENEMYBULLETS];
volatile int global_flag, gun_flag;
volatile int count;
uint16_t xpos, ypos;
uint8_t Game_state;
bool restart_game;
uint8_t dead_Enemies, live_bullets, mainbullets;

xTaskHandle xCharacter = NULL;
xTaskHandle xEnemy = NULL;
xTaskHandle xBullet = NULL;
xTaskHandle xMatrix = NULL;
xTaskHandle xIdle = NULL;

/*============================= THREADS =========================*/

void StartGameThread(void *p){
	
	//Suspend all other threads
	//initialized communication protocols
	
	count = 0;
	global_flag = 0;
	xpos = 0;
	ypos = 0;
	Game_state = RUNNING;
	restart_game = false;
	dead_Enemies = 0;
	live_bullets = 0;
	mainbullets = 0;
	
	
	//Initialize 4 test button for testing UART
	//enable PIO controller on pins
	REG_PIOC_PER |= PIO_PER_P8 | PIO_PER_P11 | PIO_PER_P14 | PIO_PER_P28;
	//set as output
	REG_PIOC_OER |= PIO_PER_P8 | PIO_PER_P11 | PIO_PER_P14 | PIO_PER_P28;
	//Clear output
	REG_PIOC_CODR |= PIO_PER_P8 | PIO_PER_P11 | PIO_PER_P14 | PIO_PER_P28;
	
	//srand(time(NULL)); // use time as random seed
	timer_start(); //start timer counter
	while(1){
		//Initialize HomeScreen
		matrix_setCursor(1, 0);    // start at top left, with one pixel of spacing
		matrix_setTextSize(1);     // size 1 == 8 pixels high
		matrix_setTextWrap(false); // Don't wrap at end of line - will do ourselves
		matrix_setTextColor(matrix_color333(7,7,7));
		matrix_writeString(" Home\n");
		matrix_writeString(" Menu\n");
		matrix_fillRect(8, 20, 3, 3, matrix_color333(0, 7, 0));  //Game 1 = Blue
		matrix_fillRect(18, 20, 3, 3, matrix_color333(0, 0, 7)); //Game 2 = Green
		matrix_swapBuffers(false);
		matrix_fillScreen(COLOR565_BLACK);
			
		if (global_flag == 1){ //If Switch 1 was pressed start game
			global_flag = 0;
			
			//initialize Fifos
			InitFIFO(XPOS_FIFO);			
			//CreateTasks
			xTaskCreate(UpdateCharacter, (signed char*)"Update_Character", STACKSIZE, 0, 2, &xCharacter);
			xTaskCreate(UpdateEnemy, (signed char*)"Update_Enemy", STACKSIZE, 0,2, &xEnemy);
			xTaskCreate(UpdateMatrix, (signed char*)"Update_Matrix", STACKSIZE, 0, 2, &xMatrix);
			xTaskCreate(BulletThread, (signed char*)"Bullet", STACKSIZE, 0,2, &xBullet);
			xTaskCreate(IdleTask, (signed char*)"IDLE", STACKSIZE, 1, 1, &xIdle);
			
			//InitializeSemaphores
			//KillThread
			//vTaskSuspend(NULL);	
			vTaskDelete(NULL);	
		}
	}
	
}


void UpdateCharacter(void *p){
	int32_t curr_xpos;
	
	MainPlayer.xpos = 13;
	MainPlayer.ypos = 29;
	MainPlayer.color = 0x0007;
	MainPlayer.alive = true;
	MainPlayer.life = 5;
	
	while(1){
		curr_xpos = readFIFO(XPOS_FIFO);		
		if(curr_xpos == -1){
			//do nothing
		}	
		else if(curr_xpos < 1000){
			MainPlayer.xpos += 2;
			if(MainPlayer.xpos < 0){
				MainPlayer.xpos = 0;
			}
		}
		else if(curr_xpos > 3000) {
			MainPlayer.xpos -= 2;
			if(MainPlayer.xpos > (MATRIX_WIDTH-1-CHARACTER_WIDTH)){
				MainPlayer.xpos = MATRIX_WIDTH-1-CHARACTER_WIDTH;			
			}
		}
		
		if(dead_Enemies == NUMBEROFENEMIES){
			Game_state = WIN;
			xTaskCreate(WinGame, (signed char*)"WIN_GAME", STACKSIZE, 0, 3, 0);
			dead_Enemies = 0;
			live_bullets = 0;
			mainbullets = 0;
		}
		
		vTaskDelay(130);
	}
}

void UpdateEnemy(void *p){
	int enemy_count = 0;
	uint8_t random_num = 0, random_num2 = 0;
	
	//Initialize Enemies
	for(int i=0; i<NUMBEROFENEMIES; i++){
		Enemies[i].color = 0x0700;
		Enemies[i].alive = true;
		Enemies[i].life = 3;
		
		if(i <= 3){
			Enemies[i].xpos = 6+(i*6);
			Enemies[i].ypos = 3;
		}
		else if(i > 3 && i <=6){
			Enemies[i].xpos = 8+((i-4)*6);
			Enemies[i].ypos = 9;
		}
		else{
			Enemies[i].xpos = 11+((i-7)*6);
			Enemies[i].ypos = 15;
		}
	}
	
	//Initialize enemy bullets
	for(int i = 0; i<MAXENEMYBULLETS; i++){
		EnemyBullet[i].alive = false;
		EnemyBullet[i].color = 0x0700;
		EnemyBullet[i].xpos = 0;
		EnemyBullet[i].xpos = 0;
	}
	
	while(1){
		for(int i=0; i<NUMBEROFENEMIES; i++){
			if(enemy_count <= 1 ){
				Enemies[i].xpos -= 2;
			}
			else if(enemy_count >= 2 && enemy_count < 4){
				Enemies[i].xpos += 2;
			}
		}
		enemy_count++;
		if(enemy_count >= 4){
			enemy_count = 0;
		}
		
		//Generate random bullet from 0 - 3
		//random_num = rand() % MAXENEMYBULLETS;
		if(EnemyBullet[random_num].alive == false && Enemies[random_num2].alive == true){
			EnemyBullet[random_num].alive = true;
			
			//Choose random Enemy to shoot
			//random_num2 = rand() % NUMBEROFENEMIES;
			EnemyBullet[random_num].xpos = Enemies[random_num2].xpos;
			EnemyBullet[random_num].ypos = Enemies[random_num2].ypos;
			live_bullets++;			
			random_num++;
		}
		random_num2++;
		if(random_num == MAXENEMYBULLETS){
			random_num = 0;
		}
		if(random_num2 == NUMBEROFENEMIES){
			random_num2 = 0;
		}
		
		//update live bullets
		if(live_bullets > 0){
			for(int i =0; i<MAXENEMYBULLETS; i++){
				
				if(EnemyBullet[i].alive == true) {
					//move any live bullets down 1;
					EnemyBullet[i].ypos++;
				
					//Check if any hit the main character
					if(EnemyBullet[i].xpos <= MainPlayer.xpos &&  EnemyBullet[i].xpos > MainPlayer.xpos-CHARACTER_WIDTH-1 && EnemyBullet[i].ypos == MainPlayer.ypos){
						//Bullet is dead
						EnemyBullet[i].alive = false;
						//Decrement Main players life
						MainPlayer.life--;
						//Flash MainPlayer color white or blue
						if(MainPlayer.life % 2 == 0){
							MainPlayer.color = 0x0777;
						}
						else
						{
							MainPlayer.color = 0x0007;
						}
						//Check if life is 0
						if(MainPlayer.life == 0){
							Game_state = LOSE;
							xTaskCreate(LoseGame, (signed char*)"LOSE_GAME", STACKSIZE, 0, 3, 0);
							dead_Enemies = 0;
							live_bullets = 0;
							mainbullets = 0;
						}	
					}
					else if(EnemyBullet[i].ypos == MATRIX_HEIGHT-1){
						//Bullet is dead
						EnemyBullet[i].alive = false;
						live_bullets--;
					}
				}					
			}
		}
		
		//Game_State.balls[ball_num].xVel = 1 + rand() % MAX_BALL_SPEED;
		
		
		vTaskDelay(500);
	}	
}


void UpdateMatrix(void *p){
	while(1){
		//Draw main character
		matrix_fillRect(MainPlayer.xpos, MainPlayer.ypos, CHARACTER_WIDTH, CHARACTER_HEIGHT, matrix_color333((MainPlayer.color >> 8 & 0xF), (MainPlayer.color >> 4 & 0xF), (MainPlayer.color & 0xF)));
		//Draw Enemies
		for(int i=0; i<NUMBEROFENEMIES; i++){
			if(Enemies[i].alive == true){
				matrix_fillRect(Enemies[i].xpos, Enemies[i].ypos, ENEMY_WIDTH, ENEMY_HEIGHT, matrix_color333((Enemies[i].color >> 8 & 0xF),(Enemies[i].color >> 4 & 0xF),(Enemies[i].color & 0xF)));
			}
		}
		//Draw Live bullets
		if(live_bullets > 0){
			for(int i=0; i<MAXENEMYBULLETS; i++){
				if(EnemyBullet[i].alive == true){
					matrix_drawPixel(EnemyBullet[i].xpos + (ENEMY_WIDTH/2), EnemyBullet[i].ypos, matrix_color333((EnemyBullet[i].color >> 8 & 0xF),(EnemyBullet[i].color >> 4 & 0xF),(EnemyBullet[i].color & 0xF)));
				}
			}
		}
		//Draw character bullets
		if(mainbullets > 0){
			for(int i=0; i<CHARACTERBULLETS; i++){
				if(MainBullet[i].alive == true){
					matrix_drawPixel(MainBullet[i].xpos+(CHARACTER_WIDTH/2), MainBullet[i].ypos, matrix_color333((MainBullet[i].color >> 8 & 0xF),(MainBullet[i].color >> 4 & 0xF),(MainBullet[i].color & 0xF)));
				}
			}
		}
		matrix_swapBuffers(false);
		matrix_fillScreen(COLOR565_BLACK);
		vTaskDelay(130);		
	}
}


void BulletThread(void *p){
	uint8_t current_bullet = 0;
	
	//initialize bullets
	for(int i =0; i<CHARACTERBULLETS; i++){
		MainBullet[i].alive = false;
		MainBullet[i].color = 0x0007; //blue
		MainBullet[i].xpos = 0;
		MainBullet[i].ypos = 0;
		
	}
	
	while(1){
		if(gun_flag == 1){
			gun_flag = 0;
			
			//Send Dac Signal with Period of 30;
			for(int i = 0; i<30; i++){
				for(int j =0; j<50; j++){
					spi_write(sineWave[j]);
				}
			}
			
			//Generate a bullet
			if(MainBullet[current_bullet].alive == false){
				//Bullet is alive
				MainBullet[current_bullet].alive = true;
				
				//Set Bullet location
				MainBullet[current_bullet].xpos = MainPlayer.xpos;
				MainBullet[current_bullet].ypos = MainPlayer.ypos;
				mainbullets++;
			}
			current_bullet++;
			if(current_bullet == CHARACTERBULLETS){
				current_bullet = 0;
			}
			
		}
							
		//Update bullets
		if(mainbullets > 0){
			for(int j = 0; j<CHARACTERBULLETS; j++){
				if(MainBullet[j].alive){
					//Move Bullet
					MainBullet[j].ypos--;
								
					//Check if it hit the enemy
					for(int i =0; i<NUMBEROFENEMIES; i++){
						if(Enemies[i].alive == true){
							//Is bullet in the same spot as the main character?
							if(MainBullet[j].xpos >= Enemies[i].xpos && MainBullet[j].xpos < Enemies[i].xpos+ENEMY_WIDTH && MainBullet[j].ypos == Enemies[i].ypos){
								//Decrement life and change its color
								Enemies[i].life--;
								Enemies[i].color >>= 4;
								Enemies[i].color |= 0x0700;
								//Delete bullet
								MainBullet[j].alive = false;
								//Decrement number of main bullets
								mainbullets--;
											
								//Check if enemy dead
								if(Enemies[i].life == 0){
									Enemies[i].alive = false;
									dead_Enemies++;
									//Send Dac Signal with Period of 60;
									for(int i = 0; i<60; i++){
										for(int j =0; j<50; j++){
											spi_write(triangleWave[j]);
										}
									}
								}
							}
						}
					}
					//Otherwise check if Bullet is at the end
					if(MainBullet[j].ypos == 0){
						MainBullet[j].alive = false;
						mainbullets--;
					}
				}
			}
		}
		
		
		
		vTaskDelay(80);
	}
}



void WinGame(void *p){
	//Kill All Threads
	vTaskDelete(xCharacter);
	vTaskDelete(xEnemy);
	vTaskDelete(xMatrix);
	vTaskDelete(xBullet);
	vTaskDelete(xIdle);
	
	//while restart game is false
	while(!restart_game){
		
		//Display winning screen
		matrix_setCursor(1, 0);    // start at top left, with one pixel of spacing
		matrix_setTextSize(1);     // size 1 == 8 pixels high
		matrix_setTextWrap(false); // Don't wrap at end of line - will do ourselves
		matrix_setTextColor(matrix_color333(0,0,7));			
		matrix_writeString("\n");
		matrix_writeString(" YOU\n");
		matrix_writeString(" WON!\n");
		matrix_swapBuffers(false);
		matrix_fillScreen(COLOR565_BLACK);
	}
	
	//Create StartGameThread again
	xTaskCreate(StartGameThread, (signed char*)"StartThread", STACKSIZE, 0, 0, 0);
	
	//Kill this current thread
	vTaskDelete(NULL);
}


void LoseGame(void *p){
	//Kill All Threads
	vTaskDelete(xCharacter);
	vTaskDelete(xEnemy);
	vTaskDelete(xMatrix);
	vTaskDelete(xBullet);
	vTaskDelete(xIdle);
	
	//while restart game is false
	while(!restart_game){
			
		//Display winning screen
		matrix_setCursor(1, 0);    // start at top left, with one pixel of spacing
		matrix_setTextSize(1);     // size 1 == 8 pixels high
		matrix_setTextWrap(false); // Don't wrap at end of line - will do ourselves
		matrix_setTextColor(matrix_color333(7,0,0));
		matrix_writeString("\n");
		matrix_writeString(" YOU\n");
		matrix_writeString(" LOSE!\n");
		matrix_swapBuffers(false);
		matrix_fillScreen(COLOR565_BLACK);
	}
	
	//Create StartGameThread again
	xTaskCreate(StartGameThread, (signed char*)"StartThread", STACKSIZE, 0, 0, 0);
		
	//Kill this current thread
	vTaskDelete(NULL);
	
}



void IdleTask(void *p){
	while(1){
		
	}
}


/*============================= ISR =========================*/

void UART0_Handler(void) {
	// when we receive a byte, transmit that byte back
	uint32_t status = REG_UART0_SR;
	
	if ((status & UART_SR_RXRDY)){
		//read receive holding register
		uint8_t readByte = REG_UART0_RHR;
		
		if (readByte == 0x40 && count == 0){
			global_flag = 1;
			
			if((REG_PIOC_ODSR & PIO_PER_P28) > 0){
				REG_PIOC_CODR |= PIO_PER_P28;
			}
			else {
				REG_PIOC_SODR |= PIO_PER_P28;
			}
		}
		else if (readByte == 0x80 && count == 0){
			restart_game = true;
			if((REG_PIOC_ODSR & PIO_PER_P8) > 0){
				REG_PIOC_CODR |= PIO_PER_P8;
			}
			else {
				REG_PIOC_SODR |= PIO_PER_P8;
			}
		}
		else if (readByte == 0xC0 && count == 0) {
			if((REG_PIOC_ODSR & PIO_PER_P11) > 0){
				REG_PIOC_CODR |= PIO_PER_P11;
			}
			else {
				REG_PIOC_SODR |= PIO_PER_P11;
			}
		}
		else if (readByte == 0xF0 && count == 0) {
			gun_flag = 1;
			if((REG_PIOC_ODSR & PIO_PER_P14) > 0){
				REG_PIOC_CODR |= PIO_PER_P14;
			}
			else {
				REG_PIOC_SODR |= PIO_PER_P14;
			}
		}
		else {
			if(count <= 1){ //X-coordinate
				xpos |= readByte;
				
				if(count == 0){
					xpos &= 0x0F;
					xpos = xpos << 8;
				}
				
				count++;
			}			
			if(count >= 2){
				writeFIFO(XPOS_FIFO, xpos);
				//reset joystick
				xpos = 0;
				ypos = 0;
				count = 0;
			}

		}
		
	}
}


void TC0_Handler(){
	//Read status register - this clears interrupt flags
	uint32_t status = REG_TC0_SR0;
	
	if((status & TC_SR_CPCS)>=1){
		//code to run on overflow
		//such as global variable
		//counter += 1;
		//count++;
		//spi_write(sineWave[i]);
		//i++;
		//if(i >= 50){
		//	i = 0;
		//	
		updateDisplay();
	}
	//REG_TC0_RC0 = 240;

}



/*=================================FIFO===============================*/
#define FIFOSIZE 16
#define MAX_NUMBER_OF_FIFOS 2

/* Create FIFO struct here */
typedef struct FIFO_t{
	int32_t Buffer[FIFOSIZE];
	int32_t *Head;
	int32_t *Tail;
	uint32_t LostData;
	uint32_t CurrentSize;
}FIFO_t;


/* Array of FIFOS */
static FIFO_t FIFOs[MAX_NUMBER_OF_FIFOS];
/*
 * Initializes FIFO Struct
 */
int InitFIFO(uint32_t FIFOIndex)
{

    if(FIFOIndex >= MAX_NUMBER_OF_FIFOS){
        return -1;
    }

    for(int i=0;i<FIFOSIZE; i++){   //Initialize buffer values to be 0 initially
        FIFOs[FIFOIndex].Buffer[i] = 0;
    }

    FIFOs[FIFOIndex].Head = &FIFOs[FIFOIndex].Buffer[0];    //Set Head and Tail pointers to first
    FIFOs[FIFOIndex].Tail = &FIFOs[FIFOIndex].Buffer[0];    //position in the FIFO

    FIFOs[FIFOIndex].LostData = 0;
	FIFOs[FIFOIndex].CurrentSize = 0;

    return 1;
}

/*
 * Reads FIFO
 *  - Waits until CurrentSize semaphore is greater than zero
 *  - Gets data and increments the head ptr (wraps if necessary)
 * Param: "FIFOChoice": chooses which buffer we want to read from
 * Returns: uint32_t Data from FIFO
 */
int32_t readFIFO(uint32_t FIFOChoice)
{
    int32_t data = 0;

    if(FIFOs[FIFOChoice].CurrentSize == 0){
        return -1;
    }

    data = *(FIFOs[FIFOChoice].Head);     //Read data from head of FIFO
    FIFOs[FIFOChoice].Head++;           //Increment next position in buffer


    if(FIFOs[FIFOChoice].Head == &FIFOs[FIFOChoice].Buffer[FIFOSIZE]){
        FIFOs[FIFOChoice].Head = &FIFOs[FIFOChoice].Buffer[0]; //Wrap to first one
    }
	
	FIFOs[FIFOChoice].CurrentSize--;

    return data;
}


/*
 * Writes to FIFO
 *  Writes data to Tail of the buffer if the buffer is not full
 *  Increments tail (wraps if ncessary)
 *  Param "FIFOChoice": chooses which buffer we want to read from
 *        "Data': Data being put into FIFO
 *  Returns: error code for full buffer if unable to write
 */
int writeFIFO(uint32_t FIFOChoice, int32_t Data)
{
    //G8RTOS_WaitSemaphore(&FIFOs[FIFOChoice].CurrentSize); //Check if full
    if(FIFOs[FIFOChoice].CurrentSize == FIFOSIZE){
        FIFOs[FIFOChoice].LostData++;   //Error in fifo
        return -1;
    }

    *FIFOs[FIFOChoice].Tail = Data;     //Write data;
    FIFOs[FIFOChoice].Tail++;      //Increment to next position in fifo

    if(FIFOs[FIFOChoice].Tail == &FIFOs[FIFOChoice].Buffer[FIFOSIZE]){
        FIFOs[FIFOChoice].Tail = &FIFOs[FIFOChoice].Buffer[0];   //Wrap
    }
	
	FIFOs[FIFOChoice].CurrentSize++;
    return 0;
}