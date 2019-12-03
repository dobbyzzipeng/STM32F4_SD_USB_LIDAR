#include "ov7670.h"
#include "delay.h"
#include "sccb.h"
/*------------------------------------------------
ע�⣺
{0x3a, 0x11},�����ڸı�ֱ��ʺ��Զ����ô���
������OV7670_config_window��QCIFĬ������174*144������176*144
OV7670_config_window��startx��������ʻ�ı�UV˳��,��RGBͼ��������
��180���ϵ�ֵ������
YUVתRGB�ļ��㷨��
	B=Y+U+16;//���Բ�+16
	G=Y-(U+V)/4;
	R=Y+V+16;
	(RGB��������0~255�ķ�Χ)
δʹ��С����������ɫ����ƫ��
	
--------------------------------------------------*/
//����ͷ�Ŀ����
#define CAMWIDTH	176
#define CAMHEIGHT	144
//#define CAMWIDTH	160
//#define CAMHEIGHT	120

#define RST_H	IOSET(GPIOE->ODR,BIT(3))
#define RST_L	IOCLR(GPIOE->ODR,BIT(3))

#define PWDN_H	IOSET(GPIOE->ODR,BIT(0))
#define PWDN_L	IOCLR(GPIOE->ODR,BIT(0))

const uint8_t OV7670_REG[][2]={
//{0x11, 0x40},//�ڲ�ʱ�Ӳ���Ƶ
//{0x6b, 0x00},//��PLL,��LDO//д������ѹ����΢����
//{0x2a, 0x00},//������
//{0x2b, 0x00},
//{0x92, 0x00},//����
//{0x93, 0x00},
//{0x3b, 0x0a},//�ر�ҹ��ģʽ
//Output format
//{0x12, 0x10},//QVGA(320*240),YUV
{0x12, 0x08},//QCIF(176��144),YUV
//-----------RGB565���������,YUV����Ҫ40��8c�Ĵ���ΪĬ��ֵ
//{0x12, 0x14},//QVGA(320*240),RAW
//{0x12, 0x0C},//QCIF(176��144),RAW
//{0x40, 0xD0},//RGB565
//{0x8c, 0x00},//RGB444�ر�

//Special effects - ��Ч 
//normal
{0x3a, 0x04},//YUYV,�Զ�UV
//{0x3a, 0x14},//YUYV,�ֶ�UV,�����ڲ���8λ���ݵ���ȷ��
//{0x3a, 0x11},//YUYV,�ֶ�UV,�Զ�����
//{0x3d, 0x88},//YUYV
//{0x67, 0x10},//�ֶ�U
//{0x68, 0x15},//�ֶ�V
//Mirror/VFlip Enable - ˮƽ����/��ֱ��תʹ��
//{0x1e, 0x01},//�޸�����ֵ������ͼ����ʾ���»����ҵߵ�
//Banding Filter Setting for 24Mhz Input Clock - �����˲���
//30fps for 60Hz light frequency
//{0x13, 0xe7},//banding filer enable
//{0x9d, 0x98},//50Hz banding filer
//{0x9e, 0x7f},//60Hz banding filer
//{0xa5, 0x02},//3 step for 50Hz
//{0xab, 0x03},//4 step for 60Hz
//{0x3b, 0x02},//select 60Hz banding filer
//Simple White Balance - ��ƽ��
//{0x13, 0xe7},//AWB��AGC��AGC Enable and ...
//{0x6f, 0x9f},//simple AWB
//AWBC - �Զ���ƽ�����(Automatic white balance control) 
//{0x43, 0x14},//�û��ֲ�����Щ�Ĵ�����ֵ���Ǳ���(Reserved),�������õ�ѽ��
//{0x44, 0xf0},
//{0x45, 0x34},        
//{0x46, 0x58},
//{0x47, 0x28},
//{0x48, 0x3a},

//AWB Control
//{0x59, 0x88},//�û��ֲ����Ĵ������Ǳ�������ʼֵ��û�ṩ
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

//AGC/AEC - Automatic Gain Control�Զ����油��/Automatic exposure Control�Զ��ع����
//{0x00, 0x00},
//{0x14, 0x20},
//{0x24, 0x75},//AGC/AEC-�ȶ������������ޣ�
//{0x25, 0x63},//AGC/AEC-�ȶ������������ޣ�
//{0x26, 0xA5},//AGC/AEC������������
//AEC algorithm selection - AEC��ʽѡ��   
//{0xaa, 0x14},//����ƽ��ֵ��AEC�㷨

{0xaa, 0x94},//����ֱ��ͼ��AEC�㷨
//����ֱ��ͼ��AGC/AEC�Ŀ���
//{0x9f, 0x78},
//{0xa0, 0x68},       
//{0xa6, 0xdf},
{0xa7, 0xff},
//{0xa8, 0xf0},
//{0xa9, 0x90},

//Fix Gain Control - �̶��������    
//{0x69, 0x5d},//Fix gain for Gr channel/for Gb channel/for R channel/for B channel
//Color saturation ��ɫ���Ͷ� + 0
//{0x4f, 0x80},
//{0x50, 0x80},
//{0x51, 0x00},
//{0x52, 0x22},
//{0x53, 0x5e},
//{0x54, 0x80},
//{0x58, 0x9e},

//Brightness - ���� + 0
{0x55, 0x00},
//Contrast - �Աȶ� + 0
{0x56, 0x40},
//Gamma Curve - ٤������
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

//Matrix Coefficient - ɫ�ʾ���ϵ��            
//{0x4f, 0x80},
//{0x50, 0x80},
//{0x51, 0x00},
//{0x52, 0x22},
//{0x53, 0x5e},
//{0x54, 0x80},

//Lens Correction Option - ��ͷ����ѡ��
//{0x62, 0x00},       
//{0x63, 0x00},
//{0x64, 0x04},
//{0x65, 0x20},        
//{0x66, 0x05},
//{0x94, 0x04},//effective only when LCC5[2] is high
//{0x95, 0x08},//effective only when LCC5[2] is high

//OV7670_config_window()���ô���
//{0x17, 0x16},//��ƵHorizontal Frame��ʼ�߰�λ(����λ��HREF[2��0])       
//{0x18, 0x04},//��ƵHorizontal Frame�����߰�λ(����λ��HREF[5��3])
//{0x19, 0x02},//��ƵVertical Frame��ʼ�߰�λ(�Ͷ�λ��VREF[1��0])
//{0x1a, 0x7b},//��ƵVertical Frame�����߰�λ(�Ͷ�λ��VREF[3��2])
//{0x32, 0x80},//HREF
//{0x03, 0x06},//VREF

//Automatic black Level Compensation - �Զ��ڵ�ƽУ��
{0xb0, 0x84},//����ʱע����������ʱ����ɫ��ʾ������
//{0xb1, 0x40},//�Զ��ڵ�ƽУ��
//{0xb2, 0x0e},      
//{0xb3, 0x82},
//{0xb8, 0x0a},
{0x73, 0x10},//DSP����ʱ�ӷ�Ƶ����
{0x13, 0x8F},//ʹ�ܿ���AGC/AEC�㷨
//Ĭ��PCLKһֱ���
//{0x15, 0x22}//PCLK��HSYNC�ڼ䲻���,VSYNC����Ч
};

