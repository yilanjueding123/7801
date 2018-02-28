/************************************************************
* gp_socket_cmd.c
*
* Purpose: GP socket command APP
*
* Author: Eugenehsu
*
* Date: 2015/10/01
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 1.10
* History :
*
************************************************************/
#include "string.h"
#include "stdio.h"
#include "project.h"
#include "application.h"
#include "video_encoder.h"
#include "gspi_cmd.h"
#include "gspi_master_drv.h"
#include "gp_cmd.h"


/**********************************************
*	Definitions
**********************************************/
typedef enum
{
	GP_SOCK_CMD_GENERAL_GETDEVICESTATUS 	= 0x0000,
	GP_SOCK_CMD_GENERAL_POWEROFF			= 0x0001,
	GP_SOCK_CMD_GENERAL_AUTHDEVICE			= 0x0002,
	GP_SOCK_CMD_GENERAL_GETINDEXLIST		= 0x0003,
	
	GP_SOCK_CMD_PLAY_START	 	  			= 0x0100,
	GP_SOCK_CMD_PLAY_PAUSERESUME  			= 0x0101,
	GP_SOCK_CMD_PLAY_STOP	 	  			= 0x0102,
	
	GP_SOCK_CMD_RECORD_START 	  			= 0x0200,
	GP_SOCK_CMD_RECORD_STOP 	  			= 0x0201,
	
	GP_SOCK_CMD_DATATRANSFER_START 			= 0x0300,
	GP_SOCK_CMD_DATATRANSFER_STOP 			= 0x0301,
	GP_SOCK_CMD_DATATRANSFER_RAWDATA 		= 0x0302,
	
	GP_SOCK_CMD_LOOPBACKTEST 				= 0xF000,
	
	GP_SOCK_CMD_VENDOR						= 0xFF00
} GP_SOCK_CMD_E;

#define GP_SOCK_CMD_TYPE	0x0001
#define GP_SOCK_ACK_TYPE	0x0002
#define GP_SOCK_NAK_TYPE	0x0003

#define	GP_CMD_TAG_SIZE		8
#define	GP_CMD_TYPE_SIZE	2
#define	GP_CMD_MODE_SIZE	1
#define	GP_CMD_ID_SIZE		1
#define	GP_CMD_PAYLAOD_SIZE	2
#define GP_CMD_HEADER_SIZE			(GP_CMD_TAG_SIZE+GP_CMD_TYPE_SIZE+GP_CMD_MODE_SIZE+GP_CMD_ID_SIZE+GP_CMD_PAYLAOD_SIZE)
#define GP_CMD_PAYLOAD_BUF_SIZE		256
#define GP_CMD_CMD_BUF_SIZE			(GP_CMD_HEADER_SIZE + GP_CMD_PAYLOAD_BUF_SIZE)

#define GP_SOCK_TAG0	'G'
#define GP_SOCK_TAG1	'P'	
#define GP_SOCK_TAG2	'S'
#define GP_SOCK_TAG3	'O'
#define GP_SOCK_TAG4	'C'
#define GP_SOCK_TAG5	'K'
#define GP_SOCK_TAG6	'E'
#define GP_SOCK_TAG7	'T'

PACKED typedef struct GP_CMD_DATA_S
{
	char tag[GP_CMD_TAG_SIZE];
	char type[GP_CMD_TYPE_SIZE];
	char mode;
	char cmdid;
	char payloadsize[GP_CMD_PAYLAOD_SIZE];
	char payload[GP_CMD_PAYLOAD_BUF_SIZE];
} GP_CMD_DATA_T;

