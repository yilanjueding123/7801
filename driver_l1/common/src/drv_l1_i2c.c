/******************************************************************** 
* Purpose:  I2C driver/interface
* Author: 
* Date: 
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
* Version : 1.00
* History :
*********************************************************************/
#include "drv_l1_i2c.h"
#include "drv_l1_timer.h"
#include "drv_l1_sfr.h"

#define PATCH_FUNC_EN 0

#if PATCH_FUNC_EN == 1
#include "patch_check.h"
#endif

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#if (defined _DRV_L1_I2C) && (_DRV_L1_I2C == 1)					  //
//================================================================//
/****************************************************
*		constant 									*
****************************************************/
/****************************************************
*		marco										*
****************************************************/
#define DBG_I2C_ENABLE		0
#define I2C_TIME_OUT		10	//ms
#define	NO_ACK_TIMEOUT		10	//ms

#if DBG_I2C_ENABLE == 1
#define DRV_I2C_DBG		DBG_PRINT
#else
#define DRV_I2C_DBG(...)
#endif

/****************************************************
*		data type 									*
*****************************************************/

/****************************************************
*		function									*
*****************************************************/
INT32S i2c_bus_xfer(i2c_bus_handle_t *handle, INT8U* data, INT8U len, INT8U cmd);
INT32S i2cStartTran(INT16U slaveAddr, INT16U clkRate, INT8U cmd, INT8U aAck);
INT32S i2cMiddleTran(INT8U* data, INT8U cmd, INT8U aAck);
INT32S i2c_bus_write(i2c_bus_handle_t *handle, INT8U* data, INT8U len);
INT32S i2c_bus_read(i2c_bus_handle_t *handle, INT8U* data, INT8U len);
void i2cStopTran(INT8U cmd);

/****************************************************
*		varaible 									*
*****************************************************/

void drv_tiny_while(INT8U arg1, INT8U arg2)
{
	INT32U i;
	INT32U loopCnt = 0xf; // [200k]0x4FF;  // [100k]0x3FF;
	for(i=0;i<loopCnt;i++);


}

void i2c_init(void)
{
	R_I2C_MISC = 0;
	R_I2C_MISC = I2C_MISC_PINMUX_EN;
	
	R_I2C_ICCR = I2C_ICCR_INIT;
	R_I2C_IDEBCLK = I2C_IDEBCLK_INIT;
}

void i2c_uninit(void)
{
	R_I2C_MISC &= ~I2C_MISC_PINMUX_EN;
}

INT32U reg_1byte_data_1byte_write(
	i2c_bus_handle_t *handle,
	INT8U reg,
	INT8U value
)
{
	INT8U data[2]={0};
	
	data[0] = reg & 0xFF;
	data[1] = value & 0xFF;
	DRV_I2C_DBG("[%s]-- addr=0x%02X, data=0x%02X\r\n", __func__, reg, value);
	
	return i2c_bus_write(handle, data, 2);
}


INT32U reg_1byte_data_1byte_read(
	i2c_bus_handle_t *handle,
	INT8U reg,
	INT8U *value
)
{
	INT32U ret=0;
	INT8U addr[1]={0}, data[1]={0};
	
	addr[0] = reg & 0xFF;
	
	ret = i2c_bus_write(handle, addr, 1);
	ret = i2c_bus_read(handle, data, 1);
	*value = data[0];
	DRV_I2C_DBG("[%s]-- addr=0x%02X, data=0x%02X\r\n", __func__, reg, data[0]);
	
	return ret;
}


INT32U reg_1byte_data_2byte_write(
	i2c_bus_handle_t *handle,
	INT8U reg,
	INT16U value
)
{
	INT8U data[3]={0};
	
	data[0] = reg & 0xFF;
	data[1] = (value >> 8) & 0xFF;
	data[2] = value & 0xFF;
	DRV_I2C_DBG("[%s]-- addr=0x%02X, data=0x%04X\r\n", __func__, reg, value);
	
	return i2c_bus_write(handle, data, 3);
}


INT32U reg_1byte_data_2byte_read(
	i2c_bus_handle_t *handle,
	INT8U reg,
	INT16U *value
)
{
	INT32U ret=0;
	INT8U addr[1]={0}, data[2]={0};
	
	addr[0] = reg & 0xFF;
	
	ret = i2c_bus_write(handle, addr, 1);
	ret = i2c_bus_read(handle, data, 2);
	*value = (((INT16U)data[0]) << 8) | (data[1]);
	DRV_I2C_DBG("[%s]-- addr=0x%02X, data=0x%04X\r\n", __func__, reg, *value);
	
	return ret;
}


