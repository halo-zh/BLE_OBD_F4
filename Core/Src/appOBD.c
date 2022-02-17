#include "can.h"
#include "gpio.h"
#include "appOBD.h"
#include "string.h"

uint16_t engineRPM=0;
uint8_t throttlePercent=0;
//                     1   2  3   4    5   6   7   8  9  10   12  13  14  15  16  17
uint8_t VIN[17]    = {'V','I','N',':','X','1','1','V','M','S','L','E','3','X','1','M'};
uint8_t ECUNAME[20] = {'E','C','U','X','-','H','W',':','V','1','.','3','S','W',':','V','3','.','0'};
uint16_t DTCs[8]={0,0};

extern uint8_t funcReq;

struct _dtc
{
  uint16_t isDTC;
  uint16_t dtc;
};

#define DTC_MAX_CNT 20
#pragma pack(4)
struct _dtc dtc[DTC_MAX_CNT];
#pragma pack()
struct _dtc dtcNow[DTC_MAX_CNT];
uint8_t DTCnumber=0;
uint8_t dtcData[DTC_MAX_CNT*2];
uint8_t dtcNowData[DTC_MAX_CNT*2];
void countDTC(void)
{
  DTCnumber =0;
  for(uint8_t i=0; i<DTC_MAX_CNT;i++)
  {
    if(dtc[i].isDTC == 1)
    {
     
      dtcData[DTCnumber*2]= (uint8_t)(dtc[i].dtc>>8);
      dtcData[DTCnumber*2+1]= (uint8_t)dtc[i].dtc;
      DTCnumber++;
    }
    
  }
}

void countDTCNow(void)
{
  DTCnumber =0;
  for(uint8_t i=0; i<DTC_MAX_CNT;i++)
  {
    if(dtcNow[i].isDTC == 1)
    {   
      dtcData[DTCnumber*2]= (uint8_t)(dtcNow[i].dtc>>8);
      dtcData[DTCnumber*2+1]= (uint8_t)dtcNow[i].dtc;
      DTCnumber++;
    }
    
  }
}

void clearAllDtc()
{
  memset(dtc,0X00,sizeof(dtc));
  memset(dtcNow,0x00,sizeof(dtcNow));
  
}
extern uint8_t preDtcCnt;
uint8_t data[1];
void monitorDTC()
{

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
}

uint32_t eraseResult=0;
void saveDtcToFlash()
{
  uint32_t startAddr = 0x080e0000;
  HAL_FLASH_Unlock();
  FLASH_EraseInitTypeDef eraseType;
  eraseType.Banks = FLASH_BANK_1;
  eraseType.Sector =FLASH_SECTOR_11;
  eraseType.NbSectors = 1;
  eraseType.TypeErase = FLASH_TYPEERASE_SECTORS;
  eraseType.VoltageRange = VOLTAGE_RANGE_3;
  HAL_FLASHEx_Erase(&eraseType,&eraseResult);
  
 for(uint8_t i=0;i<DTC_MAX_CNT;i++)
 {
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,startAddr,*(uint32_t*)&dtc[i]);
    startAddr+=4;	
 }
  HAL_FLASH_Lock();
  
}

void copyDtcFromFlash()
{
  
  uint8_t i=0;
  uint32_t* ptr =(uint32_t *)0x080e0000;
  
  for(i=0;i<DTC_MAX_CNT;i++)
  {
    *(uint32_t *)&dtc[i] = *(ptr++);
  }

  
  if(dtc[0].dtc == 0xffff)
  {
    memset((uint8_t*)&dtc,0x00,sizeof(dtc));
    saveDtcToFlash();
  }
}




