
#include "drv_l2_spifc.h"

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#if (defined _DRV_L2_SPIFC) && (_DRV_L2_SPIFC == 1)           	  //
//================================================================//

#if (USE_SPIFC_MANUFACTURER == MANUFACTURER_MXIC)
#include "spi_flash_mxic.h"
#elif (USE_SPIFC_MANUFACTURER == MANUFACTURER_WINBOND)
#include "spi_flash_winbond.h"
#endif
/****************************************************************************/

spifc_apis_ops Spifc_apis_ops;

/****************************************************************************/

/*
 *	spifc_exit_enhance_mode: The device release the enhance mode (continuous read mode) 
 *                            and return to normal SPI operation 	
 */
INT32S spifc_exit_enhance_mode(void)
{
	SPIFC_CMD_HANDLE spifc_cmd_handle;
	INT8U dummyBuf[4];

	gp_memset((INT8S*)&spifc_cmd_handle,0,sizeof(SPIFC_CMD_HANDLE));

	spifc_cmd_handle.spifc_cmd_id = 0xFF;
	spifc_cmd_handle.spifc_cmd_format = CMD_COMMAND_RX;
	spifc_cmd_handle.spifc_txrx_cnt = 4;

	if(spifc_command_send(&spifc_cmd_handle) == STATUS_FAIL)
	{
		goto ERR_EXIT_ENHANCE_MODE;
	}

	if(spifc_rx_receive(dummyBuf,4) == STATUS_FAIL)
	{
		goto ERR_EXIT_ENHANCE_MODE;
	}

	return STATUS_OK;

ERR_EXIT_ENHANCE_MODE:
	return STATUS_FAIL;	
}

/*
 *	spifc_detect: Initialize the SPIFC driver
 */
void spifc_detect(void)
{
	spifc_init();
	spifc_exit_enhance_mode(); // Avoid hang in enhance mode after system reset
}

/*
 *	spifc_write_enable: Set the Write Enable Latch(WER) bit in the Status Register
 */
static INT32S spifc_write_enable(void)
{
	SPIFC_CMD_HANDLE spifc_cmd_handle;
	
	gp_memset((INT8S*)&spifc_cmd_handle,0,sizeof(SPIFC_CMD_HANDLE));

	spifc_cmd_handle.spifc_cmd_id = 0x06;
	spifc_cmd_handle.spifc_cmd_format = CMD_COMMAND_ONLY;
	
	return (spifc_command_send(&spifc_cmd_handle));
}

/*
 *	spifc_status_read: Read the Status Register
 *
 *		pStatus: Pointer to the value read from the Status Register value
 */
INT32S spifc_status_read(INT8U* pStatus)
{
	SPIFC_CMD_HANDLE spifc_cmd_handle;
	INT32U retStatus;
	
	gp_memset((INT8S*)&spifc_cmd_handle,0,sizeof(SPIFC_CMD_HANDLE));
	
	spifc_cmd_handle.spifc_cmd_id = 0x05;
	spifc_cmd_handle.spifc_cmd_format = CMD_COMMAND_RX;
	spifc_cmd_handle.spifc_txrx_cnt = 4;
	
	if(spifc_command_send(&spifc_cmd_handle) == STATUS_FAIL)
	{
		goto ERR_READ_STATUS;
	}
	
	if(spifc_rx_receive((INT8U*)&retStatus,4) == STATUS_FAIL)
	{
		goto ERR_READ_STATUS;
	}

	*pStatus = (retStatus & 0xFF); 

	return STATUS_OK;
	
ERR_READ_STATUS:
	return STATUS_FAIL;
}

/*
 *	spifc_status_write: Write the Status Register
 *  
 *		pStatus: pStatus: Pointer to the value write to the Status Register value
 */
static INT32S spifc_status_write(INT8U* pStatus)
{
	SPIFC_CMD_HANDLE spifc_cmd_handle;
	
	gp_memset((INT8S*)&spifc_cmd_handle,0,sizeof(SPIFC_CMD_HANDLE));
	
	spifc_cmd_handle.spifc_cmd_id = 0x01;
	spifc_cmd_handle.spifc_cmd_format = CMD_COMMAND_TX;
	spifc_cmd_handle.spifc_txrx_cnt = 2;
	
	if(spifc_command_send(&spifc_cmd_handle) == STATUS_FAIL)
	{
		goto ERR_WRITE_STATUS;
	}
	
	if(spifc_tx_send(pStatus,2) == STATUS_FAIL)
	{
		goto ERR_WRITE_STATUS;
	}

	return STATUS_OK;
	
ERR_WRITE_STATUS:
	return STATUS_FAIL;
}

/*
 *	spifc_WREN_flow: Enable WEL(Write Enable Latch)
 *		
 */
INT32S spifc_WREN_flow(void)
{
	INT8U regStatus;

DO_WREN_AGAIN: // TODO:  time-out
	
	if(spifc_write_enable() == STATUS_FAIL)
	{
		goto ERR_WREN;
	}
	
	if(spifc_status_read(&regStatus) == STATUS_FAIL)
	{
		goto ERR_WREN;
	}
	
	/*
	 *	Check bit1[Write Enable] of Status Register
	 *  Bit1: 1:Write Enable 0:Write Disable 
	 */
	if((regStatus & 0x02) != 0x02)
	{
		goto DO_WREN_AGAIN;
	}

	return STATUS_OK;
	
ERR_WREN:	
	return STATUS_FAIL;	
}

/*
 *	spifc_progress_wait: Wait for the device to finish the follow operation:
 *							Program, Erase, Write Status Register
 */
