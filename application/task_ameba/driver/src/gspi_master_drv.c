/************************************************************
* gspi_master_drv.c
*
* Purpose: GSPI master driver
*
* Author: Eugenehsu
*
* Date: 2015/08/27
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 1.10
* History :
*
************************************************************/
#include <string.h>
#include "gplib.h"
#include "application.h"
#include "drv_l1_gpio.h"
#include "drv_l1_spi.h"
#include "gspi_master_drv.h"

#define SPI_LOCAL_DOMAIN 				0x0
#define SPI_TXFIFO_DOMAIN 				0xc
#define SPI_RXFIFO_DOMAIN 				0x1f

//IO Bus domain address mapping
#define DEFUALT_OFFSET					0x0
#define SPI_LOCAL_OFFSET    			0x10250000
#define SPI_TX_FIFO_OFFSET	    		0x10310000
#define SPI_RX_FIFO_OFFSET	    		0x10340000

#define SPI_LOCAL_DEVICE_ID				0
#define SPI_TXQ_FIFO_DEVICE_ID			3
#define SPI_RXQ_FIFO_DEVICE_ID			7
#define SPI_UNDEFINED_DEVICE_ID			(-1)

//SPI Local registers
#define SPI_REG_INT_CTRL				0x0004 // 4 bytes, SPI INT Control
#define SPI_REG_INT_TIMEOUT		   		0x0006  // 2 bytes, SPI 32us INT timout
#define SPI_REG_HIMR					0x0014 // 4 bytes, SPI Host Interrupt Mask
#define SPI_REG_HISR					0x0018 // 4 bytes, SPI Host Interrupt Service Routine
#define SPI_REG_RX0_REQ_LEN				0x001C // 4 bytes, RXDMA Request Length
#define SPI_REG_FREE_TX_SPACE			0x0020 // 4 bytes, Free Tx Buffer Page
#define SPI_REG_TX_SEQNUM				0x0024 // 1 byte, TX Sequence Number Definition
#define SPI_REG_HCPWM					0x0038 // 1 byte, HCI Current Power Mode
#define SPI_REG_HCPWM2			 		0x003A // 2 bytes, HCI Current Power Mode 2
#define SPI_REG_AVAI_PATH_L				0x0040 // 4 bytes, SPI TX Available Low Size reg
#define SPI_REG_AVAI_PATH_H				0x0044 // 4 bytes, SPI TX Available High Size reg
#define SPI_REG_RX_AGG_CTL				0x0048 // 4 bytes, SPI RX AGG control
#define SPI_REG_H2C_MSG					0x004C // 4 bytes, SPI_REG_H2C_MSG
#define SPI_REG_C2H_MSG					0x0050  // 4 bytes, SPI_REG_C2H_MSG
#define SPI_REG_HRPWM					0x0080  // 1 byte, SPI_REG_HRPWM
#define SPI_REG_HPS_CLKR				0x0084 // 1 byte, not uesd
#define SPI_REG_CPU_IND					0x0087 // 1 byte, firmware indication to host
#define SPI_REG_32K_TRANS_CTL			0x0088 // 1 byte, 32K transparent control, BIT0 EN32K_TRANS
#define SPI_REG_32K_IDLE_TIME			0x008B // 1 byte, 32K idle time, 
#define SPI_REG_DELY_LINE_SEL			0x008C // 1 byte, Delay line selection, 
#define SPI_REG_SPI_CFG					0x00F0 // 1 byte, SPI configuration, 

#define LOCAL_REG_FREE_TX_SPACE			(SPI_LOCAL_OFFSET | SPI_REG_FREE_TX_SPACE)

// Register SPI_REG_CPU_IND
#define SPI_CPU_RDY_IND		(BIT0)

/************************************************/
// SPI_REG_HISR: SDIO Host Interrupt Service Routine
#define SPI_HISR_RX_REQUEST				(BIT0)
#define SPI_HISR_AVAL_INT				(BIT1)
#define SPI_HISR_TXPKT_OVER_BUFF		(BIT2)
#define SPI_HISR_TX_AGG_SIZE_MISMATCH	(BIT3)
#define SPI_HISR_TXBD_OVF				(BIT4)
//BIT5~16 not used
#define SPI_HISR_C2H_MSG_INT			(BIT17)
#define SPI_HISR_CPWM1_INT				(BIT18)
#define SPI_HISR_CPWM2_INT				(BIT19)
//BIT20~31 not used
#define SPI_HISR_CPU_NOT_RDY			(BIT22)


