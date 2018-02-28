/******************************************************
* wifi_abstraction_layer.c
*
* Purpose: WiFi abstraction layer. Applications using WiFi functions should
*          call APIs in this file instead of the APIs of a specific vendor.
*
* Date: 2015/10/12
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 
* History :
*
*******************************************************/
#include "project.h"
#include "dev.h"
#include "ssv_dev.h"
#include "net_mgr.h"
#include "wifi_abstraction_layer.h"
#include "socket_cmd.h"
#include "host_apis.h"

//=============================================================================
// Define
//=============================================================================
#if WIFI_ENCRYPTION_METHOD == WEP
#define WAL_SEC				SSV6XXX_SEC_WEP_40
#else
#define WAL_SEC				SSV6XXX_SEC_WPA2_PSK
#endif

#define WAL_AP_DEFAULT_CHANNEL	6
#define WAL_SSID_LEN			8

//=============================================================================
// Static variables
//=============================================================================
static INT8U wifi_channel = WAL_AP_DEFAULT_CHANNEL;
static INT8U wifi_ssid_len = GPSOCK_WiFi_Name_Length;
static INT8U wifi_password_len = GPSOCK_WiFi_Passwd_Length;

extern char wifi_ssid[];
extern char wifi_password[];
static INT8U g_wifi_init = 0;
//=============================================================================
// External functions
//=============================================================================
extern int ssv6xxx_dev_init(ssv6xxx_hw_mode hmode);

