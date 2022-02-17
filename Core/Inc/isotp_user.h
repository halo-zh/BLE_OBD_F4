#ifndef __ISOTP_USER_H__
#define __ISOTP_USER_H__


/* user implemented, print debug message */
void isotp_user_debug(const char* message, ...);

/* user implemented, send can message */
int  isotp_user_send_can(const uint32_t arbitration_id,
                         const uint8_t* data, const uint8_t size);

/* user implemented, get millisecond */
uint32_t isotp_user_get_ms(void);

extern uint8_t isOBDConnect;
extern void CANMultiInit(void);
extern void TpInit(void);
extern void TpTask(void);
extern void DescTask(void);

int can_receive(uint32_t *id, uint8_t *data, uint16_t *len);
#endif // __ISOTP_H__

