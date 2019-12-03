#include "gps.h" 
#include "led.h" 							   
#include "usart.h" 								   
#include "stdio.h"	 
#include "stdarg.h"	 
#include "string.h"	 
#include "math.h"
#include "bsp_usart.h"
#include "FastMath.h"
#include "delay.h"

const uint32_t BAUD_id[9]={4800,9600,19200,38400,57600,115200,230400,460800,921600};//模块支持波特率数组

//从buf里面得到第cx个逗号所在的位置
//返回值:0~0XFE,代表逗号所在位置的偏移.
//       0XFF,代表不存在第cx个逗号							  
uint8_t NMEA_Comma_Pos(uint8_t *buf,uint8_t cx)
{
	uint8_t *p=buf;
	while(cx)
	{		 
		if(*buf=='*'||*buf<' '||*buf>'z')return 0XFF;//遇到'*'或者非法字符,则不存在第cx个逗号
		if(*buf==',')cx--;
		buf++;
	}
	return buf-p;	 
}
//m^n函数
//返回值:m^n次方.
uint32_t NMEA_Pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}
//str转换为数字,以','或者'*'结束
//buf:数字存储区
//dx:小数点位数,返回给调用函数
//返回值:转换后的数值
int NMEA_Str2num(uint8_t *buf,uint8_t*dx)
{
	uint8_t *p=buf;
	uint32_t ires=0,fres=0;
	uint8_t ilen=0,flen=0,i;
	uint8_t mask=0;
	int res;
	while(1) //得到整数和小数的长度
	{
		if(*p=='-'){mask|=0X02;p++;}//是负数
		if(*p==','||(*p=='*'))break;//遇到结束了
		if(*p=='.'){mask|=0X01;p++;}//遇到小数点了
		else if(*p>'9'||(*p<'0'))	//有非法字符
		{	
			ilen=0;
			flen=0;
			break;
		}	
		if(mask&0X01)flen++;
		else ilen++;
		p++;
	}
	if(mask&0X02)buf++;	//去掉负号
	for(i=0;i<ilen;i++)	//得到整数部分数据
	{  
		ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
	}
	if(flen>5)flen=5;	//最多取5位小数
	*dx=flen;	 		//小数点位数
	for(i=0;i<flen;i++)	//得到小数部分数据
	{  
		fres+=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
	} 
	res=ires*NMEA_Pow(10,flen)+fres;
	if(mask&0X02)res=-res;		   
	return res;
}
//分析GPGSV信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GPGSV_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
	uint8_t *p,*p1,dx;
	uint8_t len,i,j,slx=0;
	uint8_t posx;   	 
	p=buf;
	p1=(uint8_t*)strstr((const char *)p,"$GPGSV");
	len=p1[7]-'0';								//得到GPGSV的条数
	posx=NMEA_Comma_Pos(p1,3); 					//得到可见卫星总数
	if(posx!=0XFF)gpsx->svnum=NMEA_Str2num(p1+posx,&dx);
	for(i=0;i<len;i++)
	{	 
		p1=(uint8_t*)strstr((const char *)p,"$GPGSV");  
		for(j=0;j<4;j++)
		{	  
			posx=NMEA_Comma_Pos(p1,4+j*4);
			if(posx!=0XFF)gpsx->slmsg[slx].num=NMEA_Str2num(p1+posx,&dx);	//得到卫星编号
			else break; 
			posx=NMEA_Comma_Pos(p1,5+j*4);
			if(posx!=0XFF)gpsx->slmsg[slx].eledeg=NMEA_Str2num(p1+posx,&dx);//得到卫星仰角 
			else break;
			posx=NMEA_Comma_Pos(p1,6+j*4);
			if(posx!=0XFF)gpsx->slmsg[slx].azideg=NMEA_Str2num(p1+posx,&dx);//得到卫星方位角
			else break; 
			posx=NMEA_Comma_Pos(p1,7+j*4);
			if(posx!=0XFF)gpsx->slmsg[slx].sn=NMEA_Str2num(p1+posx,&dx);	//得到卫星信噪比
			else break;
			slx++;	   
		}   
 		p=p1+1;//切换到下一个GPGSV信息
	}   
}
//分析GLGSV信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GLGSV_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
	uint8_t *p,*p1,dx;
	uint8_t len,i,j,slx=0;
	uint8_t posx;   	 
	p=buf;
	p1=(uint8_t*)strstr((const char *)p,"$GLGSV");
	len=p1[7]-'0';								//得到GLGSV的条数
	posx=NMEA_Comma_Pos(p1,3); 					//得到可见卫星总数
	if(posx!=0XFF)gpsx->glonassnum=NMEA_Str2num(p1+posx,&dx);//GLONASS 卫星总数
	for(i=0;i<len;i++)
	{	 
		p1=(uint8_t*)strstr((const char *)p,"$GLGSV");  
		for(j=0;j<4;j++)
		{	  
			posx=NMEA_Comma_Pos(p1,4+j*4);
			if(posx!=0XFF)gpsx->glonassmsg[slx].num=NMEA_Str2num(p1+posx,&dx);	//得到GLONASS卫星编号
			else break; 
			posx=NMEA_Comma_Pos(p1,5+j*4);
			if(posx!=0XFF)gpsx->glonassmsg[slx].eledeg=NMEA_Str2num(p1+posx,&dx);//得到GLONASS卫星仰角 
			else break;
			posx=NMEA_Comma_Pos(p1,6+j*4);
			if(posx!=0XFF)gpsx->glonassmsg[slx].azideg=NMEA_Str2num(p1+posx,&dx);//得到GLONASS卫星方位角
			else break; 
			posx=NMEA_Comma_Pos(p1,7+j*4);
			if(posx!=0XFF)gpsx->glonassmsg[slx].sn=NMEA_Str2num(p1+posx,&dx);	//得到GLONASS卫星信噪比
			else break;
			slx++;	   
		}   
 		p=p1+1;//切换到下一个GLGSV信息
	}
}
//分析BDGSV信息
//gpsx:nmea信息结构体
//buf:接收到的北斗数据缓冲区首地址
void NMEA_BDGSV_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
	uint8_t *p,*p1,dx;
	uint8_t len,i,j,slx=0;
	uint8_t posx;   	 
	p=buf;
	p1=(uint8_t*)strstr((const char *)p,"$BDGSV");
	len=p1[7]-'0';								//得到BDGSV的条数
	posx=NMEA_Comma_Pos(p1,3); 					//得到可见北斗卫星总数
	if(posx!=0XFF)gpsx->beidou_svnum=NMEA_Str2num(p1+posx,&dx);
	for(i=0;i<len;i++)
	{	 
		p1=(uint8_t*)strstr((const char *)p,"$BDGSV");  
		for(j=0;j<4;j++)
		{	  
			posx=NMEA_Comma_Pos(p1,4+j*4);
			if(posx!=0XFF)gpsx->beidou_slmsg[slx].beidou_num=NMEA_Str2num(p1+posx,&dx);	//得到卫星编号
			else break; 
			posx=NMEA_Comma_Pos(p1,5+j*4);
			if(posx!=0XFF)gpsx->beidou_slmsg[slx].beidou_eledeg=NMEA_Str2num(p1+posx,&dx);//得到卫星仰角 
			else break;
			posx=NMEA_Comma_Pos(p1,6+j*4);
			if(posx!=0XFF)gpsx->beidou_slmsg[slx].beidou_azideg=NMEA_Str2num(p1+posx,&dx);//得到卫星方位角
			else break; 
			posx=NMEA_Comma_Pos(p1,7+j*4);
			if(posx!=0XFF)gpsx->beidou_slmsg[slx].beidou_sn=NMEA_Str2num(p1+posx,&dx);	//得到卫星信噪比
			else break;
			slx++;	   
		}   
 		p=p1+1;//切换到下一个BDGSV信息
	}   
}
//分析GNGGA信息
//gpsx:nmea信息结构体
//buf:接收到的GPS/北斗数据缓冲区首地址
void NMEA_GNGGA_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
	uint8_t *p1,dx;			 
	uint8_t posx;    
	p1=(uint8_t*)strstr((const char *)buf,"$GNGGA");
	posx=NMEA_Comma_Pos(p1,6);								//得到GPS状态
	if(posx!=0XFF)gpsx->gpssta=NMEA_Str2num(p1+posx,&dx);	
	posx=NMEA_Comma_Pos(p1,7);								//得到用于定位的卫星数
	if(posx!=0XFF)gpsx->posslnum=NMEA_Str2num(p1+posx,&dx); 
	posx=NMEA_Comma_Pos(p1,9);								//得到海拔高度
	if(posx!=0XFF)gpsx->altitude=NMEA_Str2num(p1+posx,&dx);  
}
//分析GNGSA信息
//gpsx:nmea信息结构体
//buf:接收到的GPS/北斗数据缓冲区首地址
void NMEA_GNGSA_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
	uint8_t *p1,dx;			 
	uint8_t posx; 
	uint8_t i;   
	p1=(uint8_t*)strstr((const char *)buf,"$GNGSA");
	posx=NMEA_Comma_Pos(p1,2);								//得到定位类型
	if(posx!=0XFF)gpsx->fixmode=NMEA_Str2num(p1+posx,&dx);	
	for(i=0;i<12;i++)										//得到定位卫星编号
	{
		posx=NMEA_Comma_Pos(p1,3+i);					 
		if(posx!=0XFF)gpsx->possl[i]=NMEA_Str2num(p1+posx,&dx);
		else break; 
	}				  
	posx=NMEA_Comma_Pos(p1,15);								//得到PDOP位置精度因子
	if(posx!=0XFF)gpsx->pdop=NMEA_Str2num(p1+posx,&dx);  
	posx=NMEA_Comma_Pos(p1,16);								//得到HDOP位置精度因子
	if(posx!=0XFF)gpsx->hdop=NMEA_Str2num(p1+posx,&dx);  
	posx=NMEA_Comma_Pos(p1,17);								//得到VDOP位置精度因子
	if(posx!=0XFF)gpsx->vdop=NMEA_Str2num(p1+posx,&dx);  
}
char rtk_data_mode;
//分析GNRMC信息
//gpsx:nmea信息结构体
//buf:接收到的GPS/北斗数据缓冲区首地址
void NMEA_GNRMC_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
	uint8_t *p1,dx;			 
	uint8_t posx;     
	uint32_t temp;	   
	float rs;
	#if (USE_GPS_TYPE == USE_ATK_MODULE)
	p1=(uint8_t*)strstr((const char *)buf,"GNRMC");//"$GNRMC",经常有&和GNRMC分开的情况,故只判断GPRMC.
	#elif (USE_GPS_TYPE == USE_M8N_MODULE)
	p1=(uint8_t*)strstr((const char *)buf,"$GNRMC");//"$GNRMC",经常有&和GNRMC分开的情况,故只判断GPRMC.
	#endif
	posx=NMEA_Comma_Pos(p1,1);								//得到UTC时间
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);	 	//得到UTC时间,去掉ms
		gpsx->utc.hour=temp/10000;
		gpsx->utc.min=(temp/100)%100;
		gpsx->utc.sec=temp%100;	 	 
	}	
	posx=NMEA_Comma_Pos(p1,3);								//得到纬度
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);		 	 
		gpsx->latitude=temp/NMEA_Pow(10,dx+2);	//得到°
		rs=temp%NMEA_Pow(10,dx+2);				//得到'		 
		#if (USE_GPS_TYPE == USE_ATK_MODULE)
		gpsx->latitude=gpsx->latitude*NMEA_Pow(10,7)+(rs*NMEA_Pow(10,7-dx))/60;//转换为°
		#elif (USE_GPS_TYPE == USE_M8N_MODULE)
		gpsx->latitude=gpsx->latitude*NMEA_Pow(10,7)+(rs*NMEA_Pow(10,7-dx))/60;//转换为°
		#endif
	}
	posx=NMEA_Comma_Pos(p1,4);								//南纬还是北纬 
	if(posx!=0XFF)gpsx->nshemi=*(p1+posx);					 
 	posx=NMEA_Comma_Pos(p1,5);								//得到经度
	if(posx!=0XFF)
	{												  
		temp=NMEA_Str2num(p1+posx,&dx);		 	 
		gpsx->longitude=temp/NMEA_Pow(10,dx+2);	//得到°
		rs=temp%NMEA_Pow(10,dx+2);				//得到'		
		#if (USE_GPS_TYPE == USE_ATK_MODULE)		
		gpsx->longitude=gpsx->longitude*NMEA_Pow(10,7)+(rs*NMEA_Pow(10,7-dx))/60;//转换为° 
		#elif (USE_GPS_TYPE == USE_M8N_MODULE)
		gpsx->longitude=gpsx->longitude*NMEA_Pow(10,7)+(rs*NMEA_Pow(10,7-dx))/60;//转换为° 
		#endif
	}
	posx=NMEA_Comma_Pos(p1,6);								//东经还是西经
	if(posx!=0XFF)gpsx->ewhemi=*(p1+posx);		 
	posx=NMEA_Comma_Pos(p1,12);
	if(posx!=0XFF)
	{
		temp = *(p1+posx);
		if(temp=='A')
		{
			rtk_data_mode = 'A';
		}
		else if(temp=='D')
		{
			rtk_data_mode = 'D';
		}
		else if(temp=='F')
		{
			rtk_data_mode = 'F';
		}
		else if(temp=='R')
		{
			rtk_data_mode = 'R';
		}
		else if(temp=='N')
		{
			rtk_data_mode = 'N';
		}
		else{
			rtk_data_mode = 10;
		}
	}
	
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);	
	}
	posx=NMEA_Comma_Pos(p1,9);								//得到UTC日期
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx);		 				//得到UTC日期
		gpsx->utc.date=temp/10000;
		gpsx->utc.month=(temp/100)%100;
		gpsx->utc.year=2000+temp%100;	 	 
	} 
}
//分析GNVTG信息
//gpsx:nmea信息结构体
//buf:接收到的GPS/北斗数据缓冲区首地址
void NMEA_GNVTG_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
	uint8_t *p1,dx;			 
	uint8_t posx;    
	p1=(uint8_t*)strstr((const char *)buf,"$GNVTG");							 
	posx=NMEA_Comma_Pos(p1,7);								//得到地面速率
	if(posx!=0XFF)
	{
		gpsx->speed=NMEA_Str2num(p1+posx,&dx);
		if(dx<3)gpsx->speed*=NMEA_Pow(10,3-dx);	 	 		//确保扩大1000倍
	}
}  
//提取NMEA-0183信息
//gpsx:nmea信息结构体
//buf:接收到的GPS/北斗数据缓冲区首地址
void GPS_Analysis(nmea_msg *gpsx,uint8_t *buf)
{
	NMEA_GPGSV_Analysis(gpsx,buf);	//GPGSV解析  可见GPS定位信息 !
	#if (USE_GPS_TYPE == USE_ATK_MODULE)
	NMEA_BDGSV_Analysis(gpsx,buf);	//BDGSV解析  可见北斗定位信息 !
	#elif (USE_GPS_TYPE == USE_M8N_MODULE)
	NMEA_GLGSV_Analysis(gpsx,buf);  //GLGSV解析  可见GLONASS定位信息 !
	#endif
	NMEA_GNGGA_Analysis(gpsx,buf);	//GNGGA解析  GPS/北斗定位信息	!!!
	#if (USE_GPS_TYPE == USE_ATK_MODULE)
	NMEA_GNGSA_Analysis(gpsx,buf);	//GNGSA解析  当前卫星信息  基本没有用
	#endif	
	NMEA_GNRMC_Analysis(gpsx,buf);	//GNRMC解析  推荐定位信息!!!
	NMEA_GNVTG_Analysis(gpsx,buf);	//GNVTG解析  地面速度信息						
									//GNGLL解析  大地坐标信息/定位地理信息
									//GNZDA解析  当前时间信息
}
///////////////////////////////////////////SkyTraq 配置代码/////////////////////////////////////
////检查CFG配置执行情况
////返回值:0,ACK成功
////       1,接收超时错误
////       2,没有找到同步字符
////       3,接收到NACK应答
uint8_t SkyTra_Cfg_Ack_Check(void)
{			 
	uint16_t len=0,i;
	uint8_t rval=0;
	while((USART3_RX_STA&0X8000)==0 && len<100)//等待接收到应答   
	{
		len++;
		delay_ms(5);
	}		 
	if(len<100)   	//超时错误.
	{
		len=USART3_RX_STA&0X7FFF;	//此次接收到的数据长度 
		for(i=0;i<len;i++)
		{
			if(USART3_RX_BUF[i]==0X83)break;
			else if(USART3_RX_BUF[i]==0X84)
			{
				rval=3;
				break;
			}
		}
		if(i==len)rval=2;						//没有找到同步字符
	}else rval=1;								//接收超时错误
    USART3_RX_STA=0;							//清除接收
	return rval;  
}
//配置SkyTra_GPS/北斗模块波特率
//baud_id:0~8，对应波特率,4800/9600/19200/38400/57600/115200/230400/460800/921600	  
//返回值:0,执行成功;其他,执行失败(这里不会返回0了)
uint8_t SkyTra_Cfg_Prt(uint8_t baud_id)
{
	SkyTra_baudrate *cfg_prt=(SkyTra_baudrate *)USART3_TX_BUF;
	cfg_prt->sos=0XA1A0;		//引导序列(小端模式)
	cfg_prt->PL=0X0400;			//有效数据长度(小端模式)
	cfg_prt->id=0X05;		    //配置波特率的ID 
	cfg_prt->com_port=0X00;			//操作串口1
	cfg_prt->Baud_id=baud_id;	 	////波特率对应编号
	cfg_prt->Attributes=1; 		  //保存到SRAM&FLASH
	cfg_prt->CS=cfg_prt->id^cfg_prt->com_port^cfg_prt->Baud_id^cfg_prt->Attributes;
	cfg_prt->end=0X0A0D;        //发送结束符(小端模式)
	SkyTra_Send_Date((uint8_t*)cfg_prt,sizeof(SkyTra_baudrate));//发送数据给SkyTra   
	delay_ms(200);				//等待发送完成   
	usart3_init(BAUD_id[baud_id]);	//重新初始化串口4
	return SkyTra_Cfg_Ack_Check();
} 
//配置SkyTra_GPS/北斗模块的时钟脉冲宽度
//width:脉冲宽度1~100000(us)
//返回值:0,发送成功;其他,发送失败.
uint8_t SkyTra_Cfg_Tp(uint32_t width)
{
	uint32_t temp=width;
	SkyTra_pps_width *cfg_tp=(SkyTra_pps_width *)USART3_TX_BUF;
	temp=(width>>24)|((width>>8)&0X0000FF00)|((width<<8)&0X00FF0000)|((width<<24)&0XFF000000);//小端模式
	cfg_tp->sos=0XA1A0;		    //cfg header(小端模式)
	cfg_tp->PL=0X0700;        //有效数据长度(小端模式)
	cfg_tp->id=0X65	;			    //cfg tp id
	cfg_tp->Sub_ID=0X01;			//数据区长度为20个字节.
	cfg_tp->width=temp;		  //脉冲宽度,us
	cfg_tp->Attributes=0X01;  //保存到SRAM&FLASH	
	cfg_tp->CS=cfg_tp->id^cfg_tp->Sub_ID^(cfg_tp->width>>24)^(cfg_tp->width>>16)&0XFF^(cfg_tp->width>>8)&0XFF^cfg_tp->width&0XFF^cfg_tp->Attributes;    	
	cfg_tp->end=0X0A0D;       //发送结束符(小端模式)
	SkyTra_Send_Date((uint8_t*)cfg_tp,sizeof(SkyTra_pps_width));//发送数据给SkyTraF8-BD  
	return SkyTra_Cfg_Ack_Check();
}
//配置SkyTraF8-BD的更新速率	    
//Frep:（取值范围:1,2,4,5,8,10,20）测量时间间隔，单位为Hz，最大不能大于20Hz
//返回值:0,发送成功;其他,发送失败.
uint8_t SkyTra_Cfg_Rate(uint8_t Frep)
{
	#if (USE_GPS_TYPE == USE_ATK_MODULE)
	SkyTra_PosRate *cfg_rate=(SkyTra_PosRate *)USART3_TX_BUF;
 	cfg_rate->sos=0XA1A0;	    //cfg header(小端模式)
	cfg_rate->PL=0X0300;			//有效数据长度(小端模式)
	cfg_rate->id=0X0E;	      //cfg rate id
	cfg_rate->rate=Frep;	 	  //更新速率
	cfg_rate->Attributes=0X01;	   	//保存到SRAM&FLASH	.
	cfg_rate->CS=cfg_rate->id^cfg_rate->rate^cfg_rate->Attributes;    //校验值
	cfg_rate->end=0X0A0D;       //发送结束符(小端模式)
	SkyTra_Send_Date((uint8_t*)cfg_rate,sizeof(SkyTra_PosRate));//发送数据给SkyTraF8-BD 
	return SkyTra_Cfg_Ack_Check();
	#elif (USE_GPS_TYPE == USE_M8N_MODULE)
 	USART3_TX_BUF[0]=0Xb5;
	USART3_TX_BUF[1]=0X62;
	USART3_TX_BUF[2]=0X06;
	USART3_TX_BUF[3]=0X08;
	USART3_TX_BUF[4]=0X06;
	USART3_TX_BUF[5]=0X00;
	USART3_TX_BUF[6]=0Xc8;
	USART3_TX_BUF[7]=0X00;
	USART3_TX_BUF[8]=0X01;
	USART3_TX_BUF[9]=0X00;
	USART3_TX_BUF[10]=0X01;
	USART3_TX_BUF[11]=0X00;
	USART3_TX_BUF[12]=0Xde;
	USART3_TX_BUF[13]=0X6a;
	SkyTra_Send_Date(USART3_TX_BUF,14);//发送数据给M8N
	delay_ms(100);
	USART3_TX_BUF[0]=0Xb5;
	USART3_TX_BUF[1]=0X62;
	USART3_TX_BUF[2]=0X06;
	USART3_TX_BUF[3]=0X08;
	USART3_TX_BUF[4]=0X00;
	USART3_TX_BUF[5]=0X00;
	USART3_TX_BUF[6]=0X0e;
	USART3_TX_BUF[7]=0X30;
	SkyTra_Send_Date(USART3_TX_BUF,8);//发送数据给M8N
	return SkyTra_Cfg_Ack_Check();
	#endif
}
//发送一批数据给SkyTraF8-BD，这里通过串口3发送
//dbuf：数据缓存首地址
//len：要发送的字节数
void SkyTra_Send_Date(uint8_t* dbuf,uint16_t len)
{
	uint16_t j;
	for(j=0;j<len;j++)//循环发送数据
	{
		while((UART4->SR&0X40)==0);//循环发送,直到发送完毕   
		UART4->DR=dbuf[j];  
	}
}


