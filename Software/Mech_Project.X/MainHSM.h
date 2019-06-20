/*
 * File: TemplateHSM.h
 * Author: J. Edward Carryer
 * Modified: Gabriel H Elkaim
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
 * Created on 23/Oct/2011
 * Updated on 16/Sep/2013
 */

#ifndef HSM_Main_H  // <- This should be changed to your own guard on both
#define HSM_Main_H  //    of these lines


/*******************************************************************************
 * PUBLIC #INCLUDES                                                            *
 ******************************************************************************/

#include "ES_Configure.h"   // defines ES_Event, INIT_EVENT, ENTRY_EVENT, and EXIT_EVENT

/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/
#define MAX_SPEED 100 //for motor functions

//#defines for robot speeds
#define FIND_TAPE_SPEED 40
#define ROBOT_SPEED 60  
#define SHIMMY_SPEED 40 //45
#define GET_ON_TAPE_SPEED 0 //-15
#define SCANNING_SPEED 30
#define PITCHING_SPEED 45 //45
#define SWEEP_SPEED 20 //15

//#defines for tape sensors    (BC(V6), TC(V5), TR(V4), TL(V3))
#define TL_TAPE 0b0001
#define TR_TAPE 0b0010
#define TC_TAPE 0b0100
#define BC_TAPE 0b1000

//#defines for beacon detectors 
#define MAIN_BEACON 0b10 //MAIN_BEACON
#define ANALOG_BEACON 0b01 //ANALOG_BEACON
#define ANALOG_LED 3

//#DEFINES for bumpers
#define LEFT_BUMP 10 
#define RIGHT_BUMP 01

//#defines for timers
#define CHECK_IN_BOUND_TIMER_TICK 725
#define GET_IN_BOUND_TIMER_TICK 500
#define REVERSE_TIMER_TICK 250 //175 //225
#define TANK_TURN_TICK 300 //300                                                             
#define AWAY_TOWARDS_TAPE_TICK 2500 //2s
#define MOVE_TO_CENTER_TICK 1150 //1250
#define SERVO_DELAY 500 //400
#define FACE_ENEMY_TICK 750 //450
#define SCAN_TIMER_TICK 2000
#define INITIAL_SCAN_TICK 1000
#define MOTOR_SPIN_UP_TIME 3000
#define RAISE_CANNON_TIME 8000
#define MOVE_A_LITTLE_BIT_TICK 200

//robot directions 
#define CCW 0
#define CW 1

#define LED_MASK 1 //use to count corners

/*******************************************************************************
 * PUBLIC TYPEDEFS                                                             *
 ******************************************************************************/
extern int IFZ_FLAG; //use global variable for getting into in IFZ state machine
extern int OBSTACLE_CLEARED;  //signal to get out of obstacle detected state machine
extern int ROBOT_DIRECTION; //CCW OR CW

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES                                                  *
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
uint8_t InitMainHSM(uint8_t Priority);


/**
 * @Function PostTemplateHSM(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be posted to queue
 * @return TRUE or FALSE
 * @brief This function is a wrapper to the queue posting function, and its name
 *        will be used inside ES_Configure to point to which queue events should
 *        be posted to. Remember to rename to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t PostMainHSM(ES_Event ThisEvent);




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
ES_Event RunMainHSM(ES_Event ThisEvent);

#endif /* HSM_Template_H */

