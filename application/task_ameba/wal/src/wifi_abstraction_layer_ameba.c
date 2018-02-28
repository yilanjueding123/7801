/******************************************************
* wifi_abstraction_layer.c
*
* Purpose: WiFi abstraction layer. Applications using WiFi functions should
*          call APIs in this file instead of the APIs of a specific vendor.
*
* Date: 2015/10/21
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 
* History :
*
*******************************************************/
#include "project.h"
#include "application.h"
#include "gp_stdlib.h"
#include "gp_cmd.h"
#include "gspi_cmd.h"
#include "wifi_abstraction_layer_ameba.h"
#include "socket_cmd.h"

//=============================================================================
// Define
//=============================================================================
#define WAL_AP_DEFAULT_CHANNEL	6
#define WAL_SSID_LEN			8

#define WAL_AT_SSID_STR			"ATW3="
#define WAL_AT_PASSWORD_STR		"ATW4="
#define WAL_AT_CHANNEL_STR		"ATW5="

#define WAL_AT_PREFIX_LEN		5	// such as 'ATW3='
#define WAL_AT_SSID_LEN			(WAL_AT_PREFIX_LEN + WAL_SSID_LEN)
#define WAL_AT_PASSWORD_LEN		(WAL_AT_PREFIX_LEN + GPSOCK_WiFi_Passwd_Length)
#define WAL_AT_CHANNEL_LEN		(WAL_AT_PREFIX_LEN + 2)	// channel is at most 2 digits such as '12'

//=============================================================================
// Static variables
//=============================================================================
static INT8U wifi_channel = WAL_AP_DEFAULT_CHANNEL;

//=============================================================================
// External variables
//=============================================================================
extern char wifi_ssid[];
extern char wifi_password[];

//=============================================================================
// External functions
//=============================================================================
extern INT8S *itoa(INT16S value, INT8S *string, INT16S radix);

//=============================================================================
// Note: Initialize WiFi
//=============================================================================
INT32S wal_init(INT32S argc, CHAR *argv[])
{
	return WAL_RET_SUCCESS;
}

//=============================================================================
// Note: Set SSID name
//=============================================================================
void wal_set_ap_channel(INT8U channel)
{
	wifi_channel = channel;
}

//=============================================================================
// Note: Set SSID name
//=============================================================================
void wal_set_ap_ssid(CHAR *ssid, INT8U len)
{
	INT8U ssid_len;

	ssid_len = (len <= WAL_SSID_LEN) ? len : WAL_SSID_LEN;
	gp_memset((INT8S*)wifi_ssid , 0 , WAL_SSID_LEN);
	gp_memcpy((INT8S*)wifi_ssid, (INT8S*)ssid, ssid_len);
}

//=============================================================================
// Note: Set password
//=============================================================================
void wal_set_ap_password(CHAR *password, INT8U len)
{
	INT8U password_len;

	password_len = (len <= GPSOCK_WiFi_Passwd_Length) ? len : GPSOCK_WiFi_Passwd_Length;
	gp_memset((INT8S*)wifi_password , 0 , GPSOCK_WiFi_Passwd_Length);
	gp_memcpy((INT8S*)wifi_password, (INT8S*)password, password_len);
}

//=============================================================================
// Note: Enable the LDO of the WiFi module
//=============================================================================
void wal_ldo_enable(void)
{
	// Ameba does not have ldo_enable pin
	return;
}

//=============================================================================
// Note: Disable the LDO of the WiFi module
//=============================================================================
void wal_ldo_disable(void)
{
	// Ameba does not have ldo_enable pin
	return;
}

//=============================================================================
// Note: Enable WiFi AP mode
//=============================================================================
INT32S wal_ap_mode_enable(void)
{
	INT32U cnt = 10;
	CHAR at_ssid[WAL_AT_SSID_LEN+1];
	CHAR at_password[WAL_AT_PASSWORD_LEN+1];
	CHAR at_channel[WAL_AT_CHANNEL_LEN+1];
	CHAR channel[3];

	gp_wifi_state.mode = WIFI_NONE_MODE;

	/* Set AP SSID */
	gp_memset((INT8S*)at_ssid, 0, WAL_AT_SSID_LEN+1);
	gp_strcpy((INT8S*)at_ssid, (INT8S*)WAL_AT_SSID_STR);
	gp_strcat((INT8S*)at_ssid, (INT8S*)wifi_ssid);
	gspi_send_docmd(at_ssid);
	DBG_PRINT("[%s] %s\r\n", __func__, at_ssid);

	/* Set AP security key */
	gp_memset((INT8S*)at_password, 0, WAL_AT_PASSWORD_LEN+1);
	gp_strcpy((INT8S*)at_password, (INT8S*)WAL_AT_PASSWORD_STR);
	//gp_strcat((INT8S*)at_password, (INT8S*)wifi_password);
	gp_strcat((INT8S*)at_password, (INT8S*)"12345678");
	gspi_send_docmd(at_password);
	DBG_PRINT("[%s] %s\r\n", __func__, at_password);

	/* Set AP channel */
	gp_memset((INT8S*)at_channel, 0, WAL_AT_CHANNEL_LEN+1);
	gp_strcpy((INT8S*)at_channel, (INT8S*)WAL_AT_CHANNEL_STR);
	itoa(wifi_channel, (INT8S*)channel, 10);
	gp_strcat((INT8S*)at_channel, (INT8S*)channel);
	gspi_send_docmd(at_channel);
	DBG_PRINT("[%s] %s\r\n", __func__, at_channel);

	/* Enable AP */
	gspi_send_docmd(AT_CMD_ACTIVATE_AP);
	
	gspi_send_docmd(GP_CMD_GET_WIFISTATUS);
	while(cnt--)
	{
		/* Wait for 2 seconds */
		OSTimeDly(200);
		
		if(gp_wifi_state.mode == WIFI_AP_MODE)
		{
			DBG_PRINT("Start service, WIFI_AP_MODE\r\n");
			break;
		}
		gspi_send_docmd(GP_CMD_GET_WIFISTATUS);	
	}

	/* Before getting WiFi setting, set isupdated to 0 */
	gp_wifi0_setting.isupdated = 0;
	gspi_send_docmd(GP_CMD_GET_WIFI0_SETTING);

	return (gp_wifi_state.mode == WIFI_AP_MODE) ? WAL_RET_SUCCESS : WAL_RET_FAIL;
}

//=============================================================================
// Note: Disable WiFi AP mode
//=============================================================================
INT32S wal_ap_mode_disable(void)
{
	INT32U cnt = 5;

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

	return (gp_wifi_state.mode == WIFI_NONE_MODE) ? WAL_RET_SUCCESS : WAL_RET_FAIL;
}
