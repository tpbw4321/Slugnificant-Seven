

#include "ES_Port.h"
#include "AD.h"
#include "BOARD.h"
#include "Read_Tape.h"

#define TOP_CENTER_PIN 
#define TOP_BOTTOM_PIN
#define TOP_LEFT_PIN
#define TOP_RIGHT_PIN
#define READ_TOP_CENTER AD_ReadADPin(TOP_CENTER_PIN)
#define READ_BOTTOM_CENTER AD_ReadADPin(TOP_BOTTOM_PIN)
#define READ_TOP_LEFT AD_ReadADPin(TOP_LEFT_PIN)
#define READ_TOP_RIGHT AD_ReadADPin(TOP_RIGHT_PIN)
#define ON_TAPE_THRESHOLD 100
#define OFF_TAPE_THRESHOLD 500



unsigned char Top_Center_Tape(void) {
	if (READ_TOP_CENTER < ON_TAPE_THRESHOLD) {
		return TAPE_DETECTED;
	} else if(READ_TOP_CENTER > OFF_TAPE_THRESHOLD) {
		return NO_TAPE_DETECTED;
	}
}

unsigned char Bottom_Center_Tape(void) {
	if (READ_BOTTOM_CENTER < ON_TAPE_THRESHOLD) {
		return TAPE_DETECTED;
	} else if(READ_BOTTOM_CENTER > OFF_TAPE_THRESHOLD) {
		return NO_TAPE_DETECTED;
	}
}

unsigned char Top_Left_Tape(void) {
	if (READ_TOP_LEFT < ON_TAPE_THRESHOLD) {
		return TAPE_DETECTED;
	} else if(READ_TOP_LEFT > OFF_TAPE_THRESHOLD) {
		return NO_TAPE_DETECTED;
	}
}


unsigned char Top_Right_Tape(void) {
	if (READ_TOP_RIGHT < ON_TAPE_THRESHOLD) {
		return TAPE_DETECTED;
	} else if(READ_TOP_RIGHT > OFF_TAPE_THRESHOLD) {
		return NO_TAPE_DETECTED;
	}	
}


unsigned char Tape_States(void) { // TOP_CENTER, TOP_LEFT, TOP_RIGHT, BOTTOM_CENTER
	return (Bottom_Center_Tape() + (Top_Right_Tape() << 1) + (Top_Left_Tape() << 2) + 
	(Top_Center_Tape() << 3));
}

