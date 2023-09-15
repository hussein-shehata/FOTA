/*
 * Functions.c
 *
 *  Created on: Aug 19, 2023
 *      Author: Scorpio
 */
//
#include <APP_Functions.h>
#include "SharedData.h"
/* ******************Critical Definition *******************/
#define BOOTLOADER_START_MEMORY_ADDRESS		0x8000000
#define BUS_SIZE_IN_BYTES						4



//void ToggleTestLed(void)
//{
//	/*Toggling the Test Led in the Bluepill board*/
//	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
//}

void GoToBootLoader(void)
{
	printf("Gonna go to BootLoader");
	/* points to the start of startup of the Application code */
	void (*ResetHandlerAPP)(void) = (void*)(* (volatile uint32_t *) (BOOTLOADER_START_MEMORY_ADDRESS + BUS_SIZE_IN_BYTES));
	/*we dont need to initialize stack pointer as the boot loader and the application have the
	 * same stack hence, the same stack pointer and it is already initialized by HW in ARM
	 */
	/* Notify Bootloader using this Flag that we want to download new software */
	SharedStruct.DownloadRequestedFromApplication = 1;
	ResetHandlerAPP();
}
