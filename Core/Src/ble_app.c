#include "ble_app.h"
#include "main.h"
#include "bsp_usartx_cc2541.h"
#include "string.h"
#include "usart.h"

char BLE_AT[]="AT\r\n";
char BLE_GET_VERSION[]="AT+VERION=?\r\n";

char BLE_GET_STATE[]="AT+SYS_STATE=?\r\n";

char BLE_SET_NAME[]="AT+NAME=";
char BLE_SET_PWD[]="AT+PSWD=";
char BLE_TX_POWER[]="AT+TX=";
char BLE_RX_POWER[]="AT+RX=";

char BLE_SET_ID[]="AT+ADV_MFR_SPC=424C455F4F42445F4151\r\n";// BLE_AQ

uint8_t isBLEConnect=0;
extern uint8_t CANDataAvalFlag;
uint8_t RxTemp=0;
uint8_t BLEStopSendMsgDelayCount=0;

void checkBLEConnectStatus(void)
{
    CC2541_Send_CMD(BLE_GET_STATE,0);
    if(strstr(uart_buff,"5"))  
    {
      isBLEConnect = 1;    
    }
    else
    {
      isBLEConnect =0;
    }
}




void enableBLE()
{
  HAL_GPIO_WritePin(BLE_WAKEUP_GPIO_Port,BLE_WAKEUP_Pin,GPIO_PIN_RESET); // set low to wake up BLE,
  HAL_GPIO_WritePin(BLE_RESET_GPIO_Port,BLE_RESET_Pin,GPIO_PIN_SET);  // RESET PIN HIGH, STOP RESET BLE
    
  HAL_UART_Receive_IT(&huart1, &RxTemp,1);   // START receive UART DATA
  HAL_Delay(10);
  CC2541_Send_CMD(BLE_SET_ID,0);  
}

void sendAllBLEMsg()
{
  if((CANDataAvalFlag!=0)&&(BLEStopSendMsgDelayCount!=0)&&(isBLEConnect ==1) )
    {
      
      sendBLEMsg();
      if(BLEStopSendMsgDelayCount>0) BLEStopSendMsgDelayCount--;    
    }
}