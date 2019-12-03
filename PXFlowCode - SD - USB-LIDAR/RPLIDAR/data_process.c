#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include "data_process.h"
#include "stm32f4xx_rcc.h"
#include "usart.h"
#include "bsp_usart.h"
#include "laser_data_parser.h"
#include "FastMath.h"
#include "crc8.h"
#include "gps.h"

uint8_t DATA_medo = 0;


//u8 send[100] = {0};
/**
  *u16 data[1000]： u16的数据数组，u8 num：数据长度，
	u8 mode：工作模式（数据模式：      设置模式：      读取模式：      ）
  */ 


void send_data_dma(u16 data[100],u8 num,u8 mode)
{
	u8 x = 0;
	if(num>15)num=15;
	USART1_TX_BUF[0]=0x11;
	USART1_TX_BUF[1]=num|0x50;
	USART1_TX_BUF[2]= mode;
	for(x=0;x<num;x++)
	{
		USART1_TX_BUF[3+2*x]=(u8)((data[x]>>8)&0xff);
		USART1_TX_BUF[3+2*x+1]=(u8)(data[x]&0xff);
	}
//	USART1_TX_BUF[3+2*num]=0x22;
	USART1_TX_BUF[3+2*num] = CRC8_Table(USART1_TX_BUF, 3+2*num);//crc code
	
	DMA_Cmd (DMA2_Stream7,DISABLE);
	while (DMA_GetCmdStatus(DMA2_Stream7) != DISABLE){}
	DMA_SetCurrDataCounter(DMA2_Stream7,4+2*num);
	DMA_Cmd (DMA2_Stream7,ENABLE);//使能DMA,开始发送
}

void sort(u16 *a, u16 l)
{
    u16 i, j;
    u16 v;
    for(i = 0; i < l - 1; i ++)
        for(j = i+1; j < l; j ++)
        {
            if(a[i] > a[j])
            {
                v = a[i];
                a[i] = a[j];
                a[j] = v;
            }
        }
}
int ABS(int value){
	if(value<0)  
		return -value;
	else
		return value;
}

const float MYPI = 3.1415926f;
u16 min_xian_dis = 0;
u16 min_xian_angle = 0;
u16 Xian_shu_count= 0,Xian_send_count= 0;
u16 test_shu_count= 0;

u16 xian_dis[500];
u16 xian_angle[500];
int shu_X[2000];
int shu_Y[2000];
u16 location_shuzhang_dis=0,location_shuzhang_X=0,location_shuzhang_Y=0;
u16 xian_num = 0, shu_num = 0;

u16 from_xian_dis=4000;

void count_dis(float angle, float distance)
{
	if (angle >80 && angle < 100)  
	{
		if(distance > 500 && distance < 6000)
		{
			xian_dis[xian_num]=distance;
			xian_angle[xian_num]=angle;
			xian_num++;
		}
		if(distance>6000&& distance < 25000){
			shu_Y[shu_num] = FastSin(angle / 180 * MYPI)*distance;
			shu_X[shu_num] = FastCos(angle / 180 * MYPI)*distance;
			shu_num++;
		}
	}
	if(angle >=0 && angle <80) 
	{
		if (distance > 500 && distance < 25000)
		{
			shu_Y[shu_num] = FastSin(angle / 180 * MYPI)*distance;
			shu_X[shu_num] = FastCos(angle / 180 * MYPI)*distance;
			shu_num++;
		}
	}
	if(angle >=100 && angle <180) 
	{
		if (distance > 500 && distance < 25000)
		{
			shu_Y[shu_num] = FastSin(angle / 180 * MYPI)*distance;
			shu_X[shu_num] = FastCos(angle / 180 * MYPI)*distance;
			shu_num++;
		}
	}
}


void mid_distance()
{
	int k;
	sort(xian_dis,xian_num);
	if(xian_num>1){
		min_xian_dis=xian_dis[1];
		for ( k= 0; k < xian_num; k++)
		{
			if(min_xian_dis == xian_dis[k])
			{
				min_xian_angle=xian_angle[k]*10;
			}
		}
	}
	else
	{
		min_xian_angle=0;
		min_xian_dis=0;
	}
}

