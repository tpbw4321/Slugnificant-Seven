/*
 * File:   PingSensorEventChecker.c
 * Author: Barron Wong
 *
 * This file is created for MIDTERM Problem 6c
 *
 */
#define EVENTCHECKER_TEST
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
#include "IO_Ports.h"
/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
#define BATTERY_DISCONNECT_THRESHOLD 175
#define TIMER1_US_PER_TICK 25 //25us

#define ECHO  PORTZ04_BIT //Pin40
#define TRIG  PORTZ03_LAT //Pin41

#define INCHES_CONVERSION_RATE 148.0
#define SENSOR_TIMEOUT 70000 //us
#define INF -1
#define INT_MAX 4294967295
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
 * inches is returned as the parameter in the DISTANCE_EVENT parameter is distance in inches.
 * Function posts an event to the MainStateMachineHSM if a distance is read.
 * @note 
 * @author Barron Wong */
uint8_t CheckPingSensorDistance(void) {

    ES_EventTyp_t curEvent = ES_NO_EVENT;
    ES_Event thisEvent;
    uint8_t returnVal = FALSE;

    static uint16_t triggerTime = 0;
    static uint16_t echoTimeMicro = 0;
    static uint16_t timeOutMicro = 0;
    uint16_t currTimeMicro = 0; //1000us per Tick
    uint16_t elapsedTimeMicro = 0;
    uint8_t distanceInInches = 0;

    thisEvent.EventType = ES_NO_EVENT;
    currTimeMicro = TIMERS_GetMicroSeconds();
    switch (currState) {
            //Sends initial pulse
        case SendPulse:
            //printf("Send Pulse: %d\r\n", (currTimeMicro - triggerTime));
            if (!triggerTime) {
                triggerTime = currTimeMicro;
                TRIG = 1;
            }
            if (currTimeMicro - triggerTime > 10) {
                TRIG = 0;
                triggerTime = 0;
                currState = DetectEcho;
                timeOutMicro = currTimeMicro;
            }
            break;
            //Detects if ECHO is high and starts timing it.
        case DetectEcho:
            //printf("Detect Echo\r\n");
            if (!echoTimeMicro) {
                if (ECHO) {
                    echoTimeMicro = currTimeMicro;
                    currState = ProcessDistance;
                }
            }
            //Sensor Timeout
            if (timeOutMicro - currTimeMicro > SENSOR_TIMEOUT) {
                currState = SendPulse;
                echoTimeMicro = 0;
                timeOutMicro = 0;
            }
            break;
            //This state checks to see if ECHO goes low and begins processing 
            //The distance once it does go LOW.
        case ProcessDistance:
            //printf("Process Distance\r\n");
            if (!ECHO) {
                elapsedTimeMicro = (currTimeMicro - echoTimeMicro);

                distanceInInches = elapsedTimeMicro / INCHES_CONVERSION_RATE;
                echoTimeMicro = 0;
                timeOutMicro = 0;
                thisEvent.EventType = DISTANCE_EVENT;
                thisEvent.EventParam = distanceInInches;
                currState = SendPulse;
                SaveEvent(thisEvent);
                return TRUE;
            }
            //Sensor Timeout
            if (timeOutMicro - currTimeMicro > SENSOR_TIMEOUT) {
                currState = SendPulse;
                echoTimeMicro = 0;
                timeOutMicro = 0;
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
    TIMERS_Init();
    PORTZ03_TRIS = 0;
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
    printf("\r\nFunc: %s\tEvent: %s\tParam: %d", eventName,
            EventNames[storedEvent.EventType], storedEvent.EventParam);
}
#endif