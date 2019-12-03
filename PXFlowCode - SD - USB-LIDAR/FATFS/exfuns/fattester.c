#include "fattester.h"	 
#include "sdio_sdcard.h"
//#include "usmart.h"
#include "bsp_usart.h"
#include "exfuns.h"
#include "malloc.h"		  
#include "ff.h"
#include "string.h"

//为磁盘注册工作区	 
//path:磁盘路径，比如"0:"、"1:"
//mt:0，不立即注册（稍后注册）；1，立即注册
//返回值:执行结果
uint8_t mf_mount(uint8_t* path,uint8_t mt)
{
	return f_mount(fs[0],(const TCHAR*)path,mt);//默认安装0号磁盘，即系SD卡
}
//打开路径下的文件
//path:路径+文件名
//mode:打开模式
//返回值:执行结果
uint8_t mf_open(uint8_t*path,uint8_t mode)
{
	uint8_t res;	 
	res=f_open(file,(const TCHAR*)path,mode);//打开文件夹
	return res;
} 
//关闭文件
//返回值:执行结果
uint8_t mf_close(void)
{
	f_close(file);
	return 0;
}
//读出数据
//len:读出的长度
//返回值:执行结果
uint8_t mf_read(u16 len)
{
	u16 i,t;
	uint8_t res=0;
	u16 tlen=0;
	u1_printf("\r\nRead file data is:\r\n");
	for(i=0;i<len/512;i++)
	{
		res=f_read(file,fatbuf,512,&br);
		if(res)
		{
			u1_printf("Read Error:%d\r\n",res);
			break;
		}
		else
		{
			tlen+=br;
			for(t=0;t<br;t++)u1_printf("%c",fatbuf[t]); 
		}
	}
	if(len%512)
	{
		res=f_read(file,fatbuf,len%512,&br);
		if(res)	//读数据出错了
		{
			u1_printf("\r\nRead Error:%d\r\n",res);   
		}else
		{
			tlen+=br;
			for(t=0;t<br;t++)u1_printf("%c",fatbuf[t]); 
		}	 
	}
	if(tlen)u1_printf("\r\nReaded data len:%d\r\n",tlen);//读到的数据长度
	u1_printf("Read data over\r\n");	 
	return res;
}
//写入数据
//dat:数据缓存区
//len:写入长度
//返回值:执行结果
uint8_t mf_write(uint8_t*dat,u16 len)
{
	uint8_t res;	   					   

	u1_printf("\r\nBegin Write file...\r\n");
	u1_printf("Write data len:%d\r\n",len);	 
	res=f_write(file,dat,len,&bw);
	if(res)
	{
		u1_printf("Write Error:%d\r\n",res);   
	}else u1_printf("Writed data len:%d\r\n",bw);
	u1_printf("Write data over.\r\n");
	return res;
}

//打开目录
 //path:路径
//返回值:执行结果
uint8_t mf_opendir(uint8_t* path)
{
	return f_opendir(&dir,(const TCHAR*)path);	
}
//关闭目录 
//返回值:执行结果
uint8_t mf_closedir(void)
{
	return f_closedir(&dir);	
}
//打读取文件夹
//返回值:执行结果
uint8_t mf_readdir(void)
{
	uint8_t res;
	char *fn;			 
#if _USE_LFN
 	fileinfo.lfsize = _MAX_LFN * 2 + 1;
	fileinfo.lfname = mymalloc(SRAMIN,fileinfo.lfsize);
#endif		  
	res=f_readdir(&dir,&fileinfo);//读取一个文件的信息
	if(res!=FR_OK||fileinfo.fname[0]==0)
	{
		myfree(SRAMIN,fileinfo.lfname);
		return res;//读完了.
	}
#if _USE_LFN
	fn=*fileinfo.lfname ? fileinfo.lfname : fileinfo.fname;
#else
	fn=fileinfo.fname;;
#endif	
	u1_printf("\r\n DIR info:\r\n");

	u1_printf("dir.id:%d\r\n",dir.id);
	u1_printf("dir.index:%d\r\n",dir.index);
	u1_printf("dir.sclust:%d\r\n",dir.sclust);
	u1_printf("dir.clust:%d\r\n",dir.clust);
	u1_printf("dir.sect:%d\r\n",dir.sect);	  

	u1_printf("\r\n");
	u1_printf("File Name is:%s\r\n",fn);
	u1_printf("File Size is:%d\r\n",fileinfo.fsize);
	u1_printf("File data is:%d\r\n",fileinfo.fdate);
	u1_printf("File time is:%d\r\n",fileinfo.ftime);
	u1_printf("File Attr is:%d\r\n",fileinfo.fattrib);
	u1_printf("\r\n");
	myfree(SRAMIN,fileinfo.lfname);
	return 0;
}			 

 //遍历文件
 //path:路径
 //返回值:执行结果
