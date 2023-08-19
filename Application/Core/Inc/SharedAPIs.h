/*
 * SharedAPIs.h
 *
 *  Created on: Aug 19, 2023
 *      Author: Scorpio
 */

#ifndef INC_SHAREDAPIS_H_
#define INC_SHAREDAPIS_H_


#define 	SHARED_APIS_MEMORY_ADDRESS		0x8003C00

extern struct APIs * SharedAPIs ;

typedef enum{
	FROM_BOOTLOADER,
	FROM_APPLICATION,
	FROM_UNKNOWN_SOURCE
}SourceCallingEnum;

typedef struct
{
	SourceCallingEnum SourceCalling;
	void (*PtrFunction) (void);
}API;

typedef struct
{
	API ToggleLed;
	API PrintHelloScreen;
}APIs;




#endif /* INC_SHAREDAPIS_H_ */
