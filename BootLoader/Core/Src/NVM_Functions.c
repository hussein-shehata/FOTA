/*
 * NVM_Functions.c
 *
 *  Created on: Aug 19, 2023
 *      Author: Scorpio
 */


#include "NVM_Functions.h"
#include "stm32f1xx_hal.h"

static uint32_t (*PointerToNVMBlocks)[NVM_BLOCKS_NUMBER] = NVM_START_ADDRESS;



void NVM_WriteBlock(uint8_t BlockID, uint32_t* Data)
{
	volatile uint8_t Res1,Res2,Res3;
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
//	HAL_Delay(10000);
	Res1 = HAL_FLASH_Unlock();
	Res2 = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (0x800EC00 + BlockID * 4) , (uint32_t*)Data);
	Res3 = HAL_FLASH_Lock();

}



void NVM_ReadBlock(uint8_t BlockID, uint32_t* Data)
{
	*Data = *PointerToNVMBlocks[BlockID] ;
	dummy_endfunc();
}
