/*
 * File:   Read_Tape.h
 * Author: Jesus Bobadilla
 */


#ifndef Read_Tape_H
#define Read_Tape_H

#include <BOARD.h>


/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES                                                  *
 ******************************************************************************/
#define TAPE_DETECTED 1
#define NO_TAPE_DETECTED 0




/**
 * @return TAPE_DETECTED or NO_TAPE_DETECTED
 * @brief  Returns the state of the top left tape sensor.
 **/
unsigned char Top_Center_Tape(void);


/**
 * @return TAPE_DETECTED or NO_TAPE_DETECTED
 * @brief  Returns the state of the top left tape sensor.
 **/
unsigned char Bottom_Center_Tape(void);


/**
 * @return TAPE_DETECTED or NO_TAPE_DETECTED
 * @brief  Returns the state of the top left tape sensor.
 **/
unsigned char Top_Left_Tape(void);


/**
 * @return TAPE_DETECTED or NO_TAPE_DETECTED
 * @brief  Returns the state of the top left tape sensor.
 **/
unsigned char Top_Right_Tape(void);


/**
 * @return 5-bit value representing all tape sensors in following order: 
			TOP_CENTER, TOP_LEFT, TOP_RIGHT, BOTTOM_CENTER
 * @brief  Returns the state of the top left tape sensor.
 **/
unsigned char Tape_States(void);


#endif