nmea_msg gpsx; 											//GPS信息
__align(4) uint8_t dtbuf[50];   								//打印缓存器
const uint8_t*fixmode_tbl[4]={"Fail","Fail"," 2D "," 3D "};	//fix mode字符串 

void Gps_Msg_Show(void)
{
 	float tp;		    	 
	tp=gpsx.longitude;	   
	sprintf((char *)dtbuf,"Longitude:%.5f %1c   ",tp/=100000,gpsx.ewhemi);	//得到经度字符串  
	tp=gpsx.latitude;	   
	sprintf((char *)dtbuf,"Latitude:%.5f %1c   ",tp/=100000,gpsx.nshemi);	//得到纬度字符串	 
	tp=gpsx.altitude;	   
 	sprintf((char *)dtbuf,"Altitude:%.1fm     ",tp/=10);	    			//得到高度字符串	 			   
	tp=gpsx.speed;	   
 	sprintf((char *)dtbuf,"Speed:%.3fkm/h     ",tp/=1000);		    		//得到速度字符串	 	 				    
	if(gpsx.fixmode<=3)														//定位状态
	{  
		sprintf((char *)dtbuf,"Fix Mode:%s",fixmode_tbl[gpsx.fixmode]);				   
	}	 	
	
	sprintf((char *)dtbuf,"GPS+BD Valid satellite:%02d",gpsx.posslnum);	 		//用于定位的GPS卫星数	    
	sprintf((char *)dtbuf,"GPS Visible satellite:%02d",gpsx.svnum%100);	 		//可见GPS卫星数
	
	sprintf((char *)dtbuf,"BD Visible satellite:%02d",gpsx.beidou_svnum%100);	 		//可见北斗卫星数
	
	sprintf((char *)dtbuf,"UTC Date:%04d/%02d/%02d   ",gpsx.utc.year,gpsx.utc.month,gpsx.utc.date);	//显示UTC日期	    
	sprintf((char *)dtbuf,"UTC Time:%02d:%02d:%02d   ",gpsx.utc.hour,gpsx.utc.min,gpsx.utc.sec);	//显示UTC时间
}

