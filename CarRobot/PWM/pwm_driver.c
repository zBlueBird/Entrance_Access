
#include "pwm_driver.h"
#include "stm32f10x_tim.h"
#include "FreeRTOS.h"
#include "task.h"

/*  关闭JTAG调试模式，以使用PA15引脚 */
void JTAG_Init(void)

{	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);    //先开启开启AFIO复用时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);   //使能GPIOA外设时钟
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);//关闭JTAG，保留SWD（选择GPIO_Remap_SWJ_Disable，将失去两个调试模式，勿用！）
}

/*  配置PC0、PA15的GPIO，初始化DRV8848芯片的使能引脚 */
void MotorEn_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);  	//使能C口GPIO时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);  	//使能A口GPIO时钟

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;     		//选择推挽输出
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_0;					//指定引脚PC0,MotEn2
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_10MHz;			//设置输出速率10MHz
	GPIO_Init(GPIOC,&GPIO_InitStruct);						//初始化外设GPIOC寄存器
	GPIO_ResetBits(GPIOC, GPIO_Pin_0);
	

	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;				//选择推挽输出
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_15;					//指定引脚PA15,MotEn1
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_10MHz;			//设置输出速率10MHz
	GPIO_Init(GPIOA,&GPIO_InitStruct);						//初始化外设GPIOA寄存器
	GPIO_ResetBits(GPIOA, GPIO_Pin_15);	
}

/*  使能DRV8848芯片的nSLEEP引脚 */
void Motor_EN(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_15);                     //PA15_MotorAB_EN
	GPIO_SetBits(GPIOC, GPIO_Pin_0);                      //PC0_MotorAB_EN
}

/*    配置TIM2定时器输出PWM       */
void PWM_TIM2_Init(void)
{
	
	/* 开启时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);    //开启AFIO复用时钟，重映射使用TIM2定时器
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);   //使能GPIOA外设时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);    //使能TIM2时钟 
	
	/* GPIO初始化结构体 */
	GPIO_InitTypeDef  GPIO_InitStructure;                 
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;   //TIM2 Ch1/2/3/4
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_10MHz;        //速度 10MHz
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;          //复用推挽输出
	GPIO_Init(GPIOA,&GPIO_InitStructure);                  //初始化 GPIOA
	
    /* TIM时间基数初始化结构体 */
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseSrtructure;     
	TIM_TimeBaseSrtructure.TIM_Period= TIM2_Reload_Num_ARR;           //计数器TIMx_CNT计数，从0累加到ARR次后溢出，设置自动重装载值
	TIM_TimeBaseSrtructure.TIM_Prescaler=TIM2_Frequency_Divide_PSC;   //设置预分频系数 PSC
	TIM_TimeBaseSrtructure.TIM_ClockDivision=0;                       //1分频，用于滤波信号抖动
	TIM_TimeBaseSrtructure.TIM_CounterMode=TIM_CounterMode_Up;        //向上计数模式溢出
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseSrtructure);					  //初始化TIM2的时钟参数
	
    /* TIM输出比较功能初始化结构体 */
	TIM_OCInitTypeDef	TIM_OCInitStructure;             
	TIM_OCInitStructure.TIM_OCMode=TIM_OCMode_PWM1;     			  //选择定时器模式为比较低输出。
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;     //比较输出使能
	TIM_OCInitStructure.TIM_Pulse =(TIM2_Reload_Num_ARR+1)*0;         //比较输出脉冲宽度，设置占空比（初始化用）,即CCR
	TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_High;           //输出极性：TIM输出比较极性高
	
	
	TIM_OC1Init(TIM2,&TIM_OCInitStructure);          				  //初始化TIM2  CH1的时钟，下同	
	TIM_OC2Init(TIM2,&TIM_OCInitStructure);
	TIM_OC3Init(TIM2,&TIM_OCInitStructure);
	TIM_OC4Init(TIM2,&TIM_OCInitStructure);


	
	
	/* 初始化CH1/2/3/4 */
	TIM_OC1PreloadConfig(TIM2,TIM_OCPreload_Enable);                  //CH1 预装载使能，功能在ARR和CCR1改变时，等他们计数完一个周期再修改
	TIM_OC2PreloadConfig(TIM2,TIM_OCPreload_Enable);                  //CH2 预装载使能
	TIM_OC3PreloadConfig(TIM2,TIM_OCPreload_Enable);                  //CH3 预装载使能
	TIM_OC4PreloadConfig(TIM2,TIM_OCPreload_Enable);                  //CH4 预装载使能
	TIM_ARRPreloadConfig(TIM2,ENABLE);                                //使能 TIMx在ARR上的预装载寄存器
	
	TIM_Cmd(TIM2, ENABLE);                                            //使能 TIM2

	
}

#if 0
void pwm_init()
{
	JTAG_Init();	
	MotorEn_Init(); 
	Motor_EN();

	os_delay(100);

	PWM_TIM2_Init();                //TIM2_PWM
    PWM_TIM4_Init();                //TIM4_PWM
}

