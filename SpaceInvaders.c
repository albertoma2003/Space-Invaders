// SpaceInvaders.c
// Runs on TM4C123
// Josemiliano Cohen
// This is a starter project for the EE319K Lab 10

// Last Modified: 8/24/2022 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php

// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// buttons connected to PE0-PE3
// 32*R resistor DAC bit 0 on PB0 (least significant bit)
// 16*R resistor DAC bit 1 on PB1
// 8*R resistor DAC bit 2 on PB2 
// 4*R resistor DAC bit 3 on PB3
// 2*R resistor DAC bit 4 on PB4
// 1*R resistor DAC bit 5 on PB5 (most significant bit)
// LED on PD1
// LED on PD0


#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/ST7735.h"
#include "Random.h"
#include "TExaS.h"
#include "../inc/ADC.h"
#include "Images1.h"
#include "Sound.h"
#include "Timer1.h"



void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay100ms(uint32_t count); // time delay in 0.1 seconds

uint32_t time=0;


void PortE_Init(void){
	volatile uint32_t delay=100;
	SYSCTL_RCGCGPIO_R |= 0x10; // Port E initialization
	delay;
	GPIO_PORTE_DIR_R &= ~0x0F; // buttons are PE0-3
	GPIO_PORTE_DEN_R |= 0x0F; // PE0-3  
}

void PortB_Init(void){
	volatile uint32_t delay=100;
		SYSCTL_RCGCGPIO_R |= 0x02; // initializing port b
		delay; // waiting for clock to turn on
	  // making my outputs 1s and inputs 0s
		GPIO_PORTB_DIR_R  |=0x3F ; // port b 5-0 outputs
	  // enabling all my inputs and outputs
		GPIO_PORTB_DEN_R  |=0x3F ; // port b 5-0
	  GPIO_PORTB_DR8R_R |= 0x3F;
}

int32_t Data;
int32_t OldPosition;
int32_t NewPosition;

typedef enum {dead, alive} status_t;

struct bullet{
	int32_t x;      // x coordinate
  int32_t y;      // y coordinate
  int32_t vy;  // pixels/30Hz // can only go up or down
  const unsigned short *image; // ptr->image
  const unsigned short *bimage;
  status_t life;        // dead/alive
  int32_t w; // width
  int32_t h; // height
};

typedef struct bullet bullet_t;
#define NUMB 30
bullet_t bullets[NUMB];

void Bullet_Init (void){
	for(int i=0; i<NUMB; i++){
		bullets[i].life =dead;
		bullets[i].image = Laser;
		bullets[i].bimage = BlackLaser;
		bullets[i].h = 6;
		bullets[i].w = 4;
	}
}
struct sprite{
 int32_t x;      // x coordinate
  int32_t y;      // y coordinate
  int32_t vx,vy;  // pixels/30Hz
  const unsigned short *image; // ptr->image
  const unsigned short *black;
  status_t life;        // dead/alive
  int32_t w; // width
  int32_t h; // height
  uint32_t needDraw; // true if need to draw
};
typedef struct sprite sprite_t;
sprite_t Enemy[18]; // creating 18 different enemies, not all will be alove at start
sprite_t Player;
int Flag;
int Anyalive;

void GameInit (void){ int i;
	Flag =0;
	for(i=0; i<6; i++){
		Enemy[i].x = 20*i; // all enemies will start at top of screen just shifted horizontally across screen
		Enemy[i].y = 10; // all enemies will start at the top of your screen just at different x positions
		Enemy[i].vx = 0;
		Enemy[i].vy = 0;
		
		Enemy[i].image = SmallEnemy10pointA;
		Player.x = NewPosition;
		Player.image = PlayerShip0;
		Player.life = alive;
		Enemy[i].black = BlackEnemy;
		Enemy[i].life = alive;
		Enemy[i].w = 16; // width
		Enemy[i].h = 10; // height
		
		Enemy[i].needDraw = 1; // since sprites are new, they will need to be drawn
	}
	
		Enemy[0].vy = 1; // down
//		Enemy[0].vx = 1; // down
//	if( Enemy[0].x > 102){
//		Enemy[0].vx = -1;
//	}
//		if( Enemy[0].x < 0){
//		Enemy[0].vx = 1;
//		}

		Enemy[1].vy = 1; // down
		Enemy[2].vy = 1; // down
		Enemy[3].vy = 1; // down
		Enemy[4].vy = 1; // down
	
		Enemy[5].vy = 1; // down
//		Enemy[5].vx = -1; // down

//		if( Enemy[5].x < 0){
//		Enemy[0].vx = 1;
//	}
//		if( Enemy[5].x > 102){
//		Enemy[0].vx = -1;
//		}

	
		for( i=7; i<18; i++){ // this will handle the rest of the 12 enemies
			Enemy[i].life = dead; 
		}
	

}
int16_t x1;
int16_t y1;
int16_t x2;
int16_t y2;

