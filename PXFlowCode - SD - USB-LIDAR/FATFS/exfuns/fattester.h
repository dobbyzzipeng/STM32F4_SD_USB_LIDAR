#ifndef __FATTESTER_H
#define __FATTESTER_H 			   
#include <stm32f4xx.h>	   
#include "ff.h"

#define USER_FLASH_IAP_ADDRESS 0x08040000

extern uint8_t    filedatabuf[2048];
extern UINT   readcount;
extern UINT   writecount;

typedef struct
{
	char INDEX [10];
	char dot1;
	char SHU_X[12];
	char dot2;
	char SHU_Y[12];
	char dot3;
	char LNG[12];
	char dot4;
	char LAT[12];
	char dot5;
	char TIME[10];
	char dot6;
	char _0XD;
	char _0XA;
}DATA;

typedef union
{
	DATA data;
	char buf[sizeof(DATA)];
}DATALOG;
extern DATALOG datalog;

#define PACKET_NUM	10
typedef union
{
	DATA data[PACKET_NUM];
	char buf[sizeof(DATA)*PACKET_NUM];
}DATALOGLIST;
extern DATALOGLIST datalist;

uint8_t File_IAP(void);
uint8_t File_BIN(void);
uint8_t File_CSV(void);
uint8_t File_CSV_Write(void);
uint8_t File_CSV_Read(void);
uint8_t File_CSV_Write_SD(DATALOG *pdatalog);
char num_to_ascii(uint8_t num);
void data_transfor_csv(int index,float shu_x,float shu_y,double lng,double lat,double time,DATALOG *pdatalog);
void weishu_chaifen(int num,uint8_t *p_ge,uint8_t *p_shi,uint8_t *p_bai,uint8_t *p_qian,uint8_t *p_wan,\
uint8_t *p_shiwan,uint8_t *p_baiwan,uint8_t *p_qianwan,uint8_t *p_qianwanplus);
void data_transfor_csv_packet(int index,float shu_x,float shu_y,double lng,double lat,double time,DATALOGLIST *pdatalist);
uint8_t File_CSV_Write_SD_Packet(DATALOGLIST *pdatalist);
void Data_List_Init(void);

u8 mf_mount(u8* path,u8 mt);//Ϊ����ע�Ṥ����
u8 mf_open(u8*path,u8 mode);//��·���µ��ļ�
u8 mf_close(void);//�ر��ļ�
u8 mf_read(u16 len);//��������
u8 mf_write(u8*dat,u16 len);//д������
u8 mf_opendir(u8* path);//��Ŀ¼
u8 mf_closedir(void);//�ر�Ŀ¼ 
u8 mf_readdir(void);//���ȡ�ļ���
u8 mf_scan_files(u8 * path);//�����ļ�
u32 mf_showfree(u8 *drv);//��ʾʣ������
u8 mf_lseek(u32 offset);//�ļ���дָ��ƫ��
u32 mf_tell(void);//��ȡ�ļ���ǰ��дָ���λ��.
u32 mf_size(void);//��ȡ�ļ���С
u8 mf_mkdir(u8*pname);//����Ŀ¼
u8 mf_fmkfs(u8* path,u8 mode,u16 au);//��ʽ��
u8 mf_unlink(u8 *pname);//ɾ���ļ�/Ŀ¼
u8 mf_rename(u8 *oldname,u8* newname);//�޸��ļ�/Ŀ¼����(���Ŀ¼��ͬ,�������ƶ��ļ�Ŷ!)
void mf_getlabel(u8 *path);//��ȡ�̷����������֣�
void mf_setlabel(u8 *path);//�����̷����������֣� 
void mf_gets(u16 size);//���ļ������ȡһ���ַ���
u8 mf_putc(u8 c);//дһ���ַ����ļ�
u8 mf_puts(u8*c);//д�ַ������ļ�

#endif





























