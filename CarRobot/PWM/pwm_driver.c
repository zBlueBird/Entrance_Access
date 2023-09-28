
#include "pwm_driver.h"
#include "stm32f10x_tim.h"
#include "FreeRTOS.h"
#include "task.h"

/*  �ر�JTAG����ģʽ����ʹ��PA15���� */
void JTAG_Init(void)

{	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);    //�ȿ�������AFIO����ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);   //ʹ��GPIOA����ʱ��
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);//�ر�JTAG������SWD��ѡ��GPIO_Remap_SWJ_Disable����ʧȥ��������ģʽ�����ã���
}

/*  ����PC0��PA15��GPIO����ʼ��DRV8848оƬ��ʹ������ */
void MotorEn_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);  	//ʹ��C��GPIOʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);  	//ʹ��A��GPIOʱ��

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;     		//ѡ���������
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_0;					//ָ������PC0,MotEn2
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_10MHz;			//�����������10MHz
	GPIO_Init(GPIOC,&GPIO_InitStruct);						//��ʼ������GPIOC�Ĵ���
	GPIO_ResetBits(GPIOC, GPIO_Pin_0);
	

	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;				//ѡ���������
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_15;					//ָ������PA15,MotEn1
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_10MHz;			//�����������10MHz
	GPIO_Init(GPIOA,&GPIO_InitStruct);						//��ʼ������GPIOA�Ĵ���
	GPIO_ResetBits(GPIOA, GPIO_Pin_15);	
}

/*  ʹ��DRV8848оƬ��nSLEEP���� */
void Motor_EN(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_15);                     //PA15_MotorAB_EN
	GPIO_SetBits(GPIOC, GPIO_Pin_0);                      //PC0_MotorAB_EN
}

/*    ����TIM2��ʱ�����PWM       */
void PWM_TIM2_Init(void)
{
	
	/* ����ʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);    //����AFIO����ʱ�ӣ���ӳ��ʹ��TIM2��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);   //ʹ��GPIOA����ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);    //ʹ��TIM2ʱ�� 
	
	/* GPIO��ʼ���ṹ�� */
	GPIO_InitTypeDef  GPIO_InitStructure;                 
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;   //TIM2 Ch1/2/3/4
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_10MHz;        //�ٶ� 10MHz
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;          //�����������
	GPIO_Init(GPIOA,&GPIO_InitStructure);                  //��ʼ�� GPIOA
	
    /* TIMʱ�������ʼ���ṹ�� */
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseSrtructure;     
	TIM_TimeBaseSrtructure.TIM_Period= TIM2_Reload_Num_ARR;           //������TIMx_CNT��������0�ۼӵ�ARR�κ�����������Զ���װ��ֵ
	TIM_TimeBaseSrtructure.TIM_Prescaler=TIM2_Frequency_Divide_PSC;   //����Ԥ��Ƶϵ�� PSC
	TIM_TimeBaseSrtructure.TIM_ClockDivision=0;                       //1��Ƶ�������˲��źŶ���
	TIM_TimeBaseSrtructure.TIM_CounterMode=TIM_CounterMode_Up;        //���ϼ���ģʽ���
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseSrtructure);					  //��ʼ��TIM2��ʱ�Ӳ���
	
    /* TIM����ȽϹ��ܳ�ʼ���ṹ�� */
	TIM_OCInitTypeDef	TIM_OCInitStructure;             
	TIM_OCInitStructure.TIM_OCMode=TIM_OCMode_PWM1;     			  //ѡ��ʱ��ģʽΪ�Ƚϵ������
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;     //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_Pulse =(TIM2_Reload_Num_ARR+1)*0;         //�Ƚ���������ȣ�����ռ�ձȣ���ʼ���ã�,��CCR
	TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_High;           //������ԣ�TIM����Ƚϼ��Ը�
	
	
	TIM_OC1Init(TIM2,&TIM_OCInitStructure);          				  //��ʼ��TIM2  CH1��ʱ�ӣ���ͬ	
	TIM_OC2Init(TIM2,&TIM_OCInitStructure);
	TIM_OC3Init(TIM2,&TIM_OCInitStructure);
	TIM_OC4Init(TIM2,&TIM_OCInitStructure);


	
	
	/* ��ʼ��CH1/2/3/4 */
	TIM_OC1PreloadConfig(TIM2,TIM_OCPreload_Enable);                  //CH1 Ԥװ��ʹ�ܣ�������ARR��CCR1�ı�ʱ�������Ǽ�����һ���������޸�
	TIM_OC2PreloadConfig(TIM2,TIM_OCPreload_Enable);                  //CH2 Ԥװ��ʹ��
	TIM_OC3PreloadConfig(TIM2,TIM_OCPreload_Enable);                  //CH3 Ԥװ��ʹ��
	TIM_OC4PreloadConfig(TIM2,TIM_OCPreload_Enable);                  //CH4 Ԥװ��ʹ��
	TIM_ARRPreloadConfig(TIM2,ENABLE);                                //ʹ�� TIMx��ARR�ϵ�Ԥװ�ؼĴ���
	
	TIM_Cmd(TIM2, ENABLE);                                            //ʹ�� TIM2

	
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
	/* ����ʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);    //����AFIO����ʱ�ӣ���ӳ��ʹ��TIM4��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);   //ʹ��GPIOB����ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);    //ʹ��TIM4ʱ�� 
	
	/* GPIO��ʼ���ṹ�� */
	GPIO_InitTypeDef  GPIO_InitStructure;                 
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;   //TIM4 Ch1/2/3/4
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_10MHz;        //�ٶ� 10MHz
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;          //�����������
	GPIO_Init(GPIOB,&GPIO_InitStructure);                  //��ʼ�� GPIOB
	
    /* TIMʱ�������ʼ���ṹ�� */
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseSrtructure;     
	TIM_TimeBaseSrtructure.TIM_Period= TIM4_Reload_Num_ARR;           //������TIMx_CNT��������0�ۼӵ�ARR�κ�����������Զ���װ��ֵ
	TIM_TimeBaseSrtructure.TIM_Prescaler=TIM4_Frequency_Divide_PSC;   //����Ԥ��Ƶϵ�� PSC
	TIM_TimeBaseSrtructure.TIM_ClockDivision=0;                       //1��Ƶ�������˲��źŶ���
	TIM_TimeBaseSrtructure.TIM_CounterMode=TIM_CounterMode_Up;        //���ϼ���ģʽ���
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseSrtructure);					  //��ʼ��TIM4��ʱ�Ӳ���
	
    /* TIM����ȽϹ��ܳ�ʼ���ṹ�� */
	TIM_OCInitTypeDef	TIM_OCInitStructure;             
	TIM_OCInitStructure.TIM_OCMode=TIM_OCMode_PWM1;     			  //ѡ��ʱ��ģʽΪ�Ƚϵ������
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;     //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_Pulse =(TIM4_Reload_Num_ARR+1)*0;         //�Ƚ���������ȣ�����ռ�ձȣ���ʼ���ã�,��CCR
	TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_High;           //������ԣ�TIM����Ƚϼ��Ը�
	
	
	TIM_OC1Init(TIM4,&TIM_OCInitStructure);          				  //��ʼ��TIM4  CH1��ʱ�ӣ���ͬ	
	TIM_OC2Init(TIM4,&TIM_OCInitStructure);
	TIM_OC3Init(TIM4,&TIM_OCInitStructure);
	TIM_OC4Init(TIM4,&TIM_OCInitStructure);
	
	/* ��ʼ��CH1/2/3/4 */
	TIM_OC1PreloadConfig(TIM4,TIM_OCPreload_Enable);                  //CH1 Ԥװ��ʹ�ܣ�������ARR��CCR1�ı�ʱ�������Ǽ�����һ���������޸�
	TIM_OC2PreloadConfig(TIM4,TIM_OCPreload_Enable);                  //CH2 Ԥװ��ʹ��
	TIM_OC3PreloadConfig(TIM4,TIM_OCPreload_Enable);                  //CH3 Ԥװ��ʹ��
	TIM_OC4PreloadConfig(TIM4,TIM_OCPreload_Enable);                  //CH4 Ԥװ��ʹ��
	TIM_ARRPreloadConfig(TIM4,ENABLE);                                //ʹ�� TIMx��ARR�ϵ�Ԥװ�ؼĴ���
	
	TIM_Cmd(TIM4, ENABLE);                                            //ʹ�� TIM4


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
//TIM_SetCompare��x��()�������Ըı�ռ�ձ�

