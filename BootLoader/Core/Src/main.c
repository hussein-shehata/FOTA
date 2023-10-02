/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "BL_Functions.h"
#include "SharedAPIs.h"
//#include "Print.h"
#include "Flashing.h"
#include "NVM.h"
#include "SharedData.h"
#include "Print.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef uint8_t	uint8;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define 	MAJOR_VERSION		0
#define 	MINOR_VERSION		5

#define 	JUST_FLASHED_4BYTES		0xAAAAAAAA

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
const uint8 BL_Version[2] = {MAJOR_VERSION, MINOR_VERSION};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//NVM_VARIABLE(uint32_t,Counter) = 0;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
//#define NVM_TEST
//	BlockIDs NVM_Blocks;
#ifdef NVM_TEST
	uint32_t* u32_Data = 0;
	NVM_ReadBlock(JustFlashed, u32_Data);

	if (*u32_Data != JUST_FLASHED_4BYTES)
	{
		/* This Software has just been Flashed */
		NVM_WriteBlock(JustFlashed,(uint32_t*) JUST_FLASHED_4BYTES);
		NVM_WriteBlock(BootCounter,(uint32_t*) 1);
		/* w hna n3ml flash l el Application software */
	}
	else
	{
		uint32_t* BootCounterFromNVM = 0;
		NVM_ReadBlock(BootCounter, BootCounterFromNVM);

		NVM_WriteBlock(BootCounter,(uint32_t*) BootCounterFromNVM + 1);
		printf("this is the Boot Number = %ul",*BootCounterFromNVM);
	}
#endif
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/
	if(SharedStruct.DownloadRequestedFromApplication != 1)
	{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
		HAL_Init();
	}
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();




  /* USER CODE BEGIN SysInit */
//	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
//  printf("Starting BootLoader (%d.%d)\r\n",BL_Version[0],BL_Version[1]);



  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	uint8_t Status;
	Status = CheckIfAppCorupted();
	if (Status == 1)
	{
		uint8_t StatusFlashing = FlashApplication();
	}
	else if (SharedStruct.DownloadRequestedFromApplication == 1)
	{
		uint8_t StatusFlashing = FlashApplication();
	}

  while (1)
  {
#define SHARED_APIS_TEST
#ifdef SHARED_APIS_TEST

//	  SharedAPIs->ToggleLedPtr();
//	  HAL_Delay(1000);
//	  SharedAPIs->ToggleLedPtr();

//		Counter ++;
//		u32 address = 0x800EC08;
//		u32* ptr = 0x800EC08;
//		u32* BootCounter = 0x800EC04;
//		u32* BootCounterAddress = 0x800EC04;

#endif
		uint32_t DataToBeWritten = 0xAABBCCDD;
		uint32_t DataBuffer;
#ifdef GITHUB_FLASH

		if (*ptr != 0x55555555)
		{
			/* we have just flashed */
			status = FLASH_Unlock();
			status = FLASH_MultiplePageErase(address,1);
			status = FLASH_WriteWord((u32*)address,0x55555555);

			status = FLASH_WriteWord((u32*)BootCounterAddress,0);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13,GPIO_PIN_SET);
			printf("We didnot Found the Saved Address\r\n");
		}
		else
		{
			uint32_t Counter = *BootCounter;
			Counter = Counter + 1;
		status = FLASH_WriteWord((u32*)BootCounterAddress,Counter);
		printf("We Booted This Software %d Times\r\n", Counter);
		}
#endif


#ifdef TESTING_NVM
		Status = FlashUnlock();
		Status = NVM_WriteAddress(AppCounter, &DataToBeWritten);
		FlashLock();
		HAL_Delay(100);
		DataToBeWritten = 0x11223344;
		Status = FlashUnlock();
		Status = NVM_OverWriteAddress(AppCounter, &DataToBeWritten);
		FlashLock();

		if(EnterOnce == 1)
		{
			EnterOnce = 0;
			NVM_ReadAddress(BootCounter, &DataBuffer);
			if (DataBuffer == 0xFFFFFFFF)
			{
				/* This is the first time boot */
				DataToBeWritten = 0;
				Status = FlashUnlock();
				Status = NVM_WriteAddress(BootCounter, &DataToBeWritten);
				FlashLock();
				printf("I have Wrote zero in the section\r\n");
			}
			else
			{
				DataToBeWritten = DataBuffer + 1;
				Status = FlashUnlock();
				Status = NVM_OverWriteAddress(BootCounter, &DataToBeWritten);
				Status = FlashUnlock();
				printf("I have Wrote %d in the Section, we entered the else\r\n",DataToBeWritten);
			}

		}
#endif

		if(SharedStruct.GoToBootloaderRequested == 1 )
		{
			SharedAPIs->UDS_MainFunction();
		}
		else
		{
//			GoToApplication();
			UDS_MainFunction();
			HAL_Delay(1000);
		}

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
		Error_Handler();
	}
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
//#ifdef __GNUC__
//  /* With GCC, small printf (option LD Linker->Libraries->Small printf
//     set to 'Yes') calls __io_putchar() */
//int __io_putchar(int ch)
//#else
//int fputc(int ch, FILE *f)
//#endif /* __GNUC__ */
//{
//  /* Place your implementation of fputc here */
//  /* e.g. write a character to the UART3 and Loop until the end of transmission */
//  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
//
//  return ch;
//}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
