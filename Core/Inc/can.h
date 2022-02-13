/**
  ******************************************************************************
  * @file    can.h
  * @brief   This file contains all the function prototypes for
  *          the can.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_CAN1_Init(void);
void MX_CAN2_Init(void);

/* USER CODE BEGIN Prototypes */
#define AQ_MSG0_0 0x181
#define AQ_MSG0_1 0x183

#define AQ_MSG1_0 0x160
#define AQ_MSG1_1 0x161

#define AQ_MSG2_0 0x162
#define AQ_MSG2_1 0x163

#define AQ_MSG3_0 0x164
#define AQ_MSG3_1 0x165

#define AQ_MSG4_0 0x166
#define AQ_MSG4_1 0x167

#define AQ_MSG5_0 0x168
#define AQ_MSG5_1 0x169

#define AQ_MSG6_0 0x184
#define AQ_MSG6_1 0x185

#define AQ_MSG7_0 0x186
#define AQ_MSG7_1 0x187

#define WOW_MSG0 0x10261022
#define WOW_MSG1 0x1811D0F3
#define WOW_MSG2 0X1812D0F3
#define WOW_MSG3 0X1814D0F3
#define WOW_MSG4 0X1815D0F3
#define WOW_MSG5 0X1816D0F3
#define WOW_MSG6 0X1817D0F3
#define WOW_MSG7 0X1818D0F3
#define WOW_MSG8 0X1819D0F3
#define WOW_MSG9 0X181AD0F3
#define WOW_MSG10 0X1813D0F3

#define WOW_MSG11 0X181BD0F3
#define WOW_MSG12 0X181CD0F3

#define WOW_MSG13 0X181DD0F3
#define WOW_MSG14 0X181ED0F3


#define WOW_MSG_COUNT 15
#define AQ_MSG_COUNT 8


#define setBit(a,b)  (a|=(1<<b))
#define clrBit(a,b)   (a&=(0<<b))

extern uint8_t CanProtocolNumb;

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
