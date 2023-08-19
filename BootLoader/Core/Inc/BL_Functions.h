/*
 * Functions.h
 *
 *  Created on: Aug 19, 2023
 *      Author: Scorpio
 */

#ifndef INC_BL_FUNCTIONS_H_
#define INC_BL_FUNCTIONS_H_

#include "stm32f1xx_hal.h"


#define TRUE	 1
#define FALSE	 0

//#define SHARED_API		__attribute__((section(".API_SHARED_Section")))


void GoToApplication(void);
//void SHARED_API ToggleTestLed(void);
//void PrintHelloScreen(void);
void DownloadNewSoftware(void);
uint8_t CheckIfAppCorupted(void);

#endif /* INC_BL_FUNCTIONS_H_ */
