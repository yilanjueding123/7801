/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _OS_PORTING_H_
#define _OS_PORTING_H_

#include <ssv_types.h>

#define GP_19B 0 //CPU 96MHz SPI
#define GP_15B 1 //144M SPI
#define GP_22B 2 //196M SDIO
/*============GP Platform selection===================*/
#define HOST_PLATFORM_SEL GP_15B

/*============Task Priority===================*/
#define RX_ISR_PRIORITY    0 //No use
#if (HOST_PLATFORM_SEL == GP_22B)
#include <typedef.h>
#include "project.h"

#define CMD_ENG_PRIORITY        6
#define TMR_TASK_PRIORITY       6
#define MLME_TASK_PRIORITY      6
#define WIFI_RX_PRIORITY        6
#define WIFI_TX_PRIORITY        5
#define TCPIP_PRIORITY          5
#define NETAPP_PRIORITY         5
#define NETAPP_PRIORITY_1       5
#define NETAPP_PRIORITY_2       5
#define DHCPD_PRIORITY          5
#define NETMGR_PRIORITY         5
#define TASK_END_PRIO           5
#else
#include "application.h"
#include "os.h"
#define TMR_TASK_PRIORITY  18
#define MLME_TASK_PRIORITY 19
#define CMD_ENG_PRIORITY   20
#define WIFI_RX_PRIORITY   21
#define WIFI_TX_PRIORITY   22
#define TCPIP_PRIORITY     23
#define DHCPD_PRIORITY     24
#define NETAPP_PRIORITY    25

#define NETAPP_PRIORITY_1  NETAPP_PRIORITY+1
#define NETAPP_PRIORITY_2  NETAPP_PRIORITY+2
#define NETMGR_PRIORITY    NETAPP_PRIORITY+3
#define VDO_ENC_PRIO       NETAPP_PRIORITY+4
#define MJPG_STREAMER_PRIO NETAPP_PRIORITY+5
#define TASK_END_PRIO      NETAPP_PRIORITY+6  //31
#endif

/*============Console setting===================*/
#define hal_print print_string
//#define PRINTF print_string
//#define stdout NULL
#define FFLUSH(x)
#if (HOST_PLATFORM_SEL == GP_19B)
#define hal_putchar(ch) uart0_data_send(ch,1)
extern void uart0_data_send(u8 data, u8 wait);
extern INT32S uart0_data_get(u8 *data, u8 wait);
#elif (HOST_PLATFORM_SEL == GP_15B)
	#if (GPDV_BOARD_VERSION == GPCV1248_V1_0)
#define hal_putchar(ch) uart_data_send(0,ch,1)		
void uart_data_send(INT8U uart_num,INT8U data, INT8U wait);	
extern INT32S uart_data_get(INT8U num, INT8U *data, INT8U wait);
	#else
#define hal_putchar(ch) drv_l1_uart1_data_send(ch,1)
extern void drv_l1_uart1_data_send(INT8U data, INT8U wait);
extern INT32S drv_l1_uart1_data_get(INT8U *data, INT8U wait);
#endif
#endif
extern u8 hal_getchar(void);


/*============Compiler setting===================*/
#if (HOST_PLATFORM_SEL == GP_22B)
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__));

#undef STRUCT_PACKED
//#define STRUCT_PACKED __attribute__ ((packed)) // williamyeo
#define STRUCT_PACKED __attribute__((__packed__))
#define UNION_PACKED
#define ALIGN_ARRAY(a) 
#else
#define ARM_ADS
#define PACK( __Declaration__ ) __packed __Declaration__;
#undef STRUCT_PACKED
#define STRUCT_PACKED __packed
#define UNION_PACKED __packed
//LWIP PACK Definition
#define PACK_STRUCT_BEGIN __packed
#define PACK_STRUCT_FIELD(x) __packed x
#define inline __inline
#define ALIGN_ARRAY(a) __align(a)
#endif
/*============SSV-DRV setting===================*/
#define	CONFIG_RX_POLL      0
#if (HOST_PLATFORM_SEL == GP_22B)
#define INTERFACE "sdio"
#define SDRV_INCLUDE_SDIO   1
#define SDIO_CARD_INT_TRIGGER
#else
#define INTERFACE "spi"
#define SDRV_INCLUDE_SPI    1
#endif
/*============Stack Size (unint: 16bytes)===================*/
#define TMR_TASK_STACK_SIZE  64
#define MLME_TASK_STACK_SIZE 0
#define CMD_ENG_STACK_SIZE   128
#define TCPIP_STACK_SIZE     128
#define DHCPD_STACK_SIZE     64
#define NETAPP1_STACK_SIZE    128
#define NETAPP2_STACK_SIZE    128
#define NETAPP3_STACK_SIZE    128
#define NETMGR_STACK_SIZE    80
#define CLI_TASK_STACK_SIZE  64
#define RX_ISR_STACK_SIZE    0
#define WIFI_RX_STACK_SIZE   64
#define WIFI_TX_STACK_SIZE   64
#define PING_THREAD_STACK_SIZE 0 //16 , ping thread doesn't enable now, I set staic size is zero to reduce data size.

#define TOTAL_STACK_SIZE (TMR_TASK_STACK_SIZE+ \
                          MLME_TASK_STACK_SIZE+ \
                          CMD_ENG_STACK_SIZE+ \
                          TCPIP_STACK_SIZE+ \
                          DHCPD_STACK_SIZE+ \
                          NETAPP1_STACK_SIZE+ \
						  NETAPP2_STACK_SIZE+ \
						  NETAPP3_STACK_SIZE+ \
                          NETMGR_STACK_SIZE+ \
                          CLI_TASK_STACK_SIZE+ \
						  RX_ISR_STACK_SIZE+ \
						  WIFI_RX_STACK_SIZE+ \
                          WIFI_TX_STACK_SIZE+ \
                          PING_THREAD_STACK_SIZE)
                          
/*============Memory========================*/
OS_APIs void *OS_MemAlloc( u32 size );
OS_APIs void __OS_MemFree( void *m );
OS_APIs void OS_MemSET(void *pdest, u8 byte, u32 size);
OS_APIs void OS_MemCPY(void *pdest, const void *psrc, u32 size);

/*=========================================*/
void platform_ldo_en_pin_init(void);
void platform_ldo_en(bool en);

#endif
