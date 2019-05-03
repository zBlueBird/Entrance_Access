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


/*=================================================================
*               Local Micro
==================================================================*/
#define APP_MSG_QUEUE_LEN    20

/*=================================================================
*               Local Variables
==================================================================*/
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

void Delay(__IO u32 nCount)
{
    for (; nCount != 0; nCount--);
}

void app_msg_handle_task(void *pvParamters)
{
    xQueue = xQueueCreate(APP_MSG_QUEUE_LEN, sizeof(APP_MsgStg));

    /*keyscan init*/
    keyscan_module_init();

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
        while (xQueueReceive(xQueue, (void *)&app_msg, 2000))
        {
            if (app_msg.msg_type == APP_MSG_KEYSCAN)
            {
                Key_ValueTypeDef *p_key = (Key_ValueTypeDef *)app_msg.p_msg_value;
                printf("[APP] app msg: keyscan, ken = %d,value = %#x\r\n",
                       app_msg.msg_len, g_key_map[p_key->row_index][p_key->col_index]);
            }
            else if (app_msg.msg_type == APP_MSG_RC522)
            {
                uint8_t flag = *(uint8_t *)app_msg.p_msg_value;
                printf("[APP] app msg: RC522, len = %d, value = %#x\r\n",
                       app_msg.msg_len, flag);

                if (flag == 1)
                {
                    gCurrentState = ENTER_STATE_ALLOWED;
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
                    RELAY_ACTION(ENTER_ENABLE);
                }
                else if ((!state) && (Bit_RESET == GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_13)))
                {
                    is_persion_arrived = FALSE;
                    gCurrentState = ENTER_STATE_IDLE;
                    OLED_Clear();
                    RELAY_ACTION(ENTER_DISABLE);
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
        case ENTER_STATE_IDLE://ø’œ–◊¥Ã¨
            {

                OLED_Clear();
                OLED_ShowCHinese(30, 0, 0); //ª∂”≠π‚¡Ÿ
                OLED_ShowCHinese(48, 0, 1); //ª∂”≠π‚¡Ÿ
                OLED_ShowCHinese(66, 0, 2); //ª∂”≠π‚¡Ÿ
                OLED_ShowCHinese(84, 0, 3); //ª∂”≠π‚¡Ÿ
                signal_n ++;
                if (signal_n % 9 > 6)
                {
                    OLED_ShowEnterIdle16x32(0, 3, 1);
                    OLED_ShowEnterIdle16x32(16, 3, 1);
                    OLED_ShowEnterIdle16x32(32, 3, 2);
                    OLED_ShowEnter32x32(48, 3, 9); //°§
                    OLED_ShowEnterIdle16x32(80, 3, 5);
                    OLED_ShowEnterIdle16x32(96, 3, 6);
                    OLED_ShowEnterIdle16x32(112, 3, 7);
                }
                else if (signal_n % 9 <= 3)
                {
                    OLED_ShowEnterIdle16x32(32, 3, 2);
                    OLED_ShowEnter32x32(48, 3, 9); //°§
                    OLED_ShowEnterIdle16x32(80, 3, 5);
                }
                else if ((signal_n % 9 <= 6) && (signal_n % 9 > 3))
                {
                    OLED_ShowEnterIdle16x32(16, 3, 1);
                    OLED_ShowEnterIdle16x32(32, 3, 2);
                    OLED_ShowEnter32x32(48, 3, 9); //°§
                    OLED_ShowEnterIdle16x32(80, 3, 5);
                    OLED_ShowEnterIdle16x32(96, 3, 6);
                }
            }
            break;
        case ENTER_STATE_WAITING_BRUSH_CARD://µ»¥˝À¢ø®
            {
                OLED_Clear();

                OLED_ShowCHinese(30, 0, 0); //ª∂”≠π‚¡Ÿ
                OLED_ShowCHinese(48, 0, 1); //ª∂”≠π‚¡Ÿ
                OLED_ShowCHinese(66, 0, 2); //ª∂”≠π‚¡Ÿ
                OLED_ShowCHinese(84, 0, 3); //ª∂”≠π‚¡Ÿ

                OLED_ShowEnter32x32(13, 3, 3); //«Î
                OLED_ShowEnter32x32(47, 3, 4); //À¢
                OLED_ShowEnter32x32(83, 3, 5); //ø®
            }
            break;
        case ENTER_STATE_ALLOWED:
            {
                OLED_Clear();

                OLED_ShowCHinese(30, 0, 0); //ª∂”≠π‚¡Ÿ
                OLED_ShowCHinese(48, 0, 1); //ª∂”≠π‚¡Ÿ
                OLED_ShowCHinese(66, 0, 2); //ª∂”≠π‚¡Ÿ
                OLED_ShowCHinese(84, 0, 3); //ª∂”≠π‚¡Ÿ

                OLED_ShowEnter32x32(13, 3, 0); //√≈
                OLED_ShowEnter32x32(47, 3, 1); //“—
                OLED_ShowEnter32x32(83, 3, 2); //ø™
            }

            break;
        case ENTER_STATE_BRUSH_CARD_FAILED:
            {
                OLED_Clear();

                OLED_ShowCHinese(30, 0, 0); //ª∂”≠π‚¡Ÿ
                OLED_ShowCHinese(48, 0, 1); //ª∂”≠π‚¡Ÿ
                OLED_ShowCHinese(66, 0, 2); //ª∂”≠π‚¡Ÿ
                OLED_ShowCHinese(84, 0, 3); //ª∂”≠π‚¡Ÿ

                OLED_ShowEnter32x32(13, 2, 6); //«Î
                OLED_ShowEnter32x32(47, 2, 7); //÷ÿ
                OLED_ShowEnter32x32(83, 2, 8); //À¢
            }
            break;
        default:
            break;
        }
    }
}

