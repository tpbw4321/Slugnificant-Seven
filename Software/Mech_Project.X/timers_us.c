/*
 * File:   timers.c
 * Author: mdunne
 *
 * Created on November 15, 2011, 9:53 AM
 */

#include <xc.h>
#include <BOARD.h>
#include <peripheral/timer.h>
#include "timers_us.h"


/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/
//#define TIMERS_TEST

#define F_PB (BOARD_GetPBClock())
#define TIMER_FREQUENCY 1000

/*******************************************************************************
 * PRIVATE VARIABLES                                                           *
 ******************************************************************************/

static unsigned int milliSecondCount;
static unsigned int microSecondCount;

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                           *
 ******************************************************************************/

/**
 * @Function TIMERS_Init(void)
 * @param none
 * @return None.
 * @brief  Initializes the timer module
 * @author Max Dunne */
void TIMERS_us_Init(void) {
    OpenTimer5(T5_ON | T5_SOURCE_INT | T5_PS_1_8, F_PB / 8 / TIMER_FREQUENCY);
    INTClearFlag(INT_T5);
    INTSetVectorPriority(INT_TIMER_5_VECTOR, 3);
    INTSetVectorSubPriority(INT_TIMER_5_VECTOR, 3);
    INTEnable(INT_T5, INT_ENABLED);
}

/**
 * Function: TIMERS_GetMilliSeconds
 * @param None
 * @return the current MilliSecond Count
 * @author Max Dunne
 */
unsigned int TIMERS_us_GetMilliSeconds(void) {
    return milliSecondCount;
}

/**
 * Function: TIMERS_GetMicroSeconds
 * @param None
 * @return the current MicroSecond Count, it will roll over in 1.9 hours
 * @author Max Dunne
 */

unsigned int TIMERS_us_GetMicroSeconds(void) {
    return (microSecondCount + TMR5 / 5);
}

/**
 * @Function Timer5IntHandler(void)
 * @param None.
 * @return None.
 * @brief  This is the interrupt handler to support the timer module. It will increment 
 * time
 * @author Max Dunne */

void __ISR(_TIMER_5_VECTOR, ipl3auto) Timer5IntHandler_us(void) {
    INTClearFlag(INT_T5);
    milliSecondCount++;
    microSecondCount+=1000;
}




#ifdef TIMERS_TEST
#include "serial.h"
#include "timers.h"
#include <stdio.h>
//#include <plib.h>

int main(void) {
    int i = 0;
    BOARD_Init();
    int curMilliSeconds;
    int curMicroSeconds;
    printf("Welcome to The Timers Test, Module will Init and then print times, get a stopwatch and compare\r\n");
    while (!IsTransmitEmpty());

    TIMERS_Init();
//    while (1) {
//        if (IsTransmitEmpty()) {
//            printf("%d\r\n",TIMERS_GetMilliSeconds());
//        }
//    }
    while (1) {
        if (IsTransmitEmpty()) {
            curMicroSeconds = TIMERS_GetMicroSeconds();
            curMilliSeconds = TIMERS_GetMilliSeconds();
            printf("ms: %d\tus: %d\tus/1000: %d\tPR5: %d\tTMR5: %d\r\n",
                    curMilliSeconds, curMicroSeconds, curMicroSeconds / 1000,
                    PR5, TMR5);
        }
    }



    while (1);
}

#endif