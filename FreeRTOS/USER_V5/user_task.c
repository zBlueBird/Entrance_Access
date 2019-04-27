#include "user_task.h"
#include "led.h"
#include "usart1.h"
#include "keyscan.h"

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
	  {0x00, 0x01, 0x02, 0x0A,},
	  {0x03, 0x04, 0x05, 0x0B,},
		{0x06, 0x07, 0x08, 0x0C,},
		{0x09, 0x0D, 0x0E, 0x0F,},
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
    xQueue = xQueueCreate( APP_MSG_QUEUE_LEN, sizeof( APP_MsgStg ) );  
    while (1)
    {
			  APP_MsgStg app_msg;
        while(xQueueReceive(xQueue, (void*)&app_msg, 2000))
			  {
					  if (app_msg.msg_type == APP_MSG_KEYSCAN)
						{
							  Key_ValueTypeDef *p_key = (Key_ValueTypeDef*)app_msg.p_msg_value;
							  printf("[APP] app msg: keyscan, ken = %d,value = %#x\r\n", 
							  app_msg.msg_len, g_key_map[p_key->row_index][p_key->col_index]);
						}
				}
    }

}

void app_send_msg(APP_MsgType type, uint8_t len, void *p_msg_value)
{
	  g_app_msg.msg_type = type;
	  g_app_msg.msg_len = len;
	  g_app_msg.p_msg_value = p_msg_value;
	  xQueueSend(xQueue, (void*)&g_app_msg, (portTickType )0xff);
}

void user_task2(void *pvParamters)
{
    while (1)
    {
        LED1(ON);
        Delay(0x6FFF0);
        LED1(OFF);
        Delay(0x6FFF0);
        printf("task2 is start \r\n");
    }
}

