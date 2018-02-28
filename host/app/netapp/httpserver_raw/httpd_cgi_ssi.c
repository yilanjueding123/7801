#include "lwip/debug.h"
#include "httpd.h"
#include "lwip/tcp.h"
#include <lwip/sockets.h>
#include <lwip/netif.h>
#include <lwip/ip_addr.h>
#include <lwip/dhcp.h>
#include <lwip/sys.h>
#include <netapp/net_app.h>

#include "http_fs.h"
#include <log.h>

#include <ssv_ex_lib.h>
#include <netmgr/net_mgr.h>

#if HTTPD_SUPPORT

#define NUM_CONFIG_CGI_URIS	(sizeof(ppcURLs) / sizeof(tCGI))
#define NUM_CONFIG_SSI_TAGS	(sizeof(ppcTAGs) / sizeof(char *))

const char* LOGIN_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char* WIFI_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char* NETCFG_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char* APLIST_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char* SCAN_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
extern int ssv6xxx_aplist_get_count();
extern void ssv6xxx_aplist_get_str(char *wifi_list_str, int page);

#define WIFI_MODE_TAG_NAME  "wifimode"
#define SSID_TAG_NAME       "ssid"
#define ENCRYPT_TAG_NAME    "encrypt"
#define IPMODE_TAG_NAME     "ipmode"
#define IPADDR_TAG_NAME     "ipaddr"
#define MASK_TAG_NAME       "mask"
#define GATEWAY_TAG_NAME    "gateway"
#define APLIST_TAG_NAME     "aplist"
#define APLIST_COUNT_TAG_NAME "aplistcount"
#define APLIST_PAGE_TAG_NAME "aplistpage"
#define CHANNEL_TAG_NAME     "channel"

static const char *ppcTAGs[]=
{
	WIFI_MODE_TAG_NAME,
	SSID_TAG_NAME,
	ENCRYPT_TAG_NAME,
	IPMODE_TAG_NAME,
	IPADDR_TAG_NAME,
	MASK_TAG_NAME,
	GATEWAY_TAG_NAME,
	APLIST_TAG_NAME,
	APLIST_COUNT_TAG_NAME,
	APLIST_PAGE_TAG_NAME,
	CHANNEL_TAG_NAME,
};


static const tCGI ppcURLs[]=
{
    {"/login.cgi",LOGIN_CGI_Handler},
    {"/wifimode.cgi",WIFI_CGI_Handler},
    {"/netcfg.cgi",NETCFG_CGI_Handler},
    {"/scan.cgi",SCAN_CGI_Handler},
    {"/aplist.cgi",APLIST_CGI_Handler},
};

static int g_aplist_page = 1;

static int FindCGIParameter(const char *pcToFind,char *pcParam[],int iNumParams)
{
	int iLoop;
	for(iLoop = 0;iLoop < iNumParams;iLoop ++ )
	{
		if(STRCMP(pcToFind,pcParam[iLoop]) == 0)
		{
			return (iLoop);
		}
	}
	return (-1);
}

void WIFImode_Handler(char *pcInsert)
{
    int ret = 0;
    wifi_mode mode;
    bool status;

    ret = netmgr_wifi_mode_get(&mode, &status);

    if ((SSV6XXX_HWM_STA == mode) && (status == true))
    {
        STRCPY(pcInsert, "Station");
    }
    else if ((SSV6XXX_HWM_AP == mode) && (status == true))
    {
        STRCPY(pcInsert, "Ap");
    }
    else if ((SSV6XXX_HWM_SCONFIG == mode) && (status == true))
    {
        STRCPY(pcInsert, "SConfig");
    }
    else
    {
        STRCPY(pcInsert, "Unknow");
    }
}

