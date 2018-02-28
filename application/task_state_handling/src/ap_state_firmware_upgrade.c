/*
* Description: This file provides APIs to upgrade firmware from SD card to SPI
*
* Author: Tristan Yang
*
* Date: 2008/02/17
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 1.00
*/
#include "string.h"
#include "ap_state_firmware_upgrade.h"
#include "ap_state_resource.h"
#include "drv_l2_spifc.h"
#include "ap_state_handling.h"
#include "socket_cmd.h"
#include "stdio.h"
#include "math.h"

extern INT32S SPIFC_Flash_erase_block(INT32U addr, INT8U eraseArgs);
extern INT32S SPIFC_Flash_read_page(INT32U addr, INT8U *buf);
extern INT32S SPIFC_Flash_write_page(INT32U addr, INT8U *buf);

extern INT32S vid_enc_disable_sensor_clock(void);

#define SPIFC_RESOURCE_OFFSET	0
#define UPGRADE_POS_X 10
#define UPGRADE_POS_Y 100

static INT8U *pWIFI_BIN = NULL;
static INT32U WIFI_SIZE = 0;		// total size
static INT32U WIFI_CUR_SIZE = 0;	// current readingl size

/*
static void cpu_draw_burn_percent(INT32U val, INT32U target_buffer)
{
	STRING_INFO str_info = {0};
	INT16U temp;

	str_info.language = LCD_EN;
	str_info.font_color = 0xFFFF;	//white
	str_info.pos_x = UPGRADE_POS_X;
	str_info.pos_y = UPGRADE_POS_Y+40;
	str_info.buff_w = TFT_WIDTH;
	str_info.buff_h = TFT_HEIGHT;

	temp = val/10;
	ap_state_resource_char_draw(temp+0x30, (INT16U *) target_buffer, &str_info, RGB565_DRAW, 0);
	temp = val - temp * 10;
	ap_state_resource_char_draw(temp+0x30, (INT16U *) target_buffer, &str_info, RGB565_DRAW, 0);
}
*/
static INT32U spifc_rewrite(int idx, INT8U *frame_buf, INT8U* verify_buf)
{
	INT32U i = 0;
	INT32U ret = -1;
	INT32U time = 5;

	do {	
		for (i=0; i<C_UPGRADE_SPI_WRITE_SIZE; ++i)
		{
			if (verify_buf[i]!=0xFF) {
				DBG_PRINT("0x%x page is not empty\r\n",idx);
				return -1;				// Erase 64KB，整個 PAGE 重新寫過
			}	
		}
		if (SPIFC_Flash_write_page(idx, frame_buf)!=0) {
			DBG_PRINT("Rewrite 0x%x Error!!\r\n");
			return -1;
		}

		if (SPIFC_Flash_read_page(idx, verify_buf)!=0) {
			DBG_PRINT("Reread 0x%x Error!!\r\n");
		}

		if (memcmp(verify_buf,frame_buf,C_UPGRADE_SPI_WRITE_SIZE)==0)
		{
			DBG_PRINT("0x%x rewrite OK\r\n",idx);
			ret = 0;
			break;
		}
	}while(--time != 0);


	return ret;
}

static INT8U  CheckSum_FileName[8+1];
static INT32S upgrade_open(INT32U upgrade_type, INT16S *pfd, INT32U *ptotal_size, struct stat_t *pstatetest)
{
	INT32S ret = 0;
	char   p[4];
	INT8U  FileName_Path[3+5+8+4+1+4];
    struct f_info	file_info;
    //struct stat_t statetest;
    INT16S nRet;
    
	switch (upgrade_type)
	{
		case 1:		// Wi-Fi
			*ptotal_size = WIFI_SIZE;
			DBG_PRINT("open size = %d\r\n",*ptotal_size);
			break;
		case 0:		// SD
		default:
			if (storage_sd_upgrade_file_flag_get() != 2) {
				ret = -1;
				goto UPGRADE_OPEN_END;
			}
			//++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			sprintf((char*)p,(const char*)"%04d",PRODUCT_NUM);
			   gp_strcpy((INT8S*)FileName_Path,(INT8S*)"C:\\JH_");
			   gp_strcat((INT8S*)FileName_Path,(INT8S*)p);
			   gp_strcat((INT8S*)FileName_Path,(INT8S*)"*.bin");

			   nRet = _findfirst((CHAR*)FileName_Path, &file_info ,D_ALL); 
			   if (nRet < 0) 
			   {
				   	ret = -1;
					goto UPGRADE_OPEN_END;
				   return ;
			   }
			   strncpy((CHAR*)CheckSum_FileName,(CHAR*)file_info.f_name+8,8);    
      		  CheckSum_FileName[8] = NULL; 
			  gp_strcpy((INT8S*)FileName_Path,(INT8S*)"C:\\");
      		  gp_strcat((INT8S*)FileName_Path,(INT8S*)file_info.f_name);
		     DBG_PRINT("FIND UPDATAFILE TYPE\r\n");
			 DBG_PRINT("FileName = %s\r\n", file_info.f_name);
			//++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			*pfd = open((CHAR*)FileName_Path, O_RDONLY);
			if (*pfd < 0) {
				ret = -1;
				goto UPGRADE_OPEN_END;
			}
			if (fstat(*pfd, pstatetest)) {
				close(*pfd);
				ret = -1;
				goto UPGRADE_OPEN_END;
			}
			*ptotal_size = pstatetest->st_size;
	}


UPGRADE_OPEN_END:
	return ret;
}

