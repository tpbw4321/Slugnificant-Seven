/*
 * File: TapeSensorSubHSM.c
 * Author: J. Edward Carryer
 * Modified: Gabriel H Elkaim
 *
 * Template file to set up a Heirarchical State Machine to work with the Events and
 * Services Framework (ES_Framework) on the Uno32 for the CMPE-118/L class. Note that
 * this file will need to be modified to fit your exact needs, and most of the names
 * will have to be changed to match your code.
 *
 * There is for a substate machine. Make sure it has a unique name
 *
 * This is provided as an example and a good place to start.
 *
 * History
 * When           Who     What/Why
 * -------------- ---     --------
 * 09/13/13 15:17 ghe      added tattletail functionality and recursive calls
 * 01/15/12 11:12 jec      revisions for Gen2 framework
 * 11/07/11 11:26 jec      made the queue static
 * 10/30/11 17:59 jec      fixed references to CurrentEvent in RunTemplateSM()
 * 10/23/11 18:20 jec      began conversion from SMTemplate.c (02/20/07 rev)
 */


/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BOARD.h"
#include "TapeSensorSubHSM.h"
#include "IO_Ports.h"
#include "AD.h"
#include "timers.h"
#include "MainStateMachineHSM.h"
#include <stdio.h>

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
typedef enum {
    InitPSubState,
    SubFirstState,
    TakeDarkSample,
    TakeLightSample,
    ProcessSamples,
} TapeSensorSubHSMState_t;

static const char *StateNames[] = {
	"InitPSubState",
	"SubFirstState",
	"TakeDarkSample",
	"TakeLightSample",
	"ProcessSamples",
};

#define LED_EMITTERS PORTX03_BIT

#define TAPE_SENSOR_SERVICE_TIMER_TICKS 2
#define DARK 0
#define LIGHT 1
#define TAPE_SENSOR_COUNT 5
#define LO_THRESHOLD 0
#define HI_THRESHOLD 1
#define ON 1
#define OFF 0


/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this machine. They should be functions
   relevant to the behavior of this state machine */
uint8_t TurnOffDiodes(void);
uint8_t TurnOnDiodes(void);
void TakeSample(void);
void ProcessAllSamples(ES_Event ThisEvent);
/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                            *
 ******************************************************************************/
/* You will need MyPriority and the state variable; you may need others as well.
 * The type of state variable should match that of enum in header file. */

static TapeSensorSubHSMState_t CurrentState = InitPSubState; // <- change name to match ENUM
static uint16_t LightSample[TAPE_SENSOR_COUNT] = {0, 0, 0, 0};
static uint16_t DarkSample[TAPE_SENSOR_COUNT] = {0, 0, 0, 0};

static uint8_t MyPriority;
static uint8_t PrevTapeSensorStates = 0;


/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @Function InitTemplateSubHSM(uint8_t Priority)
 * @param Priority - internal variable to track which event queue to use
 * @return TRUE or FALSE
 * @brief This will get called by the framework at the beginning of the code
 *        execution. It will post an ES_INIT event to the appropriate event
 *        queue, which will be handled inside RunTemplateFSM function. Remember
 *        to rename this to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t InitTapeSensorSubHSM(void) {
    ES_Event returnEvent;

    CurrentState = InitPSubState;

    //Set ports for Digital Output
    //PORTX will be dedicated output for emitters
    if (!IO_PortsSetPortDirection(PORTX, 0))
        printf("Issues with setting PORTX Emitter Pins \r\n");

    //Set up pins for the detectors
    if (!AD_AddPins(AD_PORTV3 | AD_PORTV4 | AD_PORTV5 | AD_PORTV6 | AD_PORTV7))
        printf("Issues with adding PORTV Detector Pins\r\n");
    
    returnEvent = RunTapeSensorSubHSM(INIT_EVENT);
    if (returnEvent.EventType == ES_NO_EVENT) {
        return TRUE;
    }
    return FALSE;
}

/**
 * @Function RunTemplateSubHSM(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be responded.
 * @return Event - return event (type and param), in general should be ES_NO_EVENT
 * @brief This function is where you implement the whole of the heirarchical state
 *        machine, as this is called any time a new event is passed to the event
 *        queue. This function will be called recursively to implement the correct
 *        order for a state transition to be: exit current state -> enter next state
 *        using the ES_EXIT and ES_ENTRY events.
 * @note Remember to rename to something appropriate.
 *       The lower level state machines are run first, to see if the event is dealt
 *       with there rather than at the current level. ES_EXIT and ES_ENTRY events are
 *       not consumed as these need to pass pack to the higher level state machine.
 * @author J. Edward Carryer, 2011.10.23 19:25
 * @author Gabriel H Elkaim, 2011.10.23 19:25 */
