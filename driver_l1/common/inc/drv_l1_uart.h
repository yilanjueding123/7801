#ifndef __drv_l1_UART_H__
#define __drv_l1_UART_H__

#include "driver_l1.h"
#include "drv_l1_sfr.h"
#include "driver_l1_cfg.h"

/******************************************************************************
 * UART0: 0xC0060000  UART1: 0xC0070000
 ******************************************************************************/
#define P_UART0_BASE				((volatile INT32U *) 0xC0060000)

#define P_UART1_BASE				((volatile INT32U *) 0xC0070000)


// UART RX error status register
#define	C_UART_ERR_FRAME			0x00000001
#define	C_UART_ERR_PARITY			0x00000002
#define	C_UART_ERR_BREAK			0x00000004
#define	C_UART_ERR_OVERRUN			0x00000008

// UART control register
#define	C_UART_CTRL_RX_INT			0x00008000
#define	C_UART_CTRL_TX_INT			0x00004000
#define	C_UART_CTRL_RXTO_INT		0x00002000
#define	C_UART_CTRL_UART_ENABLE		0x00001000
#define	C_UART_CTRL_MODEM_INT		0x00000800
#define	C_UART_CTRL_LOOPBACK		0x00000400
#define	C_UART_CTRL_WORD_8BIT		0x00000060
#define	C_UART_CTRL_WORD_7BIT		0x00000040
#define	C_UART_CTRL_WORD_6BIT		0x00000020
#define	C_UART_CTRL_WORD_5BIT		0x00000000
#define	C_UART_CTRL_FIFO_ENABLE		0x00000010
#define	C_UART_CTRL_2STOP_BIT		0x00000008
#define	C_UART_CTRL_EVEN_PARITY		0x00000004
#define	C_UART_CTRL_PARITY_EN		0x00000002
#define	C_UART_CTRL_SEND_BREAK		0x00000001

// UART interrupt pending bit and status register
#define	C_UART_STATUS_RX_INT		0x00008000
#define	C_UART_STATUS_TX_INT		0x00004000
#define	C_UART_STATUS_RXTO_INT		0x00002000
#define	C_UART_STATUS_TX_EMPTY		0x00000080
#define	C_UART_STATUS_RX_FULL		0x00000040
#define	C_UART_STATUS_TX_FULL		0x00000020
#define	C_UART_STATUS_RX_EMPTY		0x00000010
#define	C_UART_STATUS_BUSY			0x00000008

// UART FIFO register
#define	C_UART_FIFO_TX_LEVEL_0		0x00000000
#define	C_UART_FIFO_TX_LEVEL_1		0x00001000
#define	C_UART_FIFO_TX_LEVEL_2		0x00002000
#define	C_UART_FIFO_TX_LEVEL_3		0x00003000
#define	C_UART_FIFO_TX_LEVEL_4		0x00004000
#define	C_UART_FIFO_TX_LEVEL_5		0x00005000
#define	C_UART_FIFO_TX_LEVEL_6		0x00006000
#define	C_UART_FIFO_TX_LEVEL_7		0x00007000
#define	C_UART_FIFO_RX_LEVEL_0		0x00000000
#define	C_UART_FIFO_RX_LEVEL_1		0x00000010
#define	C_UART_FIFO_RX_LEVEL_2		0x00000020
#define	C_UART_FIFO_RX_LEVEL_3		0x00000030
#define	C_UART_FIFO_RX_LEVEL_4		0x00000040
#define	C_UART_FIFO_RX_LEVEL_5		0x00000050
#define	C_UART_FIFO_RX_LEVEL_6		0x00000060
#define	C_UART_FIFO_RX_LEVEL_7		0x00000070

#define MAX_UART_SIZE 128
#define UART_RX_MESSAGE 1  // must match "UART_USED_NUM" which is in customer.h

// UART
typedef enum
{
	WORD_LEN_5,
	WORD_LEN_6,
	WORD_LEN_7,
	WORD_LEN_8
}WORD_LEN;

typedef enum
{
	STOP_SIZE_1,
	STOP_SIZE_2
}STOP_BIT_SIZE;

typedef enum
{
	PARITY_ODD,
	PARITY_EVEN
}PARITY_SEL;

typedef enum
{
    UART_0,                 
    UART_1,
    UART_MAX
} UART_NUM;

typedef struct 
{
	volatile INT32U	DATA;
	volatile INT32U	RX_STATUS;
	volatile INT32U	CTRL;
	volatile INT32U	BAUD_RATE;
	volatile INT32U	STATUS;
	volatile INT32U	FIFO;
	volatile INT32U	TXDLY;
}UART_SFR;

// UART extern API
extern void uart_init(INT8U uart_num);
extern void uart_buad_rate_set(INT8U uart_num,INT32U bps);
extern void uart_rx_enable(INT8U uart_num);
extern void uart_rx_disable(INT8U uart_num);
extern void uart_tx_enable(INT8U uart_num);
extern void uart_tx_disable(INT8U uart_num);
extern void uart_data_send(INT8U uart_num,INT8U data, INT8U wait);
extern INT32S uart_data_get(INT8U uart_num,INT8U *data, INT8U wait);

extern INT32S uart_word_len_set(INT8U uart_num,INT8U word_len);
extern INT32S uart_stop_bit_size_set(INT8U uart_num,INT8U stop_size);
extern INT32S uart_parity_chk_set(INT8U uart_num,INT8U status, INT8U parity);
#if (defined UART_RX_MESSAGE) && (UART_RX_MESSAGE == 0)
extern void uart_resource0_register(void *msgq_id, INT32U msg_id);
extern INT8U *uart_resource0(void);
#endif
#if (defined UART_RX_MESSAGE) && (UART_RX_MESSAGE == 1)
extern void uart_resource1_register(void *msgq_id, INT32U msg_id);
extern INT8U *uart_resource1(void);
#endif
extern void uart_resource_register(void *msgq_id, INT32U msg_id);
extern INT8U *uart_resource(void);

#if WIFI_FUNC_ENABLE
extern void drv_l1_uart1_data_send(INT8U data, INT8U wait);
extern INT32S drv_l1_uart1_data_get(INT8U *data, INT8U wait);
#endif

#endif		// __drv_l1_UART_H__

