#ifndef __LED_H
#define __LED_H
#include "sys.h"
//LED端口定义

#define RED_LED_OFF() (GPIO_SetBits(GPIOC,GPIO_Pin_13))
#define RED_LED_ON()  (GPIO_ResetBits(GPIOC,GPIO_Pin_13))
#define RED_TOGGLE()  (GPIO_ToggleBits(GPIOC, GPIO_Pin_13))

#define BLUE_LED_OFF() (GPIO_SetBits(GPIOE,GPIO_Pin_3))
#define BLUE_LED_ON()  (GPIO_ResetBits(GPIOE,GPIO_Pin_3))
#define BLUE_TOGGLE()  (GPIO_ToggleBits(GPIOE, GPIO_Pin_3))

#define GREEN_LED_OFF() (GPIO_SetBits(GPIOE,GPIO_Pin_2))
#define GREEN_LED_ON()  (GPIO_ResetBits(GPIOE,GPIO_Pin_2))
#define GREEN_TOGGLE()  (GPIO_ToggleBits(GPIOE, GPIO_Pin_2))

#define  MD0_HIGH()	  	GPIO_SetBits(GPIOB,GPIO_Pin_12)
#define	 MD0_LOW()	    GPIO_ResetBits(GPIOB,GPIO_Pin_12)
#define  MD1_HIGH()	  	GPIO_SetBits(GPIOB,GPIO_Pin_13)
#define	 MD1_LOW()	    GPIO_ResetBits(GPIOB,GPIO_Pin_13)

#define  BAT_LED1_OFF()	  	GPIO_SetBits(GPIOC,GPIO_Pin_6)
#define	 BAT_LED1_ON()	    GPIO_ResetBits(GPIOC,GPIO_Pin_6)
#define	 BAT_LED1_TOGGLE()	GPIO_ToggleBits(GPIOC,GPIO_Pin_6)	

#define  BAT_LED2_OFF()	  	GPIO_SetBits(GPIOC,GPIO_Pin_7)
#define	 BAT_LED2_ON()	    GPIO_ResetBits(GPIOC,GPIO_Pin_7)
#define	 BAT_LED2_TOGGLE()	GPIO_ToggleBits(GPIOC,GPIO_Pin_7)	

#define  BAT_LED3_OFF()	  	GPIO_SetBits(GPIOD,GPIO_Pin_9)
#define	 BAT_LED3_ON()	    GPIO_ResetBits(GPIOD,GPIO_Pin_9)
#define	 BAT_LED3_TOGGLE()	GPIO_ToggleBits(GPIOD,GPIO_Pin_9)	

#define  BAT_LED4_OFF()	  	GPIO_SetBits(GPIOD,GPIO_Pin_8)
#define	 BAT_LED4_ON()	    GPIO_ResetBits(GPIOD,GPIO_Pin_8)
#define	 BAT_LED4_TOGGLE()	GPIO_ToggleBits(GPIOD,GPIO_Pin_8)	

#define  SYS_LED_OFF()	  	GPIO_SetBits(GPIOA,GPIO_Pin_15)
#define	 SYS_LED_ON()	    GPIO_ResetBits(GPIOA,GPIO_Pin_15)
#define	 SYS_LED_TOGGLE()	GPIO_ToggleBits(GPIOA,GPIO_Pin_15)	


void BAT_LED_GPIO_Config(void);
void RF_GPIO_Config(void);
void LED_Init(void);//初始化	

#endif
