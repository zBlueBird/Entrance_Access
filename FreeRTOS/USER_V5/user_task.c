#include "user_task.h"
#include "led.h"
#include "usart1.h"
#include "keyscan.h"
#include "rc522_handle.h"
#include "oled.h"
#include "hcsr505.h"
#include "relay.h"
#include "bmp.h"

#include  "FreeRTOS.h"
#include "task.h"
#include "usart1.h"
#include "queue.h"
#include "timers.h"

/*=================================================================
*               Local Micro
==================================================================*/
#define APP_MSG_QUEUE_LEN    20

/*=================================================================
*               Local Variables
==================================================================*/
App_Key_Input g_key_input = {0};
uint8_t  key_code_arr[6] = {0};
static TimerHandle_t xKeyscanTimeoutTimer = NULL;
const uint8_t  key_code[6] = {0x8, 0x02, 0xC, 0xF, 0x08, 0xB};

uint16_t signal_n = 0;
bool is_persion_arrived = FALSE;
sys_state gCurrentState = ENTER_STATE_IDLE;
APP_MsgStg g_app_msg = {0};
xQueueHandle xQueue;
uint8_t g_key_map[4][4] =
{
    {0x01, 0x02, 0x03, 0x04,},
    {0x05, 0x06, 0x07, 0x08,},
    {0x09, 0x00, 0x0a, 0x0b,},
    {0x0c, 0x0d, 0x0e, 0x0f,},
};
/*=================================================================
*               Local Functions
==================================================================*/
void vKeyscan_Input_Timeout_callback(xTimerHandle pxTimer);


void Delay(__IO u32 nCount)
{
    for (; nCount != 0; nCount--);
}

void app_msg_handle_task(void *pvParamters)
{
    xQueue = xQueueCreate(APP_MSG_QUEUE_LEN, sizeof(APP_MsgStg));

    /*keyscan init*/
    keyscan_module_init();
    g_key_input.key_input_index = 0;
    g_key_input.p_input_buf = key_code_arr;
    xKeyscanTimeoutTimer = xTimerCreate("xKeyscanTimeoutTimer", KEYSCAN_INPUT_TIMEOUT, pdFALSE,
                                        (void *) 1,
                                        vKeyscan_Input_Timeout_callback);

    /*hcsr505 module init*/
    hcsr505_module_init();

    /*oled module init*/
    OLED_Init();
    OLED_Clear();

    /*relay module init*/
    relay_module_init();

    while (1)
    {
        APP_MsgStg app_msg;
        while (xQueueReceive(xQueue, (void *)&app_msg, 8000))
        {
            if (app_msg.msg_type == APP_MSG_KEYSCAN)
            {
                Key_ValueTypeDef *p_key = (Key_ValueTypeDef *)app_msg.p_msg_value;
                printf("[APP] app msg: keyscan, ken = %d,value = %#x\r\n",
                       app_msg.msg_len, g_key_map[p_key->row_index][p_key->col_index]);

                /*密码输入*/
                if (5 >= g_key_input.key_input_index)
                {
                    g_key_input.p_input_buf[(g_key_input.key_input_index)++] =
                        g_key_map[p_key->row_index][p_key->col_index];
                }
                if (6 <= g_key_input.key_input_index)
                {
                    g_key_input.key_input_state = 0x02;//密码输入正确
                    for (uint8_t i = 0; i < 6; i++)
                    {
                        if (g_key_input.p_input_buf[i] != key_code[i])
                        {
                            g_key_input.key_input_state = 0x01;//密码输入错误
                        }
                    }
                }

                if (g_key_input.key_input_state == 0x02)//密码输入正确
                {
                    xTimerChangePeriod(xKeyscanTimeoutTimer, KEYSCAN_INPUT_TIMEOUT / 2, 0);
                    gCurrentState = ENTER_STATE_ALLOWED;
                    RELAY_ACTION(ENTER_ENABLE);//密码验证正确，开门
                }
                else if (g_key_input.key_input_state == 0x01)//密码输入错误
                {
                    xTimerChangePeriod(xKeyscanTimeoutTimer, KEYSCAN_INPUT_TIMEOUT / 5, 0);
                    gCurrentState = ENTER_STATE_KEYCODE_INPUT_ERROR;
                }
                else//密码输入中
                {
                    xTimerChangePeriod(xKeyscanTimeoutTimer, KEYSCAN_INPUT_TIMEOUT, 0);
                    gCurrentState = ENTER_STATE_KEYCODE_INPUTING;
                }


                printf("\n[Keyscan] g_key_input.key_input_index = %x\n", g_key_input.key_input_index);
            }
            else if (app_msg.msg_type == APP_MSG_RC522)
            {
                uint8_t flag = *(uint8_t *)app_msg.p_msg_value;
                printf("[APP] app msg: RC522, len = %d, value = %#x\r\n",
                       app_msg.msg_len, flag);

                if ((flag == 1) && (is_persion_arrived == TRUE))
                {
                    gCurrentState = ENTER_STATE_ALLOWED;
                    RELAY_ACTION(ENTER_ENABLE);//刷卡通过开门
                    printf("\n[Realy] Enable Relay\n");
                }
                else if ((flag == 0) && (is_persion_arrived == TRUE))
                {
                    gCurrentState = ENTER_STATE_BRUSH_CARD_FAILED;
                    RELAY_ACTION(ENTER_DISABLE);/*刷卡失败关门*/
                    printf("\n[Realy] Disable Relay Brush Failed\n");
                }
            }
            else if (app_msg.msg_type == APP_MSG_HCSR505)
            {
                uint8_t state = *(uint8_t *)app_msg.p_msg_value;
                printf("[APP] app msg: hcsr505, len = %d, value = %#x\r\n",
                       app_msg.msg_len, state);

                if ((state) && (Bit_SET == GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_13)))
                {
                    is_persion_arrived = TRUE;
                    gCurrentState = ENTER_STATE_WAITING_BRUSH_CARD;
                    printf("\n[HCSR55] Waiting for Brush Card\n");
                }
                else if ((!state) && (Bit_RESET == GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_13)))
                {
                    is_persion_arrived = FALSE;
                    gCurrentState = ENTER_STATE_IDLE;
                    RELAY_ACTION(ENTER_DISABLE);//人走之后记得关门
                    printf("\n[Realy] Disable Relay When Peopele Away\n");
                }
            }
        }
    }
}

