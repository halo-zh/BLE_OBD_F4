
#include "stdint.h"

extern uint8_t descProcessDone;
void countDTC(void);
void countDTCNow(void);
void clearAllDtc(void);
void saveDtcToFlash(void);
void copyDtcFromFlash(void);
extern uint8_t DTCnumber;

void monitorDTC();