#define GPSOCKCMD_STACK_SIZE		1024
#define GP_SOCK_RAW_DATA_BUF_SIZE	(100*1024)
#define GP_SOCK_RAW_DATA_TRANSFER_SIZE	(10*1024)
#define DISPLAY_DEV_HPIXEL 320
#define DISPLAY_DEV_VPIXEL 240
#define GP_SOCKET_Q_NUM		16
/**********************************************
*	Extern variables and functions
**********************************************/
/*********************************************
*	Variables declaration
*********************************************/
static OS_EVENT *gp_sock_q = NULL;
static void *gp_sock_q_stack[GP_SOCKET_Q_NUM];
static OS_EVENT *gp_socket_cmd_tx_sem;
static INT32U GPSocketCmdTaskStack[GPSOCKCMD_STACK_SIZE];
static INT8U gp_sock_rx_buf[GP_CMD_CMD_BUF_SIZE];
static INT8U gp_sock_tx_buf[GP_CMD_CMD_BUF_SIZE];
static INT8U gp_sock_raw_data_buf[GP_SOCK_RAW_DATA_BUF_SIZE];
static INT8U* gp_raw_data_ptr = NULL;
static INT32U gp_sock_rx_buf_len = 0;
static INT8U *gp_sock_read_ptr = 0;
INT32U gp_sock_decode_output_ptr = NULL;
INT32U gp_sock_connect_state = 0;

static void _send_handshake_back_to_gp_socket(INT32U type)
{
	GP_CMD_DATA_T* gp_sock_cmd_tx_ptr = (GP_CMD_DATA_T*)gp_sock_tx_buf;
	GP_CMD_DATA_T* gp_sock_cmd_rx_ptr = (GP_CMD_DATA_T*)gp_sock_rx_buf;
	
	/* Send ACK back to GP socket */
	memset(gp_sock_tx_buf, 0 ,sizeof(gp_sock_tx_buf));
	gp_sock_cmd_tx_ptr->tag[0] = GP_SOCK_TAG0;
	gp_sock_cmd_tx_ptr->tag[1] = GP_SOCK_TAG1;
	gp_sock_cmd_tx_ptr->tag[2] = GP_SOCK_TAG2;
	gp_sock_cmd_tx_ptr->tag[3] = GP_SOCK_TAG3;
	gp_sock_cmd_tx_ptr->tag[4] = GP_SOCK_TAG4;
	gp_sock_cmd_tx_ptr->tag[5] = GP_SOCK_TAG5;
	gp_sock_cmd_tx_ptr->tag[6] = GP_SOCK_TAG6;
	gp_sock_cmd_tx_ptr->tag[7] = GP_SOCK_TAG7;
	
	if(type == GP_SOCK_ACK_TYPE)
	{
		gp_sock_cmd_tx_ptr->type[0] = 0x02;
		gp_sock_cmd_tx_ptr->type[1] = 0x00;
	}
	else if(type == GP_SOCK_NAK_TYPE)
	{
		gp_sock_cmd_tx_ptr->type[0] = 0x03; 
		gp_sock_cmd_tx_ptr->type[1] = 0x00;
	}	

	gp_sock_cmd_tx_ptr->mode = gp_sock_cmd_rx_ptr->mode;
	gp_sock_cmd_tx_ptr->cmdid = gp_sock_cmd_rx_ptr->cmdid;
	/* Set payload size = 0 */
	gp_sock_cmd_tx_ptr->payloadsize[0] = 0x00;
	gp_sock_cmd_tx_ptr->payloadsize[1] = 0x00;

	gspi_tx_data(gp_sock_tx_buf, GP_CMD_HEADER_SIZE, DATA_GP_SOCK_CMD_TYPE);
}	

static void start_decode_image(void)
{
	if(gp_sock_decode_output_ptr == NULL)
	{
		gp_sock_decode_output_ptr = (INT32U) gp_malloc_align((DISPLAY_DEV_HPIXEL*DISPLAY_DEV_VPIXEL)*2, 64);//malloc decode frame buffer
		if(gp_sock_decode_output_ptr == NULL)
		{
			DBG_PRINT("gp_sock_decode_output_ptr malloc fail\r\n");
		}	
	}
		
	//tft_init();
	//tft_start(C_DISPLAY_DEVICE);
}

