/*
 * File:   Read_Tape.h
 * Author: Jesus Bobadilla
 */


#ifndef Read_Tape_H
#define Read_Tape_H

//#include <BOARD.h>


/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES                                                  *
 ******************************************************************************/
#define TAPE_DETECTED 1
#define NO_TAPE_DETECTED 0
#define ON_TAPE_THRESHOLD 800 //has to be greater than this to be on tape 2.6V
#define OFF_TAPE_THRESHOLD 750 //has to less then this to be off tape 2.4V




/**
 * @return TAPE_DETECTED or NO_TAPE_DETECTED
 * @brief  Returns the state of the top left tape sensor.
 **/
void Tape_Init(void);


unsigned int ReadTapeSensor(void);


/**
 * @return 5-bit value representing all tape sensors in following order: 
			TOP_CENTER, TOP_LEFT, TOP_RIGHT, BOTTOM_CENTER
 * @brief  Returns the state of the top left tape sensor.
 **/
unsigned int Tape_States(void);


#endif