void onRecvCommMessage(uint32_t canID,uint8_t canData[],uint8_t DataLen)
{
  
  uint32_t tempRPM=0;
  if(canID == 0x186)
  {
    tempRPM = (canData[0]+(canData[1]<<8)+(canData[2]<<16)+(canData[3]<<24));
    if(tempRPM <= 0x7fffffff)
      engineRPM = (uint16_t)tempRPM;
    else
      engineRPM= (uint32_t)tempRPM-0x7fffffff;
    
    throttlePercent = canData[4];   
  }
  
  if(canID == 0x184)
  {
    memset(dtcNow,0x00,sizeof(dtcNow));
    if((canData[3]&0x20) !=0)  // throttle fault  04->20
    {    
      dtc[0].isDTC =1;
      dtc[0].dtc= 0x0120; 
      dtcNow[0].isDTC =1;
      dtcNow[0].dtc= 0x0120;
    }
     
    if((canData[3]&0x80) !=0)  // MOTOR overheat 01->80
    {    
      dtc[1].isDTC =1;
      dtc[1].dtc= 0x0a2b;
      dtcNow[1].isDTC =1;
      dtcNow[1].dtc= 0x0a2b;
    }
    
    if((canData[2]&0x10) !=0)  // over volatagem08->10
    {
      dtc[2].isDTC =1;
      dtc[2].dtc=  0X0AFB;
      dtcNow[2].isDTC =1;
      dtcNow[2].dtc=  0X0AFB;
    }
    
    
    if((canData[2]&0x04) !=0)  // low volatage  20->04
    {     
      dtc[3].isDTC =1;
      dtc[3].dtc=  0X0AFa;
      dtcNow[3].isDTC =1;
      dtcNow[3].dtc=  0X0AFa;
    }
    
    if((canData[2]&0x01) !=0)  // HALL fault 80->01
    {      
      dtc[4].isDTC =1;
      dtc[4].dtc=0x0A3f;
      dtcNow[4].isDTC =1;
      dtcNow[4].dtc=0x0A3f;
    }
    
    if((canData[2]&0x40) !=0)  // drive fault 02->40
    {      
      dtc[5].isDTC =1;
      dtc[5].dtc=0x0A1B;
      dtcNow[5].isDTC =1;
      dtcNow[5].dtc=0x0A1B;
    }
    
    if((canData[2]&0x08) !=0)  // INVERTER OVER TEMP 10->08
    {      
      dtc[6].isDTC =1;
      dtc[6].dtc=0x0A3C;
      dtcNow[6].isDTC =1;
      dtcNow[6].dtc=0x0A3C;
    }
    
  }
  
  
}



