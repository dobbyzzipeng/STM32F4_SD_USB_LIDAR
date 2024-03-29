/*********************************************************************************************************
*
*	模块名称 : 串口
*	文件名称 : bsp_uart.h
*	版    本 : V1.0
*	说    明 : C文件
*	1)采用DMA不定长发送，DMA的不定长接收
*********************************************************************************************************
*/

#ifndef _BSP_USART_H_
#define _BSP_USART_H_
#include "sys.h"
#ifdef __cplusplus
 extern "C" {
#endif


#include "stdio.h"

/************************************************************************************************/
/**************************************** 接口分割线 ********************************************/
/************************************************************************************************/
/* 定义串口波特率和FIFO缓冲区大小，分为发送缓冲区和接收缓冲区, 支持全双工 */

#define UART1_PIN			0  //0:TX = PA9;RX = PA10  1:TX = PB6   RX = PB7
#define UART1_BAUD			115200
#define UART1_TX_BUF_SIZE	1024
#define UART1_RX_BUF_SIZE	1024
#define Priority_UART1_Pre 0
#define Priority_UART1_Sub 1
/************************************************************************************************/
/**************************************** 接口分割线 ********************************************/
/************************************************************************************************/

#define USE_UART1_FIFO_DMA	1

extern uint8_t g_RxBuf1[UART1_RX_BUF_SIZE];		/* 接收缓冲区 */
extern uint8_t USART1_RX_BUF[UART1_RX_BUF_SIZE];
extern uint16_t USART1_RX_STA;
extern __align(8) uint8_t USART1_TX_BUF[UART1_TX_BUF_SIZE] ;

void bsp_InitUart(void);
void UsartDMA_Init(void);
void USART1_DMA_Send(uint8_t *pbuffer, uint32_t size);
void u1_printf(char* fmt,...);
uint16_t USART_DMA_RX_LEN(DMA_Stream_TypeDef* DMAy_Streamx,uint16_t BufSize);
void UsartDMAIRQ(void);

#ifdef __cplusplus
}
#endif
#endif