#define MASK_SPI_HISR_CLEAR		(SPI_HISR_RX_REQUEST |\
						SPI_HISR_AVAL_INT |\
						SPI_HISR_TXPKT_OVER_BUFF |\
						SPI_HISR_TX_AGG_SIZE_MISMATCH |\
						SPI_HISR_TXBD_OVF |\
						SPI_HISR_C2H_MSG_INT |\
						SPI_HISR_CPWM1_INT |\
						SPI_HISR_CPWM2_INT)

// RTL8195A SPI Host Interrupt Mask Register
#define SPI_HIMR_RX_REQUEST_MSK				(BIT0)
#define SPI_HIMR_AVAL_MSK					(BIT1)
#define SPI_HIMR_TXPKT_SIZE_OVER_BUFF_MSK	(BIT2)
#define SPI_HIMR_AGG_SIZE_MISMATCH_MSK		(BIT3)
#define SPI_HIMR_TXBD_OVF_MSK				(BIT4)
//BIT5~16 not used
#define SPI_HIMR_C2H_MSG_INT_MSK			(BIT17)
#define SPI_HIMR_CPWM1_INT_MSK				(BIT18)
#define SPI_HIMR_CPWM2_INT_MSK				(BIT19)
//BIT20~31 not used
#define SPI_HIMR_DISABLED			0

// Register SPI_REG_HCPWM
#define SPI_HCPWM_WLAN_TRX			(BIT1)

#define GSPI_CMD_LEN		4
#define GSPI_STATUS_LEN		8

#define FILL_SPI_CMD(byte_en, addr, domain_id, fun, write_flag) 	((byte_en & 0xff) | ((addr & 0xffff) << 8) \
									| ((domain_id & 0x1f) << 24) | ((fun & 0x3) << 29) | ((write_flag & 0x1) << 31))

#define GET_STATUS_HISR(status)			((((*(INT32U*)status)) & 0x3) |((((*(INT32U*)status) >> 2) & 0x7) << 17))
#define GET_STATUS_FREE_TX(status)		((((*(INT32U*)status) >> 5) & 0x7ffffff) << 2)
#define GET_STATUS_RXQ_REQ_LEN(status)	(((*(INT32U*)((INT8U *)status + 4))) & 0xffffff)
#define GET_STATUS_TX_SEQ(status)		(((*(INT32U)((INT8U *)status + 4)) >> 24) & 0xff)

#define GSPI_CMD_TX         0x83
#define GSPI_CMD_RX			0X82

// GSPI configuration (big endian recommended)
#define GSPI_CONFIG SPI_BIG_ENDIAN_16

#define TwoByteSwap(_twobyte)	(_twobyte)

typedef struct _GSPI_RX_DESC{
	// u4Byte 0
	INT32U	pkt_len:16;     // bit[15:0], the packet size
	INT32U	offset:8;    	// bit[23:16], the offset from the packet start to the buf start, also means the size of RX Desc
	INT32U	rsvd0:6;        // bit[29:24]
	INT32U	icv:1;          // bit[30], ICV error
	INT32U	crc:1;          // bit[31], CRC error

	// u4Byte 1
	INT32U	type:8;         // bit[7:0], the type of this packet
	INT32U	rsvd1:24;       // bit[31:8]

	// u4Byte 2
	INT32U	rsvd2;
	
	// u4Byte 3
	INT32U	rsvd3;
	
	// u4Byte 4
	INT32U	rsvd4;

	// u4Byte 5
	INT32U	rsvd5;
} GSPI_RX_DESC, *PGSPI_RX_DESC;

typedef struct _GSPI_TX_DESC{
	// u4Byte 0
	INT32U	txpktsize:16;       // bit[15:0]
	INT32U	offset:8;    		// bit[23:16], store the sizeof(SDIO_TX_DESC)
	INT32U	bus_agg_num:8;		// bit[31:24], the bus aggregation number

	// u4Byte 1
    INT32U type:8;             // bit[7:0], the packet type
    INT32U rsvd0:24;

	// u4Byte 2
	INT32U	rsvd1;
	
	// u4Byte 3
	INT32U	rsvd2;
	
	// u4Byte 4
	INT32U	rsvd3;

	// u4Byte 5
	INT32U	rsvd4;
} GSPI_TX_DESC, *PGSPI_TX_DESC;

