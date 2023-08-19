/*
 * SharedAPIs.c
 *
 *  Created on: Aug 19, 2023
 *      Author: Scorpio
 */

#include "stm32f1xx_hal.h"
#include "SharedAPIs.h"



struct APIs * SharedAPIs  = (struct APIs *) SHARED_APIS_MEMORY_ADDRESS;
