/*
 * SharedAPIs.h
 *
 *  Created on: Aug 19, 2023
 *      Author: Scorpio
 */

#ifndef INC_SHAREDAPIS_APP_H_
#define INC_SHAREDAPIS_APP_H_

#include "stm32f1xx_hal.h"

#define 	SHARED_APIS_MEMORY_ADDRESS		0x8004C00



typedef enum{
	FROM_BOOTLOADER,
	FROM_APPLICATION,
	FROM_UNKNOWN_SOURCE
}SourceCallingEnum;


typedef struct
{
	void (*ToggleLedPtr) (void) ;
//	uint8_t (*FlashNewSoftwarePtr) (void) ;
	void (*UDS_MainFunction) (void) ;
}APIs;



extern APIs* SharedAPIs ;


void  ToggleTestLed(void);

#endif /* INC_SHAREDAPIS_APP_H_ */
