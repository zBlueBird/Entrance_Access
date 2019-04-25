

#include "stm32f10x.h"
#include "led.h"
#include "FreeRTOSConfig.h"

#include  "FreeRTOS.h"
#include  "task.h"

#include  "usart1.h"
#include  "keyscan.h"



#define  USER_TASK1_PRIO  3    //优先级1
#define  USER_TASK1_STK   128  //任务堆栈128
TaskHandle_t  user_task1_handler;  //任务句柄
void user_task1(void *pvParamters);

#define  USER_TASK2_PRIO  2    //优先级2
#define  USER_TASK2_STK   128  //任务堆栈128
TaskHandle_t  user_task2_handler;  //任务句柄
void user_task2(void *pvParamters);

/*
 * 函数名：main
 * 描述  ：主函数
 * 输入  ：无
 * 输出  ：无
 */
int main(void)
{
    /* 配置系统时钟为72M */
    SystemInit();

    /* LED 端口初始化 */
    LED_GPIO_Config();


    USART1_Config();
    NVIC_Configuration();

    printf("\r\n [Entrance Access System] \r\n");

    keyscan_module_init();
//  while(1)
//  uart_loop_proc();

    xTaskCreate((TaskFunction_t) user_task1,
                (const char *) "user_task1",
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


/******************* (C) COPYRIGHT 2011 野火嵌入式开发工作室 *****END OF FILE****/