uint8_t mf_scan_files(uint8_t * path)
{
	FRESULT res;	  
    char *fn;   /* This function is assuming non-Unicode cfg. */
#if _USE_LFN
 	fileinfo.lfsize = _MAX_LFN * 2 + 1;
	fileinfo.lfname = mymalloc(SRAMIN,fileinfo.lfsize);
#endif		  

    res = f_opendir(&dir,(const TCHAR*)path); //打开一个目录
    if (res == FR_OK) 
	{	
		u1_printf("\r\n"); 
		while(1)
		{
	        res = f_readdir(&dir, &fileinfo);                   //读取目录下的一个文件
	        if (res != FR_OK || fileinfo.fname[0] == 0) break;  //错误了/到末尾了,退出
	        //if (fileinfo.fname[0] == '.') continue;             //忽略上级目录
#if _USE_LFN
        	fn = *fileinfo.lfname ? fileinfo.lfname : fileinfo.fname;
#else							   
        	fn = fileinfo.fname;
#endif	                                              /* It is a file. */
			u1_printf("%s/", path);//打印路径	
			u1_printf("%s\r\n",  fn);//打印文件名	  
		} 
    }	  
	myfree(SRAMIN,fileinfo.lfname);
    return res;	  
}
//显示剩余容量
//drv:盘符
//返回值:剩余容量(字节)
u32 mf_showfree(uint8_t *drv)
{
	FATFS *fs1;
	uint8_t res;
    u32 fre_clust=0, fre_sect=0, tot_sect=0;
    //得到磁盘信息及空闲簇数量
    res = f_getfree((const TCHAR*)drv,(DWORD*)&fre_clust, &fs1);
    if(res==0)
	{											   
	    tot_sect = (fs1->n_fatent - 2) * fs1->csize;//得到总扇区数
	    fre_sect = fre_clust * fs1->csize;			//得到空闲扇区数	   
#if _MAX_SS!=512
		tot_sect*=fs1->ssize/512;
		fre_sect*=fs1->ssize/512;
#endif	  
		if(tot_sect<20480)//总容量小于10M
		{
		    /* Print free space in unit of KB (assuming 512 bytes/sector) */
		    u1_printf("\r\n磁盘总容量:%d KB\r\n"
		           "可用空间:%d KB\r\n",
		           tot_sect>>1,fre_sect>>1);
		}else
		{
		    /* Print free space in unit of KB (assuming 512 bytes/sector) */
		    u1_printf("\r\n磁盘总容量:%d MB\r\n"
		           "可用空间:%d MB\r\n",
		           tot_sect>>11,fre_sect>>11);
		}
	}
	return fre_sect;
}		    
//文件读写指针偏移
//offset:相对首地址的偏移量
//返回值:执行结果.
uint8_t mf_lseek(u32 offset)
{
	return f_lseek(file,offset);
}
//读取文件当前读写指针的位置.
//返回值:位置
u32 mf_tell(void)
{
	return f_tell(file);
}
//读取文件大小
//返回值:文件大小
u32 mf_size(void)
{
	return f_size(file);
} 
//创建目录
//pname:目录路径+名字
//返回值:执行结果
uint8_t mf_mkdir(uint8_t*pname)
{
	return f_mkdir((const TCHAR *)pname);
}
//格式化
//path:磁盘路径，比如"0:"、"1:"
//mode:模式
//au:簇大小
//返回值:执行结果
uint8_t mf_fmkfs(uint8_t* path,uint8_t mode,u16 au)
{
	return f_mkfs((const TCHAR*)path,mode,au);//格式化,drv:盘符;mode:模式;au:簇大小
} 
//删除文件/目录
//pname:文件/目录路径+名字
//返回值:执行结果
uint8_t mf_unlink(uint8_t *pname)
{
	return  f_unlink((const TCHAR *)pname);
}

