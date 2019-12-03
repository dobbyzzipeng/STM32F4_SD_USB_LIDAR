/*********************************************************************************************************
*
*	ģ������ : ����
*	�ļ����� : bsp_uart.h
*	��    �� : V1.0
*	˵    �� : C�ļ�
*	1)����DMA���������ͣ�DMA�Ĳ���������
*********************************************************************************************************
*/
#include "bsp_usart.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"

#define USE_DMA_RX 0//�Ƿ�ʹ��DMA RX

static uint8_t UsartDmaTxFlag = 0;//1:���ڷ���   0:�������
//__align(8) uint8_t g_RxBuf1[UART1_RX_BUF_SIZE] = {0};		/* ���ջ����� */
__align(8) uint8_t USART1_TX_BUF[UART1_TX_BUF_SIZE] = {0}; 	//���ͻ���,���USART3_MAX_SEND_LEN�ֽ�
__align(8) uint8_t USART1_RX_BUF[UART1_RX_BUF_SIZE] = {0};
static void InitHardUart(void);
static void ConfigUartNVIC(void);
/*
*********************************************************************************************************
*	�� �� ��: bsp_InitUart
*	����˵��: ��ʼ������Ӳ��������ȫ�ֱ�������ֵ.
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitUart(void)
{
	InitHardUart();		/* ���ô��ڵ�Ӳ������(�����ʵ�) */
	#if (USE_DMA_RX ==0)
	ConfigUartNVIC();	/* ���ô����ж� */
	#endif
	UsartDMA_Init();
}

/*
*********************************************************************************************************
*	�� �� ��: InitHardUart
*	����˵��: ���ô��ڵ�Ӳ�������������ʣ�����λ��ֹͣλ����ʼλ��У��λ���ж�ʹ�ܣ��ʺ���STM32-F4������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void InitHardUart(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	/* ��1���� ����GPIO */
	/* TX = PA9   RX = PA10 */
		/* �� GPIO ʱ�� */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

		/* �� UART ʱ�� */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

		/* �� PA9 ӳ��Ϊ USART1_TX */
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);

		/* �� PA10 ӳ��Ϊ USART1_RX */
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

		/* ���� USART Tx Ϊ���ù��� */
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	/* �������Ϊ���� */
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	/* �ڲ���������ʹ�� */
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	/* ����ģʽ */

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		/* ���� USART Rx Ϊ���ù��� */
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* ��2���� ���ô���Ӳ������ */
	USART_InitStructure.USART_BaudRate = UART1_BAUD;	/* ������ */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	#if (USE_DMA_RX ==0) 
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);	/* ʹ�ܽ����ж� */
	#endif
	USART_Cmd(USART1, ENABLE);		/* ʹ�ܴ��� */
}

/*
*********************************************************************************************************
*	�� �� ��: ConfigUartNVIC
*	����˵��: ���ô���Ӳ���ж�.
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void ConfigUartNVIC(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	/* ʹ�ܴ���1�ж� */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = Priority_UART1_Pre;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority 		 = Priority_UART1_Sub;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 0
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
	USART1->DR = (u8) ch;      
	return ch;
}
#endif

void UsartDMA_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	USART_Cmd(USART1,ENABLE);
	#if USE_DMA_RX
	USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);
	#endif
	USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE);
	/* -------------- Configure NVIC ---------------------------------------*/
		NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream7_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	#if USE_DMA_RX
	{
		NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}
	/* -------------- Configure DMA_RX -----------------------------------------*/
	{
		DMA_InitTypeDef DMA_InitStructure;
		DMA_DeInit(DMA2_Stream2);
		DMA_InitStructure.DMA_Channel = DMA_Channel_4;
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);
		DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)USART1_RX_BUF;
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
		DMA_InitStructure.DMA_BufferSize = UART1_RX_BUF_SIZE;
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
		DMA_Init(DMA2_Stream2,&DMA_InitStructure);
		// DMA_ITConfig(DMA2_Stream2,DMA_IT_TC,ENABLE);//���˸÷�������ҪDMA�ж�
		DMA_Cmd(DMA2_Stream2,ENABLE);
	}
	#endif
	/* -------------- Configure DMA_TX -----------------------------------------*/
	{
		DMA_InitTypeDef DMA_InitStructure;
		DMA_DeInit(DMA2_Stream7);
		DMA_InitStructure.DMA_Channel = DMA_Channel_4;
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);
		DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)USART1_TX_BUF;
		DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
//		DMA_InitStructure.DMA_BufferSize = 30;
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
		DMA_Init(DMA2_Stream7,&DMA_InitStructure);
		DMA_ITConfig(DMA2_Stream7,DMA_IT_TC,ENABLE);
		DMA_Cmd(DMA2_Stream7,DISABLE);//��ʼ��ʱҪʧ�ܡ��������ܣ�����
	}
