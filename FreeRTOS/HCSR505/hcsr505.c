/**
*  hcsr505.c
*  PA13  -- EINT
*/
#include "hcsr505.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "misc.h"
#include "usart1.h"
#include "user_task.h"

/*=================================================================
*               Local Varisbles
==================================================================*/

uint8_t gHcsr505State = PEOPLE_LEAVING;

/*=================================================================
*               Local Functions
==================================================================*/
void hscr505_nvic_config(void);
void hcsr505_exit_config(void);

/**
*  hcsr505 module init
*  none
*/
void hcsr505_module_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* config the extiline clock and AFIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);

    /* EXTI line gpio config(PC6~PC9) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;  // 上拉输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    hcsr505_exit_config();

    hscr505_nvic_config();

    printf("[HCSR] hcsr505_module_init.\r\n");
}
void hscr505_nvic_config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Configure one bit for preemption priority */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    /* 配置P[A|B|C|D|E]0为中断源 */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;

    /*中断源配置*/
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void hcsr505_exit_config(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;

    /* EXTI line(PC6~PC9) mode config */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource13);
    EXTI_InitStructure.EXTI_Line = EXTI_Line13;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; //下降沿中断
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;

    /*中断源配置*/
    {
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    }

    EXTI_Init(&EXTI_InitStructure);
}

void EXTI15_10_IRQHandler(void)
{
    if ((EXTI_GetITStatus(EXTI_Line13) == SET)) //确保是否产生了EXTI Line中断
    {

        /*1. check people whether arriving*/
        if (Bit_SET == GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_13))
        {
            printf("[HCSR] EXTI15_10_IRQHandler SET.\r\n");
            gHcsr505State = PEOPLE_ARRIVING;
        }
        else if (Bit_RESET == GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_13))
        {
            printf("[HCSR] EXTI15_10_IRQHandler RESET.\r\n");
            gHcsr505State = PEOPLE_LEAVING;
        }

        /*2. send to app msg*/
        {
            app_send_msg(APP_MSG_HCSR505, sizeof(gHcsr505State), &gHcsr505State);
        }

        /*3. 清除中断标志位*/
        EXTI_ClearITPendingBit(EXTI_Line13);
    }
}