static INT32S upgrade_close(INT32U upgrade_type, INT16S *pfd)
{
	INT32S ret = 0;

	switch (upgrade_type)
	{
		case 1:		// Wi-Fi
			DBG_PRINT("close\r\n");
			break;
		case 0:		// SD
		default:
			close(*pfd);
	}

	return ret;
}

static INT32S upgrade_read(INT32U upgrade_type, INT16S *pfd, INT32U *buf, INT32U size)
{
	INT32S ret = 0;

	switch (upgrade_type)
	{
		case 1:		// Wi-Fi
			if ( (WIFI_CUR_SIZE+size)>=WIFI_SIZE )
			{
				size = WIFI_SIZE - WIFI_CUR_SIZE;				
			}
			gp_memcpy((INT8S*)buf, (INT8S*)(pWIFI_BIN+WIFI_CUR_SIZE), size);
			WIFI_CUR_SIZE+=size;
			ret = size;		
			DBG_PRINT("Wi-Fi+size = %d(%d)  => WIFI_CUR_SIZE(%d)\r\n",size,ret,WIFI_CUR_SIZE);
			break;
		case 0:		// SD
		default:
			ret = read(*pfd, (INT32U) buf, size);
			DBG_PRINT("SD size = %d\r\n",size);
	}

	return ret;
}

