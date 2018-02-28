#include "ap_storage_service.h"
#include "ap_state_config.h"
#include "ap_state_handling.h"
#include "fs_driver.h"
#include "avi_encoder_app.h"
#include "stdio.h"
#include "string.h"

extern void save_VERSION_NUMBER_to_disk(void);
extern void save_COPYRIGHT_MESSAGE_to_disk(void);
extern void save_SENSOR_TYPE_to_disk(INT16U id, INT16U type);

const INT8U  R_CopyrightMSG_Buf[]= "版权声明: 本产品由深圳市乐信兴业科技有限公司设计,版权所有,仿冒必究"; //
const INT8U   sensor_type_name[11][15]={"Error","GC1004_DVP","GC1004_MIPI","GC1024_DVP","GC1024_MIPI","GC1064_DVP","GC1064_MIPI","SOI_H22",
	                                   "SOI_H22_MIPI","BF2116","OV9712"
};

INT16U Sensor_Type = 0;
INT16U Sensor_ID = 0;
INT8U PrintMsg_Sensor = 0;


INT8U OpenPrintMsgGet(void)
{
	return PrintMsg_Sensor;
}

void OpenPrintMsgSet(INT8U flag)
{
	PrintMsg_Sensor = flag;
}

void save_COPYRIGHT_MESSAGE_to_disk(void)
{
    INT16S fd;
	INT32U addr;
	fd = open("C:\\CopyrightMSG.txt", O_RDWR|O_TRUNC|O_CREAT);
	if(fd < 0)
		return;
	addr = (INT32U)gp_malloc(66+1);
	if(!addr)
	{
		close(fd);
		return;
	}
	gp_strcpy((INT8S*)addr, (INT8S *)R_CopyrightMSG_Buf);
	write(fd, addr, 66);
	close(fd);
	gp_free((void*) addr);
}
void save_VERSION_NUMBER_to_disk(void)
{
    INT8U  *p;
	INT16S fd;
	INT32U addr;
	INT32U product_num;
	INT32U data_num;
	product_num = PRODUCT_NUM;
	data_num = PRODUCT_DATA;
	fd = open("C:\\Version.txt", O_RDWR|O_TRUNC|O_CREAT);
	if(fd < 0)
		return;
	addr = (INT32U)gp_malloc(22+5);
	if(!addr)
	{
		close(fd);
		return;
	}
	p = (INT8U*)addr;
	sprintf((char *)p, (const char *)"%08d,JH%04d,v0.00,", data_num, product_num);
	p[17] = ((PROGRAM_VERSION_NUM%1000)/100) + '0';
	p[19] = ((PROGRAM_VERSION_NUM%100)/10) + '0';
	p[20] = ((PROGRAM_VERSION_NUM%10)/1) + '0';
	write(fd, addr, 22);
	close(fd);
	gp_free((void*) addr);
}

void save_SENSOR_TYPE_to_disk(INT16U id, INT16U type)
{
	//INT8U  *p;
	INT16S fd;
	INT32U addr;
	INT16U temp;
	
	DBG_PRINT("Print sensor type...\r\n");
	fd = open("C:\\sensor.txt", O_RDWR|O_TRUNC|O_CREAT);
	if(fd < 0)
		return;
	addr = (INT32U)gp_malloc(13+5);
	if(!addr)
	{
		close(fd);
		return;
	}
	
	if (id == 0xDC)
	{
		temp = 9; //BF2116
    }
    else if (id == 0x78)//GC1004, GC1024, GC1064
    {
    	if (type|sensor_format) temp = type;
    		else temp = type+1;
    }
	else
	{
		temp = 0; //Error
	}
	gp_strcpy((INT8S*)addr, (INT8S *)sensor_type_name[temp]);
	write(fd, addr, strlen((const char *)sensor_type_name[temp]));
	close(fd);
	gp_free((void*) addr);
}