//修改文件/目录名字(如果目录不同,还可以移动文件哦!)
//oldname:之前的名字
//newname:新名字
//返回值:执行结果
uint8_t mf_rename(uint8_t *oldname,uint8_t* newname)
{
	return  f_rename((const TCHAR *)oldname,(const TCHAR *)newname);
}
//获取盘符（磁盘名字）
//path:磁盘路径，比如"0:"、"1:"  
void mf_getlabel(uint8_t *path)
{
	uint8_t buf[20];
	u32 sn=0;
	uint8_t res;
	res=f_getlabel ((const TCHAR *)path,(TCHAR *)buf,(DWORD*)&sn);
	if(res==FR_OK)
	{
		u1_printf("\r\n磁盘%s 的盘符为:%s\r\n",path,buf);
		u1_printf("磁盘%s 的序列号:%X\r\n\r\n",path,sn); 
	}else u1_printf("\r\n获取失败，错误码:%X\r\n",res);
}
//设置盘符（磁盘名字），最长11个字符！！，支持数字和大写字母组合以及汉字等
//path:磁盘号+名字，比如"0:ALIENTEK"、"1:OPENEDV"  
void mf_setlabel(uint8_t *path)
{
	uint8_t res;
	res=f_setlabel ((const TCHAR *)path);
	if(res==FR_OK)
	{
		u1_printf("\r\n磁盘盘符设置成功:%s\r\n",path);
	}else u1_printf("\r\n磁盘盘符设置失败，错误码:%X\r\n",res);
} 

//从文件里面读取一段字符串
//size:要读取的长度
void mf_gets(u16 size)
{
 	TCHAR* rbuf;
	rbuf=f_gets((TCHAR*)fatbuf,size,file);
	if(*rbuf==0)return  ;//没有数据读到
	else
	{
		u1_printf("\r\nThe String Readed Is:%s\r\n",rbuf);  	  
	}			    	
}
//需要_USE_STRFUNC>=1
//写一个字符到文件
//c:要写入的字符
//返回值:执行结果
uint8_t mf_putc(uint8_t c)
{
	return f_putc((TCHAR)c,file);
}
//写字符串到文件
//c:要写入的字符串
//返回值:写入的字符串长度
uint8_t mf_puts(uint8_t*c)
{
	return f_puts((TCHAR*)c,file);
}


uint8_t    filedatabuf[2048] = {0};
UINT   readcount = 0;
/**
* @brief  Explore_Disk 
*         Displays disk content
* @param  path: pointer to root path
* @retval None
*/

uint8_t File_IAP(void)
{
  FRESULT res;
	void (*pUserApp)(void);
  uint32_t JumpAddress;
	u16 i=0;
	
	#if 1
	res=f_open(file, "0:ROM.BIN",FA_OPEN_EXISTING | FA_READ);
	if(res!=FR_OK)
	{
		u1_printf("f_open->ROM.bin->error.\n");
		goto updateerror;
	}
	u1_printf("f_open->ROM.bin->OK!.\n");
	
	u1_printf(">Erase.....ing\n");
	u1_printf(">Erase.........ok\n");
	u1_printf("f_read->ROM.bin->..ing.\n");
	while(1)
	{
		res=f_read(file,filedatabuf,2048, &readcount);
		if(res!=FR_OK)
		{
			u1_printf("f_read->ROM.bin->error.\n");
			goto updateerror;
		}
//		 res=IAP_Flash_Write(USER_FLASH_IAP_ADDRESS+i*2048,filedatabuf,readcount/4);
		 if(0!=res) 
		 {
			 u1_printf("Flash.........error\n");
			 goto updateerror;
		 } 
		 i++;
		 u1_printf("ffok...............%dkbyte\n",i*2);
		 if(readcount<2048) 
		 {
			u1_printf("ffok............COMPLETE\n");
			goto updateerror;
		 }
  }
	#endif
	  #if 1
	      updateerror:
	      __asm("CPSID  I");
        
		JumpAddress = *(volatile uint32_t*) (USER_FLASH_IAP_ADDRESS+4);
		pUserApp = (void (*)(void)) JumpAddress;
		/* Initialize user application's Stack Pointer */
		__set_PSP(*(volatile uint32_t*) USER_FLASH_IAP_ADDRESS);
		__set_CONTROL(0);
		__set_MSP(*(volatile uint32_t*) USER_FLASH_IAP_ADDRESS);
		pUserApp();
		/*************************Jump to User Application area end***************************/
	 #endif
	
return 0;
}