static void stop_decode_image(void)
{
    memset((INT8U*)gp_sock_decode_output_ptr, 0x0, (DISPLAY_DEV_HPIXEL*DISPLAY_DEV_VPIXEL)*2);
    //video_codec_show_image(C_DISPLAY_DEVICE, (INT32U)gp_sock_decode_output_ptr, C_DISPLAY_DEVICE, IMAGE_OUTPUT_FORMAT_RGB565);

	OSTimeDly(5);
	//tft_init();
	if(gp_sock_decode_output_ptr)
	{
		gp_free((void*)gp_sock_decode_output_ptr);
		gp_sock_decode_output_ptr = NULL;
	}	
}	
	
static void decode_jpg_buffer_to_tft(INT32U buf_address, INT32U h_pixel, INT32U v_pixel)
{
//		IMAGE_ARGUMENT image_decode;
//		MEDIA_SOURCE image_source;
//		INT32U decode_state;
//		DBG_PRINT("\r\nGP socket Decode ");
//		
//		image_source.type = SOURCE_TYPE_SDRAM;
//		image_source.type_ID.memptr = (INT8U*)buf_address;
//			
//		image_decode.ScalerOutputRatio=FIT_OUTPUT_SIZE;   
//		image_decode.OutputFormat=IMAGE_OUTPUT_FORMAT_YUYV;                  //scaler out format
//		//image output information
//		image_decode.OutputBufPtr=(INT8U *)gp_sock_decode_output_ptr;            //decode output buffer
//		image_decode.OutputBufWidth=h_pixel;                  //width of output buffer 
//		image_decode.OutputBufHeight=v_pixel;                 //Heigh of output buffer
//		image_decode.OutputWidth=h_pixel;                     //scaler width of output image
//		image_decode.OutputHeight=v_pixel;                    //scaler Heigh of output image
//		
//		//image decode function
//		image_decode_entrance();     //global variable initial for image decode 
//		image_decode_start(image_decode,image_source);    //image decode start
//		while (1) {
//			 decode_state = image_decode_status();
//			 if (decode_state == IMAGE_CODEC_DECODE_END) {
//				 video_codec_show_image(C_DISPLAY_DEVICE, (INT32U)gp_sock_decode_output_ptr, C_DISPLAY_DEVICE, IMAGE_OUTPUT_FORMAT_YUYV);
//				 break;
//			  }else if(decode_state == IMAGE_CODEC_DECODE_FAIL) {
//				 DBG_PRINT("image decode failed\r\n");
//				 break;
//			  }	
//		}	
//	
//		image_decode_stop();
}