uint8_t descProcessDone = 0;
void obdDiagReqHandler(uint8_t data[],uint8_t * dataLen)
{
      uint8_t reqData[50];
      uint8_t *resData = data;
      uint8_t resDataLen=0;
      
      uint8_t reqDataLen = *dataLen;
      
      uint16_t comPRM=0;
      
      uint8_t subSIDNotSupport=0;
      
   
       for(uint8_t i=0;i<reqDataLen;i++)
      {
        reqData[i]= data[i];
      }
      
      for(uint8_t i=0;i<100;i++)
      {
        resData[i]= 0x00;
      }
      
#define SID     reqData[0]
#define SubID   reqData[1]
     
     
      /* prepare positive resp service ID*/
      resData[resDataLen++]= reqData[0]+0x40;
      
      /* service 01  read data */
      if((SID== 0x01 ) &&(reqDataLen >=2))
      {
         switch(SubID)
         {
              case 0x00:
              {
                 resData[ resDataLen++ ]	= 0x00;
                 resData[ resDataLen++ ]	= 0x00;//mode1sup [ 0 ];00000000      PID01-08
                 resData[ resDataLen++ ]	= 0x10;//mode1sup [ 1 ];00010000      PID09-10
                 resData[ resDataLen++ ]	= 0x80;//mode1sup [ 2 ];10000000      PID11-18
                 resData[ resDataLen++ ]	= 0x00;//mode1sup [ 3 ];00000000      PID19-20
                 descProcessDone = 1;
                 break;
              }
              case 0x0C:     // enginge RPM  2byte  1/4 RPM PER BIT	
             {

                   resData[resDataLen++]=0x0c;
                   comPRM = engineRPM *4;
                   resData [ resDataLen++ ]	= (uint8_t)(comPRM>>8);
                   resData [ resDataLen++ ] = (uint8_t)comPRM;
                   descProcessDone = 1;
                   break;
         
             }
             case 0x11: //  throttle percent 1byte 0-100%  100/255 per bit
             {          
                   resData[resDataLen++] = 0x11;
                   resData [ resDataLen++ ]	= throttlePercent*255/100;
                   descProcessDone = 1;
                   break;
             }
             default:
             {
               subSIDNotSupport=1;
                break;
             }
         }
          
             
      }
      else if((SID== 0x02 ) &&(reqDataLen>=3))
      {
          #define FrameNumb reqData[2]
         for(uint8_t i=0;i<(reqDataLen-1)/2;i++)
         {
             switch(reqData[i*2+1])
             {
                  case 0x00:
                  {
                     resData[ resDataLen++ ]	= 0x00;
                     resData[ resDataLen++ ]	= 0x00;
                     resData[ resDataLen++ ]	= 0x40;//mode1sup [ 0 ];01000000      PID01-08
                     resData[ resDataLen++ ]	= 0x10;//mode1sup [ 1 ];00010000      PID09-10
                     resData[ resDataLen++ ]	= 0x80;//mode1sup [ 2 ];10000000      PID11-18
                     resData[ resDataLen++ ]	= 0x00;//mode1sup [ 3 ];00000000      PID19-20
                     descProcessDone = 1;
                     break;
                  }
                  case 0x02:     // DTC cause Freeze fram to store	
                 {

                       resData[resDataLen++] = 0x02;
                       resData[ resDataLen++ ]	= 0x00;  //frame #
                     
                       resData [ resDataLen++ ]	= 0x0a;
                       resData [ resDataLen++ ] = 0xfa;
                       descProcessDone = 1;
                       break;
             
                 }
                  case 0x0C:     // enginge RPM  2byte  1/4 RPM PER BIT	
                 {

                       resData[resDataLen++]=0x0c;
                        resData[ resDataLen++ ]	= 0x00;
                       comPRM = 200;
                       resData [ resDataLen++ ]	= (uint8_t)(comPRM>>8);
                       resData [ resDataLen++ ] = (uint8_t)comPRM;
                       descProcessDone = 1;
                       break;
             
                 }
                 case 0x11: //  throttle percent 1byte 0-100%  100/255 per bit
                 {          
                       resData[resDataLen++] = 0x11;
                       resData[ resDataLen++ ]	= 0x00;
                       resData [ resDataLen++ ]	= 200;
                       descProcessDone = 1;
                       break;
                 }
                 default:
                 {
                   subSIDNotSupport=1;
                    break;
                 }
             }
         }
          
             
      }
      else if(SID == 0x03)  /*read DTC */
      {
        countDTC();
        if(DTCnumber == 0)
        {
          resData[resDataLen++] = 0x00;
        }
        else
        {     
          resData[resDataLen++] = DTCnumber;
          memcpy(&resData[resDataLen],dtcData,DTCnumber*2);
          resDataLen+= DTCnumber*2;    
        }
        descProcessDone = 1;
        
        
      }
      
      else if(SID == 0x04)  /*read DTC */
      {
        clearAllDtc();
        
        descProcessDone = 1;
        
        
      }
      else if(SID == 0x07)  /*read DTC */
      {
        countDTCNow();
        if(DTCnumber == 0)
        {
          resData[resDataLen++] = 0x00;
        }
        else
        {     
          resData[resDataLen++] = DTCnumber;
          memcpy(&resData[resDataLen],dtcData,DTCnumber*2);
          resDataLen+= DTCnumber*2;    
        }
        descProcessDone = 1;
        
        
      }
      else if((SID == 0x09)&&(reqDataLen >= 2))  /*read Scooter info */
      {
        switch(SubID)
        {
            case 0x00: /* --- VIN --- */
           {
              resData[ resDataLen++ ]	= 0x00;
 	      resData[resDataLen++]	= 0x40;   //mode1sup [ 0 ];01000000      PID01-08
	      resData[resDataLen++]	= 0x40;  //mode1sup [ 1 ];01000000      PID09-10
	      resData[resDataLen++]	= 0x00;
	      resData[resDataLen++]	= 0x00;//mode1sup [ 2 ];00000000      PID11-18
              descProcessDone = 1;
              break;   
   	   }	  
     	   case 0x02: /* --- VIN ---  17 ASCII  */
	   {
              
              resData[resDataLen++] = 0x02;
              resData[resDataLen++] = 0x01;
              for(uint8_t _iLoop1=0;_iLoop1<17;_iLoop1++)
              {
                  resData[resDataLen++] = VIN[_iLoop1];
                  descProcessDone = 1;
              }	       
    		break;   
            }		        	  
	    case 0x0A: /* --- ECU Name ---  MAX 20 ASCII  */
	   {
             
                resData[resDataLen++] = 0x0a;
                resData[resDataLen++] = 0x01;
                for(uint8_t _iLoop1=0;_iLoop1<20;_iLoop1++)
                {
                    resData[resDataLen++] = ECUNAME[_iLoop1];
                    descProcessDone = 1;
                }	           
                  break;   
	     }
             default:
            {
              subSIDNotSupport=1;  //NRC 0X12
      		break;
	    }
        }
        
      }
      else
      {
        // service not supported  NRC 0X11
        resDataLen=0;
        resData[resDataLen++] = 0x7f;
        resData[resDataLen++] = SID;
        resData[resDataLen++] = 0x11;
        descProcessDone=1;
        if(funcReq == 1)  // if function req,no response needed
        {
          funcReq =0;
          descProcessDone=0;
          
        }
      }
      
      if(subSIDNotSupport == 1)
      {
        resDataLen=0;
        resData[resDataLen++] = 0x7f;
        resData[resDataLen++] = SID;
        resData[resDataLen++] = 0x7E;
       
        
        descProcessDone=1;
        if(funcReq == 1)  // if function req
        {
          funcReq =0;
          descProcessDone=0;
          
        }
      }
      

      
      
      if(descProcessDone == 1)
      {
       
         *dataLen = resDataLen;
         HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_7);
      }
      else
      {
        *dataLen =0;
      }
}





