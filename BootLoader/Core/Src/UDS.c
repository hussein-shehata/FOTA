/*
 * UDS.c
 *
 *  Created on: Sep 18, 2023
 *      Author: Hussein
 */

/* *************TODO in this file*************** */

/* 1- Handle the case of request download positve response in send response function as the frame is diffcult to understand now */
/* 2- Make Flags to grant access to each of the requested request */
/* 3- Check that we have received all the Data in Data Transfer Function using Appsize member in the structure RequestDownloadFrame */
/* 4- Decrypt the Data recevied from Data Transfer Frame as it come in encrypted way */
/* 5- Make use of the Data Slot member in the struct */
/* 6- Function to Mass Erase the flash memory upon request */
#include "UDS.h"
#include "Bit_Math.h"
#include "aes.h"
#include "BL_Functions.h"
#include "Flashing.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define DUMMY_VARIABLE(x)		(void)x

#define RXNE_BIT							5
#define DUMMY_BYTE							0xAA
#define SEED_LENGTH							16
#define NEGATIVE_REPONSE_CONSTANT_HEXA		0x7F
#define AES_KEY_SIZE						16

/* 2 Bytes Payload + 128 Data in Transfer Data */
#define MAX_DATA_RECEIVED					128
#define MAX_SIZE_BUFFER						130

#define APP_FLASH_START_ADDR 					0x8005000   //Application's Flash Address
#define APPLICATION_NUMBER_OF_PAGES				39			//Number of pages in Application Region

extern UART_HandleTypeDef huart1;
/* Make it private to be unaccessible from outside this Module */
static uint8_t ReceivingBuffer[MAX_SIZE_BUFFER] = {0};
static uint8_t DataEncryptingKey [AES_KEY_SIZE] = {0};

/* I will use 4 Bytes seed and key */
static uint8_t Seed[SEED_LENGTH] = {0};


/* As We are in UDS of the BootLoader Not the Application */
static uint8_t CurrentSession = BootloaderSession;
/* Pointer to the Register which indicates the Status of the UART */
static volatile uint32_t* USART_SR = (uint32_t*)0x40013800;

/* Private Bit-Field Structure Flags to indicate what process is available
 * depending on the Requests Made by the user before
 */
/* I made it Bit Field to optimize Memory as we are going to use only 1 bit for each flag */
typedef struct
{

	ValuesForUDSFlag RequestDownload : 1;
	ValuesForUDSFlag RequestSeed :1;
	ValuesForUDSFlag ComparingKey : 1;
	ValuesForUDSFlag EraseMemory : 1;

}UDS_Flags;

static UDS_Flags RequestsFlags =
{

	.RequestDownload = Failed,
	.RequestSeed     = Failed,
	.ComparingKey    = Failed,
	.EraseMemory     = Failed
};

typedef struct
{
	uint8_t DataFormat;
	uint8_t SlotUsed;
	uint16_t AppSize;
}RequestDownloadFrameLayout;

static RequestDownloadFrameLayout RequestDownloadFrame;

static uint8_t ComparingKey[SEED_LENGTH] ; /* Variable to hold the value of the Key to approve the security access */

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
	HAL_UART_Receive(&huart1, DataLength, 1, 1000);
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
			/* Make Sure that the Tool Has Requested Seed Before */
			if (RequestsFlags.RequestSeed == Success)
			{
				RequestsFlags.RequestSeed = Failed;
				UDS_CompareKeys(&(ReceivingBuffer[2]));
			}
			else
			{
				/* Send -ve Response */
				UDS_SendReponse(SendKeyNegativeResponse, ConditionsNotCorrect);
			}
		}
		else
		{
			// Do Nothing
		}
	}
	else if(ReceivingBuffer[0] == RequestDownload)
	{
		/* The Received Command is to Request Download */
		/* Make Sure that the Keys Have been Matched before */
		if(RequestsFlags.ComparingKey == Success)
		{
			UDS_RequestDownload();
		}
		else
		{
			/* Send -ve Response */
			UDS_SendReponse(RequestDownloadNegativeResponse, ConditionsNotCorrect);
		}
	}
	else if(ReceivingBuffer[0] == TransferData)
	{
		/* The Received Command is to TransferData */
		/* Make Sure that Request Download Has been Approved */
		if(RequestsFlags.RequestDownload == Success && RequestsFlags.EraseMemory == Success)
		{
			/* I Removed the PayLoad 2 Bytes to have the Actual Data of the Application Binary File Length */
			UDS_TransferData( DataLength[0] - 2);
		}
	}
	else if(ReceivingBuffer[0] == 0x31 && ReceivingBuffer[1] == 0x01 && ReceivingBuffer[2] == 0xFF && ReceivingBuffer[3] == 0x00)
	{
		UDS_MassErase();
		RequestsFlags.EraseMemory = Success;
	}

}

void UDS_SendHandShake(void)
{
	uint8_t SendingBuffer[1] = {DUMMY_BYTE};
	HAL_UART_Transmit(&huart1, SendingBuffer, 1, 100);
}