ES_Event RunTapeSensorSubHSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE; // use to flag transition
    TapeSensorSubHSMState_t nextState; // <- change type to correct enum

    ES_Tattle(); // trace call stack

    switch (CurrentState) {
        case InitPSubState: // If current state is initial Psedudo State
            if (ThisEvent.EventType == ES_INIT)// only respond to ES_Init
            {
                // this is where you would put any actions associated with the
                // transition from the initial pseudo-state into the actual
                // initial state

                // now put the machine into the actual initial state
                nextState = SubFirstState;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
            }
            break;

        case TakeLightSample: // in the first state, replace this with correct names
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    TurnOnDiodes();
                    TIMERS_InitTimer(TAPE_SENSOR_SERVICE_TIMER, TAPE_SENSOR_SERVICE_TIMER_TICKS);
                case ES_EXIT:
                    TakeSample();
                    break;
                case ES_TIMEOUT:
                    nextState = TakeDarkSample;
                    makeTransition = TRUE;
                    break;
                default: // all unhandled events pass the event back up to the next level
                    break;
            }
            break;
        case TakeDarkSample:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    TurnOffDiodes();
                    TIMERS_InitTimer(TAPE_SENSOR_SERVICE_TIMER, TAPE_SENSOR_SERVICE_TIMER_TICKS);
                case ES_EXIT:
                    TakeSample();
                    break;
                case ES_TIMEOUT:
                    nextState = ProcessSamples;
                    makeTransition = TRUE;
                    break;
                default: // all unhandled events pass the event back up to the next level
                    break;
            }
            break;
        case ProcessSamples:
            //Processes samples and then checks to see if 
            //there is a difference between the last time
            //it checked the samples.
            ProcessAllSamples(ThisEvent);
            if (ThisEvent.EventParam != PrevTapeSensorStates) {
                PrevTapeSensorStates = ThisEvent.EventParam;
                ThisEvent.EventType = TAPE_SENSOR_TRIPPED;
                nextState = TakeLightSample;
                makeTransition = TRUE;
                PostMainStateMachineHSM(ThisEvent);
            }
            break;
        default: // all unhandled states fall into here
            break;
    } // end switch on Current State

    if (makeTransition == TRUE) { // making a state transition, send EXIT and ENTRY
        // recursively call the current state with an exit event
        RunTapeSensorSubHSM(EXIT_EVENT); // <- rename to your own Run function
        CurrentState = nextState;
        RunTapeSensorSubHSM(ENTRY_EVENT); // <- rename to your own Run function
    }

    ES_Tail(); // trace call stack end
    return ThisEvent;
}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/

/**
 * @Function TurnOnDiodes(void)
 * @param None
 * @return SUCCESS or ERROR
 * @brief This function is used to turn on the emitters
 * @author Barron Wong */
uint8_t TurnOnDiodes(void) {
    LED_EMITTERS = 1;
    if (LED_EMITTERS)
        return SUCCESS;
    return ERROR;
}

/**
 * @Function TurnOffDiodes(void)
 * @param None
 * @return SUCCESS or ERROR
 * @brief This function is used to turn off the emitters
 * @author Barron Wong */
uint8_t TurnOffDiodes(void) {
    LED_EMITTERS = 0;
    if (!LED_EMITTERS) {
        return SUCCESS;
    }
    return ERROR;
}

/**
 * @Function TakeSample(void)
 * @param None
 * @return NONE
 * @brief This function toggles between taking light or dark samples.
 * @author Barron Wong */
void TakeSample(void) {
    static uint8_t state = LIGHT;
    int i = 0;

    if (state == DARK)
        for (i = 0; i < TAPE_SENSOR_COUNT; i++)
            DarkSample[i] = AD_ReadADPin(AD_PORTV3 << i);
    else
        for (i = 0; i < TAPE_SENSOR_COUNT; i++)
            LightSample[i] = AD_ReadADPin(AD_PORTV3 << i);
    state ^= 1;

}

/**
 * @Function TakeSample(void)
 * @param None
 * @return NONE
 * @brief This function compares light samples to dark samples and updates
 * a uint8_t with the states of each sensor. It takes a look at the difference
 * in reading between each sensor. If it reaches a low threshold then the
 * state of the sensor will be OFF_TAPE, if it is above the high threshold
 * the state of the sensor will be ON_TAPE. If the reading is in the middle of the
 * hysteresis bound then it takes the previous tape sensor state.
 * @author Barron Wong */
void ProcessAllSamples(ES_Event ThisEvent) {
    int i = 0;
    uint8_t TapeSensorStates = 0;
    uint16_t difference = 0;
    uint16_t mask;

    for (i = 0; i < TAPE_SENSOR_COUNT; i++) {
        difference = LightSample[i] - DarkSample[i];
        mask = 1 << i;
        if (difference < LO_THRESHOLD) {
            TapeSensorStates &= ~(mask);
        } else if (difference > HI_THRESHOLD) {
            TapeSensorStates |= mask;
        } else
            TapeSensorStates |= (mask & PrevTapeSensorStates);
    }
    
    ThisEvent.EventParam = TapeSensorStates;
}