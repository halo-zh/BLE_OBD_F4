#include "isotp.h"
#include "main.h"
#include "can.h"
#include "appOBD.h"
/* user implemented, print debug message */
extern void obdDiagReqHandler(uint8_t data[],uint8_t * dlc);
extern void onRecvCommMessage(uint32_t,uint8_t data[8],uint8_t);
extern uint8_t data[8];
extern void canRecvMsgUpdate(void);

CAN_RxHeaderTypeDef rxHeader;
uint8_t Data[8];
uint32_t canRxID=0;
uint16_t canRxLen=0;


int8_t ret=0;
uint32_t id=0;
uint8_t tpData[8];
uint16_t len=0; 
uint8_t payload[100];
#define payLoadMaxSize 100
uint8_t payload_size=100;
uint8_t in_size;
uint8_t out_size;
/* Alloc IsoTpLink statically in RAM */
static IsoTpLink g_phylink;
static IsoTpLink g_funclink;

/* Allocate send and receive buffer statically in RAM */
static uint8_t g_isotpPhyRecvBuf[100];
static uint8_t g_isotpPhySendBuf[100];
/* currently functional addressing is not supported with multi-frame messages */
static uint8_t g_isotpFuncRecvBuf[100];
static uint8_t g_isotpFuncSendBuf[100];	

CAN_HandleTypeDef *pcan;
void CANMultiInit(void)
{
   CAN_FilterTypeDef myfilter;
   myfilter.FilterActivation = CAN_FILTER_ENABLE;
   myfilter.FilterBank = 0;
   myfilter.FilterFIFOAssignment =CAN_FILTER_FIFO0;
   myfilter.FilterMode = CAN_FILTERMODE_IDMASK;
   myfilter.FilterMaskIdHigh=0x0000;
   myfilter.FilterMaskIdLow = 0x0000;
   myfilter.FilterIdHigh = 0x0000;
   myfilter.FilterIdLow = 0x0000;
   myfilter.FilterScale = CAN_FILTERSCALE_32BIT;
  
   
  
   HAL_CAN_Start(&hcan1);
   HAL_CAN_ConfigFilter(&hcan1,&myfilter);  
   HAL_CAN_ActivateNotification(&hcan1,CAN_IT_RX_FIFO0_MSG_PENDING);
  
  
  
   
    myfilter.FilterBank = 1;
  
   
   HAL_CAN_Start(&hcan2);
   HAL_CAN_ConfigFilter(&hcan2,&myfilter);
   HAL_CAN_ActivateNotification(&hcan2,CAN_IT_RX_FIFO0_MSG_PENDING);
  
  
}

void TpInit(void)
{
  
  isotp_init_link(&g_phylink, 0x7E8,g_isotpPhySendBuf, sizeof(g_isotpPhySendBuf),g_isotpPhyRecvBuf, sizeof(g_isotpPhyRecvBuf));
  isotp_init_link(&g_funclink,0x7E8,g_isotpFuncSendBuf, sizeof(g_isotpFuncSendBuf),g_isotpFuncRecvBuf, sizeof(g_isotpFuncRecvBuf));
  
}

void TpTask(void)
{
  /* Poll link to handle multiple frame transmition */
      isotp_poll(&g_phylink);
      isotp_poll(&g_funclink);
}

uint8_t isOBDConnect=0;
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
   HAL_CAN_GetRxMessage(hcan,CAN_RX_FIFO0,&rxHeader,Data);
   canRxID = rxHeader.StdId;
   canRxLen = rxHeader.DLC;
   
   udpateCanInfo(&rxHeader,Data);
   canRecvMsgUpdate();
   
   if(((canRxID == 0x7DF) || (canRxID== 0x7e0 )))
   {
        isOBDConnect = 5;
        /* 15765-4 reqMessage  */
        if (0x7E0 == canRxID) 
        {
            isotp_on_can_message(&g_phylink, Data, canRxLen);
        } 
        else if (0x7df == canRxID) 
        {
            isotp_on_can_message(&g_funclink, Data, canRxLen);
        }    
       pcan = hcan;
       memcpy(tpData,Data,canRxLen);   
       HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_4);   
    
   }  
   else if((rxHeader.StdId ==0x184 )||(rxHeader.StdId ==0x186))
   {
     HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0);
     onRecvCommMessage(rxHeader.StdId,Data,rxHeader.DLC);
     
   }
}

uint8_t funcReq=0;
void DescTask(void)
{
    /* You can receive message with isotp_receive.
         payload is upper layer message buffer, usually UDS;
         payload_size is payload buffer size;
         out_size is the actuall read size;
         */
      ret = isotp_receive(&g_phylink, payload, payLoadMaxSize, &in_size);
      if (ISOTP_RET_OK == ret) {
          /* Handle physical addressing message */
        funcReq=0;
        obdDiagReqHandler(payload,&in_size);
      }
      
      ret = isotp_receive(&g_funclink, payload, payLoadMaxSize, &in_size);
      if (ISOTP_RET_OK == ret) {
          /* Handle functional addressing message */
         funcReq=1;
         obdDiagReqHandler(payload,&in_size);
      }            
      
      /* And send message with isotp_send */
      if((in_size >0)&&(descProcessDone ==1))
      {
        ret = isotp_send(&g_phylink, payload, in_size);
        if (ISOTP_RET_OK == ret) 
        {
            /* Send ok */
          in_size =0;
        } 
        else 
        {
            /* An error occured */
        }    
      }
  

}

uint32_t tx_mailboxes= (CAN_TX_MAILBOX0 | CAN_TX_MAILBOX1 | CAN_TX_MAILBOX2);
uint32_t *mailbox = &tx_mailboxes;



void isotp_user_debug(const char* message, ...)
{
  
}

/* user implemented, send can message */
CAN_TxHeaderTypeDef diagRespMessage;
int  isotp_user_send_can(const uint32_t arbitration_id,  const uint8_t* data, const uint8_t size)
{
  diagRespMessage.DLC =size;
  diagRespMessage.StdId = arbitration_id;
  diagRespMessage.IDE = CAN_ID_STD;
  diagRespMessage.RTR = CAN_RTR_DATA;
  HAL_CAN_AddTxMessage(pcan,&diagRespMessage,(uint8_t *)data,mailbox);
  return ISOTP_RET_OK;
}


/* user implemented, get millisecond */
uint32_t isotp_user_get_ms(void)
{
  uint32_t mstick = HAL_GetTick();
 
   return mstick;
}