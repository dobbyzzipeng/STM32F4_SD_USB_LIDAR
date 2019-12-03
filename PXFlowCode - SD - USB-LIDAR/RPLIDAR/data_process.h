#ifndef __data_process_h__
#define __data_process_h__
#include "stm32f4xx_rcc.h"

#define data_mode 0x02
extern u8 DATA_medo;

extern u16 min_xian_dis ;//��С�߾�
extern u16 min_distance;//��С��������
extern u16 min_xian_angle;//��С�߾�ʱ�Ƕ�
extern u16 Xian_shu_count,Xian_send_count;
extern u16 location_shuzhang_dis,location_shuzhang_X,location_shuzhang_Y;
extern float bat_vol;
extern u16 from_xian_dis;
void Xian_shu_data_record(void);
void Send_data_dis(void);
void Send_data_end(void);
void laser_data_analysis(uint16_t rxlength);
#endif