INT32U reg_2byte_data_1byte_write(
	i2c_bus_handle_t *handle,
	INT16U reg,
	INT8U value
)
{
	INT8U data[3]={0};
	
	data[0] = (reg>>8) & 0xFF;
	data[1] = reg & 0xFF;
	data[2] = value & 0xFF;
	DRV_I2C_DBG("[%s]-- addr=0x%04X, data=0x%02X\r\n", __func__, reg, value);
	
	return i2c_bus_write(handle, data, 3);
}


INT32U reg_2byte_data_1byte_read(
	i2c_bus_handle_t *handle,
	INT16U reg,
	INT8U *value
)
{
	INT32U ret=0;
	INT8U addr[2]={0}, data[1]={0};
	
	addr[0] = (reg>>8) & 0xFF;
	addr[1] = reg & 0xFF;
	
	ret = i2c_bus_write(handle, addr, 2);
	ret = i2c_bus_read(handle, data, 1);
	*value = data[0];
	DRV_I2C_DBG("[%s]-- addr=0x%04X, data=0x%02X\r\n", __func__, reg, *value);
	
	return ret;
}


/*----------------------------------*/
/*			Local functions			*/
/*----------------------------------*/
static INT32S _i2c_bus_write(i2c_bus_handle_t *handle, INT8U* data, INT8U len)
{
	return i2c_bus_xfer(handle, data, len, (INT8U)I2C_BUS_WRITE);
}

INT32S i2c_bus_write(i2c_bus_handle_t *handle, INT8U* data, INT8U len)
{
#if PATCH_FUNC_EN == 1
	if(PATCH_PTR->i2c_pft != C_NO_PATCH_FUNC && PATCH_PTR->i2c_pft->pfunc_i2c_bus_write) {
		return PATCH_PTR->i2c_pft->pfunc_i2c_bus_write(handle, data, len);
	} else {
		return _i2c_bus_write(handle, data, len);
	}
#else
	return _i2c_bus_write(handle, data, len);
#endif
}

static INT32S _i2c_bus_read(i2c_bus_handle_t *handle, INT8U* data, INT8U len)
{
	return i2c_bus_xfer(handle, data, len, (INT8U)I2C_BUS_READ);
}

INT32S i2c_bus_read(i2c_bus_handle_t *handle, INT8U* data, INT8U len)
{
#if PATCH_FUNC_EN == 1
	if(PATCH_PTR->i2c_pft != C_NO_PATCH_FUNC && PATCH_PTR->i2c_pft->pfunc_i2c_bus_read) {
		return PATCH_PTR->i2c_pft->pfunc_i2c_bus_read(handle, data, len);
	} else {
		return _i2c_bus_read(handle, data, len);
	}
#else 
	return _i2c_bus_read(handle, data, len);
#endif
}

static INT32S _i2c_bus_xfer(i2c_bus_handle_t *hd, INT8U* data, INT8U len, INT8U cmd)
{
	INT32S i, ret = 0;
	
	if (hd->clkRate == 0)
	{
		DRV_I2C_DBG("[%s]-- Error: i2c clock rate must to be more than zero\r\n", __func__);
		return -1;
	}
	
	if (cmd == I2C_BUS_WRITE)
	{
		ret = i2cStartTran(hd->slaveAddr, hd->clkRate, I2C_BUS_WRITE, 0);
		// [TODO] OID sensor should wait for ACK. Why the code is no ACK?
		//ret = i2cStartTran(hd->slaveAddr, hd->clkRate, I2C_BUS_WRITE, 1);
		if (ret < 0)
		{
			DRV_I2C_DBG("WRITE-Error: write slave device address fail\r\n", __func__);
			return -1;
		}
		
		for (i = 0; i < len; i++)
		{
			// [TODO] OID sensor should wait for ACK. Why the code is no ACK for last byte?
			if (i==(len-1))
				ret = i2cMiddleTran((data+i), I2C_BUS_WRITE, 0);
			else
				ret = i2cMiddleTran((data+i), I2C_BUS_WRITE, 1);

			if (ret < 0)
			{
				DRV_I2C_DBG("WRITE-Error: write data fail\r\n", __func__);
				return -1;
			}
		}
		
		i2cStopTran(I2C_BUS_WRITE);
	}
	else if (cmd == I2C_BUS_READ)
	{
		ret = i2cStartTran(hd->slaveAddr, hd->clkRate, I2C_BUS_READ, 1);
		if (ret < 0)
		{
			DRV_I2C_DBG("READ-Error: write slave device address fail\r\n", __func__);
			return -1;
		}
		
		for (i = 0; i < len; i++)
		{
			if (i == (len-1))
				ret = i2cMiddleTran((data+i), I2C_BUS_READ, 0);
			else
				ret = i2cMiddleTran((data+i), I2C_BUS_READ, 1);
				
			if (ret < 0)
			{
				DRV_I2C_DBG("READ-Error: read data fail\r\n", __func__);
				return -1;
			}
		}
		
		i2cStopTran(I2C_BUS_READ);
	}
	
	ret = i;
	return ret;
}

