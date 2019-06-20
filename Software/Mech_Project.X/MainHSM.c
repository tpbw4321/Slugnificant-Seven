/*
 * File: TemplateSubHSM.c
 * Author: J. Edward Carryer
 * Modified: Gabriel Elkaim and Soja-Marie Morgens
 *
 * Template file to set up a Heirarchical State Machine to work with the Events and
 * Services Framework (ES_Framework) on the Uno32 for the CMPE-118/L class. Note that
 * this file will need to be modified to fit your exact needs, and most of the names
 * will have to be changed to match your code.
 *
 * There is another template file for the SubHSM's that is slightly differet, and
 * should be used for all of the subordinate state machines (flat or heirarchical)
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
#include "IO_Ports.h"
#include "pwm.h"
#include "AD.h"
#include "LED.h"
#include "MainHSM.h"
#include "IFZNotReachedSubHSM.h" //#include all sub state machines called
#include "IFZReachedSubHSM.h"
/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/
//Include any defines you need to do
#define LEFT_MOTOR_PWM_PIN PWM_PORTY12
#define RIGHT_MOTOR_PWM_PIN PWM_PORTY10
#define LEFT_MOTOR_DIR_PIN PIN5 //PORTY
#define RIGHT_MOTOR_DIR_PIN PIN6 //PORTY
#define RIGHT_SHOOTER_MOTOR_PIN PWM_PORTZ06
#define LEFT_SHOOTER_MOTOR_PIN PWM_PORTY04

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/


typedef enum {
    InitPState,
    IFZ_not_reached,
    IFZ_reached,
} TemplateHSMState_t;

static const char *StateNames[] = {
	"InitPState",
	"IFZ_not_reached",
	"IFZ_reached",
};


/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this machine. They should be functions
   relevant to the behavior of this state machine
   Example: char RunAway(uint_8 seconds);*/
char leftmtrspeed(char newSpeed);
char rightmtrspeed(char newSpeed);
void robotspeed(char left_speed, char right_speed);
char leftballspeed(char newSpeed);
char rightballspeed(char newSpeed);
void launchspeed(char speed);

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                            *
 ******************************************************************************/
/* You will need MyPriority and the state variable; you may need others as well.
 * The type of state variable should match that of enum in header file. */

