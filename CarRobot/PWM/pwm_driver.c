
#include "pwm_driver.h"
#include "stm32f10x_tim.h"
#include "FreeRTOS.h"
#include "task.h"

/*  πÿ±’JTAGµ˜ ‘ƒ£ Ω£¨“‘ π”√PA15“˝Ω≈ */
void JTAG_Init(void)

{	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);    //œ»ø™∆Ùø™∆ÙAFIO∏¥”√ ±÷”
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);   // πƒ‹GPIOAÕ‚…Ë ±÷”
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);//πÿ±’JTAG£¨±£¡ÙSWD£®—°‘ÒGPIO_Remap_SWJ_Disable£¨Ω´ ß»•¡Ω∏ˆµ˜ ‘ƒ£ Ω£¨Œ”√£°£©
}

/*  ≈‰÷√PC0°¢PA15µƒGPIO£¨≥ı ºªØDRV8848–æ∆¨µƒ πƒ‹“˝Ω≈ */
void MotorEn_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);  	// πƒ‹Cø⁄GPIO ±÷”
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);  	// πƒ‹Aø⁄GPIO ±÷”

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;     		//—°‘ÒÕ∆ÕÏ ‰≥ˆ
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_0;					//÷∏∂®“˝Ω≈PC0,MotEn2
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_10MHz;			//…Ë÷√ ‰≥ˆÀŸ¬ 10MHz
	GPIO_Init(GPIOC,&GPIO_InitStruct);						//≥ı ºªØÕ‚…ËGPIOCºƒ¥Ê∆˜
	GPIO_ResetBits(GPIOC, GPIO_Pin_0);
	

	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_Out_PP;				//—°‘ÒÕ∆ÕÏ ‰≥ˆ
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_15;					//÷∏∂®“˝Ω≈PA15,MotEn1
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_10MHz;			//…Ë÷√ ‰≥ˆÀŸ¬ 10MHz
	GPIO_Init(GPIOA,&GPIO_InitStruct);						//≥ı ºªØÕ‚…ËGPIOAºƒ¥Ê∆˜
	GPIO_ResetBits(GPIOA, GPIO_Pin_15);	
}

/*   πƒ‹DRV8848–æ∆¨µƒnSLEEP“˝Ω≈ */
void Motor_EN(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_15);                     //PA15_MotorAB_EN
	GPIO_SetBits(GPIOC, GPIO_Pin_0);                      //PC0_MotorAB_EN
}