//	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);//�����ж�,���ڲ���������

}

/**
 * [USART1_DMA_Send ͨ��DMA�򴮿ڷ��Ͳ������ȵ����ݣ�����������֪��]
 * @param pbuffer [���ݵĴ���ָ��]
 * @param size    [���ݵĳ���]
 *  �磺 USART1_DMA_Send((uint8_t*)data_to_send,_cnt);
 */
void USART1_DMA_Send(uint8_t *pbuffer, uint32_t size)
{
	if(UsartDmaTxFlag == 0)//����������ɺ���ܿ�ʼ����
	{
		DMA_Cmd (DMA2_Stream7,DISABLE);
		while (DMA_GetCmdStatus(DMA2_Stream7) != DISABLE){}
		DMA_MemoryTargetConfig(DMA2_Stream7,(u32)pbuffer,DMA_Memory_0);
		DMA_SetCurrDataCounter(DMA2_Stream7,size);
	 	DMA_Cmd (DMA2_Stream7,ENABLE);//ʹ��DMA,��ʼ����
		UsartDmaTxFlag=1; //���ݷ�����
	}
}

/**
 * [u1_printf printf ����]
 */
void u1_printf(char* fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	vsprintf((char*)USART1_TX_BUF,fmt,ap);
	va_end(ap);
	USART1_DMA_Send(USART1_TX_BUF, strlen((const char*)USART1_TX_BUF));
}

/**
 * [USART_DMA_RX_LEN ���ش���DMA���������յĽ��ճ���]
 * @param  DMAy_Streamx [����DMA]
 * @param  BufSize      [BUF�ĳ���]
 * @return              [�������ݵĳ���]
 */
uint16_t USART_DMA_RX_LEN(DMA_Stream_TypeDef* DMAy_Streamx,uint16_t BufSize)
{
//	printf("%dBytes\r\n",length);
//	length=BufSize - DMAy_Streamx->NDTR;
	return (BufSize - DMAy_Streamx->NDTR);
}

/*
* function name:Shake_Report
* brief:shake hand
* input:none
* output:none
*/

void Shake_Report(void)
{
	uint8_t txbuff[2] = {0XCF,0XDF};
	USART1_DMA_Send(txbuff, sizeof(txbuff));
}
/*
*********************************************************************************************************
*	�� �� ��: USART1_IRQHandler  USART2_IRQHandler USART3_IRQHandler UART4_IRQHandler UART5_IRQHandler
*	����˵��: USART�жϷ������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
uint16_t USART1_RX_STA = 0;
uint8_t usart1_sof_flag = 0,usart1_rx_cnt = 0;
extern uint8_t DATA_medo;
extern u16 from_xian_dis;
void USART1_IRQHandler(void)
{
	uint8_t Res = 0;
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)//��ͨ�ж�
	{
		#if 0
		Res = USART_ReceiveData(USART1);
		if((USART1_RX_STA&0x8000)==0)//����δ���
		{
			if(USART1_RX_STA&0x4000)//���յ���0x0d
			{
				if(Res!=0x0a)USART1_RX_STA=0;//���մ���,���¿�ʼ
				else USART1_RX_STA|=0x8000;	//��������� 
			}
			else //��û�յ�0X0D
			{
				if(Res==0x0d)
				{
					USART1_RX_STA|=0x4000;
				}
				else
				{
					USART1_RX_BUF[USART1_RX_STA&0X3FFF]=Res;
					USART1_RX_STA++;
					if(USART1_RX_STA>(UART1_RX_BUF_SIZE-1))USART1_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
				}		 
			}
		}
		#else
		Res =USART_ReceiveData(USART1);
		if(usart1_sof_flag == 0)
		{
			if(Res==0xF1)//SOF
			{
			   usart1_sof_flag=1;
			   usart1_rx_cnt = 0;
			}
		}
		if(usart1_sof_flag==1)
		{
			USART1_RX_BUF[usart1_rx_cnt] = Res;
			usart1_rx_cnt++;
			if(Res==0x19)//EOF
			{
				from_xian_dis = USART1_RX_BUF[1]*1000;
				DATA_medo = USART1_RX_BUF[2];
				usart1_sof_flag=0;
				usart1_rx_cnt = 0;
			}
		}
		#endif
	}
}


void DMA2_Stream7_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7))
	{
		UsartDmaTxFlag = 0;
		DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TCIF7);
	}
}

void DMA2_Stream2_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA2_Stream2, DMA_IT_TCIF4))
	{
		DMA_ClearITPendingBit(DMA2_Stream2, DMA_IT_TCIF4);
	}
}

#ifdef __cplusplus
}
#endif

