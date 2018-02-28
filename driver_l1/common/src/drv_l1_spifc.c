
#include "drv_l1_spifc.h"

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#if (defined _DRV_L1_SPIFC) && (_DRV_L1_SPIFC == 1)               //
//================================================================//

/*
 *	REG_SPI
 */
#define SPIFC_CTRL_STATUS_IDLE			(1<<0)
#define SPIFC_TXRX_BITWIDTH_IDX(x)		(x<<2)
#define SPIFC_ADDR_BITWIDTH_IDX(x)		(x<<4)
#define SPIFC_CMD_BITWIDTH_IDX(x)		(x<<6)
#define SPIFC_RUN_MODE_IDX(x)			(x<<8)
#define SPIFC_FIFO_STATUS_EMPTY			(1<<14)

/*
 *	REG_CMD
 */
#define SPIFC_CMD_ONLY		   (1<<9)
#define SPIFC_CMD_ONCE		   (1<<13)

/*
 *	REG_PARA
 */
#define SPIFC_PARA_DUMY_CLK_IDX(x)		(x<<8)
#define SPIFC_PARA_WITHOUT_ADDR			(1<<12)
#define SPIFC_PARA_ADDR_ONLY   			(1<<13)
#define SPIFC_PARA_WITHOUT_ENHAN    	(1<<14)

/*
 *	REG_EXCTRL
 */
#define SPIFC_EXCTRL_ENABLE_EXTEND_ADDRESS  0xFF00

#define TX_IDLE_OK_COUNT	3

/****************************************************************************/
static SPIFC_SFR* pSpifc;

/****************************************************************************/
/*
 *	spifc_init:  Initialize the structure of SPIFC_SFR
 */
void spifc_init(void)
{
	pSpifc = (SPIFC_SFR*)P_SPIFC_BASE;	
}

/*
 *	spifc_ctrl_enable:  Enable or disable the SPI Flash Controller
 *		
 *		ctrlValue: 0: disable 1: enable
 */
INT32S spifc_ctrl_enable(INT8U ctrlValue)
{
	pSpifc->EXT_CTRL &= ~(SPIFC_ENABLE_TRUE); 	

	if(ctrlValue == SPIFC_ENABLE_TRUE)
	{
		pSpifc->EXT_CTRL |= SPIFC_ENABLE_TRUE;
	}
		
	return STATUS_OK;
} 

/*
 *	spifc_txrx_bit_width_set:  Set bit-width of Tx and Rx
 *		
 *		bitWidth: 0:1bit 1:2bit 2:4bit 3:8bit 
 */
INT32S spifc_txrx_bit_width_set(INT8U bitWidth)
{
	pSpifc->CTRL &= ~(SPIFC_TXRX_BITWIDTH_IDX(3)); // Clear bit
	
	pSpifc->CTRL |= SPIFC_TXRX_BITWIDTH_IDX(bitWidth);
		
	return STATUS_OK;
} 

/*
 *	spifc_addr_bit_width_set: Set bit-width of Address 
 *		
 *		bitWidth: 0:1bit 1:2bit 2:4bit 3:8bit 
 */
INT32S spifc_addr_bit_width_set(INT8U bitWidth)
{
	pSpifc->CTRL &= ~(SPIFC_ADDR_BITWIDTH_IDX(3)); // Clear bit
	
	pSpifc->CTRL |= SPIFC_ADDR_BITWIDTH_IDX(bitWidth);
		
	return STATUS_OK;
} 

/*
 *	spifc_cmd_bit_width_set:  Set bit-width of Command
 *		
 *		bitWidth: 0:1bit 1:2bit 2:4bit 3:8bit 
 */
INT32S spifc_cmd_bit_width_set(INT8U bitWidth)
{
	pSpifc->CTRL &= ~(SPIFC_CMD_BITWIDTH_IDX(3)); // Clear bit
	
	pSpifc->CTRL |= SPIFC_CMD_BITWIDTH_IDX(bitWidth);
		
	return STATUS_OK;
} 

/*
 *	spifc_run_mode_set:  The follow operation must be in Manual Mode .
 *						1. Set SPI Flash Controller's Registers
 *						2. Write Tx or read Rx Register by Controller	
 *	
 *		runMode: 0:auto mode 1:manual mode
 */
INT32S spifc_run_mode_set(INT8U runMode)
{
	pSpifc->CTRL &= ~(SPIFC_RUN_MODE_IDX(1)); 	

	if(runMode == SPIFC_MANUAL)
	{
		pSpifc->CTRL |= SPIFC_RUN_MODE_IDX(runMode);
	}
		
	return STATUS_OK;
} 

/*
 *	spifc_ctrl_wait_idle:  Wait for the controller to report that it is idle
 *	
 *		Return:  STATUS_OK: success 
 *				STATUS_FAIL: over time
 */
