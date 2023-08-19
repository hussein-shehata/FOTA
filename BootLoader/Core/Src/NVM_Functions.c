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
	*PointerToNVMBlocks[BlockID] = *Data;
}

void NVM_ReadBlock(uint8_t BlockID, uint32_t* Data)
{
	*Data = *PointerToNVMBlocks[BlockID] ;
}
