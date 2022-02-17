/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
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
#include "can.h"

/* USER CODE BEGIN 0 */
#include "string.h"
CAN_TxHeaderTypeDef canMsg;
CAN_TxHeaderTypeDef canReqBmsMsg;
uint8_t reqToBmsData[8]={0x46,0x16,0x01,0x16,0x10,0x00};;
uint32_t mailbox1;
uint8_t CANRxData[20][20];
uint8_t CANDataAvalFlag;
extern uint8_t BLEStopSendMsgDelayCount;
uint8_t CanProtocolNumb=0;
uint8_t CANSrcIndex=0;
extern CAN_RxHeaderTypeDef rxHeader;
CAN_TxHeaderTypeDef canTxHeader;
uint32_t txMailBoxUsed=0;
extern uint8_t Data[8];
extern uint16_t CANMsgIDStatus;


uint8_t batSOC=0;
uint8_t batSOH=0;
uint32_t batVolt=0;
int32_t batCurrent=0;
int8_t  batTemp=0;
uint8_t chargeStatus;


void initCANSndMsg()
{
  canMsg.DLC =1;
  canMsg.StdId = 0x123;
  canMsg.IDE = CAN_ID_STD;
  canMsg.RTR = CAN_RTR_DATA;
  
  canReqBmsMsg.StdId = 0x528;
  canReqBmsMsg.IDE = CAN_ID_STD;
  canReqBmsMsg.RTR= CAN_RTR_DATA;
  canReqBmsMsg.DLC=6;
    
  memset(CANRxData,0xff,sizeof(CANRxData)/sizeof(uint8_t));
}
void udpateCanInfo(CAN_RxHeaderTypeDef *rxMsg,uint8_t* canRxData)
{

   uint32_t canRxID = rxMsg->StdId;
   uint8_t canRxLen = rxMsg->DLC;
   uint32_t tempCurrent=0;
   uint32_t tempRPM=0;
   
   //0x47 16 01(DIR) DID DATA_Len Data[5] 
   if(canRxID == 0x544)
   {
     if((canRxData[0]== 0x47)&&(canRxData[1] == 0x16))
     {
       if(canRxData[3] == 0x08)  //little endian
       {
         batTemp = canRxData[5];        
       }
       else if(canRxData[3] == 0x09)  //little endian
       {
         batVolt = (uint32_t)canRxData[5]+((uint32_t)canRxData[6]<<8) + ((uint32_t)canRxData[7]<<16);
         batVolt = batVolt/100;  //unit 0.1a  -->0x184 byte4-5 big endian
       }
       else if(canRxData[3] == 0x0a)
       {
          tempCurrent = (uint32_t)canRxData[5]+((uint32_t)canRxData[6]<<8) + ((uint32_t)canRxData[7]<<16);
           if(tempCurrent >0x30d40) //if current >200A,it should be negative
               batCurrent = 0xffffff- tempCurrent;
           else
              batCurrent  = tempCurrent;
           
           batCurrent = batCurrent/100;   //0.1a unit--> 0x184 byte [6-7]
       }
       else if(canRxData[3] == 0x0d)  //[byte1]
       {
         batSOC = canRxData[5]+(canRxData[6]<<8) + (canRxData[7]<<16);
       }
       else if(canRxData[3] == 0x0e) //[byte0]
       {
         batSOH = canRxData[5]+(canRxData[6]<<8) + (canRxData[7]<<16);
       }
       else if(canRxData[3] == 0x16) //[byte0]
       {
         if(canRxData[5]&0x80)   //charging
         {
           chargeStatus = 1;
         }
         if(canRxData[5]&0x40)
         {
           chargeStatus = 0;
         }     
       }
     } 
     
     
      CANRxData[0][1]= (uint8_t)AQ_MSG0_0 & 0xff;
      CANRxData[0][2]= batSOH;
      CANRxData[0][3] = batSOC;
      CANRxData[0][4] = chargeStatus;
      CANRxData[0][6] =  ((batVolt&0xff00)>>8);
      CANRxData[0][7] =  (batVolt&0x00ff);
      CANRxData[0][8] =  ((batCurrent&0xff00)>>8);
      CANRxData[0][9] =  (batCurrent&0x00ff);
      
                  
      setBit(CANMsgIDStatus,0);
      if( CANMsgIDStatus != 0)
      {
        CANDataAvalFlag=5;
        BLEStopSendMsgDelayCount=5;   /*use the delay to optimize user feeling */
      }
   }  
}