/*    ≈‰÷√TIM2∂® ±∆˜ ‰≥ˆPWM       */
void PWM_TIM2_Init(void)
{
	
	/* ø™∆Ù ±÷” */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);    //ø™∆ÙAFIO∏¥”√ ±÷”£¨÷ÿ”≥…‰ π”√TIM2∂® ±∆˜
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);   // πƒ‹GPIOAÕ‚…Ë ±÷”
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);    // πƒ‹TIM2 ±÷” 
	
	/* GPIO≥ı ºªØΩ·ππÃÂ */
	GPIO_InitTypeDef  GPIO_InitStructure;                 
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;   //TIM2 Ch1/2/3/4
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_10MHz;        //ÀŸ∂» 10MHz
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;          //∏¥”√Õ∆ÕÏ ‰≥ˆ
	GPIO_Init(GPIOA,&GPIO_InitStructure);                  //≥ı ºªØ GPIOA
	
    /* TIM ±º‰ª˘ ˝≥ı ºªØΩ·ππÃÂ */
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseSrtructure;     
	TIM_TimeBaseSrtructure.TIM_Period= TIM2_Reload_Num_ARR;           //º∆ ˝∆˜TIMx_CNTº∆ ˝£¨¥”0¿€º”µΩARR¥Œ∫Û“Á≥ˆ£¨…Ë÷√◊‘∂Ø÷ÿ◊∞‘ÿ÷µ
	TIM_TimeBaseSrtructure.TIM_Prescaler=TIM2_Frequency_Divide_PSC;   //…Ë÷√‘§∑÷∆µœµ ˝ PSC
	TIM_TimeBaseSrtructure.TIM_ClockDivision=0;                       //1∑÷∆µ£¨”√”⁄¬À≤®–≈∫≈∂∂∂Ø
	TIM_TimeBaseSrtructure.TIM_CounterMode=TIM_CounterMode_Up;        //œÚ…œº∆ ˝ƒ£ Ω“Á≥ˆ
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseSrtructure);					  //≥ı ºªØTIM2µƒ ±÷”≤Œ ˝
	
    /* TIM ‰≥ˆ±»Ωœπ¶ƒ‹≥ı ºªØΩ·ππÃÂ */
	TIM_OCInitTypeDef	TIM_OCInitStructure;             
	TIM_OCInitStructure.TIM_OCMode=TIM_OCMode_PWM1;     			  //—°‘Ò∂® ±∆˜ƒ£ ΩŒ™±»ΩœµÕ ‰≥ˆ°£
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;     //±»Ωœ ‰≥ˆ πƒ‹
	TIM_OCInitStructure.TIM_Pulse =(TIM2_Reload_Num_ARR+1)*0;         //±»Ωœ ‰≥ˆ¬ˆ≥ÂøÌ∂»£¨…Ë÷√’ºø’±»£®≥ı ºªØ”√£©,º¥CCR
	TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_High;           // ‰≥ˆº´–‘£∫TIM ‰≥ˆ±»Ωœº´–‘∏ﬂ
	
	
	TIM_OC1Init(TIM2,&TIM_OCInitStructure);          				  //≥ı ºªØTIM2  CH1µƒ ±÷”£¨œ¬Õ¨	
	TIM_OC2Init(TIM2,&TIM_OCInitStructure);
	TIM_OC3Init(TIM2,&TIM_OCInitStructure);
	TIM_OC4Init(TIM2,&TIM_OCInitStructure);


	
	
	/* ≥ı ºªØCH1/2/3/4 */
	TIM_OC1PreloadConfig(TIM2,TIM_OCPreload_Enable);                  //CH1 ‘§◊∞‘ÿ πƒ‹£¨π¶ƒ‹‘⁄ARR∫ÕCCR1∏ƒ±‰ ±£¨µ»À˚√«º∆ ˝ÕÍ“ª∏ˆ÷‹∆⁄‘Ÿ–ﬁ∏ƒ
	TIM_OC2PreloadConfig(TIM2,TIM_OCPreload_Enable);                  //CH2 ‘§◊∞‘ÿ πƒ‹
	TIM_OC3PreloadConfig(TIM2,TIM_OCPreload_Enable);                  //CH3 ‘§◊∞‘ÿ πƒ‹
	TIM_OC4PreloadConfig(TIM2,TIM_OCPreload_Enable);                  //CH4 ‘§◊∞‘ÿ πƒ‹
	TIM_ARRPreloadConfig(TIM2,ENABLE);                                // πƒ‹ TIMx‘⁄ARR…œµƒ‘§◊∞‘ÿºƒ¥Ê∆˜
	
	TIM_Cmd(TIM2, ENABLE);                                            // πƒ‹ TIM2

	
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
	/* ø™∆Ù ±÷” */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);    //ø™∆ÙAFIO∏¥”√ ±÷”£¨÷ÿ”≥…‰ π”√TIM4∂® ±∆˜
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);   // πƒ‹GPIOBÕ‚…Ë ±÷”
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);    // πƒ‹TIM4 ±÷” 
	
	/* GPIO≥ı ºªØΩ·ππÃÂ */
	GPIO_InitTypeDef  GPIO_InitStructure;                 
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;   //TIM4 Ch1/2/3/4
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_10MHz;        //ÀŸ∂» 10MHz
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;          //∏¥”√Õ∆ÕÏ ‰≥ˆ
	GPIO_Init(GPIOB,&GPIO_InitStructure);                  //≥ı ºªØ GPIOB
	
    /* TIM ±º‰ª˘ ˝≥ı ºªØΩ·ππÃÂ */
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseSrtructure;     
	TIM_TimeBaseSrtructure.TIM_Period= TIM4_Reload_Num_ARR;           //º∆ ˝∆˜TIMx_CNTº∆ ˝£¨¥”0¿€º”µΩARR¥Œ∫Û“Á≥ˆ£¨…Ë÷√◊‘∂Ø÷ÿ◊∞‘ÿ÷µ
	TIM_TimeBaseSrtructure.TIM_Prescaler=TIM4_Frequency_Divide_PSC;   //…Ë÷√‘§∑÷∆µœµ ˝ PSC
	TIM_TimeBaseSrtructure.TIM_ClockDivision=0;                       //1∑÷∆µ£¨”√”⁄¬À≤®–≈∫≈∂∂∂Ø
	TIM_TimeBaseSrtructure.TIM_CounterMode=TIM_CounterMode_Up;        //œÚ…œº∆ ˝ƒ£ Ω“Á≥ˆ
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseSrtructure);					  //≥ı ºªØTIM4µƒ ±÷”≤Œ ˝
	
    /* TIM ‰≥ˆ±»Ωœπ¶ƒ‹≥ı ºªØΩ·ππÃÂ */
	TIM_OCInitTypeDef	TIM_OCInitStructure;             
	TIM_OCInitStructure.TIM_OCMode=TIM_OCMode_PWM1;     			  //—°‘Ò∂® ±∆˜ƒ£ ΩŒ™±»ΩœµÕ ‰≥ˆ°£
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;     //±»Ωœ ‰≥ˆ πƒ‹
	TIM_OCInitStructure.TIM_Pulse =(TIM4_Reload_Num_ARR+1)*0;         //±»Ωœ ‰≥ˆ¬ˆ≥ÂøÌ∂»£¨…Ë÷√’ºø’±»£®≥ı ºªØ”√£©,º¥CCR
	TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_High;           // ‰≥ˆº´–‘£∫TIM ‰≥ˆ±»Ωœº´–‘∏ﬂ
	
	
	TIM_OC1Init(TIM4,&TIM_OCInitStructure);          				  //≥ı ºªØTIM4  CH1µƒ ±÷”£¨œ¬Õ¨	
	TIM_OC2Init(TIM4,&TIM_OCInitStructure);
	TIM_OC3Init(TIM4,&TIM_OCInitStructure);
	TIM_OC4Init(TIM4,&TIM_OCInitStructure);
	
	/* ≥ı ºªØCH1/2/3/4 */
	TIM_OC1PreloadConfig(TIM4,TIM_OCPreload_Enable);                  //CH1 ‘§◊∞‘ÿ πƒ‹£¨π¶ƒ‹‘⁄ARR∫ÕCCR1∏ƒ±‰ ±£¨µ»À˚√«º∆ ˝ÕÍ“ª∏ˆ÷‹∆⁄‘Ÿ–ﬁ∏ƒ
	TIM_OC2PreloadConfig(TIM4,TIM_OCPreload_Enable);                  //CH2 ‘§◊∞‘ÿ πƒ‹
	TIM_OC3PreloadConfig(TIM4,TIM_OCPreload_Enable);                  //CH3 ‘§◊∞‘ÿ πƒ‹
	TIM_OC4PreloadConfig(TIM4,TIM_OCPreload_Enable);                  //CH4 ‘§◊∞‘ÿ πƒ‹
	TIM_ARRPreloadConfig(TIM4,ENABLE);                                // πƒ‹ TIMx‘⁄ARR…œµƒ‘§◊∞‘ÿºƒ¥Ê∆˜
	
	TIM_Cmd(TIM4, ENABLE);                                            // πƒ‹ TIM4


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
	TIMBDTRInitStruct.TIM_OSSRState = TIM_OSSRState_Enable;							/*?????ì????î?? = 1*/
	TIMBDTRInitStruct.TIM_OSSIState = TIM_OSSIState_Enable;							/*?????ì????î?? = 1*/
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
//TIM_SetCompare£®x£©()∫Ø ˝ø…“‘∏ƒ±‰’ºø’±»

