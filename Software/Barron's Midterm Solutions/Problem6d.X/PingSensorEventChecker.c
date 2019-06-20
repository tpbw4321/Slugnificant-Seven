/*
 * File:   PingSensorEventChecker.c
 * Author: Barron Wong
 *
 * This file is created for MIDTERM Problem 6d
 *
 */

/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include "ES_Configure.h"
#include "PingSensorEventChecker.h"
#include "ES_Events.h"
#include "serial.h"
#include "AD.h"
#include "timers.h"
#include "xc.h"
#include <stdio.h>
#include "PingSensorFSM.h"
/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
#define BATTERY_DISCONNECT_THRESHOLD 175
#define TIMER1_US_PER_TICK 25 //25us
#define ReadTimer1() TIMERS_GetTime()

#define ECHO  PORTFbits.RF5 //Pin40
#define TRIG  PORTBbits.RB1 //Pin41

#define PING_SENSOR_TIMER_TICKS 1 //20ms
#define MILI_TO_MICRO_SCALAR 1000
#define INCHES_CONVERSION_RATE 148.0
#define SENSOR_TIMEOUT 38 //ms
#define INF -1
#define TIMER_ROLL_OVER 40
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

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                    *
 ******************************************************************************/

/* Any private module level variable that you might need for keeping track of
   events would be placed here. Private variables should be STATIC so that they
   are limited in scope to this module. */

typedef enum {
    SendPulse,
    DetectEcho,
    ProcessDistance,
} State;

static State currState = SendPulse;

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @Function CheckPingSensorDistance(void)
 * @param none
 * @return True if event was detected false if not
 * @brief This function checks to see if a distance conversion is reading for sending
 * once a distance is read and converted it posts it to a FSM for processing. The distance in
 * inches is returned as the parameter in the DISTANCE_EVENT
 * @note 
 * @author Barron Wong */
uint8_t CheckPingSensorDistance(void) {

    ES_EventTyp_t curEvent = ES_NO_EVENT;
    ES_Event thisEvent;
    uint8_t returnVal = FALSE;

    static uint16_t triggerTime = 0;
    static uint16_t echoTimeMili = 0;
    static uint16_t echoTimeMicro = 0;
    static uint16_t timeOutMili = 0;
    uint16_t currTimeMicro = ReadTimer1(); //25us per Tick
    uint16_t currTimeMili = TIMERS_GetTime(); //1000us per Tick
    uint16_t elapsedTimeMicro = 0;
    uint16_t elapsedTimeMili = 0;
    uint8_t distanceInInches = 0;

    thisEvent.EventType = ES_NO_EVENT;

    switch (currState) {
        //Sends initial pulse
        case SendPulse:
            if (!triggerTime) {
                triggerTime = currTimeMicro;
                TRIG = 1;
            }
            if (currTimeMicro - triggerTime) {
                TRIG = 0;
                triggerTime = 0;
                currState = DetectEcho;
                timeOutMili = currTimeMili;
            }
            break;
        case DetectEcho:
            if (!echoTimeMicro) {
                if (ECHO) {
                    echoTimeMili = currTimeMili;
                    echoTimeMicro = currTimeMicro;
                    currState = ProcessDistance;
                }
            }
            if (timeOutMili - currTimeMili > SENSOR_TIMEOUT) {
                currState = SendPulse;
                echoTimeMili = 0;
                echoTimeMicro = 0;
            }
            break;
        case ProcessDistance:
            if (!ECHO) {
                elapsedTimeMicro = (currTimeMicro - echoTimeMicro);
                elapsedTimeMili = (currTimeMili - echoTimeMili);

                //Timer rolls over after every 40 Ticks
                if (elapsedTimeMicro >= 0)
                    distanceInInches = (int) ((elapsedTimeMicro * TIMER1_US_PER_TICK + elapsedTimeMili * MILI_TO_MICRO_SCALAR) / INCHES_CONVERSION_RATE);
                else
                    //Case used for rollover protection. Adds 40 to the difference of the roll over time
                    distanceInInches = (int) (((elapsedTimeMicro + TIMER_ROLL_OVER) * TIMER1_US_PER_TICK + elapsedTimeMili * MILI_TO_MICRO_SCALAR) / INCHES_CONVERSION_RATE);
                thisEvent.EventType = DISTANCE_EVENT;
                thisEvent.EventParam = distanceInInches;
                PostPingSensorFSM(thisEvent);
                return TRUE;
            }
            if (timeOutMili - currTimeMili > SENSOR_TIMEOUT) {
                currState = SendPulse;
                echoTimeMili = 0;
                echoTimeMicro = 0;
            }
            break;
        default:
            break;

    }
    return FALSE;
}

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