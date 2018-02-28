/************************************************************
* wifi_demo.c
*
* Purpose: WiFi demo code
*
* Author: Eugenehsu
*
* Date: 2015/10/05
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 1.10
* History :
*
************************************************************/
#include "string.h"
#include "stdio.h"
#include "project.h"
#include "application.h"
#include "video_encoder.h"
#include "avi_encoder_app.h"
#include "gspi_cmd.h"
#include "gp_cmd.h"
#include "wifi_demo.h"

/**********************************************
*	Definitions
**********************************************/
/**********************************************
*	Extern variables and functions
**********************************************/
extern gp_socket_cmd_init(void);
extern mjpeg_streamer_init(void);
extern INT32U send_mjpeg;
/*********************************************
*	Variables declaration
*********************************************/
static INT32U wifi_demo_flag = WIFI_GP_SOCKET_DEMO;

static void _start_wifi_service(INT32U flag)
{
	INT32U cnt = 10;
	gp_wifi_state.mode = 0;
	
	/* Set AP SSID */
	gspi_send_docmd(AT_CMD_SET_AP_SSID);
	/* Set AP security key */
	gspi_send_docmd(AT_CMD_SET_AP_SECURE_KEY);
	/* Set AP channel */
	gspi_send_docmd(AT_CMD_SET_AP_CHANNEL);
	/* Enable AP */
	gspi_send_docmd(AT_CMD_ACTIVATE_AP);
	
	gspi_send_docmd(GP_CMD_GET_WIFISTATUS);
	while(cnt--)
	{
		/* Wait for 3 seconds */
		OSTimeDly(200);
		
		if(gp_wifi_state.mode == WIFI_AP_MODE)
		{
			DBG_PRINT("Start service, WIFI_AP_MODE\r\n");
			break;
		}
		gspi_send_docmd(GP_CMD_GET_WIFISTATUS);	
	}
	
	/* AP mode enable, start MJPEG streamer */
	if(gp_wifi_state.mode == WIFI_AP_MODE)
	{
		if(flag == WIFI_GP_SOCKET_DEMO)
		{
			/* Config GP socket command port */
			gspi_send_docmd(GP_CMD_CONFIG_GP_SOCKET_PORT);
			gp_net_app_state.isupdated = 0;
			gspi_send_docmd(GP_CMD_ENABLE_GP_SOCKET);
			cnt = 10;
			while((gp_net_app_state.isupdated == 0) && cnt)
			{
				cnt--;
				OSTimeDly(200);
			}	
			if(!gp_net_app_state.gp_socket_state)
			{
				DBG_PRINT("Enale GP socket service failed\r\n");
			}	
		}	
		else if(flag == WIFI_MJPEG_STREAMER_MODE)
		{
			/* Config GP socket command port */
			gspi_send_docmd(GP_CMD_CONFIG_MJPEG_STREAMER_PORT);
			gp_net_app_state.isupdated = 0;
			gspi_send_docmd(GP_CMD_ENABLE_MJPEG_STREAMER);
			cnt = 10;
			while((gp_net_app_state.isupdated == 0) && cnt)
			{
				cnt--;
				OSTimeDly(200);
			}	
			if(!gp_net_app_state.mjpeg_streamer_state)
			{
				DBG_PRINT("Enale MJPEG streamer failed\r\n");
			}	
		}
		else if(flag == WIFI_SOCKET_MJPEG_DEMO)
		{
			/* MJPEG + GP socket command demo */
			gspi_send_docmd(GP_CMD_CONFIG_GP_SOCKET_PORT);
			gp_net_app_state.isupdated = 0;
			gspi_send_docmd(GP_CMD_ENABLE_GP_SOCKET);
			cnt = 10;
			while((gp_net_app_state.isupdated == 0) && cnt)
			{
				cnt--;
				OSTimeDly(200);
			}
			if(!gp_net_app_state.gp_socket_state)
			{
				DBG_PRINT("Enable GP socket service failed\r\n");
			}	
			gspi_send_docmd(GP_CMD_CONFIG_MJPEG_STREAMER_PORT);
			gp_net_app_state.isupdated = 0;
			gspi_send_docmd(GP_CMD_ENABLE_MJPEG_STREAMER);
			cnt = 10;
			while((gp_net_app_state.isupdated == 0) && cnt)
			{
				cnt--;
				OSTimeDly(200);
			}
			
			if(!gp_net_app_state.mjpeg_streamer_state)
			{
				DBG_PRINT("Enable MJPEG streamer failed\r\n");
			}
		}
	}
	else
	{
		DBG_PRINT("Enable AP mode failed\r\n");
	}			

	/* Before getting WiFi setting, set isupdated to 0 */
	gp_wifi0_setting.isupdated = 0;
	gspi_send_docmd(GP_CMD_GET_WIFI0_SETTING);
}	

