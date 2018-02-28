/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <log.h>
#include <rtos.h>
#include <porting.h>

#if((SMART_CONFIG_SOLUTION==WECHAT_AIRKISS_ON_HOST)&&(ENABLE_SMART_CONFIG==1))
#include <SmartConfig/SmartConfig.h>
#include <AirKissLib/airkiss.h>
/*
The requirement of AirKiss solution
*/
airkiss_context_t g_smtcfg;
airkiss_result_t ares;
const airkiss_config_t g_airkscfg = {
    (airkiss_memset_fn)&ssv6xxx_memset,
    (airkiss_memcpy_fn)&ssv6xxx_memcpy,
    (airkiss_memcmp_fn)&ssv6xxx_memcmp,
    (airkiss_printf_fn)NULL,
};

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
    LOG_PRINTF("\33[32mAirKiss version:%s\33[0m\n",airkiss_version());
    /*
    The default channel mask is the same with g_sta_channel_mask. The value of g_sta_channel_mask is depend on DEFAULT_STA_CHANNEL_MASK.
    You can force to change the channel mask here.
    */
    //g_SconfigChannelMask=g_sta_channel_mask;
    g_channelInterval=(100/TICK_RATE_MS); //The unit of this variable is tick, the default setting of change channel is 100ms
    g_restartInterval=(60000/TICK_RATE_MS); //The unit of this variable is tick, the default setting of restart AirKiss is 60s
    return airkiss_init(&g_smtcfg,&g_airkscfg);
}

/*
Deinit customer's SmartConfig solution.
Return:
    0: Success
    1: Fail
*/
int CusSmartConfigDeInit(void)
{
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
    airkiss_change_channel(&g_smtcfg);
    return 0;
}

/*
Feed the data to customer's SmartConfig solution
Return:
    EN_SCONFIG_LOCK: Lock the channel
    EN_SCONFIG_DONE: SmartConfig has already gotten the AP information
    EN_SCONFIG_GOING: SmartConfig is not finished
*/
EN_SCONFIG_STATUS CusSmartConfigRxData(u8 *rx_buf, u32 rx_len)
{
    s32 ret=0;
    ret=airkiss_recv(&g_smtcfg,rx_buf,rx_len);

    if(ret==AIRKISS_STATUS_CHANNEL_LOCKED){
        return EN_SCONFIG_LOCK;
    }
    else if (ret==AIRKISS_STATUS_COMPLETE){
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

    airkiss_get_result(&g_smtcfg,&ares);
    sres->ssid_len=ares.ssid_length;
    ssv6xxx_memcpy(sres->ssid,ares.ssid,ares.ssid_length);
    sres->key_len=ares.pwd_length;
    ssv6xxx_memcpy(sres->key,ares.pwd,ares.pwd_length);
    return 0;

}

/*
Response SmartPhone after IP address is ready
*/
extern int netmgr_wifi_sconfig_done(u8 *resp_data, u32 len, bool IsUDP,u32 port);
int CustSmartConfigDone(void)
{
    netmgr_wifi_sconfig_done(&ares.random,1,TRUE,10000);
    return 0;
}

#endif //#if((SMART_CONFIG_SOLUTION==WECHAT_AIRKISS_ON_HOST)&&(ENABLE_SMART_CONFIG==1))

