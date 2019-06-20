/*
 * File:   Problem6a.c
 * Author: Barron
 *
 * Created on November 3, 2018, 3:22 PM
 */


#include "xc.h"
#include <stdio.h>
#include "BOARD.h"
#include "AD.h"
#include "timers.h"
#include "IO_Ports.h"

#define MILI_TO_MICRO_SCALAR 1000
#define MICRO_TO_INCHES_DIVISOR 148

#define TRIG PORTX03_LAT
#define ECHO PORTX04_BIT




#define SETPIN41(SETTING) (PORTBbits.RB1 = SETTING)
#define SET_ECHO_PIN(ECHO) (PORTX03_TRIS = ECHO)
#define SET_TRIG_PIN(TRIG) (PORTX03_TRIS = TRIG)
#define SENSOR_TIMEOUT 38 //38ms
#define INF -1

uint8_t GetDistance(void);
void SendPulse(void);
uint16_t DetectEcho(void);

int main(void) {


    BOARD_Init();

    AD_Init();

    //Setting Pins modes
    SET_ECHO_PIN(1);
    SET_TRIG_PIN(0);
    

    while (1) {
        
        printf("Distance: %d\r\n", GetDistance());
        
    }

    return 0;
}

/**
 * @Function GetDistance(void)
 * @param none
 * @return The distance in inches from an object or INF if sensor time out is reached
 * @brief This function using blocking code to figure out the distance an object is
 * from the proximity sensor
 * @note 
 * @author Barron Wong */
uint8_t GetDistance() {

    uint16_t timeOfFlight = 0;

    SendPulse();
    timeOfFlight = DetectEcho();

    if (timeOfFlight == INF) {
        return INF;
    }
    return (timeOfFlight * MILI_TO_MICRO_SCALAR) / MICRO_TO_INCHES_DIVISOR;
}

/**
 * @Function SendPulse(void)
 * @param none
 * @return Nothing
 * @brief This function sends a 1ms pulse to the Trigger pin of the 
 * proximity sensor.
 * @note 
 * @author Barron Wong */
void SendPulse(void) {
    uint16_t currTime = TIMERS_GetTime();

    TRIG = 1;
    while (TIMERS_GetTime() - currTime == 0) {
    }
    TRIG = 0;
}

/**
 * @Function SendPulse(void)
 * @param none
 * @return The pulse width of the echo pin in milliseconds. Returns INF if sensor
 * timeout is reached
 * @brief This function starts a timer when the echo pin is high and waits for
 * the pin to go low. It returns the time the echo pin is high.
 * @note 
 * @author Barron Wong */
uint16_t DetectEcho(void) {
    uint16_t echoTimestamp = 0;

    while (!ECHO) {
        if (echoTimestamp - TIMERS_GetTime() > SENSOR_TIMEOUT)
            return INF;
    }
    echoTimestamp = TIMERS_GetTime();

    while (ECHO) {
        if (echoTimestamp - TIMERS_GetTime() > SENSOR_TIMEOUT)
            return INF;
    }
    return TIMERS_GetTime() - echoTimestamp;
}