/***************************************************
TIM2Õ®µ¿1/2øÿ÷∆«∞◊Û¬÷µÁª˙’˝∑¥◊™£¨CCR1>CCR2 ±’˝◊™
TIM2Õ®µ¿3/4øÿ÷∆«∞”“¬÷µÁª˙’˝∑¥◊™£¨CCR3>CCR4 ±’˝◊™
TIM4Õ®µ¿1/2øÿ÷∆∫Û◊Û¬÷µÁª˙’˝∑¥◊™£¨CCR1>CCR2 ±’˝◊™
TIM4Õ®µ¿3/4øÿ÷∆∫Û”“¬÷µÁª˙’˝∑¥◊™£¨CCR3>CCR4 ±’˝◊™
***************************************************/

void car_forward()
{
	TIM_Cmd(TIM2, ENABLE);
	GPIO_SetBits(GPIOA, GPIO_Pin_15);                     //PA15_MotorAB_EN
	TIM_SetCompare1(TIM2,1000);        //…Ë÷√TIM2  CH1’ºø’±» ‰≥ˆ  PA0
	TIM_SetCompare2(TIM2,0);          //…Ë÷√TIM2  CH2’ºø’±» ‰≥ˆ  PA1
	TIM_SetCompare3(TIM2,0);       //…Ë÷√TIM2  CH1’ºø’±» ‰≥ˆ  PA2
	TIM_SetCompare4(TIM2,1000);          //…Ë÷√TIM2  CH2’ºø’±» ‰≥ˆ  PA3

//	TIM_SetCompare1(TIM4,0);          //…Ë÷√TIM4  CH1’ºø’±» ‰≥ˆ  PB6
//	TIM_SetCompare2(TIM4,1500);       //…Ë÷√TIM4  CH2’ºø’±» ‰≥ˆ  PB7
//	TIM_SetCompare3(TIM4,0);          //…Ë÷√TIM4  CH3’ºø’±» ‰≥ˆ  PB8
//	TIM_SetCompare4(TIM4,2000);       //…Ë÷√TIM4  CH4’ºø’±» ‰≥ˆ  PB9

}

void car_backward()
{
	TIM_Cmd(TIM2, ENABLE);
	GPIO_SetBits(GPIOA, GPIO_Pin_15);  //PA15_MotorAB_EN
	TIM_SetCompare1(TIM2, 0);            //…Ë÷√TIM2  CH1’ºø’±» ‰≥ˆ  PA0
	TIM_SetCompare2(TIM2,1000);          //…Ë÷√TIM2  CH2’ºø’±» ‰≥ˆ  PA1
	TIM_SetCompare3(TIM2, 1000);            //…Ë÷√TIM2  CH1’ºø’±» ‰≥ˆ  PA2
	TIM_SetCompare4(TIM2,0);          //…Ë÷√TIM2  CH2’ºø’±» ‰≥ˆ  PA3
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
	