#define SIZE_TX_DESC	(sizeof(GSPI_TX_DESC))
#define SIZE_RX_DESC	(sizeof(GSPI_RX_DESC))
/**********************************************
*	Extern variables and functions
**********************************************/

/*********************************************
*	Variables declaration
*********************************************/
static INT32U GSPIINTTaskStack[GSPI_INT_STACK_SIZE];
static GSPI_RX_DATA_CBK gspi_rx_cbk = NULL;
static OS_EVENT *gspi_master_sem;
static OS_EVENT *gspi_spi_sem;

static INT8U rx_raw_buf[BUFFER_LEN];
static INT8U tx_raw_buf[BUFFER_LEN];	/* Buffer to transfer descriptor and data */
void gspi_rx_task(void *parm);
/*****************************************************
    Utility functions
******************************************************/
void gspi_master_lock(void)
{
	INT8U err;
	
	OSSemPend(gspi_master_sem, 0, &err);
}

void gspi_master_unlock(void)
{
	OSSemPost(gspi_master_sem);
}

void gspi_spi_lock(void)
{
	INT8U err;
	
	OSSemPend(gspi_spi_sem, 0, &err);
}

void gspi_spi_unlock(void)
{
	OSSemPost(gspi_spi_sem);
}

static void gspi_master_cs_init(void)
{
	spi_cs_by_external_set(SPI_MASTER_NUM);
	R_GPIOCTRL |= (0x01<<3); // CS Pin[IOD6] change to GPIO Function   

	gpio_init_io(GSPI_MASTER_CS_PIN, GPIO_OUTPUT);
	gpio_set_port_attribute(GSPI_MASTER_CS_PIN, 1);
	gpio_write_io(GSPI_MASTER_CS_PIN, 1);
}

void gspi_master_cs(INT32U high)
{
	if(high)
		gpio_write_io(GSPI_MASTER_CS_PIN, 1);
	else
		gpio_write_io(GSPI_MASTER_CS_PIN, 0);
}

static void gspi_ext_isr(void)
{
	//DBG_PRINT("gspi_ext_isr\r\n");
	/* Release a semaphore */
	gspi_master_unlock();
}

static void gspi_ext_interrpt_init(void)
{
	// Use EXTC(IOB10) for SPI INT
	gpio_init_io(IO_B10 ,GPIO_INPUT);
  	gpio_set_port_attribute(IO_B10, ATTRIBUTE_LOW);
  	gpio_write_io(IO_B10, DATA_HIGH);

	extab_int_clr(EXTC);                    //if EXTA happen ,clear the interrupt flag
	extab_edge_set(EXTC, FALLING);                //falling edge
	extab_enable_set(EXTC, TRUE);           //re-enable interrupt
    extab_user_isr_set(EXTC, gspi_ext_isr); //register exta interrupt isr
}

void gspi_register_rx_cbk(void* cbk)
{
	gspi_rx_cbk = (GSPI_RX_DATA_CBK)cbk;
}

void gspi_config_slave_device(void)
{
	INT32U gspi_himr = 0;
	INT8S res = 0;

try_again:	
	//1 GSPI slave configuration
	res = gspi_configuration(GSPI_CONFIG);
	if(res)
	{
		DBG_PRINT("gspi configure error....\n");
		/* Delay 3 seconds for try to configuration again */
		OSTimeDly(300);
		goto try_again;
	}
	// HISR write one to clear
	gspi_write32(SPI_LOCAL_OFFSET |SPI_REG_HISR, 0xFFFFFFFF, NULL);
		
	// HIMR - turn all off
	gspi_write32(SPI_LOCAL_OFFSET |SPI_REG_HIMR, SPI_HIMR_DISABLED, NULL);

	// set intterrupt mask 
	gspi_himr = (INT32U)(SPI_HISR_RX_REQUEST|SPI_HISR_CPWM1_INT);

	// enable interrupt
	gspi_write32(SPI_LOCAL_OFFSET | SPI_REG_HIMR, gspi_himr, NULL);
}	