void SSID_Handler(char *pcInsert)
{
    int ret = -1;
    Ap_sta_status *info;

    info = (Ap_sta_status *)MALLOC(sizeof(Ap_sta_status));
    if(NULL==info)
    {
        LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
        return;
    }
    ret = netmgr_wifi_info_get(info);

    if ((SSV6XXX_HWM_STA == info->operate ) && (info->status == true))
    {
        //STRCPY(pcInsert, (void*)info->u.station.ssid.ssid);
        MEMCPY((void*)pcInsert, (void*)info->u.station.ssid.ssid,info->u.station.ssid.ssid_len);
    }
    else if ((SSV6XXX_HWM_AP == info->operate ) && (info->status == true))
    {
        //STRCPY(pcInsert, (void*)info->u.ap.ssid.ssid);
        MEMCPY((void*)pcInsert, (void*)info->u.ap.ssid.ssid,info->u.ap.ssid.ssid_len);
    }
    else if ((SSV6XXX_HWM_SCONFIG == info->operate ) && (info->status == true))
    {
        //STRCPY(pcInsert, (void*)info->u.station.ssid.ssid);
        MEMCPY((void*)pcInsert, (void*)info->u.station.ssid.ssid,info->u.station.ssid.ssid_len);
    }
    else
    {
        STRCPY(pcInsert, "");
    }

    FREE(info);
}

void Encrypt_Handler(char *pcInsert)
{
    int ret = -1;
    Ap_sta_status *info;

    info = (Ap_sta_status *)MALLOC(sizeof(Ap_sta_status));
    if(NULL==info)
    {
        LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
        return;
    }

    ret = netmgr_wifi_info_get(info);

    if ((SSV6XXX_HWM_STA == info->operate ) && (info->status == true))
    {
        if (info->u.station.capab_info&BIT(4))
        {
            STRCPY(pcInsert, info->u.station.proto&WPA_PROTO_WPA?"WPA":
                (info->u.station.proto&WPA_PROTO_RSN?"WPA2":"WEP"));
        }
        else
        {
            STRCPY(pcInsert, "OPEN");
        }
    }
    else if ((SSV6XXX_HWM_AP == info->operate ) && (info->status == true))
    {
        if(info->u.ap.key_mgmt == WPA_KEY_MGMT_NONE)
        {
            if(info->u.ap.pairwise_cipher == WPA_CIPHER_NONE)
                STRCPY(pcInsert, "OPEN");
            else
                STRCPY(pcInsert, "WEP");
        }
        else
        {
            STRCPY(pcInsert, "WPA2");
        }
        //STRCPY(pcInsert, info->u.ap.proto&WPA_PROTO_WPA?"WPA":
        //    (info->u.ap.proto&WPA_PROTO_RSN?"WPA2":"OPEN"));
    }
    else if ((SSV6XXX_HWM_SCONFIG == info->operate ) && (info->status == true))
    {
        if (info->u.station.capab_info&BIT(4))
        {
            STRCPY(pcInsert, info->u.station.proto&WPA_PROTO_WPA?"WPA":
                (info->u.station.proto&WPA_PROTO_RSN?"WPA2":"WEP"));
        }
        else
        {
            STRCPY(pcInsert, "OPEN");
        }
    }
    else
    {
        STRCPY(pcInsert, "");
    }

    FREE(info);
}

void IPmode_Handler(char *pcInsert)
{
    int ret = -1;
    bool dhcpd_status;
    bool dhcpc_status;

    ret = netmgr_dhcp_status_get(&dhcpd_status, &dhcpc_status);
    if (dhcpd_status)
    {
        STRCPY(pcInsert, "DHCP server");
    }
    else if(dhcpc_status)
    {
        STRCPY(pcInsert, "DHCP client");
    }
    else
    {
        STRCPY(pcInsert, "Static");
    }
}

void IPaddr_Handler(char *pcInsert)
{
    struct netif *netif;

    netif = netif_find(WLAN_IFNAME);
    if (netif)
    {
        sprintf(pcInsert, "%d.%d.%d.%d", IPV4_ADDR(&netif->ip_addr.addr));
    }
}

void Mask_Handler(char *pcInsert)
{
    struct netif *netif;

    netif = netif_find(WLAN_IFNAME);
    if (netif)
    {
        sprintf(pcInsert, "%d.%d.%d.%d", IPV4_ADDR(&netif->netmask.addr));
    }
}