//=============================================================================
// Note: Initialize WiFi
//=============================================================================
INT32S wal_init(INT32S argc, CHAR *argv[])
{
	return ssv6xxx_dev_init(SSV6XXX_HWM_STA);
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
void wal_set_ap_ssid(CHAR *ssid, INT8U ssid_len)
{
	wifi_ssid_len = (ssid_len <= WAL_SSID_LEN) ? ssid_len : WAL_SSID_LEN;
	gp_memset((INT8S*)wifi_ssid , 0 , WAL_SSID_LEN);
	gp_memcpy((INT8S*)wifi_ssid, (INT8S*)ssid, wifi_ssid_len);
}

//=============================================================================
// Note: Set password
//=============================================================================
void wal_set_ap_password(CHAR *password, INT8U password_len)
{
	wifi_password_len = (password_len <= GPSOCK_WiFi_Passwd_Length) ? password_len : GPSOCK_WiFi_Passwd_Length;
	gp_memset((INT8S*)wifi_password , 0 , GPSOCK_WiFi_Passwd_Length);
	gp_memcpy((INT8S*)wifi_password, (INT8S*)password, wifi_password_len);
}

//=============================================================================
// Note: Enable the LDO of the WiFi module
//=============================================================================
void wal_ldo_enable(void)
{
#if ((GPDV_BOARD_VERSION == GPCV1248_V1_0)||(GPDV_BOARD_VERSION == GPCV4247_WIFI))
	gpio_write_io(WIFI_LDO_EN, DATA_HIGH);
	OSTimeDly(1);
#endif
}

//=============================================================================
// Note: Disable the LDO of the WiFi module
//=============================================================================
void wal_ldo_disable(void)
{
#if ((GPDV_BOARD_VERSION == GPCV1248_V1_0)||(GPDV_BOARD_VERSION == GPCV4247_WIFI))
	gpio_write_io(WIFI_LDO_EN, DATA_LOW);
#endif
}
//=============================================================================
// Note: Enable WiFi AP mode
//=============================================================================
#if WIFI_SSID_ADD_MACADDR == 1
void char_transfer(char *dst,char *src,int len)
{
	INT8U i=0;
	INT8U temp,temp1;
	
	for(i=0;i<len;i++)
	{
		temp = *(src+i)>>4;
		if(temp<10)
		{
			*(dst+i*2) = temp+0x30;//'0'
		}
		else
		{
			//*(dst+i*2) = temp+0x61-10;//'a'
			*(dst+i*2) = temp+0x41-10;//'A'
		}
		temp1 = *(src+i)&0x0F;
		if(temp1<10)
		{
			*(dst+i*2+1) = temp1+0x30;//'0'
		}
		else
		{
			//*(dst+i*2+1) = temp1+0x61-10;//'a'
			*(dst+i*2+1) = temp1+0x41-10;//'A'
		}
	}
}
#endif

INT32S wal_ap_mode_enable(void)
{
	INT32S nRet;
	INT8U tmep_x = 0;

	INT32S ret = WAL_RET_SUCCESS;
	Ap_setting ap;
	#if WIFI_SSID_ADD_MACADDR == 1
	char temp_ssid[GPSOCK_WiFi_Name_Length+12];
	#endif
	OS_SEM_DATA sem_data;
	INT8U ssid_length;

	gp_memset((INT8S*)(&ap) , 0 , sizeof(Ap_setting));
	ap.status = TRUE;
	gp_memcpy((INT8S*)ap.password, (INT8S*)wifi_password, wifi_password_len);
	{
		int i;
		DBG_PRINT("PASSWD: ");
		for (i=0;i<wifi_password_len;++i)
			DBG_PRINT("%c",wifi_password[i]);
		DBG_PRINT("\r\n");
	}
	ap.channel = EN_CHANNEL_AUTO_SELECT;

	#if WIFI_ENCRYPTION_METHOD == WPA2
             ap.security = SSV6XXX_SEC_WPA2_PSK;
  	      ap.proto = WPA_PROTO_RSN;
             ap.key_mgmt = WPA_KEY_MGMT_PSK ;
             ap.group_cipher=WPA_CIPHER_CCMP;
             ap.pairwise_cipher = WPA_CIPHER_CCMP;
	#else
	      ap.security = SSV6XXX_SEC_NONE;
	#endif
	
	for(ssid_length=0;ssid_length<GPSOCK_WiFi_Name_Length;ssid_length++)
	{
		if(wifi_ssid[ssid_length]==0)
		{
			break;
		}
	}
	#if WIFI_SSID_ADD_MACADDR == 1
	gp_memcpy((INT8S*)temp_ssid,(INT8S*)wifi_ssid, ssid_length);
	//==================================================================
	nRet = gp_strncmp((INT8S*)temp_ssid, (INT8S *)WIFI_SSID_NAME, 8);
	if (nRet == 0)
	{
		ap.ssid.ssid_len = ssid_length+1+4;//+12
		temp_ssid[ssid_length] = '_';
		char_transfer(&temp_ssid[ssid_length+1],&mac_addr[4],2); //6
	}
	else
	{
		ap.ssid.ssid_len = ssid_length;
	}
	tmep_x = ap.ssid.ssid_len;
	//==================================================================
	gp_memcpy((INT8S*)ap.ssid.ssid,(INT8S*)temp_ssid, ap.ssid.ssid_len);
	{
		int i;
		DBG_PRINT("SSID: ");
		//for (i=0;i<ssid_length+12;++i)
		for (i=0;i<tmep_x;++i)
			DBG_PRINT("%c",ap.ssid.ssid[i]);
		DBG_PRINT("\r\n");
	}
	#else
	ap.ssid.ssid_len = ssid_length;
	gp_memcpy((INT8S*)ap.ssid.ssid,(INT8S*)wifi_ssid, ssid_length);
	{
		int i;
		DBG_PRINT("SSID: ");
		for (i=0;i<ssid_length;++i)
			DBG_PRINT("%c",wifi_ssid[i]);
		DBG_PRINT("\r\n");
	}
	#endif
	if (netmgr_wifi_control_async(SSV6XXX_HWM_AP, &ap, NULL) == SSV6XXX_FAILED)
		ret = WAL_RET_FAIL;
	g_wifi_init = 1;
	return ret;
}

//=============================================================================
// Note: Disable WiFi AP mode
//=============================================================================
INT32S wal_ap_mode_disable(void)
{
	INT32S ret = WAL_RET_SUCCESS;
	OS_SEM_DATA sem_data;
#if ((GPDV_BOARD_VERSION == GPCV1248_V1_0)||(GPDV_BOARD_VERSION == GPCV4247_WIFI))
	
	netmgr_wifi_ap_off();
	OS_MsDelay(100);
	ssv6xxx_drv_irq_disable();
	wal_ldo_disable();
#else
	Sta_setting sta;

	gp_memset((INT8S*)(&sta), 0 , sizeof(Sta_setting));
	sta.status = FALSE;

	if (netmgr_wifi_control_async(SSV6XXX_HWM_STA, NULL, &sta) == SSV6XXX_FAILED)
		ret = WAL_RET_FAIL;
#endif
	g_wifi_init = 0;

	return ret;
}

/*  SSV:
 *  status:
 *  0: wifi disable
 *  1: wifi enable
 *  2: client connectted
 */
INT32S ssv_wifi_module_conn_status(void)
{
    Ap_sta_status info;
    INT32S status = 0;
    if(g_wifi_init == 0){
        return status;
    }

    OS_MemSET(&info , 0 , sizeof(Ap_sta_status));
    ssv6xxx_wifi_status(&info);
    //status = info.u.ap.stainfo[0].status; //0: disconnect, 4:connect

    if(info.status == 0)
    {
        status = 0;
    }
    else if(info.status == 1)
    {
        status = 1;
        if(info.u.ap.stanum)
        {
            status = 2;
        }
    }
    //print_string("@@@conn_status %d %d stanum %d status %d\r\n",info.status,info.u.ap.stainfo[0].status,info.u.ap.stanum,status);
    return status;
}