/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "isotp_user.h"
#include "ble_app.h"
#include "gpio.h"
#include "can.h"
#include "appOBD.h"
extern uint8_t CANDataAvalFlag;
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId BleTaskHandle;
osThreadId CANReqTaskHandle;
osThreadId ledTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void BleMainTask(void const * argument);
void CANReqMainTask(void const * argument);
void LedControlMainTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of BleTask */
  osThreadDef(BleTask, BleMainTask, osPriorityNormal, 0, 128);
  BleTaskHandle = osThreadCreate(osThread(BleTask), NULL);

  /* definition and creation of CANReqTask */
  osThreadDef(CANReqTask, CANReqMainTask, osPriorityIdle, 0, 128);
  CANReqTaskHandle = osThreadCreate(osThread(CANReqTask), NULL);

  /* definition and creation of ledTask */
  osThreadDef(ledTask, LedControlMainTask, osPriorityIdle, 0, 128);
  ledTaskHandle = osThreadCreate(osThread(ledTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  TpInit();
  for(;;)
  {
    TpTask();
    DescTask();
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_BleMainTask */
/**
* @brief Function implementing the BleTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_BleMainTask */
void BleMainTask(void const * argument)
{
  /* USER CODE BEGIN BleMainTask */
    enableBLE();
  /* Infinite loop */
  for(;;)
  {
    checkBLEConnectStatus();
    if(CANDataAvalFlag>0) CANDataAvalFlag--;
    sendAllBLEMsg();
    osDelay(1000);
  }
  /* USER CODE END BleMainTask */
}

/* USER CODE BEGIN Header_CANReqMainTask */
/**
* @brief Function implementing the CANReqTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_CANReqMainTask */
void CANReqMainTask(void const * argument)
{
  /* USER CODE BEGIN CANReqMainTask */
  initCANSndMsg();
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END CANReqMainTask */
}

/* USER CODE BEGIN Header_LedControlMainTask */
/**
* @brief Function implementing the ledTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_LedControlMainTask */
void LedControlMainTask(void const * argument)
{
  /* USER CODE BEGIN LedControlMainTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1000);
    monitorDTC();
    toggleLeds();
  }
  /* USER CODE END LedControlMainTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