static INT32S spifc_ctrl_wait_idle(void)
{
	INT8U ok_cnt = 0;

	// TODO: timeout check
	do
	{
		// Check controller status: 0->busy 1->idle
		if((pSpifc->CTRL & SPIFC_CTRL_STATUS_IDLE) == SPIFC_CTRL_STATUS_IDLE)
		{
			/*
			因為硬件問題idle 需要對3次還放行
			SPIFC 跑12MHz 是3次 , 12MHz以下要再增加TX_IDLE_OK_COUNT次數
			*/
			ok_cnt++;
			if(ok_cnt == TX_IDLE_OK_COUNT)
			{
				return STATUS_OK;
			}
		}
	}
	while(1);
	
	return STATUS_FAIL; 
}

/*
 *	spifc_fifo_empty_check:  Check the FIFO is either empty or full
 *		
 *		Return:  STATUS_OK: success 
 *		   	      STATUS_FAIL: over time
 */
static INT32S spifc_fifo_empty_check(void)
{
	// TODO: timeout check
	do
	{
		// Check fifo status: 0->not empty  1->empty
		if((pSpifc->CTRL & SPIFC_FIFO_STATUS_EMPTY) != SPIFC_FIFO_STATUS_EMPTY)
		{
			return STATUS_OK;
		}
	}
	while(1);
	
	return STATUS_FAIL; 
}

/*
 *	spifc_clear_register:  Clear the some register values to zero and default value
 */
static void spifc_clear_register(void)
{
	pSpifc->PARA = 0;
	pSpifc->TX_BC = 0;
	pSpifc->RX_BC = 0;

	// bit-width is 1-bit by default
	pSpifc->CTRL &= ~(SPIFC_TXRX_BITWIDTH_IDX(3)|SPIFC_ADDR_BITWIDTH_IDX(3)|SPIFC_CMD_BITWIDTH_IDX(3)); // Clear bit	

	// Not enable extend address range by default
	pSpifc->EXT_CTRL &= ~(SPIFC_EXCTRL_ENABLE_EXTEND_ADDRESS); 
}

/*
 *	spifc_command_send:  Send command data to SPI Flash 
 *
 *		pSpfi_cmd_handle: Point to structure of SPIFC_CMD_HANDLE
 *
 */
