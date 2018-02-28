/******************************************************
* rtp_server.c
*
* Purpose: RTP server to wait for a RTSP connection
*
* Author: Eugene Hsu
*
* Date: 2016/03/24
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 
* History :
*
*******************************************************/
#include "lwip/api.h"
#include "stdio.h"
#include "string.h"
#include "sockets.h"
#include "rtsp.h"
#include "rtp.h"
#include "api.h"

extern int mjpeg_frame_counter_set(int mode);

#define RTP_SEND_JPEG_EVENT		0x03
static void *rtp_q_stack[RTPSRV_MSG_QUEUE_LEN];
static INT32U rtp_task_stack[RTP_TASK_STACKSIZE];

UDP_SRV_SOCKET_CTL_T rtp_server_ctl_blk =
{
	0,					/* Server socket */
	NULL,				/* TX message Q handler */
	NULL,				/* RX buffer pointer */
	NULL,				/* TX buffer pointer */
	0,					/* Receive broadcast packet */
	RTP_SERVER_PORT,	/* Server port */
};

RTP_CTL_BLK_T rtp_ctl_blk =
{
	0,		/* Time Stamp in RTP header */
	480,	/* Width of picture in RTP JPEG header */
	272,	/* Height of picture in RTP JPEG header */
	8000,	/* Audio streaming sample rate */
	100,	/* Audio ptime */
	0,		/* SSRC in RTP header */
	0, 		/* Transfer size for each RTP data transfer */
	0,		/* RTP packet send flag */
	0,		/* Transfer data size */
	0,		/* Remain transfer size */
	0,		/* Sequence number in RTP header */
	{0},	/* Remote client IP address */
	NULL,	/* RTP packet buffer pointer */
	NULL,	/* RTP data buffer */
	RTP_PT_JPEG,	/* RTP payload type */
};

static RTP_HDR_T rtphdr;
static JPEG_HDR_T jpeghdr;
static int rtp_seq_flag = 0;

void rtp_dbg_seq_set(int flag)
{
	rtp_seq_flag = flag;
}

/* Call back from RTSP task */
void rtp_rtsp_cbk(INT32U event)
{
	switch(event)
	{
		case RTP_START_EVENT:
			rtp_ctl_blk.rtp_send_flag = 1;
			break;
			
		case RTP_STOP_EVENT:
			rtp_ctl_blk.rtp_send_flag = 0;
			break;
				
		default:
			break;
	}	
}	

INT32U rtp_check_sending_data(void)
{
	return rtp_ctl_blk.rtp_send_flag;
}

int rtp_send_packet(char* buf, INT32U size)
{
	struct sockaddr_in to_addr;
	int err;
	INT32U retrycnt; 
	
	/* Copy remote client IP address */
    sprintf(rtp_ctl_blk.rtp_remote_ip, "%d.%d.%d.%d", rtsp_instance.connect_ip[0] & 0xFF, rtsp_instance.connect_ip[1] & 0xFF, rtsp_instance.connect_ip[2] & 0xFF, rtsp_instance.connect_ip[3] & 0xFF);    
    to_addr.sin_len = sizeof(to_addr);
    to_addr.sin_port = htons(rtsp_instance.client_rtp_port1);
	to_addr.sin_family = AF_INET;
	to_addr.sin_addr.s_addr = inet_addr(rtp_ctl_blk.rtp_remote_ip);
	
	/* Check WLAN buffer is enough or not */
check_skb:

#if 0
  	retrycnt = 0;
	if(skbdata_used_num > (MAX_SKB_BUF_NUM - 3))
	{     
	  	if(retrycnt ++ == 1000)	 
	  		DBG_PRINT("wait skbbuf\r\n");   
    	//vTaskDelay(1);                        
		goto check_skb;                 
	}
#endif	

send_again:	
	err = sendto(rtp_server_ctl_blk.udp_server, buf, size, 0, (struct sockaddr *)&to_addr, sizeof(to_addr));
	
	if(err <= 0)
	{
	  	DBG_PRINT("Send RTP err %d\r\n", err);
	  	OSTimeDly(1);
		goto send_again;
		//return -1;
	}
	else
	{
		return err;
	}
}