INT32S gspi_master_init(void)
{
	INT8U err;
	
	spi_init(SPI_MASTER_NUM);

	spi_clk_set(SPI_MASTER_NUM, GSPI_MASTER_CLK);

	gspi_ext_interrpt_init();
	/* Init chip selection PIN */
	gspi_master_cs_init();

    gspi_master_sem = OSSemCreate(0);
  
  	gspi_spi_sem = OSSemCreate(1);
  	
  	/* Configure the GSPI slave device to start the GSPI communication */
  	gspi_config_slave_device();
  	/* Create GSPI master interrupt task */
	err = OSTaskCreate(gspi_rx_task, (void *)NULL, &GSPIINTTaskStack[GSPI_INT_STACK_SIZE - 1], GSPI_INT_TASK_PRIORITY);	
	if(err != OS_NO_ERR)
	{ 
		DBG_PRINT("Cannot create GSPI INTERRUPT task\n\r");
		return STATUS_FAIL;
	}	
  	
	return STATUS_OK;
}

static int addr_convert(INT32U addr)
{
	INT32U domain_id = 0 ;
	INT32U temp_addr = addr&0xffff0000;

	switch (temp_addr) {
	case SPI_LOCAL_OFFSET:
		domain_id = SPI_LOCAL_DOMAIN;
		break;
	case SPI_TX_FIFO_OFFSET:
		domain_id = SPI_TXFIFO_DOMAIN;
		break;
	case SPI_RX_FIFO_OFFSET:
		domain_id = SPI_RXFIFO_DOMAIN;
		break;
	default:
		break;
	}

	return domain_id;
}

static INT32U DWORD_endian_reverse(INT32U src, _gspi_conf_t gspi_conf)
{
	switch(gspi_conf)
	{
		case SPI_LITTLE_ENDIAN_16:
			return (((src&0x000000ff)<<8)|((src&0x0000ff00)>>8)|
		((src&0x00ff0000)<<8)|((src&0xff000000)>>8));
		case SPI_LITTLE_ENDIAN_32:
			return (((src&0x000000ff)<<24)|((src&0x0000ff00)<<8)|
		((src&0x00ff0000)>>8)|((src&0xff000000)>>24));
		case SPI_BIG_ENDIAN_16:
			return src;
		case SPI_BIG_ENDIAN_32:
			return src;
		default:
			return src;	
	}
}

/*
*  src buffer bit reorder
*/
static void buf_endian_reverse(INT8U* src, INT32U len, INT8U* dummy_bytes, _gspi_conf_t gspi_conf)
{
	INT32U *buf = (INT32U*)src;
	
	INT16U count = len>>2;
	INT16U remain = len&0x03;
	int i = 0;

	if(remain)
		count ++;
	
	for(i = 0;i < count; i++)
	{
		buf[i] = DWORD_endian_reverse(buf[i], gspi_conf);
	}

	if(remain)
	{
		// if gpsi configured as 16bits
		if((gspi_conf&0x01) == SPI_WORD_LEN_16)
			*dummy_bytes = 1;
		// else if gspi configured as 32bits
		else
			*dummy_bytes = 4 - remain;
	}
}

static void gspi_transfer(INT8U* buf, INT32U buf_len)
{
	INT32S	ret = STATUS_OK;
	INT32U	index = 0;
	INT16U*	buf_temp = (INT16U*)(buf);
	INT32U	count = buf_len/2;

	// swap sccuessive two bytes for SPI 16 bits mode
	for(index=0;index<count;index++)
		buf_temp[index] = TwoByteSwap(buf_temp[index]);
	
	gspi_spi_lock();

#if 0	
	DBG_PRINT("gspi_transfer, data 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x, len %d\r\n", buf[0], buf[1], buf[2], buf[3]
							, buf[4], buf[5], buf[6], buf[7], buf_len);
#endif							
	/* Chip Select Pull Low */
	gspi_master_cs(0);

	/* start SPI transfer */
	ret = spi_transceive(SPI_MASTER_NUM, buf, buf_len, buf, buf_len);
	if (ret != STATUS_OK)
	{
		DBG_PRINT("gspi_transfer failed\r\n");
		gspi_spi_unlock();
 		gspi_master_cs(1);
 		return;
    }

	/* Chip Select Pull High */
	gspi_master_cs(1);
	
	// swap sccuessive two bytes for SPI 16 bits mode
	for(index=0;index<count;index++)
		buf_temp[index] = TwoByteSwap(buf_temp[index]);
	
	gspi_spi_unlock();
}