void GameMove (void){ int i; // this will only draw all enemies that are alive onto the screen
	Anyalive =0;
	time++;
	for(i=0; i<18; i++){
		if(Enemy[i].life == alive){
			Enemy[i].needDraw = 1;
			Anyalive = 1;
			
	// this will take care of the movement of the enemies		
			if(Enemy[i].y > 150){ // if hits bottom, dead // it was 140
				Enemy[i].life = dead; 
				Sound_Killed();
			} else {
				if(Enemy[i].y < 10){ // if hits top, dead
					Enemy[i].life = dead;
					Sound_Killed();
				} else {
					if(Enemy[i].x < 0){ // it hits left, dead
						Enemy[i].life = dead;
						Sound_Killed();
						
					} else {
						if(Enemy[i].x > 102){ // if hits right, dead
							Enemy[i].life = dead;
						Sound_Killed();
							
						} else {
							Enemy[i].x += Enemy[i].vx;
							if( (time%8) == 0){
								
							Enemy[i].y += Enemy[i].vy;
							}
						}
					}
				}
			}
		}
	}
	
	for( int i=0; i< NUMB; i++){
		if(bullets[i].life == alive){
			x1 = bullets[i].x + 4;
			y1 = bullets[i].y - 6;
			if((bullets[i].y < 4)||(bullets[i].y> 159)){
				bullets[i].life = dead;
			} else {
				bullets[i].y = bullets[i].y + bullets[i].vy;
			}
		}
	}
// this is for the collisions	
	for( int i=0; i<NUMB; i++){ 
		x2 = Enemy[i].x + 16;
		y2 = Enemy[i].y - 10;
		if( Enemy[i].life == alive){
			if((( (x1-x2)*(x1-x2))+((y1-y2)*(y1-y2))) < 20){
			
				Enemy[i].life = dead;
				bullets[i].life = dead;
				Sound_Killed();
				Enemy[i].image = BlackEnemy;
			}
		}
	}
}



void GameTask (void){
	// sound, slidepot, and buttons would be included here
	// do i just call all the functions here? // ask during OH
	Sound_Init();
	ADC_Init();
	Data = ADC_In(); // sample ADC for slide pot
	OldPosition = ((127-18)*Data)/4095; // from 0 to 127
	PortE_Init();
	
	GameMove();
	Flag=1;
}

uint32_t LastE;
uint32_t LastE2;
void ProcessInput(void){
	uint32_t Now;
	uint32_t Now2;
	Now = GPIO_PORTE_DATA_R &0x01; // PE0 // shoot button
	if( (Now==1)&&(LastE == 0)){
		bullets[0].life = alive;
		bullets[0].vy = -2;
		bullets[0].x = Player.x;
		bullets[0].y = 150;
		Sound_Shoot();
		if(( bullets[0].vy = -2)){
		ST7735_FillScreen(0x0000);            // set screen to black
		}
}
	LastE = Now;


	Now2 = GPIO_PORTE_DATA_R &0x02; // PE1 // message during game
	if((Now2==2)&&(LastE2 == 0)){
	ST7735_SetCursor(3, 1);
  ST7735_OutString("What Starts Here");
  ST7735_SetCursor(5, 2);
  ST7735_OutString("Changes the");
  ST7735_SetCursor(7, 3);
  ST7735_OutString("World!");
}
	LastE2 = Now2;

}

