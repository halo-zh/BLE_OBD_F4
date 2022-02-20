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

uint8_t gwCanBuffer[3][8];
uint8_t preCANID=0;
uint8_t canBufCnt=0;
uint8_t curCANID=0;
uint8_t grwBatDat[100];


uint8_t gwBatSOC=0;
uint8_t gwBatSOH=0;
uint32_t gwBatVolt=0;
int32_t gwBatCurrent=0;
int8_t  gwBatTemp=0;
uint8_t gwChargeStatus;
uint16_t gwMaxcellVolt;
uint16_t gwMincellVolt;
int8_t  gwMaxcellTemp;
int8_t  gwMincellTemp;

uint16_t gwVoltCell_1;
uint16_t gwVoltCell_2;
uint16_t gwVoltCell_3;
uint16_t gwVoltCell_4;
uint16_t gwVoltCell_5;
uint16_t gwVoltCell_6;
uint16_t gwVoltCell_7;
uint16_t gwVoltCell_8;
uint16_t gwVoltCell_9;
uint16_t gwVoltCell_10;
uint16_t gwVoltCell_11;
uint16_t gwVoltCell_12;
uint16_t gwVoltCell_13;
uint16_t gwVoltCell_14;
uint16_t gwVoltCell_15;
uint16_t gwVoltCell_16;
uint16_t gwVoltCell_17;
uint16_t gwVoltCell_18;
uint16_t gwVoltCell_19;
uint16_t gwVoltCell_20;

uint16_t gwVoltCellMax;
uint16_t gwVoltCellMin;

