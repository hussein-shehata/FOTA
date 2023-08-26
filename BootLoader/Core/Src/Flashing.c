/*
 * Flashing.c
 *
 *  Created on: Aug 25, 2023
 *      Author: Scorpio
 */


#include "Flashing.h"
#include "NVM.h"
#include "Bit_Math.h"

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


static ETX_OTA_PACKET_TYPE_  ExpectedFrame = ETX_OTA_PACKET_TYPE_CMD ;
static uint32_t AppSize = 0;

uint8_t FlashApplication(void)
{
	uint8_t Status = ETX_OTA_EX_ERR;
	uint8_t ReceivedFrame = 0xFF;
	uint8_t Buffer[ETX_OTA_PACKET_MAX_SIZE] = {0};

	do
	{
	/* Waiting till receiving the  Frame */
	HAL_UART_Receive(&huart1, Buffer, ETX_OTA_PACKET_MAX_SIZE, 1000);
	ReceivedFrame = Buffer[1];
	if(ReceivedFrame != ExpectedFrame)
	{
		/* Wrong Received Frame */
		 Status = ETX_OTA_EX_ERR;
		 return Status;
	}
	switch (ReceivedFrame)
	{
	case ETX_OTA_PACKET_TYPE_CMD:
		/* Received either Start or End Frame */
		ProcessCommandFrame(Buffer);
		break;

	case ETX_OTA_PACKET_TYPE_HEADER :
		/* Received Frame is Header Frame */
		ProcessHeaderFrame(Buffer);
		break;
	case ETX_OTA_PACKET_TYPE_DATA:
		ProcessDataFrame(Buffer);
		break;

	}



	}while(ExpectedFrame != ETX_OTA_PACKET_TYPE_FinishedComm);


}

//uint8_t ReceiveChunk(uint8_t* Buffer,uint8_t Len)
//{
//	uint8_t Status = HAL_UART_Receive(&huart1, Buffer, Len, 5000);
//
//	Return Status;
//}


//uint8_t ProcessChunck(uint8_t* Buffer)
//{
//	ETX_OTA_PACKET_TYPE_ PacketType = Buffer[1]; /* Second Byte */
//	switch (PacketType)
//	{
//	case ETX_OTA_PACKET_TYPE_CMD :
//		ETX_OTA_COMMAND_* CurrentFrame = (ETX_OTA_COMMAND_*) Buffer;
//		CurrentFrame->
//	}
//}

void SendResponseToHost(uint8_t Response)
{
	HAL_UART_Transmit(&huart1, &Response, 1, 100);

}

void ProcessCommandFrame(uint8_t* Buffer)
{
	uint8_t Status;
	ETX_OTA_COMMAND_* OtaStartFrame = (ETX_OTA_COMMAND_*) Buffer;
		if(OtaStartFrame->sof != ETX_OTA_SOF)
		{
			Status = ETX_OTA_EX_ERR;  /* mgtlesh el start of frame byte */
			return Status;
		}
		if( (OtaStartFrame->cmd == ETX_OTA_CMD_START) && (OtaStartFrame->packet_type == ETX_OTA_PACKET_TYPE_CMD ))
		{
			ExpectedFrame = ETX_OTA_PACKET_TYPE_HEADER;
			/* Send ACK to the host as this is the OTA Start Frame */
			SendResponseToHost(ETX_OTA_ACK);
		}
		else if ((OtaStartFrame->cmd == ETX_OTA_CMD_END) && (OtaStartFrame->packet_type == ETX_OTA_PACKET_TYPE_CMD ))
		{
			ExpectedFrame = ETX_OTA_PACKET_TYPE_FinishedComm;
			/* Send ACK to the host as this is the OTA End Frame */
			SendResponseToHost(ETX_OTA_ACK);
		}
		else
		{
			/* Send NACK to the host as this is the OTA Start Frame */
			SendResponseToHost(ETX_OTA_NACK);
			Status = ETX_OTA_EX_ERR;
			return Status;
		}
}

