/*
 * File:   TemplateEventChecker.c
 * Author: Gabriel Hugh Elkaim
 *
 * Template file to set up typical EventCheckers for the  Events and Services
 * Framework (ES_Framework) on the Uno32 for the CMPE-118/L class. Note that
 * this file will need to be modified to fit your exact needs, and most of the
 * names will have to be changed to match your code.
 *
 * This EventCheckers file will work with both FSM's and HSM's.
 *
 * Remember that EventCheckers should only return TRUE when an event has occured,
 * and that the event is a TRANSITION between two detectable differences. They
 * should also be atomic and run as fast as possible for good results.
 *
 * This file includes a test harness that will run the event detectors listed in the
 * ES_Configure file in the project, and will conditionally compile main if the macro
 * EVENTCHECKER_TEST is defined (either in the project or in the file). This will allow
 * you to check you event detectors in their own project, and then leave them untouched
 * for your project unless you need to alter their post functions.
 *
 * Created on September 27, 2013, 8:37 AM
 */

/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include "ES_Configure.h"
#include "TemplateEventChecker.h"
#include "ES_Events.h"
#include "serial.h"
#include "AD.h"
#include "pwm.h"
#include "IO_Ports.h"
#include "MainHSM.h"
#include <stdio.h>

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
#define BATTERY_DISCONNECT_THRESHOLD 175

//#defines for bumpers
#define READ_LEFT_BUMPER PORTZ11_BIT
#define READ_RIGHT_BUMPER PORTZ09_BIT
#define RIGHT_BUMP_MASK 1
#define LEFT_BUMP_MASK (1<<1) 
#define VALID_BUMP 0x07 //0111
#define VALID_UNBUMP 0x08 //1000
#define LO4 0x0F

//#defines for track wire
#define READ_TRACK_WIRE PORTZ03_BIT

//#defines for beacon
#define READ_Beacon PORTZ04_BIT
#define OBSTACLE_DETECTED (1 << 1)
#define BEACON_AVERAGE_MIN 0
#define BEACON_AVERAGE_MAX 10000 //200000
#define BEACON_DETECTED_THRESHOLD 1 //12500
#define NO_BEACON_DETECTED_THRESHOLD 1 //400

//#defines for analog beacon detector
#define READ_ANALOG_BEACON AD_ReadADPin(AD_PORTW5)
#define OBSTACLE_DETECTED_THRESHOLD 800 //700 
#define NO_OBSTACLE_DETECTED_THRESHOLD 700 //600
#define ANALOG_OBSTACLE_DETECTED 1

//#defines for ping sesnor
//#define PWM_PIN PWM_PORTX11
//#define PWM_25KHZ 250000 
//#define PWM_PIN PWM_PORTX11
#define ECHO_INPUT_PIN PORTW08_LAT     
#define READ_ECHO (IO_PortsReadPort(PORTW) & PIN8)
//#define NO_OBJECT_TIME 38 //38ms
#define LOW_threshold 10  // threshold used to determine when echo is high or low
#define HIGH_threshold 90 

/*******************************************************************************
 * EVENTCHECKER_TEST SPECIFIC CODE                                                             *
 ******************************************************************************/

//#define EVENTCHECKER_TEST
#ifdef EVENTCHECKER_TEST
#include <stdio.h>
#define SaveEvent(x) do {eventName=__func__; storedEvent=x;} while (0)

static const char *eventName;
static ES_Event storedEvent;
#endif

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this EventChecker. They should be functions
   relevant to the behavior of this particular event checker */
uint8_t ReadLeftBumper(void);
uint8_t ReadRightBumper(void);
uint8_t BumperStates(void);
uint8_t ReadBeaconDetector(void);
uint8_t Analog_beacon_detector(void);
uint8_t beacon_detector_state(void);

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                    *
 ******************************************************************************/