volatile double lon_atk = 0,lat_atk = 0;
float gps_speed = 0;
int gps_num = 0,beidou_num = 0,glonass_num = 0;
//显示GPS定位信息 
void Gps_Msg_Prf(void)
{
	#if (USE_GPS_TYPE == USE_ATK_MODULE)
	lon_atk = gpsx.longitude/10000000.0f;
	lat_atk = gpsx.latitude/10000000.0f;
	gps_speed = gpsx.speed/3600.0f;//m/s
	gps_num = gpsx.svnum%100;
	beidou_num = gpsx.beidou_svnum%100;
	#elif (USE_GPS_TYPE == USE_M8N_MODULE)
	lon_atk = gpsx.longitude/10000000.0f;
	lat_atk = gpsx.latitude/10000000.0f;
	gps_speed = gpsx.speed/3600.0f;//m/s
	gps_num = gpsx.svnum%100;
	glonass_num = gpsx.glonassnum%100;
	#endif
	
	//gpsx.utc.year,gpsx.utc.month,gpsx.utc.date//显示UTC日期	    
	//gpsx.utc.hour,gpsx.utc.min,gpsx.utc.sec	//显示UTC时间
	
//	u1_printf("Lon:%.5fE\tlat:%.5fN\tspeed:%fm/s\tGPSnum:%02d\tBDnum:%02d\r\n",lon_atk,lat_atk,gpsx.speed/3600.0,gpsx.svnum%100,gpsx.beidou_svnum%100);
//	u1_printf("GPS+BD Valid sate:%02d\t GPS Vis sate:%02d\t BD Vis sate:%02d\r\n",gpsx.posslnum,gpsx.svnum%100,gpsx.beidou_svnum%100);

//	printf("Longitude:%.5fE\t latitude:%.5fN\t Altitude:%.2fm\r\n",lon_atk,lat_atk,gpsx.altitude/10.0);	//得到经度字符串,纬度字符串 ,得到高度字符串	  
//  printf("Speed:%.3fkm/h\t Fix Mode:%d\t GPS+BD Valid satellite:%02d\t GPS Visible satellite:%02d\t BD Visible satellite:%02d\r\n",gpsx.speed/1000.0,gpsx.fixmode,gpsx.posslnum,gpsx.svnum%100,gpsx.beidou_svnum%100);	//得到速度字符串,定位状态,用于定位的卫星数,可见卫星数 
//	printf("UTC Date:%04d/%02d/%02d\t UTC Time:%02d:%02d:%02d\r\n",gpsx.utc.year,gpsx.utc.month,gpsx.utc.date,gpsx.utc.hour,gpsx.utc.min,gpsx.utc.sec);	//显示UTC日期,显示UTC时间 
}