INT32S i2c_bus_xfer(i2c_bus_handle_t *hd, INT8U* data, INT8U len, INT8U cmd)
{
#if PATCH_FUNC_EN == 1
	if(PATCH_PTR->i2c_pft != C_NO_PATCH_FUNC && PATCH_PTR->i2c_pft->pfunc_i2c_bus_read) {
		return PATCH_PTR->i2c_pft->pfunc_i2c_bus_xfer(hd, data, len, cmd);
	} else {
		return _i2c_bus_xfer(hd, data, len, cmd);
	}
#else
	return _i2c_bus_xfer(hd, data, len, cmd);
#endif
}

void i2cSetClkRate(INT16U clkRate)
{
	//==================================================================================
	// I2C_Clock = SYSCLK /( { ( TXCLKPRE[3:0], TXCLKLSB[7:0] ) + 1 } * 2)
	// ( TXCLKPRE[3:0], TXCLKLSB[7:0] ) means [4 + 8 = 12] bits combined as following
	//
	// -------------------------------------------------
	// | TXCLKPRE[3:0] |         TXCLKLSB[7:0]         |
	// -------------------------------------------------
	// | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
	// -------------------------------------------------
	//
	// ( TXCLKPRE[3:0], TXCLKLSB[7:0] ) = ( SYSCLK / 2 / I2C_Clock ) - 1
	//==================================================================================
	INT32U tmp=0;
	
	tmp = ( (48*1000000) / (2*clkRate*1000) );

	if (tmp > 0xFFF)
		tmp = 0xFFF;
	else if (tmp > 0)
		tmp--;
	
	// Setup TXCLKLSB[7:0]
	R_I2C_TXCLKLSB = (tmp & 0xFF);
	
	// Setup TXCLKPRE[3:0] in ICCR
	R_I2C_ICCR &= ~0xF;
	R_I2C_ICCR |= ((tmp & 0xF00) >> 8);
	
}


INT32S I2C_INTPEND_WAITING(INT32S ms)
{
	INT32S timeout;
	timeout = ms * 75;
	while((R_I2C_ICCR & I2C_ICCR_INTRPEND) == 0) {
		timeout--;
		if(timeout < 0) {
			break;
		}
		drv_tiny_while(0,5);
//		drv_msec_wait(1);
	}

	return ms;
}

INT32S I2C_BUSY_WAITING(INT32S ms)
{
	INT32S timeout;
	timeout = ms * 75;
	while((R_I2C_ICSR & I2C_ICSR_BUSY_STS)) {
		timeout--;
		if(timeout < 0) {
			break;
		}
		drv_tiny_while(0,5);
//		drv_msec_wait(1);
	}
	
	return ms;
}