uint8_t gwDischargeTempLow;
uint8_t gwDischargeTempHigh;
uint8_t gwChargeTempLow;
uint8_t gwChargeTempHigh;
uint8_t gwChargeOverCurrent;
uint8_t gwDischargeOverCurrent;

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
    
  memset(CANRxData,0x00,sizeof(CANRxData)/sizeof(uint8_t));
}
void updateBLEMsg(void);
void updateGwInfo(void)
{
	   uint32_t tempCurrent=0;
	   uint8_t index=5;
	   if((grwBatDat[0]!= 0x47)||(grwBatDat[1] != 0x16))
		 {
			 return;
		 }
	
	
     if((grwBatDat[0]== 0x47)&&(grwBatDat[1] == 0x16))
     {
       if(grwBatDat[3] == 0x08)  //little endian
       {
         gwBatTemp = grwBatDat[5];        
       }
       else if(grwBatDat[3] == 0x09)  //little endian
       {
         gwBatVolt = (uint32_t)grwBatDat[5]+((uint32_t)grwBatDat[6]<<8) + ((uint32_t)grwBatDat[7]<<16);
         gwBatVolt = gwBatVolt/100;  //unit 0.1a  -->0x184 byte4-5 big endian
       }
       else if(grwBatDat[3] == 0x0a)
       {
          tempCurrent = (uint32_t)grwBatDat[5]+((uint32_t)grwBatDat[6]<<8) + ((uint32_t)grwBatDat[7]<<16)+ ((uint32_t)grwBatDat[8]<<24);
           if(tempCurrent >0x7fffffff) 
               gwBatCurrent = 0xffffff- tempCurrent;
           else
              gwBatCurrent  = tempCurrent;
           
           gwBatCurrent = gwBatCurrent/100;   //0.1a unit--> 0x184 byte [6-7]
       }
       else if(grwBatDat[3] == 0x0d)  //[byte1]
       {
         gwBatSOC = grwBatDat[5]+(grwBatDat[6]<<8) + (grwBatDat[7]<<16);
       }
       else if(grwBatDat[3] == 0x0e) //[byte0]
       {
         gwBatSOH = grwBatDat[5]+(grwBatDat[6]<<8) + (grwBatDat[7]<<16);
       }
       else if(grwBatDat[3] == 0x16) //[byte0]
       {
         if(grwBatDat[5]&0x80)   //charging
         {
           gwChargeStatus = 1;
         }
         if(grwBatDat[5]&0x40)
         {
           gwChargeStatus = 0;
         }
				 
				 if(grwBatDat[5+3]&0X20)
				 {
					 gwChargeOverCurrent =1;
				 }
				 else
				 {
					 gwChargeOverCurrent =0;
				 }
				 
				 if(grwBatDat[5+3]&0X04)
				 {
					 gwDischargeOverCurrent =1;
				 }
				 else
				 {
					 gwDischargeOverCurrent =0;
				 }
				 
         if(grwBatDat[5+4]&0X04)
				 {
						gwDischargeTempHigh =2;
				 }
				 else if(grwBatDat[5+4]&0X08)
				 {
						gwChargeTempHigh =2;
				 }
					else if(grwBatDat[5+4]&0X10)
				 {
						gwDischargeTempLow =2;
				 }
				else if(grwBatDat[5+4]&0X20)
				 {
						gwChargeTempLow =2;
				 }				 
       }
			 else if(grwBatDat[3]== 0x26)
			 {
				 gwMaxcellVolt=grwBatDat[5+8]+(grwBatDat[5+9]<<8);
				 gwMincellVolt=grwBatDat[5+10]+(grwBatDat[5+11]<<8);
				 gwMaxcellTemp=grwBatDat[5+12];
				 gwMincellTemp=grwBatDat[5+13];
			 }
			 else if(grwBatDat[3]== 0x24)
			 {
			    gwVoltCell_1 = grwBatDat[index]+(grwBatDat[index+1]<<8);
				  index+=2;
				  gwVoltCell_2 = grwBatDat[index]+(grwBatDat[index+1]<<8);
				 index+=2;
				  gwVoltCell_3 = grwBatDat[index]+(grwBatDat[index+1]<<8);
				 index+=2;
				  gwVoltCell_4 = grwBatDat[index]+(grwBatDat[index+1]<<8);
				 index+=2;
				  gwVoltCell_5 = grwBatDat[index]+(grwBatDat[index+1]<<8);
				 index+=2;
				  gwVoltCell_6 = grwBatDat[index]+(grwBatDat[index+1]<<8);
				 index+=2;
				  gwVoltCell_7 = grwBatDat[index]+(grwBatDat[index+1]<<8);
				 index+=2;
				  gwVoltCell_8 = grwBatDat[index]+(grwBatDat[index+1]<<8);
				 index+=2;
				  gwVoltCell_9 = grwBatDat[index]+(grwBatDat[index+1]<<8);
				 index+=2;
				  gwVoltCell_10 = grwBatDat[index]+(grwBatDat[index+1]<<8);
				 index+=2;
				  gwVoltCell_11 = grwBatDat[index]+(grwBatDat[index+1]<<8);
					index+=2;
				  gwVoltCell_12 = grwBatDat[index]+(grwBatDat[index+1]<<8);
					index+=2;
				  gwVoltCell_13 = grwBatDat[index]+(grwBatDat[index+1]<<8);
					index+=2;
				  gwVoltCell_14 = grwBatDat[index]+(grwBatDat[index+1]<<8);
					index+=2;
				  gwVoltCell_15 = grwBatDat[index]+(grwBatDat[index+1]<<8);
					index+=2;
				  gwVoltCell_16 = grwBatDat[index]+(grwBatDat[index+1]<<8);
						
					gwVoltCellMax=gwVoltCell_1;
					gwVoltCellMin = gwVoltCell_1;
					if(gwVoltCell_2>gwVoltCellMax)
					{
						gwVoltCellMax = gwVoltCell_2;
					}
					
					if(gwVoltCell_3>gwVoltCellMax)
					{
						gwVoltCellMax = gwVoltCell_3;
					}
					
					if(gwVoltCell_4>gwVoltCellMax)
					{
						gwVoltCellMax = gwVoltCell_4;
					}
					
					if(gwVoltCell_5>gwVoltCellMax)
					{
						gwVoltCellMax = gwVoltCell_5;
					}
					if(gwVoltCell_6>gwVoltCellMax)
					{
						gwVoltCellMax = gwVoltCell_6;
					}
					if(gwVoltCell_7>gwVoltCellMax)
					{
						gwVoltCellMax = gwVoltCell_7;
					}
					if(gwVoltCell_8>gwVoltCellMax)
					{
						gwVoltCellMax = gwVoltCell_8;
					}
					if(gwVoltCell_9>gwVoltCellMax)
					{
						gwVoltCellMax = gwVoltCell_9;
					}
					if(gwVoltCell_10>gwVoltCellMax)
					{
						gwVoltCellMax = gwVoltCell_10;
					}
					if(gwVoltCell_11>gwVoltCellMax)
					{
						gwVoltCellMax = gwVoltCell_11;
					}
					if(gwVoltCell_12>gwVoltCellMax)
					{
						gwVoltCellMax = gwVoltCell_12;
					}
					if(gwVoltCell_13>gwVoltCellMax)
					{
						gwVoltCellMax = gwVoltCell_13;
					}
					if(gwVoltCell_14>gwVoltCellMax)
					{
						gwVoltCellMax = gwVoltCell_14;
					}
					if(gwVoltCell_15>gwVoltCellMax)
					{
						gwVoltCellMax = gwVoltCell_15;
					}
					if(gwVoltCell_16>gwVoltCellMax)
					{
						gwVoltCellMax = gwVoltCell_16;
					}
	
					
					if(gwVoltCell_2<gwVoltCellMax)
					{
						gwVoltCellMin = gwVoltCell_2;
					}
					
					if(gwVoltCell_3<gwVoltCellMax)
					{
						gwVoltCellMin = gwVoltCell_3;
					}
					
					if(gwVoltCell_4<gwVoltCellMax)
					{
						gwVoltCellMin = gwVoltCell_4;
					}
					
					if(gwVoltCell_5<gwVoltCellMax)
					{
						gwVoltCellMin = gwVoltCell_5;
					}
					if(gwVoltCell_6<gwVoltCellMax)
					{
						gwVoltCellMin = gwVoltCell_6;
					}
					if(gwVoltCell_7<gwVoltCellMax)
					{
						gwVoltCellMin = gwVoltCell_7;
					}
					if(gwVoltCell_8<gwVoltCellMax)
					{
						gwVoltCellMin = gwVoltCell_8;
					}
					if(gwVoltCell_9<gwVoltCellMax)
					{
						gwVoltCellMin = gwVoltCell_9;
					}
					if(gwVoltCell_10<gwVoltCellMax)
					{
						gwVoltCellMin = gwVoltCell_10;
					}
					if(gwVoltCell_11<gwVoltCellMax)
					{
						gwVoltCellMin = gwVoltCell_11;
					}
					if(gwVoltCell_12<gwVoltCellMax)
					{
						gwVoltCellMin = gwVoltCell_12;
					}
					if(gwVoltCell_13<gwVoltCellMax)
					{
						gwVoltCellMin = gwVoltCell_13;
					}
					if(gwVoltCell_14<gwVoltCellMax)
					{
						gwVoltCellMin = gwVoltCell_14;
					}
					if(gwVoltCell_15<gwVoltCellMax)
					{
						gwVoltCellMin = gwVoltCell_15;
					}
					if(gwVoltCell_16<gwVoltCellMax)
					{
						gwVoltCellMin = gwVoltCell_16;
					}
					



			 }
			 else if(grwBatDat[3]== 0x25)
			 {
				 index =5;
				 gwVoltCell_17 = grwBatDat[index]+(grwBatDat[index+1]<<8);
					index+=2;
				  gwVoltCell_18 = grwBatDat[index]+(grwBatDat[index+1]<<8);
				 index+=2;
				  gwVoltCell_19 = grwBatDat[index]+(grwBatDat[index+1]<<8);
				 index+=2;
				  gwVoltCell_20 = grwBatDat[index]+(grwBatDat[index+1]<<8);
			 }

       			 
			 
			 
			 
			 CANDataAvalFlag=5;
       BLEStopSendMsgDelayCount=5;   /*use the delay to optimize user feeling */
      
			 memset(grwBatDat,0x00,sizeof(grwBatDat));
			 
			updateBLEMsg();
	
     }

    		 
     
     
      
}