INT32S spifc_command_send(SPIFC_CMD_HANDLE* pSpfi_cmd_handle)
{
	INT32U cmdValue = 0;
		
	// Check controller idle
	if(spifc_ctrl_wait_idle() == STATUS_FAIL)
	{
		goto ERR_CMD_SEND;
	}
	
	// Clear some registers to zero
	spifc_clear_register();
	
	// Change to manual mode to send command
	spifc_run_mode_set(SPIFC_MANUAL);

	// 32-Bit Address
	if(pSpfi_cmd_handle->spifc_cmd_format & SPIFC_ADDR_IS_32_BITS)
	{
		pSpifc->EXT_CTRL |= SPIFC_EXCTRL_ENABLE_EXTEND_ADDRESS; // Enable SPIC's extend address range
		pSpfi_cmd_handle->spifc_cmd_format &= ~(SPIFC_ADDR_IS_32_BITS);
	}
	
	switch(pSpfi_cmd_handle->spifc_cmd_format)
	{
		case CMD_COMMAND_ONLY:
			cmdValue |= SPIFC_CMD_ONLY;
		break;
		
		case CMD_COMMAND_TX:
		case CMD_COMMAND_RX:
			pSpifc->PARA |= SPIFC_PARA_WITHOUT_ENHAN|SPIFC_PARA_WITHOUT_ADDR;
			if(pSpfi_cmd_handle->spifc_cmd_format == CMD_COMMAND_TX)
			{
				pSpifc->TX_BC = pSpfi_cmd_handle->spifc_txrx_cnt;
			}
			else
			{
				pSpifc->RX_BC = pSpfi_cmd_handle->spifc_txrx_cnt;
			}		
		break;
		
		case CMD_COMMAND_ADDRESS:
			pSpifc->PARA |= SPIFC_PARA_ADDR_ONLY;
		break;
		
		case CMD_COMMAND_ADDRESS_TX:
		case CMD_COMMAND_ADDRESS_RX:
			pSpifc->PARA |= SPIFC_PARA_WITHOUT_ENHAN;
			if(pSpfi_cmd_handle->spifc_cmd_format == CMD_COMMAND_ADDRESS_TX)
			{
				pSpifc->TX_BC = pSpfi_cmd_handle->spifc_txrx_cnt;
			}
			else
			{
				pSpifc->RX_BC = pSpfi_cmd_handle->spifc_txrx_cnt;
			}
		break;
		
		case CMD_COMMAND_ADDRESS_ENHAN_DUMMY_RX:
		case CMD_COMMAND_ADDRESS_DUMMY_RX:
			if(pSpfi_cmd_handle->spifc_cmd_format == CMD_COMMAND_ADDRESS_ENHAN_DUMMY_RX)
			{
				cmdValue |= SPIFC_CMD_ONCE;
				pSpifc->PARA |= SPIFC_PARA_DUMY_CLK_IDX(pSpfi_cmd_handle->spifc_dummy_cycle)|pSpfi_cmd_handle->spifc_enhance_value;
			}
			else
			{
				pSpifc->PARA |= SPIFC_PARA_DUMY_CLK_IDX(pSpfi_cmd_handle->spifc_dummy_cycle);
			}
			pSpifc->RX_BC = pSpfi_cmd_handle->spifc_txrx_cnt;
		break;
		
		default:
			goto ERR_CMD_SEND;	
	}
	
	pSpifc->ADDRL = (pSpfi_cmd_handle->spifc_addr & 0x0000FFFF);
	pSpifc->ADDRH = (pSpfi_cmd_handle->spifc_addr & 0xFFFF0000) >> 16;

	//+++ Set bit-width
	if(pSpfi_cmd_handle->spifc_bit_width & (0x03<<SPIFC_ADDR_IO_WIDTH_IDX))
	{
		spifc_addr_bit_width_set((pSpfi_cmd_handle->spifc_bit_width>>SPIFC_ADDR_IO_WIDTH_IDX) & 0x03);		
	}

	if(pSpfi_cmd_handle->spifc_bit_width & (0x03<<SPIFC_TXRX_IO_WIDTH_IDX))
	{
		spifc_txrx_bit_width_set((pSpfi_cmd_handle->spifc_bit_width>>SPIFC_TXRX_IO_WIDTH_IDX) & 0x03);		
	}

	if(pSpfi_cmd_handle->spifc_bit_width & (0x03<<SPIFC_CMD_IO_WIDTH_IDX))
	{
		spifc_cmd_bit_width_set((pSpfi_cmd_handle->spifc_bit_width>>SPIFC_CMD_IO_WIDTH_IDX) & 0x03);		
	}
	//---
	
	// Send command
	cmdValue |= pSpfi_cmd_handle->spifc_cmd_id;
	pSpifc->CMD = cmdValue;

	// The controller automatically sends read command
	if(pSpfi_cmd_handle->spifc_mem_access)
	{	
		spifc_run_mode_set(SPIFC_AUTO);
	}
	
	return STATUS_OK;
	
ERR_CMD_SEND:

	return STATUS_FAIL;
}

/*
 *	spifc_tx_send:  Write data to SPI Flash (Only support chip write)
 *
 *		pTxBuf:  Point to send buffer 
 *		TxBufLen:  buffer length
 */
INT32S spifc_tx_send(INT8U* pTxBuf,INT32U TxBufLen)
{
	INT32U dataIdx = 0;
	
	while(dataIdx < TxBufLen)
	{
		if(spifc_ctrl_wait_idle() == STATUS_FAIL)
		{
			goto ERR_TX_SEND;
		}
		
		pSpifc->TX_WD = *(pTxBuf+dataIdx);
		dataIdx++;		
	}
	
	return STATUS_OK;
	
ERR_TX_SEND:
	return 	STATUS_FAIL;
}

/*
 *	spifc_rx_receive:  Receive data from SPI Flash 
 *
 *		pRxBuf:  Point to receive buffer
 *		RxBufLen : buffer length (Must be 4 bytes alignment)
 */
INT32S spifc_rx_receive(INT8U* pRxBuf,INT32U RxBufLen)
{
	INT32U  dataIdx = 0;
	INT32U* pRxBuf_4Bytes; 
	INT32U  RxBufLen_4Bytes; 
	
	pRxBuf_4Bytes = (INT32U*)pRxBuf;
	RxBufLen_4Bytes = (RxBufLen >> 2);
	
	while(dataIdx < RxBufLen_4Bytes)
	{
		// Check controller idle
		if(spifc_ctrl_wait_idle() == STATUS_FAIL)
		{
			goto ERR_RX_SEND;
		}
		
		// Check fifo empty
		if(spifc_fifo_empty_check() == STATUS_FAIL)
		{
			goto ERR_RX_SEND;
		}
		
		// Must write dummy byte to push data into fifo
		pSpifc->RX_RD = 0;
		
		*pRxBuf_4Bytes++ = pSpifc->RX_RD;		
		dataIdx++;		
	}
	
	return STATUS_OK;
	
ERR_RX_SEND:
	return 	STATUS_FAIL;
}

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#endif //(defined _DRV_L1_SPIFC) && (_DRV_L1_SPIFC == 1)      //
//================================================================//