INT32S i2cStartTran(INT16U slaveAddr, INT16U clkRate, INT8U cmd, INT8U aAck)
{
	INT32S ret = 1;
	INT32U iccr = 0, ctrl = 0;
	INT32S timeout;

	/* check i2c bus is idle or not */
	ret = I2C_BUSY_WAITING(I2C_TIME_OUT);
	if (ret < 0) {
		DRV_I2C_DBG("[%s]-- I2C bus is busy\r\n", __func__);
		return -1;
	}

	i2cSetClkRate(clkRate);
	
	iccr = R_I2C_ICCR;
	//iccr = (iccr & I2C_ICCR_TXCLKMSB_MASK) | I2C_ICCR_INTREN;
	// [TODO]
	// TXCLKMSB_MASK was set in i2cSetClkRate() and I2C does not use interrupt.
	// The following line should be un-necessary.
	//												by Craig 2012.6.28
	iccr &= I2C_ICCR_TXCLKMSB_MASK | I2C_ICCR_INTREN;

	switch (cmd)
	{
		case I2C_BUS_WRITE:
			iccr |= I2C_ICCR_INIT;
			ctrl = I2C_ICSR_MASTER_TX | I2C_ICSR_TXRX_ENABLE;
			break;

		case I2C_BUS_READ:
			iccr |= I2C_ICCR_ACKEN;
			ctrl = I2C_ICSR_MASTER_RX | I2C_ICSR_TXRX_ENABLE;
			break;

		default:
			return -1;
			break;
	}
	
	R_I2C_ICCR = iccr;
	R_I2C_ICSR = ctrl;
	
	if (cmd == I2C_BUS_READ) {
		R_I2C_IDSR = (slaveAddr & 0xFF) | 0x01;
	} else {
		R_I2C_IDSR = slaveAddr & 0xFE;
	}
	
	DRV_I2C_DBG("[%s]-- I2C Start to Send Start Signal\r\n", __func__);
	R_I2C_ICSR |= I2C_ICSR_START;
	
	ret = I2C_INTPEND_WAITING(I2C_TIME_OUT);
	if (ret < 0) {
		DRV_I2C_DBG("[%s]-- Interrupt is not received\r\n", __func__);
		return -1;
	}
	
	timeout = NO_ACK_TIMEOUT * 75;
	while ( (R_I2C_ICSR & I2C_ICSR_NONACK ) && aAck ) {
		DRV_I2C_DBG("[%s]-- Waiting for ACK\r\n", __func__);
		timeout--;
		if(timeout < 0) {
			DRV_I2C_DBG("[%s]-- Waiting ACK timeout\r\n", __func__);
			ret = -1;
			break;
		}
		drv_tiny_while(0,5);
	}
	return ret;
}

INT32S i2cMiddleTran(INT8U *data, INT8U cmd, INT8U aAck)
{
	INT32S ret = 1;
	INT32U iccr = 0;
	INT32S timeout;
	
	iccr = R_I2C_ICCR;
	iccr &= I2C_ICCR_TXCLKMSB_MASK;
	
	switch (cmd)
	{
		case I2C_BUS_WRITE:
			R_I2C_IDSR = *data & 0xFF;
			iccr |= I2C_ICCR_INTRPEND;
			break;
		
		case I2C_BUS_READ:
			if (aAck == 1)
				iccr |= I2C_ICCR_ACKEN | I2C_ICCR_INTRPEND;
			else
				iccr |= I2C_ICCR_INTRPEND;
			break;
		
		default:
			return -1;
			break;
	}
	
	/* clear irq */
	R_I2C_ICCR = iccr;

	// Interrupt Pending
	ret = I2C_INTPEND_WAITING(I2C_TIME_OUT);
	if (ret < 0) {
		DRV_I2C_DBG("[%s]-- Interrupt is not received\r\n", __func__);
		return -1;
	}

	timeout = NO_ACK_TIMEOUT * 75;
	// Ack Received ?
	while ( (R_I2C_ICSR & I2C_ICSR_NONACK ) && aAck ) {
		DRV_I2C_DBG("[%s]-- Waiting for ACK\r\n", __func__);
		timeout--;
		if(timeout < 0) {
			DRV_I2C_DBG("[%s]-- Waiting ACK timeout\r\n", __func__);
			ret = -1;
			break;
		}
		drv_tiny_while(0,5);
	}

	*data = (R_I2C_IDSR & 0xFF);
	return ret;
}


void i2cStopTran(INT8U cmd)
{
	INT32S ret=0;
	INT32U ctrl = 0;
	
	if (cmd == I2C_BUS_WRITE)
		ctrl = I2C_ICSR_MASTER_TX | I2C_ICSR_TXRX_ENABLE;
	else
		ctrl = I2C_ICSR_MASTER_RX | I2C_ICSR_TXRX_ENABLE;
		
	R_I2C_ICSR = ctrl;

	DRV_I2C_DBG("[%s]-- Stop transmition\r\n", __func__);
	ret = I2C_BUSY_WAITING(I2C_TIME_OUT);
	if (ret < 0) {
		DRV_I2C_DBG("[%s]-- I2C bus is busy\r\n", __func__);
		return;
	}
}

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#endif //(defined _DRV_L1_I2C) && (_DRV_L1_I2C == 1)