int rtp_send_jpeg(char* buf, INT32U size, INT32U flag, INT32U ts)
{
	INT32U reamin_size = size;
	INT32U tx_len, copy_len;
	char* pkt_ptr;
	char* jpeg_ptr;
	char* temp;
	int err;
	static INT32U start_ok = 0;

	if(flag == RTP_SEND_JPEG_START_PART || flag == RTP_SEND_JPEG_FRAME)
    {
		/* Reset JPEG offset */
    	rtp_ctl_blk.rtp_transfer_size = 0;
    	rtp_ctl_blk.rtp_ts = ts;
    }	

	if(flag == RTP_SEND_JPEG_START_PART)
		start_ok = 0;

	/* Initialize RTP header */
    rtphdr.version = 2;
    rtphdr.p = 0;
    rtphdr.x = 0;
    rtphdr.cc = 0;
    rtphdr.m = 0;
    rtphdr.pt = RTP_PT_JPEG;
    rtphdr.seq = ((rtp_ctl_blk.rtp_sequence & 0xff) << 8) | ((rtp_ctl_blk.rtp_sequence & 0xff00) >> 8);
    rtphdr.ts = ((rtp_ctl_blk.rtp_ts & 0xff) << 24) | ((rtp_ctl_blk.rtp_ts & 0xff00) << 8) | ((rtp_ctl_blk.rtp_ts & 0xff0000) >> 8) | ((rtp_ctl_blk.rtp_ts & 0xff000000) >> 24);
    rtphdr.ssrc = rtp_ctl_blk.rtp_ssrc;
    
	/* Initialize JPEG header */
    jpeghdr.tspec = 0;
    jpeghdr.type = 0;
    jpeghdr.q = 1;
    jpeghdr.width = rtp_ctl_blk.rtp_width >> 3;
    jpeghdr.height = rtp_ctl_blk.rtp_height >> 3;
    jpeghdr.off = ((rtp_ctl_blk.rtp_transfer_size & 0xff) << 16) | (rtp_ctl_blk.rtp_transfer_size & 0x00ff00) | ((rtp_ctl_blk.rtp_transfer_size & 0xff0000) >> 16);

	/* Assign JPEG data buffer */
	jpeg_ptr = buf;

	while(reamin_size)
	{
		if(reamin_size > RTP_PAYLOAD_SIZE)
		{
			tx_len = RTP_PAYLOAD_SIZE;
		}	
		else
		{
			/* This is end of frame */
			tx_len = reamin_size;
			if(flag == RTP_SEND_JPEG_END_PART || flag == RTP_SEND_JPEG_FRAME)
			{
				rtphdr.m = 1;
			}	
		}	
		
		pkt_ptr = (char*)rtp_ctl_blk.rtp_packet_buf;
		
		/* Copy RTP header */
		copy_len = sizeof(rtphdr);
		memcpy(pkt_ptr, (char*)&rtphdr, copy_len);
		pkt_ptr += copy_len;
		
		/* Copy RTP JPEG header */
		copy_len = sizeof(jpeghdr);
		memcpy(pkt_ptr, (char*)&jpeghdr, copy_len);
		pkt_ptr += copy_len;
		
		memcpy(pkt_ptr, jpeg_ptr, tx_len);
		rtp_send_packet(rtp_ctl_blk.rtp_packet_buf, (tx_len+sizeof(rtphdr)+sizeof(jpeghdr)));
			
		jpeg_ptr+= tx_len;
		rtp_ctl_blk.rtp_transfer_size += tx_len;
		reamin_size -= tx_len;
		rtp_ctl_blk.rtp_sequence++;

		jpeghdr.off = ((rtp_ctl_blk.rtp_transfer_size & 0xff) << 16) | (rtp_ctl_blk.rtp_transfer_size & 0x00ff00) | ((rtp_ctl_blk.rtp_transfer_size & 0xff0000) >> 16);
		rtphdr.seq = ((rtp_ctl_blk.rtp_sequence & 0xff) << 8) | ((rtp_ctl_blk.rtp_sequence & 0xff00) >> 8);
	}

	if(flag == RTP_SEND_JPEG_END_PART || flag == RTP_SEND_JPEG_FRAME)
	{
		/* Update timestamp for per frame */
		//rtp_ctl_blk.rtp_ts += RTP_JPEG_FRAME_TS;
	}
		
	return 0;
}

