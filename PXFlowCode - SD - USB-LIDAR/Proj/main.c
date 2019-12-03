#include "main.h"
//2018-12-13 rebuild the prj
//2019-01-29 fix tf card reader bug. 
//2019-11-21 add rplidar and gps_rtk support. 

#define USE_IAP 0
USB_OTG_CORE_HANDLE USB_OTG_dev;
extern vu8 USB_STATUS_REG;		//USB״̬
extern vu8 bDeviceState;		//USB���� ���

float bat_vol = 0;
uint64_t g_utc_time = 0,g_utc_month = 0;
void usb_task_process(void);
void lidar_data_process(void);
int main(void)
{
	uint8_t iwdog_flag = 0,micro_sec_cnt = 0;
	int csv_index = 0;
	uint64_t data_time_cnt = 0;
	double data_time = 0;
	#if USE_IAP
	NVIC_SetVectorTable(0X08000000,0X20000);
	INTX_ENABLE();
	__enable_irq();
	delay_solft_ms(1000);
	
	RCC_DeInit();//Ϊ�˴�IAP������������APP��ʱ��������ʼ����ʼ��ʱҪ�ָ�RCCΪ��λ״̬���ָ�NVICΪ��                                                                 //λ״̬��
	SystemInit();  
	NVIC_SetVectorTable(0X08000000,0X20000);
	#endif
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
	delay_init(168);
	bsp_InitUart();
	LED_Init();
	BAT_LED_GPIO_Config();
	RF_GPIO_Config();
	Data_List_Init();
	usart2_init(230400);
	GPS_Init();
	my_mem_init(SRAMIN);//��ʼ���ڲ��ڴ�� 
	my_mem_init(SRAMCCM);//��ʼ��CCM�ڴ��,only CPU can read/write
	while(SD_Init())//��ⲻ��SD��
	{
		u1_printf("SD Card Error! Please check!\r\n");
		GREEN_TOGGLE();
		BAT_LED1_TOGGLE();
		delay_ms(100);
	}
	show_sdcard_info();	//��ӡSD�������Ϣ
	exfuns_init();		//Ϊfatfs��ر��������ڴ�				 
  	f_mount(fs[0],"0:",1); 	//����TF��
	delay_ms(10);
	f_setlabel((const TCHAR *)"0:JZISD");	//���ô��̵�����    
	USBD_Init(&USB_OTG_dev,USB_OTG_FS_CORE_ID,&USR_desc,&USBD_MSC_cb,&USR_cb);
	delay_ms(10);
	TIM1_Init(10000-1,840-1);//50ms
	while(1)
	{
		if(sys_timer_flag)
		{
			data_time_cnt++;
			micro_sec_cnt++;
			data_time = data_time_cnt*0.05f;//s
			lidar_data_process();
			usb_task_process();
			atk_gps_flag = gps_data_analysis();
utc_time_combination(gpsx.utc.hour,gpsx.utc.min,gpsx.utc.sec,micro_sec_cnt/2,&g_utc_time);
utc_monthyear_combination(gpsx.utc.year,gpsx.utc.month,gpsx.utc.date,&g_utc_month);
			if(micro_sec_cnt>=19){micro_sec_cnt = 0;}
			sys_timer_flag = 0;
		}
		if(DATA_medo<2)
		{
			if(usart2_rx_data_flag)
			{
				laser_data_analysis(2960);
				if(DATA_medo==1&&atk_gps_flag)
				{
					if(location_shuzhang_X>0 && location_shuzhang_Y>0){
						
data_transfor_csv_packet(csv_index,location_shuzhang_X,location_shuzhang_Y,lon_atk,lat_atk,data_time,&datalist);
//						if(iwdog_flag == 1){ IWDG_Feed(); }
//						else{ IWDG_Init(2,2048); iwdog_flag = 1; }
						BAT_LED3_TOGGLE();
						csv_index++;
					}
				}
				usart2_rx_data_flag = 0;
			}
		}
	}
}

void lidar_data_process(void)
{
	if(DATA_medo==2)
	{
		if(Xian_send_count*10>Xian_shu_count){
			Send_data_end();
		}
		else{
			Send_data_dis();
			Xian_send_count++;
		}
	}
}

void usb_task_process(void)
{
	static uint8_t offline_cnt=0,tct = 0,led_cnt = 0,USB_STA = 0,Divece_STA = 0;
	if(USB_STA!=USB_STATUS_REG)//״̬�ı��� 
	{
		USB_STA=USB_STATUS_REG;//��¼����״̬
	}
	if(Divece_STA!=bDeviceState) 
	{
		Divece_STA=bDeviceState;
	}
	
	led_cnt++;
	if(led_cnt>=5){
		led_cnt = 0;
		if(!bDeviceState){
			GREEN_TOGGLE();//������ʾUSB�γ�
			BAT_LED1_TOGGLE();
		}
	}
	tct++;
	if(tct>=10)
	{
		tct=0;
		if(USB_STATUS_REG&0x10)
		{
			GREEN_TOGGLE();//������ʾUSB���ӳɹ�
			BAT_LED1_TOGGLE();
			offline_cnt=0;//USB������,�����offline������
			bDeviceState=1;
		}
		else//û�еõ���ѯ 
		{
			offline_cnt++;  
			if(offline_cnt>2)
			{
				bDeviceState=0;//2s��û�յ����߱��,����USB���γ���
			}
		}
		USB_STATUS_REG=0;
	}
}


