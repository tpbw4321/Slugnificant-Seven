/*
 * File: TrackWireService.h
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
#include "TrackWireService.h"
#include "timers.h"
#include <stdio.h>

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/

#define BATTERY_DISCONNECT_THRESHOLD 175
#define MAX_SAMPLE_SIZE 32

#define TRACK_WIRE_NEARBY_LOW_THRESHOLD 305
#define TRACK_WIRE_NEARBY_HIGH_THRESHOLD 315

#define CLOSEST_TO_TRACKWIRE_LOW_THRESHOLD 925
#define CLOSEST_TO_TRACKWIRE_HIGH_THRESHOLD 935

#define SAMPLE_TIMER_TICKS 2

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this machine. They should be functions
   relevant to the behavior of this state machine */

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                    *
 ******************************************************************************/
/* You will need MyPriority and maybe a state variable; you may need others
 * as well. */

static uint8_t MyPriority;
static uint16_t sum32Point = 0;
static uint8_t samplesReady = FALSE;

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/
void collectSample(void);
uint8_t ProcessSamples(ES_Event ThisEvent);

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
uint8_t InitTrackWireService(uint8_t Priority) {
    ES_Event ThisEvent;

    MyPriority = Priority;

    // in here you write your initialization code
    // this includes all hardware and software initialization
    // that needs to occur.

    //Add TrackWire Pin
    AD_AddPins(AD_PORTV3);

    //Start Service Timer
    TIMERS_InitTimer(TRACKWIRE_SERVICE_TIMER, SAMPLE_TIMER_TICKS);

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
uint8_t PostTrackWireService(ES_Event ThisEvent) {
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
ES_Event RunTrackWireService(ES_Event ThisEvent) {
    ES_Event ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

    /********************************************
     in here you write your service code
     *******************************************/
    static ES_EventTyp_t lastEvent = ES_NO_EVENT;
    ES_EventTyp_t curEvent = ES_NO_EVENT;

    switch (ThisEvent.EventType) {
        case ES_INIT:
            // No hardware initialization or single time setups, those
            // go in the init function above.
            //
            // This section is used to reset service for some reason
            break;

        case ES_TIMEOUT:
            collectSample();
            if (ProcessSamples(ThisEvent))
                curEvent = ThisEvent.EventType;
            //Re-enable Service Timer
            TIMERS_InitTimer(TRACKWIRE_SERVICE_TIMER, SAMPLE_TIMER_TICKS);


            if (curEvent != lastEvent) { // check for change from last time
                lastEvent = curEvent; // update history
#ifndef SIMPLESERVICE_TEST           // keep this as is for test harness
                PostMainStateMachineHSM(ReturnEvent);
#else
                PostTemplateService(ReturnEvent);
#endif   
            }
            break;
#ifdef SIMPLESERVICE_TEST     // keep this as is for test harness      
        default:
            printf("\r\nEvent: %s\tParam: 0x%X",
                    EventNames[ThisEvent.EventType], ThisEvent.EventParam);
            break;
#endif
    }

    return ReturnEvent;
}

/*******************************************************************************
 * PRIVATE FUNCTIONs                                                           *
 ******************************************************************************/
/**
 * @Function RunTemplateService(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be passed to mainHSM.
 * @return TRUE if an event is detected, FALSE if not
 * @brief This function takes the 32 samples and averages them by shifting to the
 * left by 5. It then checks the results with a hysteresis bound as low and
 * high thresholds. It stores the event and the average as a parameter in ThisEvent
 * @note Remember to rename to something appropriate.
 *       Returns ES_NO_EVENT if the event have been "consumed." 
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t ProcessSamples(ES_Event ThisEvent) {
    uint16_t average = 0;
    if (samplesReady) {
        average = sum32Point >> 5;

        if (average > CLOSEST_TO_TRACKWIRE_HIGH_THRESHOLD) {
            ThisEvent.EventType = CLOSEST_TO_TRACKWIRE;
            ThisEvent.EventParam = average;
            return TRUE;
        } else if (average < CLOSEST_TO_TRACKWIRE_LOW_THRESHOLD && average > TRACK_WIRE_NEARBY_HIGH_THRESHOLD) {
            ThisEvent.EventType = TRACK_WIRE_NEARBY;
            ThisEvent.EventParam = average;
            return TRUE;
        }
    }
    return FALSE;
}

/**
 * @Function collectSample(void)
 * @param NONE
 * @return NONE
 * @brief This function collects samples from the AD pin that is reading
 * in the data from the track wire sensor. The function creates a 32 Point moving
 * average to act as a low pass filter. It uses a queue to store 32 points
 * so that it can cycle in new samples and cycle out old samples
 * @note 
 * @author Barron Wong */
void collectSample(void) {
    uint16_t queue[MAX_SAMPLE_SIZE];
    uint8_t sampleSize = 0;
    uint8_t head = 0;
    uint8_t tail = 0;
    uint16_t sample = AD_ReadADPin(AD_PORTV3);



    if (sampleSize >= MAX_SAMPLE_SIZE) {
        sum32Point -= queue[tail];
        sampleSize--;
        tail++;
        tail %= MAX_SAMPLE_SIZE;
        samplesReady = TRUE;
    }

    queue[head] = sample;
    sum32Point += queue[head];
    sampleSize++;
    head++;
    head %= MAX_SAMPLE_SIZE;
}

