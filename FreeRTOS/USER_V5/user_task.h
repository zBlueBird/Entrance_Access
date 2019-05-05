
#ifndef _USER_TASK_H_
#define _USER_TASK_H_

#include "FreeRTOS.h"

#define  KEYSCAN_INPUT_TIMEOUT    10000//keyscan input timeout 10s

typedef enum
{
    ENTER_STATE_IDLE,
    ENTER_STATE_WAITING_BRUSH_CARD,
    ENTER_STATE_ALLOWED,
    ENTER_STATE_BRUSH_CARD_FAILED,
    ENTER_STATE_KEYCODE_INPUT_ERROR,
    ENTER_STATE_KEYCODE_INPUTING,
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

typedef struct
{
    uint8_t key_input_state;
    uint8_t key_input_index;
    uint8_t *p_input_buf;
} App_Key_Input;

extern void app_send_msg(APP_MsgType type, uint8_t len, void *p_msg_value);


#endif
