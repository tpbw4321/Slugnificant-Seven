/*
 * File: TemplateService.h
 * Author: J. Edward Carryer
 * Modified: Gabriel H Elkaim
 *
 * Template file to set up a simple service to work with the Events and Services
 * Framework (ES_Framework) on the Uno32 for the CMPE-118/L class. Note that this file
 * will need to be modified to fit your exact needs, and most of the names will have
 * to be changed to match your code.
 *
 * This is provided as an example and a good place to start.
 *
 * Created on 23/Oct/2011
 * Updated on 13/Nov/2013
 */

/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include "BOARD.h"
#include "AD.h"
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "TemplateEventChecker.h"
#include "TapeSensorService.h"
#include "timers.h"
#include <stdio.h>
#include "IO_Ports.h"
#include <xc.h>

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/

#define BATTERY_DISCONNECT_THRESHOLD 175

//#defines for tape sesnor timer
#define TWO_MS_TICK 2
//#define DEBOUNCE_TICK 5

//#defines for sensors
#define TAPE_SENSOR_COUNT 4
#define DARK 0
#define LIGHT 1
#define ON_TAPE_THRESHOLD 600 //has to be lower than this to be on tape      
#define OFF_TAPE_THRESHOLD 900 //has to greater than this to be off tape 
//top middle tape and bottom center gets around 500 when on tape???

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this machine. They should be functions
   relevant to the behavior of this state machine */
int TurnOnLED(void);
int TurnOffLED(void);
void TakeSample(void);
void ProcessAllSamples(void);

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                    *
 ******************************************************************************/
static uint8_t PrevTapeSensorStates = 0; //stores old states of tape sensors
static uint8_t TapeSensorStates = 0;
static uint16_t LightSample[TAPE_SENSOR_COUNT] = {0};
static uint16_t DarkSample[TAPE_SENSOR_COUNT] = {0};

/* You will need MyPriority and maybe a state variable; you may need others
 * as well. */
typedef enum {
    TAKE_LIGHT_SAMPLE,
    TAKE_DARK_SAMPLE,
    PROCESS_SAMPLE,
} TAPE_FSM;

