#include "spi_flash_mxic.h"

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#if (USE_SPIFC_MANUFACTURER == MANUFACTURER_MXIC)
//================================================================//

/****************************************************************************/
/*
 *	mxic_get_id:
 */
INT32S mxic_get_id(INT8U* pMID, INT8U* pMemType, INT8U* pMemDensity)
{
	SPIFC_CMD_HANDLE spifc_cmd_handle;
	INT8U flashID[4];
	
	gp_memset((INT8S*)&spifc_cmd_handle,0,sizeof(SPIFC_CMD_HANDLE));
	
	spifc_cmd_handle.spifc_cmd_id = 0x9F;	
	spifc_cmd_handle.spifc_cmd_format = CMD_COMMAND_RX;
	
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
 *	mxic_read:
 */
INT32S mxic_read(TX_RX_ARGS* pReadArgs)
{
	SPIFC_CMD_HANDLE spifc_cmd_handle;
	INT8U readViaNIO;
	INT8U memAccess;
	INT8U Addrs_32Bit;
	
	gp_memset((INT8S*)&spifc_cmd_handle,0,sizeof(SPIFC_CMD_HANDLE));

	readViaNIO = pReadArgs->flag_args & 0x03;
	Addrs_32Bit = (pReadArgs->flag_args >> 2) & 0x01;
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
		if(Addrs_32Bit)
		{
			spifc_cmd_handle.spifc_cmd_id = 0x13;
			spifc_cmd_handle.spifc_cmd_format = (SPIFC_ADDR_IS_32_BITS|CMD_COMMAND_ADDRESS_RX);
		}
		else
		{
			spifc_cmd_handle.spifc_cmd_id = 0x03;
			spifc_cmd_handle.spifc_cmd_format = CMD_COMMAND_ADDRESS_RX;
		}
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
 *	mxic_erase_cmd:
 */
static INT32S mxic_erase_cmd(SPI_FLASH_CMD_ARGS* pEraseArgs)
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

		if(pEraseArgs->cmd_id == 0xDC) // 32Bit-Address
		{
			spifc_cmd_handle.spifc_cmd_format |= SPIFC_ADDR_IS_32_BITS;
		}		
	}
	
	return (spifc_command_send(&spifc_cmd_handle));
}

/*
 *	mxic_erase_cmd:
 */
static INT32S mxic_program_cmd(SPI_FLASH_CMD_ARGS* pEraseArgs)
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

	if(pEraseArgs->cmd_id == 0x12) // 32Bit-Address Program
	{
		spifc_cmd_handle.spifc_cmd_format |= SPIFC_ADDR_IS_32_BITS;
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
 *	mxic_security_read:
 */
static INT32S mxic_security_read(INT8U* pSecurity)
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
 *	mxic_program_erase_flow: 
 */
static INT32S mxic_program_erase_flow(INT8U ProgramErase, SPI_FLASH_CMD_ARGS* pProgramEraseArgs)
{
	INT8U regValue;

	// Check write enable
	if(spifc_WREN_flow())
	{
		goto ERR_PROGRAM_ERASE;
	}

	if(ProgramErase)
	{			
		if(mxic_program_cmd(pProgramEraseArgs) == STATUS_FAIL)
		{
			goto ERR_PROGRAM_ERASE;
		}
	}
	else
	{
		if(mxic_erase_cmd(pProgramEraseArgs) == STATUS_FAIL)
		{
			goto ERR_PROGRAM_ERASE;
		}
	}
	
	// Check write progress
	if(spifc_progress_wait())
	{
		goto ERR_PROGRAM_ERASE;
	}

	if(mxic_security_read(&regValue))
	{
		goto ERR_PROGRAM_ERASE;
	}

	if(ProgramErase)
	{
		if((regValue & 0x20)) // bit5 0: program success 1: program fail
		{
			goto ERR_PROGRAM_ERASE;
		}
	}
	else
	{
		if((regValue & 0x40)) // bit6 0: erase success 1: erase fail
		{
			goto ERR_PROGRAM_ERASE;
		}
	}
	
	return STATUS_OK;

ERR_PROGRAM_ERASE:	
	return STATUS_FAIL;	
}

/*
 *	mxic_erase
 */
INT32S mxic_erase(INT32U Addrs, INT8U eraseArgs)
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

		case ERASE_BLOCK_64K_32BIT_ADDRS:
			mxic_cmd_handle.cmd_id = 0xDC;
		break;
				
		case ERASE_WHOLE_CHIP:
			mxic_cmd_handle.cmd_id = 0xC7;
		break;
		
		default:
			return STATUS_FAIL; 			
	}
	
	return (mxic_program_erase_flow(SPIFC_DO_ERASE,&mxic_cmd_handle));	
}

/*
 *	mxic_program:
 */
INT32S mxic_program(TX_RX_ARGS* pProgramArgs)
{
	SPI_FLASH_CMD_ARGS mxic_cmd_handle;
	INT8U programViaNIO;
	INT8U Addrs_32Bit;
	
	gp_memset((INT8S*)&mxic_cmd_handle,0,sizeof(SPI_FLASH_CMD_ARGS));
	gp_memcpy((INT8S*)&(mxic_cmd_handle.tx_rx_args),(INT8S*)pProgramArgs,sizeof(TX_RX_ARGS));

	programViaNIO = pProgramArgs->flag_args & 0x03;
	Addrs_32Bit = (pProgramArgs->flag_args >> 2) & 0x01;
	
	if(programViaNIO == FLAG_READ_PROGRAM_4IO) // 4IO Program
	{
		mxic_cmd_handle.cmd_id = 0x38;
	}
	else if(programViaNIO == FLAG_READ_PROGRAM_2IO) // 2IO Program
	{
		return STATUS_FAIL; // Not support 	
	}
	else // 1IO Program
	{
		if(Addrs_32Bit)
		{
			mxic_cmd_handle.cmd_id = 0x12;
		}
		else
		{
			mxic_cmd_handle.cmd_id = 0x02;
		}
	}

	return (mxic_program_erase_flow(SPIFC_DO_PROGRAM,&mxic_cmd_handle));	
}

/*
 *	mxic_quad_mode_set:
 */
INT32S mxic_quad_mode_set(INT8U ctrlValue) 
{
	INT8U regStatus[2];

	// Write 1 to bit6 of Status register
	if(spifc_status_read(&regStatus[0]) == STATUS_FAIL)
	{
		return STATUS_FAIL; 			
	}

	regStatus[0] &= ~(1<<6); // Clear Bit6
	regStatus[1] = 0;
	
	if(ctrlValue)
	{
		regStatus[0] |= (1<<6);
	}

	
	return spifc_WRSR_flow(ctrlValue,regStatus[0],regStatus[1]);
}
//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#endif //(SPIFC_MANUFACTURER == MANUFACTURER_MXIC)      //
//================================================================//