int rtp_send_whole_jpeg(char* buf, INT32U size, INT32U ts)
{
	INT32U reamin_size = size;
	INT32U tx_len, copy_len;
	char* pkt_ptr;
	char* jpeg_ptr;
	char* temp;
	int err, ret;
	//DBG_PRINT("RTP SEND size %d, ts %d\r\n", size, ts);

	/* Add Debug Information */
	if (rtp_seq_flag)
	{ // GPEncoder ==> replace "er"
		INT16U *p = (INT16U*)(buf+32);
		*p = rtp_ctl_blk.rtp_sequence;
	}

	/* Reset JPEG offset */
	rtp_ctl_blk.rtp_transfer_size = 0;
	rtp_ctl_blk.rtp_ts = ts;

	/* Initialize RTP header */
    rtphdr.version = 2;
    rtphdr.p = 0;
    rtphdr.x = 0;
    rtphdr.cc = 0;
    rtphdr.m = 0;
    rtphdr.pt = RTP_PT_JPEG;
    rtphdr.seq = ((rtp_ctl_blk.rtp_sequence & 0xff) << 8) | ((rtp_ctl_blk.rtp_sequence & 0xff00) >> 8);
    rtphdr.ts = ((rtp_ctl_blk.rtp_ts & 0xff) << 24) | ((rtp_ctl_blk.rtp_ts & 0xff00) << 8) | ((rtp_ctl_blk.rtp_ts & 0xff0000) >> 8) | ((rtp_ctl_blk.rtp_ts & 0xff000000) >> 24);
    rtphdr.ssrc = rtp_ctl_blk.rtp_ssrc;
    
	/* Initialize JPEG header */
    jpeghdr.tspec = 0;
    jpeghdr.type = 0;
    jpeghdr.q = 1;
    jpeghdr.width = rtp_ctl_blk.rtp_width >> 3;
    jpeghdr.height = rtp_ctl_blk.rtp_height >> 3;
    jpeghdr.off = ((rtp_ctl_blk.rtp_transfer_size & 0xff) << 16) | (rtp_ctl_blk.rtp_transfer_size & 0x00ff00) | ((rtp_ctl_blk.rtp_transfer_size & 0xff0000) >> 16);

	/* Assign JPEG data buffer */
	jpeg_ptr = buf;

	while(reamin_size)
	{
		if(reamin_size > RTP_PAYLOAD_SIZE)
		{
			tx_len = RTP_PAYLOAD_SIZE;
		}	
		else
		{
			/* This is end of frame */
			tx_len = reamin_size;
			rtphdr.m = 1;
		}	

		//DBG_PRINT("In loop remain %d txlen %d, tx size %d\r\n", reamin_size, tx_len, rtp_ctl_blk.rtp_transfer_size);
		pkt_ptr = (char*)rtp_ctl_blk.rtp_packet_buf;
		
		/* Copy RTP header */
		copy_len = sizeof(rtphdr);		// 12 bytes
		memcpy(pkt_ptr, (char*)&rtphdr, copy_len);
		pkt_ptr += copy_len;
		
		/* Copy RTP JPEG header */
		copy_len = sizeof(jpeghdr);		// 8 bytes
		memcpy(pkt_ptr, (char*)&jpeghdr, copy_len);
		pkt_ptr += copy_len;

		//DBG_PRINT("JDATA:0x%x 0x%x 0x%x 0x%x\r\n", jpeg_ptr[0], jpeg_ptr[1], jpeg_ptr[2], jpeg_ptr[3]);
		memcpy(pkt_ptr, jpeg_ptr, tx_len);
		ret = rtp_send_packet(rtp_ctl_blk.rtp_packet_buf, (tx_len+sizeof(rtphdr)+sizeof(jpeghdr)));
		tx_len = ret - (sizeof(rtphdr)+sizeof(jpeghdr));	
		jpeg_ptr+= tx_len;
		rtp_ctl_blk.rtp_transfer_size += tx_len;
		reamin_size -= tx_len;
		rtp_ctl_blk.rtp_sequence++;

		jpeghdr.off = ((rtp_ctl_blk.rtp_transfer_size & 0xff) << 16) | (rtp_ctl_blk.rtp_transfer_size & 0x00ff00) | ((rtp_ctl_blk.rtp_transfer_size & 0xff0000) >> 16);
		rtphdr.seq = ((rtp_ctl_blk.rtp_sequence & 0xff) << 8) | ((rtp_ctl_blk.rtp_sequence & 0xff00) >> 8);
	}
		
	return 0;
}