/***************************************************
TIM2ͨ��1/2����ǰ���ֵ������ת��CCR1>CCR2ʱ��ת
TIM2ͨ��3/4����ǰ���ֵ������ת��CCR3>CCR4ʱ��ת
TIM4ͨ��1/2���ƺ����ֵ������ת��CCR1>CCR2ʱ��ת
TIM4ͨ��3/4���ƺ����ֵ������ת��CCR3>CCR4ʱ��ת
***************************************************/

void car_forward()
{
	TIM_Cmd(TIM2, ENABLE);
	GPIO_SetBits(GPIOA, GPIO_Pin_15);                     //PA15_MotorAB_EN
	TIM_SetCompare1(TIM2,1000);        //����TIM2  CH1ռ�ձ����  PA0
	TIM_SetCompare2(TIM2,0);          //����TIM2  CH2ռ�ձ����  PA1
	TIM_SetCompare3(TIM2,0);       //����TIM2  CH1ռ�ձ����  PA2
	TIM_SetCompare4(TIM2,1000);          //����TIM2  CH2ռ�ձ����  PA3

//	TIM_SetCompare1(TIM4,0);          //����TIM4  CH1ռ�ձ����  PB6
//	TIM_SetCompare2(TIM4,1500);       //����TIM4  CH2ռ�ձ����  PB7
//	TIM_SetCompare3(TIM4,0);          //����TIM4  CH3ռ�ձ����  PB8
//	TIM_SetCompare4(TIM4,2000);       //����TIM4  CH4ռ�ձ����  PB9

}

void car_backward()
{
	TIM_Cmd(TIM2, ENABLE);
	GPIO_SetBits(GPIOA, GPIO_Pin_15);  //PA15_MotorAB_EN
	TIM_SetCompare1(TIM2, 0);            //����TIM2  CH1ռ�ձ����  PA0
	TIM_SetCompare2(TIM2,1000);          //����TIM2  CH2ռ�ձ����  PA1
	TIM_SetCompare3(TIM2, 1000);            //����TIM2  CH1ռ�ձ����  PA2
	TIM_SetCompare4(TIM2,0);          //����TIM2  CH2ռ�ձ����  PA3
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
	
