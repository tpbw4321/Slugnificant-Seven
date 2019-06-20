/* 
 * File:   newfile.h
 * Author: tpbw4321
 *
 * Created on November 25, 2018, 4:41 PM
 */

#ifndef FiringMech_H
#define	FiringMech_H


/**
 * @Function FiringMech_Init(void)
 * @param none
 * @return SUCCESS or ERROR
 * @brief Initialize the FiringMechanism Sets pin V3 for angling and Pin V04 for loading and shooting
 * the ball
 * @author Barron Wong*/
uint8_t FiringMech_Init();

/**
 * @Function FiringMech_LoadBall(void)
 * @param none
 * @return SUCCESS or ERROR
 * @brief Sets the servo so that a ball can be loaded
 * @author Barron Wong*/
uint8_t FiringMech_LoadBall();

/**
 * @Function FiringMech_LoadBall(void)
 * @param none
 * @return SUCCESS or ERROR
 * @brief Sets the servo so that a ball can be shot
 * @author Barron Wong*/
uint8_t FiringMech_ShootBall();

/**
 * @Function FiringMech_SetAngle(int angle)
 * @param none
 * @return SUCCESS or ERROR
 * @brief takes in an angle in degrees and sets the angling servo.
 * @author Barron Wong*/
uint8_t FiringMech_SetAngle(int angle);

/**
 * @Function FiringMech_SetPitchingSpeed(int angle)
 * @param none
 * @return SUCCESS or ERROR
 * @brief Sets the pitching speed in percentage 0-100.
 * @author Barron Wong*/
uint8_t FiringMech_SetPitchingSpeed(int speed);
#endif