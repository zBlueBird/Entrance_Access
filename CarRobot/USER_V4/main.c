/******************** (C) COPYRIGHT 2011 Ұ��Ƕ��ʽ���������� ********************
 * �ļ���  ��main.c
 * ����    �����ص�LED1��LED2��LED3���Թ̶���Ƶ��������˸��Ƶ�ʿɵ�����
 * ʵ��ƽ̨��Ұ��STM32������
 * ��汾  ��ST3.0.0
 *
 * ����    ��fire  QQ: 313303034
 * ����    ��firestm32.blog.chinaunix.net
**********************************************************************************/

#include "stm32f10x.h"
#include "led.h"
#include "FreeRTOSConfig.h"

#include  "FreeRTOS.h"
#include  "task.h"

#include  "usart1.h"



#define  USER_TASK1_PRIO  3    //���ȼ�1
#define  USER_TASK1_STK   128  //�����ջ128
TaskHandle_t  user_task1_handler;  //������
void user_task1(void *pvParamters);

#define  USER_TASK2_PRIO  2    //���ȼ�2
#define  USER_TASK2_STK   128  //�����ջ128
TaskHandle_t  user_task2_handler;  //������
void user_task2(void *pvParamters);

/*
 * ��������main
 * ����  ��������
 * ����  ����
 * ���  ����
 */
int main(void)
{
    /* ����ϵͳʱ��Ϊ72M */
    SystemInit();

    /* LED �˿ڳ�ʼ�� */
    LED_GPIO_Config();

    USART1_Config();

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


/******************* (C) COPYRIGHT 2011 Ұ��Ƕ��ʽ���������� *****END OF FILE****/
