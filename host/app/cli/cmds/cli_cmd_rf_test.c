/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <os_wrapper.h>
#include <log.h>
#include <drv/ssv_drv.h>
#include <ssv_lib.h>
#include "cli_cmd_rf_test.h"

#define OP_NUM 31
static char *RADIO_OP[OP_NUM] =
{
"CHANNEL=",
"WIFI_OPMODE=",
"11N_CONFIG=",
"FIX_DATA_RATE=",
"AUTO_DATA_RATE",
"SECURITY=",
"TX_POWER=",
"AIFS=",
"APSTA=",
"SET_MAC=",
"FRAME_TYPE=",
"FRAME_SIZE=",
"FRAME_DATA=",
"SEND_FRAME=",
"SET_BSSID=",
"SET_PEERMAC=",
"SET_SSID=",
"RX_CNT_CLR=",
"RX_DISABLE=",
"START_TX=",
"SET_COUNT=",
"SET_DELAY=",
"RF_START",
"RF_RATE=",
"RF_BGAIN=",
"RF_GNGAIN=",
"RF_IQPHASE=",
"RF_IQAMP=",
"RF_STOP",
"RF_RESET",
"RF_COUNT="
};

typedef struct t_RF_TEST_CONFIGURATION
{
    u32 regChannel;
    u32 regRate;
    u32 regGain;
    u32 regIQ;
} RF_TEST_CONFIGURATION;

RF_TEST_CONFIGURATION rf_test_config = {1,0x07010400,0x6C726C72,0x00000000};
int _DBG_AT_SET_CHANNEL(u16 channel)
{
    u32 _channel=(u32)channel;
    LOG_PRINTF("CHANNEL = %d\r\n", channel);
	return  ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_CAL, &_channel, sizeof(u32));
}

