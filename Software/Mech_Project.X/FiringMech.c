#include "BOARD.h"
#include "FiringMech.h"
#include "RC_Servo.h"
#include "pwm.h"
#include "AD.h"
#include "timers.h"
#include <stdio.h>
#include <timers.h>

#define SERVO_ANGLE_CONVERSION 11.11
#define LOADING_PIN RC_PORTW07
#define ANGLE_PIN RC_PORTW08
#define LEFT_PITCHING_MOTOR_OFFSET 0
#define RIGHT_PITCHING_MOTOR_OFFSET 150 //100
#define RIGHT_SHOOTER_MOTOR_PIN PWM_PORTZ06
#define LEFT_SHOOTER_MOTOR_PIN PWM_PORTY04

/**
 * @Function FiringMech_Init(void)
 * @param none
 * @return SUCCESS or ERROR
 * @brief Initialize the FiringMechanism Sets pin V3 for angling and Pin V04 for loading and shooting
 * the ball
 * @author Barron Wong*/
uint8_t FiringMech_Init() {
    if (RC_Init())
        if (RC_AddPins(LOADING_PIN | ANGLE_PIN)) {
            FiringMech_LoadBall();
            FiringMech_SetAngle(10);//9,8,0
            return SUCCESS;
        }
    return ERROR;
}

/**
 * @Function FiringMech_LoadBall(void)
 * @param none
 * @return SUCCESS or ERROR
 * @brief Sets the servo so that a ball can be loaded
 * @author Barron Wong*/
uint8_t FiringMech_LoadBall() {
    if (RC_SetPulseTime(LOADING_PIN, MINPULSE)) {
        return SUCCESS;
    }
    return ERROR;
}

/**
 * @Function FiringMech_LoadBall(void)
 * @param none
 * @return SUCCESS or ERROR
 * @brief Sets the servo so that a ball can be shot
 * @author Barron Wong*/
uint8_t FiringMech_ShootBall() {
    if (RC_SetPulseTime(LOADING_PIN, MAXPULSE)) {
        return SUCCESS;
    }
    return ERROR;
}

/**
 * @Function FiringMech_SetAngle(int angle)
 * @param none
 * @return SUCCESS or ERROR
 * @brief takes in an angle in degrees and sets the angling servo.
 * @author Barron Wong*/
uint8_t FiringMech_SetAngle(int angle) {
    int pulseTime = MAXPULSE - (int) (angle*SERVO_ANGLE_CONVERSION);
    if (RC_SetPulseTime(ANGLE_PIN, pulseTime) == SUCCESS)
        return SUCCESS;
    return ERROR;
}

/**
 * @Function FiringMech_SetPitchingSpeed(int angle)
 * @param none
 * @return SUCCESS or ERROR
 * @brief Sets the pitching speed in percentage 0-100.
 * @author Barron Wong*/
uint8_t FiringMech_SetPitchingSpeed(int speed) {
    int pitchingSpeed = speed * 10;
    
    if (pitchingSpeed > MAXPULSE)
        pitchingSpeed = MAX_PWM;

    if (pitchingSpeed < MIN_PWM)
        pitchingSpeed = MIN_PWM;

    if ((PWM_SetDutyCycle(RIGHT_SHOOTER_MOTOR_PIN, pitchingSpeed + RIGHT_PITCHING_MOTOR_OFFSET) & PWM_SetDutyCycle(LEFT_SHOOTER_MOTOR_PIN, pitchingSpeed + LEFT_PITCHING_MOTOR_OFFSET)) == SUCCESS)
        return SUCCESS;
    

    return ERROR;

}

//#define FIRINGMECH_TEST
#ifdef FIRINGMECH_TEST

#define DUTY_MAX 1000
#define DELAY for(i=0;i<1000000; i++)
#define ANGLE_AD_PIN AD_PORTV5

int main(void) {
    BOARD_Init();
    AD_Init();
    PWM_Init();
    PWM_AddPins(RIGHT_SHOOTER_MOTOR_PIN|LEFT_SHOOTER_MOTOR_PIN);
    TIMERS_Init();
  //  AD_AddPins(ANGLE_AD_PIN);


    int dutyCycle = 0;
    int prevDutyCycle = 0;
    int i;
    int j = 0;
    uint16_t timeElapsed = 0;

    if (!FiringMech_Init()) {
        printf("Error Initializing Firing Mech\r\n");
    }
    FiringMech_SetPitchingSpeed(40);
    FiringMech_SetAngle(9);

    while (1) {
        FiringMech_LoadBall();
        timeElapsed = TIMERS_GetTime();
        DELAY;
        printf("Delay Time: %d\r\n", timeElapsed - TIMERS_GetTime());
        FiringMech_ShootBall();
        DELAY;
    }


    return 0;
}
#endif