void process_gp_socket_cmd(INT8U* buf)
{
	INT32U loopback_len, payloadsize; 
	INT16U gp_cmd;
	INT8U err;
	GP_CMD_DATA_T* gp_sock_cmd_ptr = (GP_CMD_DATA_T*)buf;
	
	if(gp_sock_cmd_ptr->tag[0] == GP_SOCK_TAG0 && gp_sock_cmd_ptr->tag[1] == GP_SOCK_TAG1 && gp_sock_cmd_ptr->tag[2] == GP_SOCK_TAG2
					&& gp_sock_cmd_ptr->tag[3] == GP_SOCK_TAG3 && gp_sock_cmd_ptr->tag[4] == GP_SOCK_TAG4 && gp_sock_cmd_ptr->tag[5] == GP_SOCK_TAG5
					&& gp_sock_cmd_ptr->tag[6] == GP_SOCK_TAG6 && gp_sock_cmd_ptr->tag[7] == GP_SOCK_TAG7)
	{
		gp_cmd = ((gp_sock_cmd_ptr->mode) << 8) | gp_sock_cmd_ptr->cmdid;
		//DBG_PRINT("GP socket cmd 0x%x got len %d\r\n", gp_cmd, gp_sock_rx_buf_len);
		
		if(gp_cmd == GP_SOCK_CMD_LOOPBACKTEST)
		{
			loopback_len = (gp_sock_cmd_ptr->payloadsize[1] << 8) | gp_sock_cmd_ptr->payloadsize[0];
			//DBG_PRINT("GPSOCKET cmd got: 0x%x, loopback len %d, buf len %d\r\n", gp_cmd, loopback_len, gp_sock_rx_buf_len);	
			
			/* Write back to GP socket client */
			memset(gp_sock_tx_buf, 0 ,sizeof(gp_sock_tx_buf));
			memcpy((char*)gp_sock_tx_buf, (char*)gp_sock_rx_buf, gp_sock_rx_buf_len);
			OSSemPend(gp_socket_cmd_tx_sem, 0, &err);
			/* Send back for loopback test */
			gspi_tx_data(gp_sock_tx_buf, gp_sock_rx_buf_len, DATA_GP_SOCK_CMD_TYPE);
		}
		else
		{
			switch(gp_cmd)
			{
				case GP_SOCK_CMD_PLAY_START:
					break;
					
				case GP_SOCK_CMD_PLAY_PAUSERESUME:
					break;
					
				case GP_SOCK_CMD_PLAY_STOP:
					gp_sock_read_ptr = 0;
					break;
					
				case GP_SOCK_CMD_DATATRANSFER_START:
					/* Reset RX raw buffer */
					memset(gp_sock_raw_data_buf, 0, GP_SOCK_RAW_DATA_BUF_SIZE);
					gp_raw_data_ptr = (INT8U*)gp_sock_raw_data_buf;
					gp_sock_read_ptr = gp_sock_raw_data_buf;
					//stop_decode_image();
					DBG_PRINT("DATA start prt 0x%x\r\n", gp_raw_data_ptr);
					break;
					
				case GP_SOCK_CMD_DATATRANSFER_RAWDATA:
					payloadsize = (gp_sock_cmd_ptr->payloadsize[1] << 8) | gp_sock_cmd_ptr->payloadsize[0];
					//DBG_PRINT("Payloadsize = 0x%x\r\n", payloadsize);
					/* Copy raw data to buffer */
					memcpy(gp_raw_data_ptr, (INT8U*)(gp_sock_cmd_ptr->payload), payloadsize);
					gp_raw_data_ptr += payloadsize;
					break;
				
				case GP_SOCK_CMD_DATATRANSFER_STOP:
					start_decode_image();
					decode_jpg_buffer_to_tft((INT32U)gp_sock_raw_data_buf, DISPLAY_DEV_HPIXEL, DISPLAY_DEV_VPIXEL);
					//DBG_PRINT("Raw data stop: total length 0x%x\r\n", (gp_raw_data_ptr - gp_sock_raw_data_buf));
					break;
				
				default:
					break;
			}	
			
			/* Send ACK back to GP socket */
			_send_handshake_back_to_gp_socket(GP_SOCK_ACK_TYPE);
		}	
	}		
}	

static void gp_socket_cmd_gspi_cbk(INT8U* buf, INT32U len, INT32U event)
{
	INT32U msg = event;
	INT32U size = len;
	
	//DBG_PRINT("gp_socket_cmd_gspi_cbk len %d, event 0x%x\r\n", len , event);
	
	if(event == GP_SOCK_RXDONE_EVENT)
	{
		if(len)
		{
	if(size > GP_CMD_CMD_BUF_SIZE)
		size = GP_CMD_CMD_BUF_SIZE;
	
	//DBG_PRINT("Got GP socket cmd buf len %d\r\n", len);
	gp_sock_rx_buf_len = size;
			memcpy(gp_sock_rx_buf, buf, gp_sock_rx_buf_len);
}	
	}
	else if(event == GP_SOCK_RAW_DATA_DONE_EVENT)
	{
		if(len)
		{
			gp_sock_rx_buf_len = size;
		}
	}
	else if(event == GP_SOCK_RAW_DATA_SEND_BACK_EVENT)
	{
		//DBG_PRINT("GP_SOCK_RAW_DATA_SEND_BACK_EVENT len %d, reamin %d\r\n", len, gp_sock_rx_buf_len);
		memcpy(gp_sock_read_ptr, buf, len);
		gp_sock_read_ptr += len;
		gp_sock_rx_buf_len -= len;

		if(gp_sock_rx_buf_len == 0)
{
			msg = GP_SOCK_RAW_DATA_SEND_BACK_DONE_EVENT;
		}	
	}	
	//DBG_PRINT("gp_socket_cmd_gspi_cbk event %d\r\n", event);
	OSQPost(gp_sock_q, (void*)msg);	
}	

