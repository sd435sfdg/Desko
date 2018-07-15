
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"

/* USER CODE BEGIN Includes */
#include "WordClock.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
RTC_TimeTypeDef rtc_time;
//RTC_DateTypeDef rtc_date;
RTC_AlarmTypeDef alarm_time;
bool isAlarmInterrupt = false;
bool isRTCInterrupt = false;
bool isSetAlarm = false;
bool isEditButtonPressed = false;
bool isUpButtonPressed = false;
bool isDownButtonPressed = false;
bool isAlarmButtonPressed = false;
uint8_t isBuzzerRinging = 0;
typedef enum {
	WORD, TIME, ALARM
}MODE;
MODE currentMode = WORD;
typedef enum {
	NONE, TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT
}EDIT_POSITION;	
EDIT_POSITION currentEditPosition = NONE;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_RTC_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc);
void HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void edit_button(void);
void up_button(void);
void down_button(void);
void edit_number(uint8_t* number, int8_t operate, uint8_t param);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
	led_init(&hspi1);
	HAL_RTCEx_SetSecond_IT(&hrtc);
	NVIC_EnableIRQ(RTC_IRQn);
	HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
	isSetAlarm = false;
	alarm_time.AlarmTime.Hours = 0;
  alarm_time.AlarmTime.Minutes = 0;
  alarm_time.AlarmTime.Seconds = 1;
  alarm_time.Alarm = RTC_ALARM_A;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
		if(isAlarmInterrupt){
			HAL_GPIO_WritePin(BZ_PORT, BZ_PIN, GPIO_PIN_SET);
			isBuzzerRinging = 1;
			isAlarmInterrupt = false;
		}
		if(isAlarmButtonPressed){
			if(isBuzzerRinging){
				HAL_GPIO_WritePin(BZ_PORT, BZ_PIN, GPIO_PIN_RESET);
				isBuzzerRinging = 0;				
			}
			if(isSetAlarm) {
				isSetAlarm = false;
				HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
			}
			else {
				isSetAlarm = true;
				HAL_RTC_SetAlarm_IT(&hrtc, &alarm_time, RTC_FORMAT_BIN);
			}
			isAlarmButtonPressed = false;
		}
		if(isEditButtonPressed){
			if(isBuzzerRinging){
				HAL_GPIO_WritePin(BZ_PORT, BZ_PIN, GPIO_PIN_RESET);
				isBuzzerRinging = 0;				
			}
			edit_button();
			isEditButtonPressed = false;
			edit_position(&hspi1, currentEditPosition);
		}	
		if(isUpButtonPressed){
			if(isBuzzerRinging){
				HAL_GPIO_WritePin(BZ_PORT, BZ_PIN, GPIO_PIN_RESET);
				isBuzzerRinging = 0;				
			}
			up_button();
			isUpButtonPressed = false;
		}
		if(isDownButtonPressed){
			if(isBuzzerRinging){
				HAL_GPIO_WritePin(BZ_PORT, BZ_PIN, GPIO_PIN_RESET);
				isBuzzerRinging = 0;				
			}
			down_button();
			isDownButtonPressed = false;
		}
		if(isRTCInterrupt){
			if(isBuzzerRinging){
				if(isBuzzerRinging == 21){
					HAL_GPIO_WritePin(BZ_PORT, BZ_PIN, GPIO_PIN_RESET);
					isBuzzerRinging = 0;
				}
				else isBuzzerRinging++;
			}
			HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
//			HAL_RTC_GetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);
			switch(currentMode)
			{
				case WORD:{		
					display_word(&hspi1, rtc_time.Hours, rtc_time.Minutes, rtc_time.Seconds);				
					break;
				}
				case TIME:{
					display_number(&hspi1, rtc_time.Hours, rtc_time.Minutes);
					edit_position(&hspi1, currentEditPosition);					
					break;
				}
//				case DATE:{
//					display_number(&hspi1, rtc_date.Date, rtc_date.Month);
//					break;
//				}
				case ALARM:{
					display_number(&hspi1, alarm_time.AlarmTime.Hours, alarm_time.AlarmTime.Minutes);
					break;
				}
			}
			isRTCInterrupt = false;
		}
  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* RTC init function */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef DateToUpdate;
  RTC_AlarmTypeDef sAlarm;

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

    /**Initialize RTC Only 
    */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
  /* USER CODE BEGIN RTC_Init 2 */
	if( HAL_RTC_GetState(& hrtc)== HAL_RTC_STATE_RESET) 
	{
  /* USER CODE END RTC_Init 2 */

    /**Initialize RTC and set the Time and Date 
    */
  sTime.Hours = 6;
  sTime.Minutes = 27;
  sTime.Seconds = 0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
  /* USER CODE BEGIN RTC_Init 3 */

  /* USER CODE END RTC_Init 3 */

  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
  DateToUpdate.Month = RTC_MONTH_JULY;
  DateToUpdate.Date = 14;
  DateToUpdate.Year = 18;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
  /* USER CODE BEGIN RTC_Init 4 */

  /* USER CODE END RTC_Init 4 */

    /**Enable the Alarm A 
    */
  sAlarm.AlarmTime.Hours = 4;
  sAlarm.AlarmTime.Minutes = 31;
  sAlarm.AlarmTime.Seconds = 1;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
  /* USER CODE BEGIN RTC_Init 5 */
	}
  /* USER CODE END RTC_Init 5 */

}

/* SPI1 init function */
static void MX_SPI1_Init(void)
{

  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_16BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6|GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA6 PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
	isAlarmInterrupt = true;
}

void HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc)
{
	if(RTC_IT_SEC) {
		isRTCInterrupt = true;
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
		if (GPIO_Pin == GPIO_PIN_15)
	 {
			isEditButtonPressed = true;
	 }
		if (GPIO_Pin == GPIO_PIN_14)
	 {
			isUpButtonPressed = true;
	 }
	 	if (GPIO_Pin == GPIO_PIN_13)
	 {
			isDownButtonPressed = true;
	 }
	 if (GPIO_Pin == GPIO_PIN_12)
	 {
			isAlarmButtonPressed = true;
	 }
}

void edit_button(void)
{
	if(currentMode == WORD) return;
	uint8_t temp = currentEditPosition; 
	temp = (temp == BOTTOM_RIGHT)? NONE: temp + 1;
	currentEditPosition = (EDIT_POSITION)temp;
}

void up_button(void)
{
		 if(currentEditPosition == NONE){
			 uint8_t temp = currentMode; 
			 temp = (temp == ALARM)? WORD: temp + 1;
			 currentMode = (MODE)temp;
		 }
		 else{
			 switch(currentMode)
			{
				case TIME:{
					switch (currentEditPosition)
					{
						case TOP_LEFT: {
							edit_number(&rtc_time.Hours, 10, 14);
							break;
						}
						case TOP_RIGHT: {
							edit_number(&rtc_time.Hours, 1, 23);
							break;
						}
						case BOTTOM_LEFT: {
							edit_number(&rtc_time.Minutes, 10, 50);
							break;
						}
						case BOTTOM_RIGHT: {
							edit_number(&rtc_time.Minutes, 1, 59);
							break;
						}
						default: break;
					}
					HAL_RTC_SetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
					break;
				}
//				case DATE:{
//					switch (currentEditPosition)
//					{
//						case TOP_LEFT: {
//							edit_number(&rtc_date.Date, 10, 22);
//							break;
//						}
//						case TOP_RIGHT: {
//							edit_number(&rtc_date.Date, 1, 31);
//							break;
//						}
//						case BOTTOM_LEFT: {
//							edit_number(&rtc_date.Month, 10, 3);
//							break;
//						}
//						case BOTTOM_RIGHT: {
//							edit_number(&rtc_date.Month, 1, 12);
//							break;
//						}
//						default: break;
//					}
//					HAL_RTC_SetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);
//					break;
//				}
				case ALARM:{
					switch (currentEditPosition)
					{
						case TOP_LEFT: {
							edit_number(&alarm_time.AlarmTime.Hours, 10, 14);
							break;
						}
						case TOP_RIGHT: {
							edit_number(&alarm_time.AlarmTime.Hours, 1, 23);
							break;
						}
						case BOTTOM_LEFT: {
							edit_number(&alarm_time.AlarmTime.Minutes, 10, 50);
							break;
						}
						case BOTTOM_RIGHT: {
							edit_number(&alarm_time.AlarmTime.Minutes, 1, 59);
							break;
						}
						default: break;
					}
					break;
				}
				default: break;
			}
		 }
}	

void down_button(void)
{
		 if(currentEditPosition == NONE){
			 uint8_t temp = currentMode; 
			 temp = (temp == WORD)? ALARM: temp - 1;
			 currentMode = (MODE)temp;
		 }	
		 else{
				switch(currentMode)
			{
				case TIME:{
					switch (currentEditPosition)
					{
						case TOP_LEFT: {
							edit_number(&rtc_time.Hours, -10, 9);
							break;
						}
						case TOP_RIGHT: {
							edit_number(&rtc_time.Hours, -1, 0);
							break;
						}
						case BOTTOM_LEFT: {
							edit_number(&rtc_time.Minutes, -10, 9);
							break;
						}
						case BOTTOM_RIGHT: {
							edit_number(&rtc_time.Minutes, -1, 0);
							break;
						}
						default: break;
					}
					HAL_RTC_SetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
					break;
				}
//				case DATE:{
//					switch (currentEditPosition)
//					{
//						case TOP_LEFT: {
//							edit_number(&rtc_date.Date, -10, 9);
//							break;
//						}
//						case TOP_RIGHT: {
//							edit_number(&rtc_date.Date, -1, 1);
//							break;
//						}
//						case BOTTOM_LEFT: {
//							edit_number(&rtc_date.Month, -10, 9);
//							break;
//						}
//						case BOTTOM_RIGHT: {
//							edit_number(&rtc_date.Month, -1, 1);
//							break;
//						}
//						default: break;
//					}
//					HAL_RTC_SetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);
//					break;
//				}
				case ALARM:{
					switch (currentEditPosition)
					{
						case TOP_LEFT: {
							edit_number(&alarm_time.AlarmTime.Hours, -10, 9);
							break;
						}
						case TOP_RIGHT: {
							edit_number(&alarm_time.AlarmTime.Hours, -1, 0);
							break;
						}
						case BOTTOM_LEFT: {
							edit_number(&alarm_time.AlarmTime.Minutes, -10, 9);
							break;
						}
						case BOTTOM_RIGHT: {
							edit_number(&alarm_time.AlarmTime.Minutes, -1, 0);
							break;
						}
						default: break;
					}
					break;
				}
				default: break;
			}
		 }		 
}

void edit_number(uint8_t* number, int8_t operate, uint8_t param)
{
	switch(operate)
	{
		case 1:
		{
			*number = (*number == param) ? *number : *number + 1;
			break;
		}
		case -1:
		{
			*number = (*number == param) ? *number : *number - 1;
			break;
		}
		case 10:
		{
			*number = (*number >= param) ? *number : *number + 10;
			break;
		}
		case -10:
		{
			*number = (*number <= param) ? *number : *number - 10;
			break;
		}
		default: break;
	}
}

bool getAlarmStatus(void)
{
	return isSetAlarm;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
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
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
