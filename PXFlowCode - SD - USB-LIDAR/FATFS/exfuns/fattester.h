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

u8 mf_mount(u8* path,u8 mt);//为磁盘注册工作区
u8 mf_open(u8*path,u8 mode);//打开路径下的文件
u8 mf_close(void);//关闭文件
u8 mf_read(u16 len);//读出数据
u8 mf_write(u8*dat,u16 len);//写入数据
u8 mf_opendir(u8* path);//打开目录
u8 mf_closedir(void);//关闭目录 
u8 mf_readdir(void);//打读取文件夹
u8 mf_scan_files(u8 * path);//遍历文件
u32 mf_showfree(u8 *drv);//显示剩余容量
u8 mf_lseek(u32 offset);//文件读写指针偏移
u32 mf_tell(void);//读取文件当前读写指针的位置.
u32 mf_size(void);//读取文件大小
u8 mf_mkdir(u8*pname);//创建目录
u8 mf_fmkfs(u8* path,u8 mode,u16 au);//格式化
u8 mf_unlink(u8 *pname);//删除文件/目录
u8 mf_rename(u8 *oldname,u8* newname);//修改文件/目录名字(如果目录不同,还可以移动文件哦!)
void mf_getlabel(u8 *path);//获取盘符（磁盘名字）
void mf_setlabel(u8 *path);//设置盘符（磁盘名字） 
void mf_gets(u16 size);//从文件里面读取一段字符串
u8 mf_putc(u8 c);//写一个字符到文件
u8 mf_puts(u8*c);//写字符串到文件

#endif





























