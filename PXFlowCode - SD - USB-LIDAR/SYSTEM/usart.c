#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
//#include "laser_data_parser.h"
//#include "data_process.h"
#include "usart.h"
#include "led.h"
#include "pwm.h"
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
}
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (u8) ch;      
	return ch;
}
#endif

void usart2_init(unsigned long int baudrate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef  nvic;
	USART_ClockInitTypeDef USART_ClockInitStruct;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2 );
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2 );

	GPIO_StructInit(&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	USART_StructInit(&USART_InitStructure);
	USART_InitStructure.USART_BaudRate=baudrate;
	USART_InitStructure.USART_WordLength=USART_WordLength_8b;
	USART_InitStructure.USART_StopBits=USART_StopBits_1;
	USART_InitStructure.USART_Parity=USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
	USART_Init(USART2  ,&USART_InitStructure);
	
	USART_ClockStructInit(&USART_ClockInitStruct);
	USART_ClockInit(USART2 ,&USART_ClockInitStruct);
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
	USART_Cmd(USART2,ENABLE);
		
	nvic.NVIC_IRQChannel = USART2_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);
}


uint8_t USART2_RX_BUF[USART2_MAX_RECV_LEN] = {0};
uint8_t USART2_RX_date[USART2_MAX_RECV_LEN] = {0};
uint16_t usart2_rx_num = 0;
uint8_t usart2_rx_data_flag = 0;
void USART2_IRQHandler(void)      //串口2中断服务程序
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) 
	{
		USART2_RX_date[usart2_rx_num] =USART_ReceiveData(USART2);	//读取接收到的数据
		usart2_rx_num++;
		if(usart2_rx_num > 2960)
		{
			for(int i=0;i<2961;i++)
			{
			   USART2_RX_BUF[i]=USART2_RX_date[i];
			}
			BAT_LED2_TOGGLE();
			usart2_rx_data_flag = 1;
			usart2_rx_num=0;
		}
	}
}

#define USE_USART3_TX_DMA 1
#define USE_USART3_RX_DMA 0
//串口发送缓存区 	
__align(8) uint8_t USART3_TX_BUF[512] = {0};
uint8_t USART3_RX_BUF[USART3_MAX_RECV_LEN] = {0};
uint16_t USART3_RX_STA = 0;
void usart3_init(unsigned long int baudrate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef  nvic;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	#if USE_USART3_TX_DMA
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	#endif
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_USART3);

	GPIO_StructInit(&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);

	USART_StructInit(&USART_InitStructure);
	USART_InitStructure.USART_BaudRate=baudrate;
	USART_InitStructure.USART_WordLength=USART_WordLength_8b;
	USART_InitStructure.USART_StopBits=USART_StopBits_1;
	USART_InitStructure.USART_Parity=USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
	USART_Init(USART3,&USART_InitStructure);
	#if !USE_USART3_RX_DMA
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);
		
	nvic.NVIC_IRQChannel = USART3_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 1;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);
	#endif
	USART_Cmd(USART3,ENABLE);
/* -------------- Configure NVIC ---------------------------------------*/
		#if USE_USART3_RX_DMA
		nvic.NVIC_IRQChannel = DMA1_Stream1_IRQn;//usart3 rx
		nvic.NVIC_IRQChannelPreemptionPriority = 2;
		nvic.NVIC_IRQChannelSubPriority = 2;
		nvic.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&nvic);
/* -------------- Configure DMA_RX -----------------------------------------*/
	{
		DMA_InitTypeDef DMA_InitStructure;
		DMA_DeInit(DMA1_Stream1);
		DMA_InitStructure.DMA_Channel = DMA_Channel_4;
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART3->DR);
		DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)USART3_RX_BUF;
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
		DMA_InitStructure.DMA_BufferSize = sizeof(USART3_RX_BUF);
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
		DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
		DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
		DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
		DMA_InitStructure.DMA_MemoryBurst = DMA_Mode_Normal;
		DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
		DMA_Init(DMA1_Stream1,&DMA_InitStructure);
		DMA_ITConfig(DMA1_Stream1,DMA_IT_TC,ENABLE);//DMA传输完成中断
		USART_DMACmd(USART3,USART_DMAReq_Rx,ENABLE);
		DMA_Cmd(DMA1_Stream1,ENABLE);