int gspi_read_write_reg(_reg_ops  ops_type, INT32U addr, char * buf, int len,_gspi_conf_t gspi_conf)
{
	int  fun = 1, domain_id = 0x0; //LOCAL
	unsigned int cmd = 0 ;
	int byte_en = 0 ;//,i = 0 ;
	int ret = 0;
	unsigned char status[GSPI_STATUS_LEN] = {0};
	unsigned int data_tmp = 0;
	INT32U spi_buf[4] = {0};
	
	if (len!=1 && len!=2 && len != 4)
	{
		return -1;
	}

	domain_id = addr_convert(addr);

	addr &= 0x7fff;
	len &= 0xff;
	if (ops_type == WRITE_REG) //write register
	{
		//int remainder = addr % 4;
		int remainder = addr & 0x03;
		INT32U val32 = *(INT32U *)buf;
		switch(len)
		{
			case 1:
				byte_en = (0x1 << remainder);
				data_tmp = (val32& 0xff)<< (remainder*8);
				break;
			case 2:
				byte_en = (0x3 << remainder);
				data_tmp = (val32 & 0xffff)<< (remainder*8);
				break;
			case 4:
				byte_en = 0xf;
				data_tmp = val32 & 0xffffffff;
				break;
			default:
				byte_en = 0xf;
				data_tmp = val32 & 0xffffffff;
				break;
		}
	}
	else //read register
	{
		switch(len)
		{
			case 1:
				byte_en = 0x1;
				break;
			case 2:
				byte_en = 0x3;
				break;
			case 4:
				byte_en = 0xf;
				break;
			default:
				byte_en = 0xf;
				break;
		}

		if(domain_id == SPI_LOCAL_DOMAIN)
			byte_en = 0;
	}

	cmd = FILL_SPI_CMD(byte_en, addr, domain_id, fun, ops_type);
	//4 command segment bytes reorder
	cmd = DWORD_endian_reverse(cmd, gspi_conf);

	if ((ops_type == READ_REG)&& (domain_id!= SPI_RXFIFO_DOMAIN))
	{
		INT32U read_data = 0;

		memset(spi_buf, 0x00, sizeof(spi_buf));

		//SPI_OUT:32bit cmd
		//SPI_IN:64bits status+ XXbits data
		spi_buf[0] = cmd;
		spi_buf[1] = 0;
		spi_buf[2] = 0;
		spi_buf[3] = 0;
		
		gspi_transfer((INT8U*)spi_buf, sizeof(spi_buf));

		memcpy(status, (INT8U *) &spi_buf[1], GSPI_STATUS_LEN);
		read_data = spi_buf[3];
		
		*(INT32U*)buf = DWORD_endian_reverse(read_data, gspi_conf);
	}
	else if (ops_type == WRITE_REG)
	{
		//4 data segment bytes reorder
		data_tmp = DWORD_endian_reverse(data_tmp, gspi_conf);
		//SPI_OUT:32bits cmd+ XXbits data
		//SPI_IN:64bits status
		spi_buf[0] = cmd;
		spi_buf[1] = data_tmp;
		spi_buf[2] = 0;
		spi_buf[3] = 0;
		
		gspi_transfer((INT8U*)spi_buf, sizeof(spi_buf));

		memcpy(status, (INT8U *) &spi_buf[2], GSPI_STATUS_LEN);
	}

	// translate status
	// TODO:
	return ret;
}

INT8U gspi_read8(INT32U addr, INT32S *err)
{
	INT32U ret = 0;
	int val32 = 0 , remainder = 0 ;
	INT32S _err = 0;

	_err = gspi_read_write_reg(READ_REG, addr&0xFFFFFFFC, (char *)&ret, 4, GSPI_CONFIG);
	//remainder = addr % 4;
	remainder = addr &0x03;
	val32 = ret;
	val32 = (val32& (0xff<< (remainder<<3)))>>(remainder<<3);

	if (err)
		*err = _err;

	return (INT8U)val32;
}