static TemplateHSMState_t CurrentState = InitPState; // <- change enum name to match ENUM
static uint8_t MyPriority;
int IFZ_FLAG = 0;
int OBSTACLE_CLEARED = 0;
int ROBOT_DIRECTION = CCW;

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @Function InitTemplateHSM(uint8_t Priority)
 * @param Priority - internal variable to track which event queue to use
 * @return TRUE or FALSE
 * @brief This will get called by the framework at the beginning of the code
 *        execution. It will post an ES_INIT event to the appropriate event
 *        queue, which will be handled inside RunTemplateFSM function. Remember
 *        to rename this to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t InitMainHSM(uint8_t Priority) {
    IO_PortsSetPortOutputs(PORTY, (LEFT_MOTOR_DIR_PIN | RIGHT_MOTOR_DIR_PIN)); //for direction of motors
    PWM_SetFrequency(PWM_1KHZ);
    PWM_AddPins(LEFT_MOTOR_PWM_PIN | RIGHT_MOTOR_PWM_PIN | LEFT_SHOOTER_MOTOR_PIN | RIGHT_SHOOTER_MOTOR_PIN );
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
 * @Function PostTemplateHSM(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be posted to queue
 * @return TRUE or FALSE
 * @brief This function is a wrapper to the queue posting function, and its name
 *        will be used inside ES_Configure to point to which queue events should
 *        be posted to. Remember to rename to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t PostMainHSM(ES_Event ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

/**
 * @Function RunTemplateHSM(ES_Event ThisEvent)
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
ES_Event RunMainHSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE; // use to flag transition
    TemplateHSMState_t nextState; // <- change type to correct enum

    ES_Tattle(); // trace call stack

    switch (CurrentState) {
        case InitPState: // If current state is initial Pseudo State
            if (ThisEvent.EventType == ES_INIT)// only respond to ES_Init
            {
                // this is where you would put any actions associated with the
                // transition from the initial pseudo-state into the actual
                // initial state
                // Initialize all sub-state machines
                Init_IFZNotReached_SubHSM();
                InitIFZReachedSubHSM();
                //other init 
                // now put the machine into the actual initial state
                //nextState = IFZ_not_reached;
                nextState = IFZ_reached;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
                ;
            }
            break;

        case IFZ_not_reached: // in the first state, replace this with correct names
            // run sub-state machine for this state
            //NOTE: the SubState Machine runs and responds to events before anything in the this
            //state machine does
            ThisEvent = Run_IFZNotReached_SubHSM(ThisEvent);
            if (IFZ_FLAG == 2) {
                nextState = IFZ_reached;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
                IFZ_FLAG = 0;
            }
            break;
        case IFZ_reached:
            ThisEvent = RunIFZReachedSubHSM(ThisEvent);
            break;
        default: // all unhandled states fall into here
            break;
    } // end switch on Current State

    if (makeTransition == TRUE) { // making a state transition, send EXIT and ENTRY
        // recursively call the current state with an exit event
        RunMainHSM(EXIT_EVENT); // <- rename to your own Run function
        CurrentState = nextState;
        RunMainHSM(ENTRY_EVENT); // <- rename to your own Run function
    }

    ES_Tail(); // trace call stack end
    return ThisEvent;
}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/
char leftmtrspeed(char newSpeed) {
    if ((newSpeed < -MAX_SPEED) || (newSpeed > MAX_SPEED)) {
        return (ERROR);
    }
    if (newSpeed < 0) {
        IO_PortsClearPortBits(PORTY, LEFT_MOTOR_DIR_PIN); //reverse direction = 0  
        newSpeed = newSpeed * (-1); // set speed to a positive value
    } else {
        IO_PortsSetPortBits(PORTY, LEFT_MOTOR_DIR_PIN); //  forward direction = 1
    }
    if (PWM_SetDutyCycle(LEFT_MOTOR_PWM_PIN, newSpeed * (MAX_PWM / MAX_SPEED)) == ERROR) { //setting speed
        return (ERROR);
    }
    return (SUCCESS);
}

char rightmtrspeed(char newSpeed) {
    if ((newSpeed < -MAX_SPEED) || (newSpeed > MAX_SPEED)) {
        return (ERROR);
    }
    if (newSpeed < 0) {
        IO_PortsSetPortBits(PORTY, RIGHT_MOTOR_DIR_PIN); //reverse direction = 0
        newSpeed = newSpeed * (-1); // set speed to a positive value
    } else {
        IO_PortsClearPortBits(PORTY, RIGHT_MOTOR_DIR_PIN); //  forward direction = 1
    }
    if (PWM_SetDutyCycle(RIGHT_MOTOR_PWM_PIN, newSpeed * (MAX_PWM / MAX_SPEED)) == ERROR) { //setting speed
        return (ERROR);
    }
    return (SUCCESS);
}

void robotspeed(char left_speed, char right_speed) {
    leftmtrspeed(left_speed);
    rightmtrspeed(right_speed);
}

//#define MOTOR_TEST
#ifdef MOTOR_TEST

#include <stdio.h>

#define MOTOR_TIME 5000000//10000000

int main(void) {
    BOARD_Init();
    AD_Init();
    TIMERS_Init();
    PWM_Init();
    IO_PortsSetPortOutputs(PORTZ, (PIN3 | PIN4)); //for direction of motors
    PWM_SetFrequency(PWM_1KHZ);
    PWM_AddPins(PWM_PORTY12 | PWM_PORTY10);
    int x;

    while (1) {
        //printf("Battery life: %d\r\n", AD_ReadADPin(BAT_VOLTAGE));
        //        if (leftmtrspeed(80) == ERROR) {
        //            printf("Error\r\n");
        //        } else {
        //            printf("Success\r\n");
        //        }
        //        for (x = 0; x < MOTOR_TIME; x++) {
        //
        //        }
        //        if (leftmtrspeed(-80) == ERROR) {
        //            printf("Error2\r\n");
        //        } else {
        //            printf("Success2\r\n");
        //        }
        //        for (x = 0; x < MOTOR_TIME; x++) {
        //
        //        }
        //        if (leftmtrspeed(0) == ERROR) {
        //            printf("Error2\r\n");
        //        } else {
        //            printf("Success2\r\n");
        //        }
        //        for (x = 0; x < MOTOR_TIME; x++) {
        //
        //        }     


        //        if (rightmtrspeed(80) == ERROR) {
        //            printf("Error\r\n");
        //        } else {
        //            printf("Success\r\n");
        //        }
        //        for (x = 0; x < MOTOR_TIME; x++) {
        //
        //        }
        //        if (rightmtrspeed(-80) == ERROR) {
        //            printf("Error2\r\n");
        //        } else {
        //            printf("Success2\r\n");
        //        }
        //        for (x = 0; x < MOTOR_TIME; x++) {
        //
        //        }
        //        if (rightmtrspeed(0) == ERROR) {
        //            printf("Error2\r\n");
        //        } else {
        //            printf("Success2\r\n");
        //        }
        //        for (x = 0; x < MOTOR_TIME; x++) {
        //
        //        }
        robotspeed(80, 80);
        for (x = 0; x < MOTOR_TIME; x++) {

        }
        robotspeed(80, 50);
        for (x = 0; x < MOTOR_TIME; x++) {

        }
        robotspeed(50, 80);
        for (x = 0; x < MOTOR_TIME; x++) {

        }
        robotspeed(-80, -80);
        for (x = 0; x < MOTOR_TIME; x++) {

        }
        robotspeed(0, 0);
        for (x = 0; x < MOTOR_TIME; x++) {

        }
    }

    while (1);
}

#endif