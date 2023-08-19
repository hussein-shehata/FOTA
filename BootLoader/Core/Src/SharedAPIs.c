/*
 * SharedAPIs.c
 *
 *  Created on: Aug 19, 2023
 *      Author: Scorpio
 */

#include "SharedAPIs.h"



/* Initialize  the Members of the structure here not in a function to optimize memory */

APIs SharedAPIs __attribute__((section(".API_SHARED"))) ={
		.PrintHelloScreen.SourceCalling = FROM_UNKNOWN_SOURCE,
		.PrintHelloScreen.PtrFunction = &PrintHelloScreen,
		.ToggleLed.SourceCalling = FROM_UNKNOWN_SOURCE,
		.ToggleLed.PtrFunction = &ToggleTestLed

};



void LOCATE_SHARED_API ToggleTestLed(void)
{
	/*Toggling the Test Led in the Bluepill board*/
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}

void LOCATE_SHARED_API PrintHelloScreen(void)
{
	if (SharedAPIs.PrintHelloScreen.SourceCalling == FROM_BOOTLOADER)
	{
		/*print hello from Bootloader*/
	}
	else if (SharedAPIs.PrintHelloScreen.SourceCalling == FROM_APPLICATION)
	{
		/*print hello from Application*/
	}
	else
	{
		/*print invalid source calling */
	}
}