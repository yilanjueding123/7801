#ifndef __DRV_L1_SDC_H__
#define __DRV_L1_SDC_H__

#include "driver_l1.h"
#include "drv_l1_sfr.h"


/******************************************************************************
 * SDC: 0xC00A0000
 ******************************************************************************/ 
#define	P_SDC0_BASE					((volatile INT32U *) 0xC00A0000)
#define	P_SDC0_DATA_TX				((volatile INT32U *) 0xC00A0000)
#define	P_SDC0_DATA_RX				((volatile INT32U *) 0xC00A0004)

#define	R_SDC0_DATA_TX				(*((volatile INT32U *) 0xC00A0000))
#define	R_SDC0_DATA_RX				(*((volatile INT32U *) 0xC00A0004))
#define	R_SDC0_CMMAND				(*((volatile INT32U *) 0xC00A0008))
#define	R_SDC0_ARGUMENT				(*((volatile INT32U *) 0xC00A000C))
#define	R_SDC0_RESPONSE				(*((volatile INT32U *) 0xC00A0010))
#define	R_SDC0_STATUS				(*((volatile INT32U *) 0xC00A0014))
#define	R_SDC0_CTRL				    (*((volatile INT32U *) 0xC00A0018))
#define	R_SDC0_INTEN				(*((volatile INT32U *) 0xC00A001C))

/******************************************************************************
 * SDC1: 0xC00F0000
 ******************************************************************************/ 
#define	P_SDC1_BASE					((volatile INT32U *) 0xC00F0000)
#define	P_SDC1_DATA_TX				((volatile INT32U *) 0xC00F0000)
#define	P_SDC1_DATA_RX				((volatile INT32U *) 0xC00F0004)
	
#define	R_SDC1_DATA_TX				(*((volatile INT32U *) 0xC00F0000))
#define	R_SDC1_DATA_RX				(*((volatile INT32U *) 0xC00F0004))
#define	R_SDC1_CMMAND				(*((volatile INT32U *) 0xC00F0008))
#define	R_SDC1_ARGUMENT				(*((volatile INT32U *) 0xC00F000C))
#define	R_SDC1_RESPONSE				(*((volatile INT32U *) 0xC00F0010))
#define	R_SDC1_STATUS				(*((volatile INT32U *) 0xC00F0014))
#define	R_SDC1_CTRL				    (*((volatile INT32U *) 0xC00F0018))
#define	R_SDC1_INTEN				(*((volatile INT32U *) 0xC00F001C))


// SDC control register
#define C_SDC_CTL_CLKDIV				((10*MHZ - 8) / 8)
#define	C_SDC_CTL_BUS_4BIT				0x00000100
#define	C_SDC_CTL_DMA_ENABLE	  		0x00000200
#define	C_SDC_CTL_IO_INT				0x00000400
#define	C_SDC_CTL_ENABLE				0x00000800
#define C_SDC_CTL_DUMMYCLK				0x00001000
#define	C_SDC_CTL_BLK8			  		0x00080000
#define	C_SDC_CTL_BLK256				0x01000000
#define	C_SDC_CTL_BLK512				0x02000000

// SDC command register
#define	C_SDC_CMD_STOP					0x00000040
#define	C_SDC_CMD_RUN					0x00000080
#define C_SDC_CMD_WITH_DATA		  		0x00000100
#define C_SDC_CMD_WRITE_DATA			0x00000200
#define C_SDC_CMD_MULTI_BLOCK	  		0x00000400
#define C_SDC_CMD_INIT_CARD	 			0x00000800
#define C_SDC_CMD_RESP_R0				0x00000000
#define C_SDC_CMD_RESP_R1				0x00001000
#define C_SDC_CMD_RESP_R2				0x00002000
#define C_SDC_CMD_RESP_R3				0x00003000
#define C_SDC_CMD_RESP_R6				0x00006000
#define C_SDC_CMD_RESP_R7  				0x00001000
#define C_SDC_CMD_RESP_R1B		  		0x00007000

// SDC status register
#define C_SDC_STATUS_CONTROLLER_BUSY	0x00000001
#define C_SDC_STATUS_CARD_BUSY			0x00000002
#define C_SDC_STATUS_CMD_COMPLETE		0x00000004
#define C_SDC_STATUS_DATA_COMPLETE		0x00000008
#define C_SDC_STATUS_RESP_INDEX_ERR		0x00000010
#define C_SDC_STATUS_RESP_CRC_ERR		0x00000020
#define C_SDC_STATUS_RESP_REG_FULL		0x00000040
#define C_SDC_STATUS_DATA_BUF_FULL		0x00000080
#define C_SDC_STATUS_DATA_BUF_EMPTY		0x00000100
#define C_SDC_STATUS_TIMEOUT			0x00000200
#define C_SDC_STATUS_DATA_CRC_ERR		0x00000400
#define C_SDC_STATUS_CARD_PROTECT		0x00000800
#define C_SDC_STATUS_CARD_PRESENT	  	0x00001000
#define	C_SDC_STATUS_SDIO_INT_PEND		0x00002000
#define C_SDC_STATUS_CLEAR_ALL			0x0000FFFF