uint8_t File_BIN(void)
{
	FRESULT res;
	u16 i=0;
	
	#if 1
	res=f_open(file, "0:Firmware.bin",FA_OPEN_EXISTING | FA_READ);
	if(res!=FR_OK)
	{
		u1_printf("f_open->Firmware.bin->error.\r\n");
		return 0;
	}
	u1_printf("f_open->Firmware.bin->OK!.\r\n");
	res=f_read(file,filedatabuf,1024, &readcount);
	if(res!=FR_OK)
	{
		u1_printf("f_read->Firmware.bin->error.\r\n");
		return 0;
	}
	for(i=0;i<1024;i++)
	{
		u1_printf("%x ",filedatabuf[i]);
		if(i%20==0)
		{
			u1_printf("\r\n");
		}
	}
//	while(1)
//	{
//		res=f_read(file,filedatabuf,2048, &readcount);
//		if(res!=FR_OK)
//		{
//			u1_printf("f_read->Firmware.bin->error.\r\n");
//			return 0;
//		}
//		 i++;
//		 u1_printf("ffok...............%dkbyte\n",i*2);
//		 if(readcount<2048) 
//		 {
//			u1_printf("ffok............COMPLETE\n");
//			return 0;
//		 }
//  }
	#endif
return 0;
}

uint8_t File_CSV(void)
{
	FRESULT res;
	u16 i=0;
	
	#if 1
	res=f_open(file, "0:datalog.CSV",FA_OPEN_EXISTING | FA_READ);
	if(res!=FR_OK)
	{
		u1_printf("f_open->datalog.CSV->error.\r\n");
		return 0;
	}
	u1_printf("f_open->datalog.CSV->OK!.\r\n");
	res=f_read(file,filedatabuf,100, &readcount);
	if(res!=FR_OK)
	{
		u1_printf("f_read->datalog.CSV->error.\r\n");
		return 0;
	}
	for(i=0;i<100;i++)
	{
		u1_printf("%x ",filedatabuf[i]);
		if(i%20==0)
		{
			u1_printf("\r\n");
		}
	}
	#endif
return 0;
}

#define USE_FATFS_DBUG 0
UINT   writecount = 0;
uint8_t fw_databuf[1024] = {0};
DATALOG datahead = {"INDEX",',',"SHU_X",',',"SHU_Y",',',"LNG",',',"LAT",',',"TIME",',',0X0D,0X0A};
//DATALOG datalog = {" ",','," ",','," ",','," ",','," ",','," ",',',0X0D,0X0A};
uint8_t File_CSV_Write_SD(DATALOG *pdatalog)
{
	FRESULT res;
	char filename[] = {"0:datalog.CSV"};
	static uint8_t rw_flag = 0;
	
	if(rw_flag==0)
	{
		res=f_open(file,"0:datalog.CSV",FA_OPEN_EXISTING | FA_READ);
		res=f_read(file,filedatabuf,2048, &readcount);
		if(filedatabuf[0]==0X0D||filedatabuf[1]==0X0A||filedatabuf[2]==0||filedatabuf[3]==0){
			rw_flag = 1;//空白CSV文件
		}
		else{
			rw_flag = 2;//已经记录过的CSV文件
		}
		mf_close();
	}
	
	res=f_open(file,"0:datalog.CSV",FA_OPEN_EXISTING | FA_WRITE);
	if(res!=FR_OK)
	{
		#if USE_FATFS_DBUG
		u1_printf("f_open->datalog.CSV->error.\r\n");
		#endif
		mf_mkdir((uint8_t *)filename);
		res=f_open(file,"0:datalog.CSV",FA_CREATE_NEW | FA_WRITE);
	}
	mf_lseek(mf_size());//定位读写位置
	if(rw_flag==1){
		res=f_write(file,datahead.buf,sizeof(DATALOG), &writecount);
		rw_flag = 2;
	}
	else if(rw_flag==2){
		res=f_write(file,pdatalog->buf,sizeof(DATALOG), &writecount);
	}
	mf_close();
return 0;
}

