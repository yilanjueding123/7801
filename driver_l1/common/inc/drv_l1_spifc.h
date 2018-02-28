#ifndef __DRV_L1_SPIFC_H__
#define __DRV_L1_SPIFC_H__

/****************************************************************************/
#include "project.h"
#include "drv_l1_sfr.h"
#include "driver_l1_cfg.h"
#include "gplib.h"

/****************************************************************************/
#define SPIFC_DISABLE_FALSE			0
#define SPIFC_ENABLE_TRUE			1

#define SPIFC_ADDR_IO_WIDTH_IDX		0 
#define SPIFC_TXRX_IO_WIDTH_IDX		2
#define SPIFC_CMD_IO_WIDTH_IDX		4

#define SPIFC_ADDR_IS_32_BITS		((INT32U)1<<31)

/****************************************************************************/
/*
 *	SPIFC Register
 */
typedef struct 
{										// Offset
	volatile INT32U	CTRL;           	// 0x0000
	volatile INT32U	CMD;  				// 0x0004
	volatile INT32U	PARA;		      	// 0x0008
	volatile INT32U	ADDRL;      		// 0x000C
	volatile INT32U	ADDRH;      		// 0x0010	
	volatile INT32U	TX_WD;      		// 0x0014
	volatile INT32U	RX_RD;      		// 0x0018
	volatile INT32U	TX_BC;      		// 0x001C
	volatile INT32U	RX_BC;      		// 0x0020
	volatile INT32U	TIMING;      		// 0x0024
	volatile INT32U	RESERVE;      		// 0x0028
	volatile INT32U	EXT_CTRL;      		// 0x002C	
}SPIFC_SFR;

typedef enum
{
	CMD_COMMAND_ONLY,
	CMD_COMMAND_TX,
	CMD_COMMAND_RX,
	CMD_COMMAND_ADDRESS,
	CMD_COMMAND_ADDRESS_TX,
	CMD_COMMAND_ADDRESS_RX,
	CMD_COMMAND_ADDRESS_ENHAN_DUMMY_RX,
	CMD_COMMAND_ADDRESS_DUMMY_RX // Without enhance	
}SPIFC_CMD_FORMAT;

typedef struct 
{
	INT32U spifc_cmd_id;  		// Command ID value 
	INT32U spifc_addr;   		// Address value
	INT32U spifc_dummy_cycle;	// Dummy cycle clock
	INT32U spifc_enhance_value; // Enhance value 
	INT32U spifc_cmd_format;    // Refer to  SPIFC_CMD_FORMAT 
	INT32U spifc_txrx_cnt;      // Read or write data count  
	INT32U spifc_mem_access;    // Enable this argument to access spi flash like memory [0x30000000: cache 0xB0000000: non-cache] after sending read command
	INT32U spifc_bit_width;     // Set bit-width for Command , Address, Tx and Rx   
}SPIFC_CMD_HANDLE;

typedef enum
{
	SPIFC_1BIT,
	SPIFC_2BIT,
	SPIFC_4BIT,
	SPIFC_8BIT
}SPIFC_IO_WIDTH;

typedef enum
{
	SPIFC_AUTO,
	SPIFC_MANUAL
}SPIFC_RUN_MODE;

/****************************************************************************/
/*
 *	SPIFC extern APIs
 */
/****************************************************************************/ 
/*
 *	spifc_init:  Initialize the structure of SPIFC_SFR
 */
extern void spifc_init(void);

/*
 *	spifc_ctrl_enable:  Enable or disable the SPI Flash Controller
 *		
 *		ctrlValue: 0: disable 1: enable
 */
extern INT32S spifc_ctrl_enable(INT8U ctrlValue);

/*
 *	spifc_txrx_bit_width_set:  Set bit-width of Tx and Rx
 *		
 *		bitWidth: 0:1bit 1:2bit 2:4bit 3:8bit 
 */
extern INT32S spifc_txrx_bit_width_set(INT8U bitWidth);

/*
 *	spifc_addr_bit_width_set: Set bit-width of Address 
 *		
 *		bitWidth: 0:1bit 1:2bit 2:4bit 3:8bit 
 */
extern INT32S spifc_addr_bit_width_set(INT8U bitWidth);

/*
 *	spifc_cmd_bit_width_set:  Set bit-width of Command
 *		
 *		bitWidth: 0:1bit 1:2bit 2:4bit 3:8bit 
 */
extern INT32S spifc_cmd_bit_width_set(INT8U bitWidth);

/*
 *	spifc_run_mode_set:  The follow operation must be in Manual Mode .
 *						1. Set SPI Flash Controller's Registers
 *						2. Write Tx or read Rx Register by Controller	
 *	
 *		runMode: 0:auto mode 1:manual mode
 */
extern INT32S spifc_run_mode_set(INT8U runMode);

/*
 *	spifc_command_send:  Send command data to SPI Flash  
 *
 *		pSpfi_cmd_handle: Point to structure of SPIFC_CMD_HANDLE
 *
 */
extern INT32S spifc_command_send(SPIFC_CMD_HANDLE* pSpfi_cmd_handle);

/*
 *	spifc_tx_send:  Write data to SPI Flash (Only support chip write)
 *
 *		pTxBuf:  Point to send buffer 
 *		TxBufLen:  buffer length
 */
extern INT32S spifc_tx_send(INT8U* pTxBuf,INT32U TxBufLen);

/*
 *	spifc_rx_receive:  Receive data from SPI Flash 
 *
 *		pRxBuf:  Point to receive buffer
 *		RxBufLen : buffer length (Must be 4 bytes alignment)
 */
extern INT32S spifc_rx_receive(INT8U* pRxBuf,INT32U RxBufLen);

/****************************************************************************/
#endif		// __DRV_L1_SPIFC_H__
/****************************************************************************/
