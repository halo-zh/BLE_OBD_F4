/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */
#include "usart.h"
#include "ble_app.h"
#include "can.h"
#include "isotp_user.h"
void toggleLeds(void)
{
  if(isBLEConnect ==1)
  {
    HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_0);
  }
  else
  {
    HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_SET);
  }
  
  if(CANDataAvalFlag >= 1)
  {
    HAL_GPIO_TogglePin(LED_Y_GPIO_Port,LED_Y_Pin);
  }
  else
  {
    HAL_GPIO_WritePin(LED_Y_GPIO_Port,LED_Y_Pin,GPIO_PIN_SET);
  }
  
  
  if(isOBDConnect>0)
  {
     isOBDConnect--;
     HAL_GPIO_TogglePin(LED_R_GPIO_Port,LED_R_Pin);
  }
  else
  {
    HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_SET);
  }
  
  
   HAL_GPIO_TogglePin(LED_G_GPIO_Port,LED_G_Pin);
}


/* Here we need to check, if slave is connected ,is there any data to send from  BLE to mcu. so we can wake up the MCU here */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  HAL_GPIO_WritePin(BLE_WAKEUP_GPIO_Port,BLE_WAKEUP_Pin,GPIO_PIN_RESET);  // WAKE UP BLE
  HAL_UART_Receive_IT(&huart1, &RxTemp,1);   // START receive UART DATA
}


/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, BLE_RESET_Pin|BLE_WAKEUP_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, LED_B_Pin|LED_G_Pin|LED_R_Pin|LED_Y_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PAPin PAPin */
  GPIO_InitStruct.Pin = BLE_RESET_Pin|BLE_WAKEUP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = BLE_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BLE_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PDPin PDPin PDPin PDPin */
  GPIO_InitStruct.Pin = LED_B_Pin|LED_G_Pin|LED_R_Pin|LED_Y_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