DATALOGLIST datalist = {0};
void Data_List_Init(void)
{
	uint8_t i = 0;
	for(i = 0;i<PACKET_NUM;i++)
	{
		datalist.data[i].dot1 = ',';
		datalist.data[i].dot2 = ',';
		datalist.data[i].dot3 = ',';
		datalist.data[i].dot4 = ',';
		datalist.data[i].dot5 = ',';
		datalist.data[i].dot6 = ',';
		datalist.data[i]._0XD = 0X0D;
		datalist.data[i]._0XA = 0X0A;
	}
}

uint8_t File_CSV_Write_SD_Packet(DATALOGLIST *pdatalist)
{
	FRESULT res;
	char filename[] = {"0:datalog.CSV"};
	static uint8_t rw_flag = 0;
	
	if(rw_flag==0)
	{
		res=f_open(file,"0:datalog.CSV",FA_OPEN_EXISTING | FA_READ);
		res=f_read(file,filedatabuf,2048, &readcount);
		if(filedatabuf[0]==0X0D||filedatabuf[1]==0X0A||filedatabuf[2]==0||filedatabuf[3]==0){
			rw_flag = 1;//空白CSV文件
		}
		else{
			rw_flag = 2;//已经记录过的CSV文件
		}
		mf_close();
	}
	
	res=f_open(file,"0:datalog.CSV",FA_OPEN_EXISTING | FA_WRITE);
	if(res!=FR_OK)
	{
		#if USE_FATFS_DBUG
		u1_printf("f_open->datalog.CSV->error.\r\n");
		#endif
		mf_mkdir((uint8_t *)filename);
		res=f_open(file,"0:datalog.CSV",FA_CREATE_NEW | FA_WRITE);
	}
	mf_lseek(mf_size());//定位读写位置
	if(rw_flag==1){
		res=f_write(file,datahead.buf,sizeof(DATALOG), &writecount);
		rw_flag = 2;
	}
	if(rw_flag==2){
		res=f_write(file,pdatalist->buf,sizeof(DATALOGLIST), &writecount);
	}
	mf_close();
return 0;
}

char num_to_ascii(uint8_t num)
{
	char res = 0;
	switch(num)
	{
		case 0:
		res = 0X30;
		break;
		case 1:
		res = 0X31;
		break;
		case 2:
		res = 0X32;
		break;
		case 3:
		res = 0X33;
		break;
		case 4:
		res = 0X34;
		break;
		case 5:
		res = 0X35;
		break;
		case 6:
		res = 0X36;
		break;
		case 7:
		res = 0X37;
		break;
		case 8:
		res = 0X38;
		break;
		case 9:
		res = 0X39;
		break;
		default:
		res = 0;
		break;
	}
	return res;
}