uint8_t gps_data_buff[1024] = {0};
uint8_t GPS_Init(void)
{
	usart3_init(115200);
	return 0;
}

uint8_t atk_gps_flag = 0,gps_num_flag = 0,atk_gps_count = 0;
uint8_t gps_data_analysis(void)
{
	uint16_t i,rxlen;
	if(USART3_RX_STA&0X8000)		//接收到一次数据了
	{
		rxlen=USART3_RX_STA&0X7FFF;	//得到数据长度
		for(i=0;i<rxlen;i++)gps_data_buff[i]=USART3_RX_BUF[i];	   
		USART3_RX_STA=0;		   	//启动下一次接收
		gps_data_buff[i]=0;			//自动添加结束符
		GPS_Analysis(&gpsx,(uint8_t*)gps_data_buff);//分析字符串
		Gps_Msg_Prf();
		atk_gps_flag = 1;
		atk_gps_count = 0;
	}
	#if (USE_GPS_TYPE==USE_ATK_MODULE)
	if((gps_num+beidou_num)>=14 && lon_atk>90 && (rtk_data_mode == 'D'|| rtk_data_mode == 'F'|| rtk_data_mode == 'R')) {
	#elif (USE_GPS_TYPE == USE_M8N_MODULE)
	if((gps_num+glonass_num)>=12&&lon_atk>90) {
	#endif
		return 1;
	}
	else {
		return 0;
	}
}

void utc_time_combination(int hour,int min,int sec,int microsec,uint64_t *utc_time)
{
	*utc_time = (hour+8)*100000+min*1000+sec*10+ microsec;
}

void utc_monthyear_combination(int year,int month,int date,uint64_t *utc_time)
{
	*utc_time = year*100000+month*1000+(date);
}
#define MYPI 3.1415926
#define REGTORAG 0.01745329
#define RAGTOREG 57.2957805
#define rad(x) (x * REGTORAG)
/**
     * 通过GPS坐标计算两点间的距离（米）
     * 
     * @param long1
     * @param lat1
     * @param long2
     * @param lat2
     * @return
     *///使用快速数学求值库加快4倍速度
//double gps_get_distance(double lon1, double lat1, double lon2,double lat2)
//{
//	double a, b, R;
//	R = 6378137; // 地球半径（米）
//	lat1 = lat1 * REGTORAG;
//	lat2 = lat2 * REGTORAG;
//	a = lat1 - lat2;
//	b = (lon1 - lon2) * REGTORAG;
//	double d;
//	double sa2, sb2;
//	sa2 = FastSin(a / 2.0);
//	sb2 = FastSin(b / 2.0);
//	d = 2* R* FastAsin(FastSqrt(sa2 * sa2 + FastCos(lat1)* FastCos(lat2) * sb2 * sb2));
//	return d;
//}
double gps_get_distance(double lon1, double lat1, double lon2,double lat2)
{
	double a, b, R;
	R = 6378137; // 地球半径（米）
	lat1 = lat1 * REGTORAG;
	lat2 = lat2 * REGTORAG;
	a = lat1 - lat2;
	b = (lon1 - lon2) * REGTORAG;
	double d;
	double sa2, sb2;
	sa2 = sin(a / 2.0);
	sb2 = sin(b / 2.0);
	d = 2* R* sin(FastSqrt(sa2 * sa2 + cos(lat1)* cos(lat2) * sb2 * sb2));
	return d;
}

double gps12d(double lona,double lata, double lonb, double latb)
{
    double d  = 0;
    double radlta = rad(lata);
    double radlna = rad(lona);
    double radltb = rad(latb);
    double radlnb = rad(lonb);

    double dlon = radlnb - radlna;
    double y = sin(dlon) * cos(radlta);
    double x = cos(radlta) * sin(radltb) - sin(radlta) * cos(radltb) * cos(dlon);
//    d = atan2(y,x) * 180.0 / PI;
    if ( y > 0 )
	{
        if (x > 0)
            d = atan2(y,x);
        else if ( x == 0 )
            d = 90;
        else
            d = 180 - atan2(-y,x);
    } 
    else if ( y == 0)
    {
        if (x > 0)
            d = 0;
        else if ( x == 0 )
            d = 0;
        else
            d = 180;
    }   
    else 
    {
        if (x > 0)
            d = -atan2(-y,x);
        else if ( x == 0 )
            d = 270;
        else
            d = atan2(y,x)-180;
    }   

    d = d * 180.0 / PI;
	if(d < 0) d += 360; 
    return d;
}

/*
*fun name: double gps_get_angle(double lng1,double lat1, double lng2, double lat2)
*@brief:根据输入的两个gps坐标计算方位角。正北方为0°，正东方为90°，正南方为180°，正西方为270°。范围0~360°.
		第一个点指向第二个点的连线与第一个点与正北方连线夹角
*@pra:lng1 lat1 第一个点的gps坐标，lng2,lat2第二个点的gps坐标。
*@return angle
*/
double gps_get_angle(double lng1,double lat1, double lng2, double lat2)
{
    double d = 0;
    double radLat1 = rad(lat1);//根据角度计算弧度
    double radLat2 = rad(lat2);
    double radLng1 = rad(lng1);
    double radLng2 = rad(lng2);

    d = sin(radLat1)*sin(radLat2)+cos(radLat1)*cos(radLat2)*cos(radLng2-radLng1);
    d = sqrt(1-d*d);
    d = cos(radLat2)*sin(radLng2-radLng1)/d;
    d = FastAsin(d) * (float)RAGTOREG;
	if(d < 0) d += 360; 
    return d;
}

float longitude1,latitude1,longitude2,latitude2;
void gps_markdown(uint8_t times)
{
	double l =0;
	if(times == 1)
	{
		longitude1 = lon_atk;
		latitude1 =  lat_atk;
		printf("p1 is ok\r\n");
	}
	else if( times == 2)
	{
		if(lon_atk!=longitude1 && lat_atk != latitude1)
		{
			longitude2 = lon_atk;
			latitude2 =  lat_atk;
			printf("p2 is ok\r\n");
		}
		else
		{
			printf("p2 is err,repeate...\r\n");
		}
	}
	else if( times == 3)
	{
		l = gps_get_distance(longitude1,latitude1,longitude2,latitude2);
		printf("dis:%f\r\n",l);
	}
	else
	{
		printf("err\r\n");
		return;	
	}
}