INT32S spifc_progress_wait(void)
{
	INT8U regStatus;

DO_WIP_AGAIN: // TODO:  time-out
	
	// Check bit 0[Write In Progress] of Status Register
	if(spifc_status_read(&regStatus) == STATUS_FAIL)
	{
		goto ERR_WRITE_PROGRESS_CHECK;
	}
	// 1: Write operation(Busy) 0: Not in write operation(Idle)
	if((regStatus & 0x01) == 0x01)
	{	
		goto DO_WIP_AGAIN;
	}

	return STATUS_OK;
	
ERR_WRITE_PROGRESS_CHECK:	
	return STATUS_FAIL;	
}

/*
 *	spifc_WRSR_flow: Write Status Register 
 *
 */
INT32S spifc_WRSR_flow(INT8U ctrlValue,INT8U StatusReg1, INT8U StatusReg2)
{
	INT8U regStatus[2];

	regStatus[0] = StatusReg1;
	regStatus[1] = StatusReg2;

	// Check write enable
	if(spifc_WREN_flow())
	{
		goto ERR_WRSR_FLOW;
	}

	// Write Status Register	
	if(spifc_status_write(regStatus) == STATUS_FAIL)
	{
		goto ERR_WRSR_FLOW;
	}

	// Waiting for the device to finish
	if(spifc_progress_wait())
	{
		goto ERR_WRSR_FLOW;
	}

	// Verify 
	regStatus[0] = 0;
	if(spifc_status_read(&regStatus[0]) == STATUS_FAIL)
	{
		goto ERR_WRSR_FLOW;
	}

	if(regStatus[0] & 0x80) //bit7-> 1: Status register write disable 
	{
		goto ERR_WRSR_FLOW;		
	}

	if(ctrlValue) // Enable Quad Mode
	{
		if(!(regStatus[0] & 0x40))  //bit6-> 1: Quad Enable
		{
			goto ERR_WRSR_FLOW;		
		}
	}
	else // Disable Quad Mode
	{
		if((regStatus[0] & 0x40))  //bit6-> 1: Quad Enable
		{
			goto ERR_WRSR_FLOW;		
		}
	}

	return STATUS_OK;
	
ERR_WRSR_FLOW:
	return STATUS_FAIL;	
}
/****************************************************************************/
/* The follow APIs depend on SPI Flash                                      */
/****************************************************************************/
/*
 *	spifc_get_id:
 *	
 *		pMID: Manufacturer ID
 *		pMemType: Device ID
 *		pMemDensity: Device ID
 */
INT32S spifc_get_id(INT8U* pMID, INT8U* pMemType, INT8U* pMemDensity)
{
	#if (USE_SPIFC_MANUFACTURER == MANUFACTURER_MXIC)	
		return mxic_get_id(pMID,pMemType,pMemDensity);
	#elif (USE_SPIFC_MANUFACTURER == MANUFACTURER_WINBOND)	
		return winbond_get_id(pMID,pMemType,pMemDensity);
	#endif
}

/*
 *	spifc_read:
 *	
 *		rAddrs: 
 *		pRxBuf: 
 *		RxBufLen: 4 bytes alignment 
 *		memAccess: 
 */
INT32S spifc_read(TX_RX_ARGS* pReadArgs)
{
	#if (USE_SPIFC_MANUFACTURER == MANUFACTURER_MXIC)
		return mxic_read(pReadArgs);
	#elif (USE_SPIFC_MANUFACTURER == MANUFACTURER_WINBOND)
		return winbond_read(pReadArgs);
	#endif
}

/*
 *	spifc_program:
 */
INT32S spifc_program(TX_RX_ARGS* pReadArgs)
{
	#if (USE_SPIFC_MANUFACTURER == MANUFACTURER_MXIC)
		return mxic_program(pReadArgs);
	#elif (USE_SPIFC_MANUFACTURER == MANUFACTURER_WINBOND)
		return winbond_program(pReadArgs);
	#endif	
}

/*
 *	spifc_erase:
 *		
 */
INT32S spifc_erase(INT32U Addrs, INT8U eraseArgs)
{
	#if (USE_SPIFC_MANUFACTURER == MANUFACTURER_MXIC)
		return mxic_erase(Addrs,eraseArgs);
	#elif (USE_SPIFC_MANUFACTURER == MANUFACTURER_WINBOND)
		return winbond_erase(Addrs,eraseArgs);
	#endif	
}

/*
 *	spifc_quad_mode_set:
 *		
 */
INT32S spifc_quad_mode_set(INT8U ctrlValue)
{
	#if (USE_SPIFC_MANUFACTURER == MANUFACTURER_MXIC)	
		return mxic_quad_mode_set(ctrlValue);		
	#elif (USE_SPIFC_MANUFACTURER == MANUFACTURER_WINBOND)
		return winbond_quad_mode_set(ctrlValue);
	#endif	
}

/*
 *	spifc_attach:
 *		Modify this api for adding a new function
 */
spifc_apis_ops* spifc_attach(void)
{
	gp_memset((INT8S*)&Spifc_apis_ops,0,sizeof(spifc_apis_ops));

	Spifc_apis_ops.init = spifc_detect;
	Spifc_apis_ops.exit_enhance_mode = spifc_exit_enhance_mode;
	Spifc_apis_ops.get_id = spifc_get_id;
	Spifc_apis_ops.read = spifc_read;
	Spifc_apis_ops.erase = spifc_erase;
	Spifc_apis_ops.program = spifc_program;
	Spifc_apis_ops.set_quad_mode = spifc_quad_mode_set;
	
	return ((spifc_apis_ops*)&Spifc_apis_ops);
}

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#endif //(defined _DRV_L2_SPIFC) && (_DRV_L2_SPIFC == 1)      //
//================================================================//
