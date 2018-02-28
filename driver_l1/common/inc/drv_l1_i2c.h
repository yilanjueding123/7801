#ifndef __drv_l1_I2C_H__
#define __drv_l1_I2C_H__

/****************************************************
*		include file								*
****************************************************/
#include "driver_l1.h"
#include "drv_l1_sfr.h"
/****************************************************
*		constant 									*
****************************************************/
#define I2C_BUS_WRITE	0
#define I2C_BUS_READ	1

#define I2C_ICCR_INIT			0x00
#define I2C_IDEBCLK_INIT		0x04

#define I2C_ICCR_TXCLKMSB_MASK	0x0F
#define I2C_ICCR_INTRPEND		0x10
#define I2C_ICCR_INTREN			0x20
#define I2C_ICCR_ACKEN			0x80

#define I2C_ICSR_NONACK			0x01
#define I2C_ICSR_TXRX_ENABLE	0x10
#define I2C_ICSR_BUSY_STS		0x20	/* when read ICSR[5], 1 means I2C bus busy */
#define I2C_ICSR_START			0x20	/* when write ICSR[5], 1 means START signal generation, 0 means STOP signal generation */
#define I2C_ICSR_MASTER_TX		0xC0
#define I2C_ICSR_MASTER_RX		0x80
#define I2C_ICSR_SLAVE_TX		0x40
#define I2C_ICSR_SLAVE_RX		0x00

#define I2C_MISC_PINMUX_EN			0x01
#define I2C_MISC_ACK_DONTCARE		0x02

/****************************************************
*		data type 									*
*****************************************************/
#ifndef _I2C_BUS_HANDLE_T_
#define _I2C_BUS_HANDLE_T_
typedef struct i2c_bus_handle_s{
	INT16U slaveAddr;			/*!< @brief slave device address*/
	INT16U clkRate;				/*!< @brief i2c bus clock rate */
}i2c_bus_handle_t;
#endif

/****************************************************
*		function
*****************************************************/
extern void i2c_init(void);
extern void i2c_uninit(void);
extern INT32U reg_1byte_data_1byte_write(i2c_bus_handle_t *handle, INT8U reg, INT8U value);
extern INT32U reg_1byte_data_1byte_read(i2c_bus_handle_t *handle, INT8U reg, INT8U *value);
extern INT32U reg_1byte_data_2byte_write(i2c_bus_handle_t *handle, INT8U reg, INT16U value);
extern INT32U reg_1byte_data_2byte_read(i2c_bus_handle_t *handle, INT8U reg, INT16U *value);
extern INT32S i2c_bus_write(i2c_bus_handle_t *handle, INT8U* data, INT8U len);
extern INT32S i2c_bus_read(i2c_bus_handle_t *handle, INT8U* data, INT8U len);
#endif	/*__drv_l1_I2C_H__*/