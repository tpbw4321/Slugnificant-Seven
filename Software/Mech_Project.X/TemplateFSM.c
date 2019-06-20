/*
 * File: TemplateFSM.c
 * Author: J. Edward Carryer
 * Modified: Gabriel H Elkaim
 *
 * Template file to set up a Flat State Machine to work with the Events and Services
 * Frameword (ES_Framework) on the Uno32 for the CMPE-118/L class. Note that this file
 * will need to be modified to fit your exact needs, and most of the names will have
 * to be changed to match your code.
 *
 * This is provided as an example and a good place to start.
 *
 *Generally you will just be modifying the statenames and the run function
 *However make sure you do a find and replace to convert every instance of
 *  "Template" to your current state machine's name
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

#include "TemplateFSM.h"
#include <BOARD.h>
//Uncomment these for the Roaches
//#include "roach.h"
//#include "RoachFrameworkEvents.h"
#include <stdio.h>
#include "AD.h"
#include "IO_Ports.h"

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
#define TWO_MS_TICK 2
#define TAPE_SENSOR_COUNT 6
#define DARK 0
#define LIGHT 1
#define ON_TAPE_THRESHOLD 800 //has to be greater than this to be on tape 2.6V
#define OFF_TAPE_THRESHOLD 750 //has to less then this to be off tape 2.4V

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this machine. They should be functions
   relevant to the behavior of this state machine.*/
int TurnOnLED(void);
int TurnOffLED(void);
void TakeSample(void);
void ProcessAllSamples(void);

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                            *
 ******************************************************************************/
static uint8_t PrevTapeSensorStates = 0; //stores old states of tape sensors
static ES_EventTyp_t lastEvent = ON_TAPE;
static uint8_t TapeSensorStates = 0;
//static uint8_t tape_senor_val;
static uint16_t LightSample[TAPE_SENSOR_COUNT] = {0};
static uint16_t DarkSample[TAPE_SENSOR_COUNT] = {0};

/* You will need MyPriority and the state variable; you may need others as well.
 * The type of state variable should match that of enum in header file. */

typedef enum {
    InitPState,
    TAKE_LIGHT_SAMPLE,
    TAKE_DARK_SAMPLE,
    PROCESS_SAMPLE,
} TemplateFSMState_t;

static const char *StateNames[] = {
	"InitPState",
	"TAKE_LIGHT_SAMPLE",
	"TAKE_DARK_SAMPLE",
	"PROCESS_SAMPLE",
};


static TemplateFSMState_t CurrentState = InitPState; // <- change enum name to match ENUM
static uint8_t MyPriority;


