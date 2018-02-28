/******************************************************
* mjpeg_stream_ameba.h
*
* Purpose: Motion JPEG streaming server --- wifi part
*
* Author: Craig Yang
*
* Date: 2015/10/12
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 
* History :
*
*******************************************************/
#ifndef __MJPEG_STREAM_AMEBA_H__
#define __MJPEG_STREAM_AMEBA_H__
#include "mjpeg_stream.h"

// Return code of mjpeg_wifi_xxx functions
#define MJW_RET_SUCCESS	0
#define MJW_RET_FAIL	(-1)

// Public APIs
extern INT32S mjpeg_wifi_start_server(mjpeg_ctl_t *ctl_blk);
extern INT32S mjpeg_wifi_wait_client_conn(mjpeg_ctl_t *ctl_blk);
extern INT32S mjpeg_wifi_alloc_rx_buff(mjpeg_ctl_t *ctl_blk);
extern void mjpeg_wifi_free_rx_buff(mjpeg_ctl_t *ctl_blk);
extern INT32S mjpeg_wifi_detect_stream_req(mjpeg_ctl_t *ctl_blk);
extern INT32S mjpeg_wifi_write_stream_header(mjpeg_ctl_t *ctl_blk);
extern void mjpeg_wifi_set_write_frame_info(mjpeg_write_data_t *info);
extern INT32S mjpeg_wifi_write_frame_start(mjpeg_ctl_t *ctl_blk);
extern INT32S mjpeg_wifi_write_frame_data(mjpeg_ctl_t *ctl_blk);
extern INT32S mjpeg_wifi_write_frame_end(mjpeg_ctl_t *ctl_blk);
extern void mjpeg_wifi_close_client_conn(mjpeg_ctl_t *ctl_blk);
extern INT32S mjpeg_wifi_del_client_conn(mjpeg_ctl_t *ctl_blk);
extern void mjpeg_wifi_close_server_conn(mjpeg_ctl_t *ctl_blk);
extern INT32S mjpeg_wifi_del_server_conn(mjpeg_ctl_t *ctl_blk);

#endif	//__MJPEG_STREAM_WIFI_H__
