#ifndef __DRV_L2_SPIFC_H__
#define __DRV_L2_SPIFC_H__

/****************************************************************************/
#include "project.h"
#include "drv_l1_spifc.h"
#include "driver_l2_cfg.h"

/****************************************************************************/
#define SPIFC_DO_ERASE  	0
#define SPIFC_DO_PROGRAM	1

/****************************************************************************/
#define	MANUFACTURER_MXIC		0 
#define	MANUFACTURER_WINBOND	1

#define USE_SPIFC_MANUFACTURER MANUFACTURER_MXIC

/****************************************************************************/
typedef enum
{
	ERASE_SECTOR,
	ERASE_BLOCK_32K,
	ERASE_BLOCK_64K,
	ERASE_BLOCK_64K_32BIT_ADDRS,
	ERASE_WHOLE_CHIP,
	ERASE_MAX
}SPIFC_ERASE_ARGS;

/****************************************************************************/
/*
 * flag_args: Bit[1~0]: Read/Program nxIO // Defalut => 1xIO                               
 *			  Bit[2]: Address is 32Bits 
 *			  Bit[3]: Mem Access
 */
#define FLAG_READ_PROGRAM_1IO 0
#define FLAG_READ_PROGRAM_2IO 1
#define FLAG_READ_PROGRAM_4IO 2
#define FLAG_32BIT_ADDRESS	1
#define FLAG_MEM_ACCESS	1

/****************************************************************************/
typedef struct {
	INT32U  addrs;
	INT8U*  buff;
	INT32U 	buff_len;
	INT32U 	flag_args; 
}TX_RX_ARGS;

typedef struct {
	INT32U  cmd_id;
	TX_RX_ARGS tx_rx_args;
}SPI_FLASH_CMD_ARGS;

/****************************************************************************/

typedef struct {
	void   (*init)(void);
	INT32S (*get_id)(INT8U* pMID, INT8U* pMemType, INT8U* pMemDensity);
	INT32S (*read)(TX_RX_ARGS* pReadArgs); 
	INT32S (*erase)(INT32U Addrs, INT8U eraseArgs);
	INT32S (*program)(TX_RX_ARGS* pProgramArgs);
	INT32S (*set_quad_mode)(INT8U ctrlValue);	
	INT32S (*exit_enhance_mode)(void);
}spifc_apis_ops;

/****************************************************************************/
/*
 *	SPIFC extern APIs
 */
/****************************************************************************/
/*
 *	spifc_attach 	
 */
extern  spifc_apis_ops* spifc_attach(void);

extern INT32S spifc_status_read(INT8U* pStatus);
extern INT32S spifc_WREN_flow(void);
extern INT32S spifc_progress_wait(void);
extern INT32S spifc_WRSR_flow(INT8U ctrlValue, INT8U StatusReg1, INT8U StatusReg2);

/****************************************************************************/
#endif		// __DRV_L2_SPIFC_H__
/****************************************************************************/