#include "application.h"
extern OS_EVENT* my_AVIEncodeApQ;
extern INT32U mjpeg_service_playjpeg_flag;
void rtp_task(void *pvParameters)
{
	struct sockaddr_in server_addr;
	INT32U msg;
	int cnt;
	int addr_reuse = 1, status, ret, max_fd, len;
	ip_addr_t addr;
	INT32U remain;
	char* write_ptr;
	char* ptr;
	INT32U i, read_len, remain_len, tick;
	static INT32U isstart = 0;
	static INT32U rtp_ts;
	static INT32U mjpeg_timestamp = 0;	
	fd_set readfds, writefds;
	struct timeval tv;
	INT8U os_err;
	mjpeg_write_data_t* pMJPEG_Write_Msg;
	
  	
	/* Create a UDP server */
	rtp_server_ctl_blk.udp_server = socket(AF_INET, SOCK_DGRAM, 0);
	if(rtp_server_ctl_blk.udp_server < 0)
	{
		DBG_PRINT("RP socket failed\r\n");
		goto exit_tx_task;
	}
	
	status = setsockopt(rtp_server_ctl_blk.udp_server, SOL_SOCKET, SO_REUSEADDR, (const char *) &addr_reuse, sizeof(addr_reuse));	
	if(status < 0)
	{
		DBG_PRINT("RP REUSEADDR error 0x%x\r\n", status);
	}
		
	/* Bind server to local port and IP address */
	FD_ZERO(&server_addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(rtp_server_ctl_blk.server_port);
     
	if (bind(rtp_server_ctl_blk.udp_server, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		DBG_PRINT("RP bind error\r\n");
	}	

	OSTimeDly(20);
	DBG_PRINT("Create RTP:%d ok\r\n", rtp_server_ctl_blk.server_port);

	while(1)
	{
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
	    max_fd = -1;
	    
	    tv.tv_sec = 0;
		tv.tv_usec = 0;
		
		FD_SET(rtp_server_ctl_blk.udp_server, &readfds);
		FD_SET(rtp_server_ctl_blk.udp_server, &writefds);	
	    max_fd = rtp_server_ctl_blk.udp_server;
	    
	    ret = select((max_fd + 1), &readfds, &writefds, NULL, &tv);
		
		if(ret > 0)
		{ 
			/* Check read socket descriptor */
	    	if(FD_ISSET(rtp_server_ctl_blk.udp_server, &readfds))
	    	{
	    		len = read(rtp_server_ctl_blk.udp_server, rtp_server_ctl_blk.udpsrv_rx_buf, RTP_RX_BUF_SIZE);
				if(len > 0)
				{
					if(len > RTP_RX_BUF_SIZE)
					{
						DBG_PRINT("RTP RX max packet len %d\r\n", len);
						len = RTP_RX_BUF_SIZE;
					}	
					DBG_PRINT("RTP RX packet[%d]:\r\n", len);
				}
	    	}
			else if(FD_ISSET(rtp_server_ctl_blk.udp_server, &writefds))
			{
				
				pMJPEG_Write_Msg = (mjpeg_write_data_t*)OSQPend(rtp_server_ctl_blk.udpsrv_tx_q, 0, &os_err);
				switch(pMJPEG_Write_Msg->msg_id)
				{
					case MJPEG_SEND_EVENT:
						{
							INT32U flag;		// Whether driver is available or not
							mjpeg_wifi_set_write_frame_info(pMJPEG_Write_Msg);
							flag = rtp_check_sending_data();
							if (flag)
							{
								#if 0
								mjpeg_timestamp = OSTimeGet()*900;
								#else
								mjpeg_timestamp += (4*900);
								#endif
								mjpeg_frame_counter_set(1);
								ret = rtp_send_whole_jpeg( (char*)(pMJPEG_Write_Msg->mjpeg_addr), pMJPEG_Write_Msg->mjpeg_size, mjpeg_timestamp);
								mjpeg_service_playjpeg_flag = 1;								
							}
							OSQPost(my_AVIEncodeApQ, (void*)(AVIPACKER_MSG_VIDEO_WRITE_DONE|pMJPEG_Write_Msg->mjpeg_addr_idx));							
						}
						break;
		
					case MJPEG_STOP_EVENT:
						break;

					case MJPEG_NETWORK_BUSY_EVENT:
						break;

					default:		
						break;
				}

			}
		}		
	}

exit_tx_task:
	/* RTP server socket error */
	DBG_PRINT("Exit RTP loop\r\n");
	while(1)
	{
		OSTimeDly(100);
	} 
}

void rtp_start_service(INT32U width, INT32U height)
{	
	INT32S err;	

	if(rtp_server_ctl_blk.udpsrv_rx_buf != NULL)
	{
		DBG_PRINT("rtp_start_service has already started.\r\n");
		return;
	}




	if(rtp_server_ctl_blk.udpsrv_rx_buf == NULL)
	{
		rtp_server_ctl_blk.udpsrv_rx_buf = (char*)gp_malloc(RTP_RX_BUF_SIZE);
		
		if(!rtp_server_ctl_blk.udpsrv_rx_buf)
		{
			DBG_PRINT("RTP rx_buf failed\r\n");
			while(1);
		}
	}	
	
	if(rtp_ctl_blk.rtp_packet_buf == NULL)
	{
		rtp_ctl_blk.rtp_packet_buf = (char*)gp_malloc(RTP_PACKET_SIZE);
		
		if(!rtp_ctl_blk.rtp_packet_buf)
		{
			DBG_PRINT("RTP pkt_buf failed\r\n");
			while(1);
		}
	}	
	
	if(rtp_ctl_blk.rtp_file_data_buf == NULL)
	{
		rtp_ctl_blk.rtp_file_data_buf = (char*)gp_malloc(RTP_DATA_BUF_SIZE);
		
		if(!rtp_ctl_blk.rtp_file_data_buf)
		{
			DBG_PRINT("Malloc rtp_file_data_buf failed\r\n");
			while(1);
		}
	}
	rtp_ctl_blk.rtp_width = width;
	rtp_ctl_blk.rtp_height = height;
	
	rtp_server_ctl_blk.udpsrv_tx_q = OSQCreate(rtp_q_stack, RTPSRV_MSG_QUEUE_LEN);
	
	if(!rtp_server_ctl_blk.udpsrv_tx_q)
	{
		DBG_PRINT("Crate rtp q failed\r\n");
		return;
	}
	
	/* Create RTP task */
	err = OSTaskCreate(rtp_task, NULL, (void *)&rtp_task_stack[RTP_TASK_STACKSIZE-1], RTP_TASK_PRIORITY);
	if(err != OS_NO_ERR)
	{
		DBG_PRINT("rtp_task create failed\r\n");
	}
	else
	{
		DBG_PRINT("rtp_task created ok. pri=%d\r\n", RTP_TASK_PRIORITY);
	}
}

void rtp_stop_service(void)
{
	return;
}