void reqBMSData(uint8_t dataID, uint8_t dataLen)
{
   HAL_CAN_AddTxMessage(&hcan2,&canReqBmsMsg,reqToBmsData,&mailbox1);
}

void canRecvMsgUpdate(void)
{
  uint8_t temp=0;
  if(rxHeader.IDE == CAN_ID_STD)
    { 
        if(((rxHeader.StdId >= AQ_MSG0_0 )&&(rxHeader.StdId <= AQ_MSG7_1)) ||
           ((rxHeader.StdId >= AQ_MSG1_0)&&(rxHeader.StdId<= AQ_MSG6_1)))
        {
           CanProtocolNumb =0;  /* this is AQ Protocol  */
           CANRxData[0][0]=0;
           CANRxData[1][0]=0;
           CANRxData[2][0]=0;
           CANRxData[3][0]=0;
           CANRxData[4][0]=0;
           CANRxData[5][0]=0;
           CANRxData[6][0]=0;
           CANRxData[7][0]=0;
             
           switch(rxHeader.StdId)
          {
                case AQ_MSG0_0:
                {
                  CANRxData[0][1]= (uint8_t)rxHeader.StdId & 0xff;
                  Data[0]=batSOH;
                  Data[1]=batSOC;
                  Data[2]=chargeStatus;
                  Data[3]=batTemp;
                  Data[4]=((batVolt&0xff00)>>8);
                  Data[5]=(batVolt&0x00ff);
                  Data[6]=((batCurrent&0xff00)>>8);
                  Data[7]=(batCurrent&0x00ff);
                  
                  memcpy((void*)&CANRxData[0][2],Data,8);    
                  setBit(CANMsgIDStatus,0);
                  break;
                }
                case AQ_MSG0_1:
                {
                  CANRxData[0][10]= rxHeader.StdId;
                  memcpy(&CANRxData[0][11],Data,8);
                  setBit(CANMsgIDStatus,1);
                  break;
                }
                case AQ_MSG1_0:
                {
                  CANRxData[1][1]= rxHeader.StdId;
                  memcpy(&CANRxData[1][2],Data,8);  
                  setBit(CANMsgIDStatus,2);
                  break;
                }
                case AQ_MSG1_1:
                {
                  CANRxData[1][10]= rxHeader.StdId;
                  memcpy(&CANRxData[1][11],Data,8) ;  
                  setBit(CANMsgIDStatus,3);
                  break;
                }
                case AQ_MSG2_0:
                {
                  CANRxData[2][1]= rxHeader.StdId;
                  memcpy(&CANRxData[2][2],Data,8); 
                  setBit(CANMsgIDStatus,4);
                  break;
                }
                case AQ_MSG2_1:
                {
                  CANRxData[2][10]= rxHeader.StdId;
                  memcpy(&CANRxData[2][11],Data,8);   
                  setBit(CANMsgIDStatus,5);
                  break;
                }
                case AQ_MSG3_0:
                {
                  CANRxData[3][1]= rxHeader.StdId;
                  memcpy(&CANRxData[3][2],Data,8);
                  setBit(CANMsgIDStatus,6);
                  break;
                }
                case AQ_MSG3_1:
                {
                  CANRxData[3][10]= rxHeader.StdId;
                  memcpy(&CANRxData[3][11],Data,8);
                  setBit(CANMsgIDStatus,7);
                  break;
                }
                 case AQ_MSG4_0:
                {
                  CANRxData[4][1]= rxHeader.StdId;
                  memcpy(&CANRxData[4][2],Data,8);
                  setBit(CANMsgIDStatus,8);
                  break;
                }
                case AQ_MSG4_1:
                {
                  CANRxData[4][10]= rxHeader.StdId;
                  memcpy(&CANRxData[4][11],Data,8); 
                  setBit(CANMsgIDStatus,9);
                  break;
                }
                case AQ_MSG5_0:
                {
                  CANRxData[5][1]= rxHeader.StdId;
                  memcpy(&CANRxData[5][2],Data,8);
                  setBit(CANMsgIDStatus,10);
                  break;
                }
                case AQ_MSG5_1:
                {
                  CANRxData[5][10]= rxHeader.StdId;
                  memcpy(&CANRxData[5][11],Data,8) ;
                  setBit(CANMsgIDStatus,11);
                  break;
                }
                case AQ_MSG6_0:  //0x184 fault info of inverter
                {
                  CANRxData[6][1]= rxHeader.StdId;
                  
                  temp= ((Data[2]&0x01)<<7)+((Data[2]&0x02)<<5)+((Data[2]&0x04)<<3)+((Data[2]&0x08)<<1)+((Data[2]&0x10)>>1)+((Data[2]&0x20)>>3)+((Data[2]&0x40)>>5)+((Data[2]&0x80)>>7);
                  Data[2]= temp;
                  temp=0;
                  temp = ((Data[3]&0x01)<<7)+((Data[3]&0x02)<<5)+((Data[3]&0x04)<<3);
                  Data[3]=temp;
                  memcpy(&CANRxData[6][2],Data,8);
                  setBit(CANMsgIDStatus,12);
                  break;
                }
                case AQ_MSG6_1:
                {
                  CANRxData[6][10]= rxHeader.StdId;
                  memcpy(&CANRxData[6][11],Data,8) ;
                  setBit(CANMsgIDStatus,13);
                  break;
                }
                case AQ_MSG7_0:
                {
                  CANRxData[7][1]= rxHeader.StdId;
                  memcpy(&CANRxData[7][2],Data,8);
                  setBit(CANMsgIDStatus,14);
                  break;
                }
                case AQ_MSG7_1:
                {
                  CANRxData[7][10]= rxHeader.StdId;
                  memcpy(&CANRxData[7][11],Data,8) ;
                  setBit(CANMsgIDStatus,15);
                  break;
                }
                            
                default:
                {
                  break;
                }
          }  /*end of  switch */
      
      } // end of if 
   } // end of std_id
   else
   {
      uint32_t CanExtId =  rxHeader.ExtId;
      uint32_t *canExtIdPtr;
      
      if ((CanExtId == WOW_MSG0 )||(CanExtId == WOW_MSG1 )||
          (CanExtId == WOW_MSG2 )||(CanExtId == WOW_MSG3 )||
          (CanExtId == WOW_MSG4 )||(CanExtId == WOW_MSG5 )||
          (CanExtId == WOW_MSG6 )||(CanExtId == WOW_MSG7 )||
          (CanExtId == WOW_MSG8 )||(CanExtId == WOW_MSG9 )||
          (CanExtId == WOW_MSG10 )||(CanExtId == WOW_MSG11 )||
          (CanExtId == WOW_MSG12 )||(CanExtId == WOW_MSG13 )||
          (CanExtId == WOW_MSG14 )     
            )
      {        
        CanProtocolNumb = 1;
         switch(CanExtId)
          {
                case WOW_MSG0:
                {
                  CANRxData[0][0] = CanProtocolNumb;
                  CANRxData[0][1] = (uint8_t)(WOW_MSG0>>24);
                  CANRxData[0][2] = (uint8_t)(WOW_MSG0>>16);
                  CANRxData[0][3] = (uint8_t)(WOW_MSG0>>8);
                  CANRxData[0][4] = (uint8_t)(WOW_MSG0>>0);
                             
                  memcpy((void*)&CANRxData[0][5],Data,8);    
                  setBit(CANMsgIDStatus,0);
                  break;
                }
                case WOW_MSG1:
                {
                  CANRxData[1][0] = CanProtocolNumb;
                  CANRxData[1][1] = (uint8_t)(WOW_MSG1>>24);
                  CANRxData[1][2] = (uint8_t)(WOW_MSG1>>16);
                  CANRxData[1][3] = (uint8_t)(WOW_MSG1>>8);
                  CANRxData[1][4] = (uint8_t)(WOW_MSG1>>0);             
                  memcpy((void*)&CANRxData[1][5],Data,8);    
                  setBit(CANMsgIDStatus,1);
                  break;
                }
                case WOW_MSG2:
                {
                  CANRxData[2][0] = CanProtocolNumb;
                  CANRxData[2][1] = (uint8_t)(WOW_MSG2>>24);
                  CANRxData[2][2] = (uint8_t)(WOW_MSG2>>16);
                  CANRxData[2][3] = (uint8_t)(WOW_MSG2>>8);
                  CANRxData[2][4] = (uint8_t)(WOW_MSG2>>0);             
                  memcpy((void*)&CANRxData[2][5],Data,8);    
                  setBit(CANMsgIDStatus,2);
                  break;
                }
                case WOW_MSG3:
                {
                  CANRxData[3][0] = CanProtocolNumb;
                  CANRxData[3][1] = (uint8_t)(WOW_MSG3>>24);
                  CANRxData[3][2] = (uint8_t)(WOW_MSG3>>16);
                  CANRxData[3][3] = (uint8_t)(WOW_MSG3>>8);
                  CANRxData[3][4] = (uint8_t)(WOW_MSG3>>0);             
                  memcpy((void*)&CANRxData[3][5],Data,8);    
                  setBit(CANMsgIDStatus,3);
                  break;
                }
                case WOW_MSG4:
                {
                  CANRxData[4][0] = CanProtocolNumb;
                  CANRxData[4][1] = (uint8_t)(WOW_MSG4>>24);
                  CANRxData[4][2] = (uint8_t)(WOW_MSG4>>16);
                  CANRxData[4][3] = (uint8_t)(WOW_MSG4>>8);
                  CANRxData[4][4] = (uint8_t)(WOW_MSG4>>0);             
                  memcpy((void*)&CANRxData[4][5],Data,8);    
                  setBit(CANMsgIDStatus,4);
                  break;
                }
                case WOW_MSG5:
                {
                  CANRxData[5][0] = CanProtocolNumb;
                  CANRxData[5][1] = (uint8_t)(WOW_MSG5>>24);
                  CANRxData[5][2] = (uint8_t)(WOW_MSG5>>16);
                  CANRxData[5][3] = (uint8_t)(WOW_MSG5>>8);
                  CANRxData[5][4] = (uint8_t)(WOW_MSG5>>0);             
                  memcpy((void*)&CANRxData[5][5],Data,8);    
                  setBit(CANMsgIDStatus,5);
                  break;
                }
                case WOW_MSG6:
                {
                  CANRxData[6][0] = CanProtocolNumb;
                  CANRxData[6][1] = (uint8_t)(WOW_MSG6>>24);
                  CANRxData[6][2] = (uint8_t)(WOW_MSG6>>16);
                  CANRxData[6][3] = (uint8_t)(WOW_MSG6>>8);
                  CANRxData[6][4] = (uint8_t)(WOW_MSG6>>0);             
                  memcpy((void*)&CANRxData[6][5],Data,8);    
                  setBit(CANMsgIDStatus,6);
                  break;
                }
                case WOW_MSG7:
                {
                  CANRxData[7][0] = CanProtocolNumb;
                  CANRxData[7][1] = (uint8_t)(WOW_MSG7>>24);
                  CANRxData[7][2] = (uint8_t)(WOW_MSG7>>16);
                  CANRxData[7][3] = (uint8_t)(WOW_MSG7>>8);
                  CANRxData[7][4] = (uint8_t)(WOW_MSG7>>0);             
                  memcpy((void*)&CANRxData[7][5],Data,8);    
                  setBit(CANMsgIDStatus,7);
                  break;
                }
                case WOW_MSG8:
                {
                  CANRxData[8][0] = CanProtocolNumb;
                  CANRxData[8][1] = (uint8_t)(WOW_MSG8>>24);
                  CANRxData[8][2] = (uint8_t)(WOW_MSG8>>16);
                  CANRxData[8][3] = (uint8_t)(WOW_MSG8>>8);
                  CANRxData[8][4] = (uint8_t)(WOW_MSG8>>0);           
                  memcpy((void*)&CANRxData[8][5],Data,8);    
                  setBit(CANMsgIDStatus,8);
                  break;
                }
                case WOW_MSG9:
                {
                  CANRxData[9][0] = CanProtocolNumb;
                  CANRxData[9][1] = (uint8_t)(WOW_MSG9>>24);
                  CANRxData[9][2] = (uint8_t)(WOW_MSG9>>16);
                  CANRxData[9][3] = (uint8_t)(WOW_MSG9>>8);
                  CANRxData[9][4] = (uint8_t)(WOW_MSG9>>0);            
                  memcpy((void*)&CANRxData[9][5],Data,8);    
                  setBit(CANMsgIDStatus,9);
                  break;
                }
                case WOW_MSG10:
                {
                  CANRxData[10][0] = CanProtocolNumb;
                  CANRxData[10][1] = (uint8_t)(WOW_MSG10>>24);
                  CANRxData[10][2] = (uint8_t)(WOW_MSG10>>16);
                  CANRxData[10][3] = (uint8_t)(WOW_MSG10>>8);
                  CANRxData[10][4] = (uint8_t)(WOW_MSG10>>0);             
                  memcpy((void*)&CANRxData[10][5],Data,8);    
                  setBit(CANMsgIDStatus,10);
                  break;
                }
                case WOW_MSG11:
                {
                  CANRxData[11][0] = CanProtocolNumb;
                  CANRxData[11][1] = (uint8_t)(WOW_MSG11>>24);
                  CANRxData[11][2] = (uint8_t)(WOW_MSG11>>16);
                  CANRxData[11][3] = (uint8_t)(WOW_MSG11>>8);
                  CANRxData[11][4] = (uint8_t)(WOW_MSG11>>0);             
                  memcpy((void*)&CANRxData[11][5],Data,8);    
                  setBit(CANMsgIDStatus,11);
                  break;
                }
                case WOW_MSG12:
                {
                  CANRxData[12][0] = CanProtocolNumb;
                  CANRxData[12][1] = (uint8_t)(WOW_MSG12>>24);
                  CANRxData[12][2] = (uint8_t)(WOW_MSG12>>16);
                  CANRxData[12][3] = (uint8_t)(WOW_MSG12>>8);
                  CANRxData[12][4] = (uint8_t)(WOW_MSG12>>0);             
                  memcpy((void*)&CANRxData[12][5],Data,8);    
                  setBit(CANMsgIDStatus,12);
                  break;
                }
                case WOW_MSG13:
                {
                  CANRxData[13][0] = CanProtocolNumb;
                  CANRxData[13][1] = (uint8_t)(WOW_MSG13>>24);
                  CANRxData[13][2] = (uint8_t)(WOW_MSG13>>16);
                  CANRxData[13][3] = (uint8_t)(WOW_MSG13>>8);
                  CANRxData[13][4] = (uint8_t)(WOW_MSG13>>0);             
                  memcpy((void*)&CANRxData[13][5],Data,8);    
                  setBit(CANMsgIDStatus,13);
                  break;
                }
                case WOW_MSG14:
                {
                  CANRxData[14][0] = CanProtocolNumb;
                  CANRxData[14][1] = (uint8_t)(WOW_MSG14>>24);
                  CANRxData[14][2] = (uint8_t)(WOW_MSG14>>16);
                  CANRxData[14][3] = (uint8_t)(WOW_MSG14>>8);
                  CANRxData[14][4] = (uint8_t)(WOW_MSG14>>0);             
                  memcpy((void*)&CANRxData[14][5],Data,8);    
                  setBit(CANMsgIDStatus,14);
                  break;
                }
              default:
                break;
          }      
      } // END  OF ID compare     
   }  // end of IF IDE= EXT_ID
 
  
  if( CANMsgIDStatus != 0)
  {
    CANDataAvalFlag=5;
    BLEStopSendMsgDelayCount=5;   /*use the delay to optimize user feeling */
  }
   else
   {
     //CANDataAvalFlag=0;
   }; 
}
/* USER CODE END 0 */

