#ifndef _BLE_APP_H_
#define _BLE_APP_H_
#include "main.h"

void sendAllBLEMsg();
void checkBLEConnectStatus(void);

void enableBLE();
void sendAllBLEMsg();
void checkBLEConnectStatus(void);
extern uint8_t isBLEConnect;
extern uint8_t RxTemp;

#endif