//	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);//空闲中断,用于不定长接收
	}
		#endif
		
		#if USE_USART3_TX_DMA
		nvic.NVIC_IRQChannel = DMA1_Stream3_IRQn;//usart3 tx
		nvic.NVIC_IRQChannelPreemptionPriority = 1;
		nvic.NVIC_IRQChannelSubPriority = 2;
		nvic.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&nvic);
/* -------------- Configure DMA_TX -----------------------------------------*/
	{
		DMA_InitTypeDef DMA_InitStructure;
		DMA_DeInit(DMA1_Stream3);
		DMA_InitStructure.DMA_Channel = DMA_Channel_4;
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART3->DR);
		DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)USART3_TX_BUF;
		DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
		DMA_InitStructure.DMA_BufferSize = 10;
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
		DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
		DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
		DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
		DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
		DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
		DMA_Init(DMA1_Stream3,&DMA_InitStructure);
		USART_DMACmd(USART3,USART_DMAReq_Tx,ENABLE);
		DMA_ITConfig(DMA1_Stream3,DMA_IT_TC,ENABLE);//for rx dma run successfully,tx dma interrupt must be open
		DMA_Cmd(DMA1_Stream3,DISABLE);//初始化时要失能。。。不能！！！
	}
	#endif
	
	TIM7_Init(1000-1,8400-1);		//100ms中断
	USART3_RX_STA=0;		//清零
	TIM_Cmd(TIM7, DISABLE); //关闭定时器7
}


void USART3_IRQHandler(void)
{
	uint8_t Res = 0;
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		Res =USART_ReceiveData(USART3);
		if((USART3_RX_STA&(1<<15))==0)//接收完的一批数据,还没有被处理,则不再接收其他数据
		{ 
			if(USART3_RX_STA<USART3_MAX_RECV_LEN)	//还可以接收数据
			{
				TIM7->CNT=0;         				//计数器清空	
				if(USART3_RX_STA==0) 				//使能定时器7的中断 
				{
					TIM7->CR1|=1<<0;     			//使能定时器7
				}
				USART3_RX_BUF[USART3_RX_STA++]=Res;	//记录接收到的值	 
			}
			else 
			{
				USART3_RX_STA|=1<<15;				//强制标记接收完成
			} 
		}
	}
}
/**
 * [USART3_DMA_Send 通过DMA向串口发送不定长度的数据，看函数名就知道]
 * @param pbuffer [数据的传递指针]
 * @param size    [数据的长度]
 *  如： USART3_DMA_Send((uint8_t*)data_to_send,_cnt);
 */
void USART3_DMA_Send(uint8_t *pbuffer, uint32_t size)
{
	DMA_Cmd (DMA1_Stream3,DISABLE);
	while (DMA_GetCmdStatus(DMA1_Stream3) != DISABLE){}
	DMA_MemoryTargetConfig(DMA1_Stream3,(uint32_t)pbuffer,DMA_Memory_0);
	DMA_SetCurrDataCounter(DMA1_Stream3,size);
	DMA_Cmd (DMA1_Stream3,ENABLE);//使能DMA,开始发送
}

void send_data_dma_u3(uint8_t data[100],uint8_t num)
{
	uint8_t x = 0;
	for(x=0;x<num;x++)
	{
		USART3_TX_BUF[x] = data[x];
	}
	USART3_DMA_Send(USART3_TX_BUF,num);
}

void DMA1_Stream1_IRQHandler(void)//USART3 RX DMA
{
	if(DMA_GetITStatus(DMA1_Stream1, DMA_IT_TCIF1))
	{
		DMA_ClearITPendingBit(DMA1_Stream1, DMA_IT_TCIF1);
	}
}
void DMA1_Stream3_IRQHandler(void)//USART3 TX DMA
{
	if(DMA_GetITStatus(DMA1_Stream3, DMA_IT_TCIF3))
	{
		DMA_ClearITPendingBit(DMA1_Stream3, DMA_IT_TCIF3);
	}
}