void UDS_ChangeSession(uint8_t RequestedSession)
{
	/* We can not change to Extended Session unless we are in Application  */
	/* Otherwise we can change to any session from any session */
	if (RequestedSession == ExtendedSession )
	{
		/* Negative Response */
		UDS_SendReponse(ExtendedSessionNegativeResponse, ServiceNotSupported);
	}
	else if (RequestedSession == BootloaderSession)
	{
		CurrentSession = BootloaderSession;
		UDS_SendReponse(BootLoaderSessionPostiveResponse, NoNRC);
	}
	else if (RequestedSession == DefaultSession)
	{
		CurrentSession = DefaultSession;
		UDS_SendReponse(DefaultSessionPostiveResponse, NoNRC);
		GoToApplication();
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
//	    uint8_t Seed[SEED_LENGTH] = {0};
	    /* To Randomize the Seed Generated */
		uint32_t Test = HAL_GetTick();
	    srand(Test);
	    /* Generate Number in between 0 -> 255 */
//	    Seed[0] = GenerateSeed(0,255);
//	    Seed[1] = GenerateSeed(0,255);
//	    Seed[2] = GenerateSeed(0,255);
//	    Seed[3] = GenerateSeed(0,255);

	    /* Storing the Value before encrypting them to compare them with the
	     * decrypted values which comes from the Tool
	     */
	    for (uint8_t idx = 0; idx < SEED_LENGTH; idx++)
	    {
	    	Seed[idx] = GenerateSeed(0,255);
	    	ComparingKey[idx] = Seed[idx];
	    }
	    /* The Key used to encrypt and decrypt the data */
	    /* It is a private key which is only known to the 2 parties " the Tool and the ECU" */
	    uint8_t Seed_Encrypting_Key[SEED_LENGTH] = { 0x2b, 0x7e, 0x15, 0x16};

	    struct AES_ctx ctx;

	    AES_init_ctx(&ctx, Seed_Encrypting_Key);
	    AES_ECB_encrypt(&ctx, Seed);
	//    printf("Seed After Encrypting is : %x %x %x %x\r\n",Seed[0],Seed[1], Seed[2], Seed[3]);

	    /* Sending the Seed by Uart to the tool */
//	    HAL_UART_Transmit(&huart1, Seed, 4, 1000);
//	    uint8_t Testing[4] = {0};

	    UDS_SendReponse(RequestSeedPostiveResponse, NoNRC);
	    RequestsFlags.RequestSeed = Success;

}


void UDS_CompareKeys(uint8_t* ReceivedKey)
{
	for (uint8_t idx = 0; idx < SEED_LENGTH; idx++)
	{
		if (ReceivedKey[idx] !=  ComparingKey[idx])
		{
			/* Send -ve Response as the Key received from the tool is incorrect*/
			UDS_SendReponse(SendKeyNegativeResponse, InvalidKey);
			RequestsFlags.ComparingKey = Failed;
			return;
		}
	}
	/* if we come here then the Received Key matches our Key */
	UDS_SendReponse(SendKeyPostiveResponse, NoNRC);
	RequestsFlags.ComparingKey = Success;

}

/* RequestFrame Bytes :
 * Byte 0   -> SID
 * Byte 1   -> DataFormat
 * Byte 2   -> Slot Used
 * Byte 3-4 -> Size of Application Binary File
 */
void UDS_RequestDownload()
{
	RequestDownloadFrame.DataFormat = ReceivingBuffer[1];
	RequestDownloadFrame.SlotUsed = ReceivingBuffer[2];
	/* Take Byte 3 and 4 in the AppSize Variable */
	RequestDownloadFrame.AppSize = ReceivingBuffer[4] | (ReceivingBuffer[3] << 8);

	/* Know the used Encryption Technique " First 4 Bits in the DataFormat" */
	uint8_t EncryptionTechnique  = (RequestDownloadFrame.DataFormat & 0x0F) ;
	/* Know the used Compression Technique " Second 4 Bits in the DataFormat" */
	uint8_t CompressionTechnique = ((RequestDownloadFrame.DataFormat & 0xF0) >> 4); /* No Compression Technique Implemented Yet */

	/* Make Sure that the Encryption Technique Sent is Available */
	if (EncryptionTechnique > Key3Encryption )
	{
		/* Wrong Value */
		UDS_SendReponse(RequestDownloadNegativeResponse, RequestOutOfRange);
		RequestsFlags.RequestDownload = Failed;
		return;
	}
	ChangeDataEncryptingKey(EncryptionTechnique);
	RequestsFlags.RequestDownload = Success;
	UDS_SendReponse(RequestDownloadPostiveResponse, NoNRC);


}

void ChangeDataEncryptingKey(EncryptionTechniques Technique)
{
	uint8_t KeyByte = 0;	/* Will be used to make the Encryption Key */
	if (Technique == Key1Encryption)
	{
		KeyByte = 0xAB;
	}
	else if (Technique == Key2Encryption)
	{
		KeyByte = 0xCD;
	}
	else if (Technique == Key3Encryption)
	{
		KeyByte = 0xEF;
	}
	else
	{
		// Do Nothing
	}


	for (uint16_t idx = 0; idx < AES_KEY_SIZE; idx++)
	{
		DataEncryptingKey[idx] = KeyByte;
	}
}


/* Data Transfer Frame Layout :
 * Byte 0 -> SID
 * Byte 1 -> Block Number
 * Byte 2-n -> Actual Data with Length = DataLength
 */
void UDS_TransferData(const uint8_t DataLength)
{
	static uint8_t PreviusBlockNumber = 0;
	/* The Size of the Received Data So far to check that I Have received all the Data */
	static uint16_t AccumlativeReceivedDataSize = 0;

	if(DataLength > MAX_DATA_RECEIVED)
	{
		/* This means I have Received more than 128 Bytes which is the Max Data Bytes
		 * I can receive */
		UDS_SendReponse(DataTransferNegativeResponse, IncorrectMessageLengthOrInvalidFormat);
		return;
	}

	uint8_t BlockNumber = ReceivingBuffer[1];
	uint8_t* Data = &ReceivingBuffer[2];

	if (BlockNumber != PreviusBlockNumber + 1)
	{
		/* Wrong BlockNumber */
		UDS_SendReponse(DataTransferNegativeResponse, WrongBlockSequenceCounter);
		return;
	}

	/* Steps to Write The Data in the Flash Region of the Application */

	if( DataLength < MAX_DATA_RECEIVED )
	{
		/* Padding the Remaining Bytes with 0xFF */
		/* We need to padding the last 3 bytes because if we have data length
		 * not divisible by 4 like 10 byte we need to padd byte 11 and 12 and write 12 bytes
		 * in the flash memory not 10 as we write in 4 bytes not 1 bytes
		 */
		for (uint8_t idx = DataLength ; idx < DataLength + 4; idx++)
		{
			Data[idx] = 0xFF;
		}
	}

	uint32_t* DataToBeWritten = (uint32_t* )&ReceivingBuffer[2];
	FlashUnlock();
	for (uint8_t idx = 0; idx < (MAX_DATA_RECEIVED / 4); idx++)
	{
		Flash_WriteAddress(APP_FLASH_START_ADDR + AccumlativeReceivedDataSize + (idx * 0x04 ) , &DataToBeWritten[idx]);
	}
	FlashLock();


	AccumlativeReceivedDataSize = AccumlativeReceivedDataSize + DataLength;
	UDS_SendReponse(DataTransferPostiveResponse, NoNRC);
	PreviusBlockNumber++;
	if( AccumlativeReceivedDataSize == RequestDownloadFrame.AppSize)
	{
		/* Then we Received all the Application Binary file */
		RequestsFlags.RequestDownload = Failed;
		RequestsFlags.EraseMemory = Failed;
		AccumlativeReceivedDataSize = 0 ;
	}
}

/* Upon Receiving the request to Erase all of the region of the application */
void UDS_MassErase()
{
	FlashUnlock();
	for (uint32_t PageAddress = 0; PageAddress < APPLICATION_NUMBER_OF_PAGES; PageAddress++)
	{
		Flash_ErasePage(APP_FLASH_START_ADDR + (PageAddress * 1024) );
	}
	FlashLock();
	UDS_SendReponse(MassErasePositiveReponse, NoNRC);
}

void UDS_SendReponse(McuResponse Reponse,NrcResponse NRC)
{
	uint8_t FrameLength = 0;
	uint8_t SendingFrame[18] = {0};
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
		SendingFrame[3] = Seed[0];
		SendingFrame[4] = Seed[1];
		SendingFrame[5] = Seed[2];
		SendingFrame[6] = Seed[3];
		SendingFrame[7] = Seed[4];
		SendingFrame[8] = Seed[5];
		SendingFrame[9] = Seed[6];
		SendingFrame[10] = Seed[7];
		SendingFrame[11] = Seed[8];
		SendingFrame[12] = Seed[9];
		SendingFrame[13] = Seed[10];
		SendingFrame[14] = Seed[11];
		SendingFrame[15] = Seed[12];
		SendingFrame[16] = Seed[13];
		SendingFrame[17] = Seed[14];
		SendingFrame[18] = Seed[15];


		FrameLength = 18;
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
		/* TODO This Case need to be handled more carefully after reading more */
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

		/* Mass Erase Not handled correctly yet as it is a routine not a servie ID */
	case MassErasePositiveReponse:
		/* Adding 0x40 to the service ID */
		SendingFrame[1] = RoutineControl + 0x40;
		SendingFrame[2] = RoutineStart;
		FrameLength = 2;
		DUMMY_VARIABLE(NRC);
		break;

	case MassEraseNegativeReponse:
		SendingFrame[1] = NEGATIVE_REPONSE_CONSTANT_HEXA;
		SendingFrame[2] = RoutineControl;
		SendingFrame[3] = RoutineStart;
		SendingFrame[4] = NRC;
		FrameLength = 4;
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
