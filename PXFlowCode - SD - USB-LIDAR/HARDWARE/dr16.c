#include "dr16.h"

/* ------ ----------------- Internal Data ----------------------------------- */
volatile unsigned char sbus_rx_buffer[25];
uint16_t Channel_0,Channel_1,Channel_2,Channel_3,Switch_left,Switch_right;
int Mouse_X,Mouse_Y,Mouse_Z,Mouse_Left,Mouse_Right,KeyBoard_value;

/* ----------------------- Function Implements ---------------------------- */
/******************************************************************************
* @fn RC_Init
*
* @brief configure stm32 usart2 port
* - USART Parameters
* - 100Kbps
* - 8-N-1
* - DMA Mode
*
* @return None.
*
* @note This code is fully tested on STM32F405RGT6 Platform, You can port it
* to the other platform.
*/
void RC_Init(void)
{
	/* -------------- Enable Module Clock Source ----------------------------*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_DMA1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3, GPIO_AF_USART2);
	/* -------------- Configure GPIO ---------------------------------------*/
	{
		GPIO_InitTypeDef gpio;
		USART_InitTypeDef usart2;
		gpio.GPIO_Pin = GPIO_Pin_3 ;
		gpio.GPIO_Mode = GPIO_Mode_AF;
		gpio.GPIO_OType = GPIO_OType_PP;
		gpio.GPIO_Speed = GPIO_Speed_100MHz;
		gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOA, &gpio);
		USART_DeInit(USART2);
		usart2.USART_BaudRate = 100000;
		usart2.USART_WordLength = USART_WordLength_8b;
		usart2.USART_StopBits = USART_StopBits_1;
		usart2.USART_Parity = USART_Parity_Even;
		usart2.USART_Mode = USART_Mode_Rx;
		usart2.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_Init(USART2,&usart2);
		USART_Cmd(USART2,ENABLE);
		USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE);
	}
	/* -------------- Configure NVIC ---------------------------------------*/
	{
		NVIC_InitTypeDef nvic;
		nvic.NVIC_IRQChannel = DMA1_Stream5_IRQn;
		nvic.NVIC_IRQChannelPreemptionPriority = 0;
		nvic.NVIC_IRQChannelSubPriority = 1;
		nvic.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&nvic);
	}
	/* -------------- Configure DMA -----------------------------------------*/
	{
		DMA_InitTypeDef dma;
		DMA_DeInit(DMA1_Stream5);
		dma.DMA_Channel = DMA_Channel_4;
		dma.DMA_PeripheralBaseAddr = (uint32_t)&(USART2->DR);
		dma.DMA_Memory0BaseAddr = (uint32_t)sbus_rx_buffer;
		dma.DMA_DIR = DMA_DIR_PeripheralToMemory;
		dma.DMA_BufferSize = 18;
		dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
		dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		dma.DMA_Mode = DMA_Mode_Circular;
		dma.DMA_Priority = DMA_Priority_VeryHigh;
		dma.DMA_FIFOMode = DMA_FIFOMode_Disable;
		dma.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
		dma.DMA_MemoryBurst = DMA_Mode_Normal;
		dma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
		DMA_Init(DMA1_Stream5,&dma);
		DMA_ITConfig(DMA1_Stream5,DMA_IT_TC,ENABLE);
		DMA_Cmd(DMA1_Stream5,ENABLE);
	}
}
/******************************************************************************
* @fn DMA1_Stream5_IRQHandler
*
* @brief USART2 DMA ISR
*
* @return None.
*
* @note This code is fully tested on STM32F405RGT6 Platform, You can port it
* to the other platform.
*/
void DMA1_Stream5_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_Stream5, DMA_IT_TCIF5))
	{
		DMA_ClearFlag(DMA1_Stream5, DMA_FLAG_TCIF5);
		DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);
		Channel_0 = (sbus_rx_buffer[0]| (sbus_rx_buffer[1] << 8)) & 0x07ff; //!< Channel 0
 		Channel_1 = (uint16_t)((sbus_rx_buffer[1] >> 3) | (sbus_rx_buffer[2] << 5)) & 0x07ff; //!< Channel 1
 		Channel_2 = (uint16_t)((sbus_rx_buffer[2] >> 6) | (sbus_rx_buffer[3] << 2) |(sbus_rx_buffer[4] << 10)) & 0x07ff;//!< Channel 2
 		Channel_3 = (uint16_t)((sbus_rx_buffer[4] >> 1) | (sbus_rx_buffer[5] << 7)) & 0x07ff; //!< Channel 3
		Switch_left = (uint16_t)((sbus_rx_buffer[5] >> 4)& 0x000C) >> 2; //!< Switch left
		Switch_right = (uint16_t)((sbus_rx_buffer[5] >> 4)& 0x0003); //!< Switch right
		
		Mouse_X = sbus_rx_buffer[6] | (sbus_rx_buffer[7] << 8); //!< Mouse X axis
		Mouse_Y = sbus_rx_buffer[8] | (sbus_rx_buffer[9] << 8); //!< Mouse Y axis
		Mouse_Z = sbus_rx_buffer[10] | (sbus_rx_buffer[11] << 8); //!< Mouse Z axis
		Mouse_Left = sbus_rx_buffer[12]; //!< Mouse Left Is Press ?
		Mouse_Right=sbus_rx_buffer[13]; //!< Mouse Right Is Press ?
		KeyBoard_value = sbus_rx_buffer[14] | (sbus_rx_buffer[15] << 8); //!< KeyBoard value
	
		//printf("Channel_0:%dChannel_1:%dChannel_2:%dChannel_3:%d\r\n",Channel_0,Channel_1,Channel_2,Channel_3);
		//printf("Switch_left:%dSwitch_right:%d\r\n",Switch_left,Switch_right);
		//printf("Mouse_X:%dMouse_Y:%dMouse_Z:%d\r\n",Mouse_X,Mouse_Y,Mouse_Z);
	}
}


