/*
 * SharedData.h
 *
 *  Created on: Sep 11, 2023
 *      Author: Scorpio
 */

#ifndef INC_SHAREDDATA_H_
#define INC_SHAREDDATA_H_

#include "stm32f1xx_hal.h"

typedef struct {
//	UART_HandleTypeDef huart1;
    uint8_t DownloadRequestedFromApplication;
    // Add more shared variables as needed
} SharedData;

//extern uint8_t DownloadRequestedFromApplication ;
extern volatile SharedData SharedStruct;

#endif /* INC_SHAREDDATA_H_ */
