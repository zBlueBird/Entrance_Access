#include "user_task.h"
#include "led.h"
#include "usart1.h"
#include "bmp.h"

#include  "FreeRTOS.h"
#include "task.h"
#include "usart1.h"
#include "queue.h"
#include "timers.h"
#include "signalcheck.h"

/*=================================================================
*               Local Micro
==================================================================*/
#define APP_MSG_QUEUE_LEN    20

/*=================================================================
*               Local Variables
==================================================================*/

APP_MsgStg g_app_msg = {0};
xQueueHandle xQueue;

/*=================================================================
*               Local Functions
==================================================================*/
void vKeyscan_Input_Timeout_callback(xTimerHandle pxTimer);

extern uint16_t g_signal_map;


void Delay(__IO u32 nCount)
{
    for (; nCount != 0; nCount--);
}

void app_msg_handle_task(void *pvParamters)
{
    xQueue = xQueueCreate(APP_MSG_QUEUE_LEN, sizeof(APP_MsgStg));
										
	signal_module_init();
	printf("\r\n app_msg_handle_task init");
    while (1)
    {
        APP_MsgStg app_msg;
        if (xQueueReceive(xQueue, (void *)&app_msg, 2000))
        {
            printf("\r\n app_msg_handle_task");
        }
		else
		{
			printf("\r\n app_msg_handle_task 01 map 0x%x, car state %d", g_signal_map, signal_get_state());
			
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
        vTaskDelay(5000);
		printf("\r\n user_task2");
    }
}


void vKeyscan_Input_Timeout_callback(xTimerHandle pxTimer)
{
    printf("\n[Keyscan] vKeyscan_Input_Timeout_callback \n");


}
