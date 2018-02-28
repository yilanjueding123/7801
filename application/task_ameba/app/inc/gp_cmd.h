/***************************************************************************
* gp_cmd.h
*
* Purpose: For GP command to record the status in Ameba and send to GP MCU.
*
* Author: Eugene Hsu
*
* Date: 2015/09/24
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 
* History :
*
******************************************************************************/
#ifndef __GP_CMD_H__
#define __GP_CMD_H__

/* FPS structure(NET & GSPI) */
PACKED typedef struct GP_FPS_S
{
	INT32U net_fps;			/* FPS in wifi path */
	INT32U rssi;			/* RSSI */
} GP_FPS_T;

/* WiFi setting structure */
PACKED typedef struct GP_WIFI_SETTING_S
{
	char	ifname[12];
	INT32U	mode;
	char 	ssid[33];
	char	channel;
	INT32U	security_type;
	char 	password[65];
	char	key_idx;
	INT8U	mac[6];
	INT8U	ip[4];
	INT8U	gw[4];
	INT8U   isupdated;		/* Check if updated, this is extra byte to check setting is updated */
} GP_WIFI_SETTING_T;

/* WiFi status structure */
PACKED typedef struct GP_WIFI_STATE_S
{
	INT32U mode;
} GP_WIFI_STATE_T; 

/* NET App service information structure */
PACKED typedef struct GP_NET_APP_S
{
	INT32U mjpeg_streamer_state;
	INT32U gp_socket_state;
	INT8U  isupdated;
} GP_NET_APP_T;

typedef enum
{
	WIFI_NONE_MODE = 0,
	WIFI_STATION_MODE,
	WIFI_AP_MODE,
	WIFI_STA_AP_MODE,
	WIFI_PROMISC_MODE,
	WIFI_P2P_MODE
	
} WIFI_MODE_E;

#define WEP_ENABLED	  0x0001
#define TKIP_ENABLED  0x0002
#define AES_ENABLED   0x0004
#define WSEC_SWFLAG   0x0008

#define SHARED_ENABLED  0x00008000
#define WPA_SECURITY    0x00200000
#define WPA2_SECURITY   0x00400000
#define WPS_ENABLED     0x10000000

#define AT_CMD_SET_AP_SSID					"ATW3=SPORT_DV_WIFI"
#define AT_CMD_SET_AP_SECURE_KEY			"ATW4=0987654321"
#define AT_CMD_SET_AP_CHANNEL				"ATW5=6"
#define AT_CMD_ACTIVATE_AP					"ATWA"
#define AT_CMD_WIFI_DISCONNECT				"ATWD"

#define GP_CMD_GET_WIFI0_SETTING			"GPGS=WIFI0SETTING"
#define GP_CMD_GET_WIFISTATUS				"GPGS=WIFISTATUS"
#define GP_CMD_ENABLE_MJPEG_STREAMER		"GPMJ=1"
#define GP_CMD_DISABLE_MJPEG_STREAMER		"GPMJ=0"
#define GP_CMD_CONFIG_MJPEG_STREAMER_PORT	"GPMP=8080"
#define GP_CMD_ENABLE_GP_SOCKET				"GPSR=1"
#define GP_CMD_DISABLE_GP_SOCKET			"GPSR=0"
#define GP_CMD_CONFIG_GP_SOCKET_PORT		"GPSP=8081"

typedef enum
{
    SECURITY_OPEN           = 0,                                                /**< Open security                           */
    SECURITY_WEP_PSK        = WEP_ENABLED,                                      /**< WEP Security with open authentication   */
    SECURITY_WEP_SHARED     = ( WEP_ENABLED | SHARED_ENABLED ),                 /**< WEP Security with shared authentication */
    SECURITY_WPA_TKIP_PSK   = ( WPA_SECURITY  | TKIP_ENABLED ),                 /**< WPA Security with TKIP                  */
    SECURITY_WPA_AES_PSK    = ( WPA_SECURITY  | AES_ENABLED ),                  /**< WPA Security with AES                   */
    SECURITY_WPA2_AES_PSK   = ( WPA2_SECURITY | AES_ENABLED ),                  /**< WPA2 Security with AES                  */
    SECURITY_WPA2_TKIP_PSK  = ( WPA2_SECURITY | TKIP_ENABLED ),                 /**< WPA2 Security with TKIP                 */
    SECURITY_WPA2_MIXED_PSK = ( WPA2_SECURITY | AES_ENABLED | TKIP_ENABLED ),   /**< WPA2 Security with AES & TKIP           */
    SECURITY_WPA_WPA2_MIXED = ( WPA_SECURITY  | WPA2_SECURITY ),                /**< WPA/WPA2 Security                       */
    
    SECURITY_WPS_OPEN       = WPS_ENABLED,                                      /**< WPS with open security                  */
    SECURITY_WPS_SECURE     = (WPS_ENABLED | AES_ENABLED),                      /**< WPS with AES security                   */

    SECURITY_UNKNOWN        = -1,                                               /**< May be returned by scan function if security is unknown. Do not pass this to the join function! */

     SECURITY_FORCE_32_BIT   = 0x7fffffff                                        /**< Exists only to force rtw_security_t type to 32 bits */
} SRCURITY_T;

/*	Extern variables and functions*/
extern GP_FPS_T	gp_fps;
extern GP_WIFI_STATE_T gp_wifi_state;
extern GP_WIFI_SETTING_T gp_wifi0_setting;
extern GP_WIFI_SETTING_T gp_wifi1_setting;
extern GP_NET_APP_T gp_net_app_state;

#endif	//__GP_CMD_H__