#endif

void PWM_TIM4_Init(void)
{
	/* 开启时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);    //开启AFIO复用时钟，重映射使用TIM4定时器
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);   //使能GPIOB外设时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);    //使能TIM4时钟 
	
	/* GPIO初始化结构体 */
	GPIO_InitTypeDef  GPIO_InitStructure;                 
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;   //TIM4 Ch1/2/3/4
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_10MHz;        //速度 10MHz
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;          //复用推挽输出
	GPIO_Init(GPIOB,&GPIO_InitStructure);                  //初始化 GPIOB
	
    /* TIM时间基数初始化结构体 */
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseSrtructure;     
	TIM_TimeBaseSrtructure.TIM_Period= TIM4_Reload_Num_ARR;           //计数器TIMx_CNT计数，从0累加到ARR次后溢出，设置自动重装载值
	TIM_TimeBaseSrtructure.TIM_Prescaler=TIM4_Frequency_Divide_PSC;   //设置预分频系数 PSC
	TIM_TimeBaseSrtructure.TIM_ClockDivision=0;                       //1分频，用于滤波信号抖动
	TIM_TimeBaseSrtructure.TIM_CounterMode=TIM_CounterMode_Up;        //向上计数模式溢出
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseSrtructure);					  //初始化TIM4的时钟参数
	
    /* TIM输出比较功能初始化结构体 */
	TIM_OCInitTypeDef	TIM_OCInitStructure;             
	TIM_OCInitStructure.TIM_OCMode=TIM_OCMode_PWM1;     			  //选择定时器模式为比较低输出。
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;     //比较输出使能
	TIM_OCInitStructure.TIM_Pulse =(TIM4_Reload_Num_ARR+1)*0;         //比较输出脉冲宽度，设置占空比（初始化用）,即CCR
	TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_High;           //输出极性：TIM输出比较极性高
	
	
	TIM_OC1Init(TIM4,&TIM_OCInitStructure);          				  //初始化TIM4  CH1的时钟，下同	
	TIM_OC2Init(TIM4,&TIM_OCInitStructure);
	TIM_OC3Init(TIM4,&TIM_OCInitStructure);
	TIM_OC4Init(TIM4,&TIM_OCInitStructure);
	
	/* 初始化CH1/2/3/4 */
	TIM_OC1PreloadConfig(TIM4,TIM_OCPreload_Enable);                  //CH1 预装载使能，功能在ARR和CCR1改变时，等他们计数完一个周期再修改
	TIM_OC2PreloadConfig(TIM4,TIM_OCPreload_Enable);                  //CH2 预装载使能
	TIM_OC3PreloadConfig(TIM4,TIM_OCPreload_Enable);                  //CH3 预装载使能
	TIM_OC4PreloadConfig(TIM4,TIM_OCPreload_Enable);                  //CH4 预装载使能
	TIM_ARRPreloadConfig(TIM4,ENABLE);                                //使能 TIMx在ARR上的预装载寄存器
	
	TIM_Cmd(TIM4, ENABLE);                                            //使能 TIM4


}


