

/* 包含头文件 ----------------------------------------------------------------*/
#include "bsp_usartx_CC2541.h"
#include <string.h>
#include <stdio.h>
#include "usart.h"
#include "can.h"

/* 私有类型定义 -----------哈喽---------------------------------------------------*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);

/* 私有宏定义 ----------------------------------------------------------------*/
/* 私有变量 ------------------------------------------------------------------*/
extern UART_HandleTypeDef huart1;
/* 扩展变量 ------------------------------------------------------------------*/
extern uint8_t RxTemp;

extern uint8_t CANRxData[20][20];

/**
  * 函数功能: 串口发送一个字节数据 
  * 输入参数: ch：待发送字符
  * 返 回 值: 无
  * 说    明：无
  */
void Usart_SendByte(uint8_t ch )
{
  	while(__HAL_UART_GET_FLAG(&huart1,UART_FLAG_TXE)==0); //循环发送,直到发送完毕
	/* 发送一个字节数据到USART2 */
	HAL_UART_Transmit(&huart1,(uint8_t *)&ch,1,0xffff);
		
}

/**
  * 函数功能: 串口发送指定长度的字符串
  * 输入参数: str：待发送字符串缓冲器
  *           strlen:指定字符串长度
  * 返 回 值: 无
  * 说    明：无
  */
void Usart_SendStr_length(uint8_t *str,uint32_t strlen )
{
	unsigned int k=0;
    do 
    {
        Usart_SendByte( *(str + k) );
        k++;
    } while(k < strlen);
}

/**
  * 函数功能: 串口发送字符串，直到遇到字符串结束符
  * 输入参数: str：待发送字符串缓冲器
  * 返 回 值: 无
  * 说    明：无
  */
void Usart_SendString(uint8_t *str)
{
	unsigned int k=0;
    do 
    {
        Usart_SendByte(*(str + k) );
        k++;
    } while(*(str + k)!='\0');
}

//中断缓存串口数据
#define UART_BUFF_SIZE      40
__IO  uint16_t uart_p = 0;
uint8_t   uart_buff[UART_BUFF_SIZE];

/**
  * 函数功能: 接收中断回调函数
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
extern uint8_t DataInFlag;
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  
    uart_buff[uart_p] =RxTemp; 
    uart_p++; 
    DataInFlag=1;
    HAL_UART_Receive_IT(&huart1,&RxTemp,1);
   
}

/**
  * 函数功能: 获取接收到的数据和长度 
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
char *get_rebuff(uint16_t *len) 
{
    *len = uart_p;
    return (char *)&uart_buff;
}

/**
  * 函数功能: 清空缓冲区
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
void clean_rebuff(void)
{
  uint16_t i=UART_BUFF_SIZE+1;
  
  uart_p = 0;
	while(i)
		uart_buff[--i]=0;
}

uint8_t CC2541_Send_CMD(char* cmd,uint8_t clean)
{ 
  uint16_t len;
  char * redata;
  uint8_t retry=5;
  uint8_t i;
  clean_rebuff();
  
  while(retry--)
  { 
    HAL_Delay(1); 
    Usart_SendString((uint8_t*)cmd);
    for(i=0;i<5;i++)
    {
      HAL_Delay(30);
      redata=get_rebuff(&len);
      if(len>0)
      {
        if(redata[0]!=0)
        {
    //      printf("send CMD: %s",cmd);
    //      printf("receive %s",redata);
        }
        if(strstr(redata,"OK"))				
        {          
          if(clean==1)
            clean_rebuff();
          return 0; 
        }      
      }
      else
      {					
        HAL_Delay(10);
      }
    } 
   }      
      if(clean==1)
              clean_rebuff();
      return 1 ;	
}



uint16_t CANMsgIDStatus=0;
extern uint8_t CANTxDelayCount;
extern uint8_t CANDataAvalFlag;




/*
  根据接收到CAN报文的有效位，使用BLE发送数据


*/


extern uint8_t ConnStopFlag;
uint8_t CC2541_Send_Data(char* data,uint8_t txlen)
{ 
      char * redata;
      uint16_t len=0;
  
 
      Usart_SendStr_length((uint8_t*)data,txlen); 
      HAL_Delay(10);
      redata=get_rebuff(&len);
      if(len>0)
      {
        if(redata[0]!=0)
        {
            if((strstr(redata,"AT+CON=STOP")) ||(strstr(redata,"ERR")))
              ConnStopFlag=1;
              
        // printf("receive %s",redata);
        }      
           clean_rebuff();
          return 1; 
              
      }
      else
      {					
        
         clean_rebuff();
        return 0;
      }
         
      	
}


static uint8_t  BLE_MSG_CNT=0;
static uint8_t BLE_MSG_LEN=0;


void calculateCheckSum(void);

void sendBLEMsg()
{
  if( CanProtocolNumb ==0 )
  {
     BLE_MSG_CNT = AQ_MSG_COUNT;
     BLE_MSG_LEN = 20;
  }
  else if(CanProtocolNumb == 1)
  {
      BLE_MSG_CNT = WOW_MSG_COUNT;
      BLE_MSG_LEN = 14;
  }

  calculateCheckSum();
  for(uint8_t i=0;i<BLE_MSG_CNT;i++)
  {
     CC2541_Send_Data((char *)CANRxData[i],BLE_MSG_LEN);
     HAL_Delay(20);
  }
   
}


void calculateCheckSum(void)
{
    for(uint8_t m=0;m<BLE_MSG_CNT; m++)
    {
      CANRxData[m][BLE_MSG_LEN-1]=0;     
      for(uint8_t n=0; n<(BLE_MSG_LEN-1);n++)
      {
        CANRxData[m][BLE_MSG_LEN-1] += CANRxData[m][n];
      }
      
    }
  
}



/******************* (C) COPYRIGHT 2015-2020 硬石嵌入式开发团队 *****END OF FILE****/
