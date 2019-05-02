#include "user_task.h"
#include "led.h"
#include "usart1.h"
#include "keyscan.h"
#include "rc522_handle.h"
#include "oled.h"
#include "hcsr505.h"
#include "relay.h"

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
            else if (app_msg.msg_type == APP_MSG_HCSR505)
            {
                uint8_t state = *(uint8_t *)app_msg.p_msg_value;
                printf("[APP] app msg: hcsr505, len = %d, value = %#x\r\n",
                       app_msg.msg_len, state);


                if (state)
                {
                    OLED_ShowCHinese(30, 0, 0); //Ôª
                    OLED_ShowCHinese(48, 0, 1); //Æ÷
                    OLED_ShowCHinese(66, 0, 2); //¿Æ
                    OLED_ShowCHinese(84, 0, 3); //¼¼

                    RELAY_ACTION(ENTER_ENABLE);
                }
                else
                {
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