void gp_sock_cmd_task(void *parm)
{
	INT32U msg_id;
	INT8U err;
	
	while(1)
	{
		msg_id = (INT32U) OSQPend(gp_sock_q, 0, &err);
		if(err != OS_NO_ERR)
			continue;
			
		switch(msg_id)
		{
			case GP_SOCK_TXDONE_EVENT:
				OSSemPost(gp_socket_cmd_tx_sem);
				break;
			
			case GP_SOCK_RXDONE_EVENT:
		process_gp_socket_cmd(gp_sock_rx_buf);
		memset(gp_sock_rx_buf, 0, sizeof(gp_sock_rx_buf));
		gp_sock_rx_buf_len = 0;	
				break;
			
			case GP_SOCK_CLI_DISC_EVENT:
				/* Client disconnected from GP socket */
				DBG_PRINT("Client disconnected from GP socket\r\n");
				stop_decode_image();
				break;
			
			case GP_SOCK_CLI_CONN_EVENT:
				/* Client connected from GP socket */
				DBG_PRINT("Client connected to GP socket\r\n");
				break;
				
			case GP_SOCK_RAW_DATA_DONE_EVENT:
				/* Send GSPI_GET_GP_SOCKET_RAW_CMD to get 1k bytes data */
				if(gp_sock_rx_buf_len >= GP_SOCK_RAW_DATA_TRANSFER_SIZE)
				{
					gspi_tx_cmd(GSPI_GET_GP_SOCKET_RAW_CMD, GP_SOCK_RAW_DATA_TRANSFER_SIZE);
				}	
				else
				{
					gspi_tx_cmd(GSPI_GET_GP_SOCKET_RAW_CMD, gp_sock_rx_buf_len);	
				}	
				break;
			
			case GP_SOCK_RAW_DATA_SEND_BACK_EVENT:
				if(gp_sock_rx_buf_len)
				{
					if(gp_sock_rx_buf_len >= GP_SOCK_RAW_DATA_TRANSFER_SIZE)
					{
						gspi_tx_cmd(GSPI_GET_GP_SOCKET_RAW_CMD, GP_SOCK_RAW_DATA_TRANSFER_SIZE);
					}	
					else
					{
						gspi_tx_cmd(GSPI_GET_GP_SOCKET_RAW_CMD, gp_sock_rx_buf_len);
					}	
				}
				break;
			
			case GP_SOCK_RAW_DATA_SEND_BACK_DONE_EVENT:		
					gp_sock_rx_buf[10] = 0x03;
					gp_sock_rx_buf[11] = 0x02;
					//DBG_PRINT("send ack back to ameba\r\n");
					/* Send ACK back to GP socket */
					_send_handshake_back_to_gp_socket(GP_SOCK_ACK_TYPE);
				break;
					
			default:
				break;
		}		
	}	
}	

void gp_socket_cmd_init(void)
{
	INT8U err;
	
	/* Init GSPI command module */
	gspi_cmd_init();
	
	/* Register call back function for GSPI RX data */
	gspi_register_gp_sock_cmd_cbk((INT32U)gp_socket_cmd_gspi_cbk);
	
	if(gp_sock_q == NULL)
	{
		gp_sock_q = OSQCreate(gp_sock_q_stack, GP_SOCKET_Q_NUM);
		if(gp_sock_q == 0)
		{
			DBG_PRINT("Create gp_sock_q failed\r\n");	
			return;
		}
	}
	
	gp_socket_cmd_tx_sem = OSSemCreate(1);
	err = OSTaskCreate(gp_sock_cmd_task, (void *)NULL, &GPSocketCmdTaskStack[GPSOCKCMD_STACK_SIZE - 1], GP_SOCKET_CMD_PRIORITY);	
	if(err != OS_NO_ERR)
	{ 
		DBG_PRINT("Cannot create GSPI socket command task\n\r");
		return;
	}
}	