void updateBLEMsg(void)
{
	
	   CANRxData[0][0]=0;
		 CANRxData[1][0]=0;
		 CANRxData[2][0]=0;
		 CANRxData[3][0]=0;
		 CANRxData[4][0]=0;
		 CANRxData[5][0]=0;
		 CANRxData[6][0]=0;
		 CANRxData[7][0]=0;
	
			CANRxData[0][1]= (uint8_t)AQ_MSG0_181 & 0xff;
      CANRxData[0][2]= gwBatSOH;
      CANRxData[0][3] = gwBatSOC;
      CANRxData[0][4] = gwChargeStatus;
			CANRxData[0][5] = gwBatTemp;
      CANRxData[0][6] =  ((gwBatVolt&0xff00)>>8);
      CANRxData[0][7] =  (gwBatVolt&0x00ff);
      CANRxData[0][8] =  ((gwBatCurrent&0xff00)>>8);
      CANRxData[0][9] =  (gwBatCurrent&0x00ff);
	
			CANRxData[0][10]=   0x183 & 0xff;
	
			CANRxData[1][1] = 	(uint8_t)AQ_MSG160 & 0xff;
      CANRxData[1][2] = 	(gwVoltCell_1&0xff00)>>8;
      CANRxData[1][3] = 	gwVoltCell_1&0xff;
      CANRxData[1][4] =	  (gwVoltCell_2&0xff00)>>8;
	    CANRxData[1][5] = 	gwVoltCell_2&0xff;
      CANRxData[1][6] =  (gwVoltCell_3&0xff00)>>8;
      CANRxData[1][7] =   gwVoltCell_3&0xff;
      CANRxData[1][8] =  (gwVoltCell_4&0xff00)>>8;
      CANRxData[1][9] =   gwVoltCell_4&0xff;
	
			CANRxData[1][10]= (uint8_t)AQ_MSG161 & 0xff;
      CANRxData[1][11] = 	(gwVoltCell_5&0xff00)>>8;
      CANRxData[1][12] = 	gwVoltCell_5&0xff;
      CANRxData[1][13] =	  (gwVoltCell_6&0xff00)>>8;
	    CANRxData[1][14] = 	gwVoltCell_6&0xff;
      CANRxData[1][15] =  (gwVoltCell_7&0xff00)>>8;
      CANRxData[1][16] =   gwVoltCell_7&0xff;
      CANRxData[1][17] =  (gwVoltCell_8&0xff00)>>8;
      CANRxData[1][18] =   gwVoltCell_8&0xff;
			
			CANRxData[2][1] = 	(uint8_t)AQ_MSG162 & 0xff;
      CANRxData[2][2] = 	(gwVoltCell_9&0xff00)>>8;
      CANRxData[2][3] = 	gwVoltCell_9&0xff;
      CANRxData[2][4] =	  (gwVoltCell_10&0xff00)>>8;
	    CANRxData[2][5] = 	gwVoltCell_10&0xff;
      CANRxData[2][6] =  (gwVoltCell_11&0xff00)>>8;
      CANRxData[2][7] =   gwVoltCell_11&0xff;
      CANRxData[2][8] =  (gwVoltCell_12&0xff00)>>8;
      CANRxData[2][9] =   gwVoltCell_12&0xff;
	
			CANRxData[2][10]= (uint8_t)AQ_MSG163 & 0xff;
      CANRxData[2][11] = 	(gwVoltCell_13&0xff00)>>8;
      CANRxData[2][12] = 	gwVoltCell_13&0xff;
      CANRxData[2][13] =	  (gwVoltCell_14&0xff00)>>8;
	    CANRxData[2][14] = 	gwVoltCell_14&0xff;
      CANRxData[2][15] =  (gwVoltCell_15&0xff00)>>8;
      CANRxData[2][16] =   gwVoltCell_15&0xff;
      CANRxData[2][17] =  (gwVoltCell_16&0xff00)>>8;
      CANRxData[2][18] =   gwVoltCell_16&0xff;

			CANRxData[3][1] = 	(uint8_t)AQ_MSG164 & 0xff;
      CANRxData[3][2] = 	(gwVoltCell_17&0xff00)>>8;
      CANRxData[3][3] = 	gwVoltCell_17&0xff;
      CANRxData[3][4] =	  (gwVoltCell_18&0xff00)>>8;
	    CANRxData[3][5] = 	gwVoltCell_18&0xff;
      CANRxData[3][6] =  (gwVoltCell_19&0xff00)>>8;
      CANRxData[3][7] =   gwVoltCell_19&0xff;
      CANRxData[3][8] =  (gwVoltCell_20&0xff00)>>8;
      CANRxData[3][9] =   gwVoltCell_20&0xff;
			
			CANRxData[3][10] = 	(uint8_t)AQ_MSG165 & 0xff;
      CANRxData[3][11] = 	gwBatTemp>>8;
      CANRxData[3][12] = 	gwBatTemp;
      CANRxData[3][13] =	gwBatTemp>>8;
	    CANRxData[3][14] = 	gwBatTemp;
      CANRxData[3][15] =  gwBatTemp>>8;
      CANRxData[3][16] =  gwBatTemp;
      CANRxData[3][17] =  gwBatTemp>>8;
      CANRxData[3][18] =  gwBatTemp;
			
			
		  CANRxData[4][1] = 	(uint8_t)AQ_MSG166 & 0xff;
      CANRxData[4][2] = 	(gwVoltCellMax&0xff00)>>8;
      CANRxData[4][3] = 	gwVoltCellMax&0xff;
      CANRxData[4][4] =	  (gwVoltCellMin&0xff00)>>8;
	    CANRxData[4][5] = 	gwVoltCellMin&0xff;
      CANRxData[4][6] =   0;
      CANRxData[4][7] =   0;
      CANRxData[4][8] =  ((gwVoltCellMax-gwVoltCellMin) &0xff00)>>8;
      CANRxData[4][9] =   (gwVoltCellMax-gwVoltCellMin)&0xff;
			
			CANRxData[4][10] = 	(uint8_t)AQ_MSG167 & 0xff;
      CANRxData[4][11] = 	(gwBatTemp&0xff00)>>8;
      CANRxData[4][12] = 	gwBatTemp&0xff;
      CANRxData[4][13] =	(gwBatTemp&0xff00)>>8;
	    CANRxData[4][14] = 	gwBatTemp&0xff;
      CANRxData[4][15] =  0;
      CANRxData[4][16] =  0;
      CANRxData[4][17] =  ((gwBatTemp-gwBatTemp) &0xff00)>>8;
      CANRxData[4][18] =  (gwBatTemp-gwBatTemp)&0xff;
			
			
			CANRxData[5][1] = 	(uint8_t)AQ_MSG168 & 0xff;
      CANRxData[5][2] = 	gwDischargeTempLow + (gwDischargeTempHigh<<2) + (gwChargeTempLow<<4) +(gwDischargeTempHigh<<6);
      CANRxData[5][3] = 	0;
      CANRxData[5][4] =	  0;
	    CANRxData[5][5] = 	0;
      CANRxData[5][6] =   0;
      CANRxData[5][7] =   0;
      CANRxData[5][8] =   0;
      CANRxData[5][9] =   0;
			
			CANRxData[5][10]= 0x69;
			
			CANRxData[6][1]= 0x84;
			CANRxData[6][10]= 0x85;
			
			CANRxData[7][1]= 0x86;
			CANRxData[7][10]= 0x87;
	    
}



