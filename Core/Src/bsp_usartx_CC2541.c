

/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include "bsp_usartx_CC2541.h"
#include <string.h>
#include <stdio.h>
#include "usart.h"
#include "can.h"

/* ˽�����Ͷ��� -----------���---------------------------------------------------*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);

/* ˽�к궨�� ----------------------------------------------------------------*/
/* ˽�б��� ------------------------------------------------------------------*/
extern UART_HandleTypeDef huart1;
/* ��չ���� ------------------------------------------------------------------*/
extern uint8_t RxTemp;

extern uint8_t CANRxData[20][20];

/**
  * ��������: ���ڷ���һ���ֽ����� 
  * �������: ch���������ַ�
  * �� �� ֵ: ��
  * ˵    ������
  */
void Usart_SendByte(uint8_t ch )
{
  	while(__HAL_UART_GET_FLAG(&huart1,UART_FLAG_TXE)==0); //ѭ������,ֱ���������
	/* ����һ���ֽ����ݵ�USART2 */
	HAL_UART_Transmit(&huart1,(uint8_t *)&ch,1,0xffff);
		
}

/**
  * ��������: ���ڷ���ָ�����ȵ��ַ���
  * �������: str���������ַ���������
  *           strlen:ָ���ַ�������
  * �� �� ֵ: ��
  * ˵    ������
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
  * ��������: ���ڷ����ַ�����ֱ�������ַ���������
  * �������: str���������ַ���������
  * �� �� ֵ: ��
  * ˵    ������
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

//�жϻ��洮������
#define UART_BUFF_SIZE      40
__IO  uint16_t uart_p = 0;
uint8_t   uart_buff[UART_BUFF_SIZE];

/**
  * ��������: �����жϻص�����
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
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
  * ��������: ��ȡ���յ������ݺͳ��� 
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
char *get_rebuff(uint16_t *len) 
{
    *len = uart_p;
    return (char *)&uart_buff;
}

/**
  * ��������: ��ջ�����
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
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
  ���ݽ��յ�CAN���ĵ���Чλ��ʹ��BLE��������


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



/******************* (C) COPYRIGHT 2015-2020 ӲʯǶ��ʽ�����Ŷ� *****END OF FILE****/
