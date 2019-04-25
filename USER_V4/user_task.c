#include "user_task.h"
#include "led.h"
#include  "FreeRTOS.h"
#include "task.h"
#include "usart1.h"

void Delay(__IO u32 nCount)
{
    for (; nCount != 0; nCount--);
}

void user_task1(void *pvParamters)
{

    while (1)
    {
        LED1(ON);              // ÁÁ
        Delay(0x6FFFEF);
        LED1(OFF);         // Ãð
        vTaskDelay(4000);
        printf("task1 is start \r\n");
    }

}

void user_task2(void *pvParamters)
{
    while (1)
    {
        LED2(ON);
        Delay(0x6FFF0);
        LED2(OFF);
        Delay(0x6FFF0);
        printf("task2 is start \r\n");
    }
}

