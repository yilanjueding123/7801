/******************************************************
* rtsp.h
*
* Purpose: RTSP header file
*
* Author: Eugene Hsu
*
* Date: 2016/03/21
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 
* History :
*
*******************************************************/
#ifndef __RTSP_H__
#define __RTSP_H__

#define LINE_END		"\r\n"
#define DESCRIBE_METHOD "DESCRIBE"
#define OPTION_METHOD	"OPTIONS"
#define SETUP_METHOD	"SETUP"
#define PLAY_METHOD		"PLAY"
#define PAUSE_METHOD	"PAUSE"
#define TEARDOWN_METHOD	"TEARDOWN"

#define RTSP_FILE_PATCH			"?action=stream"
#define RTSP_METHOD_URL			"rtsp:"
#define RTSP_CSEQ_HEAD			"CSeq: "
#define RTSP_TRANSPORT_HEAD 	"Transport: "
#define RTSP_CLIENT_PORT_FIELD	"client_port="
#define RTSP_SPACE		" "

#define RTSP_URL_LEN	64
#define RTSP_CSEQ_LEN	12
#define RTSP_IP_LEN		4

#define RTSP_OPTION_REPLY2		"Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE\r\n"

#define RTSP_DESCRIBE_REPLY2	"Content-type: application/sdp\r\nContent-Base: rtsp://"

#define RTSP_SDP_JPEG_REPLY1	"i=goplus\r\n"
#define RTSP_SDP_JPEG_REPLY2	"t=0 0\r\nm=video 0 RTP/AVP 26\r\n"
#define RTSP_SDP_JPEG_REPLY3	"a=rtpmap:26 JPEG/90000\r\na=control:streamid=0\r\na=framerate:30\r\n"

#define RTSP_SDP_H264_REPLY1	"i=goplus\r\n"
#define RTSP_SDP_H264_REPLY2	"t=0 0\r\nm=video 0 RTP/AVP 96\r\n"
#define RTSP_SDP_H264_REPLY3	"a=rtpmap:96 H264/90000\r\na=control:streamid=0\r\na=fmtp:96 packetization-mode=0\r\n"

#define RTSP_SDP_AUDIO_REPLY1	"i=goplus\r\n"
#define RTSP_SDP_AUDIO_REPLY2	"t=0 0\r\nm=audio 0 RTP/AVP "
#define RTSP_SDP_AUDIO_REPLY3	"a=control:streamid=0\r\n"

#define RTSP_SETUP_REPLY2		"Transport: RTP/AVP/UDP;unicast;"

#define RTSP_PLAY_REPLY1		"RTSP/1,0 200 OK\r\nCseq: "

#define RTSP_SERVER_PORT	8080
#define RTCP_SERVER_PORT	10851	/* default port of RTCP */
#define RTSP_RX_BUF_LEN	(1600)
#define RTSP_TX_BUF_LEN	(1500)
#define RTSP_QUEUE_LEN	16
#define RTSP_TASK_STACKSIZE	1024

/* TCP server control block */
typedef struct TCP_SRV_SOCKET_CTL_S
{
	int tcp_server;
	int tcp_client;
	OS_EVENT* tcpsrv_tx_q;
	char* tcpsrv_rx_buf;
	char* tcpsrv_tx_buf;
	unsigned short	server_port;
} TCP_SRV_SOCKET_CTL_T;

PACKED typedef struct RTSP_INSTANCE_CTL_BLK_S
{
	char local_ip[RTSP_IP_LEN];		/* Local IP address */
	char connect_ip[RTSP_IP_LEN];	/* Connection IP address */
	char rtsp_url[RTSP_URL_LEN];
	int cseqnum;
	INT32U session_id;
	int server_rtp_port1;
	int server_rtp_port2;
	int client_rtp_port1;
	int client_rtp_port2;
} RTSP_INSTANCE_CTL_BLK_T;

typedef enum
{
	RTSP_METHOD_START = 0,
	RTSP_DESCRIBE_METHOD,
	RTSP_OPTION_METHOD,
	RTSP_SETUP_METHOD,
	RTSP_PLAY_METHOD,
	RTSP_PAUSE_METHOD,
	RTSP_TEARDOWN_METHOD,
	RTSP_ERROR_METHOD
} RTSP_METHOD_E;

extern RTSP_INSTANCE_CTL_BLK_T rtsp_instance;
#endif	//__RTSP_H__