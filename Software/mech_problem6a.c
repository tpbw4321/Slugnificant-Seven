#include "pwm.h"
#include "AD.h"
#include "BOARD.h"
#include "IO_Ports.h"
#include "ES_Port.h"

#define PWM_PIN PWM_PORTX11
#define PWM_25KHZ 250000 
#define PWM_PIN PWM_PORTX11
#define ECHO_INPUT_PIN PORTW08_LAT     
#define READ_ECHO (IO_PortsReadPort(PORTW) & PIN8)
#define NO_OBJECT_TIME 38 //38ms
#define LOW 0  
#define HIGH 100

PWM_Init();
PWM_SetFrequency(PWM_FREQUENCY);
PWM_AddPins(PWM_PIN);
PWM_SetDutyCycle(PWM_PIN, 500); //50% use to trigger the sensor
AD_AddPins(ADC_PIN);

uint8_t distance (void) {
	
	uint8_t D = 0; //distance from sensor
	uint8_t EW = NO_OBJECT_TIME; // pulse width of echo output
	
	start_of_echo = FreeRunningTimer();
	while (READ_ECHO == HIGH) {
	}
	end_of_echo = FreeRunningTimer();
	if ((end_of_echo - start_of_echo) == NO_OBJECT_TIME) { // object to far
		return 0;
	} else {
		EW = (end_of_echo - start_of_echo) * 1000; // getting pulse width & 
		D = EW / 148;											//converting from ms to us
	}
	return D;
}



#include "pwm.h"
#include "AD.h"
#include "BOARD.h"
#include "IO_Ports.h"
#include "ES_Port.h"
#include "ES_Configure.h"
#include "ES_Framework.h"

#define PWM_PIN PWM_PORTX11
#define PWM_25KHZ 250000 
#define PWM_PIN PWM_PORTX11
#define ECHO_INPUT_PIN PORTW08_LAT     
#define READ_ECHO (IO_PortsReadPort(PORTW) & PIN8)
#define NO_OBJECT_TIME 38 //38ms
#define LOW_threshold 10  // threshold used to determine when echo is high or low
#define HIGH_threshold 90 

PWM_Init();
PWM_SetFrequency(PWM_FREQUENCY);
PWM_AddPins(PWM_PIN);
PWM_SetDutyCycle(PWM_PIN, 500); //50% use to trigger the sensor
AD_AddPins(ADC_PIN);

uint8_t distance (void) {	
	uint8_t D = 0; //distance from sensor
	uint8_t EW = NO_OBJECT_TIME; // pulse width of echo output
	
	if (Echo_Event() && thisEvent.EventType == ECHO_GOES_HIGH) {
		start_of_echo = FreeRunningTimer();
	} else if (Echo_Event() && thisEvent.EventType == ECHO_GOES_LOW) {
		end_of_echo = FreeRunningTimer();
	}
	if ((end_of_echo - start_of_echo) == NO_OBJECT_TIME) { // object to far
		return 0;
	} else {
		EW = (end_of_echo - start_of_echo) * 1000; // getting pulse width & 
		D = EW / 148;											//converting from ms to us
	}
	return D;
}

uint8_t Echo_Event(void) {
    static ES_EventTyp_t lastEvent = ECHO_GOES_LOW;
    ES_EventTyp_t curEvent;
    ES_Event thisEvent;
    uint8_t returnVal = FALSE;
	static uint8_t old_echo_value = 0;
    uint8_t current_echo_value =  READ_ECHO;
    if (current_echo_value > old_echo_value) { 
        curEvent = ECHO_GOES_HIGH;
    } else if(current_echo_value < old_echo_value){
        curEvent = ECHO_GOES_LOW;
    } 
    if (curEvent != lastEvent) { // check for change from last time
        thisEvent.EventType = curEvent;
        thisEvent.EventParam = current_echo_value;
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