INT16U gspi_read16(INT32U addr, INT32S *err)
{
   	INT32U ret = 0;
	int val32 = 0 , remainder = 0 ;
	INT32S _err = 0;

	_err = gspi_read_write_reg(READ_REG, addr&0xFFFFFFFC,(char *)&ret, 4, GSPI_CONFIG);
	//remainder = addr % 4;
	remainder = addr &0x03;
	val32 = ret;
	val32 = (val32& (0xffff<< (remainder<<3)))>>(remainder<<3);

	if (err)
		*err = _err;

	return (INT16U)val32;
}

INT32U gspi_read32(INT32U addr, INT32S *err)
{
	unsigned int ret = 0;
	INT32S _err = 0;

	_err = gspi_read_write_reg(READ_REG, addr&0xFFFFFFFC,(char *)&ret, 4, GSPI_CONFIG);
	if (err)
		*err = _err;

	return  ret;
}

INT32S gspi_write8(INT32U addr, INT8U buf, INT32S *err)
{
	int ret = 0;

	ret = gspi_read_write_reg(WRITE_REG, addr, (char *)&buf, 1, GSPI_CONFIG);
	if (err)
		*err = ret;
	return ret;
}

INT32S gspi_write16(INT32U addr, INT16U buf, INT32S *err)
{
	int ret = 0;

	ret = gspi_read_write_reg(WRITE_REG,addr,(char *)&buf, 2, GSPI_CONFIG);
	if (err)
		*err = ret;
	return ret;
}

INT32S gspi_write32(INT32U addr, INT32U buf, INT32S *err)
{
	int	ret = 0;

	ret = gspi_read_write_reg(WRITE_REG, addr,(char *)&buf, 4, GSPI_CONFIG);
	if (err)
		*err = ret;
	return ret;
}

int gspi_read_rx_fifo(INT8U *buf, INT32U len, struct gspi_more_data * pmore_data,_gspi_conf_t gspi_conf)
{
	int fun = 1;
	INT32U cmd = 0;
	INT8U* spi_buf = (INT8U *) (buf);
	INT8U* spi_data = spi_buf + GSPI_CMD_LEN;
	INT8U* spi_status = spi_data + len;
	int spi_buf_len = GSPI_CMD_LEN + len + GSPI_STATUS_LEN;
	INT8U dummy_bytes = 0;
	
	cmd = FILL_SPI_CMD(len, ((len&0xff00) >>8), SPI_RXFIFO_DOMAIN, fun, (unsigned int)0);

	//4 command segment bytes reorder
	cmd = DWORD_endian_reverse(cmd, gspi_conf);
	memcpy(spi_buf, (INT8U *)&cmd, GSPI_CMD_LEN);
	//4 clean data segment 
	memset(spi_data,0x00, len);
	//4  clean status segment 
	memset(spi_status, 0x00, GSPI_STATUS_LEN);
	
	gspi_transfer((INT8U *)spi_buf, spi_buf_len);
	
	// data segement reorder
	buf_endian_reverse(spi_data, len, &dummy_bytes, gspi_conf);	
	// status segment reorder
	buf_endian_reverse(spi_status, GSPI_STATUS_LEN, &dummy_bytes, gspi_conf);

	pmore_data->more_data = GET_STATUS_HISR(spi_status) & SPI_HIMR_RX_REQUEST_MSK;
	pmore_data->len = GET_STATUS_RXQ_REQ_LEN(spi_status);
	return 0;
}

