/*
 * SharedAPIs.c
 *
 *  Created on: Aug 19, 2023
 *      Author: Scorpio
 */

#include "SharedAPIs.h"
#include "Print.h"
#include "Flashing.h"

#define 	SHARED_APIS_MEMORY_ADDRESS		0x800E800


APIs* SharedAPIs  = (APIs* ) SHARED_APIS_MEMORY_ADDRESS;



//void  ToggleTestLed(void)
//{
//	/*Toggling the Test Led in the Bluepill board*/
//	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
//}


