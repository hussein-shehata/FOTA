/*
 * NVM_Functions.h
 *
 *  Created on: Aug 19, 2023
 *      Author: Scorpio
 */

#ifndef INC_NVM_FUNCTIONS_H_
#define INC_NVM_FUNCTIONS_H_

#include "stm32f1xx_hal.h"

#define 	NVM_VARIABLE(VarType,VarName)	VarType VarName __attribute__((section(".NVM")))

#define 	NVM_BLOCKS_NUMBER			3

extern  const uint32_t  __NVM_Section_start__ ;

#define 	NVM_START_ADDRESS		0x800EC00

typedef enum
{
	JustFlashed,
	BootCounter,
	AppCounter
}BlockIDs;


void NVM_WriteBlock(uint8_t BlockID, uint32_t* Data);
void NVM_ReadBlock(uint8_t BlockID, uint32_t* Data);

#endif /* INC_NVM_FUNCTIONS_H_ */
