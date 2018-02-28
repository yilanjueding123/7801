/******************************************************
* gspi_master_drv.h
*
* Purpose: GSPI master header file
*
* Author: Eugene Hsu
*
* Date: 2015/08/27
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 
* History :
*
*******************************************************/
#ifndef __GSPI_MASTER_DRV_H__
#define __GSPI_MASTER_DRV_H__

#define BIT0	0x0001
#define BIT1	0x0002
#define BIT2	0x0004
#define BIT3	0x0008
#define BIT4	0x0010
#define BIT5	0x0020
#define BIT6	0x0040
#define BIT7	0x0080
#define BIT8	0x0100
#define BIT9	0x0200
#define BIT10	0x0400
#define BIT11	0x0800
#define BIT12	0x1000
#define BIT13	0x2000
#define BIT14	0x4000
#define BIT15	0x8000
#define BIT16	0x00010000
#define BIT17	0x00020000
#define BIT18	0x00040000
#define BIT19	0x00080000
#define BIT20	0x00100000
#define BIT21	0x00200000
#define BIT22	0x00400000
#define BIT23	0x00800000
#define BIT24	0x01000000
#define BIT25	0x02000000
#define BIT26	0x04000000
#define BIT27	0x08000000
#define BIT28	0x10000000
#define BIT29	0x20000000
#define BIT30	0x40000000
#define BIT31	0x80000000

enum
{
	SPI_LITTLE_ENDIAN = 2,
	SPI_BIG_ENDIAN = 0
};

enum
{
	SPI_WORD_LEN_16 = 0,
	SPI_WORD_LEN_32 = 1
};

typedef enum
{
	SPI_LITTLE_ENDIAN_16 = SPI_LITTLE_ENDIAN|SPI_WORD_LEN_16,
	SPI_LITTLE_ENDIAN_32 = SPI_LITTLE_ENDIAN|SPI_WORD_LEN_32, // default configure
	SPI_BIG_ENDIAN_16 = SPI_BIG_ENDIAN|SPI_WORD_LEN_16,
	SPI_BIG_ENDIAN_32 = SPI_BIG_ENDIAN|SPI_WORD_LEN_32
}_gspi_conf_t;

typedef enum
{
	READ_REG = 0,
	WRITE_REG
}_reg_ops;

struct gspi_more_data
{
	INT32U more_data;
	INT32U len;
};

#define AGG_SIZE	5000
#define PACK_SIZE	2048
#define BUFFER_LEN	(4+ 24 + PACK_SIZE + 8) // GSPI_CMD + TX_DEC + DATA + GSPI_STATUS

typedef void(*GSPI_RX_DATA_CBK)(INT8U* buf, INT32U len);

/* Following is GP's definition */
#define GSPI_INT_STACK_SIZE		1024
#define	SPI_MASTER_NUM			0			/* Use SPI 1 */
#define GSPI_MASTER_CS_PIN		IO_E0
#define GSPI_MASTER_CLK			SYSCLK_8	/* system clock(144Mhz) / GSPI_MASTER_CLK */

#define GSPI_INT_TASK_PRIORITY		19
#define GSPI_TASK_PRIORITY   		20
#define CONSOLE_TASK_PRIORITY		21
#define MJPEG_STREAMER_PRIORITY		22
#define GP_SOCKET_CMD_PRIORITY		23
#define GPCMD_TASK_PRIORITY		24
#define JPG_REPEAT_PRIORITY		25

extern void print_string(CHAR *fmt, ...);

extern INT32S gspi_master_init(void);
extern int gspi_read_rx_fifo(INT8U *buf, INT32U len, struct gspi_more_data * pmore_data,_gspi_conf_t gspi_conf);
extern int gspi_write_tx_fifo(INT8U *buf, INT32U len, _gspi_conf_t gspi_conf);
extern int gspi_write_page(INT8U *buf, INT32U len, INT8U agg_cnt);
extern int gspi_read_page(INT8U *buf, INT32U* len);
extern void gspi_tx_data(INT8U* buf, INT32U len, INT32U type);

extern INT32S gspi_write32(INT32U addr, INT32U buf, INT32S *err);
extern INT32U gspi_read32(INT32U addr, INT32S *err);
extern int gspi_configuration(_gspi_conf_t gspi_conf);
extern void gspi_config_slave_device(void);
extern void gspi_register_rx_cbk(void* cbk);

#endif	//__GSPI_MASTER_DRV_H__