/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @Function InitTemplateFSM(uint8_t Priority)
 * @param Priority - internal variable to track which event queue to use
 * @return TRUE or FALSE
 * @brief This will get called by the framework at the beginning of the code
 *        execution. It will post an ES_INIT event to the appropriate event
 *        queue, which will be handled inside RunTemplateFSM function. Remember
 *        to rename this to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t InitTemplateFSM(uint8_t Priority) {
    //Set ports for Digital Output
    //PORTX will be dedicated output for emitters
    if (!IO_PortsSetPortOutputs(PORTX, PIN3))
        printf("Issues with setting PORTX Emitter Pins \r\n");

    //Set up pins for the detectors
    if (!AD_AddPins(AD_PORTV3 | AD_PORTV4 | AD_PORTV5 | AD_PORTV6 | AD_PORTV7))
        printf("Issues with adding PORTV Detector Pins\r\n");

    MyPriority = Priority;
    // put us into the Initial PseudoState
    CurrentState = InitPState;
    // post the initial transition event
    if (ES_PostToService(MyPriority, INIT_EVENT) == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * @Function PostTemplateFSM(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be posted to queue
 * @return TRUE or FALSE
 * @brief This function is a wrapper to the queue posting function, and its name
 *        will be used inside ES_Configure to point to which queue events should
 *        be posted to. Remember to rename to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t PostTemplateFSM(ES_Event ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

/**
 * @Function RunTemplateFSM(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be responded.
 * @return Event - return event (type and param), in general should be ES_NO_EVENT
 * @brief This function is where you implement the whole of the flat state machine,
 *        as this is called any time a new event is passed to the event queue. This
 *        function will be called recursively to implement the correct order for a
 *        state transition to be: exit current state -> enter next state using the
 *        ES_EXIT and ES_ENTRY events.
 * @note Remember to rename to something appropriate.
 *       Returns ES_NO_EVENT if the event have been "consumed."
 * @author J. Edward Carryer, 2011.10.23 19:25 */
ES_Event RunTemplateFSM(ES_Event ThisEvent) {
    ES_Event ReturnEvent;
    ES_EventTyp_t curEvent = ES_NO_EVENT;
    uint8_t makeTransition = FALSE; // use to flag transition
    TemplateFSMState_t nextState; // <- need to change enum type here

    ES_Tattle(); // trace call stack

    switch (CurrentState) {
        case InitPState: // If current state is initial Psedudo State
            if (ThisEvent.EventType == ES_INIT)// only respond to ES_Init
            {
                // this is where you would put any actions associated with the
                // transition from the initial pseudo-state into the actual
                // initial state


                // now put the machine into the actual initial state
                nextState = TAKE_LIGHT_SAMPLE;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
            }
            break;

        case TAKE_LIGHT_SAMPLE: // in the first state, replace this with appropriate state
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    TurnOnLED();
                    ES_Timer_InitTimer(TWO_MS_TIMER, TWO_MS_TICK);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_EXIT:
                    TakeSample();
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    nextState = TAKE_DARK_SAMPLE;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                default: // all unhandled events pass the event back up to the next level
                    break;
            }
            break;
        case TAKE_DARK_SAMPLE:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    TurnOffLED();
                    ES_Timer_InitTimer(TWO_MS_TIMER, TWO_MS_TICK);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_EXIT:
                    TakeSample();
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    nextState = PROCESS_SAMPLE;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                default: // all unhandled events pass the event back up to the next level
                    break;
            }
            break;
        case PROCESS_SAMPLE:
            ProcessAllSamples(); // TL(V3), TR(V4), TC(V5), BL(V6), BR(V7), BC(V8) 
            if (TapeSensorStates > PrevTapeSensorStates) {
                curEvent = OFF_TAPE;
            } else if (TapeSensorStates < PrevTapeSensorStates) {
                curEvent = ON_TAPE;
            }
            if (curEvent != lastEvent) { //check for event
                ReturnEvent.EventType = curEvent;
                ReturnEvent.EventParam = TapeSensorStates;
                lastEvent = curEvent; // update history	
                PrevTapeSensorStates = TapeSensorStates;
                //PostMainStateMachineHSM(ReturnEvent);
            }
            nextState = TAKE_LIGHT_SAMPLE;
            makeTransition = TRUE;
            ThisEvent.EventType = ES_NO_EVENT;
            break;
        default: // all unhandled states fall into here
            break;
    } // end switch on Current State
    if (makeTransition == TRUE) { // making a state transition, send EXIT and ENTRY
        // recursively call the current state with an exit event
        RunTemplateFSM(EXIT_EVENT);
        CurrentState = nextState;
        RunTemplateFSM(ENTRY_EVENT);
    }
    ES_Tail(); // trace call stack end
    return ThisEvent;
}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/
int TurnOnLED(void) {
    IO_PortsWritePort(PORTX, PIN3);
}

int TurnOffLED(void) {
    IO_PortsWritePort(PORTX, 0);
}

void TakeSample(void) {
    static uint8_t state = LIGHT;
    int i = 0;

    if (state == DARK)
        for (i = 0; i < TAPE_SENSOR_COUNT; i++)
            DarkSample[i] = AD_ReadADPin(AD_PORTV3 << i); // TL(V3), TR(V4), TC(V5), BL(V6), BR(V7), BC(V8) 
    else
        for (i = 0; i < TAPE_SENSOR_COUNT; i++)
            LightSample[i] = AD_ReadADPin(AD_PORTV3 << i);
    state ^= 1; //toggle between light and dark sampling

}

void ProcessAllSamples(void) {
    int i = 0;
    //uint8_t TapeSensorStates = 0;
    uint16_t difference = 0;
    uint16_t mask;

    for (i = 0; i < TAPE_SENSOR_COUNT; i++) {
        difference = LightSample[i] - DarkSample[i];
        mask = 1 << i;
        if (difference < OFF_TAPE_THRESHOLD) {
            TapeSensorStates &= ~(mask); //SET CURRENT BIT LOW
        } else if (difference > ON_TAPE_THRESHOLD) {
            TapeSensorStates |= mask; //SET CURRENT BIT HIGH
        } else
            TapeSensorStates |= (mask & PrevTapeSensorStates); //???
    }
    //ThisEvent.EventParam = TapeSensorStates;
}
