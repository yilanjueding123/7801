/*
* Purpose: UART driver/interface
*
* Author: Tristan Yang
*
* Date: 2007/09/21
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 1.00
*/

#include "drv_l1_uart.h"
#include "drv_l1_interrupt.h"
#include "drv_l1_system.h"

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#if (defined _DRV_L1_UART) && (_DRV_L1_UART == 1)                 //
//================================================================//
#define UART_RX			0
#define UART_TX			1
#define UART_DISABLE	0
#define UART_ENABLE		1

#define UART_FIFO_SIZE 32

//================================================================//
typedef struct fifo {
	INT8U *p_start;
	INT8U *p_end;
	INT8U *p_write;
	INT8U *p_read;
} UART_FIFO;

//================================================================//
UART_FIFO 	sw_fifo_head[UART_MAX];
INT8U 		sw_fifo_buf[UART_MAX][UART_FIFO_SIZE];
INT32U		uart_tx_on[UART_MAX];
//================================================================//

void uart_set_ctrl(INT8U uart_num,INT32U val);
INT32U uart_get_ctrl(INT8U uart_num);

#if _OPERATING_SYSTEM != _OS_NONE	
static OS_EVENT		*sw_uart_sem[UART_MAX] = {NULL};
#endif

//================================================================//

#if _OPERATING_SYSTEM == 1	
void sw_uart_lock(INT8U uart_num)
{
	INT8U err;

	OSSemPend(sw_uart_sem[uart_num], 0, &err);
}

void sw_uart_unlock(INT8U uart_num)
{
	OSSemPost(sw_uart_sem[uart_num]);
}
#endif

static UART_SFR * get_UART_SFR_base(INT8U uart_num)
{
	if (uart_num == 0){
		return (UART_SFR *)P_UART0_BASE;
	}
	else {
		return (UART_SFR *)P_UART1_BASE;
	}
}

INT32S uart_sw_fifo_init(INT8U uart_num)
{
	INT32U i;

	for (i=0; i<UART_FIFO_SIZE; i++) {
		sw_fifo_buf[uart_num][i] = 0;
	}

	sw_fifo_head[uart_num].p_start = &sw_fifo_buf[uart_num][0];
	sw_fifo_head[uart_num].p_end = &sw_fifo_buf[uart_num][UART_FIFO_SIZE-1];
	sw_fifo_head[uart_num].p_write = &sw_fifo_buf[uart_num][0];
	sw_fifo_head[uart_num].p_read = &sw_fifo_buf[uart_num][0];

	return 0;
}

INT32S uart_sw_fifo_get(INT8U uart_num,INT8U *data)
{
	// Only read p_write once, and only write p_read once
	INT8U *read_ptr;

	if (!data) {
		return -1;
	}
  #if _OPERATING_SYSTEM != _OS_NONE				// Soft Protect for critical section
	sw_uart_lock(uart_num);
  #endif
	if (sw_fifo_head[uart_num].p_write == sw_fifo_head[uart_num].p_read) {		// FIFO is empty
	  #if _OPERATING_SYSTEM != _OS_NONE
		sw_uart_unlock(uart_num);
	  #endif
		return -2;
	}

	read_ptr = sw_fifo_head[uart_num].p_read;
	*data = *read_ptr++;
	if (read_ptr > sw_fifo_head[uart_num].p_end) {
		read_ptr = sw_fifo_head[uart_num].p_start;
	}
	sw_fifo_head[uart_num].p_read = read_ptr;
  #if _OPERATING_SYSTEM != _OS_NONE
	sw_uart_unlock(uart_num);
  #endif

	return 0;
}