void updateGreenwayBatInfo(CAN_RxHeaderTypeDef *rxMsg,uint8_t* canRxData)
{

   uint32_t canRxID = rxMsg->StdId;
   uint8_t canRxLen = rxMsg->DLC;
   
   uint32_t tempRPM=0;
	 curCANID = rxMsg->StdId;
	
	 if(canRxID != 0x544)
	 {
		 return;
	 }
	
	if((canRxData[0] == 0x47)&&(canRxData[1] == 0x16))
	{	
		updateGwInfo();
		canBufCnt=0;	
		memcpy(&grwBatDat[canBufCnt*8],canRxData,canRxLen);
		canBufCnt++;		
	}
	else
	{
		if(canBufCnt*8<80)
		{
			memcpy(&grwBatDat[canBufCnt*8],canRxData,canRxLen);
			canBufCnt++;
		}
	}
}


void reqBMSData(uint8_t dataID, uint8_t dataLen)
{
	uint8_t i=0;
	reqToBmsData[3]=dataID;
	reqToBmsData[4]=dataLen;
	reqToBmsData[5]=0;
	
	for(i=0;i<5;i++)
	{
		reqToBmsData[5]+=reqToBmsData[i];
	}
	 HAL_CAN_AddTxMessage(&hcan1,&canReqBmsMsg,reqToBmsData,&mailbox1);
   HAL_CAN_AddTxMessage(&hcan2,&canReqBmsMsg,reqToBmsData,&mailbox1);
}