void ProcessHeaderFrame(uint8_t* Buffer)
{
	uint8_t Status = ETX_OTA_EX_ERR;
	ETX_OTA_HEADER_* OtaHeaderFrame = (ETX_OTA_HEADER_*) Buffer;
			if(OtaHeaderFrame->sof != ETX_OTA_SOF)
			{
				Status = ETX_OTA_EX_ERR;  /* mgtlesh el start of frame byte */
				return Status;
			}
			if((OtaHeaderFrame->packet_type == ETX_OTA_PACKET_TYPE_HEADER ))
			{
				Status = ETX_OTA_EX_OK;
				ExpectedFrame = ETX_OTA_PACKET_TYPE_DATA;
				AppSize = OtaHeaderFrame->meta_data.package_size;
				/* Send ACK to the host as this is the OTA Start Frame */
				SendResponseToHost(ETX_OTA_ACK);
			}
			else
			{
				/* Send NACK to the host as this is the OTA Start Frame */
				SendResponseToHost(ETX_OTA_NACK);
				Status = ETX_OTA_EX_ERR;
				return Status;
			}
}

void ProcessDataFrame(uint8_t* Buffer)
{
	uint8_t Status = ETX_OTA_EX_ERR;
	ETX_OTA_DATA_* OtaDataFrame = (ETX_OTA_DATA_*) Buffer;
	uint8_t DataLength = OtaDataFrame->data_len ;
	static uint32_t NumberOfPagesWritten = 0;

	if (DataLength == ETX_OTA_DATA_MAX_SIZE)
	{
		/* Write Full page in flash memorry */
		ExpectedFrame = ETX_OTA_PACKET_TYPE_DATA;
		OverWritePageFlash(ETX_APP_FLASH_ADDR + (NumberOfPagesWritten * 0x0400 ), (uint32_t *) Buffer);
		NumberOfPagesWritten++;
	}
	else
	{
		/* Write Address by Address as we are gonna write less than one page */

	}
}



uint8_t OverWritePageFlash(uint32_t StartingAddress, uint32_t* DataBuffer)
{
	uint8_t Status = ETX_OTA_EX_ERR;
	if(StartingAddress < ETX_APP_FLASH_ADDR )
	{
		/* not allowed to write in non-Flash App Region */
		Status = ETX_OTA_EX_ERR;
		return Status;
	}
	Status = NVM_ErasePage(StartingAddress);
	FlashUnlock();
	for (uint16_t idx = 0; idx < ETX_OTA_DATA_MAX_SIZE / 4 ; idx++)
	{
		Flash_WriteAddress(StartingAddress + idx * 0x04, &DataBuffer[idx]);

	}

	FlashLock();
}

uint8_t Flash_WriteAddress(uint32_t StartingAddress, uint32_t* DataBuffer)
{
	uint8_t Status = ETX_OTA_EX_ERR;
	uint32_t* Address = (uint32_t*) (StartingAddress);
	/* The Write in flash is performed by 16 bits only
	 * for example if we want to write 32 bit we have to write 2 x 16 bits */
//	uint8_t NumberOfHalfWords = DataSize;
	if ( GET_BIT(FLASH_REGISTERS->Flash_CR , (uint32_t)7) != 0  )
	{
		/* The unlock Sequence is not detected */
		Status = ETX_OTA_EX_ERR;
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
		Status = ETX_OTA_EX_ERR;
		return Status;
	}
	/* Check that the Write Operation is done successfully */
	if( *(uint32_t*) Address == *DataBuffer)
	{
		Status = ETX_OTA_EX_OK;
	}
	/* End Flash Programming */
	CLR_BIT(FLASH_REGISTERS->Flash_CR ,0);

//	for (uint8_t iteration = 0 ; iteration < NumberOfHalfWords ; iteration++)
//	{
//		*Address = (DataBuffer[iteration] >> iteration * 16);
//	}
	return Status;
}


