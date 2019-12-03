#include "solft_spi.h"
//#include <intrins.h>
//#include <ctype.h>
//#include <stdlib.h>
//#include <stdio.h>

void solft_spi_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE, ENABLE);//使能GPIOF时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;//SCK PB5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIO
	GPIO_SetBits(GPIOB,GPIO_Pin_5);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;//MOSI PD5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化GPIO
	GPIO_SetBits(GPIOD,GPIO_Pin_5);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//MISO PE0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;//开漏
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIO
	GPIO_SetBits(GPIOE,GPIO_Pin_0);
}

unsigned char solft_spi_send_byte(unsigned char data)
{
	u8 i=0,temp =0;
	for(i=8;i>0;i--)
	{
		if(data&0x80) SOLFT_MOSI_H();
		else SOLFT_MOSI_L();
		data<<=1;
		SOLFT_SCK_H();
//		_nop();
//		_nop();
//		_nop();
		temp<<=1;
		if(SOLFT_MISO_READ()) temp++;
		SOLFT_SCK_L();
//		_nop();
//		_nop();
//		_nop();
	}
	return temp;
}

unsigned char solft_spi_readwrite_byte(unsigned char data)
{//CPOL=1,CPHA=1
	u8 i=0,temp =0;
	for(i=8;i>0;i--)
	{
		SOLFT_SCK_L();
		if(data&0x80) SOLFT_MOSI_H();
		else SOLFT_MOSI_L();
		data<<=1;
		SOLFT_SCK_H();
//		_nop();
		temp<<=1;
		if(SOLFT_MISO_READ()) temp++;
		SOLFT_SCK_L();
//		_nop();
	}
	return temp;
}