//���ô���
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
	//����HREF
	temp=x_reg|((endx&0x7)<<3)|(startx&0x7);
	SCCB_WR_Reg(0x32,temp);//HREF
	temp=(startx&0x7F8)>>3;
	SCCB_WR_Reg(0x17,temp);//HSTART
	temp=(endx&0x7F8)>>3;
	SCCB_WR_Reg(0x18,temp);//HSTOP
	//����VREF
	temp=y_reg|((endy&0x3)<<2)|(starty&0x3);
	SCCB_WR_Reg(0x03,temp);//VREF
	temp=(starty&0x3FC)>>2;
	SCCB_WR_Reg(0x19,temp);//VSTART
	temp=(endy&0x3FC)>>2;
	SCCB_WR_Reg(0x1A,temp);//VSTOP
}

//------------------------------------------------
//��ʼ��
//------------------------------------------------
void OV7670_Init(void)
{
	uint8_t i;
	
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);//ʹ��GPIOAʱ��
	//GPIOF9,F10��ʼ������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;//RST,PWDN
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
	GPIO_Init(GPIOD, &GPIO_InitStructure);//��ʼ��GPIO
	
	DCMI_RESET_H();
	delay_ms(1);
	DCMI_PWDN_L();
	delay_ms(10);
	DCMI_PWDN_L();
	delay_ms(1);
	DCMI_RESET_H();	
	delay_ms(10);

	SCCB_WR_Reg(0x12, 0x80);//SCCB��λ
	delay_ms(10);
	for(i=0;i<ARRLEN(OV7670_REG);i++)
	{
		SCCB_WR_Reg(OV7670_REG[i][0],OV7670_REG[i][1]);
	}
	//QVGA,RGB565,startx=180\182\184\186\188\......
	//QVGA,YUV,startx=180\181\184\185\188\189\......
	//OV7670_config_window(184,15,CAMWIDTH,CAMHEIGHT);//QVGA��startx��������ʻ�ı�UV˳��,��RGBͼ�������㣿	
	//QCIF,RGB565,startx=181\182\185\186\189\190\193\194\197\198\201\202\205\206
	//QCIF,YUV,startx=181\182\183\184\189\190\191\192\197\198\199\200\205\206\207\208	
	OV7670_config_window(181,15,CAMWIDTH*2,CAMHEIGHT*5/3);//QCIF��startx��������ʻ�ı�UV˳��,��RGBͼ�������㣿
}
