/*
 * SharedAPIs.c
 *
 *  Created on: Aug 19, 2023
 *      Author: Scorpio
 */

#include "SharedAPIs.h"
#include "Print.h"
#include "Flashing.h"


/* Initialize  the Members of the structure here not in a function to optimize memory */

APIs SharedAPIs __attribute__((section(".API_SHARED_Variables"))) ={
		.ToggleLedPtr = &ToggleTestLed,
		.FlashNewSoftwarePtr = &FlashApplication

};



void  ToggleTestLed(void)
{
	/*Toggling the Test Led in the Bluepill board*/
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}


