/*
 * Functions.h
 *
 *  Created on: Aug 19, 2023
 *      Author: Scorpio
 */

#ifndef INC_APP_FUNCTIONS_H_
#define INC_APP_FUNCTIONS_H_

#include "stm32f1xx_hal.h"

void GoToBootLoader(void);
void ToggleTestLed(void);
void PrintHelloScreen(void);
void RequestNewSoftware(void);
uint8_t CheckIfAppCorupted(void);

#endif /* INC_APP_FUNCTIONS_H_ */
