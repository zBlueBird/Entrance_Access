/**
*     relay.c
**/

#include "relay.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "misc.h"
#include "usart1.h"
#include "user_task.h"


/**
*  realy module init
*  none
*/
void relay_module_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* config the extiline clock and AFIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

    /* EXTI line gpio config(PC6~PC9) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // ÍÆÍìÊä³ö
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOD, GPIO_Pin_2);

    printf("[RELAY] relay_module_init.\r\n");
}