/* Any private module level variable that you might need for keeping track of
   events would be placed here. Private variables should be STATIC so that they
   are limited in scope to this module. */

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @Function TemplateCheckBattery(void)
 * @param none
 * @return TRUE or FALSE
 * @brief This function is a prototype event checker that checks the battery voltage
 *        against a fixed threshold (#defined in the .c file). Note that you need to
 *        keep track of previous history, and that the actual battery voltage is checked
 *        only once at the beginning of the function. The function will post an event
 *        of either BATTERY_CONNECTED or BATTERY_DISCONNECTED if the power switch is turned
 *        on or off with the USB cord plugged into the Uno32. Returns TRUE if there was an 
 *        event, FALSE otherwise.
 * @note Use this code as a template for your other event checkers, and modify as necessary.
 * @author Gabriel H Elkaim, 2013.09.27 09:18
 * @modified Gabriel H Elkaim/Max Dunne, 2016.09.12 20:08 */
uint8_t TemplateCheckBattery(void) {
    static ES_EventTyp_t lastEvent = BATTERY_DISCONNECTED;
    ES_EventTyp_t curEvent;
    ES_Event thisEvent;
    uint8_t returnVal = FALSE;
    uint16_t batVoltage = AD_ReadADPin(BAT_VOLTAGE); // read the battery voltage

    if (batVoltage > BATTERY_DISCONNECT_THRESHOLD) { // is battery connected?
        curEvent = BATTERY_CONNECTED;
    } else {
        curEvent = BATTERY_DISCONNECTED;
    }
    if (curEvent != lastEvent) { // check for change from last time
        thisEvent.EventType = curEvent;
        thisEvent.EventParam = batVoltage;
        returnVal = TRUE;
        lastEvent = curEvent; // update history
#ifndef EVENTCHECKER_TEST           // keep this as is for test harness
        //PostGenericService(thisEvent);
#else
        SaveEvent(thisEvent);
#endif   
    }
    return (returnVal);
}

uint8_t CheckBumpers(void) {
    static ES_EventTyp_t lastEvent = UNBUMPED;
    static uint16_t oldParam = 0;
    static ES_EventTyp_t curEvent = 0;
    ES_Event thisEvent;
    uint8_t returnVal = FALSE;
    uint8_t bumper_value = BumperStates();
    static uint8_t Leftbumper_memory = 0;
    static uint8_t Rightbumper_memory = 0;
    static uint8_t debounced_bumper_state = 0;


    //printf("\r\n%d\r\n", bumper_value);
    //Build Memories
    Leftbumper_memory <<= 1; //shift each time we enter the checkbumper function.
    if ((bumper_value & LEFT_BUMP_MASK)) { //if the bumper value is a 1, add 1. Otherwise the shift puts a 0.
        Leftbumper_memory += 1;
    }
    Rightbumper_memory <<= 1;
    if ((bumper_value & RIGHT_BUMP_MASK)) {
        Rightbumper_memory += 1;
    }
    //Integrate memories into debounced_bumper_state:: LB,RB

    if ((LO4 & Rightbumper_memory) == VALID_BUMP) { //set high
        debounced_bumper_state |= RIGHT_BUMP_MASK;
    } else if ((LO4 & Rightbumper_memory) == VALID_UNBUMP) { //set low
        debounced_bumper_state &= (~RIGHT_BUMP_MASK);
    }

    if ((LO4 & Leftbumper_memory) == VALID_BUMP) {
        debounced_bumper_state |= LEFT_BUMP_MASK;
    } else if ((LO4 & Leftbumper_memory) == VALID_UNBUMP) {
        debounced_bumper_state &= (~LEFT_BUMP_MASK);
    }

    if (debounced_bumper_state > oldParam) { // is bumper pressed?
        curEvent = BUMPED;
    } else if (debounced_bumper_state < oldParam) {
        curEvent = UNBUMPED;
    }

    if ((curEvent != lastEvent) || (oldParam != debounced_bumper_state)) { //check for change from last time    || (oldParam != bumper_value)
        thisEvent.EventType = curEvent;
        thisEvent.EventParam = debounced_bumper_state;
        returnVal = TRUE;
        lastEvent = curEvent; // update history
        oldParam = debounced_bumper_state; //DEBUG:: CHANGED FROM bumper_value
#ifndef EVENTCHECKER_TEST           // keep this as is for test harness
        PostMainHSM(thisEvent); //PostTemplateService
#else
        SaveEvent(thisEvent);
#endif   
    }
    return (returnVal);
}

uint8_t TrackWireEvent(void) {
    static ES_EventTyp_t lastEvent = NO_TRACK_WIRE_DETECTED;
    ES_EventTyp_t curEvent = 0;
    ES_Event thisEvent;
    uint8_t returnVal = FALSE;
    uint16_t track_wire_val = READ_TRACK_WIRE; // 

    if (track_wire_val) { // 
        curEvent = NO_TRACK_WIRE_DETECTED;
    } else {
        curEvent = TRACK_WIRE_DETECTED;
    }
    if (curEvent != lastEvent) { // check for change from last time
        thisEvent.EventType = curEvent;
        thisEvent.EventParam = track_wire_val;
        returnVal = TRUE;
        lastEvent = curEvent; // update history
#ifndef EVENTCHECKER_TEST           // keep this as is for test harness
        PostMainHSM(thisEvent);
#else
        SaveEvent(thisEvent);
#endif   
    }
    return (returnVal);
}

//uint8_t BeaconEvent(void) {
//    static ES_EventTyp_t lastEvent = NO_BEACON_DETECTED;
//    static int beaconSum = 0;
//    ES_EventTyp_t curEvent = 0;
//    ES_Event thisEvent;
//    uint8_t returnVal = FALSE;
//    
//    int i;
//
//    printf("Analog Beacon Value: %d\r\n", READ_ANALOG_BEACON);
//    for (i = 0; i<10000; i++);
//    
//    uint16_t beacon_val = READ_Beacon;
//    if (beacon_val == 0)
//        beaconSum++;
//    else
//        beaconSum--;
//
//    if (beaconSum > BEACON_AVERAGE_MAX)
//        beaconSum = BEACON_AVERAGE_MAX;
//    if (beaconSum < 0)
//        beaconSum = BEACON_AVERAGE_MIN;
//
//    if (beaconSum < NO_BEACON_DETECTED_THRESHOLD) 
//        curEvent = NO_BEACON_DETECTED;
//    else if (beaconSum > BEACON_DETECTED_THRESHOLD) //&& Analog_beacon_detector() == TRUE_OBSTACLE
//        curEvent = BEACON_DETECTED;
////    else if (beaconSum > BEACON_DETECTED_THRESHOLD && Analog_beacon_detector() == FALSE_OBSTACLE)
////        curEvent = NO_BEACON_DETECTED_THRESHOLD;
//    else
//        curEvent = ES_NO_EVENT;
//
////    if (beaconSum > BEACON_DETECTED_THRESHOLD) 
////        curEvent = BEACON_DETECTED;
////    else if (beaconSum < NO_BEACON_DETECTED_THRESHOLD) 
////        curEvent = NO_BEACON_DETECTED;
////    else
////        curEvent = ES_NO_EVENT;
//
//    if (curEvent != lastEvent) { // check for change from last time
//        thisEvent.EventType = curEvent;
//        thisEvent.EventParam = beaconSum;
//        returnVal = TRUE;
//        lastEvent = curEvent; // update history
//#ifndef EVENTCHECKER_TEST           // keep this as is for test harness
//        PostMainHSM(thisEvent);
//#else
//        SaveEvent(thisEvent);
//#endif   
//    }
//    return (returnVal);
//}

uint8_t BeaconEvent(void) {
    static ES_EventTyp_t lastEvent = NO_BEACON_DETECTED;
    ES_EventTyp_t curEvent = 0;
    ES_Event thisEvent;
    uint8_t returnVal = FALSE;
    uint8_t beacon_val = beacon_detector_state();
    static uint8_t previous_beacon_val = 0;
    static int highest = 0;
    int test = 0;
    //Test code

    test = READ_ANALOG_BEACON;
    //printf("%d\r\n", test);

    if (beacon_val > previous_beacon_val)
        curEvent = BEACON_DETECTED;
    else if (beacon_val < previous_beacon_val)
        curEvent = NO_BEACON_DETECTED;

    if (curEvent != lastEvent) { // check for change from last time
        thisEvent.EventType = curEvent;
        thisEvent.EventParam = beacon_val;
        previous_beacon_val = beacon_val;
        returnVal = TRUE;
        lastEvent = curEvent; // update history
#ifndef EVENTCHECKER_TEST           // keep this as is for test harness
        PostMainHSM(thisEvent);
#else
        SaveEvent(thisEvent);
#endif   
    }
    return (returnVal);
}


//uint8_t Echo_Event(void) {
//    static ES_EventTyp_t lastEvent = ECHO_GOES_LOW;
//    ES_EventTyp_t curEvent;
//    ES_Event thisEvent;
//    uint8_t returnVal = FALSE;
//	static uint8_t old_echo_value = 0;
//    uint8_t current_echo_value =  READ_ECHO;
//    if (current_echo_value > old_echo_value) { 
//        curEvent = ECHO_GOES_HIGH;
//    } else if(current_echo_value < old_echo_value){
//        curEvent = ECHO_GOES_LOW;
//    } 
//    if (curEvent != lastEvent) { // check for change from last time
//        thisEvent.EventType = curEvent;
//        thisEvent.EventParam = current_echo_value;
//        returnVal = TRUE;
//        lastEvent = curEvent; // update history
//#ifndef EVENTCHECKER_TEST           // keep this as is for test harness
//        //MainHSM(thisEvent);
//#else
//        SaveEvent(thisEvent);
//#endif   
//    }
//    return (returnVal);
//}

/* 
 * The Test Harness for the event checkers is conditionally compiled using
 * the EVENTCHECKER_TEST macro (defined either in the file or at the project level).
 * No other main() can exist within the project.
 * 
 * It requires a valid ES_Configure.h file in the project with the correct events in 
 * the enum, and the correct list of event checkers in the EVENT_CHECK_LIST.
 * 
 * The test harness will run each of your event detectors identically to the way the
 * ES_Framework will call them, and if an event is detected it will print out the function
 * name, event, and event parameter. Use this to test your event checking code and
 * ensure that it is fully functional.
 * 
 * If you are locking up the output, most likely you are generating too many events.
 * Remember that events are detectable changes, not a state in itself.
 * 
 * Once you have fully tested your event checking code, you can leave it in its own
 * project and point to it from your other projects. If the EVENTCHECKER_TEST marco is
 * defined in the project, no changes are necessary for your event checkers to work
 * with your other projects.
 */
#ifdef EVENTCHECKER_TEST
#include <stdio.h>
static uint8_t(*EventList[])(void) = {EVENT_CHECK_LIST};

void PrintEvent(void);

void main(void) {
    BOARD_Init();
    /* user initialization code goes here */
    PORTZ03_TRIS = 1;
    PORTZ04_TRIS = 1;


    // Do not alter anything below this line
    int i;

    printf("\r\nEvent checking test harness for %s", __FILE__);

    while (1) {
        if (IsTransmitEmpty()) {
            for (i = 0; i< sizeof (EventList) >> 2; i++) {
                if (EventList[i]() == TRUE) {
                    PrintEvent();
                    break;
                }

            }
        }
    }
}

void PrintEvent(void) {
    printf("\r\nFunc: %s\tEvent: %s\tParam: 0x%X", eventName,
            EventNames[storedEvent.EventType], storedEvent.EventParam);
}
#endif

uint8_t ReadLeftBumper(void) { //DIGITAL INPUT IS INVERTED
    //    printf("\r\n%d\r\n", READ_LEFT_BUMPER);
    if (READ_LEFT_BUMPER)
        return 0;
    else
        return LEFT_BUMP_MASK;
}

uint8_t ReadRightBumper(void) {
    //printf("\r\n%d\r\n", READ_RIGHT_BUMPER);
    if (READ_RIGHT_BUMPER)
        return 0;
    else
        return RIGHT_BUMP_MASK;
}

uint8_t BumperStates(void) {
    return (ReadLeftBumper() | ReadRightBumper());
}

uint8_t ReadBeaconDetector(void) {
    if (READ_Beacon)
        return 0;
    else
        return OBSTACLE_DETECTED;
}

uint8_t Analog_beacon_detector(void) {
    if (READ_ANALOG_BEACON > OBSTACLE_DETECTED_THRESHOLD)
        return ANALOG_OBSTACLE_DETECTED;
    else if (READ_ANALOG_BEACON < NO_OBSTACLE_DETECTED_THRESHOLD)
        return 0;
}

uint8_t beacon_detector_state(void) { //(regular beacon, analog beacon detector)
    return (ReadBeaconDetector() | Analog_beacon_detector());
}
