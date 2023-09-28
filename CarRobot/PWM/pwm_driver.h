#ifndef _PWM_DRIVER_H_
#define _PWM_DRIVER_H_

# include "stm32f10x.h"

#define TIM2_Reload_Num_ARR  1999             //自动重装载寄存器值
#define TIM2_Frequency_Divide_PSC   35    	  //TIM时钟预分频值

#define TIM4_Reload_Num_ARR  1999             //自动重装载寄存器值
#define TIM4_Frequency_Divide_PSC   35        //TIM时钟预分频值


void JTAG_Init(void);
void MotorEn_Init(void);
void Motor_EN(void);	
void PWM_TIM2_Init(void);                       //TIM2_PWM输出初始化
void PWM_TIM4_Init(void);                       //TIM4_PWM输出初始化

void pwm_init();
void car_forward();
void car_backward();
void car_stop();

void car_turn_left();
void car_turn_right();

void PWM_TIM1_Init();

#endif