INT32S uart_sw_fifo_put(INT8U uart_num,INT8U data)		// Self protect FIFO
{
	// Only read p_read once, and only write p_write once
	INT8U *read_ptr, *write_ptr;

	read_ptr = sw_fifo_head[uart_num].p_read - 1;
	if (read_ptr < sw_fifo_head[uart_num].p_start) {
		read_ptr = sw_fifo_head[uart_num].p_end;
	}
	write_ptr = sw_fifo_head[uart_num].p_write;
	if (write_ptr == read_ptr) {			// FIFO is full
		return -2;
	}
	*write_ptr++ = data;
	if (write_ptr > sw_fifo_head[uart_num].p_end) {
		write_ptr = sw_fifo_head[uart_num].p_start;
	}
	sw_fifo_head[uart_num].p_write = write_ptr;
	return 0;
}

void uart_set_to_int(INT8U uart_num,INT8U status)
{
	INT32U val;
	val = uart_get_ctrl(uart_num);

	if (status == UART_ENABLE) {
		val |= C_UART_CTRL_RXTO_INT;
	} else {
		val &= ~C_UART_CTRL_RXTO_INT;
	}

	uart_set_ctrl(uart_num,val);

}
INT32S uart_device_protect(void) 
{
	return vic_irq_disable(VIC_UART);
}

void uart_device_unprotect(INT32S mask)
{
	if (mask == 0) {						// Clear device interrupt mask
		vic_irq_enable(VIC_UART);
	} else if (mask == 1) {
		vic_irq_disable(VIC_UART);
	} else {								// Something is wrong, do nothing
		return;
	}
}

void uart_isr(void)
{
	INT32U status = 0;
	INT8U  uart_num;
	UART_SFR* pUart;

	pUart = get_UART_SFR_base(UART_0);
	status = pUart->STATUS;
	uart_num = UART_0;
	
	if (status & C_UART_CTRL_RX_INT) {
		while ((pUart->STATUS & C_UART_STATUS_RX_EMPTY) == 0) {
			uart_sw_fifo_put(uart_num,(INT8U) pUart->DATA);
		}
	}

	if (status & C_UART_CTRL_RXTO_INT) {
		while ((pUart->STATUS & C_UART_STATUS_RX_EMPTY) == 0) {
			uart_sw_fifo_put(uart_num,(INT8U) pUart->DATA);
		}
	}
	//-------------------------------------------------------
	pUart = get_UART_SFR_base(UART_1);
	status = pUart->STATUS;
	uart_num = UART_1;
	
	if (status & C_UART_CTRL_RX_INT) {
		while ((pUart->STATUS & C_UART_STATUS_RX_EMPTY) == 0) {
			uart_sw_fifo_put(uart_num,(INT8U) pUart->DATA);
		}
	}

	if (status & C_UART_CTRL_RXTO_INT) {
		while ((pUart->STATUS & C_UART_STATUS_RX_EMPTY) == 0) {
			uart_sw_fifo_put(uart_num,(INT8U) pUart->DATA);
		}
	}
		
}

void uart_set_ctrl(INT8U uart_num,INT32U val)
{
	// Bit15: Receive interrupt enable
	// Bit14: Transmit interrupt enable
	// Bit13: Receive timeout interrupt enable
	// Bit12: UART enable
	// Bit11: Modem status interrupt enable
	// Bit10: Self loop test enable(used in IrDA mode only)
	// Bit9-7: Reservede
	// Bit6-5: Word length, 00=5 bits, 01=6 bits, 10=7 bits, 11=8 bits
	// Bit4: FIFO enable
	// Bit3: Stop bit size, 0=1 bit, 1=2 bits
	// Bit2: Parity, 0=odd parity, 1=even parity
	// Bit1: Parity enable, 0=disable, 1=enable
	// Bit0: Send break, 0=normal, 1=send break signal
	UART_SFR* pUart = get_UART_SFR_base(uart_num);

	pUart->CTRL = val & 0xFC7F;
}

INT32U uart_get_ctrl(INT8U uart_num)
{
	UART_SFR* pUart = get_UART_SFR_base(uart_num);
	
	return pUart->CTRL;
}