//#define HARDCODE_TEST
void RFStart()
{
    LOG_PRINTF("RFStart\r\n");
    //rf script
    //REG32(0xCE010000) = 0x40002000;
    ssv6xxx_drv_write_reg(0xCE010000,0x40002000);

    //REG32(0xCE010004) = 0x00004FC0;
    ssv6xxx_drv_write_reg(0xCE010004,0x00004FC0);

    //REG32(0xCE010008) = 0x008B7C1C;
    ssv6xxx_drv_write_reg(0xCE010008,0x008B7C1C);

    //REG32(0xCE01000C) = 0x151558C4;
    ssv6xxx_drv_write_reg(0xCE01000C,0x151558C4);

    //REG32(0xCE010010) = 0x01011A88;
    ssv6xxx_drv_write_reg(0xCE010010,0x01011A88);

    //REG32(0xCE010014) = 0x3D7E84FE;
    ssv6xxx_drv_write_reg(0xCE010014,0x3D7E84FE);

    //REG32(0xCE010018) = 0x01457D79;
    ssv6xxx_drv_write_reg(0xCE010018,0x01457D79);

    //REG32(0xCE01001C) = 0x000103EB;
    ssv6xxx_drv_write_reg(0xCE01001C,0x000103EB);

    //REG32(0xCE010020) = 0x000103EA;
    ssv6xxx_drv_write_reg(0xCE010020,0x000103EA);

    //REG32(0xCE010024) = 0x00012001;
    ssv6xxx_drv_write_reg(0xCE010024,0x00012001);

    //REG32(0xCE010028) = 0x00036000;
    ssv6xxx_drv_write_reg(0xCE010028,0x00036000);

    //REG32(0xCE01002C) = 0x00030CA8;
    ssv6xxx_drv_write_reg(0xCE01002C,0x00030CA8);

    //REG32(0xCE010030) = 0x20EA0224;
    ssv6xxx_drv_write_reg(0xCE010030,0x20EA0224);

    //REG32(0xCE010034) = 0x3C800755;
    ssv6xxx_drv_write_reg(0xCE010034,0x3C800755);

    //REG32(0xCE010038) = 0x0003E07C;
    ssv6xxx_drv_write_reg(0xCE010038,0x0003E07C);

    //REG32(0xCE01003C) = 0x55276276;
    ssv6xxx_drv_write_reg(0xCE01003C,0x55276276);

    //REG32(0xCE010040) = 0x005508BE;
    ssv6xxx_drv_write_reg(0xCE010040,0x005508BE);

    //REG32(0xCE010044) = 0x07C08BFF;
    ssv6xxx_drv_write_reg(0xCE010044,0x07C08BFF);

    //REG32(0xCE010048) = 0xFCCCCC27;
    ssv6xxx_drv_write_reg(0xCE010048,0xFCCCCC27);

    //REG32(0xCE01004C) = 0x07700830;
    ssv6xxx_drv_write_reg(0xCE01004C,0x07700830);

    //REG32(0xCE010050) = 0x0047C000;
    ssv6xxx_drv_write_reg(0xCE010050,0x0047C000);

    //REG32(0xCE010054) = 0x00007FF4;
    ssv6xxx_drv_write_reg(0xCE010054,0x00007FF4);

    //REG32(0xCE010058) = 0x0000000E;
    ssv6xxx_drv_write_reg(0xCE010058,0x0000000E);

    //REG32(0xCE01005C) = 0x00088018;
    ssv6xxx_drv_write_reg(0xCE01005C,0x00088018);

    //REG32(0xCE010060) = 0x00406000;
    ssv6xxx_drv_write_reg(0xCE010060,0x00406000);

    //REG32(0xCE010064) = 0x08820820;
    ssv6xxx_drv_write_reg(0xCE010064,0x08820820);

    //REG32(0xCE010068) = 0x00820820;
    ssv6xxx_drv_write_reg(0xCE010068,0x00820820);

    //REG32(0xCE01006C) = 0x00820820;
    ssv6xxx_drv_write_reg(0xCE01006C,0x00820820);

    //REG32(0xCE010070) = 0x00820820;
    ssv6xxx_drv_write_reg(0xCE010070,0x00820820);

    //REG32(0xCE010074) = 0x00820820;
    ssv6xxx_drv_write_reg(0xCE010074,0x00820820);

    //REG32(0xCE010078) = 0x00820820;
    ssv6xxx_drv_write_reg(0xCE010078,0x00820820);

    //REG32(0xCE01007C) = 0x00820820;
    ssv6xxx_drv_write_reg(0xCE01007C,0x00820820);

    //REG32(0xCE010080) = 0x00820820;
    ssv6xxx_drv_write_reg(0xCE010080,0x00820820);

    //REG32(0xCE010084) = 0x00004080;
    ssv6xxx_drv_write_reg(0xCE010084,0x00004080);

    //REG32(0xCE010088) = 0x200800FE;
    ssv6xxx_drv_write_reg(0xCE010088,0x200800FE);

    //REG32(0xCE01008C) = 0xAAAAAAAA;
    ssv6xxx_drv_write_reg(0xCE01008C,0xAAAAAAAA);

    //REG32(0xCE010090) = 0xAAAAAAAA;
    ssv6xxx_drv_write_reg(0xCE010090,0xAAAAAAAA);

    //REG32(0xCE010094) = 0x0000A407;
    ssv6xxx_drv_write_reg(0xCE010094,0x0000A407);

    //REG32(0xCE010098) = 0x00000150;
    ssv6xxx_drv_write_reg(0xCE010098,0x00000150);

    //REG32(0xCE01009C) = 0x00000024;
    ssv6xxx_drv_write_reg(0xCE01009C,0x00000024);

    //REG32(0xCE0100A0) = 0x00EC4CC5;
    ssv6xxx_drv_write_reg(0xCE0100A0,0x00EC4CC5);

    //REG32(0xCE0100A4) = 0x00000F73;
    ssv6xxx_drv_write_reg(0xCE0100A4,0x00000F73);

    //REG32(0xCE0100A8) = 0x00098900;
    ssv6xxx_drv_write_reg(0xCE0100A8,0x00098900);

    //REG32(0xCE0100AC) = 0x00000000;
    ssv6xxx_drv_write_reg(0xCE0100AC,0x00000000);


    //gpio CLOSE FOR iqpad
    //REG32(0xC00003AC) = 0x00000000;
    ssv6xxx_drv_write_reg(0xC00003AC,0x00000000);

    //REG32(0xC00003B0) = 0x00000000;
    ssv6xxx_drv_write_reg(0xC00003B0,0x00000000);

    //REG32(0xC00003B4) = 0x00000000;
    ssv6xxx_drv_write_reg(0xC00003B4,0x00000000);

    //REG32(0xC00003BC) = 0x00000000;
    ssv6xxx_drv_write_reg(0xC00003BC,0x00000000);


    //urn off phy
    //REG32(0xCE000004) = 0x00000178;
    ssv6xxx_drv_write_reg(0xCE000004,0x00000178);

    //REG32(0xCE000000) = 0x80000016; // [4] sign_swap, [5] iq_swap, [2:1] rdyack_sel, [3] adc_edge
    ssv6xxx_drv_write_reg(0xCE000000,0x80000016);

#ifdef HARDCODE_TEST
    //Rate
    //REG32(0xCE00000C) = 0x07010400;
    ssv6xxx_drv_write_reg(0xCE00000C,0x07010400);

    //REG32(0xCE000004) = 0x0000017F;
    ssv6xxx_drv_write_reg(0xCE000004,0x0000017F);

    //manual Cali IQphase and IQAMP
    //REG32(0xCE00704C) = 0x00001F1B;
    ssv6xxx_drv_write_reg(0xCE00704C,0x00001F1B);

    //manual tx GAIN TABLE
    //REG32(0xCE0071BC) = 0x79806C72;
    ssv6xxx_drv_write_reg(0xCE0071BC,0x79806C72);

#else
    //Rate
    //REG32(0xCE00000C) = rf_test_config.regRate;
    ssv6xxx_drv_write_reg(0xCE00000C,rf_test_config.regRate);

    //REG32(0xCE000004) = 0x0000017F;
    ssv6xxx_drv_write_reg(0xCE000004,0x0000017F);

    //manual Cali IQphase and IQAMP
    //REG32(0xCE00704C) = rf_test_config.regIQ;
    ssv6xxx_drv_write_reg(0xCE00704C,rf_test_config.regIQ);

    //manual tx GAIN TABLE
    //REG32(0xCE0071BC) = rf_test_config.regGain;
    ssv6xxx_drv_write_reg(0xCE0071BC,rf_test_config.regGain);

#endif
    // start tx
    //REG32(0xC0000304) = 0x00000001;
    ssv6xxx_drv_write_reg(0xC0000304,0x00000001);

    //REG32(0xC0000308) = 0x00000001;
    ssv6xxx_drv_write_reg(0xC0000308,0x00000001);

    //REG32(0xC000030C) = 0x00000001;
    ssv6xxx_drv_write_reg(0xC000030C,0x00000001);

    //REG32(0xCA000400) = 0x00000400;
    ssv6xxx_drv_write_reg(0xCA000400,0x00000400);

    //REG32(0xCA000408) = 0x000A0007;
    ssv6xxx_drv_write_reg(0xCA000408,0x000A0007);

    //REG32(0xC0001D08) = 0x00000000; //REG32(0xC0001D08) = 0x00000001;
    ssv6xxx_drv_write_reg(0xC0001D08,0x00000000);

    //REG32(0xCE000020) = 0x20000400;
    ssv6xxx_drv_write_reg(0xCE000020,0x20000400);

    //REG32(0xCE000004) = 0x0000017F;
    ssv6xxx_drv_write_reg(0xCE000004,0x0000017F);

    //REG32(0xCE00007C) = 0x10110003;
    ssv6xxx_drv_write_reg(0xCE00007C,0x10110003);

    //REG32(0xCE000094) = 0x01012425; // RF ramp on   2013/1212
    ssv6xxx_drv_write_reg(0xCE000094,0x01012425);

    //REG32(0xCE000098) = 0x01010101; // RF ramp down   2013/1212
    ssv6xxx_drv_write_reg(0xCE000098,0x01010101);

    //REG32(0xCE0010B4) = 0x00003001; // 11b baseband ramp on/off  2013/1212
    ssv6xxx_drv_write_reg(0xCE0010B4,0x00003001);

    //REG32(0xCE0030A4) = 0x00001901; // 11gn baseband ramp on/off  2013/1212
    ssv6xxx_drv_write_reg(0xCE0030A4,0x00001901);

    //REG32(0xCE000018) = 0x0055083D;
    ssv6xxx_drv_write_reg(0xCE000018,0x0055083D);

    //REG32(0xCE00001C) = 0xFFFFFFFF;
    ssv6xxx_drv_write_reg(0xCE00001C,0xFFFFFFFF);


    _DBG_AT_SET_CHANNEL(rf_test_config.regChannel);
}
void RFRate(u16 rate_index)
{
    LOG_PRINTF("RFRate index = %d\r\n", rate_index);

    switch(rate_index)
    {
        case 0:
            rf_test_config.regRate = 0x00000400; // 11b 1M
            break;
        case 1:
            rf_test_config.regRate = 0x01000400; // 11b 2M
            break;
        case 2:
            rf_test_config.regRate = 0x02000400; // 11b 5.5M
            break;
        case 3:
            rf_test_config.regRate = 0x03000400; // 11b 11M
            break;
        //11b short-preamble
        case 4:
            rf_test_config.regRate = 0x01400400; // 11b 2M SP
            break;
        case 5:
            rf_test_config.regRate = 0x02400400; // 11b 5.5M SP
            break;
        case 6:
            rf_test_config.regRate = 0x03400400; // 11b 11M SP
            break;
        //11gn non-ht
        case 7:
            rf_test_config.regRate = 0x00010400; // NON-HT 6M
            break;
        case 8:
            rf_test_config.regRate = 0x01010400; // NON-HT 9M
            break;
        case 9:
            rf_test_config.regRate = 0x02010400; // NON-HT 12M
            break;
        case 10:
            rf_test_config.regRate = 0x03010400; // NON-HT 18M
            break;
        case 11:
            rf_test_config.regRate = 0x04010400; // NON-HT 24M
            break;
        case 12:
            rf_test_config.regRate = 0x05010400; // NON-HT 36M
            break;
        case 13:
            rf_test_config.regRate = 0x06010400; // NON-HT 48M
            break;
        case 14:
            rf_test_config.regRate = 0x07010400; // NON-HT 54M
            break;
        //11gn ht-mm
        case 15:
            rf_test_config.regRate = 0x00021000; // HT-MM MCS0
            break;
        case 16:
            rf_test_config.regRate = 0x01021000; // HT-MM MCS1
            break;
        case 17:
            rf_test_config.regRate = 0x02021000; // HT-MM MCS2
            break;
        case 18:
            rf_test_config.regRate = 0x03021000; // HT-MM MCS3
            break;
        case 19:
            rf_test_config.regRate = 0x04021000; // HT-MM MCS4
            break;
        case 20:
            rf_test_config.regRate = 0x05021000; // HT-MM MCS5
            break;
        case 21:
            rf_test_config.regRate = 0x06021000; // HT-MM MCS6
            break;
        case 22:
            rf_test_config.regRate = 0x07021000; // HT-MM MCS7
            break;
        //11gn ht-gf
        case 23:
            rf_test_config.regRate = 0x00421000; // HT-GF MCS0
            break;
        case 24:
            rf_test_config.regRate = 0x01421000; // HT-GF MCS1
            break;
        case 25:
            rf_test_config.regRate = 0x02421000; // HT-GF MCS2
            break;
        case 26:
            rf_test_config.regRate = 0x03421000; // HT-GF MCS3
            break;
        case 27:
            rf_test_config.regRate = 0x04421000; // HT-GF MCS4
            break;
        case 28:
            rf_test_config.regRate = 0x05421000; // HT-GF MCS5
            break;
        case 29:
            rf_test_config.regRate = 0x06421000; // HT-GF MCS6
            break;
        case 30:
            rf_test_config.regRate = 0x07421000; // HT-GF MCS7
            break;
        //11gn ht-mm sgi
        case 31:
            rf_test_config.regRate = 0x00821000; // HT-MM SGI MCS0
            break;
        case 32:
            rf_test_config.regRate = 0x01821000; // HT-MM SGI MCS1
            break;
        case 33:
            rf_test_config.regRate = 0x02821000; // HT-MM SGI MCS2
            break;
        case 34:
            rf_test_config.regRate = 0x03821000; // HT-MM SGI MCS3
            break;
        case 35:
            rf_test_config.regRate = 0x04821000; // HT-MM SGI MCS4
            break;
        case 36:
            rf_test_config.regRate = 0x05821000; // HT-MM SGI MCS5
            break;
        case 37:
            rf_test_config.regRate = 0x06821000; // HT-MM SGI MCS6
            break;
        case 38:
            rf_test_config.regRate = 0x07821000; // HT-MM SGI MCS7
            break;
        default:
            rf_test_config.regRate = 0x07010400; // NON-HT 54M

    }
    //REG32(0xCE00000C) = rf_test_config.regRate;
    ssv6xxx_drv_write_reg(0xCE00000C,rf_test_config.regRate);
    //REG32(0xCE000004) = 0x0000017F;
    ssv6xxx_drv_write_reg(0xCE000004,0x0000017F);
}