//static FSM TAPE_FSM = TAKE_LIGHT_SAMPLE;
static uint8_t MyPriority;

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @Function InitTemplateService(uint8_t Priority)
 * @param Priority - internal variable to track which event queue to use
 * @return TRUE or FALSE
 * @brief This will get called by the framework at the beginning of the code
 *        execution. It will post an ES_INIT event to the appropriate event
 *        queue, which will be handled inside RunTemplateService function. Remember
 *        to rename this to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t InitTapeSensorService(uint8_t Priority) {
    ES_Event ThisEvent;

    MyPriority = Priority;

    // in here you write your initialization code
    // this includes all hardware and software initialization
    // that needs to occur.
    if (!IO_PortsSetPortOutputs(PORTY, PIN3))
        printf("Issues with setting PORTY Emitter Pins \r\n");
    //Set up pins for the detectors

    if (!AD_AddPins(AD_PORTV3 | AD_PORTV4 | AD_PORTV5 | AD_PORTV6 | AD_PORTW5))// | AD_PORTV3 | AD_PORTV4 | AD_PORTV5 | AD_PORTV6 | AD_PORTV7 | AD_PORTV8
        printf("Issues with adding PORTV Detector Pins\r\n");

    ES_Timer_InitTimer(TWO_MS_TIMER, TWO_MS_TICK);
    TurnOnLED();
    //ES_Timer_InitTimer(DEBOUNCE_TIMER, DEBOUNCE_TICK);

    // post the initial transition event
    ThisEvent.EventType = ES_INIT;
    if (ES_PostToService(MyPriority, ThisEvent) == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * @Function PostTemplateService(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be posted to queue
 * @return TRUE or FALSE
 * @brief This function is a wrapper to the queue posting function, and its name
 *        will be used inside ES_Configure to point to which queue events should
 *        be posted to. Remember to rename to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t PostTapeSensorService(ES_Event ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

/**
 * @Function RunTemplateService(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be responded.
 * @return Event - return event (type and param), in general should be ES_NO_EVENT
 * @brief This function is where you implement the whole of the service,
 *        as this is called any time a new event is passed to the event queue. 
 * @note Remember to rename to something appropriate.
 *       Returns ES_NO_EVENT if the event have been "consumed." 
 * @author J. Edward Carryer, 2011.10.23 19:25 */
ES_Event RunTapeSensorService(ES_Event ThisEvent) {
    ES_Event ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

    /********************************************
     in here you write your service code
     *******************************************/
    static ES_EventTyp_t lastEvent = ON_TAPE;
    ES_EventTyp_t curEvent = ES_NO_EVENT;
    static TAPE_FSM TAPE_SYNC = TAKE_LIGHT_SAMPLE;

    switch (ThisEvent.EventType) {
        case ES_INIT:
            // No hardware initialization or single time setups, those
            // go in the init function above.
            //
            // This section is used to reset service for some reason
            break;

        case ES_TIMEOUT:
//            printf("hello\r\n");
            if (ThisEvent.EventParam == TWO_MS_TIMER) {
//printf("hello\r\n");
                switch (TAPE_SYNC) {
                    case TAKE_LIGHT_SAMPLE:
                        //printf("Take Light Sample\r\n");
                        TakeSample();
                        TurnOffLED();
                        TAPE_SYNC = TAKE_DARK_SAMPLE;
                        break;
                    case TAKE_DARK_SAMPLE:
                        // printf("Take Dark Sample\r\n");
                        TakeSample();
                        TAPE_SYNC = PROCESS_SAMPLE;
                        break;
                    case PROCESS_SAMPLE:
                        // printf("Process Sample\r\n");
                        ProcessAllSamples();
                        if (TapeSensorStates > PrevTapeSensorStates) {
                            curEvent = ON_TAPE;
                        } else if (TapeSensorStates < PrevTapeSensorStates) {
                            curEvent = OFF_TAPE;
                        }
                        TurnOnLED();
                        TAPE_SYNC = TAKE_LIGHT_SAMPLE;
                        break;
                }
                ES_Timer_InitTimer(TWO_MS_TIMER, TWO_MS_TICK);
            } 
            if (curEvent != lastEvent) { //check for event
                ReturnEvent.EventType = curEvent;
                if ((curEvent == ON_TAPE) || (curEvent == OFF_TAPE)) {
                    ReturnEvent.EventParam = TapeSensorStates;
                    PrevTapeSensorStates = TapeSensorStates;
                }
                lastEvent = curEvent; // update history	
#ifndef SIMPLESERVICE_TEST           // keep this as is for test harness
                PostMainHSM(ReturnEvent);
#else
                PostTapeSensorService(ReturnEvent);
#endif   
            }
            break;
#ifdef SIMPLESERVICE_TEST     // keep this as is for test harness      
        default:
            if (ThisEvent.EventType == ON_TAPE || ThisEvent.EventType == OFF_TAPE) { //ThisEvent.EventType == ON_TAPE || ThisEvent.EventType == OFF_TAPE
                printf("\r\nEvent: %s\tParam: 0x%X",
                        EventNames[ThisEvent.EventType], ThisEvent.EventParam);
            }
            //printf("%d\r\n", AD_ReadADPin(BAT_VOLTAGE));
            break;
#endif
    }
    return ReturnEvent;
}

/*******************************************************************************
 * PRIVATE FUNCTIONs                                                           *
 ******************************************************************************/
int TurnOnLED(void) {
    //printf("Hello\r\n");
    IO_PortsSetPortBits(PORTY, PIN3);
}

int TurnOffLED(void) {
    //printf("Hello\r\n");
    IO_PortsClearPortBits(PORTY, PIN3);
}

void TakeSample(void) {
    //printf("Hello\r\n");
    static uint8_t state = LIGHT;
    int i = 0;

    if (state == DARK) {
        for (i = 0; i < TAPE_SENSOR_COUNT; i++) {
            DarkSample[i] = AD_ReadADPin(AD_PORTV3 << i); // (BC(V6), TC(V5), TR(V4), TL(V3))
        }
        //printf("darksmaple: %d\r\n", DarkSample[0]);
        state = LIGHT;
    } else if (state == LIGHT) {
        for (i = 0; i < TAPE_SENSOR_COUNT; i++) {
            LightSample[i] = AD_ReadADPin(AD_PORTV3 << i);
        }
        //printf("lightsmaple: %d\r\n", LightSample[0]);
        state = DARK;
    }
}

void ProcessAllSamples(void) {
    //printf("Hello\r\n");
    int i = 0;
    uint16_t difference = 0;
    uint16_t mask = 0;

    for (i = 0; i < TAPE_SENSOR_COUNT; i++) {
        difference = LightSample[i] - DarkSample[i];
        //                if (i == 2) {
        //                    printf("\r\n AD Value: %d\r\n", difference);
        //                }
        mask = (1 << i);
        if (difference < ON_TAPE_THRESHOLD) {
            TapeSensorStates |= mask; //SET CURRENT BIT HIGH
        } else if (difference > OFF_TAPE_THRESHOLD) {
            TapeSensorStates &= ~(mask); //SET CURRENT BIT LOW
        } else {
            TapeSensorStates |= (mask & PrevTapeSensorStates); //set to bit same as PrevTapeSensorStates
        }
    }
}
