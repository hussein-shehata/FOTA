/*
 * UDS.c
 *
 *  Created on: Sep 18, 2023
 *      Author: Hussein
 */
#include "UDS.h"
#include "Bit_Math.h"


#define RXNE_BIT			5
#define DUMMY_BYTE			0xAA

extern UART_HandleTypeDef huart1;
/* Make it private to be unaccessible from outside this Module */
static uint8_t CurrentSession = DefaultSession;
static volatile uint32_t* USART_SR = (uint32_t*)0x40013800;


/* Runnable to check if the tool want to send Command */
/* It is done by watching the RXNE Bit in the Status Register to check
 * if there is Data in the Register needs to be received or not
 */
void UDS_MainFunction(void)
{
	uint8_t IsDummyByteReceived = GET_BIT(*USART_SR,RXNE_BIT);
	uint8_t Buffer[1] = {0};

	if(IsDummyByteReceived == 1)
	{
		/* Make Sure that the received Byte is the Dummy One */
		HAL_UART_Receive(&huart1, Buffer, 1, 1);
		if (Buffer[0] == DUMMY_BYTE )
		{
			UDS_ReceiveCommand();
		}
		else
		{
			// Do Nothing
		}

	}
}


void UDS_ReceiveCommand(void)
{
	uint8_t ReceivingBuffer[8] = {0};
	uint8_t DataLength[1] = {0};
	UDS_SendHandShake();
	/* Receive The Data Length of the following command */
	HAL_UART_Receive(&huart1, DataLength, 1, 10);
	/* Receive the Command */
	HAL_UART_Receive(&huart1, ReceivingBuffer, DataLength[0], 1000);

	/* *************Parsing the Received Command ***********/


	if(ReceivingBuffer[0] == ChangeSession)
	{
		/* Received Command is Change Session Command */
		UDS_ChangeSession(ReceivingBuffer[1]);
	}
	else if(ReceivingBuffer[0] == SecurityAccess)
	{
		/* Received Command is Security Access Command */
		/* either Request Seed or send Key */
		if (ReceivingBuffer[1] == RequestSeed)
		{
			UDS_SendSeed();
		}
		else if(ReceivingBuffer[1] == SendKeyRequest)
		{
			UDS_CompareKeys(ReceivingBuffer);
		}
	}

}

void UDS_SendHandShake(void)
{
	HAL_UART_Transmit(&huart1, (uint8_t*) DUMMY_BYTE, 1, 1);
}

void UDS_ChangeSession(uint8_t RequestedSession)
{
	/* We can not change to Bootloader Sequence unless we are in extended session */
	/* Otherwise we can change to any session from any session */
	if (RequestedSession == BootloaderSession && CurrentSession == DefaultSession)
	{
		/* Negative Response */
		UDS_SendReponse(BootLoaderSessionRequestCommand, BootLoaderSessionNegativeResponse);
	}
	else if (RequestedSession == BootloaderSession && CurrentSession == ExtendedSession)
	{
		/* Positive Response */
		CurrentSession = BootloaderSession;
		UDS_SendReponse(BootLoaderSessionRequestCommand, BootLoaderSessionPostiveResponse);
	}
	else if (RequestedSession == ExtendedSession)
	{
		CurrentSession = ExtendedSession;
		UDS_SendReponse(ExtendedSessionRequestCommand, ExtendedSessionPostiveResponse);
	}
	else if (RequestedSession == DefaultSession)
	{
		CurrentSession = DefaultSession;
		UDS_SendReponse(DefaultSessionRequestCommand, DefaultSessionPostiveResponse);
	}
	else
	{
		// Do Nothing
	}
}
