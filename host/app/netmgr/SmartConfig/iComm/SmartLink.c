/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


//#include <lwip/sockets.h>
//#include <lwip/netif.h>
#include <log.h>
#include "rtos.h"
#include "porting.h"
#include "core/smartConf.h"
#include <SmartConfig/SmartConfig.h>
#include <hctrl.h>
#include <net_mgr.h>
#include <hdr80211.h>
#include <netstack.h>

#if ((SMART_CONFIG_SOLUTION==ICOMM_SMART_LINK)&&(ENABLE_SMART_CONFIG==1))
AP_DETAIL_INFO info;
extern u32 g_channelInterval;
extern u32 g_restartInterval;
/*
Init customer's SmartConfig solution.
Return:
    0: Success
    1: Fail
*/
int CusSmartConfigInit(void)
{
    /*
        The default channel mask is the same with g_sta_channel_mask. The value of g_sta_channel_mask is depend on DEFAULT_STA_CHANNEL_MASK.
        You can force to change the channel mask here.
    */
    //g_SconfigChannelMask=g_sta_channel_mask;
    g_channelInterval=(150/TICK_RATE_MS); //The unit of this variable is tick, the default setting of change channel is 150ms
    g_restartInterval=(60000/TICK_RATE_MS); //The unit of this variable is tick, the default setting of restart SmartLink is 60s
    initSmartLink();
    return 0;
}

/*
Deinit customer's SmartConfig solution.
Return:
    0: Success
    1: Fail
*/
int CusSmartConfigDeInit(void)
{
    resetSmartLink();
    return 0;
}

/*
Flush data after channel change
Return:
    0: Success
    1: Fail
*/
int CusSmartConfigFlushData(void)
{
    resetSmartLink();
    return 0;
}

/*
Feed the data to customer's SmartConfig solution
Return:
    EN_SCONFIG_LOCK: Lock the channel
    EN_SCONFIG_DONE: Get the AP information
    EN_SCONFIG_GOING: SmartConfig is not finished
*/

