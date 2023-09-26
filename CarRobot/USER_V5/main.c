

#include "stm32f10x.h"
#include "led.h"
#include "FreeRTOSConfig.h"

#include  "FreeRTOS.h"
#include  "task.h"

#include  "usart1.h"
#include  "oled.h"

#define  USER_TASK1_PRIO  5    //优先级1
#define  USER_TASK1_STK   128  //任务堆栈128
TaskHandle_t  user_task1_handler;  //任务句柄
void app_msg_handle_task(void *pvParamters);

#define  USER_TASK2_PRIO  3    //优先级2
#define  USER_TASK2_STK   128  //任务堆栈128
TaskHandle_t  user_task2_handler;  //任务句柄
void user_task2(void *pvParamters);

#define  OLED_DISPLAY_PRIO  4    //优先级1
#define  OLED_DISPLAY_STK   128  //任务堆栈128
TaskHandle_t  oled_display_handler;  //任务句柄
void oled_display_task(void *pvParamters);

/**************************************
 * Entrance main
 *************************************/
int main(void)
{
    /* 配置系统时钟为72M */
    SystemInit();

    USART1_Config();
    NVIC_Configuration();

    printf("\r\n [Entrance Access System] \r\n");


    xTaskCreate((TaskFunction_t) app_msg_handle_task,
                (const char *) "app_msg_handle_task",
                (uint16_t) USER_TASK1_STK,
                (void *) NULL,
                (UBaseType_t) USER_TASK1_PRIO,
                (TaskHandle_t *) &user_task1_handler);

    xTaskCreate((TaskFunction_t) user_task2,
                (const char *) "user_task2",
                (uint16_t) USER_TASK2_STK,
                (void *) NULL,
                (UBaseType_t) USER_TASK2_PRIO,
                (TaskHandle_t *) &user_task2_handler);

    vTaskStartScheduler();


}


