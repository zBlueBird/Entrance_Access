
#ifndef _USER_TASK_H_
#define _USER_TASK_H_

#include "FreeRTOS.h"

typedef enum
{
    ENTER_STATE_IDLE,
    ENTER_STATE_WAITING_BRUSH_CARD,
    ENTER_STATE_ALLOWED,
    ENTER_STATE_BRUSH_CARD_FAILED,
} sys_state;

typedef enum
{
    APP_MSG_KEYSCAN = 1,
    APP_MSG_RC522,
    APP_MSG_HCSR505,
} APP_MsgType;

typedef struct
{
    uint8_t msg_type;
    uint8_t msg_len;
    uint8_t *p_msg_value;
} APP_MsgStg;


extern void app_send_msg(APP_MsgType type, uint8_t len, void *p_msg_value);


#endif