#define SDIO_RX_BUFSZ	(2048+64) //n*64, must be rounded to 64
int gspi_write_tx_fifo(INT8U *buf, INT32U len, _gspi_conf_t gspi_conf)
{
	int fun = 1; //TX_HIQ_FIFO
	unsigned int cmd = 0;
	INT8U *spi_buf = (INT8U *) (buf);
	INT8U* spi_data = spi_buf + GSPI_CMD_LEN;
	INT8U* spi_status;// = buf + len
	INT32U spi_buf_len = 0;
	
	INT32U NumOfFreeSpace;
	INT8U wait_num = 0;
	INT8U dummy_bytes = 0;

	NumOfFreeSpace = gspi_read32(LOCAL_REG_FREE_TX_SPACE, NULL);

	while ((NumOfFreeSpace * SDIO_RX_BUFSZ) < len)
	{
		if((++wait_num) >= 4)
		{
			DBG_PRINT("gspi_write_tx_fifo(): wait_num is >= 4 , NumOfFreeSpace = %d\r\n", NumOfFreeSpace);
			return -1;
		}
		/* space not enough, sleep a while and get new status of space */
		OSTimeDly(1); 
		NumOfFreeSpace = gspi_read32(LOCAL_REG_FREE_TX_SPACE, NULL);
	}
	//DBG_PRINT("NumOfFreeSpace %d, len 0x%d\r\n", NumOfFreeSpace, len);
	cmd = FILL_SPI_CMD(len, ((len&0xff00) >>8), SPI_TXFIFO_DOMAIN, fun, (unsigned int)1);
	//4 command segment bytes reorder
	cmd = DWORD_endian_reverse(cmd, gspi_conf);
	memcpy(spi_buf, (INT8U*)&cmd, GSPI_CMD_LEN);
	
	//4 data segment bytes reorder
	buf_endian_reverse(spi_data, len, &dummy_bytes, gspi_conf);

	//4 status segment 
	spi_status = spi_data + len + dummy_bytes;
	memset(spi_status, 0x00, GSPI_STATUS_LEN);

	spi_buf_len = GSPI_CMD_LEN + len + dummy_bytes + GSPI_STATUS_LEN;
	
	gspi_transfer((INT8U *) spi_buf, spi_buf_len);
	
	return 0;
}

int gspi_write_page(INT8U *buf, INT32U len, INT8U agg_cnt)
{
	int res;
	INT32U tot_len, i;
	PGSPI_TX_DESC ptxdesc;
	
	tot_len = SIZE_TX_DESC + len;
	// clean T/RX command and descriptor header, the unused bit field must be zero or will cause SDIO of Ameba error.
	//memset(tx_raw_buf, 0, BUFFER_LEN);
	memset(tx_raw_buf,  0, GSPI_CMD_LEN+SIZE_TX_DESC);
	
	for(i=0; i<(GSPI_CMD_LEN+SIZE_TX_DESC); i++)
	{
		if(tx_raw_buf[i] != 0x00)
			DBG_PRINT("raw header != 0\r\n");
	}	

	ptxdesc = (PGSPI_TX_DESC)(tx_raw_buf + GSPI_CMD_LEN); // reserve 4 byte for GSPI cmd
	ptxdesc->txpktsize = len;
	ptxdesc->offset = SIZE_TX_DESC;
	ptxdesc->bus_agg_num = agg_cnt;
	ptxdesc->type = GSPI_CMD_TX;
	
	memcpy(tx_raw_buf+GSPI_CMD_LEN+SIZE_TX_DESC, buf, len);
	res = gspi_write_tx_fifo(tx_raw_buf, tot_len, GSPI_CONFIG);

	return res; 
}

int gspi_read_page(INT8U *buf, INT32U* len)
{
	int res = -1;
	INT32U gspi_hisr;
	INT32U gspi_himr;
	PGSPI_RX_DESC prxdesc;
	struct gspi_more_data more_data;
	
		
	gspi_himr = gspi_read32(SPI_LOCAL_OFFSET | SPI_REG_HIMR, NULL);
	gspi_hisr = gspi_read32(SPI_LOCAL_OFFSET | SPI_REG_HISR, NULL);

	if(gspi_hisr & gspi_himr)
	{
			if(gspi_hisr & SPI_HISR_RX_REQUEST)
			{
                INT32U rx_len = 0;
                INT8U rx_len_rdy = 0;
				
				more_data.more_data = 0;
				more_data.len = 0;
				do
				{
					more_data.more_data = 0;
					more_data.len = 0;
					//validate RX_LEN_RDY before reading RX0_REQ_LEN
					rx_len_rdy = gspi_read8(SPI_LOCAL_OFFSET|(SPI_REG_RX0_REQ_LEN + 3), NULL);
					if(rx_len_rdy & BIT7)
					{
						/* rx_len is 4 bytes alignment */
						rx_len = (gspi_read32(SPI_LOCAL_OFFSET | SPI_REG_RX0_REQ_LEN, NULL)) & 0xffffff;
					}
					
					if(rx_len >(PACK_SIZE+SIZE_RX_DESC))
						rx_len = PACK_SIZE+SIZE_RX_DESC;
					
					if(rx_len)
					{
						res = gspi_read_rx_fifo(buf, rx_len, &more_data, GSPI_CONFIG);
						prxdesc = (PGSPI_RX_DESC)(buf + GSPI_CMD_LEN);
						*len = prxdesc->pkt_len;
					}				
				} while(more_data.more_data && more_data.len);
			}
	}

	return res; 
}