void find_min_dis(int *data2_X, int *data2_Y, u16 num2)
{
	int k;
	u16 min_dis = 0;
	u16 shuzhang_dis[2000];
	u16 shuzhang_count = 0;
	if(num2>5){
		for (k = 0; k < num2; k++)
		{
			min_dis =(u16)FastSqrt(pow((0 - data2_X[k]), 2) + pow((from_xian_dis - data2_Y[k]), 2));   
			shuzhang_dis[shuzhang_count]=min_dis;
			shuzhang_count++;
		}
		sort(shuzhang_dis,shuzhang_count);
		min_dis=shuzhang_dis[5];
		for (k = 0; k < num2; k++)
			{
				if(min_dis==(u16)FastSqrt(pow((0 - data2_X[k]), 2) + pow((from_xian_dis - data2_Y[k]), 2))){
					location_shuzhang_dis=min_dis;
					location_shuzhang_X=ABS(0 - data2_X[k]);
					location_shuzhang_Y=ABS(from_xian_dis - data2_Y[k]);
				}
			}
	}
	else{
		location_shuzhang_dis=0;
		location_shuzhang_X=0;
		location_shuzhang_Y=0;
	}
}


uint16_t senddata[100] = {0};
void find_min_xs_dis(void)
{
	if (xian_num > 1)
	{
		mid_distance();
	}
	else
	{
		min_xian_angle=0;
		min_xian_dis=0;
	}
	find_min_dis(shu_X, shu_Y, shu_num);
	if(DATA_medo==1){
		Xian_shu_data_record();
	}
	
	senddata[0]=(uint16_t)min_xian_dis;//最小线距角度
	senddata[1]=(uint16_t)min_xian_angle;//最小线距
	senddata[2]=(uint16_t)location_shuzhang_dis;//
	senddata[3]=(uint16_t)(bat_vol*100);//
	senddata[4]=(uint16_t)(DATA_medo);//
	senddata[5]=(uint16_t)(location_shuzhang_X);//
	senddata[6]=(uint16_t)(location_shuzhang_Y);//
	senddata[7]=(uint16_t)(shu_num);//
	senddata[8]=(uint16_t)(atk_gps_flag);//
//	send_data_dma(senddata,9,data_mode);
	u1_printf("shu_x:%d shu_y:%d num:%d vol:%.2f mode:%d gpsmo:%c lng:%f lat:%f\r\n",location_shuzhang_X,location_shuzhang_Y,shu_num,bat_vol,DATA_medo,rtk_data_mode,lon_atk,lat_atk);
	shu_num = 0;
	xian_num = 0;
}

unsigned char tempdata[6000] = {0};
laser_data_message_t R_message;
laser_data_status_t R_status;
uint16_t length = 0,cnt = 0;
float angle = 0, dis = 0;

void laser_data_analysis(uint16_t rxlength)
{
	u16 i=0,j=0,t=0;
	for(i=0;i<rxlength;i++)
	{
		if(USART2_RX_BUF[i] == 0xCE && USART2_RX_BUF[i+1] == 0xFA)//find head
		{	 
			cnt = 0;
		}
		tempdata[cnt] = USART2_RX_BUF[i];
		if(cnt==3)
		{
			length = 2 * (tempdata[3] * 256 + tempdata[2]);//find len
		}
		if(cnt==(7+length))
		{
			for(t = 0; t < (length+8); t++)
			{	
				laser_data_parse_char(tempdata[t], &R_message, &R_status);
			}
			for(j = 0; j < R_message.len; j++)
			{
				angle = (float)(R_message.begin_angle + j * 360.0f / R_message.len) / 10.0f;
				dis = R_message.distance[j]*10;
				angle=angle+90;                           //新版本雷达方向校准
				if(angle<0)angle=angle+360;
				if(angle>360)angle=angle-360;						
				count_dis(angle,dis);
			}
		}
		cnt++;
	}
	//-----------------//
	find_min_xs_dis();
	cnt=0;
	rxlength=0;
}

void Xian_shu_data_record(void)
{
	Xian_shu_count++;	
}

void Send_data_dis(void)
{
	u16 i=0;
	for(i=0;i<10;i++)
	{
		//senddata[i]=(uint16_t)Xian_shu[Xian_send_count*10+i];
	}
	send_data_dma(senddata,10,data_mode);
}

void Send_data_end(void)
{
	u16 i=0;
	for(i=0;i<10;i++)
	{
		senddata[i]=123;
	}
	send_data_dma(senddata,10,data_mode);
}