static void _stop_wifi_service(INT32U flag)
{
	INT32U cnt = 5;
	
	if(flag == WIFI_GP_SOCKET_DEMO)
	{
		/* Stop GP socket */
		gspi_send_docmd(GP_CMD_DISABLE_GP_SOCKET);
	}	
	else if(flag == WIFI_MJPEG_STREAMER_MODE)
	{
		/* Stop MJPEG streamer */
		gspi_send_docmd(GP_CMD_DISABLE_MJPEG_STREAMER);
	}
	else if(flag == WIFI_SOCKET_MJPEG_DEMO)
	{
		/* Stop GP socket + MJPEG streamer */
		gspi_send_docmd(GP_CMD_DISABLE_GP_SOCKET);
		gspi_send_docmd(GP_CMD_DISABLE_MJPEG_STREAMER);
	}	
	
	/* Disconnect WiFI */
	gspi_send_docmd(AT_CMD_WIFI_DISCONNECT);
	/* Get WiFi module status */
	gspi_send_docmd(GP_CMD_GET_WIFISTATUS);
	while(cnt--)
	{
		/* Wait for 1 seconds */
		OSTimeDly(100);
		
		if(gp_wifi_state.mode == WIFI_NONE_MODE)
		{
			DBG_PRINT("Stop service, WIFI_NONE_MODE\r\n");
			break;
		}
		gspi_send_docmd(GP_CMD_GET_WIFISTATUS);	
	}
}	

void wifi_demo_init(INT32U flag)
{
	wifi_demo_flag = flag;
	
	if(wifi_demo_flag == WIFI_GP_SOCKET_DEMO)
	{
		/* GP socket command demo */
		gp_socket_cmd_init();
	}	
	else if(wifi_demo_flag == WIFI_MJPEG_STREAMER_MODE)
	{
		/* MJPEG streamer demo */
		mjpeg_streamer_init();
	}
		
	else if(wifi_demo_flag == WIFI_SOCKET_MJPEG_DEMO)
	{
		/* MJPEG + GP socket command demo */
		gp_socket_cmd_init();
		mjpeg_streamer_init();
	}
	
	_start_wifi_service(wifi_demo_flag);
	
	while(1)
	{
		if(gp_wifi0_setting.isupdated)
		{
			gp_wifi0_setting.isupdated = 0;
			DBG_PRINT("==========WiFi Setting==========\r\n");
			DBG_PRINT("Interface: %s\r\n", gp_wifi0_setting.ifname);
			DBG_PRINT("Mode: %s\r\n", (gp_wifi0_setting.mode == WIFI_AP_MODE)?"AP MODE":((gp_wifi0_setting.mode == WIFI_STATION_MODE)?"STATION MODE":"NONE"));
			DBG_PRINT("SSID: %s\r\n", gp_wifi0_setting.ssid);
			DBG_PRINT("Channel: %d\r\n", gp_wifi0_setting.channel);
			DBG_PRINT("Password: %s\r\n", gp_wifi0_setting.password);
			switch(gp_wifi0_setting.security_type)
			{
				case SECURITY_OPEN:
					DBG_PRINT("Security: OPEN");
					break;
					
				case SECURITY_WEP_PSK:
					DBG_PRINT("Security: WEP");
					DBG_PRINT("Key index: %d", gp_wifi0_setting.key_idx);
					break;
					
		        case SECURITY_WPA_TKIP_PSK:
					DBG_PRINT("Security: TKIP");
					break;
					
				case SECURITY_WPA2_AES_PSK:
					DBG_PRINT("Security: AES");
					break;
					
				default:
					DBG_PRINT("Security: UNKNOWN");
			}
			DBG_PRINT("\r\n==========Interface %s==========\r\n", gp_wifi0_setting.ifname);
			DBG_PRINT("MAC: %x:%x:%x:%x:%x:%x\r\n", gp_wifi0_setting.mac[0], gp_wifi0_setting.mac[1], gp_wifi0_setting.mac[2], gp_wifi0_setting.mac[3]
					, gp_wifi0_setting.mac[4], gp_wifi0_setting.mac[5]);
			DBG_PRINT("IP : %d.%d.%d.%d\r\n", gp_wifi0_setting.ip[0], gp_wifi0_setting.ip[1], gp_wifi0_setting.ip[2], gp_wifi0_setting.ip[3]);
			DBG_PRINT("GW : %d.%d.%d.%d\r\n", gp_wifi0_setting.gw[0], gp_wifi0_setting.gw[1], gp_wifi0_setting.gw[2], gp_wifi0_setting.gw[3]);
		}
				
		OSTimeDly(100);
	}	
}	