#ifndef __drv_l1_I2S_TX_H__
#define __drv_l1_I2S_TX_H__

#include "driver_l1.h"
#include "drv_l1_sfr.h"


#ifdef __cplusplus
extern "C" {
#endif


extern void i2s_tx_init(void);
extern void i2s_tx_exit(void);
extern INT32S i2s_tx_sample_rate_set(INT32U hz);
extern INT32S i2s_tx_mono_ch_set(void);
extern void i2s_tx_fifo_clear(void);
extern INT32S i2s_tx_start(void);
extern INT32S i2s_tx_stop(INT32U status);


#ifdef __cplusplus
}
#endif

#endif		// __drv_l1_I2S_TX_H__