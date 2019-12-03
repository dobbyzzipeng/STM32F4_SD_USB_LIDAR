#ifndef _SOLFT_SPI_H_
#define _SOLFT_SPI_H_
#include "sys.h"

#define SOLFT_MOSI_L()  (GPIO_ResetBits(GPIOD,GPIO_Pin_5))
#define SOLFT_MISO_L()	(GPIO_ResetBits(GPIOE,GPIO_Pin_0))
#define SOLFT_SCK_L()   (GPIO_ResetBits(GPIOB,GPIO_Pin_5))
#define SOLFT_CS_L()    (GPIO_ResetBits(GPIOD,GPIO_Pin_4))

#define SOLFT_MOSI_H()  (GPIO_SetBits(GPIOD,GPIO_Pin_5))
#define SOLFT_MISO_H()	(GPIO_SetBits(GPIOE,GPIO_Pin_0))
#define SOLFT_SCK_H()   (GPIO_SetBits(GPIOB,GPIO_Pin_5))
#define SOLFT_CS_H()    (GPIO_SetBits(GPIOD,GPIO_Pin_4))

//#define SOLFT_MISO_IN()	 {GPIOE->MODER&=~(3<<(0*2));GPIOE->MODER|=0<<(0*2);}
//#define SOLFT_MISO_OUT() {GPIOE->MODER&=~(3<<(0*2));GPIOE->MODER|=1<<(0*2);}

#define SOLFT_MISO_READ()	(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_0))

void solft_spi_init(void);
unsigned char solft_spi_send_byte(unsigned char data);
unsigned char solft_spi_readwrite_byte(unsigned char data);
#endif