void RFBGain(s32 gainBMode)
{
    LOG_PRINTF("RFBGain = %d\r\n", gainBMode);
    if(gainBMode <3 && gainBMode > -4)
    {
        if(gainBMode == 2)
            rf_test_config.regGain = (rf_test_config.regGain & 0xFFFF0000) | 0x98A1;
        else if(gainBMode == 1)
            rf_test_config.regGain = (rf_test_config.regGain & 0xFFFF0000) | 0x8890;
        else if(gainBMode == 0)
            rf_test_config.regGain = (rf_test_config.regGain & 0xFFFF0000) | 0x7980;
        else if(gainBMode == -1)
            rf_test_config.regGain = (rf_test_config.regGain & 0xFFFF0000) | 0x6C72;
        else if(gainBMode == -2)
            rf_test_config.regGain = (rf_test_config.regGain & 0xFFFF0000) | 0x6066;
        else if(gainBMode == -3)
            rf_test_config.regGain = (rf_test_config.regGain & 0xFFFF0000) | 0x565B;
       else
            rf_test_config.regGain = (rf_test_config.regGain & 0xFFFF0000) | 0x6C72;

       //printf("gain value = 0x%x\n", rf_test_config.regGain);
    }
    else
        LOG_PRINTF("gain out of range!\n");
    //REG32(0xCE0071BC) = rf_test_config.regGain;
    ssv6xxx_drv_write_reg(0xCE0071BC,rf_test_config.regGain);
}

