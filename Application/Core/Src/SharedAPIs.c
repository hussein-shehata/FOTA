/*
 * SharedAPIs.c
 *
 *  Created on: Aug 19, 2023
 *      Author: Scorpio
 */

#include "stm32f1xx_hal.h"
#include "SharedAPIs.h"


/* b3ml pointer points to the memory address of the shared APIs 3shan a3rf ast5dm el functions*/


struct APIs * SharedAPIs  = (struct APIs *) SHARED_APIS_MEMORY_ADDRESS;
