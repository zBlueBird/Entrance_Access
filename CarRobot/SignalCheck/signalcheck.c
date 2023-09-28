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

#include "signalCheck.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "misc.h"
#include "usart1.h"
#include "user_task.h"

#include "FreeRTOS.h"
#include "timers.h"
#include "queue.h"

#define  A 0x0A //PC2
#define  B 0x0B //PC4
#define  C 0x0C //PC5
#define  D 0x0D //PC11

#define ABCD  ((A << 12) | (B << 8) | (C << 4) | (D << 0x00))
#define BCDA  ((B << 12) | (C << 8) | (D << 4) | (A << 0x00))
#define CDAB  ((C << 12) | (D << 8) | (A << 4) | (B << 0x00))
#define DABC  ((D << 12) | (A << 8) | (B << 4) | (C << 0x00))

#define DCBA  ((D << 12) | (C << 8) | (B << 4) | (A << 0x00))
#define CBAD  ((C << 12) | (B << 8) | (A << 4) | (D << 0x00))
#define BADC  ((B << 12) | (A << 8) | (D << 4) | (C << 0x00))
#define ADCB  ((A << 12) | (D << 8) | (C << 4) | (B << 0x00))

typedef enum state{
	MOTOR_STATE_0,
	MOTOR_STATE_1,
	MOTOR_STATE_2,
}motor_state;



#define  SIGNAL_TIMEOUT_PERIOD    2000
/*=================================================================
*               Local Variables
==================================================================*/
static TimerHandle_t xSignalTimeoutTimer = NULL;
uint16_t g_signal_map = 0x00;
motor_state g_motor_state = MOTOR_STATE_0;
car_state g_car_state = CAR_STATE_STOP;
/*=================================================================
*               Local Function
==================================================================*/
void signal_timeout_callback(xTimerHandle pxTimer);
void signal_gpio_Config(void);


void signal_module_init(void)
{
    xSignalTimeoutTimer = xTimerCreate("xKeyscanTimer", SIGNAL_TIMEOUT_PERIOD, pdFALSE, (void *) 1,
                                 signal_timeout_callback);
    if (NULL == xSignalTimeoutTimer)
    {
        printf("[Keyscan] keyscan tiemr create failed");
    }
    else
    {
        printf("[Keyscan] keyscan tiemr create success.\r\n");
    }

    signal_gpio_Config();
}

void signal_timeout_callback(xTimerHandle pxTimer)
{
    printf("[Keyscan] signal_timeout_callback.\r\n");
    
	
	g_motor_state = MOTOR_STATE_0;
}

car_state signal_get_state(void)
{
	if (g_motor_state == MOTOR_STATE_1)
	{
		if ((g_signal_map == ABCD) || (g_signal_map == BCDA) || (g_signal_map == CDAB)||(g_signal_map == DABC))
		{
			
			printf("[CAR] go backward\r\n");
			return CAR_STATE_BACKWARD;
		}
		
		if ((g_signal_map == DCBA) || (g_signal_map == CBAD) || (g_signal_map == BADC)||(g_signal_map == ADCB))
		{
			printf("[CAR] go forward\r\n");
			return CAR_STATE_FORWARD;
		}
	}
	
	if (g_motor_state == MOTOR_STATE_2)
	{
		if ((g_signal_map == ABCD) || (g_signal_map == BCDA) || (g_signal_map == CDAB)||(g_signal_map == DABC))
		{
			printf("[CAR] go right\r\n");
			return CAR_STATE_RIGHT;
		}
		
		if ((g_signal_map == DCBA) || (g_signal_map == CBAD) || (g_signal_map == BADC)||(g_signal_map == ADCB))
		{
			printf("[CAR] go left\r\n");
			return CAR_STATE_LEFT;
		}
	}
	
	return CAR_STATE_STOP;
}


void signal_gpio_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

    /* config the extiline clock and AFIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);

    /* EXTI line gpio config(PC5) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_4 |GPIO_Pin_5 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;  
    GPIO_Init(GPIOC, &GPIO_InitStructure);
	

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
	
    {
        NVIC_InitTypeDef NVIC_InitStructure;

        /* Configure one bit for preemption priority */
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

        /* 配置P[A|B|C|D|E]0为中断源 */
        NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;

        /*中断源配置*/
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    }

    {
        NVIC_InitTypeDef NVIC_InitStructure;

        /* Configure one bit for preemption priority */
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

        /* 配置P[A|B|C|D|E]0为中断源 */
        NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;

        /*中断源配置*/
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    }
	

    {
        NVIC_InitTypeDef NVIC_InitStructure;

        /* Configure one bit for preemption priority */
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

        /* 配置P[A|B|C|D|E]0为中断源 */
        NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;

        /*中断源配置*/
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    }
	
	{
		EXTI_InitTypeDef EXTI_InitStructure;

		/* EXTI line(PC6~PC9) mode config */
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource2);
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource4);
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource5);
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource10);
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource11);
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource12);
		
		EXTI_InitStructure.EXTI_Line = EXTI_Line2 | EXTI_Line4 | EXTI_Line5 | EXTI_Line10 | EXTI_Line11 | EXTI_Line12;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //raising edge trrigle
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;

		EXTI_Init(&EXTI_InitStructure);
		
		
	}
	
}
void EXTI9_5_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line5) != RESET) //确保是否产生了EXTI Line中断)
    {
        //printf("[Keyscan] EXTI9_5_IRQHandler.\r\n");
		
		g_signal_map = g_signal_map << 4;
		g_signal_map = g_signal_map & (~0x000f);
		g_signal_map = g_signal_map | C;

        //清除中断标志位
        EXTI_ClearITPendingBit(EXTI_Line5);
    }
}

void EXTI15_10_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line10) != RESET)
    {
        //printf("[Keyscan] EXTI_Line10 reset.\r\n");

        //清除中断标志位
        EXTI_ClearITPendingBit(EXTI_Line10 );
		
		g_motor_state = MOTOR_STATE_1;
		xTimerStartFromISR(xSignalTimeoutTimer, 0);
    }
	
	if (EXTI_GetITStatus(EXTI_Line11) != RESET)
	{
		EXTI_ClearITPendingBit( EXTI_Line11 );
		
		//printf("[Keyscan] EXTI_Line11 reset.\r\n");
		g_signal_map = g_signal_map << 4;
		g_signal_map = g_signal_map & (~0x000f);
		g_signal_map = g_signal_map | D;
		
	}
	
	if (EXTI_GetITStatus(EXTI_Line12) != RESET)
	{
		//printf("[Keyscan] EXTI_Line12 reset.\r\n");
		EXTI_ClearITPendingBit( EXTI_Line12);
		
		g_motor_state = MOTOR_STATE_2;
		xTimerStartFromISR(xSignalTimeoutTimer, 0);
	}
}

void EXTI4_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line4) != RESET)
	
    {
		//清除中断标志位
        EXTI_ClearITPendingBit(EXTI_Line4);
        //printf("[Keyscan] EXTI4_IRQHandler.\r\n");
		
		g_signal_map = g_signal_map << 4;
		g_signal_map = g_signal_map & (~0x000f);
		g_signal_map = g_signal_map | B;

        
    }
}

void EXTI2_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line2) != RESET)
	
    {
		//清除中断标志位
        EXTI_ClearITPendingBit(EXTI_Line2);
		
        //printf("[Keyscan] EXTI2_IRQHandler.\r\n");
		g_signal_map = g_signal_map << 4;
		g_signal_map = g_signal_map & (~0x000f);
		g_signal_map = g_signal_map | A;

        
    }
}

#if 0


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

#endif