void uart_buad_rate_set(INT8U uart_num,INT32U bps)
{
	UART_SFR* pUart = get_UART_SFR_base(uart_num);
	
	pUart->BAUD_RATE= MCLK / bps;
}

INT32U uart_get_status(INT8U uart_num)
{
	UART_SFR* pUart = get_UART_SFR_base(uart_num);

	return (pUart->STATUS& 0xE0FF);
}

void uart_fifo_ctrl(INT8U uart_num,INT8U tx_level, INT8U rx_level)
{
	INT32U val;
	UART_SFR* pUart = get_UART_SFR_base(uart_num);

	val = ((INT32U) tx_level & 0x7) << 12;
	val |= ((INT32U) rx_level & 0x7) << 4;
	pUart->FIFO= val;
}

void uart_init(INT8U uart_num)
{
	UART_SFR* pUart = get_UART_SFR_base(uart_num);

	// 8 bits, 1 stop bit, no parity check, RX disabled
	pUart->CTRL= (C_UART_CTRL_UART_ENABLE | C_UART_CTRL_WORD_8BIT);

#ifdef UART_PIN_POS
    #if UART_PIN_POS == UART_TX_IOH2__RX_IOH3
        R_GPIOCTRL = 0; /*switch off ice*/
        R_FUNPOS0 |= 1;
    #else
        R_FUNPOS0 &= ~(1);
    #endif
#endif
#if _OPERATING_SYSTEM == 1	
	if(sw_uart_sem[uart_num] == NULL)
	{
		sw_uart_sem[uart_num] = OSSemCreate(1);
	}
#endif    

  #if _OPERATING_SYSTEM != _OS_NONE				// Soft Protect for critical section
	sw_uart_lock(uart_num);
  #endif
	uart_tx_on[uart_num] = 0;					// TX is disabled by software
  #if _OPERATING_SYSTEM != _OS_NONE
	sw_uart_unlock(uart_num);
  #endif

	uart_sw_fifo_init(uart_num);
    vic_irq_register(VIC_UART, uart_isr);	// Non vector interrupt register
    vic_irq_enable(VIC_UART);	
    //uart_set_to_int(uart_num,UART_ENABLE); /* enable rx timeout interrupt */
}

void uart_rx_tx_en(INT8U uart_num,INT32U dir, INT32U enable)
{
	UART_SFR* pUart = get_UART_SFR_base(uart_num);

  #if _OPERATING_SYSTEM != _OS_NONE				// Soft Protect for critical section
	sw_uart_lock(uart_num);
  #endif
	if (dir == UART_RX) {					// RX
		if (enable == UART_ENABLE) {
			pUart->CTRL |= C_UART_CTRL_RX_INT;
		} else {
			pUart->CTRL &= ~C_UART_CTRL_RX_INT;
		}
	} else {								// TX
		if (enable == UART_ENABLE) {
			uart_tx_on[uart_num] = 1;
		} else {
			uart_tx_on[uart_num] = 0;
		}
	}
  #if _OPERATING_SYSTEM != _OS_NONE
	sw_uart_unlock(uart_num);
  #endif
}

void uart_rx_enable(INT8U uart_num)
{
	uart_rx_tx_en(uart_num,UART_RX, UART_ENABLE);
}

void uart_rx_disable(INT8U uart_num)
{
	uart_rx_tx_en(uart_num,UART_RX, UART_DISABLE);
}

void uart_tx_enable(INT8U uart_num)
{
	uart_rx_tx_en(uart_num,UART_TX, UART_ENABLE);
}

void uart_tx_disable(INT8U uart_num)
{
	uart_rx_tx_en(uart_num,UART_TX, UART_DISABLE);
}

