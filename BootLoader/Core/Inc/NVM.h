/*
 * NVM.h
 *
 *  Created on: Aug 24, 2023
 *      Author: Scorpio
 */

#ifndef INC_NVM_H_
#define INC_NVM_H_


#include "stm32f1xx_hal.h"

/* Definitions */
#define 	NVM_BLOCKS_NUMBER			3
#define 	NVM_START_ADDRESS		0x800EC00
extern  const uint32_t  __NVM_Section_start__ ;

#define 	FLASH_REGISTER_BASE_ADDRESS		0x40022000

/* Keys for unlocking Flash  sequence */
#define 		RDPRT_KEY  		(uint32_t) 0x00A5
#define 		KEY1 			(uint32_t) 0x45670123
#define 		KEY2 			(uint32_t) 0xCDEF89AB

/* Functions prototype */
uint8_t FlashUnlock(void);
void FlashLock(void);
void NVM_Init(void);
uint8_t NVM_WriteAll(void);
uint8_t NVM_WriteAddress(uint8_t BlockID, uint32_t* DataBuffer);
void NVM_ReadAddress(uint8_t BlockID, uint32_t* DataBuffer);
uint8_t NVM_OverWriteAddress(uint8_t BlockID, uint32_t* DataBuffer);
uint8_t NVM_EraseAll(void);
uint8_t NVM_ErasePage(uint32_t Starting_Address);
uint8_t NVM_FlashApplication(void);



#endif /* INC_NVM_H_ */
