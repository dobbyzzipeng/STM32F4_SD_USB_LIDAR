#include "ov7670.h"
#include "delay.h"
#include "sccb.h"
/*------------------------------------------------
注意：
{0x3a, 0x11},可以在改变分辨率后自动设置窗口
不调用OV7670_config_window，QCIF默认数据174*144，并非176*144
OV7670_config_window的startx如果不合适会改变UV顺序,或RGB图像产生噪点
用180以上的值挨个试
YUV转RGB的简单算法：
	B=Y+U+16;//可以不+16
	G=Y-(U+V)/4;
	R=Y+V+16;
	(RGB需限制在0~255的范围)
未使用小数，可能颜色略有偏差
	
--------------------------------------------------*/
//摄像头的宽与高
#define CAMWIDTH	176
#define CAMHEIGHT	144
//#define CAMWIDTH	160
//#define CAMHEIGHT	120

#define RST_H	IOSET(GPIOE->ODR,BIT(3))
#define RST_L	IOCLR(GPIOE->ODR,BIT(3))

#define PWDN_H	IOSET(GPIOE->ODR,BIT(0))
#define PWDN_L	IOCLR(GPIOE->ODR,BIT(0))

const uint8_t OV7670_REG[][2]={
//{0x11, 0x40},//内部时钟不分频
//{0x6b, 0x00},//关PLL,开LDO//写入这句电压会略微升高
//{0x2a, 0x00},//空相素
//{0x2b, 0x00},
//{0x92, 0x00},//空行
//{0x93, 0x00},
//{0x3b, 0x0a},//关闭夜晚模式
//Output format
//{0x12, 0x10},//QVGA(320*240),YUV
{0x12, 0x08},//QCIF(176×144),YUV
//-----------RGB565用下面这段,YUV则需要40、8c寄存器为默认值
//{0x12, 0x14},//QVGA(320*240),RAW
//{0x12, 0x0C},//QCIF(176×144),RAW
//{0x40, 0xD0},//RGB565
//{0x8c, 0x00},//RGB444关闭

//Special effects - 特效 
//normal
{0x3a, 0x04},//YUYV,自动UV
//{0x3a, 0x14},//YUYV,手动UV,可用于测试8位数据的正确性
//{0x3a, 0x11},//YUYV,手动UV,自动窗口
//{0x3d, 0x88},//YUYV
//{0x67, 0x10},//手动U
//{0x68, 0x15},//手动V
//Mirror/VFlip Enable - 水平镜像/竖直翻转使能
//{0x1e, 0x01},//修改配置值将产生图像显示上下或左右颠倒
//Banding Filter Setting for 24Mhz Input Clock - 条纹滤波器
//30fps for 60Hz light frequency
//{0x13, 0xe7},//banding filer enable
//{0x9d, 0x98},//50Hz banding filer
//{0x9e, 0x7f},//60Hz banding filer
//{0xa5, 0x02},//3 step for 50Hz
//{0xab, 0x03},//4 step for 60Hz
//{0x3b, 0x02},//select 60Hz banding filer
//Simple White Balance - 白平衡
//{0x13, 0xe7},//AWB、AGC、AGC Enable and ...
//{0x6f, 0x9f},//simple AWB
//AWBC - 自动白平衡控制(Automatic white balance control) 
//{0x43, 0x14},//用户手册里这些寄存器的值都是保留(Reserved),不用设置的呀？
//{0x44, 0xf0},
//{0x45, 0x34},        
//{0x46, 0x58},
//{0x47, 0x28},
//{0x48, 0x3a},

//AWB Control
//{0x59, 0x88},//用户手册连寄存器都是保留，初始值都没提供
//{0x5a, 0x88},        
//{0x5b, 0x44},
//{0x5c, 0x67},
//{0x5d, 0x49},
//{0x5e, 0x0e},

//AWB Control
//{0x6c, 0x0a},
//{0x6d, 0x55},       
//{0x6e, 0x11},
//{0x6f, 0x9f},

//AGC/AEC - Automatic Gain Control自动增益补偿/Automatic exposure Control自动曝光控制
//{0x00, 0x00},
//{0x14, 0x20},
//{0x24, 0x75},//AGC/AEC-稳定运行区域（上限）
//{0x25, 0x63},//AGC/AEC-稳定运行区域（下限）
//{0x26, 0xA5},//AGC/AEC快速运行区域
//AEC algorithm selection - AEC公式选择   
//{0xaa, 0x14},//基于平均值的AEC算法

{0xaa, 0x94},//基于直方图的AEC算法
//基于直方图的AGC/AEC的控制
//{0x9f, 0x78},
//{0xa0, 0x68},       
//{0xa6, 0xdf},
{0xa7, 0xff},
//{0xa8, 0xf0},
//{0xa9, 0x90},

//Fix Gain Control - 固定增益控制    
//{0x69, 0x5d},//Fix gain for Gr channel/for Gb channel/for R channel/for B channel
//Color saturation 颜色饱和度 + 0
//{0x4f, 0x80},
//{0x50, 0x80},
//{0x51, 0x00},
//{0x52, 0x22},
//{0x53, 0x5e},
//{0x54, 0x80},
//{0x58, 0x9e},

//Brightness - 亮度 + 0
{0x55, 0x00},
//Contrast - 对比度 + 0
{0x56, 0x40},
//Gamma Curve - 伽马曲线
//{0x7a, 0x20},
//{0x7b, 0x1c},
//{0x7c, 0x28},        
//{0x7d, 0x3c},
//{0x7e, 0x55},
//{0x7f, 0x68},
//{0x80, 0x76},
//{0x81, 0x80},        
//{0x82, 0x88},
//{0x83, 0x8f},
//{0x84, 0x96},
//{0x85, 0xa3},
//{0x86, 0xaf},        
//{0x87, 0xc4},
//{0x88, 0xd7},
//{0x89, 0xe8},

//Matrix Coefficient - 色彩矩阵系数            
//{0x4f, 0x80},
//{0x50, 0x80},
//{0x51, 0x00},
//{0x52, 0x22},
//{0x53, 0x5e},
//{0x54, 0x80},

//Lens Correction Option - 镜头补偿选项
//{0x62, 0x00},       
//{0x63, 0x00},
//{0x64, 0x04},
//{0x65, 0x20},        
//{0x66, 0x05},
//{0x94, 0x04},//effective only when LCC5[2] is high
//{0x95, 0x08},//effective only when LCC5[2] is high

//OV7670_config_window()配置窗口
//{0x17, 0x16},//行频Horizontal Frame开始高八位(低三位在HREF[2：0])       
//{0x18, 0x04},//行频Horizontal Frame结束高八位(低三位在HREF[5：3])
//{0x19, 0x02},//场频Vertical Frame开始高八位(低二位在VREF[1：0])
//{0x1a, 0x7b},//场频Vertical Frame结束高八位(低二位在VREF[3：2])
//{0x32, 0x80},//HREF
//{0x03, 0x06},//VREF

//Automatic black Level Compensation - 自动黑电平校正
{0xb0, 0x84},//调试时注释这项配置时，颜色显示不正常
//{0xb1, 0x40},//自动黑电平校正
//{0xb2, 0x0e},      
//{0xb3, 0x82},
//{0xb8, 0x0a},
{0x73, 0x10},//DSP缩放时钟分频控制
{0x13, 0x8F},//使能快速AGC/AEC算法
//默认PCLK一直输出
//{0x15, 0x22}//PCLK在HSYNC期间不输出,VSYNC负有效
};