void data_transfor_csv(int index,float shu_x,float shu_y,double lng,double lat,double time,DATALOG *pdatalog)
{
	uint8_t ge = 0,shi = 0,bai = 0,qian = 0,wan = 0,shiwan = 0,baiwan = 0,qianwan = 0,yi = 0;
	int val = 0;
	
	int shu_x_num = 0,shu_y_num = 0;
	int lng_num = 0,lat_num = 0;
	int datatime = 0;
	shu_x_num = shu_x*1000000;
	shu_y_num = shu_y*1000000;
	lng_num = lng*1000000;
	lat_num = lat*1000000;
	datatime = time*1000000;
	
	val = index;
	weishu_chaifen(val,&ge,&shi,&bai,&qian,&wan,&shiwan,&baiwan,&qianwan,&yi);
	strcpy(pdatalog->data.INDEX,"");
	pdatalog->data.INDEX[0] = wan;
	pdatalog->data.INDEX[1] = qian;
	pdatalog->data.INDEX[2] = bai;
	pdatalog->data.INDEX[3] = shi;
	pdatalog->data.INDEX[4] = ge;
	
	val = shu_x_num;
	weishu_chaifen(val,&ge,&shi,&bai,&qian,&wan,&shiwan,&baiwan,&qianwan,&yi);
	strcpy(pdatalog->data.SHU_X,"");
	pdatalog->data.SHU_X[0] = qianwan;
	pdatalog->data.SHU_X[1] = baiwan;
	pdatalog->data.SHU_X[2] = shiwan;
	pdatalog->data.SHU_X[3] = wan;
	pdatalog->data.SHU_X[4] = qian;
	pdatalog->data.SHU_X[5] = bai;
	pdatalog->data.SHU_X[6] = shi;
	pdatalog->data.SHU_X[7] = ge;
	
	val = shu_y_num;
	weishu_chaifen(val,&ge,&shi,&bai,&qian,&wan,&shiwan,&baiwan,&qianwan,&yi);
	strcpy(pdatalog->data.SHU_Y,"");
	pdatalog->data.SHU_Y[0] = qianwan;
	pdatalog->data.SHU_Y[1] = baiwan;
	pdatalog->data.SHU_Y[2] = shiwan;
	pdatalog->data.SHU_Y[3] = wan;
	pdatalog->data.SHU_Y[4] = qian;
	pdatalog->data.SHU_Y[5] = bai;
	pdatalog->data.SHU_Y[6] = shi;
	pdatalog->data.SHU_Y[7] = ge;
	
	val = lng_num;
	weishu_chaifen(val,&ge,&shi,&bai,&qian,&wan,&shiwan,&baiwan,&qianwan,&yi);
	strcpy(pdatalog->data.LNG,"");
	pdatalog->data.LNG[0] = yi;
	pdatalog->data.LNG[1] = qianwan;
	pdatalog->data.LNG[2] = baiwan;
	pdatalog->data.LNG[3] = shiwan;
	pdatalog->data.LNG[4] = wan;
	pdatalog->data.LNG[5] = qian;
	pdatalog->data.LNG[6] = bai;
	pdatalog->data.LNG[7] = shi;
	pdatalog->data.LNG[8] = ge;
	
	val = lat_num;
	weishu_chaifen(val,&ge,&shi,&bai,&qian,&wan,&shiwan,&baiwan,&qianwan,&yi);
	strcpy(pdatalog->data.LAT,"");
	pdatalog->data.LAT[0] = yi;
	pdatalog->data.LAT[1] = qianwan;
	pdatalog->data.LAT[2] = baiwan;
	pdatalog->data.LAT[3] = shiwan;
	pdatalog->data.LAT[4] = wan;
	pdatalog->data.LAT[5] = qian;
	pdatalog->data.LAT[6] = bai;
	pdatalog->data.LAT[7] = shi;
	pdatalog->data.LAT[8] = ge;
	
	val = datatime;
	weishu_chaifen(val,&ge,&shi,&bai,&qian,&wan,&shiwan,&baiwan,&qianwan,&yi);
	strcpy(pdatalog->data.TIME,"");
	pdatalog->data.TIME[0] = qianwan;
	pdatalog->data.TIME[1] = baiwan;
	pdatalog->data.TIME[2] = shiwan;
	pdatalog->data.TIME[3] = wan;
	pdatalog->data.TIME[4] = qian;
	pdatalog->data.TIME[5] = bai;
	pdatalog->data.TIME[6] = shi;
	pdatalog->data.TIME[7] = ge;
}

