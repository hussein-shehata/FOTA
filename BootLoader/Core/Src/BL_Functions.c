/*
 * Functions.c
 *
 *  Created on: Aug 19, 2023
 *      Author: Scorpio
 */
//
#include <BL_Functions.h>
/* ******************Critical Definition *******************/
#define APPLICATION_START_MEMORY_ADDRESS		0x8005000
#define BUS_SIZE_IN_BYTES						4
#define BOOTLOADER_START_MEMORY_ADDRESS			0x8000000



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


uint8_t CheckIfAppCorupted(void)
{
	uint32_t PtrToSpOfAPP = (* (uint32_t *) APPLICATION_START_MEMORY_ADDRESS ) ;
	uint32_t PtrToSpOfBL = (* (uint32_t *) BOOTLOADER_START_MEMORY_ADDRESS ) ;

	if (PtrToSpOfAPP == PtrToSpOfBL)
	{
		/* There is a APP region as there is SP in the start of the APP memory address */
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