void app_send_msg(APP_MsgType type, uint8_t len, void *p_msg_value)
{
    g_app_msg.msg_type = type;
    g_app_msg.msg_len = len;
    g_app_msg.p_msg_value = p_msg_value;
    xQueueSend(xQueue, (void *)&g_app_msg, (portTickType)0xff);
}

void user_task2(void *pvParamters)
{
    printf("\r\n RC522 task \r\n");

    while (1)
    {
        MFRC522_Module_Init();
        MFRC522_Handle();
    }
}

void oled_display_task(void *pvParamters)
{
    printf("\r\n oled_display_task \r\n");

    while (1)
    {
        //printf("\r\n oled_display_task \r\n");
        vTaskDelay(200);
        //gCurrentState = ENTER_STATE_IDLE;
        switch (gCurrentState)
        {
        case ENTER_STATE_IDLE://空闲状态
            {

                OLED_Clear();
                OLED_ShowCHinese(30, 0, 0); //欢迎光临
                OLED_ShowCHinese(48, 0, 1); //欢迎光临
                OLED_ShowCHinese(66, 0, 2); //欢迎光临
                OLED_ShowCHinese(84, 0, 3); //欢迎光临
                signal_n ++;
                if (signal_n % 9 > 6)
                {
                    OLED_ShowEnterIdle16x32(0, 3, 1);
                    OLED_ShowEnterIdle16x32(16, 3, 1);
                    OLED_ShowEnterIdle16x32(32, 3, 2);
                    OLED_ShowEnter32x32(48, 3, 9); //·
                    OLED_ShowEnterIdle16x32(80, 3, 5);
                    OLED_ShowEnterIdle16x32(96, 3, 6);
                    OLED_ShowEnterIdle16x32(112, 3, 7);
                }
                else if (signal_n % 9 <= 3)
                {
                    OLED_ShowEnterIdle16x32(32, 3, 2);
                    OLED_ShowEnter32x32(48, 3, 9); //"·"
                    OLED_ShowEnterIdle16x32(80, 3, 5);
                }
                else if ((signal_n % 9 <= 6) && (signal_n % 9 > 3))
                {
                    OLED_ShowEnterIdle16x32(16, 3, 1);
                    OLED_ShowEnterIdle16x32(32, 3, 2);
                    OLED_ShowEnter32x32(48, 3, 9); //·
                    OLED_ShowEnterIdle16x32(80, 3, 5);
                    OLED_ShowEnterIdle16x32(96, 3, 6);
                }
            }
            break;
        case ENTER_STATE_WAITING_BRUSH_CARD://等待刷卡
            {
                OLED_Clear();

                OLED_ShowCHinese(30, 0, 0); //欢迎光临
                OLED_ShowCHinese(48, 0, 1); //欢迎光临
                OLED_ShowCHinese(66, 0, 2); //欢迎光临
                OLED_ShowCHinese(84, 0, 3); //欢迎光临

                OLED_ShowEnter32x32(13, 3, 3); //请
                OLED_ShowEnter32x32(47, 3, 4); //刷
                OLED_ShowEnter32x32(83, 3, 5); //卡
            }
            break;
        case ENTER_STATE_ALLOWED:
            {
                OLED_Clear();

                OLED_ShowCHinese(30, 0, 0); //欢迎光临
                OLED_ShowCHinese(48, 0, 1); //欢迎光临
                OLED_ShowCHinese(66, 0, 2); //欢迎光临
                OLED_ShowCHinese(84, 0, 3); //欢迎光临

                OLED_ShowEnter32x32(13, 3, 0); //门
                OLED_ShowEnter32x32(47, 3, 1); //已
                OLED_ShowEnter32x32(83, 3, 2); //开
            }

            break;
        case ENTER_STATE_BRUSH_CARD_FAILED:
            {
                OLED_Clear();

                OLED_ShowCHinese(30, 0, 0); //欢迎光临
                OLED_ShowCHinese(48, 0, 1); //欢迎光临
                OLED_ShowCHinese(66, 0, 2); //欢迎光临
                OLED_ShowCHinese(84, 0, 3); //欢迎光临

                OLED_ShowEnter32x32(13, 3, 6); //请
                OLED_ShowEnter32x32(47, 3, 7); //重
                OLED_ShowEnter32x32(83, 3, 8); //刷
            }
            break;
        case ENTER_STATE_KEYCODE_INPUTING:
            {
                OLED_Clear();

                OLED_ShowCHinese(30, 0, 0); //欢迎光临
                OLED_ShowCHinese(48, 0, 1); //欢迎光临
                OLED_ShowCHinese(66, 0, 2); //欢迎光临
                OLED_ShowCHinese(84, 0, 3); //欢迎光临

                OLED_ShowEnter32x32(0, 3, 10); //密码输入中
                OLED_ShowEnter32x32(32, 3, 11); //密码输入中
                OLED_ShowEnter32x32(64, 3, 12); //密码输入中
                OLED_ShowEnter32x32(96, 3, 13); //密码输入中
                //OLED_ShowEnter32x32(112, 3, 14); //密码输入中
            }
            break;
        case ENTER_STATE_KEYCODE_INPUT_ERROR:
            {
                OLED_Clear();

                OLED_ShowCHinese(30, 0, 0); //欢迎光临
                OLED_ShowCHinese(48, 0, 1); //欢迎光临
                OLED_ShowCHinese(66, 0, 2); //欢迎光临
                OLED_ShowCHinese(84, 0, 3); //欢迎光临

                OLED_ShowEnter32x32(0, 3, 15); //密码输入错误
                OLED_ShowEnter32x32(32, 3, 16); //密码输入错误
                OLED_ShowEnter32x32(64, 3, 17); //密码输入错误
                OLED_ShowEnter32x32(96, 3, 18); //密码输入错误
            }
            break;
        default:
            break;
        }
    }
}
void vKeyscan_Input_Timeout_callback(xTimerHandle pxTimer)
{
    printf("\n[Keyscan] vKeyscan_Input_Timeout_callback \n");


    if (g_key_input.key_input_state == 0x02)//密码输入成功
    {
        //gCurrentState = ENTER_STATE_ALLOWED;
        //人俩开之后，门自动关闭
        if (is_persion_arrived == FALSE)
        {
            gCurrentState = ENTER_STATE_IDLE;
            RELAY_ACTION(ENTER_DISABLE);/*刷卡失败关门*/
        }
    }
    else if (g_key_input.key_input_state == 0x01)//密码输入错误
    {
        if (is_persion_arrived == TRUE)
        {
            gCurrentState = ENTER_STATE_WAITING_BRUSH_CARD;
        }
        else
        {
            gCurrentState = ENTER_STATE_IDLE;
            RELAY_ACTION(ENTER_DISABLE);/*刷卡失败关门*/
        }
    }

    g_key_input.key_input_index = 0;
    g_key_input.key_input_state = 0;
}
