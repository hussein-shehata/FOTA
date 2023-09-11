/*
 * SharedData.c
 *
 *  Created on: Sep 11, 2023
 *      Author: Scorpio
 */


#include "SharedData.h"

volatile SharedData SharedStruct __attribute__((section(".SharedDataSection")));
