/*
 * File: TemplateSubHSM.c
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
#include "MainHSM.h"
#include "IFZNotReachedSubHSM.h"
#include "TLTapeDetectedSubHSM.h"
#include "ObstacleDetectedSubHSM.h"
#include <stdio.h>

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
typedef enum {
    InitPSubState,
    REVERSE,
    RIGHT_TANK_TURN,
    MOVE_AWAY_TAPE,
    MOVE_A_BIT,
    BACK_ON_TAPE_CCW,
    BACK_ON_TAPE_CW,
    LEFT_TANK_TURN,

} TemplateSubHSMState_t;

static const char *StateNames[] = {
	"InitPSubState",
	"REVERSE",
	"RIGHT_TANK_TURN",
	"MOVE_AWAY_TAPE",
	"MOVE_A_BIT",
	"BACK_ON_TAPE_CCW",
	"BACK_ON_TAPE_CW",
	"LEFT_TANK_TURN",
};

//obstacle flag states
#define CLEARED 2
#define ALMOST_CLEARED 1

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this machine. They should be functions
   relevant to the behavior of this state machine */

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                            *
 ******************************************************************************/
/* You will need MyPriority and the state variable; you may need others as well.
 * The type of state variable should match that of enum in header file. */

static TemplateSubHSMState_t CurrentState = InitPSubState; // <- change name to match ENUM
static uint8_t MyPriority;
static int Middle_tape_cleared = 0; //counts number of right tank turns

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
uint8_t InitObstacleDetectedSubHSM(void) {
    ES_Event returnEvent;

    CurrentState = InitPSubState;
    returnEvent = RunObstacleDetectedSubHSM(INIT_EVENT);
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
ES_Event RunObstacleDetectedSubHSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE; // use to flag transition
    TemplateSubHSMState_t nextState; // <- change type to correct enum

    ES_Tattle(); // trace call stack

    switch (CurrentState) {
        case InitPSubState: // If current state is initial Psedudo State
            if (ThisEvent.EventType == ES_INIT)// only respond to ES_Init
            {
                // this is where you would put any actions associated with the
                // transition from the initial pseudo-state into the actual
                // initial state

                // now put the machine into the actual initial state
                nextState = REVERSE;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
            }
            break;
        case REVERSE:
            robotspeed(-ROBOT_SPEED, -ROBOT_SPEED);
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    ES_Timer_InitTimer(TURN_TIMER, REVERSE_TIMER_TICK);
                    break;
                case ES_NO_EVENT:
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == TURN_TIMER && ROBOT_DIRECTION == CCW) {
                        nextState = LEFT_TANK_TURN;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        Middle_tape_cleared = FALSE;
                    } else if (ThisEvent.EventParam == TURN_TIMER && ROBOT_DIRECTION == CW) {
                        nextState = RIGHT_TANK_TURN;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        Middle_tape_cleared = FALSE;
                    }
                    break;
                default: // all unhandled events pass the event back up to the next level
                    break;
            }
            break;

        case LEFT_TANK_TURN: // in the first state, replace this with correct names
            robotspeed(-ROBOT_SPEED, ROBOT_SPEED);
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    ES_Timer_InitTimer(TURN_TIMER, TANK_TURN_TICK);
                    break;
                case ES_NO_EVENT:
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == TURN_TIMER) { //go to the other side of the field
                        nextState = MOVE_AWAY_TAPE;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                default: // all unhandled events pass the event back up to the next level
                    break;
            }
            break;
        case RIGHT_TANK_TURN: // in the first state, replace this with correct names
            robotspeed(ROBOT_SPEED, -ROBOT_SPEED);
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case ES_ENTRY:
                    ES_Timer_InitTimer(TURN_TIMER, TANK_TURN_TICK);
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == TURN_TIMER) { //go to the other side of the field
                        nextState = MOVE_AWAY_TAPE;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                default: // all unhandled events pass the event back up to the next level
                    break;
            }
            break;
        case MOVE_AWAY_TAPE:
            robotspeed(FIND_TAPE_SPEED, FIND_TAPE_SPEED);
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case ES_ENTRY:
                    ES_Timer_InitTimer(TURN_TIMER, AWAY_TOWARDS_TAPE_TICK);
                    break;
                case ES_TIMEOUT:
                    Middle_tape_cleared = TRUE;
                    nextState = MOVE_AWAY_TAPE;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ON_TAPE:
                    if ((ThisEvent.EventParam & TR_TAPE) && (ThisEvent.EventParam & TC_TAPE) && (ThisEvent.EventParam & TR_TAPE) && Middle_tape_cleared == TRUE) { //RIGHT TURN 
                        nextState = MOVE_A_BIT;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        Middle_tape_cleared = FALSE;
                    } 
                    break;
                default: // all unhandled events pass the event back up to the next level
                    break;
            }
            break;
        case MOVE_A_BIT: // in the first state, replace this with correct names
            robotspeed(FIND_TAPE_SPEED, FIND_TAPE_SPEED);
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case ES_ENTRY:
                    ES_Timer_InitTimer(TURN_TIMER, MOVE_A_LITTLE_BIT_TICK);
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == TURN_TIMER && ROBOT_DIRECTION == CCW) {
                        nextState = BACK_ON_TAPE_CCW;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    } else if (ThisEvent.EventParam == TURN_TIMER && ROBOT_DIRECTION == CW) {
                        nextState = BACK_ON_TAPE_CW;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                default: // all unhandled events pass the event back up to the next level
                    break;
            }
            break;
        case BACK_ON_TAPE_CCW:
            robotspeed(FIND_TAPE_SPEED, -FIND_TAPE_SPEED); 
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case ON_TAPE:
                    if ((ThisEvent.EventParam & TC_TAPE) && (ThisEvent.EventParam & TL_TAPE)) {
                        nextState = REVERSE;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        OBSTACLE_CLEARED = TRUE; //get out of obstacle detected
                        ROBOT_DIRECTION ^= 1; //TOGGLE DIRECTION
                    }
                    break;
                default: // all unhandled events pass the event back up to the next level
                    break;
            }
            break;
        case BACK_ON_TAPE_CW:
            robotspeed(-FIND_TAPE_SPEED, FIND_TAPE_SPEED); //robotspeed(GET_ON_TAPE_SPEED, FIND_TAPE_SPEED);
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case ON_TAPE:
                    if ((ThisEvent.EventParam & TC_TAPE) && (ThisEvent.EventParam & TR_TAPE)) {
                        nextState = REVERSE;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        OBSTACLE_CLEARED = TRUE; //get out of obstacle detected
                        ROBOT_DIRECTION ^= 1; //TOGGLE DIRECTION
                    }
                    break;
                default: // all unhandled events pass the event back up to the next level
                    break;
            }
            break;
        default: // all unhandled states fall into here
            break;
    } // end switch on Current State

    if (makeTransition == TRUE) { // making a state transition, send EXIT and ENTRY
        // recursively call the current state with an exit event
        RunObstacleDetectedSubHSM(EXIT_EVENT); // <- rename to your own Run function
        CurrentState = nextState;
        RunObstacleDetectedSubHSM(ENTRY_EVENT); // <- rename to your own Run function
    }

    ES_Tail(); // trace call stack end
    return ThisEvent;
}


/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/
