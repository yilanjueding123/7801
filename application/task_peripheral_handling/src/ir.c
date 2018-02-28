#include "ir.h"
#include "application.h"
#include "customer.h"

//this file only for "红外遥控接收功能 with MK6A12P";
#if C_IR_REMOTE

volatile INT32U g_ir=0;

void ir_init(void)
{
  	gpio_init_io(IR_MK6A12P_CLK, GPIO_INPUT);
  	gpio_set_port_attribute(IR_MK6A12P_CLK, ATTRIBUTE_LOW);
  	gpio_write_io(IR_MK6A12P_CLK, DATA_LOW);

  	gpio_init_io(IR_MK6A12P_DATA, GPIO_INPUT);
  	gpio_set_port_attribute(IR_MK6A12P_DATA, ATTRIBUTE_LOW);
  	gpio_write_io(IR_MK6A12P_DATA, DATA_LOW);
}

void F_128Hz_IR_Service(void)		//这个函数应放在128Hz中断中执行
{
	static INT32U ir_index = 0;
	static INT32U ir_t_data = 0;
	static INT32U ir_t_cnt = 0;
	static INT32U bak_1 = 0;
	static INT32U bak_2 = 0;
	INT8U clk, sda;
	
	clk = gpio_read_io(IR_MK6A12P_CLK);
	
	if (ir_index)
	{//ir_index 长时间不等于零, 这是异常的, 就将它清零
		bak_2++;
		if (ir_index != bak_1)
		{
			bak_1 = ir_index;
			bak_2 = 0;
		}
		else
		{
			if (bak_2 >= 20)	//20/128Hz=156.25ms
			{
				ir_index = 0;
			}
		}
	}
	
	switch (ir_index)
	{
		case 0:
			if (clk)
			{
				sda = gpio_read_io(IR_MK6A12P_DATA);
				if (sda) ir_t_data = 0x01;
				else ir_t_data = 0x00;
				ir_index = 1;
				ir_t_cnt = 0;
			}
			break;
		case 1:
			if (clk==0)
			{
				ir_t_data <<= 1;
				sda = gpio_read_io(IR_MK6A12P_DATA);
				if (sda) ir_t_data |= 0x01;
				ir_index = 2;
				ir_t_cnt += 1;
			}
			break;
		case 2:
			if (clk)
			{
				ir_t_data <<= 1;
				sda = gpio_read_io(IR_MK6A12P_DATA);
				if (sda) ir_t_data |= 0x01;
				ir_index = 1;
				ir_t_cnt += 1;
				if (ir_t_cnt >= 8)
				{
					ir_index = 0;
					g_ir = ir_t_data >> 1;
				}
			}
			break;
		default:
			ir_index = 0;
			break;
	}
}

INT32U g_ir_get(void)
{
	return g_ir;
}


void g_ir_set(INT32U val)
{
	g_ir = val;
}


#endif