// SDC interrupt register
#define	C_SDC_INT_CMD_COMP				0x00000001
#define	C_SDC_INT_DATA_COMP				0x00000002
#define	C_SDC_INT_RBUF_FULL				0x00000004
#define	C_SDC_INT_DBUF_FULL				0x00000008
#define	C_SDC_INT_DBUF_EMPT				0x00000010
#define	C_SDC_INT_PRESENT				0x00000020
#define	C_SDC_INT_IO_INT				0x00000040


typedef struct sdReg_s {
	volatile INT32U DataTx;		/* Data Transmit register */
	volatile INT32U DataRx;		/* Data Receive register */
	volatile INT32U Cmd;		/* Command register */ 
	volatile INT32U Arg;		/* Argument register */
	volatile INT32U Resp;		/* Response register */
	volatile INT32U Status;		/* Controller status register */
	volatile INT32U Ctrl;		/* Control register */
	volatile INT32U IntEn;		/* Interrupt enable register */	
} sdReg_t;

#define SetSDBase(x) ((x==0)?P_SDC0_BASE : P_SDC1_BASE)

// SD Controller: APIs for SD controller
INT32S drvl1_sdc_init(INT32U device_id);
INT32S drvl1_sdc_enable(INT32U device_id);
INT32S drvl1_sdc_disable(INT32U device_id);
INT32S drvl1_sdc_controller_busy_wait(INT32U device_id, INT32U timeout);
INT32S drvl1_sdc_clock_set(INT32U device_id, INT32U speed);
INT32S drvl1_sdc_block_len_set(INT32U device_id, INT32U size);
INT32S drvl1_sdc_bus_width_set(INT32U device_id, INT8U width);
INT32S drvl1_sdc_stop_controller(INT32U device_id, INT32U timeout);



// SD Controller: APIs for SD controller
INT32S drvl1_sdc_init(INT32U device_id);
INT32S drvl1_sdc_enable(INT32U device_id);
INT32S drvl1_sdc_disable(INT32U device_id);
INT32S drvl1_sdc_controller_busy_wait(INT32U device_id, INT32U timeout);
INT32S drvl1_sdc_clock_set(INT32U device_id, INT32U speed);
INT32S drvl1_sdc_block_len_set(INT32U device_id, INT32U size);
INT32S drvl1_sdc_bus_width_set(INT32U device_id, INT8U width);
INT32S drvl1_sdc_stop_controller(INT32U device_id, INT32U timeout);
void drvl1_sdc_dummyclk_en(INT32U device_id, INT8U en);							//This function only work on GPL326XXB
void drvl1_sdc_ioint_en(INT32U device_id, INT8U en);
void drvl1_sdc_int_en(INT32U device_id, INT32U mask, INT8U en);
INT32S drvl1_sdc_sdio_int_check(INT32U device_id);
void drvl1_sdc_sdio_int_clear(INT32U device_id);
INT32S drvl1_sdc_int_en_check(INT32U device_id, INT32U mask);

// SD Controller: APIs for SD card
extern INT32S drvl1_sdc_card_init_74_cycles(INT32U device_id);
extern INT32S drvl1_sdc_card_busy_wait(INT32U device_id, INT32U timeout);
extern INT32S drvl1_sdc_command_send(INT32U device_id, INT32U command, INT32U argument);
extern INT32S drvl1_sdc_response_get(INT32U device_id, INT32U *response, INT32U timeout);
extern INT32S drvl1_sdc_data_get(INT32U device_id, INT32U *buf, INT32U timeout);
extern INT32S drvl1_sdc_command_complete_wait(INT32U device_id, INT32U timeout);
extern void drvl1_sdc_clear_rx_data_register(INT32U device_id);
extern INT32S drvl1_sdc_read_data_by_dma(INT32U device_id, INT32U *buffer, INT32U sector_count, INT8S *poll_status);
extern INT32S drvl1_sdc_write_data_by_dma(INT32U device_id, INT32U *buffer, INT32U sector_count, INT8S *poll_status);
extern INT32S drvl1_sdc_data_complete_wait(INT32U device_id, INT32U timeout);
extern INT32S drvl1_sdc_data_crc_status_get(INT32U device_id);


#endif		// __DRV_L1_SDC_H__
