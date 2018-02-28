/******************************************************
* socket_cmd.h
*
* Purpose: Socket command handler header file
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
#ifndef __SOCKET_CMD_H__
#define __SOCKET_CMD_H__

#include "wifi_abstraction_layer.h"

#define SOCKET_CMD_PORT				8081
#define SOCKET_CMD_BUF_SIZE			256
#define SOCKET_CMD_DOWNLOAD_BUF_SIZE	(2*1024)
#define SOCKET_CMD_FIRMWARE_BUF_SIZE	(2*1024*1024)
#define SOCKET_CMD_SHORT_DELAY		10
#define SOCKET_CMD_TASK_STACKSIZE	512

#define GP_SOCK_CMD_TYPE_OFFSET				8
#define GP_SOCK_CMD_MODE_ID_OFFSET			10
#define GP_SOCK_CMD_CMD_ID_OFFSET			11
#define GP_SOCK_CMD_PAYLOAD_OFFSET			12

#define GP_SOCK_RESP_TYPE_OFFSET			8
#define GP_SOCK_RESP_PAYLOAD_SIZE_OFFSET	12
#define GP_SOCK_RESP_PAYLOAD_OFFSET			14

#define GP_SOCK_DOUBLE_BUFFER_DOWNLOAD		1

#define VLC_RESET_NOTIFICATION					1

#define PLAYBACK_STOP_TIME_OUT  500

enum {	/* GP_SOCK_TYPE */
	GP_SOCK_TYPE_CMD = 0x0001,
	GP_SOCK_TYPE_ACK = 0x0002,
	GP_SOCK_TYPE_NAK = 0x0003
};

enum {	/* GP_SOCK_CMD */
	GPSOCK_General_SetMode = 0x0000,
	GPSOCK_General_GetDeviceStatus = 0x0001,
	GPSOCK_General_GetParameterFile = 0x0002,
	GPSOCK_General_PowerOff = 0x0003,
	GPSOCK_General_RestarStreaming = 0x0004,
	GPSOCK_General_AuthDevice = 0x0005,
	GPSOCK_Record_Start = 0x0100,
	GPSOCK_Record_Audio = 0x0101,
	GPSOCK_CapturePicture_Capture = 0x0200,
	GPSOCK_Playback_Start = 0x0300,
	GPSOCK_Playback_Pause = 0x0301,
	GPSOCK_Playback_GetFileCount = 0x302,
	GPSOCK_Playback_GetFileList = 0x0303,
	GPSOCK_Playback_GetThumbnail = 0x0304,
	GPSOCK_Playback_GetRawData = 0x0305,
	GPSOCK_Playback_Stop = 0x0306,
	GPSOCK_Playback_GetSpecificName = 0x0307,
	GPSOCK_Playback_DeleteFile = 0x0308,
	GPSOCK_Menu_GetParameter = 0x0400,
	GPSOCK_Menu_SetParameter = 0x0401,
	GPSOCK_Firmware_Download = 0x0500,
	GPSOCK_Firmware_SendRawData = 0x0501,
	GPSOCK_Firmware_Upgrade = 0x0502,
	GPSOCK_Vendor_Command = 0xFFFFFF00
};

enum {	/* MENU ID */
	Record_Resolution = 0x0000,
	Record_Exposure = 0x0001,
	Record_MotionDetection = 0x0002,
	Record_Loop_Recording = 0x0003,
	Record_WDR = 0x0004,
	Record_RecordAudio = 0x0005,
	Record_DateStamp = 0x0006,
	Capture_Resolution = 0x0100,
	Capture_Exposure = 0x0101,
	Capture_Quality = 0x0102,
	Capture_Sequence = 0x0103,
	Capture_Sharpness = 0x0104,
	Capture_ISO = 0x0105,
	Capture_AntiShaking = 0x0106,
	Capture_DateTime = 0x0107,
	Capture_WhiteBalance = 0x0108,
	System_Frequency = 0x0200,
	System_ScreenSaver = 0x0201,
	System_AutoPowerOff = 0x0202,
	System_Language = 0x0203,
	System_BeepSound = 0x0204,
	System_DataTime = 0x0205,
	System_ClearBuffer = 0x0206,
	System_Format = 0x0207,
	System_DefaultSetting = 0x0208,
	System_Version = 0x0209,
	WiFi_Name = 0x300,
	WiFi_Passwd = 0x301,
	Firmware_Version = 0x20B,
	System_SyncTime = 0x20A
};

enum {
	NAK_OK = 0,
	NAK_Server_is_busy = -1,
	NAK_Invalid_command = -2,
	NAK_Request_timeout = -3,
	NAK_Mode_error = -4,
	NAK_No_storage = -5,
	NAK_Write_Fail = -6,
	NAK_Get_file_list_fail = -7,
	NAK_Get_thumbnail_fail = -8,
	NAK_Full_storage = -9,
	NAK_Battery_low = -10,
	NAK_Mem_malloc_error = -11,
	NAK_Checksum_error = -12,
	NAK_SyncTime_error = -13
	
};

enum {
	GPSOCK_General_DeviceStatus_Length = 20,	//14,
	GPSOCK_General_Key_Length = (4+2),
	GPSOCK_Vendor_Command_Buf_Length = 8
};

enum {
	GPSOCK_WiFi_Name_Length = 8,
	#if WIFI_ENCRYPTION_METHOD == WPA2
	GPSOCK_WiFi_Passwd_Length = GPSOCK_WiFi_Name_Length,
	#else
	GPSOCK_WiFi_Passwd_Length = 5,
	#endif
	GPSOCK_Sync_Time_Buf_Length = 6
};

enum {
	WIFI_DOWNLOAD_IDLE = 0,
	WIFI_DOWNLOAD_ACTIVE = 1
};

enum
{
	RECORD_MODE = 0x00,
	CAPTURE_MODE = 0x01,
	BROWSE_MODE = 0x02,
	FALSE_MENU_MODE = 0x03
};

typedef struct socket_cmd_ctl_s
{
	netconn_ptr socket_cmd_server;
	netconn_ptr socket_cmd_client;
} socket_cmd_ctl_t;	

extern void socket_cmd_start_service(void);
extern void socket_cmd_stop_service(void);
extern void socket_cmd_stop_service_flag_set(void);
extern INT32S gp_wifi_upgrade_check(INT8U **p_Firmware , INT32U *p_Size);

extern char wifi_ssid[];
extern char wifi_password[];
#if WIFI_SSID_ADD_MACADDR == 1
extern char mac_addr[];
#endif
extern INT32U wifi_dl_cur_size;
extern INT32U curr_wifi, curr_buf;
extern INT32U buf_dl_flag[2];	// A-B buffer switch
extern INT32U buf_dl_size[2];
extern INT32U wifi_dl_state;
extern INT32U wifi_dl_sd_get_size;
extern INT8U *wifi_dl_buf_A;
extern INT8U *wifi_dl_buf_B;
extern INT16U wifi_dl_play_file_index;

extern STOR_SERV_DOWNLOAD_FILEINFO download_file_info;

#endif	//__SOCKET_CMD_H__

