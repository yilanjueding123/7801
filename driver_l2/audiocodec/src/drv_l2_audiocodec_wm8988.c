/*
* Purpose: Wolfson Codec WM8988 driver/interface
*
* Author: josephhsieh
*
* Date: 2014/4/8
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 1.00
* History :
*/

//Include files
#include "drv_l1_i2c.h"
#include "drv_l2_audiocodec_wm8988.h"

// Constant definitions used in this file only go here
#define  WM8988_ID 0x34
typedef union _WM8988
{
	short info;
	char  data[2];
}WM8988;

// Variables defined in this file
static i2c_bus_handle_t i2c_handle; 
static WM8988 pack;

// Global inline functions(Marcos) used in this file only go here
static void i2s_delay(unsigned int num)
{
	#if _OPERATING_SYSTEM != _OS_NONE
		OSTimeDly(2);
	#else
		int i;
		for (i=0; i<num; ++i)
		{	// nonsense , just for delay
			(*((volatile INT32U *) 0xD0500380)) = i;
		}
	#endif
}
 
static short i2c_wolfson_WM8988(int addr, int data)
{
   int hi = addr << (8+1);
   int lo = data;
   int cmd_swap = (hi|lo);
   int cmd = ( (cmd_swap>>8)|(cmd_swap<<8) );

   return (short)cmd;
}

static int i2c_write(i2c_bus_handle_t *handle, INT8U reg, INT8U value)
{
	int ret;
	int i=0;

	for (i=0;i<100;++i)
	{
		ret = reg_1byte_data_1byte_write(handle, reg, value);
		if (ret != -1)
			break;
		i2s_delay(0x4FFFF);
	}

	i2s_delay(0x4FFFF);	

	return ret;
}

int drv_l2_audiocodec_wm8988_init(void)
{
	i2c_handle.slaveAddr = WM8988_ID;
	i2c_handle.clkRate = 20;
	i2c_init();
	
	pack.info = i2c_wolfson_WM8988(15,0x0);
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	i2s_delay(0x1F);
	return 0;
}

int drv_l2_audiocodec_wm8988_tx_init(void)
{
	// TX register	
	pack.info = i2c_wolfson_WM8988(67,0x0);
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	pack.info = i2c_wolfson_WM8988(24,0x0);
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	pack.info = i2c_wolfson_WM8988(25,0xEC);
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	pack.info = i2c_wolfson_WM8988(26,0x1F8);
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	pack.info = i2c_wolfson_WM8988(2,0x179);
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	pack.info = i2c_wolfson_WM8988(3,0x179);
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	pack.info = i2c_wolfson_WM8988(40,0x179);
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	pack.info = i2c_wolfson_WM8988(41,0x179);
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	pack.info = i2c_wolfson_WM8988(5,0x0);
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	pack.info = i2c_wolfson_WM8988(7,0x2);
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	pack.info = i2c_wolfson_WM8988(34,0x150);
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	pack.info = i2c_wolfson_WM8988(35,0x50);
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);		
	pack.info = i2c_wolfson_WM8988(36,0x50);
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);		
	pack.info = i2c_wolfson_WM8988(37,0x150);
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	return 0;
}

int drv_l2_audiocodec_wm8988_rx_init(void)
{
	INT32U R, boost = 0x3, mic_pga_l = 0x20, mic_pga_r = 0x7;
	INT32U alc_max = 0x7, alc_target = 0xB;
	
	// RX Register
	pack.info = i2c_wolfson_WM8988(67,0x0);
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	pack.info = i2c_wolfson_WM8988(24,0x0);
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	pack.info = i2c_wolfson_WM8988(25,0xEC);
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	pack.info = i2c_wolfson_WM8988(26,0x1F8);
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	pack.info = i2c_wolfson_WM8988(7,0x2);
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	pack.info = i2c_wolfson_WM8988(8,0x18);
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);	
	pack.info = i2c_wolfson_WM8988(31,0x100);
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	pack.info = i2c_wolfson_WM8988(32,0xC0|(boost<<4));
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	pack.info = i2c_wolfson_WM8988(33,0xC0|(boost<<4));
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	{
		R = 0x97;
		R &= ~0x3F;
		R &= ~0x80; 		//Disable Mute
		R |= 0x100;
		R |= mic_pga_l; 	
		pack.info = i2c_wolfson_WM8988(0,R);
		i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	}
	{
		R = 0x97;
		R &= ~0x3F;
		R &= ~0x80; 	 //Disable Mute
		R |= 0x100;
		R |= mic_pga_r;
		pack.info = i2c_wolfson_WM8988(1,R);
		i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	}
	{
		R  = 0x7B;
		R &= ~0x7;
		R |= alc_max << 4;
		R |= alc_target;
		//R |= 0x180; //turn on ALC	
		pack.info = i2c_wolfson_WM8988(17,R);
		i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	}
	pack.info = i2c_wolfson_WM8988(21,0x1D0);
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	pack.info = i2c_wolfson_WM8988(22,0x1D0);
	i2c_write(&i2c_handle,pack.data[0] ,pack.data[1]);
	return 0;
}

