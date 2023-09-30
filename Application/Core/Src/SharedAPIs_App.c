/*
 * SharedAPIs.c
 *
 *  Created on: Aug 19, 2023
 *      Author: Scorpio
 */

#include "SharedAPIs_App.h"
#include "stm32f1xx_hal.h"
#include "UDS.h"

/* Initialize  the Members of the structure here not in a function to optimize memory */

APIs SharedAPIs __attribute__((section(".API_SHARED_Variables"))) ={
		.UDS_MainFunction = &UDS_MainFunction,
		.ToggleLedPtr = &ToggleTestLed

};


void  ToggleTestLed(void)
{
	/*Toggling the Test Led in the Bluepill board*/
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}
/* b3ml pointer points to the memory address of the shared APIs 3shan a3rf ast5dm el functions*/


//APIs* SharedAPIs  = (APIs* ) SHARED_APIS_MEMORY_ADDRESS;
