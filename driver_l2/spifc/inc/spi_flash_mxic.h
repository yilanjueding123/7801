#ifndef __SPI_FLASH_MXIC_H__
#define __SPI_FLASH_MXIC_H__
/****************************************************************************/
#include "drv_l1_spifc.h"
#include "drv_l2_spifc.h"

/****************************************************************************/

extern INT32S mxic_get_id(INT8U* pMID, INT8U* pMemType, INT8U* pMemDensity);
extern INT32S mxic_read(TX_RX_ARGS* pReadArgs);
extern INT32S mxic_erase(INT32U Addrs, INT8U eraseArgs);
extern INT32S mxic_program(TX_RX_ARGS* pProgramArgs);
extern INT32S mxic_quad_mode_set(INT8U ctrlValue);

/****************************************************************************/
#endif		// __SPI_FLASH_MXIC_H__
/****************************************************************************/