void data_transfor_csv_packet(int index,float shu_x,float shu_y,double lng,double lat,double time,DATALOGLIST *pdatalist)
{
	uint8_t ge = 0,shi = 0,bai = 0,qian = 0,wan = 0,shiwan = 0,baiwan = 0,qianwan = 0,yi = 0/*,shiyi = 0,baiyi = 0*/;
	static int mark_cnt = 0;
	int val = 0;
	int shu_x_num = 0,shu_y_num = 0;
	int lng_num = 0,lat_num = 0;
	int datatime = 0;
	
	shu_x_num = shu_x*1000000;
	shu_y_num = shu_y*1000000;
	lng_num = lng*1000000;
	lat_num = lat*1000000;
	datatime = time*1000000;
	
	val = index;
	weishu_chaifen(val,&ge,&shi,&bai,&qian,&wan,&shiwan,&baiwan,&qianwan,&yi);
	strcpy(pdatalist->data[mark_cnt].INDEX,"");
	pdatalist->data[mark_cnt].INDEX[0] = wan;
	pdatalist->data[mark_cnt].INDEX[1] = qian;
	pdatalist->data[mark_cnt].INDEX[2] = bai;
	pdatalist->data[mark_cnt].INDEX[3] = shi;
	pdatalist->data[mark_cnt].INDEX[4] = ge;
	
	val = shu_x_num;
	weishu_chaifen(val,&ge,&shi,&bai,&qian,&wan,&shiwan,&baiwan,&qianwan,&yi);
	strcpy(pdatalist->data[mark_cnt].SHU_X,"");
	pdatalist->data[mark_cnt].SHU_X[0] = yi;
	pdatalist->data[mark_cnt].SHU_X[1] = qianwan;
	pdatalist->data[mark_cnt].SHU_X[2] = baiwan;
	pdatalist->data[mark_cnt].SHU_X[3] = shiwan;
	pdatalist->data[mark_cnt].SHU_X[4] = wan;
	pdatalist->data[mark_cnt].SHU_X[5] = qian;
	pdatalist->data[mark_cnt].SHU_X[6] = bai;
	pdatalist->data[mark_cnt].SHU_X[7] = shi;
	pdatalist->data[mark_cnt].SHU_X[8] = ge;
	
	val = shu_y_num;
	weishu_chaifen(val,&ge,&shi,&bai,&qian,&wan,&shiwan,&baiwan,&qianwan,&yi);
	strcpy(pdatalist->data[mark_cnt].SHU_Y,"");
	pdatalist->data[mark_cnt].SHU_Y[0] = yi;
	pdatalist->data[mark_cnt].SHU_Y[1] = qianwan;
	pdatalist->data[mark_cnt].SHU_Y[2] = baiwan;
	pdatalist->data[mark_cnt].SHU_Y[3] = shiwan;
	pdatalist->data[mark_cnt].SHU_Y[4] = wan;
	pdatalist->data[mark_cnt].SHU_Y[5] = qian;
	pdatalist->data[mark_cnt].SHU_Y[6] = bai;
	pdatalist->data[mark_cnt].SHU_Y[7] = shi;
	pdatalist->data[mark_cnt].SHU_Y[8] = ge;
	
	val = lng_num;
	weishu_chaifen(val,&ge,&shi,&bai,&qian,&wan,&shiwan,&baiwan,&qianwan,&yi);
	strcpy(pdatalist->data[mark_cnt].LNG,"");
	pdatalist->data[mark_cnt].LNG[0] = yi;
	pdatalist->data[mark_cnt].LNG[1] = qianwan;
	pdatalist->data[mark_cnt].LNG[2] = baiwan;
	pdatalist->data[mark_cnt].LNG[3] = shiwan;
	pdatalist->data[mark_cnt].LNG[4] = wan;
	pdatalist->data[mark_cnt].LNG[5] = qian;
	pdatalist->data[mark_cnt].LNG[6] = bai;
	pdatalist->data[mark_cnt].LNG[7] = shi;
	pdatalist->data[mark_cnt].LNG[8] = ge;
	
	val = lat_num;
	weishu_chaifen(val,&ge,&shi,&bai,&qian,&wan,&shiwan,&baiwan,&qianwan,&yi);
	strcpy(pdatalist->data[mark_cnt].LAT,"");
	pdatalist->data[mark_cnt].LAT[0] = yi;
	pdatalist->data[mark_cnt].LAT[1] = qianwan;
	pdatalist->data[mark_cnt].LAT[2] = baiwan;
	pdatalist->data[mark_cnt].LAT[3] = shiwan;
	pdatalist->data[mark_cnt].LAT[4] = wan;
	pdatalist->data[mark_cnt].LAT[5] = qian;
	pdatalist->data[mark_cnt].LAT[6] = bai;
	pdatalist->data[mark_cnt].LAT[7] = shi;
	pdatalist->data[mark_cnt].LAT[8] = ge;
	
	val = datatime;
	weishu_chaifen(val,&ge,&shi,&bai,&qian,&wan,&shiwan,&baiwan,&qianwan,&yi);
	strcpy(pdatalist->data[mark_cnt].TIME,"");
	pdatalist->data[mark_cnt].TIME[0] = qianwan;
	pdatalist->data[mark_cnt].TIME[1] = baiwan;
	pdatalist->data[mark_cnt].TIME[2] = shiwan;
	pdatalist->data[mark_cnt].TIME[3] = wan;
	pdatalist->data[mark_cnt].TIME[4] = qian;
	pdatalist->data[mark_cnt].TIME[5] = bai;
	pdatalist->data[mark_cnt].TIME[6] = shi;
	pdatalist->data[mark_cnt].TIME[7] = ge;
	
	mark_cnt++;
	if(mark_cnt>=PACKET_NUM){
		File_CSV_Write_SD_Packet(&datalist);
		mark_cnt = 0;
	}
}

