/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2007 Embedded Artists AB
 *
 * Description:
 *    Main program for LPC2148 Education Board test program
 * TO JEST TEN
 *****************************************************************************/

#include "pre_emptive_os/api/osapi.h"
#include "pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <ea_init.h>
#include <lpc2xxx.h>
#include <consol.h>
#include "spi.h"
#include "rfid.h"
#include "buttons.h"
#include "led.h"
#include "pwm.h"

#define PROC1_STACK_SIZE 1024
#define PROC2_STACK_SIZE 1024
#define INIT_STACK_SIZE  400

static  tU32 KEYPIN_1  = (1 << 20);
static  tU32 KEYPIN_2  = (1 << 21);
static  tU32 KEYPIN_3  = (1 << 22);
static  tU32 KEYPIN_4  = (1 << 23);

static tU8 proc1Stack[PROC1_STACK_SIZE];
static tU8 initStack[INIT_STACK_SIZE];
static tU8 pid1;

static void proc1(void* arg);
static void initProc(void* arg);

volatile tU32 msClock;
volatile tU8 killProc1 = FALSE;

tU8 buttonOpen[3];
tU8 serNum[5];

void udelay(tU32 delayInUs) {
  /*
   * setup timer #1 for delay
   */
  T1TCR = 0x02;          //stop and reset timer
  T1PR  = 0x00;          //set prescaler to zero (PR + 1)
  T1MR0 = (((long)delayInUs-1) * (long)CORE_FREQ/1000) / 1000;
  T1IR  = 0xff;          //reset all interrrupt flags
  T1MCR = 0x04;          //stop timer on match
  T1TCR = 0x01;          //start timer

  //wait until delay time has elapsed
  while (T1TCR & 0x01)
    ;
}

void mdelay(tU32 delayInUs) {
  /*
   * setup timer #1 for delay
   */
  T1TCR = 0x02;          //stop and reset timer
  T1PR  = 0x00;          //set prescaler to zero (PR + 1)
  T1MR0 = ((long)delayInUs-1) * (long)CORE_FREQ/1000;
  T1IR  = 0xff;          //reset all interrrupt flags
  T1MCR = 0x04;          //stop timer on match
  T1TCR = 0x01;          //start timer

  //wait until delay time has elapsed
  while (T1TCR & 0x01)
    ;
}

void openDoor() {
    udelay(50);
    pwmON();
    ledON();
    mdelay(5000);
    pwmOFF();
    ledOFF();
    udelay(50);
}


int main(void) {
  tU8 error;
  tU8 pid;

  initPWM(0xe8);
  SPI_Master_Init();
  initLED();
  initRFID();

  osInit();
  osCreateProcess(initProc, initStack, INIT_STACK_SIZE, &pid, 1, NULL, &error);
  osStartProcess(pid, &error);
  
  osStart();
  return 0;
}

static void proc1(void* arg) {
	for(;;){
		if (checkButton(KEYPIN_1)) {
	    	buttonOpen[0] = 1;
	    	osSleep(50);
	    }
		if (buttonOpen[0]) {
	    	if(checkButton(KEYPIN_3)){
	    		buttonOpen[1] = 1;
	    		osSleep(50);

	    		buttonOpen[0] = 0;
	    	}
	    	if (checkButton(KEYPIN_2) || checkButton(KEYPIN_1) || checkButton(KEYPIN_4)) {
	    		buttonOpen[0] = 0;
	    	}
	    }
	    if (buttonOpen[1]) {
	    	if (checkButton(KEYPIN_2)) {
	    		buttonOpen[0] = 0;
	    		buttonOpen[1] = 0;
	    		openDoor();
	    		osSleep(50);
	    	}
	    	if (checkButton(KEYPIN_1) || checkButton(KEYPIN_3) || checkButton(KEYPIN_4)) {
	    		buttonOpen[0] = 0;
	    		buttonOpen[1] = 0;
	    	}
	    }

	    if (isCard()) {
	    	if (readCardSerial()==1){
	    		if(checkIfValidTag()){
	    			openDoor();
	    			printf("JEST OK ");
	    		}
	    	}
	    }
	}

}

static void initProc(void* arg) {
  tU8 error;

  eaInit();   //initialize printf
  osCreateProcess(proc1, proc1Stack, PROC1_STACK_SIZE, &pid1, 3, NULL, &error);
  osStartProcess(pid1, &error);
  osDeleteProcess();
}

void appTick(tU32 elapsedTime) {
  msClock += elapsedTime;
}
