/**
*  keyscan module
*  ROW0  PC6
*  ROW1  PC7
*  ROW2  PC8
*  ROW   PC9
*  COLUMN   PB12
*  COLUMN   PB13
*  COLUMN   PB14
*  COLUMN   PB15
*
*    Step1. keyscan 中断
*    Step2. 有中断，启动延时消抖定时器；
*    Step3. 消抖定时器超时，再次检测按键；
*    Step4. 再次检测按键有按键按下的话，就启动按键扫描流程，输出键值；
*
**/


#include "keyscan.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "misc.h"
#include "usart1.h"
#include "user_task.h"

#include "FreeRTOS.h"
#include "timers.h"
#include "queue.h"

/*=================================================================
*               Local Variables
==================================================================*/
static TimerHandle_t xKeyscanTimer = NULL;
Key_ValueTypeDef key_value = {0};
uint16_t row_arr[4] = {GPIO_Pin_6, GPIO_Pin_7, GPIO_Pin_8, GPIO_Pin_9};
uint16_t column_arr[4] = {GPIO_Pin_12, GPIO_Pin_13, GPIO_Pin_14, GPIO_Pin_15};
/*=================================================================
*               Local Function
==================================================================*/
void vKeyscan_debunce_callback(xTimerHandle pxTimer);
void keyscan_row_gpio_Config(void);
void keyscan_column_gpio_Config(void);
void keyscan_interrupt_config(bool flag);
bool keyscan_check(void);
Key_ValueTypeDef key_scan(void);

void keyscan_module_init(void)
{
    xKeyscanTimer = xTimerCreate("xKeyscanTimer", KEY_SCAN_PERIOD, pdFALSE, (void *) 1,
                                 vKeyscan_debunce_callback);
    if (NULL == xKeyscanTimer)
    {
        printf("[Keyscan] keyscan tiemr create failed");
    }
    else
    {
        printf("[Keyscan] keyscan tiemr create success.\r\n");
    }

    keyscan_row_gpio_Config();
    keyscan_column_gpio_Config();
}
void vKeyscan_debunce_callback(xTimerHandle pxTimer)
{
    printf("[Keyscan] vKeyscan_debunce_callback.\r\n");
    /*1. check keys whether is still pressed*/
    if (!keyscan_check())
    {
        keyscan_interrupt_config(TRUE);
        return;
    }
    /*2. scan key value*/
    {
        
        key_value = key_scan();
        printf("[Keyscan] key_value, row_index = %#x, col_index = %#x.\r\n", key_value.row_index,
               key_value.col_index);
    }
		
		/*3. send to app msg*/
		{
			  app_send_msg(APP_MSG_KEYSCAN, 2, &key_value);
		}

    keyscan_interrupt_config(TRUE);
}

Key_ValueTypeDef key_scan(void)
{
    Key_ValueTypeDef key_value = {0};
    for (uint8_t column_index = 0; column_index < 4; column_index ++)
    {
        key_value.col_index = column_index;

        /*1. all column output high*/
        GPIO_SetBits(GPIOB, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);

        /*2. pull low specific column */
        GPIO_ResetBits(GPIOB, column_arr[column_index]);

        /*3. check rows*/
        for (uint8_t row_index = 0; row_index < 4; row_index ++)
        {
            key_value.row_index = row_index;
            if (Bit_RESET == GPIO_ReadInputDataBit(GPIOC, row_arr[row_index]))
            {
							  GPIO_ResetBits(GPIOB, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
                return key_value;
            }
        }
    }
    key_value.col_index = 0xff;
    key_value.row_index = 0xff;

		GPIO_ResetBits(GPIOB, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
    return key_value;
}

bool keyscan_check(void)
{
    /*1. all column output low*/
    GPIO_ResetBits(GPIOB, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);

    /*2. check all row status*/
    if ((Bit_RESET == GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_6))
        || (Bit_RESET == GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_7))
        || (Bit_RESET == GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8))
        || (Bit_RESET == GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9)))
    {
        printf("[Keyscan] key is still pressed.\r\n");
        return TRUE;
    }
    else
    {
        printf("[Keyscan] key is already released.\r\n");
        return FALSE;
    }
}

void keyscan_interrupt_config(bool flag)
{
    EXTI_InitTypeDef EXTI_InitStructure;

    /* EXTI line(PC6~PC9) mode config */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource6);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource7);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource8);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource9);
    EXTI_InitStructure.EXTI_Line = EXTI_Line6 | EXTI_Line7 | EXTI_Line8 | EXTI_Line9;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //下降沿中断
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;

    /*中断源配置*/
    if (TRUE == flag)
    {
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    }
    else
    {
        EXTI_InitStructure.EXTI_LineCmd = DISABLE;
        EXTI_ClearITPendingBit(EXTI_Line6 | EXTI_Line7 | EXTI_Line8 | EXTI_Line9);
    }
		EXTI_Init(&EXTI_InitStructure);
		
		{
        NVIC_InitTypeDef NVIC_InitStructure;

        /* Configure one bit for preemption priority */
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

        /* 配置P[A|B|C|D|E]0为中断源 */
        NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;

        /*中断源配置*/        
			  /*中断源配置*/
				if (TRUE == flag)
				{
						NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
				}
				else
				{
						NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
						EXTI_ClearITPendingBit(EXTI_Line6 | EXTI_Line7 | EXTI_Line8 | EXTI_Line9);
				}
        NVIC_Init(&NVIC_InitStructure);
    }

    
}

void keyscan_row_gpio_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* config the extiline clock and AFIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);

    /* EXTI line gpio config(PC6~PC9) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    keyscan_interrupt_config(TRUE);

    //keyscan_interrupt_config(TRUE);
    {
        NVIC_InitTypeDef NVIC_InitStructure;

        /* Configure one bit for preemption priority */
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

        /* 配置P[A|B|C|D|E]0为中断源 */
        NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;

        /*中断源配置*/
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    }
}
void EXTI9_5_IRQHandler(void)
{
    if ((EXTI_GetITStatus(EXTI_Line6) != RESET) //确保是否产生了EXTI Line中断
        || (EXTI_GetITStatus(EXTI_Line7) != RESET)
        || (EXTI_GetITStatus(EXTI_Line8) != RESET)
        || (EXTI_GetITStatus(EXTI_Line9) != RESET))
    {
        printf("[Keyscan] EXTI9_5_IRQHandler.\r\n");

        keyscan_interrupt_config(FALSE);

        if (xTimerIsTimerActive(xKeyscanTimer) == pdFALSE)
        {
            xTimerStartFromISR(xKeyscanTimer, 0);
        }
        else
        {
            xTimerResetFromISR(xKeyscanTimer, 0);
        }
        //清除中断标志位
        EXTI_ClearITPendingBit(EXTI_Line6 | EXTI_Line7 | EXTI_Line8 | EXTI_Line9);
    }
}
void keyscan_column_gpio_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* config the extiline clock and AFIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* EXTI line gpio config(PC6~PC9) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /*all column pull low*/
    GPIO_ResetBits(GPIOB, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
}



