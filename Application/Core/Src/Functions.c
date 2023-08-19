/*
 * Functions.c
 *
 *  Created on: Aug 19, 2023
 *      Author: Scorpio
 */
//
#include "Functions.h"
/* ******************Critical Definition *******************/
#define APPLICATION_START_MEMORY_ADDRESS		0x8030000
#define BUS_SIZE_IN_BYTES						4

void ToggleTestLed(void)
{
	/*Toggling the Test Led in the Bluepill board*/
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}

void GoToApplication(void)
{
	printf("Gonna go to Application");
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13,GPIO_PIN_SET);
	HAL_Delay(5000);
	/* points to the start of startup of the Application code */
	void (*ResetHandlerAPP)(void) = (void*)(* (volatile uint32_t *) (APPLICATION_START_MEMORY_ADDRESS + BUS_SIZE_IN_BYTES));
	/*we dont need to initialize stack pointer as the boot loader and the application have the
	 * same stack hence, the same stack pointer and it is already initialized by HW in ARM
	 */
	ResetHandlerAPP();
}