INT32S upgrade_exit(INT32U upgrade_type)
{
	INT32S ret = 0;
	INT32U i;

	switch (upgrade_type)
	{
		case 1:		// Wi-Fi
			break;
		case 2:		// WIFI UPGRADE FINISH
			while(sys_pwr_key0_read()||sys_pwr_key1_read())
			{
				OSTimeDly(1);
			}
			//gpio_write_io(POWER_EN_PIN, DATA_LOW);
			gpio_write_io(POWER_EN_PIN, DATA_LOW);
			sys_ldo33_off();	 // turn off LDO 3.3
			R_SYSTEM_WATCHDOG_CTRL = 0x8004;
			OSTimeDly(100);
			break;	
		case 0:		// SD
		default:
	#if  (PWR_KEY_TYPE == READ_FROM_GPIO)
		i=0;
		while(1) {
			#if KEY_ACTIVE
			  if (gpio_read_io(PW_KEY)) {
			#else
			  if (!gpio_read_io(PW_KEY)) {
			#endif
				  i++;
			  }
			  else {
				  i=0;
			  }
			if (i >= 3) {
				#if KEY_ACTIVE
				  while(gpio_read_io(PW_KEY));
				#else
				  while(!gpio_read_io(PW_KEY));
				#endif
				
				sys_ldo33_off();	// turn off LDO 3.3
				R_SYSTEM_WATCHDOG_CTRL = 0x8004;								 
			}
			OSTimeDly(5);
		}
	 #else  // EVB
		 i=0;
		 while(1) {
			 if ( (PW_KEY==PWR_KEY0)&&(sys_pwr_key0_read()) ) {
				 i++;
			 }
			 else if ( (PW_KEY==PWR_KEY1)&&(sys_pwr_key1_read()) ) {
				 i++;
			 }
			 else {
				 i=0;
			 }

			 if (i >= 3) {
				 if (PW_KEY==PWR_KEY0) while(sys_pwr_key0_read());
				 if (PW_KEY==PWR_KEY1) while(sys_pwr_key1_read());
				 gpio_write_io(POWER_EN_PIN, DATA_LOW);
				 sys_ldo33_off();	 // turn off LDO 3.3
				 R_SYSTEM_WATCHDOG_CTRL = 0x8004;								 
			 }
			 OSTimeDly(5);
		 }
	 #endif			
	}

	return ret;
}

static INT32U STRING_Convert_HEX(CHAR *str)
{
    INT32U sum = 0;
    INT8U len;
    INT16S i;

    len = strlen((const char *)str);
    for(i = 0; i<len; i++)
    {
        if (str[i]>='0'&&str[i]<='9') sum += (str[i]-'0')*(pow(16,len-1-i));
        else if(str[i]>='a'&&str[i]<='f') sum += (str[i]-'a'+10)*(pow(16,len-1-i));
        else if(str[i]>='A'&&str[i]<='F') sum += (str[i]-'A'+10)*(pow(16,len-1-i));
    }
    return sum; 
}

INT32S ap_state_firmware_upgrade(void)
{
	INT32U /*i,*/ j, k, upgrade_type, total_size, complete_size;
	INT32U *firmware_buffer, *verify_buffer;
	//INT32U display_buffer;
	//INT32U buff_size, *buff_ptr;	
	INT32S verify_size;
	//INT16U *ptr; 
	struct stat_t statetest;
	INT16S fd;
	//STRING_ASCII_INFO ascii_str;
	//INT8S  prog_str[6];
	INT8U retry=0;
	//OS_Q_DATA OSQData;
	INT32U led_type;
	INT32U CheckSum_Bin = 0;
	INT8U *p;

	if ( gp_wifi_upgrade_check(&pWIFI_BIN, &WIFI_SIZE) )
	{
		upgrade_type = 1;		// Wi-Fi upgrade
	}
	else
	{
		upgrade_type = 0;		// SD upgrade
	}


	//////////////  start to upgrade ///////////////////////
	if (upgrade_open(upgrade_type, &fd, &total_size, &statetest)!=0)
	{
		DBG_PRINT("Can't find the update file! \r\n");
		return -1;
	}
	//++++++++++++++++++ Check Sum Verify+++++++++++++++++++++++++
	//SD縐汔撰奀, 勤掀潰桄睿
	DBG_PRINT("total_size=%08X\r\n",total_size);
	if (upgrade_type == 0) 
	{
		INT32U Bin_Addr = 0;
		INT32U *ReadBin_Buf;
		INT32U AddCnt;
		
		ReadBin_Buf = (INT32U *) gp_malloc(256);
		
	    for(k=0; k<total_size/256; k++)
	    {
		   lseek(fd, Bin_Addr, SEEK_SET);	 
		   if (read(fd,(INT32U)ReadBin_Buf,256)<=0)
		   {
			   gp_free((void*)ReadBin_Buf);
				led_type = LED_UPDATE_FAIL; 
				msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
			   return -1;
		   }
		   p = (INT8U*)ReadBin_Buf;
		   for(AddCnt=0; AddCnt<256; AddCnt++)
		   {
			   CheckSum_Bin += *p++;
		   }
		   Bin_Addr += 256;
	    }
		if((total_size%256)!= 0)
		{
	      lseek(fd, Bin_Addr, SEEK_SET);
		  if (read(fd,(INT32U)ReadBin_Buf,total_size%256)<=0)
		   {
			   gp_free((void*)ReadBin_Buf);
				led_type = LED_UPDATE_FAIL; 
				msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
			   return -1;
		   }
	       p = (INT8U*)ReadBin_Buf;
	       for(AddCnt=0; AddCnt<(AddCnt%256); AddCnt++)
          {
			   CheckSum_Bin += *p++;
          }
		}
		gp_free((void*)ReadBin_Buf);
		DBG_PRINT("CheckSum_Bin=%08X\r\n",CheckSum_Bin);
		if (CheckSum_Bin != STRING_Convert_HEX((CHAR*)CheckSum_FileName))
		{
			DBG_PRINT("Upgrade file check failed! \r\n");
			led_type = LED_UPDATE_FAIL; 
	   		msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
			return -1;
		}
		DBG_PRINT("Upgrade file check is successful! \r\n");
	}
	//++++++++++++++++++ Check Sum Verify+++++++++++++++++++++++++
	led_type = LED_UPDATE_PROGRAM;
	msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
    vid_enc_disable_sensor_clock();
	ap_state_handling_tv_uninit();

	verify_size = (INT32S)total_size;

	firmware_buffer = (INT32U *) gp_malloc(C_UPGRADE_BUFFER_SIZE);
	if (!firmware_buffer) {
		DBG_PRINT("firmware upgrade allocate firmware_buffer fail\r\n");
		upgrade_close(upgrade_type, &fd);
		led_type = LED_UPDATE_FAIL; 
   		msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
		return -1;
	}

	verify_buffer = (INT32U *) gp_malloc(C_UPGRADE_SPI_WRITE_SIZE);
	if (!verify_buffer) {
		DBG_PRINT("firmware upgrade allocate verify_buffer fail\r\n");
		upgrade_close(upgrade_type, &fd);
		led_type = LED_UPDATE_FAIL; 
   		msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
		return -1;
	}
	
	sys_kill_timer(STORAGE_SERVICE_MOUNT_TIMER_ID);
	DBG_PRINT("kill STORAGE_SERVICE_MOUNT_TIMER_ID:%d\r\n",STORAGE_SERVICE_MOUNT_TIMER_ID);
	OSTaskDel(STORAGE_SERVICE_PRIORITY);
	OSTaskDel(USB_DEVICE_PRIORITY);
	sys_kill_timer(AD_DETECT_TIMER_ID);	// It may cause Wi-Fi upgrade unstable.
	DBG_PRINT("kill AD_DETECT_TIMER_ID:%d\r\n",AD_DETECT_TIMER_ID);		
	//OSTaskDel(PERIPHERAL_HANDLING_PRIORITY);

#if GPDV_BOARD_VERSION == GPCV4247_WIFI
	DBG_PRINT("Upgrading firmware...\r\n");
	DBG_PRINT("Do not power off now\r\n");
#else
	buff_size = TFT_WIDTH * TFT_HEIGHT * 2;
	display_buffer = (INT32U) gp_malloc_align(buff_size, 64);
	if (!display_buffer) {
		upgrade_close(upgrade_type, &fd);
		DBG_PRINT("firmware upgrade allocate display buffer fail\r\n");
		return -1;
	}
	
	buff_ptr = (INT32U*) display_buffer;
	buff_size >>= 2;
	for (i=0;i<buff_size;i++) {
		*buff_ptr++ = 0;
	}
	
	OSQPost(DisplayTaskQ, (void *) MSG_DISPLAY_TASK_EFFECT_INIT);

	ascii_str.font_color = 0xFFFF;
	ascii_str.font_type = 0;
	ascii_str.buff_w = TFT_WIDTH;
	ascii_str.buff_h = TFT_HEIGHT;
	ascii_str.pos_x = UPGRADE_POS_X;
	ascii_str.pos_y = UPGRADE_POS_Y;
	ascii_str.str_ptr = "Upgrading firmware...";
	ap_state_resource_string_ascii_draw((INT16U *)display_buffer, &ascii_str, RGB565_DRAW);
	
	ascii_str.pos_x = UPGRADE_POS_X;
	ascii_str.pos_y = UPGRADE_POS_Y+20;
	ascii_str.str_ptr = "Do not power off now";
	ap_state_resource_string_ascii_draw((INT16U *)display_buffer, &ascii_str, RGB565_DRAW);
	
	ascii_str.pos_x = UPGRADE_POS_X;
	ascii_str.pos_y = UPGRADE_POS_Y+40;
	ascii_str.str_ptr = "00%";
	ap_state_resource_string_ascii_draw((INT16U *)display_buffer, &ascii_str, RGB565_DRAW);

	OSQPost(DisplayTaskQ, (void *) (display_buffer|MSG_DISPLAY_TASK_JPEG_DRAW));
	OSQQuery(DisplayTaskQ, &OSQData);
	while(OSQData.OSMsg != NULL) {
		OSTimeDly(2);
		OSQQuery(DisplayTaskQ, &OSQData);
	}

#endif

	R_SYSTEM_PLLEN  |= 0xC00;	// 把 SPI clock 降低，讀寫比較穩
	complete_size = 0;
	lseek(fd, complete_size, SEEK_SET);
	while (complete_size < total_size) {
		INT32U buffer_left;
		INT32S block_size;

	    block_size = upgrade_read(upgrade_type, &fd, firmware_buffer, C_UPGRADE_BUFFER_SIZE);
		if (block_size <= 0) {
			break;
		}
		// DBG_PRINT("S:%d\r\n",block_size);
		buffer_left = (total_size - complete_size + (C_UPGRADE_SPI_BLOCK_SIZE-1)) & ~(C_UPGRADE_SPI_BLOCK_SIZE-1);
		if (buffer_left > C_UPGRADE_BUFFER_SIZE) {
			buffer_left = C_UPGRADE_BUFFER_SIZE;
		}
		retry = 0;  // 每個 64KB 有 20 次機會
		while (buffer_left && retry<C_UPGRADE_FAIL_RETRY) {
			INT32U complete_size_bak;
			complete_size &= ~(C_UPGRADE_SPI_BLOCK_SIZE-1);
			complete_size_bak = complete_size;
			if (SPIFC_Flash_erase_block(SPIFC_RESOURCE_OFFSET+complete_size_bak, ERASE_BLOCK_64K)) {
				retry++;
				continue;
			}
			for (j=C_UPGRADE_SPI_BLOCK_SIZE; j; j-=C_UPGRADE_SPI_WRITE_SIZE) {
				if (SPIFC_Flash_write_page(SPIFC_RESOURCE_OFFSET+complete_size_bak, (INT8U *) (firmware_buffer + ((complete_size_bak & (C_UPGRADE_BUFFER_SIZE-1))>>2)))) {
					break;
				}
				complete_size_bak += C_UPGRADE_SPI_WRITE_SIZE;
				if(complete_size_bak>=total_size)
				{
					j=0;
					break;
				}
			}
			
			if ((j == 0)&&(verify_size>0)) {	// verify stage
				complete_size_bak = complete_size;
				
				for (j=C_UPGRADE_SPI_BLOCK_SIZE; j; j-=C_UPGRADE_SPI_WRITE_SIZE) {
					INT32U i;
					INT32U flag = 0;
					for (i=0;i<5;++i)
					{
						if ((SPIFC_Flash_read_page(SPIFC_RESOURCE_OFFSET+complete_size_bak, (INT8U*)verify_buffer) )==0 )
							break;
					}

					flag = 0;
					if (memcmp( (void*)verify_buffer  ,(void *)(firmware_buffer + ((complete_size_bak & (C_UPGRADE_BUFFER_SIZE-1))>>2)), C_UPGRADE_SPI_WRITE_SIZE)!=0) {
						DBG_PRINT("Verify 0x%x (0x%x) Error !!\r\n",complete_size_bak,C_UPGRADE_SPI_WRITE_SIZE);
						flag = 1;
					}
					//else DBG_PRINT("Verify 0x%x (0x%x) OK !!\r\n",complete_size_bak,verify_size);

					if (flag == 1)
					{
						if ( spifc_rewrite(SPIFC_RESOURCE_OFFSET+complete_size_bak, (INT8U*)(firmware_buffer + ((complete_size_bak & (C_UPGRADE_BUFFER_SIZE-1))>>2)),  (INT8U*)verify_buffer)!=0 )
						{
							break;
						}
					}
					
					complete_size_bak += C_UPGRADE_SPI_WRITE_SIZE;
					if(complete_size_bak>=total_size)
					{
						j=0;
						break;
					}
					verify_size-= C_UPGRADE_SPI_WRITE_SIZE;
					if (verify_size <=0) // 檔案結束，沒必要再比下去
					{
						j = 0;
						break;
					}
				}				
			}	

			complete_size = complete_size_bak;
			if (j == 0) {
				buffer_left -= C_UPGRADE_SPI_BLOCK_SIZE;

				if (complete_size < total_size) {
					j = complete_size*100/total_size;
				} else {
					j = 99;//100;
				}

				#if GPDV_BOARD_VERSION == GPCV4247_WIFI
					DBG_PRINT("%02d ",j);
				#else
				for (i=0;i<50;i++) {
					ptr = (INT16U *) (display_buffer+((UPGRADE_POS_Y+40+i)*TFT_WIDTH+UPGRADE_POS_X)*2);
					for(k=0;k<22/*50*/;k++) {
						*ptr++ = 0x0;
					}
				}

				/*
				ascii_str.pos_x = UPGRADE_POS_X;
				ascii_str.pos_y = UPGRADE_POS_Y+40;
				sprintf((CHAR*)prog_str,"%d%c",j,'%');
				ascii_str.str_ptr = (CHAR *)prog_str;
				ap_state_resource_string_ascii_draw((INT16U *)display_buffer, &ascii_str, RGB565_DRAW);
				*/

				cpu_draw_burn_percent(j, display_buffer);
				OSQPost(DisplayTaskQ, (void *) (display_buffer|MSG_DISPLAY_TASK_JPEG_DRAW));
				OSQQuery(DisplayTaskQ, &OSQData);
				while(OSQData.OSMsg != NULL) {
					OSTimeDly(2);
					OSQQuery(DisplayTaskQ, &OSQData);
				}
				#endif
			} else {
				retry++;
			}
		}
		if (retry == C_UPGRADE_FAIL_RETRY) {
			break;
		}
	}
#if SDC_UPGRADE_ERASE_USER_CONFIG	
	//DBG_PRINT("user set pos: %d\r\n",(SPIFC_RESOURCE_OFFSET+complete_size+C_UPGRADE_SPI_BLOCK_SIZE)&0xFF0000);
	//DBG_PRINT("user set pos: %d\r\n",(SPIFC_RESOURCE_OFFSET+complete_size+C_UPGRADE_SPI_BLOCK_SIZE));
  
	//SPIFC_Flash_erase_block((SPIFC_RESOURCE_OFFSET+complete_size+C_UPGRADE_SPI_BLOCK_SIZE)&0xFF0000, ERASE_BLOCK_64K);
	SPIFC_Flash_erase_block((SPIFC_RESOURCE_OFFSET+complete_size+C_UPGRADE_SPI_BLOCK_SIZE), ERASE_BLOCK_64K);
#endif
	OSTimeDly(5);

#if GPDV_BOARD_VERSION != GPCV4247_WIFI	
	buff_ptr = (INT32U*) display_buffer;
	for (i=0;i<buff_size;i++) {
		*buff_ptr++ = 0;
	}
#endif	
	ap_state_resource_exit();
	nvmemory_init();
	ap_state_resource_init();
	
	if (retry != C_UPGRADE_FAIL_RETRY) {
		DBG_PRINT("Upgrade OK\r\n");
		
		#if GPDV_BOARD_VERSION == GPCV4247_WIFI
		led_type = LED_UPDATE_FINISH;
       	msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
		DBG_PRINT("Remove SD card and\r\n");
		DBG_PRINT("restart now\r\n");
		#else
		ascii_str.pos_x = UPGRADE_POS_X;
		ascii_str.pos_y = UPGRADE_POS_Y;
		ascii_str.str_ptr = "Remove SD card and";
		ap_state_resource_string_ascii_draw((INT16U *)display_buffer, &ascii_str, RGB565_DRAW);
		
		ascii_str.pos_x = UPGRADE_POS_X;
		ascii_str.pos_y = UPGRADE_POS_Y+20;
		ascii_str.str_ptr = "restart now";
		ap_state_resource_string_ascii_draw((INT16U *)display_buffer, &ascii_str, RGB565_DRAW);
		
		ascii_str.pos_x = UPGRADE_POS_X;
		ascii_str.pos_y = UPGRADE_POS_Y+40;
		ascii_str.str_ptr = "100%";
		ap_state_resource_string_ascii_draw((INT16U *)display_buffer, &ascii_str, RGB565_DRAW);
		#endif
	}
	else {
		DBG_PRINT("Upgrade Fail\r\n");
		#if GPDV_BOARD_VERSION != GPCV4247_WIFI
		ascii_str.pos_x = UPGRADE_POS_X+10;
		ascii_str.pos_y = UPGRADE_POS_Y;
		ascii_str.str_ptr = "Upgrade fail";
		ap_state_resource_string_ascii_draw((INT16U *)display_buffer, &ascii_str, RGB565_DRAW);
		#endif
		led_type = LED_UPDATE_FAIL;
    	msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
	}

	#if GPDV_BOARD_VERSION != GPCV4247_WIFI
	OSQPost(DisplayTaskQ, (void *) (display_buffer|MSG_DISPLAY_TASK_JPEG_DRAW));
	OSQQuery(DisplayTaskQ, &OSQData);
	while(OSQData.OSMsg != NULL) {
		OSTimeDly(2);
		OSQQuery(DisplayTaskQ, &OSQData);
	}
	#endif
	
	upgrade_exit(upgrade_type);

	return 0;
}
