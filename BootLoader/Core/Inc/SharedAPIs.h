/*
 * SharedAPIs.h
 *
 *  Created on: Aug 19, 2023
 *      Author: Scorpio
 */

#ifndef INC_SHAREDAPIS_H_
#define INC_SHAREDAPIS_H_

#include "stm32f1xx_hal.h"


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

/* Put Here the Shared APIs name and edit them in the .c File to initialize them */
typedef struct
{
	API ToggleLed;
	API PrintHelloScreen;
}APIs;




#define LOCATE_SHARED_API		__attribute__((section(".API_SHARED_Section")))

extern APIs SharedAPIs ;

void InitSharedAPIs(void);
void LOCATE_SHARED_API ToggleTestLed(void);
void LOCATE_SHARED_API PrintHelloScreen(void);

#endif /* INC_SHAREDAPIS_H_ */
