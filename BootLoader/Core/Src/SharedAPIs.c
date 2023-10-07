/*
 * SharedAPIs.c
 *
 *  Created on: Aug 19, 2023
 *      Author: Scorpio
 */

#include "SharedAPIs.h"
#include "Print.h"
#include "Flashing.h".
#include "UDS.h"

#define 	SHARED_APIS_MEMORY_ADDRESS		0x8004C00


//APIs* SharedAPIs  = (APIs* ) SHARED_APIS_MEMORY_ADDRESS;

APIs SharedAPIs __attribute__((section(".API_SHARED_Variables"))) ={
		.UDS_MainFunction = &UDS_MainFunction,
		.ToggleLedPtr = &ToggleTestLed
};



void  ToggleTestLed(void)
{
	/*Toggling the Test Led in the Bluepill board*/
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}


