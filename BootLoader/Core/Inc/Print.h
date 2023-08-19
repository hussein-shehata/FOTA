/*
 * Print.h
 *
 *  Created on: Aug 19, 2023
 *      Author: Scorpio
 */

#ifndef INC_PRINT_H_
#define INC_PRINT_H_

#include <stdio.h>
#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef huart1;

#ifdef __GNUC__
  /* With GCC, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
int __io_putchar(int ch);
#else
int fputc(int ch, FILE *f);
#endif /* __GNUC__ */
//{
//  /* Place your implementation of fputc here */
//  /* e.g. write a character to the UART3 and Loop until the end of transmission */
//  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
//
//  return ch;
//}


#endif /* INC_PRINT_H_ */