//配置窗口
void OV7670_config_window(uint16_t startx,uint16_t starty,uint16_t width,uint16_t height)
{
	uint16_t endx,endy;
	uint8_t x_reg,y_reg,temp;
	endx=(startx+width*2)%784;
	endy=(starty+height*2);
	x_reg=SCCB_RD_Reg(0x32);//HREF
	x_reg&=0xC0;
	y_reg=SCCB_RD_Reg(0x03);//VREF
	y_reg&=0xF0;
	//设置HREF
	temp=x_reg|((endx&0x7)<<3)|(startx&0x7);
	SCCB_WR_Reg(0x32,temp);//HREF
	temp=(startx&0x7F8)>>3;
	SCCB_WR_Reg(0x17,temp);//HSTART
	temp=(endx&0x7F8)>>3;
	SCCB_WR_Reg(0x18,temp);//HSTOP
	//设置VREF
	temp=y_reg|((endy&0x3)<<2)|(starty&0x3);
	SCCB_WR_Reg(0x03,temp);//VREF
	temp=(starty&0x3FC)>>2;
	SCCB_WR_Reg(0x19,temp);//VSTART
	temp=(endy&0x3FC)>>2;
	SCCB_WR_Reg(0x1A,temp);//VSTOP
}

//------------------------------------------------
//初始化
//------------------------------------------------
void OV7670_Init(void)
{
	uint8_t i;
	
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);//使能GPIOA时钟
	//GPIOF9,F10初始化设置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;//RST,PWDN
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化GPIO
	
	DCMI_RESET_H();
	delay_ms(1);
	DCMI_PWDN_L();
	delay_ms(10);
	DCMI_PWDN_L();
	delay_ms(1);
	DCMI_RESET_H();	
	delay_ms(10);

	SCCB_WR_Reg(0x12, 0x80);//SCCB复位
	delay_ms(10);
	for(i=0;i<ARRLEN(OV7670_REG);i++)
	{
		SCCB_WR_Reg(OV7670_REG[i][0],OV7670_REG[i][1]);
	}
	//QVGA,RGB565,startx=180\182\184\186\188\......
	//QVGA,YUV,startx=180\181\184\185\188\189\......
	//OV7670_config_window(184,15,CAMWIDTH,CAMHEIGHT);//QVGA，startx如果不合适会改变UV顺序,或RGB图像产生噪点？	
	//QCIF,RGB565,startx=181\182\185\186\189\190\193\194\197\198\201\202\205\206
	//QCIF,YUV,startx=181\182\183\184\189\190\191\192\197\198\199\200\205\206\207\208	
	OV7670_config_window(181,15,CAMWIDTH*2,CAMHEIGHT*5/3);//QCIF，startx如果不合适会改变UV顺序,或RGB图像产生噪点？
}
