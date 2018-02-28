#include "spi_flash_winbond.h"

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#if (USE_SPIFC_MANUFACTURER == MANUFACTURER_WINBOND)
//================================================================//

/****************************************************************************/
static INT32S winbond_status2_read(INT8U* pStatus)
{
	SPIFC_CMD_HANDLE spifc_cmd_handle;
	INT32U retStatus;
	
	gp_memset((INT8S*)&spifc_cmd_handle,0,sizeof(SPIFC_CMD_HANDLE));
	
	spifc_cmd_handle.spifc_cmd_id = 0x35;
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
 *	winbond_get_id:
 */
INT32S winbond_get_id(INT8U* pMID, INT8U* pMemType, INT8U* pMemDensity)
{
	SPIFC_CMD_HANDLE spifc_cmd_handle;
	INT8U flashID[4];
	
	gp_memset((INT8S*)&spifc_cmd_handle,0,sizeof(SPIFC_CMD_HANDLE));
	
	spifc_cmd_handle.spifc_cmd_id = 0x90;	
	spifc_cmd_handle.spifc_cmd_format = CMD_COMMAND_ADDRESS_RX;	
	spifc_cmd_handle.spifc_txrx_cnt = 4;
	
	if(spifc_command_send(&spifc_cmd_handle) == STATUS_FAIL)
	{
		goto ERR_GET_ID;
	}
	
	if(spifc_rx_receive(flashID,4) == STATUS_FAIL)
	{
		goto ERR_GET_ID;
	}

	*pMID = flashID[0];
	*pMemType = flashID[1];
	*pMemDensity = flashID[2];
	
	return STATUS_OK;
	
ERR_GET_ID:
	return STATUS_FAIL;
}

/*
 *	winbond_read:
 */
INT32S winbond_read(TX_RX_ARGS* pReadArgs)
{
	SPIFC_CMD_HANDLE spifc_cmd_handle;
	INT8U readViaNIO;
	INT8U memAccess;
	
	gp_memset((INT8S*)&spifc_cmd_handle,0,sizeof(SPIFC_CMD_HANDLE));

	readViaNIO = pReadArgs->flag_args & 0x03;
	memAccess = (pReadArgs->flag_args >> 3) & 0x01;
	
	spifc_cmd_handle.spifc_addr = pReadArgs->addrs;
	
	if(readViaNIO == FLAG_READ_PROGRAM_4IO) // 4IO Read with enhance mode
	{
		spifc_cmd_handle.spifc_cmd_id = 0xEB;
		spifc_cmd_handle.spifc_dummy_cycle = 4;
		spifc_cmd_handle.spifc_enhance_value = 0xA5;
		spifc_cmd_handle.spifc_cmd_format = CMD_COMMAND_ADDRESS_ENHAN_DUMMY_RX;
		spifc_cmd_handle.spifc_bit_width = (SPIFC_4BIT << SPIFC_ADDR_IO_WIDTH_IDX)|(SPIFC_4BIT << SPIFC_TXRX_IO_WIDTH_IDX);		
	}
	else if(readViaNIO == FLAG_READ_PROGRAM_2IO) // 2IO Read
	{
		spifc_cmd_handle.spifc_cmd_id = 0xBB;
		spifc_cmd_handle.spifc_dummy_cycle = 0;
		spifc_cmd_handle.spifc_cmd_format = CMD_COMMAND_ADDRESS_DUMMY_RX;
		spifc_cmd_handle.spifc_bit_width = (SPIFC_2BIT << SPIFC_ADDR_IO_WIDTH_IDX)|(SPIFC_2BIT << SPIFC_TXRX_IO_WIDTH_IDX);		
	}
	else // 1IO Read
	{
		spifc_cmd_handle.spifc_cmd_id = 0x03;
		spifc_cmd_handle.spifc_cmd_format = CMD_COMMAND_ADDRESS_RX;
	}
	
	spifc_cmd_handle.spifc_txrx_cnt = pReadArgs->buff_len;
	
	if(memAccess) // Access spi flash like memory
	{
		spifc_cmd_handle.spifc_mem_access = SPIFC_ENABLE_TRUE;
		spifc_cmd_handle.spifc_txrx_cnt = 28; // It is hw's default value
	}
	
	if(spifc_command_send(&spifc_cmd_handle) == STATUS_FAIL)
	{
		goto ERR_READ_DATA;
	}

	if(memAccess == SPIFC_DISABLE_FALSE) // Read data into RxBuf
	{
		if(spifc_rx_receive(pReadArgs->buff,pReadArgs->buff_len) == STATUS_FAIL)
		{
			goto ERR_READ_DATA;
		}
	}
	
	return STATUS_OK;
	
ERR_READ_DATA:
	return STATUS_FAIL;
}

/*
 *	winbond_erase_cmd:
 */
static INT32S winbond_erase_cmd(SPI_FLASH_CMD_ARGS* pEraseArgs)
{
	SPIFC_CMD_HANDLE spifc_cmd_handle;
	
	gp_memset((INT8S*)&spifc_cmd_handle,0,sizeof(SPIFC_CMD_HANDLE));
	
	spifc_cmd_handle.spifc_cmd_id = pEraseArgs->cmd_id;
	spifc_cmd_handle.spifc_addr = pEraseArgs->tx_rx_args.addrs;
	
	if(pEraseArgs->cmd_id == 0xC7) // Chip Erase
	{
		spifc_cmd_handle.spifc_cmd_format = CMD_COMMAND_ONLY;
	}
	else // Sector,Block32K,Block64K
	{
		spifc_cmd_handle.spifc_cmd_format = CMD_COMMAND_ADDRESS;
	}
	
	return (spifc_command_send(&spifc_cmd_handle));
}

/*
 *	winbond_erase_cmd:
 */
static INT32S winbond_program_cmd(SPI_FLASH_CMD_ARGS* pEraseArgs)
{
	SPIFC_CMD_HANDLE spifc_cmd_handle;
	
	gp_memset((INT8S*)&spifc_cmd_handle,0,sizeof(SPIFC_CMD_HANDLE));

	spifc_cmd_handle.spifc_cmd_id = pEraseArgs->cmd_id;
	spifc_cmd_handle.spifc_addr = pEraseArgs->tx_rx_args.addrs;
	spifc_cmd_handle.spifc_txrx_cnt = pEraseArgs->tx_rx_args.buff_len;
	spifc_cmd_handle.spifc_cmd_format = CMD_COMMAND_ADDRESS_TX;

	if(pEraseArgs->cmd_id == 0x38) // 4xIO Program
	{
		spifc_cmd_handle.spifc_bit_width = (SPIFC_4BIT << SPIFC_ADDR_IO_WIDTH_IDX)|(SPIFC_4BIT << SPIFC_TXRX_IO_WIDTH_IDX);		
	}
		
	if(spifc_command_send(&spifc_cmd_handle) == STATUS_FAIL)
	{
		goto ERR_PROGRAM_DATA;
	}
	
	if(spifc_tx_send(pEraseArgs->tx_rx_args.buff,pEraseArgs->tx_rx_args.buff_len) == STATUS_FAIL)
	{
		goto ERR_PROGRAM_DATA;
	}
	
	return STATUS_OK;
	
ERR_PROGRAM_DATA:
	return STATUS_FAIL;
}

/*
 *	winbond_security_read:
 */
static INT32S winbond_security_read(INT8U* pSecurity)
{
	SPIFC_CMD_HANDLE spifc_cmd_handle;
	INT32U retSecurity;
	
	gp_memset((INT8S*)&spifc_cmd_handle,0,sizeof(SPIFC_CMD_HANDLE));
	
	spifc_cmd_handle.spifc_cmd_id = 0x2B;
	spifc_cmd_handle.spifc_cmd_format = CMD_COMMAND_RX;
	spifc_cmd_handle.spifc_txrx_cnt = 4;
	
	if(spifc_command_send(&spifc_cmd_handle) == STATUS_FAIL)
	{
		goto ERR_READ_SECURITY;
	}
	
	if(spifc_rx_receive((INT8U*)&retSecurity,4) == STATUS_FAIL)
	{
		goto ERR_READ_SECURITY;
	}

	*pSecurity = (retSecurity & 0xFF); 

	return STATUS_OK;
	
ERR_READ_SECURITY:
	return STATUS_FAIL;
}

/*
 *	winbond_program_erase_flow: 
 */
static INT32S winbond_program_erase_flow(INT8U ProgramErase, SPI_FLASH_CMD_ARGS* pProgramEraseArgs)
{
	INT8U regValue;

	// Check write enable
	if(spifc_WREN_flow())
	{
		goto ERR_PROGRAM_ERASE;
	}

	if(ProgramErase)
	{			
		if(winbond_program_cmd(pProgramEraseArgs) == STATUS_FAIL)
		{
			goto ERR_PROGRAM_ERASE;
		}
	}
	else
	{
		if(winbond_erase_cmd(pProgramEraseArgs) == STATUS_FAIL)
		{
			goto ERR_PROGRAM_ERASE;
		}
	}
	
	// Check write progress
	if(spifc_progress_wait())
	{
		goto ERR_PROGRAM_ERASE;
	}
	
	return STATUS_OK;

ERR_PROGRAM_ERASE:	
	return STATUS_FAIL;	
}

/*
 *	winbond_erase
 */
INT32S winbond_erase(INT32U Addrs, INT8U eraseArgs)
{
	SPI_FLASH_CMD_ARGS mxic_cmd_handle;
	
	gp_memset((INT8S*)&mxic_cmd_handle,0,sizeof(SPI_FLASH_CMD_ARGS));

	mxic_cmd_handle.tx_rx_args.addrs = Addrs;	
	
	switch(eraseArgs)
	{
		case ERASE_SECTOR:
			mxic_cmd_handle.cmd_id = 0x20;
		break;

		case ERASE_BLOCK_32K:
			mxic_cmd_handle.cmd_id = 0x52;
		break;

		case ERASE_BLOCK_64K:
			mxic_cmd_handle.cmd_id = 0xD8;
		break;
				
		case ERASE_WHOLE_CHIP:
			mxic_cmd_handle.cmd_id = 0xC7;
		break;
		
		default:
			return STATUS_FAIL; 			
	}
	
	return (winbond_program_erase_flow(SPIFC_DO_ERASE,&mxic_cmd_handle));	
}

/*
 *	winbond_program:
 */
INT32S winbond_program(TX_RX_ARGS* pProgramArgs)
{
	SPI_FLASH_CMD_ARGS mxic_cmd_handle;
	INT8U programViaNIO;
	
	gp_memset((INT8S*)&mxic_cmd_handle,0,sizeof(SPI_FLASH_CMD_ARGS));
	gp_memcpy((INT8S*)&(mxic_cmd_handle.tx_rx_args),(INT8S*)pProgramArgs,sizeof(TX_RX_ARGS));

	programViaNIO = pProgramArgs->flag_args & 0x03;
	
	if(programViaNIO == FLAG_READ_PROGRAM_4IO) // 4IO Program
	{
		mxic_cmd_handle.cmd_id = 0x32;
	}
	else if(programViaNIO == FLAG_READ_PROGRAM_2IO) // 2IO Program
	{
		return STATUS_FAIL; // Not support 	
	}
	else // 1IO Program
	{
		mxic_cmd_handle.cmd_id = 0x02;
	}

	return (winbond_program_erase_flow(SPIFC_DO_PROGRAM,&mxic_cmd_handle));	
}

/*
 *	winbond_quad_mode_set:
 */
INT32S winbond_quad_mode_set(INT8U ctrlValue) 
{
	INT8U regStatus[2];

	if(spifc_status_read(&regStatus[0]) == STATUS_FAIL)
	{
		return STATUS_FAIL; 			
	}

	if(winbond_status2_read(&regStatus[1]) == STATUS_FAIL)
	{
		return STATUS_FAIL; 			
	}

	regStatus[1] &= ~(1<<1); 
	
	if(ctrlValue)
	{
		regStatus[1] |= (1<<1);
	}
	
	return spifc_WRSR_flow(ctrlValue,regStatus[0],regStatus[1]);
}
//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#endif //(SPIFC_MANUFACTURER == MANUFACTURER_WINBOND)      //
//================================================================//