EN_SCONFIG_STATUS CusSmartConfigRxData(u8 *rx_buf, u32 rx_len)
{
    //#define	_PBUF_HDR80211(p, i)				(*((u8 *)(p)+RXINFO_SIZE + (i)))
    //#define _GET_HDR80211_FC(p)				(((p)->f80211==1) ? (((u16)_PBUF_HDR80211(p, 1) << 8) | _PBUF_HDR80211(p, 0)) : 0)
    //#define _GET_HDR80211_FC_FROMDS(p)		((_GET_HDR80211_FC(p) & M_FC_FROMDS)    >>  9)
    //struct cfg_host_rxpkt *pPktInfo=(struct cfg_host_rxpkt *)(rx_buf-RXINFO_SIZE);

    //bool unicast=pPktInfo->unicast;
    //bool unicast=(rx_buf[4]&0x01)? FALSE: TRUE;

    //bool fromDS=_GET_HDR80211_FC_FROMDS(pPktInfo);
    u16 fc=(rx_buf[0]|(rx_buf[1]<<8));
    bool fromDS=(fc& M_FC_FROMDS)>>9;
    u8 broadcast_mac[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    SLINK_STATUS ret=0;
    //if(TRUE==unicast){
    //    return EN_SCONFIG_GOING;
    //}

    if(fromDS==1){
        //From DS, check Addr1
        if(0!=memcmp(&rx_buf[4],broadcast_mac,sizeof(broadcast_mac))){ //unicast frame
            return EN_SCONFIG_GOING;
        }
    }
    else{
        //To DS, check Addr3
        if(0!=memcmp(&rx_buf[16],broadcast_mac,sizeof(broadcast_mac))){ //unicast frame
            return EN_SCONFIG_GOING;
        }
    }

    rx_process_smartConf(rx_buf,rx_len,fromDS);
    ret=getSmartLinkStatus();

    if(ret==SLINK_STATUS_CHANNEL_LOCKED){
        return EN_SCONFIG_LOCK;
    }
    else if (ret==SLINK_STATUS_COMPLETE){
        return EN_SCONFIG_DONE;
    }
    else {
        return EN_SCONFIG_GOING;
    }

}

/*
Get the AP info
Return:
    0: Success
    1: Fail
*/
int CusSmartGetResult(SMART_COFIG_RESULT *sres)
{

    if(sres==NULL){
        return -1;
    }

    if(0!=getSmartLinkResult(&info)){
        return -1;
    }

    sres->ssid_len=info.ssid_len;
    ssv6xxx_memcpy(sres->ssid,info.ssid,info.ssid_len);
    sres->key_len=info.key_len;
    ssv6xxx_memcpy(sres->key,info.key,info.key_len);
    return 0;

}

// Response SmartPhone after IP address is ready
extern int ssv6xxx_get_cust_mac(u8 *mac);
int CustSmartConfigDone(void)
{
#define SENDER_PORT_NUM 8208
#define SERVER_PORT_NUM (SENDER_PORT_NUM+1) //8208+1 = 8209

#define STA_MAC_LEN 6
#define STA_IP_LEN 4
#define TCP_SEND_LEN (STA_MAC_LEN+STA_IP_LEN)

    int i=0;
    char data_buffer[80];
    u32 phoneIp=0, srcip=0;
    u8 temp, hwaddr[6];
    //ipinfo info;
    //struct netif *netif=netif_list;

    //netmgr_ipinfo_get(WLAN_IFNAME, &info);
    netdev_getipv4info(WLAN_IFNAME, &srcip, NULL, NULL);
    
    netdev_getmacaddr(WLAN_IFNAME, hwaddr);
    /* Receiver connects to server ip-address. */
    getSmartPhoneIP((u8 *)(&phoneIp),sizeof(phoneIp));

    //LOG_PRINTF("IP: %d:%d:%d:%d\r\n",((u8 *)&info.ipv4)[0],((u8 *)&info.ipv4)[1],((u8 *)&info.ipv4)[2],((u8 *)&info.ipv4)[3]);
    //LOG_DEBUGF(LOG_SCONFIG|LOG_LEVEL_ALL,("IP: %d:%d:%d:%d\r\n",((u8 *)&netif->ip_addr.addr)[0],((u8 *)&netif->ip_addr.addr)[1],((u8 *)&netif->ip_addr.addr)[2],((u8 *)&netif->ip_addr.addr)[3]));
    LOG_DEBUGF(LOG_SCONFIG|LOG_LEVEL_ALL,("IP: %d:%d:%d:%d\r\n",((u8 *)&srcip)[0],((u8 *)&srcip)[1],((u8 *)&srcip)[2],((u8 *)&srcip)[3]));
    //LOG_DEBUGF(LOG_SCONFIG|LOG_LEVEL_ALL,("mac: %02x:%02x:%02x:%02x:%02x:%02x\r\n",netif->hwaddr[0],netif->hwaddr[1],netif->hwaddr[2],netif->hwaddr[3],netif->hwaddr[4],netif->hwaddr[5]));
    LOG_DEBUGF(LOG_SCONFIG|LOG_LEVEL_ALL,("mac: %02x:%02x:%02x:%02x:%02x:%02x\r\n",hwaddr[0],hwaddr[1],hwaddr[2],hwaddr[3],hwaddr[4],hwaddr[5]));
    LOG_DEBUGF(LOG_SCONFIG|LOG_LEVEL_ALL,("PhoneIP: %d:%d:%d:%d\r\n",((u8 *)&phoneIp)[0],((u8 *)&phoneIp)[1],((u8 *)&phoneIp)[2],((u8 *)&phoneIp)[3]));
   
    for(i=0;i<STA_MAC_LEN;i++){
        //Transform integer to ASNII.
        //In ASNII Table, "0" is 0x30, "A" is 0x41"
        //0x41-0x0A=0x37
        // "1" = 0x30+1; "2" = 0x30+2
        // "A" = 0x37 + 10; "B" = 0x37 + 11
        temp=((hwaddr[i]>>4)&0x0F);
        data_buffer[i*3]=((temp<10)?(temp+0x30):(temp+0x37));
        temp=(hwaddr[i]&0x0F);
        data_buffer[i*3+1]=((temp<10)?(temp+0x30):(temp+0x37));
        data_buffer[i*3+2]=':';
        //LOG_PRINTF("%x %x %x \r\n",data_buffer[i*3],data_buffer[i*3+1],data_buffer[i*3+2]);
    }
    //Send 17 bytes to smartphone. xx:xx:xx:xx:xx:xx
    LOG_DEBUGF(LOG_SCONFIG|LOG_LEVEL_ALL,("Send ... \r\n"));
    if (netstack_tcp_send(data_buffer, (STA_MAC_LEN*3-1), srcip, SENDER_PORT_NUM, phoneIp, SERVER_PORT_NUM) == 0)
    {
        LOG_PRINTF("SmartLink ok\r\n");
        return 0;
    }
   
    LOG_DEBUGF(LOG_SCONFIG|LOG_LEVEL_WARNING,("Send failed \n"));
    return -1;

#undef SENDER_PORT_NUM
#undef SERVER_PORT_NUM

#undef STA_MAC_LEN
#undef STA_IP_LEN
#undef TCP_SEND_LEN

}

#endif //#if ((SMART_CONFIG_SOLUTION==ICOMM_SMART_LINK)&&(ENABLE_SMART_CONFIG==1))

