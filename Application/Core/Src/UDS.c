/*
 * UDS.c
 *
 *  Created on: Sep 18, 2023
 *      Author: Hussein
 */
#include "UDS.h"
#include "Bit_Math.h"
#include "aes.h"


#define RXNE_BIT			5
#define DUMMY_BYTE			0xAA

extern UART_HandleTypeDef huart1;
/* Make it private to be unaccessible from outside this Module */
static uint8_t CurrentSession = DefaultSession;
static volatile uint32_t* USART_SR = (uint32_t*)0x40013800;

static uint32_t ComparingKey[4] ; /* Variable to hold the value of the Key to approve the security access */

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


void UDS_SendSeed()
{
	/* The Seed which is gonna be send to the Flashing Tool */
	/* I will use 4 Bytes seed and key */
	    uint8_t Seed[4] = {0};
	    /* Generate Number in between 0 -> 255 */
	    Seed[0] = GenerateSeed(0,255);
	    Seed[1] = GenerateSeed(0,255);
	    Seed[2] = GenerateSeed(0,255);
	    Seed[3] = GenerateSeed(0,255);

	    for (uint8_t idx = 0; idx < 4; idx++)
	    {
	    	ComparingKey[idx] = Seed[idx];
	    }
	    /* The Key used to encrypt and decrypt the data */
	    /* It is a private key which is only known to the 2 parties " the Tool and the ECU" */
	    uint8_t Encrypting_Key[] = { 0x2b, 0x7e, 0x15, 0x16};

	    struct AES_ctx ctx;

	    AES_init_ctx(&ctx, Encrypting_Key);
	    AES_ECB_encrypt(&ctx, Seed);
	//    printf("Seed After Encrypting is : %x %x %x %x\r\n",Seed[0],Seed[1], Seed[2], Seed[3]);

	    /* Sending the Seed by Uart to the tool */
//	    HAL_UART_Transmit(&huart1, Seed, 4, 1000);
//	    mafroud lsa hn3mlo send bl send positve response

}
