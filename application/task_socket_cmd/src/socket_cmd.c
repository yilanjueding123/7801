/******************************************************
 * socket_cmd.c
 *
 * Purpose: Accept commands via a TCP socket
 *
 * Author: Craig Yang
 *
 * Date: 2015/07/28
 *
 * Copyright Generalplus Corp. ALL RIGHTS RESERVED.
 *
 * Version :
 * History :
 *
 *******************************************************/
#include "stdio.h"
#include "string.h"
#include "gplib.h"
#include "host_apis.h"
#include "gp_stdlib.h"
#include "application.h"
#include "socket_cmd_wifi.h"
#include "socket_cmd.h"
#include "mjpeg_stream.h"
#include "state_video_record.h"
#include "state_video_preview.h"
#include "state_browse.h"
#include "ap_state_config.h"
#include "ap_storage_service.h"
#include "ap_peripheral_handling.h"
#include "ap_state_firmware_upgrade.h"
#include "ap_browse.h"
#include "state_wifi.h"
#if PREVIEW_TCPIP
#else
#include "rtp.h"
#endif

#if 1
#define printf  DBG_PRINT
#else
#define printf(...)
#endif

#define MJ_STREAM_TIMEOUT 10000		// unit: ms
#define SOCKET_CMD_TIMEOUT 1200		// unit: 100ms
#if VLC_RESET_NOTIFICATION
#define SOCKET_MODE_TIMEOUT 200		// unit: 100ms
#define GoPlusCam_VLC_TIMEOUT  10		// unit: 500ms
static int vlc_reset_timer = 0;
#else
#define SOCKET_MODE_TIMEOUT 50		// unit: 100ms
#endif


extern INT8U ap_setting_right_menu_active(STRING_INFO *str, INT8U type, INT8U *sub_tag);
extern INT16U present_state;
extern INT8U bat_lvl_cal_bak;	// battery level
extern INT8U adp_status;		// adapter
extern INT8S video_record_sts;	// status (record mode)
extern INT8S browse_sts;		// status (broswer)

char wifi_ssid[GPSOCK_WiFi_Name_Length+1] = WIFI_SSID_NAME; //"GPLUSPRO";
#if WIFI_SSID_ADD_MACADDR == 1
char mac_addr[6];
#endif
#if WIFI_ENCRYPTION_METHOD == WPA2
char wifi_password[GPSOCK_WiFi_Passwd_Length+1] = "12345678";
#else
char wifi_password[GPSOCK_WiFi_Passwd_Length+1] = "12345";
#endif
INT32U mjpeg_service_ready_flag = 0;		// mjpeg service 非称
INT32U mjpeg_service_playstop_flag = 0;	// 氨ゎ篨夹
INT32U mjpeg_service_playjpeg_flag = 0;	//  JPEG秆 JPEG
INT32U g_wifi_notify_pic_done = 0;
INT32U g_wifi_sd_full_flag = 0;
volatile INT32U g_wifi_pic_work = 0; //拍照状态标志
char system_version[GPSOCK_System_Version_Length] = " ";
static INT32U g_sd_size = 0;
static volatile INT8U ui_language = 0; //默认为: 英文.  0=>英文, 1=>中文
static volatile INT8U app_system_type = 0; //默认为: Android.  0=>Android, 1=>IOS

static int mode_bak = -1;

static int cmd_service_enable = 0; /* cmd service enable flag */
static int cmd_service_start  = 0; /* cmd service status flag */
static INT16U playfile_index_table[PLAYFILE_INDEX_TABLE_SIZE];		// Each 512 files creates a record.
static INT32U firmware_download_size = 0;		// frimware total size
static INT32U firmware_download_checksum = 0;
static INT32U firmware_upgrade_flag = 0;
static INT8U *pfirmware = NULL;
static INT8U *pfirmware_cur = NULL;
static INT32U firmware_cur_size = 0;
static INT8U *prx_firmware_buf;

enum
{
	SWTICHING_PERIOD = 16
};

enum
{
	CAPTURE_PERIOD = 150
};

void wifi_menu_language_set(INT8U sta)
{
	ui_language = sta;
	DBG_PRINT("language: %d\r\n",ui_language);
}

void wifi_app_system_type_set(INT8U sta)
{
	app_system_type = sta;
	DBG_PRINT("app system type: %d\r\n",app_system_type);
}

INT32S gp_wifi_upgrade_check(INT8U **p_Firmware , INT32U *p_Size)
{
	INT32S ret = 0;
	
	if ( (pfirmware!=NULL)&&(firmware_download_size!=0) )
	{
		ret = 1;
		*p_Firmware = pfirmware;
		*p_Size = firmware_download_size;
	}

	return ret;
}

void gp_clear_modebak(void)
{
	mode_bak = -1;
}

void gp_capture_card_full(void)
{
	g_wifi_notify_pic_done = 0;
}

static INT8U g_mode_falg = 0;
INT8U gp_mode_flag_get(void)
{
	return g_mode_falg;
}

void gp_sd_size_set(INT32U Val)
{
	g_sd_size = Val;
}