void Gateway_Handler(char *pcInsert)
{
    struct netif *netif;

    netif = netif_find(WLAN_IFNAME);
    if (netif)
    {
        sprintf(pcInsert, "%d.%d.%d.%d", IPV4_ADDR(&netif->gw.addr));
    }
}

void Aplist_Handler(char *pcInsert)
{
    ssv6xxx_aplist_get_str(pcInsert, g_aplist_page);
}

void AplistCount_Handler(char *pcInsert)
{
    sprintf(pcInsert, "%d", ssv6xxx_aplist_get_count());
}

void AplistPage_Handler(char *pcInsert)
{
    sprintf(pcInsert, "%d", g_aplist_page);
}

void Channel_Handler(char *pcInsert)
{
    int ret = -1;
    Ap_sta_status *info;

    info = (Ap_sta_status *)MALLOC(sizeof(Ap_sta_status));
    if(NULL==info)
    {
        LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
        return;
    }

    ret = netmgr_wifi_info_get(info);

    if (ret == 0)
    {
        if ((SSV6XXX_HWM_STA == info->operate ) && (info->status == true))
        {
            sprintf(pcInsert, "%d", info->u.station.channel);
        }
        else if ((SSV6XXX_HWM_AP == info->operate ) && (info->status == true))
        {
            sprintf(pcInsert, "%d", info->u.ap.channel);
        }
        else if ((SSV6XXX_HWM_SCONFIG == info->operate ) && (info->status == true))
        {
            //sprintf(pcInsert, "");
            pcInsert[0] = '\0';
        }
        else
        {
            //sprintf(pcInsert, "");
            pcInsert[0] = '\0';
        }
    }

    FREE(info);
}


const char * SSIGetTagName(int iIndex)
{
    if (iIndex < (int)NUM_CONFIG_SSI_TAGS)
    {
        return ppcTAGs[iIndex];
    }
    else
    {
        return NULL;
    }
}

static u16_t SSIHandler(int iIndex,char *pcInsert,int iInsertLen)
{
    char *TagName = NULL;

    TagName = (char *)SSIGetTagName(iIndex);

    if (TagName == NULL) return 0;

	if (STRCMP(TagName, WIFI_MODE_TAG_NAME) == 0)
	{
        WIFImode_Handler(pcInsert);
    }
	else if (STRCMP(TagName, SSID_TAG_NAME) == 0)
	{
        SSID_Handler(pcInsert);
	}
    else if (STRCMP(TagName, ENCRYPT_TAG_NAME) == 0)
    {
	    Encrypt_Handler(pcInsert);
    }
    else if (STRCMP(TagName, IPMODE_TAG_NAME) == 0)
    {
        IPmode_Handler(pcInsert);
    }
    else if (STRCMP(TagName, IPADDR_TAG_NAME) == 0)
    {
        IPaddr_Handler(pcInsert);
    }
    else if (STRCMP(TagName, MASK_TAG_NAME) == 0)
    {
        Mask_Handler(pcInsert);
    }
    else if (STRCMP(TagName, GATEWAY_TAG_NAME) == 0)
    {
        Gateway_Handler(pcInsert);
    }
    else if (STRCMP(TagName, APLIST_TAG_NAME) == 0)
    {
        Aplist_Handler(pcInsert);
    }
    else if (STRCMP(TagName, APLIST_COUNT_TAG_NAME) == 0)
    {
        AplistCount_Handler(pcInsert);
    }
    else if (STRCMP(TagName, APLIST_PAGE_TAG_NAME) == 0)
    {
        AplistPage_Handler(pcInsert);
    }
    else if (STRCMP(TagName, CHANNEL_TAG_NAME) == 0)
    {
        Channel_Handler(pcInsert);
    }

	return STRLEN(pcInsert);
}

const char* LOGIN_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
  LOG_PRINTF("LOGIN_CGI_Handler\r\n");

  iIndex = FindCGIParameter("username",pcParam,iNumParams);
  if (iIndex != -1)
  {
      LOG_PRINTF("username: %s\r\n", pcValue[iIndex]);
  }

  iIndex = FindCGIParameter("password",pcParam,iNumParams);
  if (iIndex != -1)
  {
      LOG_PRINTF("password: %s\r\n", pcValue[iIndex]);
  }

  return "/run_status.shtml";
}

