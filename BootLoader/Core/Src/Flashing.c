/*
 * Flashing.c
 *
 *  Created on: Aug 25, 2023
 *      Author: Scorpio
 */


#include "Flashing.h"
#include "NVM.h"
#include "Bit_Math.h"
#include "SharedAPIs.h"
#include "SharedData.h"

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

uint8_t  FlashApplication()
{
	uint8_t Status = ETX_OTA_EX_ERR;
	uint8_t ReceivedFrame = 0xFF;
	uint8_t Buffer[ETX_OTA_PACKET_MAX_SIZE] = {0};


	do
	{
	/* Waiting till receiving the  Frame */
	Write0xFFToTheBuffer(Buffer);
//	uint16_t NumberOfBytes = CalculateTheExpectedBytes();
	if(ExpectedFrame == ETX_OTA_PACKET_TYPE_CMD)
	{
		SendResponseToHost(ETX_OTA_ACK);
	}
	/* There is a problem in this timeout parameter when jumping from Application to BL */
	HAL_UART_Receive(&huart1, Buffer, 2, 1000);
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
		HAL_UART_Receive(&huart1, &(Buffer[2]), 8, 1000);
		ProcessCommandFrame(Buffer);
		break;

	case ETX_OTA_PACKET_TYPE_HEADER :
		/* Received Frame is Header Frame */
		HAL_UART_Receive(&huart1, &(Buffer[2]), 23, 1000);
		ProcessHeaderFrame(Buffer);
		break;
	case ETX_OTA_PACKET_TYPE_DATA:
		HAL_UART_Receive(&huart1, &(Buffer[2]), 2, 1000);
		uint16_t DataLength = Buffer[3];
		DataLength = (DataLength<<8) | Buffer[2];
		HAL_UART_Receive(&huart1, &(Buffer[4]), DataLength + 5, 100000);
		ProcessDataFrame(Buffer);
		break;

	}



	}while(ExpectedFrame != ETX_OTA_PACKET_TYPE_FinishedComm);
	ExpectedFrame = ETX_OTA_PACKET_TYPE_CMD; /* for the next time to download new Application in the same reset */
	SendResponseToHost(ETX_OTA_ACK);
	Status = ETX_OTA_EX_OK;
	return Status;
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
	uint8_t ResponseBuffer[10]={0};
	ETX_OTA_RESP_*  ResponseFrame = (ETX_OTA_RESP_*) ResponseBuffer;
	ResponseFrame->sof = ETX_OTA_SOF;
	ResponseFrame->packet_type = ETX_OTA_PACKET_TYPE_RESPONSE;
	ResponseFrame->status = Response;
	HAL_UART_Transmit(&huart1, ResponseBuffer, 10, 100);

}

uint8_t ProcessCommandFrame(uint8_t* Buffer)
{
	uint8_t Status;
	ETX_OTA_COMMAND_* OtaStartFrame = (ETX_OTA_COMMAND_*) Buffer;
		if(OtaStartFrame->sof != ETX_OTA_SOF)
		{
			Status = ETX_OTA_EX_ERR;  /* mgtlesh el start of frame byte */
		}
		if( (OtaStartFrame->cmd == ETX_OTA_CMD_START) && (OtaStartFrame->packet_type == ETX_OTA_PACKET_TYPE_CMD ))
		{
			ExpectedFrame = ETX_OTA_PACKET_TYPE_HEADER;
			/* Send ACK to the host as this is the OTA Start Frame */
			SendResponseToHost(ETX_OTA_ACK);
			Status = ETX_OTA_EX_OK;
		}
		else if ((OtaStartFrame->cmd == ETX_OTA_CMD_END) && (OtaStartFrame->packet_type == ETX_OTA_PACKET_TYPE_CMD ))
		{
			ExpectedFrame = ETX_OTA_PACKET_TYPE_FinishedComm;
			/* Send ACK to the host as this is the OTA End Frame */
			SendResponseToHost(ETX_OTA_ACK);
			Status = ETX_OTA_EX_OK;
		}
		else
		{
			/* Send NACK to the host as this is the OTA Start Frame */
			SendResponseToHost(ETX_OTA_NACK);
			Status = ETX_OTA_EX_ERR;
		}
		return Status;
}

uint8_t ProcessHeaderFrame(uint8_t* Buffer)
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
			}
			return Status;
}

