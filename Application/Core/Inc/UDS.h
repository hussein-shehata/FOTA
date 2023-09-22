/*
 * UDS.h
 *
 *  Created on: Sep 18, 2023
 *      Author: Hussein
 */

#ifndef INC_UDS_H_
#define INC_UDS_H_

#include "stm32f1xx_hal.h"

typedef enum
{
	DefaultSession,
	ExtendedSession,
	BootloaderSession
}Sessions;

typedef enum
{
	ChangeSession = 0x10,
	SecurityAccess = 0x27,
	RequestDownload = 0x34,
	TransferData = 0x36,
	RequestTransferExit = 0x37
}UDS_Frames_Service_ID;


typedef enum
{
	DefualtSessionRequest = 0x01,
	ExtendedSessionRequest = 0x03,
	BootLoaderSessionRequest = 0x02,
	RequestSeed = 0x11,
	SendKeyRequest = 0x12
}UDS_Frames_SubService_ID;


typedef enum
{
	DefaultSessionRequestCommand,
	ExtendedSessionRequestCommand,
	BootLoaderSessionRequestCommand,
	RequestSeedRequestCommand,
	SendKeyRequestCommand,
	RequestDownloadCommand,
	DataTransferCommand ,
	RequestExitCommand
}ToolCommands;


typedef enum
{

	DefaultSessionPostiveResponse,
	DefaultSessionNegativeResponse,

	ExtendedSessionPostiveResponse,
	ExtendedSessionNegativeResponse,

	BootLoaderSessionPostiveResponse,
	BootLoaderSessionNegativeResponse,

	RequestSeedPostiveResponse,
	RequestSeedNegativeResponse,

	SendKeyPostiveResponse,
	SendKeyNegativeResponse,

	RequestDownloadPostiveResponse,
	RequestDownloadNegativeResponse,

	DataTransferPostiveResponse,
	DataTransferNegativeResponse,

	RequestExitPostiveResponse,
	RequestExitNegativeResponse,
}McuResponse;

typedef enum
{
	NoNRC = 0xFF,   /* Used to be sent as parameter in send response function when it is +ve response */
	ServiceNotSupported = 0x11,
	IncorrectMessageLengthOrInvalidFormat = 0x13,
	InvalidKey = 0x33
}NrcResponse;

void UDS_MainFunction(void);
void UDS_ReceiveCommand(void);
void UDS_SendHandShake(void);
void UDS_SendReponse(McuResponse Reponse, NrcResponse NRC);
void UDS_ChangeSession(uint8_t RequestedSession);
void UDS_SendSeed(void);
void UDS_CompareKeys(uint8_t* Buffer);


#endif /* INC_UDS_H_ */
