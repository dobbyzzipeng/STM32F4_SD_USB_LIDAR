#ifndef _OV7670_H_
#define _OV7670_H_
#include "stm32f4xx.h"

#define OV7660ADDR  0X42
//求数组元素个数
#define ARRLEN(arr) (sizeof(arr)/sizeof(arr[0]))
	
#define DCMI_RESET_H()       (GPIO_SetBits(GPIOD,GPIO_Pin_8))
#define DCMI_RESET_L()       (GPIO_ResetBits(GPIOD,GPIO_Pin_8))
#define DCMI_RESET_TOGGLE()  (GPIO_ToggleBits(GPIOD, GPIO_Pin_8))

#define DCMI_PWDN_H()  		 (GPIO_SetBits(GPIOD,GPIO_Pin_9))
#define DCMI_PWDN_L()  		 (GPIO_ResetBits(GPIOD,GPIO_Pin_9))
#define DCMI_PWDN_TOGGLE()   (GPIO_ToggleBits(GPIOD, GPIO_Pin_9))


void OV7670_Init(void);
void OV7670_config_window(uint16_t startx,uint16_t starty,uint16_t width,uint16_t height);
#endif