uint32_t pausenow;
void Timer1A_Handler(void){ // can be used to perform tasks in background
  TIMER1_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER1A timeout
	GameTask();
	Data = ADC_In();
	OldPosition = NewPosition;
	NewPosition = ((127-18)*Data)/4095; // from 0 to 127
	Player.x = NewPosition;
	ProcessInput();
	GameMove();

}
void GameDraw (void){ // drawing sprite on screen
	int32_t distance;
	distance = OldPosition - NewPosition;
	if(distance>2 || (distance< -2)){
		ST7735_DrawBitmap(OldPosition,159,BlackEnemy,18,8);
		ST7735_DrawBitmap(NewPosition,159,PlayerShip0,18,8);
	} else {
		ST7735_DrawBitmap(NewPosition,159,PlayerShip0,18,8);
	}

	for(int i=0; i<18; i++){
		if(Enemy[i].needDraw){
			if(Enemy[i].life == alive){ // if  current enemy sprite is alive, draw on screen 
				ST7735_DrawBitmap(Enemy[i].x,Enemy[i].y,
				Enemy[i].image, Enemy[i].w, Enemy[i].h);
			} else {
				ST7735_DrawBitmap(Enemy[i].x,Enemy[i].y,
				Enemy[i].black, Enemy[i].w, Enemy[i].h); // if enemy is dead, then draw black box
			}
			Enemy[i].needDraw = 0;
		}
	}
	
	for( int i=0; i<NUMB; i++){
	if(bullets[i].life == alive){
		//bullets[i].x = Player.x;
		ST7735_DrawBitmap(
		bullets[i].x,
		bullets[i].y,
		bullets[i].image,
		bullets[i].w, 
		bullets[i].h);
	//	bullets[i].x = Player.x;
	}

	
	if(bullets[i].life == dead){
		ST7735_DrawBitmap(
		bullets[i].x,
		bullets[i].y,
		bullets[i].bimage,
		bullets[i].w,
		bullets[i].h);
		bullets[i].life =dead;
}
}
	}

void Sound_Shooting_Init (void){ uint32_t ispressed;
	ispressed = GPIO_PORTE_DATA_R &0x01;
	while( ispressed == 1){
		Sound_Shoot();
}
	}

	

void Delay100ms (uint32_t count); 
const char Hello_English[] ="Hello";
const char Goodbye_English[]="Goodbye";
const char Language_English[]="English";

const char Hello_Spanish[] ="\xADHola!";
const char Goodbye_Spanish[]="Adi\xA2s";
const char Language_Spanish[]="Espa\xA4ol";

const char SpaceInvaders[] = "Weclome to";
const char SpaceInvaders1[] = "SpaceInvaders!";
const char SpaceInvaders2[] = "Bienvenidos a";

int main(void){
  DisableInterrupts();
  TExaS_Init(NONE);       // Bus clock is 80 MHz 
  Random_Init(1);
  Output_Init();
	GameInit();
	Bullet_Init();
	
	ST7735_SetCursor(8,2);
	ST7735_OutString((char *) Hello_English);
	ST7735_SetCursor(5,3);
	ST7735_OutString((char *) SpaceInvaders);
	ST7735_SetCursor(4,4);
	ST7735_OutString((char *) SpaceInvaders1);


	ST7735_SetCursor(8,7);
	ST7735_OutString((char *) Hello_Spanish);
	ST7735_SetCursor(5,8);
	ST7735_OutString((char *) SpaceInvaders2);
	ST7735_SetCursor(4,9);
	ST7735_OutString((char *) SpaceInvaders1);
	
	Delay100ms(20);
	
  ST7735_FillScreen(0x0000);            // set screen to black
	
//	ST7735_DrawBitmap(52, 159, PlayerShip0, 18,8);
//	ST7735_DrawBitmap(53, 151, Bunker0, 18,5);
	GameDraw();
	Timer1_Init(80000000/30, 5);//Timer1_Init(&GameTask, 80000000/30); // same issue as sound // ask during office hours //
	ADC_Init();
	Data = ADC_In(); // sample ADC for slide pot
	OldPosition = ((127-18)*Data)/4095; // from 0 to 127
	PortE_Init();
	Sound_Init();
	Sound_Shooting_Init();
	EnableInterrupts(); // ready to run
	do{ 
		while(Flag==0) {};
			Flag = 0;
			GameDraw();
		}
	while(Anyalive);


  ST7735_FillScreen(0x0000);   // set screen to black
  ST7735_SetCursor(6, 1);
  ST7735_OutString("GAME OVER");
  ST7735_SetCursor(6, 2);
  ST7735_OutString("Good Job,");
  ST7735_SetCursor(6, 3);
  ST7735_OutString("Earthling!");
  ST7735_SetCursor(5, 6);
	ST7735_OutString("Win or Lose,");
	ST7735_SetCursor(5, 7);
	ST7735_OutString("Everyone's a");
	ST7735_SetCursor(7, 8);
	ST7735_OutString("Winner!");
	ST7735_SetCursor(5, 11);
	ST7735_OutString("Score:");
	ST7735_SetCursor(11, 11);
	ST7735_OutUDec(100000);
  //while(1){
  //}

}
	
// You can't use this timer, it is here for starter code only 
// you must use interrupts to perform delays
void Delay100ms(uint32_t count){uint32_t volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
      time--;
    }
    count--;
  }
}




