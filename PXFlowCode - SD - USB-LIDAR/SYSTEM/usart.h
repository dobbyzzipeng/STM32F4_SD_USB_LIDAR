#ifndef __usart_h__
#define __usart_h__
#include "stm32f4xx_rcc.h"
 
#define USART1_BUFFER_SIZE      100
#define USART2_MAX_RECV_LEN		6000					//最大接收缓存字节数
#define USART1_MAX_SEND_LEN		6000					//最大发送缓存字节数

#define USART3_MAX_RECV_LEN	1024

extern uint8_t  USART2_RX_BUF[USART2_MAX_RECV_LEN];
extern uint16_t usart2_rx_num;
extern uint8_t  usart2_rx_data_flag;

extern uint16_t USART3_RX_STA;   	//接收数据状态
extern uint8_t USART3_TX_BUF[512];
extern uint8_t  USART3_RX_BUF[USART3_MAX_RECV_LEN];
extern uint16_t usart3_rx_num;
extern uint8_t  usart3_rx_data_flag;

void usart2_init(unsigned long int baudrate);
void usart3_init(unsigned long int baudrate);
//----------------------- Extern variable ----------------------//
#endif // __uart_h__