const char* WIFI_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    wifi_mode mode;
    wifi_ap_cfg ap_cfg;
    wifi_sta_join_cfg join_cfg;

    LOG_PRINTF("WIFI_CGI_Handler\r\n");
    MEMSET((void * )&ap_cfg, 0, sizeof(ap_cfg));
    MEMSET((void * )&join_cfg, 0, sizeof(join_cfg));

    ap_cfg.channel = EN_CHANNEL_AUTO_SELECT;
    iIndex = FindCGIParameter("wifimode",pcParam,iNumParams);
    if (iIndex != -1)
    {
        LOG_PRINTF("wifimode: %s\r\n", pcValue[iIndex]);

        if (STRCMP(pcValue[iIndex],"ap") == 0)
        {
            mode = SSV6XXX_HWM_AP;
        }
        else if(STRCMP(pcValue[iIndex],"sta") == 0)
        {
            mode = SSV6XXX_HWM_STA;
        }
        else
        {
            mode = SSV6XXX_HWM_INVALID;
        }
    }
    else
    {
        return "/run_status.shtml";
    }

    iIndex = FindCGIParameter("ssid",pcParam,iNumParams);
    if (iIndex != -1)
    {
        LOG_PRINTF("ssid: %s\r\n", pcValue[iIndex]);
        if (STRLEN( pcValue[iIndex]) == 0)
        {
            return "/run_status.shtml";
        }

        if (mode == SSV6XXX_HWM_AP)
        {
            //STRCPY((void *)ap_cfg.ssid.ssid, pcValue[iIndex]);
            MEMCPY((void *)ap_cfg.ssid.ssid,pcValue[iIndex],STRLEN(pcValue[iIndex]));
            ap_cfg.ssid.ssid_len = STRLEN(pcValue[iIndex]);
        }
        else if (mode == SSV6XXX_HWM_STA)
        {
            //STRCPY((void *)join_cfg.ssid.ssid, pcValue[iIndex]);
            MEMCPY((void *)join_cfg.ssid.ssid,pcValue[iIndex],STRLEN(pcValue[iIndex]));
            join_cfg.ssid.ssid_len=STRLEN(pcValue[iIndex]);
        }
    }
    else
    {
        return "/run_status.shtml";
    }

    iIndex = FindCGIParameter("key",pcParam,iNumParams);
    if (iIndex != -1)
    {
        LOG_PRINTF("key: %s\r\n", pcValue[iIndex]);
        if (mode == SSV6XXX_HWM_AP)
        {
            STRCPY((void *)(ap_cfg.password), pcValue[iIndex]);
        }
        else if (mode == SSV6XXX_HWM_STA)
        {
            STRCPY((void *)join_cfg.password, pcValue[iIndex]);
        }
        else
        {
            return "/run_status.shtml";
        }
    }

    iIndex = FindCGIParameter("encrypt",pcParam,iNumParams);
    if (iIndex != -1)
    {
        LOG_PRINTF("encrypt: %s\r\n", pcValue[iIndex]);

        if (mode == SSV6XXX_HWM_AP)
        {
            if (STRCMP(pcValue[iIndex],"open") == 0)
            {
                ap_cfg.status = TRUE;
                ap_cfg.security = SSV6XXX_SEC_NONE;
            }
            else if (STRCMP(pcValue[iIndex],"wep") == 0)
            {
                ap_cfg.status = TRUE;
                if (STRLEN((char *)ap_cfg.password) == 5)
                {
                    ap_cfg.security =   SSV6XXX_SEC_WEP_40;
                }
                else if(STRLEN((char *)ap_cfg.password) == 13)
                {
                    ap_cfg.security =   SSV6XXX_SEC_WEP_104;
                }
                else
                {
                    return "/run_status.shtml";
                }
            }
            else if (STRCMP(pcValue[iIndex],"wpa") == 0)
            {
                ap_cfg.status = TRUE;
                ap_cfg.security =   SSV6XXX_SEC_WPA_PSK;
            }
            else if (STRCMP(pcValue[iIndex],"wpa2") == 0)
            {
                ap_cfg.status = TRUE;
                ap_cfg.security =   SSV6XXX_SEC_WPA2_PSK;
            }
            else
            {
                return "/run_status.shtml";
            }
        }
    }

    iIndex = FindCGIParameter("channel",pcParam,iNumParams);
    if (iIndex != -1)
    {
        LOG_PRINTF("channel: %s\r\n", pcValue[iIndex]);
        if (mode == SSV6XXX_HWM_AP)
        {
            ap_cfg.channel = ATOI(pcValue[iIndex]);
            if ((ap_cfg.channel <= 0) || (ap_cfg.channel > 13))
            {
                ap_cfg.channel = EN_CHANNEL_AUTO_SELECT;
            }
        }
    }
    
    if (ap_cfg.security == SSV6XXX_SEC_WPA2_PSK)
    {
        ap_cfg.proto = WPA_PROTO_RSN;
        ap_cfg.key_mgmt = WPA_KEY_MGMT_PSK ;
        ap_cfg.group_cipher=WPA_CIPHER_CCMP;
        ap_cfg.pairwise_cipher = WPA_CIPHER_CCMP;
    }
    
    #if 1
    if (mode == SSV6XXX_HWM_AP)
    {
        netmgr_wifi_switch_async(mode, &ap_cfg, NULL);
    }
    else if (mode == SSV6XXX_HWM_STA)
    {
        netmgr_wifi_switch_async(mode, NULL, &join_cfg);
    }

    #endif
    return "/run_status.shtml";
}


