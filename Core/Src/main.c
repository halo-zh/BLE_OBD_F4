/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "isotp.h"
#include "bsp_usartx_CC2541.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern void obdDiagReqHandler(uint8_t data[],uint8_t * dlc);
extern void onRecvCommMessage(uint32_t,uint8_t data[8],uint8_t);
extern uint8_t DTCnumber;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
char BLE_AT[]="AT\r\n";
char BLE_GET_VERSION[]="AT+VERION=?\r\n";

char BLE_GET_STATE[]="AT+SYS_STATE=?\r\n";

char BLE_SET_NAME[]="AT+NAME=";
char BLE_SET_PWD[]="AT+PSWD=";
char BLE_TX_POWER[]="AT+TX=";
char BLE_RX_POWER[]="AT+RX=";

char BLE_SET_ID[]="AT+ADV_MFR_SPC=424C455F4F42445F4151\r\n";// BLE_AQ
extern uint8_t   uart_buff[];
uint8_t CANRxData[20][20];  // max 20 byte per frame 
uint8_t ConnStopFlag=0;
extern uint8_t isOBDConnect;
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t data[1];
CAN_TxHeaderTypeDef canMsg;
uint32_t mailbox1;
uint32_t delay=0;
uint32_t preTick=0;
extern void countDTCNow(void);
extern void saveDtcToFlash();
uint8_t preDtcCnt=0;

uint8_t isConnect=0;
uint8_t DataInFlag=0;
uint8_t CANDataAvalFlag=0;
uint8_t BLEStopSendMsgDelayCount=0;
uint8_t SlaveConnected =0;
uint8_t RxTemp=0;
/* Here we need to check, if slave is connected ,is there any data to send from  BLE to mcu. so we can wake up the MCU here */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  HAL_GPIO_WritePin(BLE_WAKEUP_GPIO_Port,BLE_WAKEUP_Pin,GPIO_PIN_RESET);  // WAKE UP BLE
  HAL_UART_Receive_IT(&huart1, &RxTemp,1);   // START receive UART DATA
}


void checkBLEConnectStatus(void)
{
    CC2541_Send_CMD(BLE_GET_STATE,0);
    if(strstr(uart_buff,"5"))  
    {
      isConnect = 1;    
    }
    else
    {
      isConnect =0;
    }
}


void toggleLeds()
{
  if(isConnect ==1)
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
FLASH_OBProgramInitTypeDef obInit;
HAL_StatusTypeDef status;
void programOption()
{

   HAL_FLASHEx_OBGetConfig(&obInit);
  
   obInit.OptionType = OPTIONBYTE_RDP;
   obInit.RDPLevel = OB_RDP_LEVEL_0;
   while(1)
   {
   status = HAL_FLASHEx_OBProgram(&obInit);
   }
  
}


void sendAllBLEMsg()
{
  if((CANDataAvalFlag!=0)&&(BLEStopSendMsgDelayCount!=0)&&(isConnect ==1) )
    {
      
      sendBLEMsg();
      if(BLEStopSendMsgDelayCount>0) BLEStopSendMsgDelayCount--;
      
      
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

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
 
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  
  

  TpInit();
  canMsg.DLC =1;
  canMsg.StdId = 0x123;
  canMsg.IDE = CAN_ID_STD;
  canMsg.RTR = CAN_RTR_DATA;
  
  memset(CANRxData,0xff,sizeof(CANRxData)/sizeof(uint8_t));
  HAL_GPIO_WritePin(BLE_WAKEUP_GPIO_Port,BLE_WAKEUP_Pin,GPIO_PIN_RESET); // set low to wake up BLE,
  HAL_GPIO_WritePin(BLE_RESET_GPIO_Port,BLE_RESET_Pin,GPIO_PIN_SET);  // RESET PIN HIGH, STOP RESET BLE
  
  copyDtcFromFlash();
  countDTC();
  preDtcCnt = DTCnumber;
 
  
  HAL_UART_Receive_IT(&huart1, &RxTemp,1);   // START receive UART DATA
  HAL_Delay(10);
  CC2541_Send_CMD(BLE_SET_ID,0);  
  
 // MX_CAN1_Init();
  MX_CAN2_Init();
  CANMultiInit();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
   checkBLEConnectStatus();
   TpTask();
   DescTask();
   
   if((HAL_GetTick()- preTick) >=800)
   {
     preTick = HAL_GetTick();
     countDTC();
     if(preDtcCnt != DTCnumber)
     {      
        saveDtcToFlash();
        preDtcCnt= DTCnumber;
     }
     
     data[0]= 0x55;
     countDTCNow();
     if(DTCnumber !=0)
     {    
       data[0]= 0xAA;
     }
     
     countDTC();
     if(DTCnumber !=0)
     {    
       data[0]= 0xAA;
     }
     
     HAL_CAN_AddTxMessage(&hcan2,&canMsg,data,&mailbox1);
     HAL_CAN_AddTxMessage(&hcan1,&canMsg,data,&mailbox1);
     toggleLeds();
     sendAllBLEMsg();
     if(CANDataAvalFlag>0) CANDataAvalFlag--;
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 256;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
