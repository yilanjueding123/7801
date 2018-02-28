/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <ssv_common.h>
#include <host_config.h>


#if (CONFIG_CHIP_ID==SSV6051Q)
const ssv_rf_temperature rf_temper_setting[RF_TEMPER_ARRARY_SIZE]={
    //addrr         high                regular         low
    {0xCE010008, 0x000DF61B, 0x000DF61B, 0x000DF61B},
    {0xCE010014, 0x3D3E84FE, 0x3D3E84FE, 0x3D3E84FE},
    {0xCE010018, 0x01457D79, 0x01457D79, 0x01457D79},
    {0xCE010048, 0xFCCCCF27, 0xFCCCCF27, 0xFCCCCF27},
    {0xCE010050, 0x00444000, 0x00444000, 0x00444000},    
};
#endif

#if (CONFIG_CHIP_ID==SSV6051Z)
const ssv_rf_temperature rf_temper_setting[RF_TEMPER_ARRARY_SIZE]={
    //addrr         high                regular         low
    {0xCE010008, 0x00EB7C1C, 0x00EB7C1C, 0x00EB7C1C},
    {0xCE010014, 0x3D7E84FE, 0x3D7E84FE, 0x3D7E84FE},
    {0xCE010018, 0x01457D79, 0x01457D79, 0x01457D79},
    {0xCE010048, 0xFCCCCF27, 0xFCCCCF27, 0xFCCCCF27},
    {0xCE010050, 0x0047C000, 0x0047C000, 0x0047C000},    
};
#endif

#if (CONFIG_CHIP_ID==SSV6030P)
const ssv_rf_temperature rf_temper_setting[RF_TEMPER_ARRARY_SIZE]={
    //addrr         high                regular         low
    {0xCE010008, 0x008B7C1C, 0x008B7C1C, 0x008B7C1C},
    {0xCE010014, 0x3D7E84FE, 0x3D7E84FE, 0x3D7E84FE},
    {0xCE010018, 0x01457D79, 0x01457D79, 0x01457D79},
    {0xCE010048, 0xFCCCCC27, 0xFCCCCC27, 0xFCCCCC27},
    {0xCE010050, 0x0047C000, 0x0047C000, 0x0047C000},    
};
#endif