//https://www.iotword.com/7734.html
void PWM_TIM1_Init(void) 
{
	GPIO_InitTypeDef		GPIOInitStruct;	

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	/*??GPIOA,GPIOB??*/
	
	GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	
//	GPIOInitStruct.GPIO_Pin = GPIO_Pin_8;											/*PA8:CH1*/
//	GPIO_Init(GPIOA, &GPIOInitStruct);
	
	GPIOInitStruct.GPIO_Pin = GPIO_Pin_13;											/*PB13:CH1N*/
	GPIO_Init(GPIOB, &GPIOInitStruct);
	
//	GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;				
//	GPIOInitStruct.GPIO_Pin = GPIO_Pin_12;											/*PB12:BKIN*/
//	GPIO_Init(GPIOB, &GPIOInitStruct);

//	GPIO_SetBits(GPIOB, GPIO_Pin_12);
	
	
    TIM_TimeBaseInitTypeDef		TIMTimeBaseStruct;
	TIM_OCInitTypeDef			TIMOCInitStruct;
	TIM_BDTRInitTypeDef			TIMBDTRInitStruct;
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE); 							/*??TIM1??*/
 
	/*???????*/
	TIMTimeBaseStruct.TIM_Period = 200 - 1;										/*?0?? ????????1000?*/ 
	/*?????:72MHz / 7200 = 10kHz;??:(1 / 10kHz) * 1000 = 100us*/
	TIMTimeBaseStruct.TIM_Prescaler = 7200 - 1;										/*?????10kHz,??????? 100us*/
	/*??????????:1000 * 100us = 100ms*/
	TIMTimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1; 							/*?????? = 1,tDTS=tCKINT*/
	TIMTimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up; 						/*????*/
	TIMTimeBaseStruct.TIM_RepetitionCounter = 0; 									/*???????*/
	TIM_TimeBaseInit(TIM1, &TIMTimeBaseStruct); 
 
	/*?????????*/
	TIMOCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;									/*PWM1??*/
	TIMOCInitStruct.TIM_OutputState = TIM_OutputState_Enable;						/*????*/
	TIMOCInitStruct.TIM_OutputNState = TIM_OutputNState_Enable;						/*??????*/
	TIMOCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;							/*??????????*/
	TIMOCInitStruct.TIM_OCNPolarity = TIM_OCNPolarity_High;							/*????????????*/
	TIMOCInitStruct.TIM_OCIdleState = TIM_OCIdleState_Set;							/*?????????*/
	TIMOCInitStruct.TIM_OCNIdleState = TIM_OCNIdleState_Reset;						/*???????????*/
 
	/*???TIM1???1*/
	TIMOCInitStruct.TIM_Pulse = 175 -1;//50 - 1; 75% 											/*??? = 250 / 1000 = 25%*/
	TIM_OC1Init(TIM1,&TIMOCInitStruct);
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);								/*?????,??????????????*/		
 
	/*??????????*/
	TIMBDTRInitStruct.TIM_OSSRState = TIM_OSSRState_Enable;							/*?????�????�?? = 1*/
	TIMBDTRInitStruct.TIM_OSSIState = TIM_OSSIState_Enable;							/*?????�????�?? = 1*/
	TIMBDTRInitStruct.TIM_LOCKLevel = TIM_LOCKLevel_1;								/*????1,?????*/
	TIMBDTRInitStruct.TIM_DeadTime = 0x80;											/*????:12.8ms*/
	TIMBDTRInitStruct.TIM_Break = TIM_Break_Enable;									/*??????*/
	TIMBDTRInitStruct.TIM_BreakPolarity = TIM_BreakPolarity_Low;					/*?????????,??????????????PWM???,????????*/
	TIMBDTRInitStruct.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;				/*??????*/
	TIM_BDTRConfig(TIM1, &TIMBDTRInitStruct);
 
	TIM_Cmd(TIM1, ENABLE);  														/*?????,???????*/
	
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
	
	//TIM_SetCompare1(TIM1, 189);
	TIM_SetCompare1(TIM1, 185);
}
//TIM_SetCompare（x）()函数可以改变占空比

/***************************************************
TIM2通道1/2控制前左轮电机正反转，CCR1>CCR2时正转
TIM2通道3/4控制前右轮电机正反转，CCR3>CCR4时正转
TIM4通道1/2控制后左轮电机正反转，CCR1>CCR2时正转
TIM4通道3/4控制后右轮电机正反转，CCR3>CCR4时正转
***************************************************/

void car_forward()
{
	TIM_Cmd(TIM2, ENABLE);
	GPIO_SetBits(GPIOA, GPIO_Pin_15);                     //PA15_MotorAB_EN
	TIM_SetCompare1(TIM2,1000);        //设置TIM2  CH1占空比输出  PA0
	TIM_SetCompare2(TIM2,0);          //设置TIM2  CH2占空比输出  PA1
	TIM_SetCompare3(TIM2,0);       //设置TIM2  CH1占空比输出  PA2
	TIM_SetCompare4(TIM2,1000);          //设置TIM2  CH2占空比输出  PA3

//	TIM_SetCompare1(TIM4,0);          //设置TIM4  CH1占空比输出  PB6
//	TIM_SetCompare2(TIM4,1500);       //设置TIM4  CH2占空比输出  PB7
//	TIM_SetCompare3(TIM4,0);          //设置TIM4  CH3占空比输出  PB8
//	TIM_SetCompare4(TIM4,2000);       //设置TIM4  CH4占空比输出  PB9

}

void car_backward()
{
	TIM_Cmd(TIM2, ENABLE);
	GPIO_SetBits(GPIOA, GPIO_Pin_15);  //PA15_MotorAB_EN
	TIM_SetCompare1(TIM2, 0);            //设置TIM2  CH1占空比输出  PA0
	TIM_SetCompare2(TIM2,1000);          //设置TIM2  CH2占空比输出  PA1
	TIM_SetCompare3(TIM2, 1000);            //设置TIM2  CH1占空比输出  PA2
	TIM_SetCompare4(TIM2,0);          //设置TIM2  CH2占空比输出  PA3
}

void car_stop()
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_15);
	
	TIM_Cmd(TIM2, DISABLE);
}

void car_turn_left()
{
	TIM_Cmd(TIM1, ENABLE); 
	
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
	
	//TIM_SetCompare1(TIM1, 189);
	TIM_SetCompare1(TIM1, 189);
}

void car_turn_right()
{
	TIM_Cmd(TIM1, ENABLE); 
	
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
	
	//TIM_SetCompare1(TIM1, 189);
	TIM_SetCompare1(TIM1, 180);
}

void car_turn_reset()
{
	TIM_Cmd(TIM1, ENABLE); 
	
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
	
	//TIM_SetCompare1(TIM1, 189);
	TIM_SetCompare1(TIM1, 185);
}
	
