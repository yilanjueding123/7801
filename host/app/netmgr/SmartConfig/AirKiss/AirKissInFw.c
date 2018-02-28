/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <log.h>
#include <rtos.h>
#include <porting.h>

/*
This file is only for compile pass. When user choose the "WECHAT_AIRKISS_IN_FW", we must offer the fake functions to SmartConfig
*/
#if((SMART_CONFIG_SOLUTION==WECHAT_AIRKISS_IN_FW)&&(ENABLE_SMART_CONFIG==1))
#include <SmartConfig/SmartConfig.h>
int CusSmartConfigInit(void)
{
    LOG_PRINTF("Wrong SmartConfig solution\r\n");
    return -1;
}

/*
Deinit customer's SmartConfig solution.
Return:
    0: Success
    1: Fail
*/
int CusSmartConfigDeInit(void)
{
    //LOG_PRINTF("Wrong SmartConfig solution\r\n");
    return -1;
}

/*
Flush data after channel change
Return:
    0: Success
    1: Fail
*/
int CusSmartConfigFlushData(void)
{
    //LOG_PRINTF("Wrong SmartConfig solution\r\n");
    return -1;
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
    //LOG_PRINTF("Wrong SmartConfig solution\r\n");
    return EN_SCONFIG_NOT_READY;
}

/*
Get the AP info
Return:
    0: Success
    1: Fail
*/
int CusSmartGetResult(SMART_COFIG_RESULT *sres)
{
    //LOG_PRINTF("Wrong SmartConfig solution\r\n");
    return -1;

}

/*
Response SmartPhone after IP address is ready
*/
int CustSmartConfigDone(void)
{
    //LOG_PRINTF("Wrong SmartConfig solution\r\n");
    return -1;
}

#endif //#if((SMART_CONFIG_SOLUTION==WECHAT_AIRKISS_ON_HOST)&&(ENABLE_SMART_CONFIG==1))