extern volatile INT8U video_down_flag;
static INT8U cmd_flag_clr_enable = 0;//是否清除超时标志
static int gp_mode_set(int mode)
{
	// static int mode_bak = -1;
	int mode_cur = (present_state&0xFF);
	int ret = 0;
	mjpeg_write_data_t stop_event;	
	INT16U cnt = 0;
	INT8U data;
	
	cmd_flag_clr_enable = 0;
	__msg("mode = %d, mode_bak = %d, mode_cur = %d\n", mode, mode_bak, mode_cur);
	//if ((mode_bak == mode)||(video_down_flag == 1))
	if (mode_bak == mode)
	{
		if ( (mode_bak == FALSE_MENU_MODE) && (mode == FALSE_MENU_MODE) )
		{//设置及菜单模式下清除超时标志, 避免设置参数时超时退出返回连接界面
			cmd_flag_clr_enable = 1;
		}
		goto GP_MODE_SET_END;
	}

	if (  (mode_cur != mode) && (mode==BROWSE_MODE)&&(mjpeg_service_ready_flag))
	{
		#if PREVIEW_TCPIP
		mjpeg_playback_stop(&stop_event);
		#else
		rtp_rtsp_cbk(RTP_STOP_EVENT);
		#endif
		DBG_PRINT("BROWSE_MODE(Stop streaming)\r\n");
	}

	if ( (mode_bak != FALSE_MENU_MODE) && (mode == FALSE_MENU_MODE) )
	{
		//ap_peripheral_menu_key_exe(&cnt);
		//DBG_PRINT("ENTER MENU\r\n");
		msgQSend(ApQ, MSG_APQ_MENU_KEY_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
		OSTimeDly(25);
	}
	else if ((mode_bak == FALSE_MENU_MODE) && (mode != FALSE_MENU_MODE)&&(mode_cur != BROWSE_MODE))
	{
		//DBG_PRINT("EXIT MENU\r\n");
		msgQSend(ApQ, MSG_APQ_WIFI_MENU_EXIT, &data, sizeof(INT8U), MSG_PRI_NORMAL);
		OSTimeDly(25);
	}
	else
	{
		while (mode_cur != mode)
		{ 
			INT32U i = 0;
			INT32S mode_cur_bak = mode_cur;

			g_mode_falg = mode;
			//DBG_PRINT("MODE_ACTIVE\r\n");
			msgQSend(ApQ, MSG_APQ_MODE, NULL, NULL, MSG_PRI_NORMAL);
			printf("MODE CHANGES FROM %d TO %d\r\n",mode_cur,mode);
			for (i = 0; i < SWTICHING_PERIOD; ++i)
			{
				OSTimeDly(5);
				mode_cur = (present_state&0xFF);
				if (mode_cur_bak != mode_cur)
				{
					printf("MODE CHANGES TO %d  NOW\r\n",mode_cur);
					OSTimeDly(10);
					break;
				}
			}
			if (i == SWTICHING_PERIOD)	// time out
			{
				ret = -1;
				mode_bak = (present_state&0xFF);
				printf("MODE CHANGE FAIL (%d)\r\n",mode_bak);
				goto GP_MODE_SET_END;
			}
		}
		printf("MODE changes finish(%d)\r\n",mode);
	}

	mode_bak = mode;

GP_MODE_SET_END:
	return ret;
}


static socket_cmd_ctl_t socket_cmd_ctl_blk =
{
	NULL,				/* Server connection */
	NULL,				/* Client connection */
};

static INT8U socket_cmd_tx_buf[SOCKET_CMD_BUF_SIZE] = {0};
static INT8U socket_cmd_rx_buf[SOCKET_CMD_BUF_SIZE] = {0};
static INT32U socket_cmd_stack[SOCKET_CMD_TASK_STACKSIZE];

//
// Purpose: Prepare the server->client response buffer
// Return: Length of the response buffer
// Note:
//
// Server -> client response format:
//
// Low byte                                                                  High byte
// +-------------+--------------+-----------------+----------------+-----------------+
// | GP_SOCK_Tag | GP_SOCK_Type | GP_SOCK_Mode_ID | GP_SOCK_Cmd_ID | GP_SOCK_Payload |
// +-------------+--------------+-----------------+----------------+-----------------+
//      8byte         2byte           1byte              1byte            Nbyte
//
// GP_SOCK_Type   : 2(ACK), 3(NAK)
// GP_SOCK_Mode_ID: Same as client packet
// GP_SOCK_Cmd_ID : Same as client packet
// GP_SOCK_Payload(ACK): Byte[0]~[1]: Payload size
//                       Byte[2]~[N]: Payload data
// GP_SOCK_Payload(NAK): Byte[0]~[1]: Error code
//
static INT32S gp_resp_set(INT32U type, INT16U payload_attr, INT8U *payload_data, INT32U payload_len)
{
	INT8U *resp_type = (INT8U *)(socket_cmd_tx_buf+GP_SOCK_RESP_TYPE_OFFSET);
	INT16U *resp_payload_attr = (INT16U *)(socket_cmd_tx_buf+GP_SOCK_RESP_PAYLOAD_SIZE_OFFSET);
	CHAR *resp_payload_data = (CHAR *)(socket_cmd_tx_buf+GP_SOCK_RESP_PAYLOAD_OFFSET);
	CHAR *p = (CHAR *)(&type);

	// Write "GP_SOCK_Type + GP_SOCK_Mode_ID + GP_SOCK_Cmd_ID"
	// endian convert
	resp_type[0] = p[2];
	resp_type[1] = p[3];
	resp_type[2] = p[1];
	resp_type[3] = p[0];

	// Write "GP_SOCK_Payload"
	*resp_payload_attr = payload_attr;
	gp_memcpy((INT8S*)resp_payload_data,(INT8S*)payload_data, (INT32U)payload_len);

	return (payload_len+GP_SOCK_RESP_PAYLOAD_OFFSET);
}

static INT32S GPSOCK_General_SetMode_Decode(INT16U gp_sock_cmd, socket_cmd_ctl_t *ctl_blk, INT8U* rxstring)
{
	INT32S ret = NAK_OK;
	INT32S mode = 0;

	// Byte 0 of SetMode's payload is 'mode'
	mode = (INT32S)rxstring[GP_SOCK_CMD_PAYLOAD_OFFSET];
	DBG_PRINT("\r\nSetMode...[%d]\r\n", mode);
	gp_mode_set(mode);
     
	return ret;
}

static int General_GetParameterFile_Decode(unsigned short gp_sock_cmd, socket_cmd_ctl_t *ctl_blk, INT8U* rxstring)
{
	INT32S ret = NAK_OK;
	INT16S* pGP_Cmd = (INT16S *)(socket_cmd_tx_buf+GP_SOCK_CMD_TYPE_OFFSET);
	INT8U  *pGP_ID = (INT8U *)(socket_cmd_tx_buf+GP_SOCK_CMD_MODE_ID_OFFSET);
	INT8U  *p = (INT8U*)(&gp_sock_cmd);
	INT32S xml_len;
	INT16U xml_fd;
	
	if (ui_language != 1)
		xml_fd = nv_open((INT8U *) "WIFI_EN.XML");
	else
		xml_fd = nv_open((INT8U *) "WIFI_CN.XML");
	
	if (xml_fd == 0xFFFF)
	{
		ret = -1;
		printf("Open XML(Wifi configuration) Error\r\n");
		goto CMD_GETPARAMETERFILE_DECODE_END;
	}

	xml_len = (INT32S)nv_rs_size_get(xml_fd);
	*pGP_Cmd = GP_SOCK_TYPE_ACK;
	*pGP_ID++ = p[1];		// MODE
	*pGP_ID++ = p[0];		// CMD_ID
	pGP_Cmd += 2;	// point to payload
	printf("xml_len = %d, %d, %d\r\n",xml_len, p[1], p[0]);
	while (xml_len > 0)
	{
		INT32S err = 0;
		int flag = 0;

		xml_len -= (SOCKET_CMD_BUF_SIZE-GP_SOCK_RESP_PAYLOAD_OFFSET);
		if (xml_len > 0)
		{
			*pGP_Cmd = (short)(SOCKET_CMD_BUF_SIZE-GP_SOCK_RESP_PAYLOAD_OFFSET);		// continuous frame
			flag = nv_read(xml_fd,  (INT32U)(socket_cmd_tx_buf+GP_SOCK_RESP_PAYLOAD_OFFSET), SOCKET_CMD_BUF_SIZE-GP_SOCK_RESP_PAYLOAD_OFFSET);
		}
		else
		{
			*pGP_Cmd = (short)(xml_len + SOCKET_CMD_BUF_SIZE-GP_SOCK_RESP_PAYLOAD_OFFSET);		// last frame
			flag = nv_read(xml_fd,  (INT32U)(socket_cmd_tx_buf+GP_SOCK_RESP_PAYLOAD_OFFSET), (xml_len + SOCKET_CMD_BUF_SIZE-GP_SOCK_RESP_PAYLOAD_OFFSET));
		}
		// printf("flag = %d\r\n",flag);

		err = socket_cmd_wifi_send_data(ctl_blk, socket_cmd_tx_buf, (*pGP_Cmd+GP_SOCK_RESP_PAYLOAD_OFFSET));
		if(err != SC_RET_SUCCESS)
		{
			printf("xml get error\r\n");
		}
	}
	printf("xml  finish\r\n");

CMD_GETPARAMETERFILE_DECODE_END:
	return ret;
}

extern INT8U ap_sd_status_get(void);
static int General_GetDeviceStatus_Decode(unsigned short gp_sock_cmd, socket_cmd_ctl_t *ctl_blk, INT8U* rxstring, unsigned char *buf)
{
	int ret = NAK_OK;
	int val;
	char *p_val = (char *)(&val);
	char adp = 0;
	unsigned char flag = 0;
	char device_status = 0;
   	INT32U t;

	// buf[1]:bit0
	if (present_state == STATE_VIDEO_RECORD)	// video mode
	{
		if (video_record_sts & VIDEO_RECORD_BUSY)
			device_status = 1;
	}
	else if (present_state == STATE_VIDEO_PREVIEW) //captuer mode
	{
		//if (g_wifi_notify_pic_done != 0)
		//	device_status = 1;
	}
	else if (present_state == STATE_AUDIO_RECORD)	// record mode
	{
		//if (video_record_sts & 0x2)
			device_status = 1;
	}
	else if (present_state == STATE_BROWSE)		// browser mode
	{
		if (browse_sts & BROWSE_PLAYBACK_BUSY)
			device_status = 1;
	}
	// buf[1]:bit1
	flag = ap_state_config_voice_record_switch_get();
	if (flag == 1)
	{
		device_status |= 0x2;
	}
#if VLC_RESET_NOTIFICATION
	// buf[1]:bit7
	if ( mjpeg_service_ready_flag )
	{
		vlc_reset_timer = 0;
	}
	else if  ( (mjpeg_service_ready_flag==0)&&(vlc_reset_timer == GoPlusCam_VLC_TIMEOUT) )
	{
		device_status |= 0x80;
		DBG_PRINT("Notify GoPlus Cam to reset VLC\r\n");
		vlc_reset_timer++;
	}
	else
	{
		vlc_reset_timer++;
		DBG_PRINT("vlc_reset_timer=%d\r\n",vlc_reset_timer);
	}
#endif

	// buf[3]
	if (adp_status == 1)
		adp = 1;

	//if (g_wifi_pic_work) device_status |= 0x04; //拍照标志

	buf[0] = (unsigned char)(present_state);		// current mode
	buf[1] = device_status;
	buf[2] = bat_lvl_cal_bak;		// battery level
	buf[3] = adp;

	/////////// Add recording status 
	ap_display_timer();
	val = ap_display_timer_rec_time_get();
	buf[4] = ap_state_config_video_resolution_get();
	buf[5] = p_val[0];
	buf[6] = p_val[1];
	buf[7] = p_val[2];
	buf[8] = p_val[3];
	//printf("%d ",val);
	
	/////////// Add capture status 
	val = left_capture_num;
	buf[9] = ap_state_config_pic_size_get();
	buf[10] = p_val[0];
	buf[11] = p_val[1];
	buf[12] = p_val[2];
	buf[13] = p_val[3];
	//liuxi add
	buf[14] = ap_sd_status_get(); //SD卡状态
	buf[15] = 0;
	val = g_sd_size;//TF卡剩余容量
	buf[16] = p_val[0]; 
	buf[17] = p_val[1]; 
	buf[18] = p_val[2]; 
	buf[19] = p_val[3];	

	return ret;
}

static int GPSOCK_General_PowerOff_Decode(unsigned short gp_sock_cmd, socket_cmd_ctl_t *ctl_blk, INT8U* rxstring)
{
	INT32U type;
	
	type = 1;
	msgQSend(ApQ, MSG_APQ_POWER_KEY_ACTIVE, &type, sizeof(INT32U), MSG_PRI_NORMAL);
	return NAK_OK;
}

static int General_RestarStreaming_Decode(unsigned short gp_sock_cmd, socket_cmd_ctl_t *ctl_blk, INT8U* rxstring)
{
	int ret = NAK_Request_timeout;
	int i;

	for (i = 0; i < 20; ++i)
	{
		if (mjpeg_service_ready_flag)
		{
			ret = NAK_OK;
			break;
		}
		printf("General_RestarStreaming_Decode\r\n");
		OSTimeDly(10);
	}

	//ret = NAK_OK;
	OSTimeDly(30);	// ňゎ繵羉ち mode 硑Θぃタ盽

#if PREVIEW_TCPIP
#else
	rtp_rtsp_cbk(RTP_START_EVENT);
#endif

#if VLC_RESET_NOTIFICATION
	vlc_reset_timer = 0;
#endif

	return ret;
}

static int GPSOCK_General_AuthDevice_Decode(unsigned short gp_sock_cmd, socket_cmd_ctl_t *ctl_blk, INT8U* rxstring, unsigned char *buf)
{
	INT8U *seed = (INT8U*)(&rxstring[GP_SOCK_CMD_PAYLOAD_OFFSET]);
	__here__;
	g_LFSRseed[0] = seed[0];
	g_LFSRseed[1] = seed[1];
	g_LFSRseed[2] = seed[2];
	g_LFSRseed[3] = seed[3];
	Gen_LFSR_32Bit_Key();
	buf[0] = g_LFSR_Key[0];
	buf[1] = g_LFSR_Key[1];
	buf[2] = g_LFSR_Key[2];
	buf[3] = g_LFSR_Key[3];
	buf[4] = g_LFSR_Key[4];
	buf[5] = g_LFSR_Key[5];
	return 0;
}

static int GPSOCK_Record_Start_Decode(unsigned short gp_sock_cmd, socket_cmd_ctl_t *ctl_blk, INT8U* rxstring)
{
	INT32S ret = NAK_OK;
	//INT16U cnt = 0; //128;

//APP 端只需要发送录像命令即可, 在其它模式下不需要单独切换模式
/*
	if (present_state != STATE_VIDEO_RECORD)
	{
		ret = NAK_Mode_error;
		goto GPSOCK_RECORD_START_DECODE_END;
	}
*/
	if ((video_record_sts & VIDEO_RECORD_UNMOUNT)||(ap_state_handling_storage_id_get() == 0xFF))
	{
		ret = NAK_No_storage;
		goto GPSOCK_RECORD_START_DECODE_END;
	}

	if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
	{
		INT32U record_stop_flag = 0;	// record start
	
		if (video_record_sts & VIDEO_RECORD_BUSY)
		{
			record_stop_flag = 1;		// record stop
		}
	
		g_wifi_sd_full_flag = 0;
		//ap_peripheral_ok_key_exe(&cnt);
        DBG_PRINT("WIFI ACTIVE VIDEO...\r\n");
        if ((video_busy_cnt_get() >= 128*2)&& (Image_Processing_Busy() == 1))
        {
	    	DBG_PRINT("WIFI ACTIVE VIDEO...\r\n");
			msgQSend(ApQ, MSG_APQ_VIDEO_RECORD_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
			video_busy_cnt_clr();
        }
		if (record_stop_flag == 0)	// record start
		{
		OSTimeDly(80);
			printf("Wi-Fi Record Start\r\n", ret);	
		}
		else		// record stop
		{
			INT32U i;
			for (i=0;i<10;++i)
			{
				OSTimeDly(50);
				if ( (video_record_sts & VIDEO_RECORD_BUSY) == 0)
				{
					break;
				}
			}
			if (i==10)
			{
				DBG_PRINT("record stop abnormal\r\n");
			}
		       OSTimeDly(10);
			printf("Wi-Fi Record Stop\r\n", ret);				
		}
		if (g_wifi_sd_full_flag==1)
		{
			ret = NAK_Full_storage;
			g_wifi_sd_full_flag = 0;
			DBG_PRINT("[Record] SD Card Full\r\n");
		}
	}

GPSOCK_RECORD_START_DECODE_END:

	return ret;
}

static int GPSOCK_Record_Audio_Decode(unsigned short gp_sock_cmd, socket_cmd_ctl_t *ctl_blk, INT8U* rxstring)
{
	INT32S ret = NAK_OK;
	INT8U flag = 0;
	if (present_state != STATE_VIDEO_RECORD)
	{
		ret = NAK_Mode_error;
		goto GPSOCK_RECORD_AUDIO_DECODE_END;
	}

	flag = ap_state_config_voice_record_switch_get();
	if(flag)
	{
		ap_state_config_voice_record_switch_set(0);
	}
	else
	{
		ap_state_config_voice_record_switch_set(1);
	}

GPSOCK_RECORD_AUDIO_DECODE_END:
	return ret;
}

static int GPSOCK_CapturePicture_Capture_Decode(unsigned short gp_sock_cmd, socket_cmd_ctl_t *ctl_blk, INT8U* rxstring)
{
	INT32S ret = NAK_OK;
	INT16U cnt = 0;
	INT32U i;
	
//APP 端只需要发送拍照命令即可, 在其它模式下不需要单独切换模式
/*
	if (present_state != STATE_VIDEO_PREVIEW)
	{
		ret = NAK_Mode_error;
		goto CAPTUREPICTURE_CAPTURE_DECODE_END;
	}
*/
	if (ap_state_handling_storage_id_get() == 0xFF)
	{
		ret = NAK_No_storage;
		goto CAPTUREPICTURE_CAPTURE_DECODE_END;
	}

	if  (ap_state_config_burst_get())	// 连拍
	{
		//ap_peripheral_pw_key_exe(&cnt);
		if (Image_Processing_Busy())
		{
			DBG_PRINT("ACTIVE CAPTURE...\r\n");
			msgQSend(ApQ, MSG_APQ_CAPTURE_KEY_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
		}
		//ap_peripheral_ok_key_exe(&cnt);
		if (g_wifi_notify_pic_done == 0)	// 秨﹍秨﹍
		{
			printf("Sequence Start\r\n");
			for (i = 0; i < CAPTURE_PERIOD; ++i)
			{
				if  ( g_wifi_notify_pic_done )
				{
					break;
				}
				OSTimeDly(2);		// one picture per 3 seconds
			}
		}
		else		// 硈╃挡
		{
			for (i = 0; i < (CAPTURE_PERIOD); ++i)
			{
				if  ( g_wifi_notify_pic_done == 0xFFFFFFFF )
				{
					printf("Sequence Stop\r\n");
					break;
				}
				OSTimeDly(2);		// one picture per 3 seconds
			}
			OSTimeDly(100);		// Τ程眎临⊿╃Ч玂臔 g_wifi_notify_pic_done ﹚ 0
			g_wifi_notify_pic_done = 0;
		}
	}
	else		// 虫╃
	{
		g_wifi_notify_pic_done = 0;	// trigger
		g_wifi_sd_full_flag = 0;
		//ap_peripheral_ok_key_exe(&cnt);
		//ap_peripheral_pw_key_exe(&cnt);
		if (Image_Processing_Busy())
		{
			DBG_PRINT("ACTIVE CAPTURE...\r\n");
			msgQSend(ApQ, MSG_APQ_CAPTURE_KEY_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
		}
		printf("Capture_Start\r\n");

		for (i = 0; i < CAPTURE_PERIOD; ++i)
		{
			if  ( g_wifi_notify_pic_done )
			{
				printf("Capture Done !!\r\n");
				g_wifi_notify_pic_done = 0;
				break;
			}
			OSTimeDly(2);		// one picture per 3 seconds
		}
		if (g_wifi_sd_full_flag==1)
		{
			ret = NAK_Full_storage;
			g_wifi_sd_full_flag = 0;
			DBG_PRINT("[Capture] SD Card Full\r\n");		
		}			
	}


	if ( (i == CAPTURE_PERIOD)&&(ret==NAK_OK) )
	{
		printf("Capture Time Out\r\n");
		ret = NAK_Request_timeout;
	}

CAPTUREPICTURE_CAPTURE_DECODE_END:
	return ret;
}

#define PLAYBACK_START_TIME_OUT 500
static int GPSOCK_Playback_Start_Decode(unsigned short gp_sock_cmd, socket_cmd_ctl_t *ctl_blk, INT8U* rxstring)
{
	INT32U i;
	INT32S ret = NAK_OK;
	//INT16U cnt = 0;
	INT16U file_index = 0;

	if (present_state != STATE_BROWSE)
	{
		ret = NAK_Mode_error;
		goto GPSOCK_PLAYBACK_START_DECODE_END;
	}

	if (browse_sts & BROWSE_UNMOUNT)
	{
		ret = NAK_No_storage;
	}

	mjpeg_service_playjpeg_flag = 0;
	file_index = rxstring[13]<<8 | rxstring[12];
	ap_browse_wifi_file_index_set(file_index);
	//ap_peripheral_ok_key_exe(&cnt);
	DBG_PRINT("PLAYINT...\r\n");
	msgQSend(ApQ, MSG_APQ_FUNCTION_KEY_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
	for (i = 0; i < PLAYBACK_START_TIME_OUT; ++i)
	{
		if (mjpeg_service_playjpeg_flag)
			break;
		OSTimeDly(1);
	}
	if (i == PLAYBACK_START_TIME_OUT)
		ret = NAK_Invalid_command;

GPSOCK_PLAYBACK_START_DECODE_END:

	printf("Playback_Start .... (%d)\r\n", ret);

	return ret;
}

static int GPSOCK_Playback_Pause_Decode(unsigned short gp_sock_cmd, socket_cmd_ctl_t *ctl_blk, INT8U* rxstring)
{
	INT32S ret = NAK_OK;
	//INT16U cnt = 0;

	if (rxstring[12] == 'J')
	{
		// JPEG 郎
		//ap_peripheral_ok_key_exe(&cnt);
		DBG_PRINT("PLAYINT...\r\n");
		msgQSend(ApQ, MSG_APQ_FUNCTION_KEY_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
		//ap_peripheral_ok_key_exe(&cnt);
		OSTimeDly(1);
		msgQSend(ApQ, MSG_APQ_FUNCTION_KEY_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
		printf("GPSOCK_JPG_Pause_Resume\r\n");
	}
	else
	{
		// AVI 郎
		msgQSend(ApQ, MSG_APQ_MJPEG_DECODE_PAUSE, NULL, NULL, MSG_PRI_NORMAL);
		printf("GPSOCK_AVI_Pause_Resume\r\n");
	}

	return ret;
}

static int Menu_GetParameter_Decode(unsigned short gp_sock_cmd, socket_cmd_ctl_t *ctl_blk, INT8U* rxstring,  unsigned char *buf)
{
	int ret = 1;
	INT32U t;
	int *pID = (int*)(&rxstring[GP_SOCK_CMD_PAYLOAD_OFFSET]);
	INT8U ssid_length;

	switch (*pID)
	{
	// ===============================================
	// RECORD
	// ===============================================
	case Record_Resolution:
		buf[0] = ap_state_config_video_resolution_get();
		break;
	case Record_Exposure:
		buf[0] = ap_state_config_ev_get();
		break;
	case Record_MotionDetection:
		buf[0] = ap_state_config_md_get();
		break;
	case Record_Loop_Recording:
		buf[0] = ap_state_config_record_time_get();
		break;
	case Record_WDR:
		buf[0] = ap_state_config_wdr_get();
		break;
	case Record_RecordAudio:
		buf[0] = ap_state_config_voice_record_switch_get();
		break;
	case Record_DateStamp:
		buf[0] = ap_state_config_date_stamp_get();
		break;
	// ===============================================
	// CAPTURE
	// ===============================================
	case Capture_Resolution:
		buf[0] = ap_state_config_pic_size_get();
		break;
	case Capture_Exposure:
		buf[0] = ap_state_config_ev1_get();
		break;
	case Capture_Quality:
		buf[0] = ap_state_config_quality_get();
		break;
	case Capture_Sequence:
		buf[0] = ap_state_config_burst_get();
		break;
	case Capture_Sharpness:
		buf[0] = ap_state_config_sharpness_get();
		break;
	case Capture_ISO:
		buf[0] = ap_state_config_iso_get();
		break;
	case Capture_AntiShaking:
		buf[0] = ap_state_config_anti_shaking_get();
		break;
	case Capture_DateTime:
		buf[0] = ap_state_config_capture_date_stamp_get();
		break;
	case Capture_WhiteBalance:
		buf[0] = ap_state_config_white_balance_get();
		break;
	// ===============================================
	// SYSTEM
	// ===============================================
	case System_Frequency:
		buf[0] = ap_state_config_light_freq_get();
		break;
	case System_ScreenSaver:
		buf[0] = ap_state_config_auto_off_TFT_BL_get();
		break;
	case System_AutoPowerOff:
		buf[0] = ap_state_config_auto_off_get();
		break;
	case System_Language:
		// The stored value is actual menu value + 1.
		// We have to minus 1 to get the original menu value.
		buf[0] = ap_state_config_language_get() - 1;
		break;
	case System_BeepSound:
		buf[0] = ap_state_config_beep_sound_get();
		break;
	case System_DataTime:
		buf[0] = ap_state_config_data_time_mode_get();
		break;
//        case System_TVMode:
//            buf[0] = ap_state_config_tv_out_get();
//            break;
	case WiFi_Name:
		for(ssid_length=0;ssid_length<GPSOCK_WiFi_Name_Length;ssid_length++)
		{
			if(wifi_ssid[ssid_length]==0)
			{
				break;
			}
		}
		gp_memcpy( (INT8S*)buf, (INT8S *)wifi_ssid, (INT32U)ssid_length);//GPSOCK_WiFi_Name_Length
		ret = ssid_length;//GPSOCK_WiFi_Name_Length
		break;
	case WiFi_Passwd:
		gp_memcpy( (INT8S*)buf, (INT8S *)wifi_password, GPSOCK_WiFi_Passwd_Length);
		ret = GPSOCK_WiFi_Passwd_Length;
		break;
	case System_ClearBuffer:
	case System_Format:
	case System_DefaultSetting:
	case System_Version:
		 break;
	case Firmware_Version:
		gp_memcpy( (INT8S*)buf, (INT8S *)system_version, GPSOCK_System_Version_Length);
		ret = GPSOCK_System_Version_Length;
		break;
	case System_SyncTime:
		break;	
	default:
		buf[0] = 0;
	}
	printf("ID = 0x%x  Data:0x%x\r\n",*pID,buf[0]);

	return ret;
}

static INT32S handle_system_format(void)
{
	INT32S ret;

	if(ap_state_handling_storage_id_get() == NO_STORAGE)
	{
		ret = NAK_No_storage;
		printf("no storage to format\r\n");
	}
	else
	{
		printf("format start...\r\n");
		ap_storage_service_format_req(FALSE);
		printf("format done\r\n");
		ret = 0;
	}
	gp_sd_size_set(vfsFreeSpace(MINI_DVR_STORAGE_TYPE) >> 20);

	return ret;
}

enum
{
	MENU_FUNCTION_ACTIVE = 0x2,
	SetParameterDataOffset = 0x5
};

static int System_Sync_Time(INT8U* rxstring)
{
	INT32S ret = NAK_SyncTime_error;
	TIME_T	time_set;
	INT8U *p = (INT8U *)(rxstring+GP_SOCK_CMD_PAYLOAD_OFFSET+SetParameterDataOffset+1);
	INT8U size = *((INT8U *)(rxstring+GP_SOCK_CMD_PAYLOAD_OFFSET+SetParameterDataOffset));
	int i;
	
	if (size > GPSOCK_Sync_Time_Buf_Length)
	{
		size = GPSOCK_Sync_Time_Buf_Length;
		goto GPSOCK_Sync_Time_END;
	}

	if ((p[0] < 16)||(p[0] > 26)) goto GPSOCK_Sync_Time_END;
	time_set.tm_year = p[0]+2000;
	if (p[1] > 12) goto GPSOCK_Sync_Time_END;
	time_set.tm_mon = p[1];
	if (p[2] > 31) goto GPSOCK_Sync_Time_END;
	time_set.tm_mday = p[2];
	if (p[3] > 23) goto GPSOCK_Sync_Time_END;
	time_set.tm_hour = p[3];
	if (p[4] > 59) goto GPSOCK_Sync_Time_END;
	time_set.tm_min = p[4];
	if (p[5] > 59) goto GPSOCK_Sync_Time_END;
	time_set.tm_sec = p[5];
		
	cal_time_set(time_set);
	ap_state_handling_calendar_init();
	cal_time_get(&time_set);
	DBG_PRINT("\r\n%04d-%02d-%02d  %02d:%02d:%02d\r\n", time_set.tm_year,time_set.tm_mon,time_set.tm_mday,time_set.tm_hour,time_set.tm_min,time_set.tm_min);
	
	ret = NAK_OK;
	
	GPSOCK_Sync_Time_END:
	return ret;
}

static int Menu_SetParameter_Decode(unsigned short gp_sock_cmd, socket_cmd_ctl_t *ctl_blk, INT8U* rxstring)
{
	int ret = NAK_OK;
	int id = *((int*)(&rxstring[GP_SOCK_CMD_PAYLOAD_OFFSET]));
	unsigned char value = rxstring[GP_SOCK_CMD_PAYLOAD_OFFSET+SetParameterDataOffset];
	STRING_INFO str;

	printf("set:0x%x, 0x%x\r\n",id,value);
	str.font_color = (INT16U)value;
	switch (id)
	{
	// ===============================================
	// RECORD
	// ===============================================
	case Record_Resolution:
		DBG_PRINT("wifi *** STR_RESOLUTION\r\n");
		str.str_idx = STR_RESOLUTION;
		ap_setting_right_menu_active(&str, MENU_FUNCTION_ACTIVE, (INT8U *)NULL);
		// ap_state_config_video_resolution_set(value);
		break;
	case Record_Exposure:
		str.str_idx = STR_EV;
		ap_setting_right_menu_active(&str, MENU_FUNCTION_ACTIVE, (INT8U *)NULL);
		// ap_state_config_ev_set(value);
		break;
	case Record_MotionDetection:
		str.str_idx = STR_MOTIONDETECT;
		ap_setting_right_menu_active(&str, MENU_FUNCTION_ACTIVE, (INT8U *)NULL);
		// ap_state_config_md_set(value);
		break;
	case Record_Loop_Recording:
		str.str_idx = STR_LOOPRECORDING;
		ap_setting_right_menu_active(&str, MENU_FUNCTION_ACTIVE, (INT8U *)NULL);
		// ap_state_config_record_time_set(value);
		break;
	case Record_WDR:
		str.str_idx = STR_WDR;
		ap_setting_right_menu_active(&str, MENU_FUNCTION_ACTIVE, (INT8U *)NULL);
		// ap_state_config_wdr_set(value);
		break;
	case Record_RecordAudio:
		str.str_idx = STR_RECORD_AUDIO;
		ap_setting_right_menu_active(&str, MENU_FUNCTION_ACTIVE, (INT8U *)NULL);
		// ap_state_config_voice_record_switch_set(value);
		break;
	case Record_DateStamp:
		str.str_idx = STR_DATE_STAMP;
		ap_setting_right_menu_active(&str, MENU_FUNCTION_ACTIVE, (INT8U *)NULL);
		// ap_state_config_date_stamp_set(value);
		break;
	// ===============================================
	// CAPTURE
	// ===============================================
	case Capture_Resolution:
		str.str_idx = STR_RESOLUTION2;
		ap_setting_right_menu_active(&str, MENU_FUNCTION_ACTIVE, (INT8U *)NULL);
		// ap_state_config_pic_size_set(value);
		break;
	case Capture_Exposure:
		str.str_idx = STR_EV;
		ap_setting_right_menu_active(&str, MENU_FUNCTION_ACTIVE, (INT8U *)NULL);
		// ap_state_config_ev1_set(value);
		break;
	case Capture_Quality:
		str.str_idx = STR_QUALITY;
		ap_setting_right_menu_active(&str, MENU_FUNCTION_ACTIVE, (INT8U *)NULL);
		// ap_state_config_quality_set(value);
		break;
	case Capture_Sequence:
		str.str_idx = STR_SEQUENCE;
		ap_setting_right_menu_active(&str, MENU_FUNCTION_ACTIVE, (INT8U *)NULL);
		// ap_state_config_burst_set(value);
		break;
	case Capture_Sharpness:
		str.str_idx = STR_SHARPNESS;
		ap_setting_right_menu_active(&str, MENU_FUNCTION_ACTIVE, (INT8U *)NULL);
		// ap_state_config_sharpness_set(value);
		break;
	case Capture_ISO:
		str.str_idx = STR_ISO;
		ap_setting_right_menu_active(&str, MENU_FUNCTION_ACTIVE, (INT8U *)NULL);
		// ap_state_config_iso_set(value);
		break;
	case Capture_AntiShaking:
		str.str_idx = STR_ANTI_SHAKING;
		ap_setting_right_menu_active(&str, MENU_FUNCTION_ACTIVE, (INT8U *)NULL);
		// ap_state_config_anti_shaking_set(value);
		break;
	case Capture_DateTime:
		str.str_idx = STR_DATE_STAMP;
		ap_setting_right_menu_active(&str, MENU_FUNCTION_ACTIVE, (INT8U *)NULL);
		// ap_state_config_capture_date_stamp_set(value);
		break;
	case Capture_WhiteBalance:
		str.str_idx = STR_WHITE_BALANCE;
		ap_setting_right_menu_active(&str, MENU_FUNCTION_ACTIVE, (INT8U *)NULL);
		// ap_state_config_white_balance_set(value);
		break;
	// ===============================================
	// SYSTEM
	// ===============================================
	case System_Frequency:
		str.str_idx = STR_FREQUENCY;
		ap_setting_right_menu_active(&str, MENU_FUNCTION_ACTIVE, (INT8U *)NULL);
		// ap_state_config_light_freq_set(value);
		break;
	case System_ScreenSaver:
		str.str_idx = STR_SCREEN_SAVER;
		ap_setting_right_menu_active(&str, MENU_FUNCTION_ACTIVE, (INT8U *)NULL);
		// ap_state_config_auto_off_TFT_BL_set(value);
		break;
	case System_AutoPowerOff:
		str.str_idx = STR_AUTO_POWER_OFF;
		ap_setting_right_menu_active(&str, MENU_FUNCTION_ACTIVE, (INT8U *)NULL);
		// ap_state_config_auto_off_set(value);
		break;
	case System_Language:
		str.str_idx = STR_LANGUAGE;
		ap_setting_right_menu_active(&str, MENU_FUNCTION_ACTIVE, (INT8U *)NULL);
		// ap_state_config_language_set(value);
		break;
	case System_BeepSound:
		str.str_idx = STR_BEEP_SOUND;
		ap_setting_right_menu_active(&str, MENU_FUNCTION_ACTIVE, (INT8U *)NULL);
		// ap_state_config_beep_sound_set(value);
		break;
	case System_DataTime:
		ap_state_config_data_time_mode_set(value);
		

		break;
//        case System_TVMode:
//            ap_state_config_tv_out_set(value);
//            break;
	case System_ClearBuffer:
		break;
	case System_Format:
		ret = handle_system_format();
		break;
	case System_DefaultSetting:
		str.str_idx = STR_DEFAULT_SETTING;
		str.font_color = (INT16U)1;
		ap_setting_right_menu_active(&str, MENU_FUNCTION_ACTIVE, (INT8U *)NULL);
		// ap_state_config_default_set();
		break;
	case System_Version:
		break;
	case WiFi_Name:
	{
		int i;
		char *p = (char *)(rxstring+GP_SOCK_CMD_PAYLOAD_OFFSET+SetParameterDataOffset);
		gp_memcpy((INT8S*)wifi_ssid, (INT8S*)p, GPSOCK_WiFi_Name_Length);
		ap_state_config_wifi_ssid_set((INT8S *)wifi_ssid);
		for     (i = 0; i < GPSOCK_WiFi_Name_Length; ++i)
		{
			printf("%c",wifi_ssid[i]);
			//printf("%x,",wifi_ssid[i]);
		}
		printf("\r\n");
	}
	break;
	case WiFi_Passwd:
	{
		int i;
		char *p = (char *)(rxstring+GP_SOCK_CMD_PAYLOAD_OFFSET+SetParameterDataOffset);
		gp_memcpy((INT8S*)wifi_password, (INT8S*)p, GPSOCK_WiFi_Passwd_Length);
		ap_state_config_wifi_pwd_set((INT8S *)wifi_password);
		for     (i = 0; i < GPSOCK_WiFi_Passwd_Length; ++i)
		{
			printf("%c",wifi_password[i]);
		}
		printf("\r\n");
	}
	break;
	case Firmware_Version:
		break;
	case System_SyncTime:
		ret = System_Sync_Time(rxstring);
		if (ret == NAK_OK) DBG_PRINT("System sync time successful!!!\r\n");
		else DBG_PRINT("System sync time failed!!!\r\n");
		break;	
	default:
		ret = NAK_OK;
		break;
	}

	return ret;
}

static int GPSOCK_Playback_GetFileCount_Decode(unsigned short gp_sock_cmd, socket_cmd_ctl_t *ctl_blk, INT8U* rxstring)
{
	INT32S ret = NAK_OK;
	INT32S i;
	INT8U packet_const[12] = {0x47,0x50,0x53,0x4F,0x43,0x4B,0x45,0x54,0x02,0x00,0x03,0x02};

	gp_memcpy((INT8S*)socket_cmd_tx_buf,(INT8S*)packet_const,12);

	// Initial file index talbe
	for (i=0;i<PLAYFILE_INDEX_TABLE_SIZE;++i)
	{
		playfile_index_table[i] = 0xFFFF;
	}
	playfile_index_table[0] = 0xFFFE;	// 1~512  file パ index = 0 秨﹍ま
	
	ret = ap_storage_service_playfile_totalcount(playfile_index_table);
	printf("total  %d  files\r\n",ret);

	if (ret == 0)
	{
		ret = NAK_No_storage;
	}

	return ret;
}

// rxstring : input stream
// socket_cmd_tx_buf : output steram
// return: <0  ===> error
//         >=0 ===> length of output steram
static int GPSOCK_Playback_GetFileList_Decode(unsigned short gp_sock_cmd, socket_cmd_ctl_t *ctl_blk, INT8U* rxstring)
{
	INT32S size = 0;
	INT32S ret = NAK_OK;
	INT16U play_file_index;
	INT8U packet_const[12] = {0x47,0x50,0x53,0x4F,0x43,0x4B,0x45,0x54,0x02,0x00,0x03,0x03};

	gp_memcpy((INT8S*)socket_cmd_tx_buf,(INT8S*)packet_const,12);

	// rxstring[12]=0: Get list by file index
	// rxstring[12]=1: Get list from 1st file
	if(*(rxstring+12) == 1)
	{
		play_file_index = 0xffff;
	}
	else
	{
		play_file_index = *(rxstring+14);
		play_file_index = play_file_index<<8;
		play_file_index |= *(rxstring+13);
		if(play_file_index >= 9999)
		{
			ret = NAK_Invalid_command;
			goto EXIT_GET_FILE_LIST;
		}
	}

	size = (INT32S)ap_storage_service_playfilelist_req(&socket_cmd_tx_buf[12],play_file_index);
	if (size != -1)
	{
		size = (size + GP_SOCK_RESP_PAYLOAD_OFFSET);	// GP_SOCK_Tag(8) + GP_SOCKType(2) + GP_SOCK_MODE_ID_(1) +GP_SOCK_CMD_ID(1) + paylad size filed
	}
	else
	{
		ret = NAK_Get_file_list_fail;
		printf("GetFileList Error!!\r\n");
	}

EXIT_GET_FILE_LIST:
	return size;
}

static int GPSOCK_Playback_GetThumbnail_Decode(unsigned short gp_sock_cmd, socket_cmd_ctl_t *ctl_blk, INT8U* rxstring)
{
	INT32S ret = NAK_OK, sc_ret;
	INT32U size;
	INT8U *buf = 0;
	INT8U packet_const[12] = {0x47,0x50,0x53,0x4F,0x43,0x4B,0x45,0x54,0x02,0x00,0x03,0x04};
	INT16U play_file_index;
	INT16U *p;
	INT8U *p_sock;

	if (mjpeg_service_ready_flag)
	{
		ret = NAK_Server_is_busy;
		goto GPSOCK_PLAYBACK_GETTHUMBNAIL_DECODE_END;
	}
	if ((present_state&0xFF)!=BROWSE_MODE)
	{
		DBG_PRINT("MODE(0x%x) error\r\n",present_state);
		ret = NAK_Server_is_busy;
		goto GPSOCK_PLAYBACK_GETTHUMBNAIL_DECODE_END;
	}

	play_file_index = *(rxstring+13);
	play_file_index = play_file_index<<8;
	play_file_index |= *(rxstring+12);

	DBG_PRINT("Thnail=%d\r\n",play_file_index);
	buf = (INT8U*)ap_storage_service_thumbnail_req(play_file_index, &size);
	if (buf == NULL)
	{
		// maybe remove SD card
		ret = NAK_Invalid_command;
		goto GPSOCK_PLAYBACK_GETTHUMBNAIL_DECODE_END;
	}

	p_sock = (INT8U*)(buf+64-GP_SOCK_RESP_PAYLOAD_OFFSET);
	gp_memcpy((INT8S*)p_sock, (INT8S*)packet_const, 12);
	p = (INT16U*)(buf+64-2);
	*p = (INT16U)size;

	sc_ret = socket_cmd_wifi_send_data(ctl_blk, p_sock, size+GP_SOCK_RESP_PAYLOAD_OFFSET);
	if(sc_ret != SC_RET_SUCCESS)
	{
		ret = NAK_Get_thumbnail_fail;
		printf("thumbnail get error\r\n");
	}
	if (buf)
	{
		gp_free((void*)buf);
	}

GPSOCK_PLAYBACK_GETTHUMBNAIL_DECODE_END:
	DBG_PRINT("Thnail:%d(%d)\r\n",play_file_index,ret);
	return ret;
}

#if SUPPORT_APP_DEL_FILE == CUSTOM_ON
static int GPSOCK_Playback_DelFile_Decode(unsigned short gp_sock_cmd, socket_cmd_ctl_t *ctl_blk, INT8U* rxstring)
{
	INT32S ret = NAK_OK, sc_ret;
	INT8U packet_const[12] = {0x47,0x50,0x53,0x4F,0x43,0x4B,0x45,0x54,0x02,0x00,0x03,0x08};
	INT16U play_file_index;

	gp_memcpy((INT8S*)socket_cmd_tx_buf,(INT8S*)packet_const,12);
	if ((present_state&0xFF)!=BROWSE_MODE)
	{
		DBG_PRINT("MODE(0x%x) error\r\n",present_state);
		ret = NAK_Server_is_busy;
		goto GPSOCK_PLAYBACK_DELFILE_DECODE_END;
	}

	play_file_index = *(rxstring+13);
	play_file_index = play_file_index<<8;
	play_file_index |= *(rxstring+12);

	//DBG_PRINT("Thnail=%d\r\n",play_file_index);
	sc_ret = ap_storage_service_given_file_del(play_file_index);
	if (sc_ret == -1)
	{
		// maybe remove SD card
		ret = NAK_Invalid_command;
		goto GPSOCK_PLAYBACK_DELFILE_DECODE_END;
	}

GPSOCK_PLAYBACK_DELFILE_DECODE_END:
	DBG_PRINT("Del:%d(%d)\r\n",play_file_index,ret);
	return ret;
}
#endif

#if GP_SOCK_DOUBLE_BUFFER_DOWNLOAD

#define DOWNLOAD_RETRY 1000
#define DOWNLOAD_SD_BLOCK_SIZE (2*1024*1024)
#define DOWNLOAD_PACKET_SIZE (60*1024)
// ヘ玡 CMD  payload size 程や 64KB
// SD Ω弄 150KB矗硉礛–50KB  packet commandτ– packetwi-fi drvier –Ω癳 1.5KB

static INT32U netwr_t;
INT32U wifi_dl_cur_size;
INT32U curr_wifi, curr_buf;
INT32U buf_dl_flag[2];
INT32U buf_dl_size[2];
INT32U wifi_dl_state;
INT32U wifi_dl_sd_get_size;
INT8U *wifi_dl_buf_A;
INT8U *wifi_dl_buf_B;
INT16U wifi_dl_play_file_index;
STOR_SERV_DOWNLOAD_FILEINFO download_file_info;

static INT32S Playback_GetRawData(socket_cmd_ctl_t *ctl_blk, INT8U *buf, INT32U sd_get_size)
{
	INT32U sd_get_idx = 0;
	INT8U *pPacketbuf;
	INT8U *pBuf;	// download current buffer
	INT16U *pPacksize;		// packet size
	INT16U available;
	INT32U cur_t,str_t;
	INT32S ret;
	INT32S err = NAK_OK;
	INT8U packet_const[12] = {0x47,0x50,0x53,0x4F,0x43,0x4B,0x45,0x54,0x02,0x00,0x03,0x05};	

	sd_get_idx = 0;
	// each DOWNLOAD_PACKET_SIZE
	while (sd_get_size != 0)
	{
		INT32U retry_cnt = DOWNLOAD_RETRY;
		INT32U wlen, tlen, size;

		if(sd_get_idx==0)
		{
			pPacketbuf = (INT8U*)(buf + 64-GP_SOCK_RESP_PAYLOAD_OFFSET);
			sd_get_idx++;
			pBuf = (INT8U*)pPacketbuf;
			gp_memcpy((INT8S*)pBuf,(INT8S*)packet_const,sizeof(packet_const) );
			if (sd_get_size > (DOWNLOAD_PACKET_SIZE-64+GP_SOCK_RESP_PAYLOAD_OFFSET))
			{
				size = DOWNLOAD_PACKET_SIZE-64+GP_SOCK_RESP_PAYLOAD_OFFSET;
				sd_get_size -= size;
			}
			else
			{
				size = sd_get_size;
				sd_get_size = 0;
			}
			pPacksize = (INT16U*)(pBuf+GP_SOCK_RESP_PAYLOAD_SIZE_OFFSET);
			*pPacksize = size;
			tlen = size+GP_SOCK_RESP_PAYLOAD_OFFSET;
		}
		else
		{
			pPacketbuf = (INT8U*)(buf + sd_get_idx*DOWNLOAD_PACKET_SIZE);
			sd_get_idx++;
			pBuf = (INT8U*)pPacketbuf;
			gp_memcpy((INT8S*)pBuf,(INT8S*)packet_const,sizeof(packet_const) );
			if (sd_get_size > DOWNLOAD_PACKET_SIZE)
			{
				size = DOWNLOAD_PACKET_SIZE;
				sd_get_size -= DOWNLOAD_PACKET_SIZE;
			}
			else
			{
				size = sd_get_size;
				sd_get_size = 0;
			}
			pPacksize = (INT16U*)(pBuf+GP_SOCK_RESP_PAYLOAD_SIZE_OFFSET);
			*pPacksize = size;
			tlen = size+GP_SOCK_RESP_PAYLOAD_OFFSET;
		}
		// Wi-Fi driver divide into few packets
		while(tlen)
		{
			available = socket_cmd_wifi_get_client_avail_tx_buf_size(ctl_blk);
			if (available)
			{
				if(tlen > available)
					wlen = available;
				else
					wlen = tlen;

				// printf("available=%d,wlen=%d,tlen=%d,pBuf=%p\r\n",available,wlen,tlen,pBuf);
				str_t = OSTimeGet();
				ret = socket_cmd_wifi_send_data(ctl_blk, pBuf, wlen);
				cur_t = OSTimeGet();
				netwr_t += (cur_t-str_t);
				if(ret != SC_RET_SUCCESS)
				{
					err = NAK_Write_Fail;
					printf("raw data get error: send data fail(wlen=%d)\r\n", wlen);
					goto Playback_GetRawData_END;
				}
							
				tlen -= wlen;
				pBuf += wlen;
				retry_cnt = DOWNLOAD_RETRY;
			}
			else
			{
				OSTimeDly(1);
				retry_cnt--;
				if(!retry_cnt)
				{
					printf("wifi send data timeout,%d\r\n",retry_cnt);
					err = NAK_Request_timeout;
					goto Playback_GetRawData_END;
				}
			}
		}
		wifi_dl_cur_size += size;
		// printf("wifi send data OK,%d\r\n",retry_cnt);
	}

Playback_GetRawData_END:
	return err;	
}


static int GPSOCK_Playback_GetRawData_Decode(unsigned short gp_sock_cmd, socket_cmd_ctl_t *ctl_blk, INT8U* rxstring)
{
	INT32S ret;
	INT32S err = NAK_OK;	
	INT32U cmd_st, cmd_et;	
	

	// mm_dump();
	wifi_dl_cur_size = 0;
	wifi_dl_buf_A = NULL;
	wifi_dl_buf_B = NULL;	
	wifi_dl_buf_A = (INT8U*)gp_malloc_align(DOWNLOAD_SD_BLOCK_SIZE+64, 8);
	if (wifi_dl_buf_A == NULL)
	{
		printf("GPSOCK_Playback_GetRawData memory A alloc fail\r\n");
		err = NAK_Invalid_command;
		goto GPSOCK_PLAYBACK_GETRAWDATA_DECODE_END;
	}
	wifi_dl_buf_B = (INT8U*)gp_malloc_align(DOWNLOAD_SD_BLOCK_SIZE+64, 8);
	if (wifi_dl_buf_B == NULL)
	{
		printf("GPSOCK_Playback_GetRawData memory B alloc fail\r\n");
		err = NAK_Invalid_command;
		goto GPSOCK_PLAYBACK_GETRAWDATA_DECODE_END;
	}	

	gp_memset((INT8S*)(&download_file_info), 0, sizeof(download_file_info) );
	ap_storage_service_download_file_block_size(DOWNLOAD_SD_BLOCK_SIZE);
	wifi_dl_play_file_index = *(rxstring+13);
	wifi_dl_play_file_index = wifi_dl_play_file_index<<8;
	wifi_dl_play_file_index |= *(rxstring+12);		// file index
	curr_wifi = 0; curr_buf = 1;		// A-B buffer current index
	buf_dl_flag[0] = buf_dl_flag[1] = 0;
	netwr_t = 0;
	wifi_dl_state = WIFI_DOWNLOAD_ACTIVE;

	ap_storage_service_timer_start();
	DBG_PRINT("start download\r\n");
	cmd_st = OSTimeGet();
	while (1)
	{
		// New data detected (i.e. cancel download) during download.
		// Leave the while loop to handle it.
		ret = socket_cmd_wifi_check_new_data(ctl_blk);
		if (ret == SC_RET_SUCCESS)
		{
			printf("Cancel download\r\n");
			err = NAK_Invalid_command;
			break;
		}
	
		if ( (curr_wifi==0)&&(buf_dl_flag[1]==1) )
		{
			// Wi-Fi will be transmitted through B buffer
			//DBG_PRINT("B_");
			err = Playback_GetRawData(ctl_blk, wifi_dl_buf_B, buf_dl_size[1]);
			//DBG_PRINT("B ");
			if (err!=NAK_OK)
			{
				//DBG_PRINT("B buffer send error\r\n");
				err = NAK_Write_Fail;
				goto GPSOCK_PLAYBACK_GETRAWDATA_DECODE_END;
			}
			///////////			
			curr_wifi = 1;
			buf_dl_flag[1] = 0;
		}
		else if ( (curr_wifi==1)&&(buf_dl_flag[0]==1) )
		{
			// Wi-Fi will be transmitted through A buffer
			//DBG_PRINT("A_");
			err = Playback_GetRawData(ctl_blk, wifi_dl_buf_A, buf_dl_size[0]);
			//DBG_PRINT("A ");
			if (err!=NAK_OK)
			{
				err = NAK_Write_Fail;
				//DBG_PRINT("A buffer send error\r\n");
				goto GPSOCK_PLAYBACK_GETRAWDATA_DECODE_END;
			}
			///////////
			curr_wifi = 0;
			buf_dl_flag[0] = 0;
		}
		else
		{
			OSTimeDly(1);	// loading data
		}

		if ( (curr_wifi==0)&&(buf_dl_flag[1]==0xFF) ) {
			DBG_PRINT("B buffer end\r\n");
			break;
		}
		
		if ( (curr_wifi==1)&&(buf_dl_flag[0]==0xFF) ) {
			DBG_PRINT("A buffer end\r\n");
			break;
		}

		if ( (buf_dl_flag[0]==0xFFFFFFFF)||(buf_dl_flag[1]==0xFFFFFFFF) )
		{
			printf("SD file load error!!\r\n");
			err = NAK_Server_is_busy;
			break;
		}

	}


GPSOCK_PLAYBACK_GETRAWDATA_DECODE_END:
	cmd_et = OSTimeGet();
	if (wifi_dl_buf_A)
	{
		gp_free((void*)wifi_dl_buf_A);
	}
	if (wifi_dl_buf_B)
	{
		gp_free((void*)wifi_dl_buf_B);
	}
	wifi_dl_state = WIFI_DOWNLOAD_IDLE;	
	printf("download finis(%d), Total time = %d\r\n",netwr_t, cmd_et-cmd_st);
	msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_STORAGE_CHECK, NULL, NULL, MSG_PRI_NORMAL);
	return err;
}

#else

#define DOWNLOAD_RETRY 1000
#define DOWNLOAD_SD_BLOCK_SIZE (70*60*1024)
#define DOWNLOAD_PACKET_SIZE (60*1024)

STOR_SERV_DOWNLOAD_FILEINFO download_file_info;
// ヘ玡 CMD  payload size 程や 64KB
// SD Ω弄 150KB矗硉礛–50KB  packet commandτ– packetwi-fi drvier –Ω癳 1.5KB
static int GPSOCK_Playback_GetRawData_Decode(unsigned short gp_sock_cmd, socket_cmd_ctl_t *ctl_blk, INT8U* rxstring)
{
	INT8U *buf = 0;
	INT16U play_file_index;
	INT8U packet_const[12] = {0x47,0x50,0x53,0x4F,0x43,0x4B,0x45,0x54,0x02,0x00,0x03,0x05};
	INT32U sd_get_size;		//
	INT32U sd_get_idx = 0;
	INT32U sd_download_size;
	INT32S ret;
	INT32U cur_size = 0,cur_t,str_t,readsd_t = 0, netwr_t = 0;
	INT8U *pPacketbuf;
	INT16U *pPacksize;		// packet size
	INT8U *pBuf;	// download current buffer
	INT32S err = NAK_OK;
	INT16U available;

	// mm_dump();
	play_file_index = *(rxstring+13);
	play_file_index = play_file_index<<8;
	play_file_index |= *(rxstring+12);		// file index
	sd_download_size = DOWNLOAD_SD_BLOCK_SIZE;
	buf = (INT8U*)gp_malloc_align(sd_download_size+64, 8);
	if (buf == NULL)
	{
		printf("GPSOCK_Playback_GetRawData memory alloc fail\r\n");
		err = NAK_Invalid_command;
		goto GPSOCK_PLAYBACK_GETRAWDATA_DECODE_END;
	}
	gp_memset((INT8S*)(&download_file_info), 0, sizeof(download_file_info) );
	ap_storage_service_download_file_block_size(sd_download_size);

	//////////// TODO ///////////
	DBG_PRINT("start download\r\n");
	do
	{
		// New data detected (i.e. cancel download) during download.
		// Leave the while loop to handle it.
		ret = socket_cmd_wifi_check_new_data(ctl_blk);
		if (ret == SC_RET_SUCCESS)
		{
			printf("Cancel download\r\n");
			err = NAK_Invalid_command;
			break;
		}

		str_t = OSTimeGet();
		// each     DOWNLOAD_SD_BLOCK_SIZE
		ret = ap_storage_service_download_file_req(play_file_index,&download_file_info,buf+64,&sd_get_size);
		cur_t = OSTimeGet();
		if(ret == 0)
		{
			sd_get_idx = 0;
			// each DOWNLOAD_PACKET_SIZE
			while (sd_get_size != 0)
			{
				INT32U retry_cnt = DOWNLOAD_RETRY;
				INT32U wlen, tlen, size;

				readsd_t += (cur_t-str_t);
				if(sd_get_idx==0)
				{
					pPacketbuf = (INT8U*)(buf + 64-GP_SOCK_RESP_PAYLOAD_OFFSET);
					sd_get_idx++;
					pBuf = (INT8U*)pPacketbuf;
					gp_memcpy((INT8S*)pBuf,(INT8S*)packet_const,sizeof(packet_const) );
					if (sd_get_size > (DOWNLOAD_PACKET_SIZE-64+GP_SOCK_RESP_PAYLOAD_OFFSET))
					{
						size = DOWNLOAD_PACKET_SIZE-64+GP_SOCK_RESP_PAYLOAD_OFFSET;
						sd_get_size -= size;
					}
					else
					{
						size = sd_get_size;
						sd_get_size = 0;
					}
					pPacksize = (INT16U*)(pBuf+GP_SOCK_RESP_PAYLOAD_SIZE_OFFSET);
					*pPacksize = size;
					tlen = size+GP_SOCK_RESP_PAYLOAD_OFFSET;
				}
				else
				{
					pPacketbuf = (INT8U*)(buf + sd_get_idx*DOWNLOAD_PACKET_SIZE);
					sd_get_idx++;
					pBuf = (INT8U*)pPacketbuf;
					gp_memcpy((INT8S*)pBuf,(INT8S*)packet_const,sizeof(packet_const) );
					if (sd_get_size > DOWNLOAD_PACKET_SIZE)
					{
						size = DOWNLOAD_PACKET_SIZE;
						sd_get_size -= DOWNLOAD_PACKET_SIZE;
					}
					else
					{
						size = sd_get_size;
						sd_get_size = 0;
					}
					pPacksize = (INT16U*)(pBuf+GP_SOCK_RESP_PAYLOAD_SIZE_OFFSET);
					*pPacksize = size;
					tlen = size+GP_SOCK_RESP_PAYLOAD_OFFSET;
				}
				// Wi-Fi driver divide into few packets
				while(tlen)
				{
					available = socket_cmd_wifi_get_client_avail_tx_buf_size(ctl_blk);
					if (available)
					{
						if(tlen > available)
							wlen = available;
						else
							wlen = tlen;

						// printf("available=%d,wlen=%d,tlen=%d,pBuf=%p\r\n",available,wlen,tlen,pBuf);
						str_t = OSTimeGet();
						ret = socket_cmd_wifi_send_data(ctl_blk, pBuf, wlen);
						cur_t = OSTimeGet();
						netwr_t += (cur_t-str_t);
						if(ret != SC_RET_SUCCESS)
						{
							err = NAK_Write_Fail;
							printf("raw data get error: send data fail(wlen=%d)\r\n", wlen);
							goto GPSOCK_PLAYBACK_GETRAWDATA_DECODE_END;
						}
							
						tlen -= wlen;
						pBuf += wlen;
						retry_cnt = DOWNLOAD_RETRY;
					}
					else
					{
						OSTimeDly(1);
						retry_cnt--;
						if(!retry_cnt)
						{
							printf("wifi send data timeout,%d\r\n",retry_cnt);
							err = NAK_Request_timeout;
							goto GPSOCK_PLAYBACK_GETRAWDATA_DECODE_END;
						}
					}
				}
				cur_size += size;
				// printf("wifi send data OK,%d\r\n",retry_cnt);
			}
		}
		else
		{
			printf("file down load error!!\r\n");
			err = NAK_Server_is_busy;
			break;
		}
	}
	while(download_file_info.file_size > cur_size);

GPSOCK_PLAYBACK_GETRAWDATA_DECODE_END:
	if (buf)
	{
		gp_free((void*)buf);
		printf("download finis  %d,%d\r\n",readsd_t,netwr_t);
	}

	return err;
}

#endif


static int GPSOCK_Playback_Stop_Decode(unsigned short gp_sock_cmd, socket_cmd_ctl_t *ctl_blk, INT8U* rxstring)
{
	INT32U i;
	INT32S ret = NAK_OK;
	mjpeg_write_data_t stop_event;

	if (browse_sts & BROWSE_PLAYBACK_BUSY)
	{	// stop avi file
		msgQSend(ApQ, MSG_APQ_MJPEG_DECODE_END, NULL, NULL, MSG_PRI_NORMAL);
		OSTimeDly(10);
		printf("GPSOCK_Playback_Stop_Decode  pause current avi file\r\n");
	}

	mjpeg_service_playstop_flag = 1;
	#if PREVIEW_TCPIP
	mjpeg_playback_stop(&stop_event);
	#else
	rtp_rtsp_cbk(RTP_STOP_EVENT);	
	#endif
	for (i = 0; i < PLAYBACK_STOP_TIME_OUT; ++i)
	{
		if (mjpeg_service_playstop_flag == 0)
		{
			break;
		}
		OSTimeDly(1);
	}
	if (i == PLAYBACK_STOP_TIME_OUT)
	{
		ret = NAK_Request_timeout;
	}

	return ret;
}

static int GPSOCK_Playback_GetSpecificName_Decode(unsigned short gp_sock_cmd, socket_cmd_ctl_t *ctl_blk, INT8U* rxstring, unsigned short *p_status)
{
	INT16U play_file_order;
	INT16S play_file_index;
	INT32S ret = NAK_OK;	

	play_file_order = *(rxstring+13);
	play_file_order = play_file_order<<8;
	play_file_order |= *(rxstring+12);

	play_file_index = ap_storage_service_indexfile_req(play_file_order, playfile_index_table);
	if  (play_file_index == -1)
	{		
		ret = NAK_Get_file_list_fail;
	}
	else
	{
		*p_status =(unsigned short)play_file_index;
	}

	return ret;
}

static int GPSOCK_Firmware_Download_Decode(unsigned short gp_sock_cmd, socket_cmd_ctl_t *ctl_blk, INT8U* rxstring)
{
	INT32S ret = NAK_OK;
	INT32U *p;

	if ( (bat_lvl_cal_bak<3)&&(adp_status == 0) )
	{
		ret = NAK_Battery_low;
		goto GPSOCK_Firmware_Download_Decode_END;
	}
	// malloc memory
	pfirmware = gp_malloc_align(SOCKET_CMD_FIRMWARE_BUF_SIZE, 64);
	prx_firmware_buf = gp_malloc_align(SOCKET_CMD_DOWNLOAD_BUF_SIZE, 64);
	if ( (pfirmware==NULL)||(prx_firmware_buf==NULL) )
	{
		ret = NAK_Mem_malloc_error;
		goto GPSOCK_Firmware_Download_Decode_END;
	}
	else
	{
		pfirmware_cur = pfirmware;
	}

	p = (INT32U*)(rxstring+12);
	firmware_download_size = *p;
	p++;
	firmware_download_checksum = *p;
	firmware_cur_size = 0;
	DBG_PRINT("download_size=%d, download_checksum=0x%x\r\n",firmware_download_size,firmware_download_checksum);
	
GPSOCK_Firmware_Download_Decode_END:	
	return ret;
}

static int GPSOCK_Firmware_SendRawData_Decode(unsigned short gp_sock_cmd, socket_cmd_ctl_t *ctl_blk, INT8U* rxstring)
{
	INT32S ret = NAK_OK;
	INT32U size;
	INT8U *p;

	p = (INT8U*)(rxstring+12);
	size = (INT32U)(*p++);
	size += ((*p++)<<8);
	
	//DBG_PRINT("cur_size=%d, size = %d\r\n", firmware_cur_size, size);
	firmware_upgrade_flag=0;
	if (size!=0)
	{	// downloading
		gp_memcpy((INT8S*)pfirmware_cur, (INT8S*)p, size);
		firmware_cur_size += size;
		pfirmware_cur += (INT32U)size;
	}
	else
	{	// checksum
		INT32U i;
		INT32U chksum = 0;

		DBG_PRINT("download finish\r\n");
		if (firmware_cur_size!=firmware_download_size)
		{
			gp_free(pfirmware);	pfirmware = NULL;
			gp_free(prx_firmware_buf); prx_firmware_buf = NULL;
			DBG_PRINT("download size mismatch !!\r\n");
			ret = NAK_Checksum_error;
			goto GPSOCK_Firmware_SendRawData_Decode_END;
		}
		for (i=0; i<firmware_download_size;++i)
		{
			chksum += pfirmware[i];
		}
		if (chksum!=firmware_download_checksum)
		{
			gp_free(pfirmware); pfirmware = NULL;
			gp_free(prx_firmware_buf); prx_firmware_buf = NULL;			
			DBG_PRINT("firmware checksum error !!\r\n");
			ret = NAK_Checksum_error;
			goto GPSOCK_Firmware_SendRawData_Decode_END;
		}
		firmware_upgrade_flag = 1;		// firmware verify pass
		
	}

GPSOCK_Firmware_SendRawData_Decode_END:
	return ret;
}

static int GPSOCK_Firmware_Upgrade_Decode(unsigned short gp_sock_cmd, socket_cmd_ctl_t *ctl_blk, INT8U* rxstring)
{
	INT32S ret = NAK_OK;

	DBG_PRINT("GPSOCK_Firmware_Upgrade_Decode\r\n");
	if (firmware_upgrade_flag!=1)
	{
		gp_free(pfirmware); pfirmware = NULL;
		gp_free(prx_firmware_buf); prx_firmware_buf = NULL;
		DBG_PRINT("NAK_Checksum_error\r\n");
		ret = NAK_Checksum_error;
		goto GPSOCK_Firmware_Upgrade_Decode_END;
	}
	if ( ap_state_firmware_upgrade()==-1 )
	{
		DBG_PRINT("ap_state_firmware_upgrade malloc buffer error\r\n");
		gp_free(pfirmware); pfirmware = NULL;
		gp_free(prx_firmware_buf); prx_firmware_buf = NULL;
		ret = NAK_Mem_malloc_error;
		goto GPSOCK_Firmware_Upgrade_Decode_END;
	}
	
	firmware_upgrade_flag = 2;

GPSOCK_Firmware_Upgrade_Decode_END:
	return ret;
}

static int Vendor_Command_Decode(unsigned short gp_sock_cmd, socket_cmd_ctl_t *ctl_blk, INT8U* rxstring,  unsigned char *buf)
{
	int i;
	unsigned char *p = (unsigned char *)(&rxstring[GP_SOCK_CMD_PAYLOAD_OFFSET+2]);
	short size = *((short*)(&rxstring[GP_SOCK_CMD_PAYLOAD_OFFSET]));

	if (size > GPSOCK_Vendor_Command_Buf_Length)
	{
		size = GPSOCK_Vendor_Command_Buf_Length;
	}

	for (i = 0; i < size; ++i)
	{
		buf[i] = p[i];
	}

	return (int)size;
}

//
// Client -> server command format:
//
// Low byte                                                                  High byte
// +-------------+--------------+-----------------+----------------+-----------------+
// | GP_SOCK_Tag | GP_SOCK_Type | GP_SOCK_Mode_ID | GP_SOCK_Cmd_ID | GP_SOCK_Payload |
// +-------------+--------------+-----------------+----------------+-----------------+
//      8byte         2byte           1byte              1byte            Nbyte
//
static int gp_cmd_process(socket_cmd_ctl_t *ctl_blk, INT8U* rxstring)
{
	INT32S ret = NAK_OK, ret1;
	INT8U *pGP_SOCK_Tag = (INT8U *)(rxstring+0);
	INT16S GP_SOCK_Cmd;
	INT16U len = 0;
	INT32U t;

	// Check tag
	GP_SOCK_Cmd = (INT16S)(pGP_SOCK_Tag[GP_SOCK_CMD_MODE_ID_OFFSET]<<8)|(INT16S)pGP_SOCK_Tag[GP_SOCK_CMD_CMD_ID_OFFSET];	
	if ((pGP_SOCK_Tag[0] != 'G') || (pGP_SOCK_Tag[1] != 'P') || (pGP_SOCK_Tag[2] != 'S') ||
	    (pGP_SOCK_Tag[3] != 'O') || (pGP_SOCK_Tag[4] != 'C') || (pGP_SOCK_Tag[5] != 'K') ||
	    (pGP_SOCK_Tag[6] != 'E') || (pGP_SOCK_Tag[7] != 'T'))
	{
		ret = -1;
		printf("socket_cmd:skip\r\n");
		goto GP_CMD_PROCESS_END;
	}
	else
	{
		//printf("CMD=0x%x\r\n",GP_SOCKs_Cmd);
	}
	//DBG_PRINT("CMD=0x%x\r\n",GP_SOCK_Cmd);
	switch (GP_SOCK_Cmd)
	{
	case GPSOCK_General_SetMode:
		GPSOCK_General_SetMode_Decode(GP_SOCK_Cmd, ctl_blk, pGP_SOCK_Tag);
		len = gp_resp_set((GP_SOCK_TYPE_ACK<<16)|(GP_SOCK_Cmd), 0x0000, 0, 0);
		break;
	case GPSOCK_General_GetParameterFile:
		DBG_PRINT("GetParameterFile...\r\n");
		General_GetParameterFile_Decode(GP_SOCK_Cmd, ctl_blk, pGP_SOCK_Tag); 
		len = gp_resp_set((GP_SOCK_TYPE_ACK<<16)|(GP_SOCK_Cmd), 0x0000, 0, 0);
		break; 
	case GPSOCK_General_GetDeviceStatus:
		{
			unsigned char status[GPSOCK_General_DeviceStatus_Length];
			//DBG_PRINT("GetDeviceStatus...\r\n");
			General_GetDeviceStatus_Decode(GP_SOCK_Cmd, ctl_blk, pGP_SOCK_Tag, status);
			len = gp_resp_set((GP_SOCK_TYPE_ACK<<16)|(GP_SOCK_Cmd), GPSOCK_General_DeviceStatus_Length, status, GPSOCK_General_DeviceStatus_Length);
		}
		break;
	case GPSOCK_General_PowerOff:
		GPSOCK_General_PowerOff_Decode(GP_SOCK_Cmd,ctl_blk, pGP_SOCK_Tag);
		len = gp_resp_set((GP_SOCK_TYPE_ACK<<16)|(GP_SOCK_Cmd), 0x0000, 0, 0);
		break;
	case GPSOCK_General_RestarStreaming:
		ret = General_RestarStreaming_Decode(GP_SOCK_Cmd,ctl_blk, pGP_SOCK_Tag);
		if (ret == NAK_OK)
		{
			len = gp_resp_set((GP_SOCK_TYPE_ACK<<16)|(GP_SOCK_Cmd), 0x0000, 0, 0);
		}
		else
		{
			len = gp_resp_set((GP_SOCK_TYPE_NAK<<16)|(GP_SOCK_Cmd), (short)ret, 0, 0);
		}
		break;
	case GPSOCK_General_AuthDevice:
		{
			unsigned char key[GPSOCK_General_Key_Length];
			GPSOCK_General_AuthDevice_Decode(GP_SOCK_Cmd, ctl_blk, pGP_SOCK_Tag, key);
			len = gp_resp_set((GP_SOCK_TYPE_ACK<<16)|(GP_SOCK_Cmd), GPSOCK_General_Key_Length, key, GPSOCK_General_Key_Length);
		}
	break;
	case GPSOCK_Record_Start:
		ret = GPSOCK_Record_Start_Decode(GP_SOCK_Cmd,ctl_blk, pGP_SOCK_Tag);
		if (ret == NAK_OK)
		{
			len = gp_resp_set((GP_SOCK_TYPE_ACK<<16)|(GP_SOCK_Cmd), 0x0000, 0, 0);
		}
		else
		{
			len = gp_resp_set((GP_SOCK_TYPE_NAK<<16)|(GP_SOCK_Cmd), (short)ret, 0, 0);
		}
		break;
	case GPSOCK_Record_Audio:
		ret = GPSOCK_Record_Audio_Decode(GP_SOCK_Cmd, ctl_blk, pGP_SOCK_Tag);
		if (ret == NAK_OK)
		{
			len = gp_resp_set((GP_SOCK_TYPE_ACK<<16)|(GP_SOCK_Cmd), 0x0000, 0, 0);
		}
		else
		{
			len = gp_resp_set((GP_SOCK_TYPE_NAK<<16)|(GP_SOCK_Cmd), (short)ret, 0, 0);
		}
		break;
	case GPSOCK_CapturePicture_Capture:		// send capture command to APQ
		ret = GPSOCK_CapturePicture_Capture_Decode(GP_SOCK_Cmd, ctl_blk, pGP_SOCK_Tag);
		__msg("send capture command, ret = %d\n", ret);
		if (ret == NAK_OK)
		{
			len = gp_resp_set((GP_SOCK_TYPE_ACK<<16)|(GP_SOCK_Cmd), 0x0000, 0, 0);
		}
		else
		{
			len = gp_resp_set((GP_SOCK_TYPE_NAK<<16)|(GP_SOCK_Cmd), (short)ret, 0, 0);
		}
		break;
	case GPSOCK_Playback_Start:
		GPSOCK_Playback_Start_Decode(GP_SOCK_Cmd, ctl_blk, pGP_SOCK_Tag);
		len = gp_resp_set((GP_SOCK_TYPE_ACK<<16)|(GP_SOCK_Cmd), 0x0000, 0, 0);
		break;
	case GPSOCK_Playback_Pause:
		GPSOCK_Playback_Pause_Decode(GP_SOCK_Cmd, ctl_blk, pGP_SOCK_Tag);
		len = gp_resp_set((GP_SOCK_TYPE_ACK<<16)|(GP_SOCK_Cmd), 0x0000, 0, 0);
		break;
	case GPSOCK_Playback_GetFileCount:
		ret =   GPSOCK_Playback_GetFileCount_Decode(GP_SOCK_Cmd, ctl_blk, pGP_SOCK_Tag);
		if (ret > 0)
		{
			len = gp_resp_set((GP_SOCK_TYPE_ACK<<16)|(GP_SOCK_Cmd), 0x0002, (unsigned char *)(&ret), 2);
		}
		else
		{
			len = gp_resp_set((GP_SOCK_TYPE_NAK<<16)|(GP_SOCK_Cmd), (short)ret, 0, 0);
		}
		break;
	case GPSOCK_Playback_GetFileList:
		ret = GPSOCK_Playback_GetFileList_Decode(GP_SOCK_Cmd, ctl_blk, pGP_SOCK_Tag);
		if (ret < 0)
		{
			len = gp_resp_set((GP_SOCK_TYPE_NAK<<16)|(GP_SOCK_Cmd), (short)ret, 0, 0);
		}
		else
			len = ret;
		break;
	case GPSOCK_Playback_GetThumbnail:
		ret = GPSOCK_Playback_GetThumbnail_Decode(GP_SOCK_Cmd, ctl_blk, pGP_SOCK_Tag);
		if (ret == NAK_OK)
		{
			len = gp_resp_set(GP_SOCK_TYPE_ACK<<16|(GP_SOCK_Cmd), 0x0000, 0, 0);
		}
		else
		{
			len = gp_resp_set((GP_SOCK_TYPE_NAK<<16)|(GP_SOCK_Cmd), (short)ret, 0, 0);
		}
		break;
	case GPSOCK_Playback_DeleteFile:
		#if SUPPORT_APP_DEL_FILE == CUSTOM_ON
		ret = GPSOCK_Playback_DelFile_Decode(GP_SOCK_Cmd, ctl_blk, pGP_SOCK_Tag);
		#else
		ret = 0xFFFF;
		#endif
		if (ret == NAK_OK)
		{
			len = gp_resp_set(GP_SOCK_TYPE_ACK<<16|(GP_SOCK_Cmd), 0x0000, 0, 0);
		}
		else
		{
			len = gp_resp_set((GP_SOCK_TYPE_NAK<<16)|(GP_SOCK_Cmd), (short)ret, 0, 0);
		}
		break;
	case GPSOCK_Playback_GetRawData:
		ret = GPSOCK_Playback_GetRawData_Decode(GP_SOCK_Cmd, ctl_blk, pGP_SOCK_Tag);
		if (ret == NAK_OK)
		{
			len = gp_resp_set(GP_SOCK_TYPE_ACK<<16|(GP_SOCK_Cmd), 0x0000, 0, 0);
		}
		else
		{
			len = gp_resp_set((GP_SOCK_TYPE_NAK<<16)|(GP_SOCK_Cmd), (short)ret, 0, 0);
		}
		break;
	case GPSOCK_Playback_Stop:
		ret = GPSOCK_Playback_Stop_Decode(GP_SOCK_Cmd, ctl_blk, pGP_SOCK_Tag);
		if (ret == NAK_OK)
		{
			len = gp_resp_set((GP_SOCK_TYPE_ACK<<16)|(GP_SOCK_Cmd), 0x0000, 0, 0);
		}
		else
		{
			len = gp_resp_set((GP_SOCK_TYPE_NAK<<16)|(GP_SOCK_Cmd), (short)ret, 0, 0);
		}
		break;
	case GPSOCK_Playback_GetSpecificName:
		{
			unsigned short status;
			ret = GPSOCK_Playback_GetSpecificName_Decode(GP_SOCK_Cmd, ctl_blk, pGP_SOCK_Tag, &status);
			if (ret == NAK_OK)
			{
				len = gp_resp_set((GP_SOCK_TYPE_ACK<<16)|(GP_SOCK_Cmd), 0x0002, (unsigned char *)(&status), 2);
			}
			else
			{
				len = gp_resp_set((GP_SOCK_TYPE_NAK<<16)|(GP_SOCK_Cmd), (short)ret, 0, 0);
			}
		}
		break;		
	case GPSOCK_Menu_GetParameter:
		{
			unsigned char value[GPSOCK_WiFi_Name_Length + 20] = {0};
			unsigned int value_len = 0;
			DBG_PRINT("Menu_GetParameter...\r\n");
			value_len = Menu_GetParameter_Decode(GP_SOCK_Cmd, ctl_blk, pGP_SOCK_Tag, (unsigned char *)(&value));
			len = gp_resp_set((GP_SOCK_TYPE_ACK<<16)|(GP_SOCK_Cmd), value_len, (unsigned char *)(&value), value_len);
		}
		break;
	case GPSOCK_Menu_SetParameter:
		DBG_PRINT("Menu_SetParameter...\r\n");
		ret = Menu_SetParameter_Decode(GP_SOCK_Cmd, ctl_blk, pGP_SOCK_Tag);
		if (ret == NAK_OK)
		{
			len = gp_resp_set(GP_SOCK_TYPE_ACK<<16|(GP_SOCK_Cmd), 0x0000, 0, 0);
		}
		else
		{
			len = gp_resp_set((GP_SOCK_TYPE_NAK<<16)|(GP_SOCK_Cmd), (short)ret, 0, 0);
		}
		//len = gp_resp_set(GP_SOCK_TYPE_ACK<<16|(GP_SOCK_Cmd), 0x0000, 0, 0);
		break;
	case GPSOCK_Firmware_Download:
		ret = GPSOCK_Firmware_Download_Decode(GP_SOCK_Cmd, ctl_blk, pGP_SOCK_Tag);
		if (ret==NAK_OK)
		{
			len = gp_resp_set((GP_SOCK_TYPE_ACK<<16)|(GP_SOCK_Cmd), 0x0000, 0, 0);		
		}
		else
		{
			len = gp_resp_set((GP_SOCK_TYPE_NAK<<16)|(GP_SOCK_Cmd), (short)ret, 0, 0);
		}
		break;
	case GPSOCK_Firmware_SendRawData:
		ret = GPSOCK_Firmware_SendRawData_Decode(GP_SOCK_Cmd, ctl_blk, pGP_SOCK_Tag);
		if (ret==NAK_OK)
		{
			len = gp_resp_set((GP_SOCK_TYPE_ACK<<16)|(GP_SOCK_Cmd), 0x0000, 0, 0);		
		}
		else if (ret == NAK_Checksum_error)
		{
			len = gp_resp_set((GP_SOCK_TYPE_NAK<<16)|(GP_SOCK_Cmd), (short)ret, 0, 0);
		}		
		break;
	case GPSOCK_Firmware_Upgrade:
		ret = GPSOCK_Firmware_Upgrade_Decode(GP_SOCK_Cmd, ctl_blk, pGP_SOCK_Tag);
		if (ret==NAK_OK)
		{
			len = gp_resp_set((GP_SOCK_TYPE_ACK<<16)|(GP_SOCK_Cmd), 0x0000, 0, 0);		
		}
		else
		{
			len = gp_resp_set((GP_SOCK_TYPE_NAK<<16)|(GP_SOCK_Cmd), (short)ret, 0, 0);
		}		
		break;
	case GPSOCK_Vendor_Command:
		{
			unsigned char vendor_buf[GPSOCK_Vendor_Command_Buf_Length];
			unsigned int vendor_len;
			vendor_len = Vendor_Command_Decode(GP_SOCK_Cmd, ctl_blk, pGP_SOCK_Tag, vendor_buf);
			len = gp_resp_set((GP_SOCK_TYPE_ACK<<16)|(GP_SOCK_Cmd&0x0000FFFF), vendor_len, vendor_buf, vendor_len);
		}
		break;
	default:
		len = gp_resp_set((GP_SOCK_TYPE_NAK<<16)|(GP_SOCK_Cmd), 0xFFFF, 0, 0);
	}
	//__msg("tx_buf: %d, %d, %s\n", socket_cmd_tx_buf[10], socket_cmd_tx_buf[11], socket_cmd_tx_buf);
	ret1 = socket_cmd_wifi_send_data(ctl_blk, socket_cmd_tx_buf, len);
	if(ret1 != SC_RET_SUCCESS)
	{
		ret = NAK_Write_Fail;
	}
	if (firmware_upgrade_flag==2)
	{
		DBG_PRINT("upgrade finish reboot\r\n");
		//upgrade_exit(0);			// only power off can be chosen
		upgrade_exit(firmware_upgrade_flag);			// only power off can be chosen
	}

GP_CMD_PROCESS_END:
	return ret;
}

static INT32S process_socket_cmd(socket_cmd_ctl_t *ctl_blk)
{
	static INT32S polling_cnt = 0;
	static INT16S GP_SOCK_MODE = 0;
	static INT16S GP_SOCK_ID = 0;	
	INT32S ret = 0;
	INT8U *rx_buf;
	INT32U rx_buf_size;
	INT16S GP_SOCK_Cmd;

	if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
	{
		printf("\r\n process_socket_cmd doesn't allow\r\n");
		ret = -2;	// Don't delete mjpeg task@josephhsieh@20150827
		goto PROCESS_SOCKET_CMD_END;
	}
	
	while (cmd_service_enable)
	{
		if (prx_firmware_buf==NULL)
		{
			rx_buf = socket_cmd_rx_buf;
			rx_buf_size = SOCKET_CMD_BUF_SIZE;
		}
		else		// firmware upgrade mode
		{
			rx_buf = prx_firmware_buf;
			rx_buf_size = SOCKET_CMD_DOWNLOAD_BUF_SIZE;
		}
		ret = socket_cmd_wifi_recv_data(ctl_blk, rx_buf, rx_buf_size);
		if (ret == SC_RET_SUCCESS)
		{
			GP_SOCK_MODE =  (INT16S)(rx_buf[GP_SOCK_CMD_MODE_ID_OFFSET]);
			GP_SOCK_ID = (INT16S)(rx_buf[GP_SOCK_CMD_CMD_ID_OFFSET]);
			GP_SOCK_Cmd = (GP_SOCK_MODE<<8)|GP_SOCK_ID;
		
			gp_cmd_process(ctl_blk, rx_buf);
			if (GP_SOCK_Cmd!=GPSOCK_General_GetDeviceStatus)
			{
			    polling_cnt = 0;
			}
		}
		else if (ret == SC_RET_RECV_TIMEOUT)
		{
			Ap_sta_status info;
		
			//DBG_PRINT("%d_%d ",polling_cnt,GP_SOCK_MODE);
	        memset(&info , 0 , sizeof(Ap_sta_status));
       		ssv6xxx_wifi_status(&info);
			if (info.u.ap.stanum == 0)
			{
				DBG_PRINT("Server Disconnect !!\r\n");
				ret = SC_RET_FAIL;		// return Connect Page
				break;			
			}
			if (mjpeg_service_ready_flag || cmd_flag_clr_enable)
			{
				polling_cnt = 0;		// Record mode, Capture mode, playing AVI
			}
			else if (polling_cnt>SOCKET_MODE_TIMEOUT)
			{
				if ( ( (present_state == STATE_VIDEO_RECORD)||(present_state == STATE_VIDEO_PREVIEW) ) && (GP_SOCK_MODE<4/*MENU*/) )
				{
					DBG_PRINT("Assert !! Record/Capture mode(PORT 8080) is abnormal\r\n");	
					ret = SC_RET_FAIL;		// return Connect Page
					break;
				}
			}

			if ( (polling_cnt>SOCKET_CMD_TIMEOUT)&&(present_state != STATE_BROWSE) )	// 魁钩╃酚紇埃妓诀竚2だ牧笆耞秨 GoPlus Cam
			{																		// 更Ч紇冀穦禬筁2だ牧┮ぃ咀家Α
				DBG_PRINT("No active Key!! Auto disconnect GoPlusCam\r\n");
				ret = SC_RET_FAIL;		// return Connect Page				
				break;
			}
			polling_cnt++;
			continue;
		}
		else
		{
			break;
		}
	}


PROCESS_SOCKET_CMD_END:
	polling_cnt = 0;
	GP_SOCK_MODE = 0;
	GP_SOCK_ID = 0;
	socket_cmd_wifi_close_client_conn(ctl_blk);

	/* Exit this socket to accept a new connection */
	printf("Exit %s\r\n", __func__);

	return ret;
}

#define SOCK_TIME_OUT 50
extern void create_new_udp_for_cmd(void);
void socket_cmd_task(void *pvParameters)
{
	INT32S err;
    Ap_sta_status info;

	create_new_udp_for_cmd();
	socket_cmd_wifi_start_server(&socket_cmd_ctl_blk);

	cmd_service_enable =  0;
	cmd_service_start  = 0;

	for(;; )
	{
		if (cmd_service_enable == 0)
		{
			OSTimeDlyHMSM(0,0,0,100);
			continue;
		}
		cmd_service_start = 1;

		memset(socket_cmd_tx_buf, 0, sizeof(socket_cmd_tx_buf));
		memset(socket_cmd_rx_buf, 0, sizeof(socket_cmd_rx_buf));
		strncpy((char*)socket_cmd_tx_buf, (char*)"GPSOCKET", 8);

		err = socket_cmd_wifi_wait_client_conn(&socket_cmd_ctl_blk);
		if (err == SC_RET_SUCCESS)
		{
			printf("L:%d,M:%s, A new client connected\r\n", __LINE__, __func__);
			//Wifi_symbol_show(0xFFFF);		// white color

			/* A client connected */
			err = process_socket_cmd(&socket_cmd_ctl_blk);

			while(socket_cmd_wifi_del_client_conn(&socket_cmd_ctl_blk) != SC_RET_SUCCESS)
			{
				printf("Unable to delete socket cmd client connection\r\n");

				/* Delay 10 ms */
				OSTimeDly(SOCKET_CMD_SHORT_DELAY/10);
			}

			// 絋粄 port 8080 竒闽奔
			if (mjpeg_service_ready_flag)
			{
				#if PREVIEW_TCPIP
				mjpeg_write_data_t stop_event;				
				mjpeg_playback_stop(&stop_event);
				DBG_PRINT("stop mjstreamer service\r\n");				
				#else
				rtp_rtsp_cbk(RTP_STOP_EVENT);	
				#endif
			}
			// 璝硈╃挡硈╃
			if ( ap_state_config_burst_get() && (g_wifi_notify_pic_done != 0) )
			{
				INT16U cnt;
				//ap_peripheral_ok_key_exe(&cnt);
				//ap_peripheral_pw_key_exe(&cnt);
				DBG_PRINT("ACTIVE CAPTURE...\r\n");
				msgQSend(ApQ, MSG_APQ_CAPTURE_KEY_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
				for (cnt=0;cnt<SOCK_TIME_OUT;cnt++)
				{
					OSTimeDly(1);
					if (g_wifi_notify_pic_done==0xFFFFFFFF)
					{
						DBG_PRINT("sequence capture stop\r\n");
						break;
					}
				}
				g_wifi_notify_pic_done = 0;   
				if (cnt >= SOCK_TIME_OUT) {					
					DBG_PRINT("sequence capture stop ERROR !!\r\n");
				}
			}
			// 璝魁紇挡魁紇
			if (video_record_sts & VIDEO_RECORD_BUSY)
			{
				INT16U cnt;
				//ap_peripheral_ok_key_exe(&cnt);
				DBG_PRINT("WIFI ACTIVE STOP VIDEO...\r\n");
				msgQSend(ApQ, MSG_APQ_VIDEO_RECORD_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
				for (cnt=0;cnt<SOCK_TIME_OUT;cnt++)
				{
					OSTimeDly(1);
					if ( (video_record_sts&VIDEO_RECORD_BUSY)==0 )
					{
						DBG_PRINT("record stop\r\n");
						break;
					}
				}
				if (cnt >= SOCK_TIME_OUT) {					
					DBG_PRINT("record stop ERROR !!\r\n");
				}				
			}
		}
		else
		{
	        memset(&info , 0 , sizeof(Ap_sta_status));
       		ssv6xxx_wifi_status(&info);

			if (info.u.ap.stanum == 0)
			{
				//Wifi_symbol_show(0x1EBF);		// blue color			
			}
			else
			{
				//Wifi_symbol_show(0xFFE0);		// yellow color
			}
			//DBG_PRINT("stanum = %d\r\n",info.u.ap.stanum);
		}

		cmd_service_start = 0;
	}
}

void socket_cmd_service_init(void)
{
    INT8U err;

    err = OSTaskCreate(socket_cmd_task, NULL, (void *)&socket_cmd_stack[SOCKET_CMD_TASK_STACKSIZE-1], SOCKET_CMD_PRIORITY);
    if(err != OS_NO_ERR)
    {
        printf("socket cmd_task create failed\r\n");
    }
    else
    {
        printf("socket_cmd_task created ok. pri=%d\r\n", SOCKET_CMD_PRIORITY);
    }
}

void socket_cmd_start_service(void)
{
    int times = 100;
    
    if (cmd_service_start == 0)
    {
        cmd_service_enable = 1;
        
        /* waiting for mpjeg service start */
        while (cmd_service_start == 0)
        {
            OSTimeDlyHMSM(0,0,0,100);
            if (--times <= 0)
            {
                break;
            }
        }
        
        if (times <= 0)
        {
            printf("cmd service start failed\r\n");
        }
        else
        {
            printf("cmd service start success\r\n");
        }
    }
    else
    {
        //printf("cmd service already start\r\n");
    }
}

void socket_cmd_stop_service(void)
{
    int times = 100;
    
    if (cmd_service_start == 1)
    {
        cmd_service_enable = 0;

        /* waiting for mpjeg service stop */
        while (cmd_service_start == 1)
        {
            OSTimeDlyHMSM(0,0,0,100);
            if (--times <= 0)
            {
                break;
            }
        }
        
        if (times <= 0)
        {
            printf("cmd service stop failed\r\n");
        }
        else
        {
            printf("cmd service stop success\r\n");
        }
    }
    else
    {
        printf("cmd service already stop\r\n");
    }
}
