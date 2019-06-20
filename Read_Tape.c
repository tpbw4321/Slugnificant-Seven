#include "ES_Port.h"
#include "AD.h"
#include "BOARD.h"
#include "Read_Tape.h"
#include <stdio.h>

#define TOP_CENTER_PIN AD_PORTV5
#define TOP_LEFT_PIN AD_PORTV6
#define TOP_RIGHT_PIN AD_PORTV7
#define BOTTOM_CENTER_PIN AD_PORTV8


void Tape_Init(void) {
    AD_Init();
    AD_AddPins(AD_PORTV5);
//    AD_AddPins(TOP_LEFT_PIN);
//    AD_AddPins(TOP_RIGHT_PIN);
//    AD_AddPins(BOTTOM_CENTER_PIN);
}

//unsigned int ReadTapeSensor(void) {
//    unsigned int readout = 0;
//    readout = AD_ReadADPin(AD_PORTV5);
//    //printf("Top Center AD: %d", readout);
//    if (readout == ERROR) {
//        printf("Unable to Read Pin \r\n");
//    }
//
//    return readout;
//}

unsigned int Tape_States(void) { // TOP_CENTER, TOP_LEFT, TOP_RIGHT, BOTTOM_CENTER
    unsigned int tape_states = 0;
    unsigned int topCenterTape = AD_ReadADPin(AD_PORTV5);
//    unsigned int topLeftTape = AD_ReadADPin(TOP_LEFT_PIN);
//    unsigned int topRightTape = AD_ReadADPin(TOP_RIGHT_PIN);
//    unsigned int BottomCenterTape = AD_ReadADPin(BOTTOM_CENTER_PIN);
    
    if (topCenterTape > ON_TAPE_THRESHOLD) {
        tape_states |= 0b1000;                          //setting bit high
    } else if (topCenterTape < OFF_TAPE_THRESHOLD) {
        tape_states &= 0b0111;                             //setting bit low
    }
    //printf("Top Center Tape: %d\r\n", topCenterTape);
//    if (topLeftTape > ON_TAPE_THRESHOLD) {
//        tape_states |= 0b0100;
//    } else if (topLeftTape < OFF_TAPE_THRESHOLD) {
//        tape_states &= 0b1011;
//    }
//    
//    if (topRightTape > ON_TAPE_THRESHOLD) {
//        tape_states |= 0b0010;
//    } else if (topRightTape < OFF_TAPE_THRESHOLD) {
//        tape_states &= 0b1101;
//    }
//    
//    if (BottomCenterTape > ON_TAPE_THRESHOLD) {
//        tape_states |= 0b0001;
//    } else if (BottomCenterTape < OFF_TAPE_THRESHOLD) {
//        tape_states &= 0b1110;
//    }
    
    return tape_states;
}

