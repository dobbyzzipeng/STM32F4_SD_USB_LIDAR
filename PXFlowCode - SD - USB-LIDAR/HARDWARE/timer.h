#ifndef __TIMER_H__
#define __TIMER_H__
#include "stm32f4xx.h"

//timer
void TIM2_Configuration(void);
void TIM6_Configuration(void);
void TIM6_Start(void);
void TIM8_Configuration(void);
uint32_t Get_Time_Micros(void);

//pwm output
//void TIM1_PWM_Init(u16 arr,u16 psc);
void TIM3_PWM_Init(u16 arr,u16 psc);
void TIM4_PWM_Init(u16 arr,u16 psc);
//
#define PWM1  TIM3->CCR2
#define PWM2  TIM3->CCR3
#define PWM3  TIM3->CCR4
#define PWM4  TIM4->CCR4
#define PWM5  TIM4->CCR3
#define PWM6  TIM1->CCR4
#define PWM7  TIM4->CCR2
#define PWM8  TIM4->CCR1

#endif