int gspi_configuration(_gspi_conf_t gspi_conf)
{

	INT32U conf = gspi_conf;
	char *s;

	gspi_read_write_reg(WRITE_REG, SPI_LOCAL_OFFSET|SPI_REG_SPI_CFG,(char *)&conf, 1, SPI_LITTLE_ENDIAN_32);
	// read gspi config
	conf = 0xffffffff;
	gspi_read_write_reg(READ_REG, SPI_LOCAL_OFFSET|SPI_REG_SPI_CFG,(char *)&conf, 4, gspi_conf);

	if(conf != gspi_conf)
	{
		DBG_PRINT("gspi_configuration: config fail@ 0x%x\r\n", conf);
		return 1;
	}

	switch(conf)
	{
		case SPI_LITTLE_ENDIAN_16:
		s = "LITTLE_ENDIAN|WORD_LEN_16"; break;
		case SPI_LITTLE_ENDIAN_32:
		s = "LITTLE_ENDIAN|WORD_LEN_32"; break;
		case SPI_BIG_ENDIAN_16:
		s = "BIG_ENDIAN|WORD_LEN_16"; break;
		case SPI_BIG_ENDIAN_32:
		s = "BIG_ENDIAN|WORD_LEN_32"; break;
		default:
		s = "UNKNOW CONFIGURATION"; break;
	};
	DBG_PRINT("gspi_configuration: Current configuration:%s\r\n", s);
	return 0;
}

void gspi_rx_task(void *parm)
{
	INT32U gspi_hisr;
	INT32U gspi_himr;
	PGSPI_RX_DESC prxdesc;
	struct gspi_more_data more_data;
	
	while(1)
	{
		/* wait semaphore */
		gspi_master_lock();
		
		gspi_himr = gspi_read32(SPI_LOCAL_OFFSET | SPI_REG_HIMR, NULL);
		gspi_hisr = gspi_read32(SPI_LOCAL_OFFSET | SPI_REG_HISR, NULL);

		if(gspi_hisr & gspi_himr)
		{
			if(gspi_hisr & SPI_HISR_RX_REQUEST)
			{
                INT32U rx_len = 0;
                INT8U rx_len_rdy = 0;
				
				more_data.more_data = 0;
				more_data.len = 0;
				do
				{
					more_data.more_data = 0;
					more_data.len = 0;
					//validate RX_LEN_RDY before reading RX0_REQ_LEN
					rx_len_rdy = gspi_read8(SPI_LOCAL_OFFSET|(SPI_REG_RX0_REQ_LEN + 3), NULL);
					if(rx_len_rdy & BIT7)
					{
						/* rx_len is 4 bytes alignment */
						rx_len = (gspi_read32(SPI_LOCAL_OFFSET | SPI_REG_RX0_REQ_LEN, NULL)) & 0xffffff;
					}
					
					if(rx_len >(PACK_SIZE+SIZE_RX_DESC))
						rx_len = PACK_SIZE+SIZE_RX_DESC;

					if(rx_len)
					{
						//memset(rx_raw_buf, 0, BUFFER_LEN);
						gspi_read_rx_fifo(rx_raw_buf, rx_len, &more_data, GSPI_CONFIG);
						prxdesc = (PGSPI_RX_DESC)(rx_raw_buf + GSPI_CMD_LEN);

						/* Call back to upper application with data pointer and length if the callback function was registered */
						if(gspi_rx_cbk)
						{
							gspi_rx_cbk((INT8U*)(rx_raw_buf + GSPI_CMD_LEN + SIZE_RX_DESC), prxdesc->pkt_len);
						}						
					}				
				} while(more_data.more_data && more_data.len);
			}
		}
	}
}