void updateAQWowInfo(void)
{
  uint8_t temp=0;
  if(rxHeader.IDE == CAN_ID_STD)
    { 
        if(((rxHeader.StdId >= AQ_MSG0_181 )&&(rxHeader.StdId <= AQ_MSG187)) ||
           ((rxHeader.StdId >= AQ_MSG160)&&(rxHeader.StdId<= AQ_MSG185)))
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
                case AQ_MSG0_181:
                {
                  CANRxData[0][1]= (uint8_t)rxHeader.StdId & 0xff;
                  Data[0]=gwBatSOH;
                  Data[1]=gwBatSOC;
                  Data[2]=gwChargeStatus;
                  Data[3]=gwBatTemp;
                  Data[4]=((gwBatVolt&0xff00)>>8);
                  Data[5]=(gwBatVolt&0x00ff);
                  Data[6]=((gwBatCurrent&0xff00)>>8);
                  Data[7]=(gwBatCurrent&0x00ff);
                  
                  memcpy((void*)&CANRxData[0][2],Data,8);    
                  setBit(CANMsgIDStatus,0);
                  break;
                }
                case AQ_MSG0_183:
                {
                  CANRxData[0][10]= rxHeader.StdId;
                  memcpy(&CANRxData[0][11],Data,8);
                  setBit(CANMsgIDStatus,1);
                  break;
                }
                case AQ_MSG160:
                {
                  CANRxData[1][1]= rxHeader.StdId;
                  memcpy(&CANRxData[1][2],Data,8);  
                  setBit(CANMsgIDStatus,2);
                  break;
                }
                case AQ_MSG161:
                {
                  CANRxData[1][10]= rxHeader.StdId;
                  memcpy(&CANRxData[1][11],Data,8) ;  
                  setBit(CANMsgIDStatus,3);
                  break;
                }
                case AQ_MSG162:
                {
                  CANRxData[2][1]= rxHeader.StdId;
                  memcpy(&CANRxData[2][2],Data,8); 
                  setBit(CANMsgIDStatus,4);
                  break;
                }
                case AQ_MSG163:
                {
                  CANRxData[2][10]= rxHeader.StdId;
                  memcpy(&CANRxData[2][11],Data,8);   
                  setBit(CANMsgIDStatus,5);
                  break;
                }
                case AQ_MSG164:
                {
                  CANRxData[3][1]= rxHeader.StdId;
                  memcpy(&CANRxData[3][2],Data,8);
                  setBit(CANMsgIDStatus,6);
                  break;
                }
                case AQ_MSG165:
                {
                  CANRxData[3][10]= rxHeader.StdId;
                  memcpy(&CANRxData[3][11],Data,8);
                  setBit(CANMsgIDStatus,7);
                  break;
                }
                 case AQ_MSG166:
                {
                  CANRxData[4][1]= rxHeader.StdId;
                  memcpy(&CANRxData[4][2],Data,8);
                  setBit(CANMsgIDStatus,8);
                  break;
                }
                case AQ_MSG167:
                {
                  CANRxData[4][10]= rxHeader.StdId;
                  memcpy(&CANRxData[4][11],Data,8); 
                  setBit(CANMsgIDStatus,9);
                  break;
                }
                case AQ_MSG168:
                {
                  CANRxData[5][1]= rxHeader.StdId;
                  memcpy(&CANRxData[5][2],Data,8);
                  setBit(CANMsgIDStatus,10);
                  break;
                }
                case AQ_MSG169:
                {
                  CANRxData[5][10]= rxHeader.StdId;
                  memcpy(&CANRxData[5][11],Data,8) ;
                  setBit(CANMsgIDStatus,11);
                  break;
                }
                case AQ_MSG184:  //0x184 fault info of inverter
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
                case AQ_MSG185:
                {
                  CANRxData[6][10]= rxHeader.StdId;
                  memcpy(&CANRxData[6][11],Data,8) ;
                  setBit(CANMsgIDStatus,13);
                  break;
                }
                case AQ_MSG186:
                {
                  CANRxData[7][1]= rxHeader.StdId;
                  memcpy(&CANRxData[7][2],Data,8);
                  setBit(CANMsgIDStatus,14);
                  break;
                }
                case AQ_MSG187:
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
