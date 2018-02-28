/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/



#ifndef _SMARTCONF_H_
#define _SMARTCONF_H_

//#include "arc4.h"
#include <ssv_lib.h>

#define U32 u32
#define U16 u16
#define U8 u8
#define memset ssv6xxx_memset
#define memcpy ssv6xxx_memcpy
#define memcmp ssv6xxx_memcmp
#define strncpy ssv6xxx_strncpy


#define IP_LEN 4
#define MAC_LEN 6
#define PHONE_PORT 8209
#define DEVICE_PORT 8302
#define SEND_MAX_BUF 64

typedef struct _smartComm {
    U8 sendToPhoneEnable;
    U8 sendToPhoneDataEnable;
    U8 phoneIP[IP_LEN];
    U8 phoneMAC[MAC_LEN];
    U8 buf[SEND_MAX_BUF];
    U8 bufLen;
} smartComm;

typedef struct t_AP_DETAIL_INFO
{
	u8 		ssid[MAX_SSID_LEN];//ssid[] didnot include '\0' at end of matrix
	u8		ssid_len;
	u8		key[64];
	u8		key_len;
	u8 		mac[6];
	char	pmk[32];
	u8		channel;
} AP_DETAIL_INFO;

typedef enum{
    SLINK_STATUS_CONTINUE=0,
    SLINK_STATUS_CHANNEL_LOCKED=1,
    SLINK_STATUS_COMPLETE=2,
}SLINK_STATUS;

typedef struct t_IEEE80211STATUS
{
	AP_DETAIL_INFO		connAP;
    U8 smart_state[2];
    #if 0 //Ian 20151225
	bool                ptk_in_checking;
    u32                 ch_cfg_size;
	struct ssv6xxx_ch_cfg *ch_cfg;
    #endif
}IEEE80211STATUS;



#define SEQ_BASE_CMD 256 //0 ~ 255 SSID&PASS_DATA_CMD, 256 ~ 512 SEQ_CMD
#define SSID_DATA 1 // 1BYTE
#define PASS_DATA 1 // 1BYTE
#define CRC8_DATA 1 // 1BYTE
#define IP_DATA 4 // 4BYTE
#define MAC_DATA 6 // 6BYTE
#define CMD_DATA_MAX_LEN 256
#define MAX_BASE_LEN_BUF 5
//#define SMARTLINK_SCAN_PERIOD 10 * CLOCK_MINI_SECOND

#define TIM_NOTE2_80211_ADDR {0x38, 0xAA, 0x3C, 0xE1, 0xDC, 0xC9}

typedef struct smart_control {
    U8 stopScan;
    U16 backData[2];//0xffff data is clear data
    U8 crcData;
    U8 decrypValue;
    //ssid & pass len
    U8 ssidLen[2];
    U8 passLen[2];
    //seq
    U8 seqData[2];
    U8 NumSeq[2];
    U8 backSeq[2];
    //cmd
    U8 cmdNum; // strat cmd number 1
    U8 CmdNumLen[2]; //every cmd have diff len
    //get current packet len
    U8 baseLenBufCount[2];
    U8 checkBaseLenBuf[2][5];
    U8 turnBaseLenBufCount[2];
    U8 turnCheckBaseLenBuf[2][5];
    //rc4 & cmd  buf
    U8 sonkey[CMD_DATA_MAX_LEN];//rc4 buf
    U8 packet[2][CMD_DATA_MAX_LEN];//cmd buf
    U8 phoneIP[2][IP_DATA];//
    U8 phoneMAC[2][MAC_DATA];//
    U8 buf[SEND_MAX_BUF];
    U8 stage1Timer;
    U8 stage2Timer;
} smart_control;

void initSmartLink();
void rx_process_smartConf(u8 *rx_data, u32 rx_len,bool fromDS);
SLINK_STATUS getSmartLinkStatus(void);
int getSmartLinkResult(AP_DETAIL_INFO *result);

#endif
