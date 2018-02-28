/******************************************************
* wifi_abstraction_layer_ameba.h
*
* Purpose: WiFi abstraction layer. Applications using WiFi functions should
*          call APIs in this file instead of the APIs of a specific vendor.
*
* Date: 2015/10/12
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 
* History :
*
*******************************************************/
#ifndef __WIFI_ABSTRACTION_LAYER_AMEBA_H__
#define __WIFI_ABSTRACTION_LAYER_AMEBA_H__

// Return code of wifi abstraction layer functions
#define WAL_RET_SUCCESS	0
#define WAL_RET_FAIL	(-1)

// Typedef
typedef void *netconn_ptr;
typedef void *netbuf_ptr;

// APIs
extern INT32S wal_init(INT32S argc, CHAR *argv[]);
extern void wal_set_ap_channel(INT8U channel);
extern void wal_set_ap_ssid(CHAR *ssid, INT8U ssid_len);
extern void wal_set_ap_password(CHAR *password, INT8U password_len);
extern void wal_ldo_enable(void);
extern void wal_ldo_disable(void);
extern INT32S wal_ap_mode_enable(void);
extern INT32S wal_ap_mode_disable(void);

#endif	//__WIFI_ABSTRACTION_LAYER_AMEBA_H__
