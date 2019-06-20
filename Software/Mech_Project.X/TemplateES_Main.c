#include <BOARD.h>
#include <xc.h>
#include <stdio.h>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "timers.h"
#include "IO_Ports.h"
#include "LED.h"
#include "FiringMech.h"
void main(void)
{
    ES_Return_t ErrorType;

    BOARD_Init();
    AD_Init();
    TIMERS_Init();
    PWM_Init();
    LED_Init();
    FiringMech_Init();
    LED_AddBanks(LED_BANK3);
    PORTZ11_TRIS = 1; //left bumper digital pin
    PORTZ09_TRIS = 1; //right bumper digital pin
//    PORTZ03_TRIS = 1; //track wire digital pin
    PORTZ04_TRIS = 1; //beacon detector digital pin
    
    printf("Starting ES Framework Template\r\n");
    printf("using the 2nd Generation Events & Services Framework\r\n");
    


    // Your hardware initialization function calls go here

    // now initialize the Events and Services Framework and start it running
    ErrorType = ES_Initialize();
    if (ErrorType == Success) {
        ErrorType = ES_Run();
        printf("%d\r\n", ON_TAPE);

    }
    //if we got to here, there was an error
    switch (ErrorType) {
    case FailedPointer:
        printf("Failed on NULL pointer");
        break;
    case FailedInit:
        printf("Failed Initialization");
        break;
    default:
        printf("Other Failure: %d", ErrorType);
        break;
    }
    for (;;)
        ;

};

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