const char* NETCFG_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{

  LOG_PRINTF("NETCFG_CGI_Handler\r\n");

  iIndex = FindCGIParameter("ipmode",pcParam,iNumParams);
  if (iIndex != -1)
  {
      LOG_PRINTF("ipmode: %s\r\n", pcValue[iIndex]);
  }

  iIndex = FindCGIParameter("ipaddr",pcParam,iNumParams);
  if (iIndex != -1)
  {
      LOG_PRINTF("ipaddr: %s\r\n", pcValue[iIndex]);
  }

  iIndex = FindCGIParameter("mask",pcParam,iNumParams);
  if (iIndex != -1)
  {
      LOG_PRINTF("mask: %s\r\n", pcValue[iIndex]);
  }

  iIndex = FindCGIParameter("gateway",pcParam,iNumParams);
  if (iIndex != -1)
  {
      LOG_PRINTF("gateway: %s\r\n", pcValue[iIndex]);
  }

  return "/run_status.shtml";
}

const char* APLIST_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    int totalpage = 0;

    LOG_PRINTF("APLIST_CGI_Handler\r\n");

    iIndex = FindCGIParameter("page",pcParam,iNumParams);
    if (iIndex != -1)
    {
        LOG_PRINTF("page: %s\r\n", pcValue[iIndex]);
        if (ATOI(pcValue[iIndex]) > 0)
        {
            g_aplist_page = ATOI(pcValue[iIndex]);
            totalpage = ssv6xxx_aplist_get_count();
            g_aplist_page = (g_aplist_page > totalpage) ? totalpage : g_aplist_page;
        }
        else
        {
            g_aplist_page = 1;
        }
    }

  return "/wireless_config.shtml";
}

const char* SCAN_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
  LOG_PRINTF("SCAN_CGI_Handler\r\n");

  netmgr_wifi_scan_async(0xffff, 0, 0);

  g_aplist_page = 1;

  return "/wireless_config.shtml";
}


void httpd_ssi_init(void)
{
	http_set_ssi_handler(SSIHandler,ppcTAGs,NUM_CONFIG_SSI_TAGS);
}

void httpd_cgi_init(void)
{
     http_set_cgi_handlers(ppcURLs, NUM_CONFIG_CGI_URIS);
}
#endif /* HTTPD_SUPPORT */

