#include "AD.h"

#define TRACK_WRIE_PIN 
#define READ_TRACK_WIRE AD_ReadADPin(TRACK_WRIE_PIN)
#define UPPER_VOLTAGE_THRESHOLD
#define LOWER_VOLTAGE_THRESHOLD

unsigned char track_wire_event(void) {
    static ES_EventTyp_t lastEvent = DETECTED;
    ES_EventTyp_t curEvent;
    ES_Event thisEvent;
    uint8_t returnVal = FALSE;
    uint16_t track_wire_val = READ_TRACK_WIRE; // 
	
    if (track_wire_val > UPPER_VOLTAGE_THRESHOLD) { // 
        curEvent = OBSTACLE_NEAR;
    } else if (track_wire_val < LOWER_VOLTAGE_THRESHOLD) {
        curEvent = NO_OBSTACLE;
    }
    if (curEvent != lastEvent) { // check for change from last time
        thisEvent.EventType = curEvent;
        thisEvent.EventParam = track_wire_val;
        returnVal = TRUE;
        lastEvent = curEvent; // update history
#ifndef EVENTCHECKER_TEST           // keep this as is for test harness
        PostGenericService(thisEvent);
#else
        SaveEvent(thisEvent);
#endif   
    }
    return (returnVal);	
}

//same format for beacon detector