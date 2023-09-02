/*
 * NVM.c
 *
 *  Created on: Aug 24, 2023
 *      Author: Scorpio
 */

#include "NVM.h"
#include "Bit_Math.h"

/* **************** Typedefs ************** */

typedef enum
{
	NVM_SUCCESS,
	NVM_ERROR,
	NVM_FLASH_IS_LOCKED,
	NVM_UNKNOWN
}NVM_Errors;

typedef enum
{
	JustFlashed,
	BootCounter,
	AppCounter
}BlockIDs;

typedef enum
{
	Bits_16 = 1,
	Bits_32 =  2,
	Bits_64 = 4
}WriteDataSize;
typedef struct
{
	volatile uint32_t		Flash_ACR;
	volatile uint32_t		Flash_KEYR;
	volatile uint32_t		Flash_OPTKEYR;
	volatile uint32_t		Flash_SR;
	volatile uint32_t		Flash_CR;
	volatile uint32_t		Flash_AR;
	uint32_t*				Reserved;
	volatile uint32_t		Flash_OBR;
	volatile uint32_t		Flash_WRPR;
}Flash_Registers;

#define FLASH_REGISTERS   		((Flash_Registers *) FLASH_REGISTER_BASE_ADDRESS )
/* Global Variables */



uint8_t FlashUnlock(void)
{
	uint8_t Status = NVM_UNKNOWN;
	FLASH_REGISTERS->Flash_KEYR = KEY1;
	FLASH_REGISTERS->Flash_KEYR = KEY2;
	HAL_Delay(100);
	/*  Check that the unlock sequence was detected and the Flash in unlocked  now  */
	if ( GET_BIT(FLASH_REGISTERS->Flash_CR , (uint32_t)7) == 0  )
	{
		/* The unlock Sequence is detected */
		Status = NVM_SUCCESS;
	}

	return Status;
}


void FlashLock(void)
{
	SET_BIT(FLASH_REGISTERS->Flash_CR, 7);
}
/* TODO mafroud deh ta5od kol ely fe el NVM section w t7oto fe variable 3shan  lma a3ml page erase lw 7abet
 * a3ml overwrite  3la ely maktob fe el nvm
 */
void NVM_Init(void)
{

}

/* TODO I want to make the option to write  16 bits or 64 bits using void pointer to the data */
uint8_t NVM_WriteAddress(BlockIDs BlockID, uint32_t* DataBuffer)
{
	uint8_t Status = NVM_UNKNOWN;
	uint32_t* Address = (uint32_t*) (NVM_START_ADDRESS  + (0x4 * BlockID) );
	/* The Write in flash is performed by 16 bits only
	 * for example if we want to write 32 bit we have to write 2 x 16 bits */
//	uint8_t NumberOfHalfWords = DataSize;
	if ( GET_BIT(FLASH_REGISTERS->Flash_CR , (uint32_t)7) != 0  )
	{
		/* The unlock Sequence is not detected */
		Status = NVM_FLASH_IS_LOCKED;
		return Status;
	}
	/* Choose Flash Programming */
	SET_BIT(FLASH_REGISTERS->Flash_CR ,0);
	/* Wait till the flash operation in progress ends */
	while(GET_BIT(FLASH_REGISTERS->Flash_SR , (uint32_t)1) == 1);

	*(uint16_t * )Address = *DataBuffer &  0x0000FFFF; /* Take the first 16 bits */
	while(GET_BIT(FLASH_REGISTERS->Flash_SR , (uint32_t)1) == 1);

	*((uint16_t * )Address +1 )= *DataBuffer >> 16 ; /* Take the second 16 bits */
	while(GET_BIT(FLASH_REGISTERS->Flash_SR , (uint32_t)1) == 1);

	/* Check that the Flash operation is completed by watching the EOP bit in SR register*/
	if(GET_BIT(FLASH_REGISTERS->Flash_SR , (uint32_t)5) == 1)
	{
		/* Reset the bit by writing 1 to the bit */
		SET_BIT(FLASH_REGISTERS->Flash_SR , (uint32_t)5);
	}
	else
	{
		Status = NVM_ERROR;
		return Status;
	}
	/* Check that the Write Operation is done successfully */
	if( *(uint32_t*) Address == *DataBuffer)
	{
		Status = NVM_SUCCESS;
	}
	/* End Flash Programming */
	CLR_BIT(FLASH_REGISTERS->Flash_CR ,0);

//	for (uint8_t iteration = 0 ; iteration < NumberOfHalfWords ; iteration++)
//	{
//		*Address = (DataBuffer[iteration] >> iteration * 16);
//	}
	return Status;
}

void NVM_ReadAddress(uint8_t BlockID, uint32_t* DataBuffer)
{
	*DataBuffer = *(uint32_t*) (NVM_START_ADDRESS  + (0x4 * BlockID) );
}


uint8_t NVM_OverWriteAddress(uint8_t BlockID, uint32_t* DataBuffer)
{
	uint32_t Status = NVM_UNKNOWN;
	/* Buffer to read all the values in NVM to be saved before erase and then write them again after erase*/
	uint32_t ReadAllBuffer[NVM_BLOCKS_NUMBER] = {0};
	for (uint32_t idx = 0; idx < NVM_BLOCKS_NUMBER; idx++)
	{
		NVM_ReadAddress(idx, &ReadAllBuffer[idx]);
	}
	/*  Erase the NVM page to write the new value in the block */
	Status = NVM_ErasePage(NVM_START_ADDRESS);

	FlashUnlock();
	for (uint32_t idx = 0; idx < NVM_BLOCKS_NUMBER; idx++)
	{
		if ( BlockID != idx )
		{
			/* If not the current Block is the wanted block to overwrite its value */
			Status = NVM_WriteAddress(idx,&ReadAllBuffer[idx]);
		}
		else if(BlockID == idx )
		{
			/* this is the wanted block to be overwritten */
			Status = NVM_WriteAddress(idx, DataBuffer);
			ReadAllBuffer[idx] = *DataBuffer;
		}
		else
		{
			// Do nothing
		}
	}
	FlashLock();
	return Status;
}


uint8_t NVM_ErasePage(uint32_t Starting_Address)
{
	uint32_t Status = NVM_UNKNOWN;
	uint32_t DataBuffer = 0;
	/* Wait till the flash memory operation in progress ends */
	while(GET_BIT(FLASH_REGISTERS->Flash_SR , (uint32_t)0) == 1);
	CLR_BIT(FLASH_REGISTERS->Flash_CR, 0);
	SET_BIT(FLASH_REGISTERS->Flash_CR, 1);
	FLASH_REGISTERS->Flash_AR = Starting_Address;
	SET_BIT(FLASH_REGISTERS->Flash_CR, 6);

	/* Wait till the flash memory operation in progress ends */
	while(GET_BIT(FLASH_REGISTERS->Flash_SR , (uint32_t)0) == 1);

	/* Check that the Flash operation is completed by watching the EOP bit in SR register*/
	while(GET_BIT(FLASH_REGISTERS->Flash_SR , (uint32_t)5) == 1);

	/* Reset the bit by writing 1 to the bit */
	SET_BIT(FLASH_REGISTERS->Flash_SR , (uint32_t)5);

	/* Check that Erase is successful */
	NVM_ReadAddress(JustFlashed, &DataBuffer);
	if(DataBuffer == 0xFFFFFFFF)
	{
		Status = NVM_SUCCESS;
	}
	else
	{
		Status = NVM_ERROR;
	}
	CLR_BIT(FLASH_REGISTERS->Flash_CR, 1);
	return Status ;
}