uint8_t ProcessDataFrame(uint8_t* Buffer)
{
	/* hna bi3ml write l el frame kolo msh el data bs */
	/* Fixed el mafroud */
	uint8_t Status = ETX_OTA_EX_ERR;
	ETX_OTA_DATA_* OtaDataFrame = (ETX_OTA_DATA_*) Buffer;
	uint16_t DataLength = OtaDataFrame->data_len ;

	/* Here I want the address of the variable data not the value it holds
	 * The Address -> is the address of the first data byte received from the tool
	 * the value -> the value of the fist byte received from the tool
	 *
	 * Example : if we received 0xAA from the tool as the first byte:
	 * uint8_t* data = 0xAA;                // Which means that it will points to address 0xAA so it is wrong
	 * to deference it   (data[0])
	 *
	 * so to get 0xAA i need to point to the address of data and deference it
	 * PtrData = &data;                PtrData[0]
	 */
	uint8_t* PtrData = (uint8_t * )(&(OtaDataFrame->data));


	static uint32_t NumberOfPagesWritten = 0;

	if (DataLength == ETX_OTA_DATA_MAX_SIZE)
	{
		/* Write Full page in flash memorry */
		ExpectedFrame = ETX_OTA_PACKET_TYPE_DATA;
		FlashUnlock();
		OverWritePageFlash(ETX_APP_FLASH_ADDR + (NumberOfPagesWritten * 0x0400 ), (uint32_t *) PtrData);
		FlashLock();
		NumberOfPagesWritten++;
		SendResponseToHost(ETX_OTA_ACK);
	}
	else
	{
		/* Write Address by Address as we are gonna write less than one page */
//		Status = NVM_ErasePage(ETX_APP_FLASH_ADDR + (NumberOfPagesWritten * 0x0400) );
		uint32_t StartingAddress = ETX_APP_FLASH_ADDR + (NumberOfPagesWritten * 0x0400) ;
		FlashUnlock();
//		uint8_t (*DebugBuffer)[252] = (uint8_t*)Buffer;
//		for (uint16_t idx = 0; idx < DataLength / 4; idx++)
//		{
//			uint32_t CurrentHexa = *(uint32_t*) ( &( PtrData[idx]) );
//			Flash_WriteAddress(StartingAddress + idx * 0x04, (uint32_t*) ( &( PtrData[idx]) ) );
//		}

//		Write0xFFToTheBuffer(Buffer);
		/* Removing the SOF and CRC Bytes equal to 0xFF till finding a better solution */
		/* The Adding of 4 as the first 4 bytes are not data and they are SOF PacketType and Datalength*/
		Buffer[DataLength + 1 + 4] = 0xFF;
		Buffer[DataLength + 2 + 4] = 0xFF;
		Buffer[DataLength + 3 + 4] = 0xFF;
		Buffer[DataLength + 4 + 4] = 0xFF;
		Buffer[DataLength + 5 + 4] = 0xFF;

		OverWritePageFlash(ETX_APP_FLASH_ADDR + (NumberOfPagesWritten * 0x0400 ), (uint32_t *) PtrData);
		FlashLock();
		SendResponseToHost(ETX_OTA_ACK);
		ExpectedFrame = ETX_OTA_PACKET_TYPE_FinishedComm;

	}
	Status = ETX_OTA_EX_OK;
	return Status;
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
	FlashUnlock();
	Status = Flash_ErasePage(StartingAddress);

	for (uint16_t idx = 0; idx < ETX_OTA_DATA_MAX_SIZE / 4 ; idx++)
	{
		Flash_WriteAddress(StartingAddress + idx * 0x04, &DataBuffer[idx]);

	}

	FlashLock();
	Status = ETX_OTA_EX_OK;
	return Status;
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


volatile uint8_t Flash_ErasePage(uint32_t StartingAddress)
{
	uint32_t Status = ETX_OTA_EX_ERR;
	/* Wait till the flash memory operation in progress ends */
	while(GET_BIT(FLASH_REGISTERS->Flash_SR , (uint32_t)0) == 1);
	CLR_BIT(FLASH_REGISTERS->Flash_CR, 0);
	SET_BIT(FLASH_REGISTERS->Flash_CR, 1);
	FLASH_REGISTERS->Flash_AR = StartingAddress;
	SET_BIT(FLASH_REGISTERS->Flash_CR, 6);

	/* Wait till the flash memory operation in progress ends */
	while(GET_BIT(FLASH_REGISTERS->Flash_SR , (uint32_t)0) == 1);

	/* Check that the Flash operation is completed by watching the EOP bit in SR register*/
	while(GET_BIT(FLASH_REGISTERS->Flash_SR , (uint32_t)5) == 0);

	/* Reset the bit by writing 1 to the bit */
	SET_BIT(FLASH_REGISTERS->Flash_SR , (uint32_t)5);

	/* Check that Erase is successful */
	uint32_t* CheckPtr = (uint32_t*) StartingAddress;
	if (*CheckPtr != 0xFFFFFFFF)
	{
		Status = ETX_OTA_EX_ERR;
	}
	else
	{
		Status = ETX_OTA_EX_OK;
	}
	CLR_BIT(FLASH_REGISTERS->Flash_CR, 1);
	return Status ;
}


void Write0xFFToTheBuffer(uint8_t* Buffer)
{
	for (uint32_t idx = 0; idx <ETX_OTA_PACKET_MAX_SIZE; idx++)
	{
		Buffer[idx] = 0xFF;
	}
}

/* to Calculate the expected receiving bytes from UART */
//uint16_t CalculateTheExpectedBytes(void)
//{
//	uint16_t ExpectedBytes;
//	switch(ExpectedFrame)
//	{
//	case ETX_OTA_PACKET_TYPE_CMD:
//		ExpectedBytes = 10;
//		break
//
//	case ETX_OTA_PACKET_TYPE_DATA
//	}
//
//}