CAN_HandleTypeDef hcan1;
CAN_HandleTypeDef hcan2;

/* CAN1 init function */
void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 8;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_2TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_12TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_3TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

}
/* CAN2 init function */
void MX_CAN2_Init(void)
{

  /* USER CODE BEGIN CAN2_Init 0 */

  /* USER CODE END CAN2_Init 0 */

  /* USER CODE BEGIN CAN2_Init 1 */

  /* USER CODE END CAN2_Init 1 */
  hcan2.Instance = CAN2;
  hcan2.Init.Prescaler = 8;
  hcan2.Init.Mode = CAN_MODE_NORMAL;
  hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan2.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan2.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan2.Init.TimeTriggeredMode = DISABLE;
  hcan2.Init.AutoBusOff = DISABLE;
  hcan2.Init.AutoWakeUp = DISABLE;
  hcan2.Init.AutoRetransmission = DISABLE;
  hcan2.Init.ReceiveFifoLocked = DISABLE;
  hcan2.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN2_Init 2 */

  /* USER CODE END CAN2_Init 2 */

}

static uint32_t HAL_RCC_CAN1_CLK_ENABLED=0;

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    HAL_RCC_CAN1_CLK_ENABLED++;
    if(HAL_RCC_CAN1_CLK_ENABLED==1){
      __HAL_RCC_CAN1_CLK_ENABLE();
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN1 GPIO Configuration
    PB8     ------> CAN1_RX
    PB9     ------> CAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
  else if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspInit 0 */

  /* USER CODE END CAN2_MspInit 0 */
    /* CAN2 clock enable */
    __HAL_RCC_CAN2_CLK_ENABLE();
    HAL_RCC_CAN1_CLK_ENABLED++;
    if(HAL_RCC_CAN1_CLK_ENABLED==1){
      __HAL_RCC_CAN1_CLK_ENABLE();
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN2 GPIO Configuration
    PB5     ------> CAN2_RX
    PB6     ------> CAN2_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* CAN2 interrupt Init */
    HAL_NVIC_SetPriority(CAN2_RX0_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(CAN2_RX0_IRQn);
  /* USER CODE BEGIN CAN2_MspInit 1 */

  /* USER CODE END CAN2_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    HAL_RCC_CAN1_CLK_ENABLED--;
    if(HAL_RCC_CAN1_CLK_ENABLED==0){
      __HAL_RCC_CAN1_CLK_DISABLE();
    }

    /**CAN1 GPIO Configuration
    PB8     ------> CAN1_RX
    PB9     ------> CAN1_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
  else if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspDeInit 0 */

  /* USER CODE END CAN2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN2_CLK_DISABLE();
    HAL_RCC_CAN1_CLK_ENABLED--;
    if(HAL_RCC_CAN1_CLK_ENABLED==0){
      __HAL_RCC_CAN1_CLK_DISABLE();
    }

    /**CAN2 GPIO Configuration
    PB5     ------> CAN2_RX
    PB6     ------> CAN2_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_5|GPIO_PIN_6);

    /* CAN2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN2_RX0_IRQn);
  /* USER CODE BEGIN CAN2_MspDeInit 1 */

  /* USER CODE END CAN2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
