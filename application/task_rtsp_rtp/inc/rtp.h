/******************************************************
* rtsp.h
*
* Purpose: RTP server header file
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
#ifndef __RTP_H__
#define __RTP_H__

PACKED typedef struct RTP_HDR_S
{
	INT16U cc:4;       	/* CSRC count */
    INT16U x:1;        	/* header extension flag */
    INT16U p:1;         	/* padding flag */
    INT16U version:2;   	/* protocol version */
    INT16U pt:7;        	/* payload type */
    INT16U m:1;         	/* marker bit */
    INT16U seq;            /* sequence number */
    INT32U ts;             /* timestamp */
    INT32U ssrc;			/* synchronization source */
} RTP_HDR_T;

PACKED typedef struct JPEG_HDR_S
{
	unsigned int tspec:8;   /* type-specific field */
	unsigned int off:24;    /* fragment byte offset */
    INT8U type;            	/* id of jpeg decoder params */
    INT8U q;               	/* quantization factor (or table id) */
    INT8U width;           	/* frame width in 8 pixel blocks */
    INT8U height;          	/* frame height in 8 pixel blocks */
} JPEG_HDR_T;

PACKED typedef struct JPEG_HDR_RST_S
{
    INT16U dri;
    unsigned int f:1;
    unsigned int l:1;
    unsigned int count:14;
} JPEGHDR_RST_T;

PACKED typedef struct JPEG_HDR_QTABLE_S
{
    INT8U  mbz;
    INT8U  precision;
    INT16U length;
} JPEG_HDR_QTABLE_T;

PACKED typedef struct H264_NALU_S
{
	INT8U f:1;		/* forbidden_zero_bit, must be zero */
	INT8U nri:2;   /* nal reference indicator, 0b11 highest, 0x00 lowest */
    INT8U type:5;	/* NALU type */
} H264_NALU_T;

PACKED typedef struct H264_FU_INDICATOR_S
{
	INT8U f:1;		/* forbidden_zero_bit, must be zero */
	INT8U nri:2;   /* nal reference indicator, 0b11 highest, 0x00 lowest */
    INT8U type:5;	/* NALU type, it is 28(FU-A type)0b11100 */
} H264_FU_INDICATOR_T;

PACKED typedef struct H264_FU_HEADER_S
{
	INT8U s:1;
	INT8U e:1;
	INT8U r:1;
    INT8U type:5;	/* Real NALU type */
} H264_FU_HEADER_T;

PACKED typedef struct RTP_CTL_BLK_S
{
	INT32U rtp_ts;
	INT32U rtp_width;
	INT32U rtp_height;
	INT32U rtp_sample_rate;
	INT32U rtp_ptime;
	INT32U rtp_ssrc;
	INT32U rtp_transfer_size;
	INT32U rtp_send_flag;
	INT32U rtp_file_size;
	INT32U rtp_flie_remain;
    INT16U rtp_sequence;
    char rtp_remote_ip[16];
    char* rtp_packet_buf;
    char* rtp_file_data_buf;
    INT8U rtp_payload_type;
} RTP_CTL_BLK_T;

/* UDP server control block */
typedef struct UDP_SRV_SOCKET_CTL_S
{
	int udp_server;
	OS_EVENT* udpsrv_tx_q;
	char* udpsrv_rx_buf;
	char* udpsrv_tx_buf;
	INT32U	broadcast;
	INT32U	server_port;
} UDP_SRV_SOCKET_CTL_T;

typedef enum
{
	RTP_START_EVENT = 0,
	RTP_STOP_EVENT
} RTP_CBK_E;

enum
{
	RTP_SEND_JPEG_START_PART = 1,
	RTP_SEND_JPEG_DATA_PART,
	RTP_SEND_JPEG_END_PART,
	RTP_SEND_JPEG_FRAME
};

#define RTP_PT_PCMU	0
#define RTP_PT_PCMA	8
#define RTP_HDR_SZ	12
#define RTP_PT_JPEG	26
#define RTP_PT_H264	96
#define RTP_JPEG_RESTART	0x40
#define RTP_JPEG_DATA_BUF_SIZE		(4096)
#define RTP_RX_BUF_SIZE				(1500)
#define RTP_PACKET_SIZE				(1450)
#define RTP_PAYLOAD_SIZE			(RTP_PACKET_SIZE - (sizeof(RTP_HDR_T) + sizeof(JPEG_HDR_T)))    // -12=8=-20
#define RTP_H264_PAYLOAD_SIZE		(RTP_PACKET_SIZE - (sizeof(RTP_HDR_T)))
#define RTPSRV_MSG_QUEUE_LEN			8
#define RTP_JPEG_FRAME_TS			3000
#define RTP_H264_FRAME_TS			4500

#define RTP_DATA_BUF_SIZE		(16*1024)
#define RTP_DATA_AB_BUF_SIZE  	(8*1024)
#define RTP_SESSION_ID		10069888	/* Default session id */
#define RTP_SERVER_PORT		10850	/* default port of RTP */
#define RTP_TASK_STACKSIZE	1024

extern INT32U rtp_send_flag;
extern INT32U rtp_testcnt;
extern RTP_CTL_BLK_T rtp_ctl_blk;

#define JPEG_PAYLOAD	1
#define H264_PAYLOAD	2
#define PCMA_PAYLOAD	3
#define SEND_PAYLOAD	PCMA_PAYLOAD

#define NAL_START_PREFIX_1	0x00
#define NAL_START_PREFIX_2	0x00
#define NAL_START_PREFIX_3	0x00
#define NAL_START_PREFIX_4	0x01
#define NAL_START_PREFIX_LENGTH	4
#define NAL_FU_TYPE				28
#define NAL_FU_LENGTH			2

extern RTP_CTL_BLK_T rtp_ctl_blk;
extern void rtp_rtsp_cbk(INT32U event);
extern void rtp_start_service(INT32U width, INT32U height);
extern void rtp_stop_service(void);
#endif	//__RTP_H__