void weishu_chaifen(int num,uint8_t *p_ge,uint8_t *p_shi,uint8_t *p_bai,uint8_t *p_qian,uint8_t *p_wan,\
uint8_t *p_shiwan,uint8_t *p_baiwan,uint8_t *p_qianwan,uint8_t *p_yi)
{
	uint8_t ge = 0,shi = 0,bai = 0,qian = 0,wan = 0,shiwan = 0,baiwan = 0,qianwan = 0,yi = 0;
	int val = num;
	if(val>=100000000){
		yi = num_to_ascii(val/100000000%10);
	}
	else{
		yi = 0;
	}
	
	if(val>=10000000){
		qianwan = num_to_ascii(val/10000000%10);
	}
	else{
		qianwan = 0;
	}
	if(val>=1000000){
		baiwan = num_to_ascii(val/1000000%10);
	}
	else{
		baiwan = 0;
	}
	if(val>=100000){
		shiwan = num_to_ascii(val/100000%10);
	}
	else{
		shiwan = 0;
	}
	if(val>=10000){
		wan = num_to_ascii(val/10000%10);
	}
	else{
		wan = 0;
	}
	if(val>=1000){
		qian = num_to_ascii(val/1000%10);
	}
	else{
		qian = 0;
	}
	if(val>=100){
		bai = num_to_ascii(val/100%10);
	}
	else{
		bai = 0;
	}
	if(val>=10){
		shi = num_to_ascii(val/10%10);
	}
	else{
		shi = 0;
	}
	ge = num_to_ascii(val%10);
	
	*p_ge = ge;*p_shi = shi;*p_bai = bai;*p_qian = qian;
	*p_wan = wan;
	*p_shiwan = shiwan;
	*p_baiwan = baiwan;
	*p_qianwan = qianwan;
	*p_yi = yi;
}

//uint8_t File_CSV_Write(void)
//{
//	FRESULT res;
//	uint16_t i=0 , j = 0;
//	for(i = 0;i<1024;i++)
//	{
//		fw_databuf[i] = i;
//	}
//	for(i = 0;i<10;i++)
//	{
//		for(j = 0;j<sizeof(DATALIST);j++)
//		{
//			DATALIST[i].buf[j] = datalog.buf[j];
//		}
//	}
//	#if 1
//	res=f_open(file, "0:datalog.CSV",FA_OPEN_EXISTING | FA_WRITE);
//	if(res!=FR_OK)
//	{
//		u1_printf("f_open->datalog.CSV->error.\r\n");
//		return 0;
//	}
//	u1_printf("f_open->datalog.CSV->OK!.\r\n");
////	mf_lseek(mf_size());//定位读写位置
//	mf_lseek(32);//定位读写位置
////	mf_write(fw_databuf,1024);
//	res=f_write(file,datalog.buf,sizeof(DATALOG), &writecount);
//	if(res!=FR_OK)
//	{
//		u1_printf("f_write->datalog.CSV->error.\r\n");
//		return 0;
//	}
//	u1_printf("f_write->datalog.CSV->ok.\r\n");
//	mf_close();	
//	
//	res=f_open(file, "0:datalog.CSV",FA_OPEN_EXISTING | FA_READ);
//	if(res!=FR_OK)
//	{
//		u1_printf("f_open->datalog.CSV->error.\r\n");
//		return 0;
//	}
//	u1_printf("f_open->datalog.CSV->OK!.\r\n");
//	res=f_read(file,filedatabuf,2048, &readcount);
//	if(res!=FR_OK)
//	{
//		u1_printf("f_read->datalog.CSV->error.\r\n");
//		return 0;
//	}
//	u1_printf("f_read->datalog.CSV->ok.\r\n");
//	mf_close();	
//	#endif
//return 0;
//}
