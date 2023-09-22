/*
 * UDS.c
 *
 *  Created on: Sep 18, 2023
 *      Author: Hussein
 */

/* *************TODO in this file*************** */

/* 1- Handle the case of request download positve response in send response function as the frame is diffcult to understand now */
/* 2- Make Flags to grant access to each of the requested request */
#include "UDS.h"
#include "Bit_Math.h"
#include "aes.h"

#define DUMMY_VARIABLE(x)		(void)x

#define RXNE_BIT							5
#define DUMMY_BYTE							0xAA
#define SEED_LENGTH							4
#define NEGATIVE_REPONSE_CONSTANT_HEXA		0x7F
extern UART_HandleTypeDef huart1;
/* Make it private to be unaccessible from outside this Module */
static uint8_t ReceivingBuffer[8] = {0};
static uint8_t CurrentSession = DefaultSession;
static volatile uint32_t* USART_SR = (uint32_t*)0x40013800;

static uint32_t ComparingKey[SEED_LENGTH] ; /* Variable to hold the value of the Key to approve the security access */

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
	/* Made it Global Variable to be used in another functions */
//	uint8_t ReceivingBuffer[8] = {0};
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
		/* but we must be in Bootloader Session */
		/* either Request Seed or send Key */
		if (ReceivingBuffer[1] == RequestSeed)
		{
			UDS_SendSeed();
		}
		else if(ReceivingBuffer[1] == SendKeyRequest)
		{
			UDS_CompareKeys(&(ReceivingBuffer[2]));
		}
		else
		{
			// Do Nothing
		}
	}
	else if(ReceivingBuffer[0] == RequestDownload)
	{
		/* The Received Command is to Request Download */
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
		UDS_SendReponse(BootLoaderSessionNegativeResponse, ServiceNotSupported);
	}
	else if (RequestedSession == BootloaderSession && CurrentSession == ExtendedSession)
	{
		/* Positive Response */
		CurrentSession = BootloaderSession;

		UDS_SendReponse(BootLoaderSessionPostiveResponse, NoNRC);
	}
	else if (RequestedSession == ExtendedSession)
	{
		CurrentSession = ExtendedSession;
		UDS_SendReponse(ExtendedSessionPostiveResponse, NoNRC);
	}
	else if (RequestedSession == DefaultSession)
	{
		CurrentSession = DefaultSession;
		UDS_SendReponse(DefaultSessionPostiveResponse, NoNRC);
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
	    uint8_t Seed[SEED_LENGTH] = {0};
	    /* Generate Number in between 0 -> 255 */
	    Seed[0] = GenerateSeed(0,255);
	    Seed[1] = GenerateSeed(0,255);
	    Seed[2] = GenerateSeed(0,255);
	    Seed[3] = GenerateSeed(0,255);

	    /* Storing the Value before encrypting them to compare them with the
	     * decrypted values which comes from the Tool
	     */
	    for (uint8_t idx = 0; idx < SEED_LENGTH; idx++)
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


void UDS_CompareKeys(uint8_t* ReceivedKey)
{
	for (uint8_t idx = 0; idx < SEED_LENGTH; idx++)
	{
		if (ReceivedKey[idx] !=  ComparingKey[idx])
		{
			/* Send -ve Response as the Key received from the tool is incorrect*/
			UDS_SendReponse(SendKeyNegativeResponse, InvalidKey);
			return;
		}
	}
	/* if we come here then the Received Key matches our Key */
	UDS_SendReponse(SendKeyPostiveResponse, NoNRC);

}

void UDS_SendReponse(McuResponse Reponse,NrcResponse NRC)
{
	uint8_t FrameLength = 0;
	uint8_t SendingFrame[8] = {0};
	switch (Reponse)
	{
	case DefaultSessionPostiveResponse:
		/* Adding 0x40 to the service ID */
		SendingFrame[1] = ChangeSession + 0x40;
		SendingFrame[2] = DefualtSessionRequest;
		FrameLength = 2;
		DUMMY_VARIABLE(NRC);
		break;

	case DefaultSessionNegativeResponse:
		SendingFrame[1] = NEGATIVE_REPONSE_CONSTANT_HEXA;
		SendingFrame[2] = ChangeSession;
		SendingFrame[3] = DefualtSessionRequest;
		SendingFrame[4] = NRC;
		FrameLength = 4;
		break;

	case ExtendedSessionPostiveResponse:
		/* Adding 0x40 to the service ID */
		SendingFrame[1] = ChangeSession + 0x40;
		SendingFrame[2] = ExtendedSessionRequest;
		FrameLength = 2;
		DUMMY_VARIABLE(NRC);
		break;

	case ExtendedSessionNegativeResponse:
		SendingFrame[1] = NEGATIVE_REPONSE_CONSTANT_HEXA;
		SendingFrame[2] = ChangeSession;
		SendingFrame[3] = ExtendedSessionRequest;
		SendingFrame[4] = NRC;
		FrameLength = 4;
		break;

	case BootLoaderSessionPostiveResponse:
		/* Adding 0x40 to the service ID */
		SendingFrame[1] = ChangeSession + 0x40;
		SendingFrame[2] = BootLoaderSessionRequest;
		FrameLength = 2;
		DUMMY_VARIABLE(NRC);
		break;

	case BootLoaderSessionNegativeResponse:
		SendingFrame[1] = NEGATIVE_REPONSE_CONSTANT_HEXA;
		SendingFrame[2] = ChangeSession;
		SendingFrame[3] = BootLoaderSessionRequest;
		SendingFrame[4] = NRC;
		FrameLength = 4;
		break;

	case RequestSeedPostiveResponse:
		/* Adding 0x40 to the service ID */
		SendingFrame[1] = SecurityAccess + 0x40;
		SendingFrame[2] = RequestSeed;
		FrameLength = 2;
		DUMMY_VARIABLE(NRC);
		break;

	case RequestSeedNegativeResponse:
		SendingFrame[1] = NEGATIVE_REPONSE_CONSTANT_HEXA;
		SendingFrame[2] = SecurityAccess;
		SendingFrame[3] = RequestSeed;
		SendingFrame[4] = NRC;
		FrameLength = 4;
		break;

	case SendKeyPostiveResponse:
		/* Adding 0x40 to the service ID */
		SendingFrame[1] = SecurityAccess + 0x40;
		SendingFrame[2] = SendKeyRequest;
		FrameLength = 2;
		DUMMY_VARIABLE(NRC);
		break;

	case SendKeyNegativeResponse:
		SendingFrame[1] = NEGATIVE_REPONSE_CONSTANT_HEXA;
		SendingFrame[2] = SecurityAccess;
		SendingFrame[3] = SendKeyRequest;
		SendingFrame[4] = NRC;
		FrameLength = 4;
		break;

	case RequestDownloadPostiveResponse:
		/* This Case need to be handled more carefully after reading more */
		/* Adding 0x40 to the service ID */
		SendingFrame[1] = RequestDownload + 0x40;
		/* No SubService in this service */
		FrameLength = 1;
		DUMMY_VARIABLE(NRC);
		break;

	case RequestDownloadNegativeResponse:
		SendingFrame[1] = NEGATIVE_REPONSE_CONSTANT_HEXA;
		SendingFrame[2] = RequestDownload;
		SendingFrame[3] = NRC;
		FrameLength = 3;
		break;

	case DataTransferPostiveResponse:
		/* Adding 0x40 to the service ID */
		SendingFrame[1] = TransferData + 0x40;
		SendingFrame[2] = ReceivingBuffer[2];
		FrameLength = 2;
		DUMMY_VARIABLE(NRC);
		break;

	case DataTransferNegativeResponse:
		SendingFrame[1] = NEGATIVE_REPONSE_CONSTANT_HEXA;
		SendingFrame[2] = TransferData;
		SendingFrame[3] = NRC;
		FrameLength = 3;
		break;

	case RequestExitPostiveResponse:
		/* Adding 0x40 to the service ID */
		SendingFrame[1] = RequestTransferExit + 0x40;
		FrameLength = 1;
		DUMMY_VARIABLE(NRC);
		break;

	case RequestExitNegativeResponse:
		SendingFrame[1] = NEGATIVE_REPONSE_CONSTANT_HEXA;
		SendingFrame[2] = RequestTransferExit;
		SendingFrame[3] = NRC;
		FrameLength = 3;
		break;

	default:
		/* Wrong Parameter Given */
		return;

	}
	/* Make the first Byte in the Sending Frame the frame length to receive
	   it in the PC Tool */

	SendingFrame[0] = FrameLength;
	HAL_UART_Transmit(&huart1, SendingFrame, FrameLength + 1, 1000);
}