void RFGNGain(s32 gainGNMode)
{
    LOG_PRINTF("RFGNGain = %d\r\n", gainGNMode);

    if(gainGNMode <3 && gainGNMode > -4)
    {
        if(gainGNMode == 2)
            rf_test_config.regGain = (rf_test_config.regGain & 0x0000FFFF) | (0x98A1 << 16);
        else if(gainGNMode == 1)
            rf_test_config.regGain = (rf_test_config.regGain & 0x0000FFFF) | (0x8890 << 16);
        else if(gainGNMode == 0)
            rf_test_config.regGain = (rf_test_config.regGain & 0x0000FFFF) | (0x7980 << 16);
        else if(gainGNMode == -1)
            rf_test_config.regGain = (rf_test_config.regGain & 0x0000FFFF) | (0x6C72 << 16);
        else if(gainGNMode == -2)
            rf_test_config.regGain = (rf_test_config.regGain & 0x0000FFFF) | (0x6066 << 16);
        else if(gainGNMode == -3)
            rf_test_config.regGain = (rf_test_config.regGain & 0x0000FFFF) | (0x565B << 16);
       else
            rf_test_config.regGain = (rf_test_config.regGain & 0x0000FFFF) | (0x6C72 << 16);

       //printf("gain value = 0x%x\n", rf_test_config.regGain);
    }
    else
        LOG_PRINTF("gain out of range!\n");

    //REG32(0xCE0071BC) = rf_test_config.regGain;
    ssv6xxx_drv_write_reg(0xCE0071BC,rf_test_config.regGain);
}

void RFIQPhase(s8 value)
{
    s8 nOffset = 0x20;
    LOG_PRINTF("RFIQPhase = %d\r\n", value);

    if(value<0)
    nOffset = nOffset + value;
    else
       nOffset = value;
    rf_test_config.regIQ = (rf_test_config.regIQ & 0x0000FF00) | nOffset;
    //printf("regIQ = 0x%x\n", rf_test_config.regIQ);
    //REG32(0xCE00704C) = rf_test_config.regIQ;
    ssv6xxx_drv_write_reg(0xCE00704C,rf_test_config.regIQ);
}

void RFIQAmp(s8 value)
{
    s8 nOffset = 0x20;
    LOG_PRINTF("RFIQAmp = %d\r\n", value);
    if(value<0)
    nOffset = nOffset + value;
    else
       nOffset = value;
    rf_test_config.regIQ = (rf_test_config.regIQ & 0x000000FF) |  (nOffset << 8 );
    //printf("regIQ = 0x%x\n", rf_test_config.regIQ);
    //REG32(0xCE00704C) = rf_test_config.regIQ;
    ssv6xxx_drv_write_reg(0xCE00704C,rf_test_config.regIQ);
}
void RFStop()
{
    LOG_PRINTF("RFStop\r\n");
    //REG32(0xCE000018) = 0x005A0220;
    ssv6xxx_drv_write_reg(0xCE000018,0x005A0220);

    //REG32(0xCE00001C) = 0x00000001;
    ssv6xxx_drv_write_reg(0xCE00001C,0x00000001);
}

