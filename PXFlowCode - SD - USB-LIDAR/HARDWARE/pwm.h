#ifndef __pwm_h__
#define __pwm_h__
//----------------------- Include files ------------------------//
#include "stm32f4xx_rcc.h"

extern uint32_t sys_timer_counter;
extern unsigned char sys_timer_flag;
//----------------------- Extern function ----------------------//
//void TIM3_PWM_Init(uint16_t arr,uint16_t psc);
//----------------------- Extern variable ----------------------//
void TIM5_PWM_Init(uint16_t arr,uint16_t psc);
//void TIM2_Init(uint16_t arr,uint16_t psc);
void TIM7_Init(uint16_t arr,uint16_t psc);
void TIM1_Init(uint16_t arr,uint16_t psc);
//void SERVO_PWM_Init(uint16_t arr,uint16_t psc);

#define SERVOPWM1 TIM4->CCR1
#define SERVOPWM2 TIM2->CCR2
#define SERVOPWM3 TIM3->CCR1
#define SERVOPWM4 TIM3->CCR2

#endif