void uart_fifo_en(INT8U uart_num,INT32U enable)
{
	UART_SFR* pUart = get_UART_SFR_base(uart_num);

  #if _OPERATING_SYSTEM != _OS_NONE				// Soft Protect for critical section
	sw_uart_lock(uart_num);
  #endif
	if (enable == UART_ENABLE) {
		pUart->CTRL |= C_UART_CTRL_FIFO_ENABLE;
	} else {
		pUart->CTRL &= ~C_UART_CTRL_FIFO_ENABLE;
	}
  #if _OPERATING_SYSTEM != _OS_NONE
	sw_uart_unlock(uart_num);
  #endif
}

void uart_fifo_enable(INT8U uart_num)
{
	uart_fifo_en(uart_num,UART_ENABLE);
}

void uart_fifo_disable(INT8U uart_num)
{
	uart_fifo_en(uart_num,UART_DISABLE);
}

#if WIFI_FUNC_ENABLE
void drv_l1_uart1_data_send(INT8U data, INT8U wait)
{
	uart_data_send(UART_1, data, wait);
}
#endif

void uart_data_send(INT8U uart_num,INT8U data, INT8U wait)
{
	UART_SFR* pUart = get_UART_SFR_base(uart_num);

  #if _OPERATING_SYSTEM != _OS_NONE				// Soft Protect for critical section
	sw_uart_lock(uart_num);
  #endif
	if (uart_tx_on[uart_num] == 0) {
	  #if _OPERATING_SYSTEM != _OS_NONE
		sw_uart_unlock(uart_num);
	  #endif
	  	return;
	}
  #if _OPERATING_SYSTEM != _OS_NONE
	sw_uart_unlock(uart_num);
  #endif

	if (wait) {
		while (pUart->STATUS& C_UART_STATUS_TX_FULL) ;		// Wait until FIFO is not full
	}

	pUart->DATA= data;						// Send our data now
	
}

#if WIFI_FUNC_ENABLE
INT32S drv_l1_uart1_data_get(INT8U *data, INT8U wait)
{
	return uart_data_get(UART_1, data, wait);
}
#endif


INT32S uart_data_get(INT8U uart_num,INT8U *data, INT8U wait)
{
    while (uart_sw_fifo_get(uart_num,data)) {
    	if (!wait) {						// Queue is empty and the caller doesn't want to wait
    		return STATUS_FAIL;
    	}
    }
    return STATUS_OK;
}

INT32S uart_word_len_set(INT8U uart_num,INT8U word_len)
{
	UART_SFR* pUart = get_UART_SFR_base(uart_num);

	pUart->CTRL &= ~(3<<5);

	switch(word_len) {
		case WORD_LEN_5:
			pUart->CTRL |= C_UART_CTRL_WORD_5BIT;
			break;
		case WORD_LEN_6:
			pUart->CTRL |= C_UART_CTRL_WORD_6BIT;
			break;
		case WORD_LEN_7:
			pUart->CTRL |= C_UART_CTRL_WORD_7BIT;
			break;
		case WORD_LEN_8:
			pUart->CTRL |= C_UART_CTRL_WORD_8BIT;
			break;
		default :
			break;
	}

	return STATUS_OK;
}

INT32S uart_stop_bit_size_set(INT8U uart_num,INT8U stop_size)
{
	UART_SFR* pUart = get_UART_SFR_base(uart_num);

	pUart->CTRL &= ~(1<<3);

	if (stop_size == STOP_SIZE_2) {
		pUart->CTRL |= C_UART_CTRL_2STOP_BIT;
	}

	return STATUS_OK;
}

INT32S uart_parity_chk_set(INT8U uart_num,INT8U status, INT8U parity)
{
	UART_SFR* pUart = get_UART_SFR_base(uart_num);

	if (status == UART_DISABLE) {
		pUart->CTRL &= ~(1<<1);
	}

	else {
		pUart->CTRL |= C_UART_CTRL_PARITY_EN;
		pUart->CTRL &= ~(1<<2);
		if (parity == PARITY_EVEN) {
			pUart->CTRL |= C_UART_CTRL_EVEN_PARITY;
		}
	}

	return STATUS_OK;
}

#endif