void RFReset()
{
    LOG_PRINTF("RFReset\r\n");

    ///////rf script
    //REG32(0xCE010000) = 0x40002000;
    ssv6xxx_drv_write_reg(0xCE010000,0x40002000);

    //REG32(0xCE010004) = 0x00004FC0; //
    ssv6xxx_drv_write_reg(0xCE010004,0x00004FC0);

    //REG32(0xCE010008) = 0x008B7C1C;
    ssv6xxx_drv_write_reg(0xCE010008,0x008B7C1C);

    //REG32(0xCE01000C) = 0x151558C4; //
    ssv6xxx_drv_write_reg(0xCE01000C,0x151558C4);

    //REG32(0xCE010010) = 0x01011A88;
    ssv6xxx_drv_write_reg(0xCE010010,0x01011A88);

    //REG32(0xCE010014) = 0x3D7E84FE;
    ssv6xxx_drv_write_reg(0xCE010014,0x3D7E84FE);

    //REG32(0xCE010018) = 0x01457D79;
    ssv6xxx_drv_write_reg(0xCE010018,0x01457D79);

    //REG32(0xCE01001C) = 0x000103EB;
    ssv6xxx_drv_write_reg(0xCE01001C,0x000103EB);

    //REG32(0xCE010020) = 0x000103EA;
    ssv6xxx_drv_write_reg(0xCE010020,0x000103EA);

    //REG32(0xCE010024) = 0x00012001;
    ssv6xxx_drv_write_reg(0xCE010024,0x00012001);

    //REG32(0xCE010028) = 0x00036000;
    ssv6xxx_drv_write_reg(0xCE010028,0x00036000);

    //REG32(0xCE01002C) = 0x00030CA8;
    ssv6xxx_drv_write_reg(0xCE01002C,0x00030CA8);

    //REG32(0xCE010030) = 0x20EA0224;
    ssv6xxx_drv_write_reg(0xCE010030,0x20EA0224);

    //REG32(0xCE010034) = 0x3C800755;
    ssv6xxx_drv_write_reg(0xCE010034,0x3C800755);

    //REG32(0xCE010038) = 0x0003E07C;
    ssv6xxx_drv_write_reg(0xCE010038,0x0003E07C);

    //REG32(0xCE01003C) = 0x55276276; //  ivan:b  crc ¶Ã¸õ
    ssv6xxx_drv_write_reg(0xCE01003C,0x55276276);

    //REG32(0xCE010040) = 0x005508BE; //  ivan: crc and count always 0
    ssv6xxx_drv_write_reg(0xCE010040,0x005508BE);

    //REG32(0xCE010044) = 0x07C08BFF;
    ssv6xxx_drv_write_reg(0xCE010044,0x07C08BFF);

    //REG32(0xCE010048) = 0xFCCCCC27;
    ssv6xxx_drv_write_reg(0xCE010048,0xFCCCCC27);

    //REG32(0xCE01004C) = 0x07700830;
    ssv6xxx_drv_write_reg(0xCE01004C,0x07700830);

    //REG32(0xCE010050) = 0x0047C000;
    ssv6xxx_drv_write_reg(0xCE010050,0x0047C000);

    //REG32(0xCE010054) = 0x00007FF4;
    ssv6xxx_drv_write_reg(0xCE010054,0x00007FF4);

    //REG32(0xCE010058) = 0x0000000E;
    ssv6xxx_drv_write_reg(0xCE010058,0x0000000E);

    //REG32(0xCE01005C) = 0x00088018;
    ssv6xxx_drv_write_reg(0xCE01005C,0x00088018);

    //REG32(0xCE010060) = 0x00406000;
    ssv6xxx_drv_write_reg(0xCE010060,0x00406000);

    //REG32(0xCE010064) = 0x08820820;
    ssv6xxx_drv_write_reg(0xCE010064,0x08820820);

    //REG32(0xCE010068) = 0x00820820;
    ssv6xxx_drv_write_reg(0xCE010068,0x00820820);

    //REG32(0xCE01006C) = 0x00820820;
    ssv6xxx_drv_write_reg(0xCE01006C,0x00820820);

    //REG32(0xCE010070) = 0x00820820;
    ssv6xxx_drv_write_reg(0xCE010070,0x00820820);

    //REG32(0xCE010074) = 0x00820820;
    ssv6xxx_drv_write_reg(0xCE010074,0x00820820);

    //REG32(0xCE010078) = 0x00820820;
    ssv6xxx_drv_write_reg(0xCE010078,0x00820820);

    //REG32(0xCE01007C) = 0x00820820;
    ssv6xxx_drv_write_reg(0xCE01007C,0x00820820);

    //REG32(0xCE010080) = 0x00820820;
    ssv6xxx_drv_write_reg(0xCE010080,0x00820820);

    //REG32(0xCE010084) = 0x00004080;
    ssv6xxx_drv_write_reg(0xCE010084,0x00004080);

    //REG32(0xCE010088) = 0x200800FE;
    ssv6xxx_drv_write_reg(0xCE010088,0x200800FE);

    //REG32(0xCE01008C) = 0xAAAAAAAA;
    ssv6xxx_drv_write_reg(0xCE01008C,0xAAAAAAAA);

    //REG32(0xCE010090) = 0xAAAAAAAA;
    ssv6xxx_drv_write_reg(0xCE010090,0xAAAAAAAA);

    //REG32(0xCE010094) = 0x0000A407; //
    ssv6xxx_drv_write_reg(0xCE010094,0x0000A407);

    //REG32(0xCE010098) = 0x000003F0; //
    ssv6xxx_drv_write_reg(0xCE010098,0x000003F0);

    //REG32(0xCE01009C) = 0x40452024;
    ssv6xxx_drv_write_reg(0xCE01009C,0x40452024);

    //REG32(0xCE0100A0) = 0x00EC4EC5;
    ssv6xxx_drv_write_reg(0xCE0100A0,0x00EC4EC5);

    //REG32(0xCE0100A4) = 0x00000F23; //
    ssv6xxx_drv_write_reg(0xCE0100A4,0x00000F23);

    //REG32(0xCE0100A8) = 0x00098900;
    ssv6xxx_drv_write_reg(0xCE0100A8,0x00098900);

    //REG32(0xCE0100AC) = 0x00000000;
    ssv6xxx_drv_write_reg(0xCE0100AC,0x00000000);

    ////////////

    //gpio CLOSE FOR iqpad
    //REG32(0xC00003AC) = 0x00000000;
    ssv6xxx_drv_write_reg(0xC00003AC,0x00000000);

    //REG32(0xC00003B0) = 0x00000000;
    ssv6xxx_drv_write_reg(0xC00003B0,0x00000000);

    //REG32(0xC00003B4) = 0x00000000;
    ssv6xxx_drv_write_reg(0xC00003B4,0x00000000);

    //REG32(0xC00003BC) = 0x00000000;
    ssv6xxx_drv_write_reg(0xC00003BC,0x00000000);

    /////channel cal
    //REG32(0xCE01003C) = 0x554EC4EC; //r15
    //REG32(0xCE010040) = 0x005508BA; //r16
    //REG32(0xCE0100A4) = 0x00000F23;  //r41  channel 3
    //REG32(0xCE010004) = 0x00020FC0; //cal
    //REG32(0xCE010004) = 0x00024FC0;
    //REG32(0xCE010004) = 0x00020FC0; //cal
    //REG32(0xCE010004) = 0x00024FC0;

    /////// start rx
    //REG32(0xC0000304) = 0x00000001;
    ssv6xxx_drv_write_reg(0xC0000304,0x00000001);

    //REG32(0xC0000308) = 0x00000001;
    ssv6xxx_drv_write_reg(0xC0000308,0x00000001);

    //REG32(0xC000030C) = 0x00000001;
    ssv6xxx_drv_write_reg(0xC000030C,0x00000001);

    //REG32(0xCA000400) = 0x00000400;
    ssv6xxx_drv_write_reg(0xCA000400,0x00000400);

    //REG32(0xCA000408) = 0x000A0007;
    ssv6xxx_drv_write_reg(0xCA000408,0x000A0007);

    //ldo=0 -dcdc=1
    //REG32(0xC0001D08) = 0x00000000;  //REG32(0xC0001D08) = 0x00000001;


    ////// CabrioE PHY Scirpt
    //REG32(0xCE000004) = 0x00000178;
    ssv6xxx_drv_write_reg(0xCE000004,0x00000178);

    //REG32(0xCE000000) = 0x80000016;
    ssv6xxx_drv_write_reg(0xCE000000,0x80000016);

    //REG32(0xCE000010) = 0x00007FFF;
    ssv6xxx_drv_write_reg(0xCE000010,0x00007FFF);

    //REG32(0xCE000018) = 0x0055003C;  // bit 11 ¤w®³±¼2013/12/27
    ssv6xxx_drv_write_reg(0xCE000018,0x0055003C);

    //REG32(0xCE00001C) = 0x00000064;
    ssv6xxx_drv_write_reg(0xCE00001C,0x00000064);

    //REG32(0xCE000020) = 0x20000400;
    ssv6xxx_drv_write_reg(0xCE000020,0x20000400);

    //REG32(0xCE000020) = 0x20000000;  // for FPGA system test
    //REG32(0xCE000030) = 0x80046072;  // tune AGC
    ssv6xxx_drv_write_reg(0xCE000030,0x80046072);

    //REG32(0xCE000038) = 0x660F36D0;
    ssv6xxx_drv_write_reg(0xCE000038,0x660F36D0);

    //REG32(0xCE00003C) = 0x106c0004;
    ssv6xxx_drv_write_reg(0xCE00003C,0x106c0004);

    //REG32(0xCE000040) = 0x01600500;
    ssv6xxx_drv_write_reg(0xCE000040,0x01600500);

    //REG32(0xCE000048) = 0xFF000160;
    //REG32(0xCE00004C) = 0x00200840;
    //REG32(0xCE00007C) = 0x10110003; // reduce TX descriptor fetch time from 5.6us to 1.4us 2013/1212
    ssv6xxx_drv_write_reg(0xCE00007C,0x10110003);

    //REG32(0xCE000080) = 0x0110000F; // RSSI Offset
    ssv6xxx_drv_write_reg(0xCE000080,0x0110000F);

    //REG32(0xCE000094) = 0x01012425; // RF ramp on   2013/1212
    ssv6xxx_drv_write_reg(0xCE000094,0x01012425);

    //REG32(0xCE000098) = 0x01010101; // RF ramp down   2013/1212
    ssv6xxx_drv_write_reg(0xCE000098,0x01010101);

    //REG32(0xCE0010B4) = 0x00003001; // 11b baseband ramp on/off  2013/1212
    ssv6xxx_drv_write_reg(0xCE0010B4,0x00003001);

    //REG32(0xCE0030A4) = 0x00001901; // 11gn baseband ramp on/off  2013/1212
    ssv6xxx_drv_write_reg(0xCE0030A4,0x00001901);

    //REG32(0xCE004004) = 0x00750075;
    ssv6xxx_drv_write_reg(0xCE004004,0x00750075);

    //REG32(0xCE004008) = 0x00000075;
    ssv6xxx_drv_write_reg(0xCE004008,0x00000075);

    //REG32(0xCE00400c) = 0x10000075;
    ssv6xxx_drv_write_reg(0xCE00400c,0x10000075);

    //REG32(0xCE004010) = 0x3F404905; // CCA bit extent
    ssv6xxx_drv_write_reg(0xCE004010,0x3F404905);

    //REG32(0xCE004014) = 0x40182000; // modify auto 32 threshold for better symbol boundary
    ssv6xxx_drv_write_reg(0xCE004014,0x40182000);

    //REG32(0xCE004018) = 0x28680000;
    ssv6xxx_drv_write_reg(0xCE004018,0x28680000);

    //REG32(0xCE00401C) = 0x0c010120;
    ssv6xxx_drv_write_reg(0xCE00401C,0x0c010120);

    //REG32(0xCE004020) = 0x50505050;
    ssv6xxx_drv_write_reg(0xCE004020,0x50505050);

    //REG32(0xCE004024) = 0x50000000;
    ssv6xxx_drv_write_reg(0xCE004024,0x50000000);

    //REG32(0xCE004028) = 0x50505050;
    ssv6xxx_drv_write_reg(0xCE004028,0x50505050);

    //REG32(0xCE00402c) = 0x506070A0;
    ssv6xxx_drv_write_reg(0xCE00402c,0x506070A0);

    //REG32(0xCE004030) = 0xF0000000;
    ssv6xxx_drv_write_reg(0xCE004030,0xF0000000);

    //REG32(0xCE004034) = 0x00002424;
    ssv6xxx_drv_write_reg(0xCE004034,0x00002424);

    //REG32(0xCE00409c) = 0x0000300A;
    ssv6xxx_drv_write_reg(0xCE00409c,0x0000300A);

    //REG32(0xCE0040C0) = 0x40000280;
    ssv6xxx_drv_write_reg(0xCE0040C0,0x40000280);

    //REG32(0xCE0040C4) = 0x30023002;
    ssv6xxx_drv_write_reg(0xCE0040C4,0x30023002);

    //REG32(0xCE0040C8) = 0x0000003A; // cca re-check, auto correlation off
    //REG32(0xCE004130) = 0x40000000;
    ssv6xxx_drv_write_reg(0xCE004130,0x40000000);

    //REG32(0xCE004164) = 0x009C007E;
    ssv6xxx_drv_write_reg(0xCE004164,0x009C007E);

    //REG32(0xCE004180) = 0x00044400;   // bit 0~1 ¤w®³±¼2013/12/27
    //REG32(0xCE004188) = 0x82050000; // add new channel smoothing for RX TEST
    ssv6xxx_drv_write_reg(0xCE004188,0x82050000);

    //REG32(0xCE004188) = 0x82000000; // remove channel smoothing for throughtput test
    //REG32(0xCE004190) = 0x00001820; // modify pkt format detection threshold
    ssv6xxx_drv_write_reg(0xCE004190,0x00001820);

    //REG32(0xCE0043FC) = 0x00010421; //RX TEST
    ssv6xxx_drv_write_reg(0xCE0043FC,0x00010421);

    //REG32(0xCE0043FC) = 0x000004E1; //throughput TEST
    // 11b Rx
    //REG32(0xCE002008) = 0x20400040; // b_CCA with index check
    ssv6xxx_drv_write_reg(0xCE002008,0x20400040);

    //REG32(0xCE002014) = 0x20304015;
    ssv6xxx_drv_write_reg(0xCE002014,0x20304015);

    //REG32(0xCE00201C) = 0x02333567;
    ssv6xxx_drv_write_reg(0xCE00201C,0x02333567);

    //REG32(0xCE002030) = 0x04061787;
    ssv6xxx_drv_write_reg(0xCE002030,0x04061787);

    //REG32(0xCE00209c) = 0x00c0000A;
    ssv6xxx_drv_write_reg(0xCE00209c,0x00c0000A);

    //REG32(0xCE0020A0) = 0x00000000;
    ssv6xxx_drv_write_reg(0xCE0020A0,0x00000000);

    //REG32(0xCE0023FC) = 0x00000001;
    ssv6xxx_drv_write_reg(0xCE0023FC,0x00000001);

    //REG32(0xCE000004) = 0x0000017F;
    ssv6xxx_drv_write_reg(0xCE000004,0x0000017F);

    //REG32(0xCE000020) = 0x20000000;
    ssv6xxx_drv_write_reg(0xCE000020,0x20000000);


    /////RX Reset
    //REG32(0xCE0023F8) = 0x00000000;
    ssv6xxx_drv_write_reg(0xCE0023F8,0x00000000);

    //REG32(0xCE0043F8) = 0x00000000;
    ssv6xxx_drv_write_reg(0xCE0043F8,0x00000000);

    //REG32(0xCE0023F8) = 0x00100000;
    ssv6xxx_drv_write_reg(0xCE0023F8,0x00100000);

    //REG32(0xCE0043F8) = 0x00100001;
    ssv6xxx_drv_write_reg(0xCE0043F8,0x00100001);

    /////////
    _DBG_AT_SET_CHANNEL(rf_test_config.regChannel);
    //end
}
void RFCount(u16 mode)
{
    u32 nCount = 0;
    u32 nCRC = 0;
    LOG_PRINTF("RFCount mode = %d\r\n", mode);
    if(mode == 1)
    {
        nCount = ssv6xxx_drv_read_reg(0xce0023ec) & 0xffff;
        nCRC = ssv6xxx_drv_read_reg(0xce0023e8) & 0xffff;
    }
    else
    {
        nCount = ssv6xxx_drv_read_reg(0xce0043ec) & 0xffff;
        nCRC = ssv6xxx_drv_read_reg(0xce0043e8) & 0xffff;
    }
    LOG_PRINTF("count = '%d'\n", nCount);
    LOG_PRINTF("crc = '%d'\n", nCRC);

}
int Radio_Command(TAG_RADIOOP op, char *buff)
{
  int ret = 0;
  //int i = 0;
  //char nbr = 0;
  //char mac_t[MAC_ADDR_LEN];
  //struct tx_pktbuf txbuf;
  //int pktlen;

  //ssv6xxx_memset(mac_t, 0, sizeof(mac_t));

  switch (op) {
#if 0
    case FIXDATARATE:
     _DBG_AT_SET_DATA_RATE(atoi(buff));
      break;
    case AIFS:
      _DBG_AT_SET_AIFS(atoi(buff));
      break;
    case SETMAC:
      mac_transfer(buff, mac_t);
      _DBG_AT_SET_MAC((U8 *)mac_t);
      memcpy(test_data+10, mac_t, sizeof(mac_t));
      break;
    case FRAMETYPE:
      //_DBG_AT_SET_FRAME_TYPE((u32)buff);
      break;
    case FRAMESIZE:
      _DBG_AT_SET_FRAME_SIZE(atoi(buff));
      break;
    case FRAMEDATA:
      _DBG_AT_SET_FRAME_DATA(buff[0]);
      break;
    case BSSID:
      mac_transfer(buff, mac_t);
      _DBG_AT_SET_BSSID((U8 *)mac_t);
      memcpy(test_data+16, mac_t, sizeof(mac_t));
      break;
    case PEERMAC:
      mac_transfer(buff, mac_t);
      _DBG_AT_SET_PEERMAC((U8 *)mac_t, 0);
      memcpy(test_data+4, mac_t, sizeof(mac_t));
      break;
    case APSTA:
      break;
    case WIFI_OPMODE:
      break;
    case CONFIG11N:
      break;
    case AUTODATARATE:
      break;

    case SECURITY:
      break;
    case TX_POWER:
      break;
    case SSID:
      break;
    case SEND_FRAME:
		pktlen = sizeof(test_data);

		_DBG_AT_SET_CHANNEL(1);
		_DBG_AT_SET_DATA_RATE(0);
		SET_MRX_LEN_FLT(0); // RX enable

		if(-1==getpacketbuffer(&txbuf, pktlen)){
                break;  //skip this case!!
             }
		memcpy(txbuf.databuf, test_data, pktlen);

		SET_MRX_LEN_FLT(10); // RX disable//pre_set ();
		NETSTACK_RDC.send(NULL, txbuf.pktbuf);
		SET_MRX_LEN_FLT(0); // RX enable//post_set ();
#if 0
      for (i = 0; i < atoi(buff); i++) {
        _DBG_AT_SEND_FRAME(hdr_data, sizeof(hdr_data));
      }
#endif
      break;
    case RX_CNT_CLR:
        rx_data_clear();
      break;
    case RX_DISABLE:
        if (atoi(buff) == 1) {
          SET_MRX_LEN_FLT(10);
        } else {
          SET_MRX_LEN_FLT(0);
        }
      break;
    case START_TX:
        process_start(&start_tx_process, NULL);
        process_post (&start_tx_process, PROCESS_EVENT_CONTINUE, NULL);
        break;
    case SET_COUNT:
        nTotalCount = atoi(buff);
        break;
    case SET_DELAY:
        nDelayMSec = atoi(buff);
        break;
#endif
    case CHANNEL:
       // rf_test_config.regChannel = atoi(buff);
      _DBG_AT_SET_CHANNEL(ssv6xxx_atoi(buff));
      break;
    case RF_START:
        RFStart();
        break;
    case RF_RATE:
        RFRate(ssv6xxx_atoi(buff));
        break;
    case RF_BGAIN:
        RFBGain(ssv6xxx_atoi(buff));
        break;
    case RF_GNGAIN:
        RFGNGain(ssv6xxx_atoi(buff));
        break;
    case RF_IQPHASE:
        RFIQPhase(ssv6xxx_atoi(buff));
        break;
    case RF_IQAMP:
        RFIQAmp(ssv6xxx_atoi(buff));
        break;
    case RF_STOP:
        RFStop();
        break;
    case RF_RESET:
        RFReset();
        break;
    case RF_COUNT:
        RFCount(ssv6xxx_atoi(buff));
        break;

    default:
      ret = -1;
      break;
  }
  return ret;
}
int Parse_Radio_Command(s32 argc, char *argv[])
{
	int i=0;
    int nRet=0;

    if(argc==1){
        LOG_PRINTF("usage:\r\n");
        LOG_PRINTF("rf CHANNEL [value] \r\n");
        LOG_PRINTF("rf RF_START \r\n");
        LOG_PRINTF("rf RF_RATE [value]\r\n");
        LOG_PRINTF("rf RF_BGAIN [value]\r\n");
        LOG_PRINTF("rf RF_GNGAIN [value]\r\n");
        LOG_PRINTF("rf RF_IQPHASE [value]\r\n");
        LOG_PRINTF("rf RF_IQAMP [value]\r\n");
        LOG_PRINTF("rf RF_STOP \r\n");
        LOG_PRINTF("rf RF_RESET \r\n");
        LOG_PRINTF("rf RF_COUNT [value] \r\n");
        return -1;
    }

	for(i=0; i<OP_NUM; i++)
	{
		if( !ssv6xxx_strncmp (argv[1], RADIO_OP[i], ssv6xxx_strlen(RADIO_OP[i])-1)   )
		{
			break;
		}
	}

	if( i >= OP_NUM )
	{
		LOG_PRINTF("No Op Find\n");
		return 1;
	}
    nRet = Radio_Command(i, argv[2]);

    if(nRet == 0){
        LOG_PRINTF("+OK\r\n");
    }

	return nRet;
}

void cmd_rf_test(s32 argc, char *argv[])
{
    Parse_Radio_Command(argc, argv);
}
