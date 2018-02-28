/******************************************************
* drv_l2_usbd.h.h
*
* Purpose: USB l2 controller driver/interface
*
* Author: Eugene Hsu
*
* Date: 2011/10/05
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 
* History :
*
*******************************************************/
#ifndef DRV_L2_USBD_H
#define DRV_L2_USBD_H


typedef void (*USBD_L2_STATE_FUN)(INT32U);
typedef void (*USBD_L2_UAC_SET_GAIN_CALLBACK_FUN)(INT32U, INT32S);
typedef void (*USBD_L2_HID_SET_IDLE_CALLBACK_FUN)(INT32U);
/*********************************************************************
        Structure, enumeration and definition
**********************************************************************/
/* Use for storage type, copy from storage.c */
#define		USBD_STORAGE_CF_TYPE		0
#define		USBD_STORAGE_SDC_TYPE	 	1
#define		USBD_STORAGE_MS_TYPE		2
#define		USBD_STORAGE_UHOST_TYPE		3
#define		USBD_STORAGE_USBD_TYPE		4
#define		USBD_STORAGE_NAND_TYPE		5
#define		USBD_STORAGE_XD_TYPE		6
#define		USBD_STORAGE_UHOST_PIN_TYPE	7

/* SETUP data stage direction */
#define    USBD_DATA_DIR_H2D	0
#define    USBD_DATA_DIR_D2H	1

/* Data Direction in handler function */
#define    USBD_DATA_DIR_OUT_HANDLE      0
#define    USBD_DATA_DIR_IN_HANDLE       1

/* SETUP Request type */
#define    USBD_REQ_STANDARD	0
#define    USBD_REQ_CLASS		1
#define    USBD_REQ_VENDOR		2
#define    USBD_REQ_RESERVED	3

/* SETUP Recipient */
#define    USBD_RECIPIENT_STANDARD 	0
#define    USBD_RECIPIENT_CLASS		1
#define    USBD_RECIPIENT_VENDOR	2		
#define    USBD_RECIPIENT_RESERVED	3

/* SETUP standard request type */
#define	   USBD_STDREQ_GET_STATUS			0
#define    USBD_STDREQ_CLEAR_FEATURE		1
#define    USBD_STDREQ_RESERVE_1			2
#define    USBD_STDREQ_SET_FEATURE			3
#define    USBD_STDREQ_RESERVE_2			4
#define    USBD_STDREQ_SET_ADDRESS			5
#define    USBD_STDREQ_GET_DESCRIPTOR		6
#define    USBD_STDREQ_SET_DESCRIPTOR		7
#define    USBD_STDREQ_GET_CONFIGURATION	8
#define    USBD_STDREQ_SET_CONFIGURATION	9
#define    USBD_STDREQ_GET_INTERFACE		10
#define    USBD_STDREQ_SET_INTERFACE		11
#define    USBD_STDREQ_SYNCH_FRAME			12
#define    USBD_STDREQ_MAX_TYPE				13

/* SETUP GET/SET descriptor type */
#define    USBD_DES_DEVICE_TYPE						0x01
#define    USBD_DES_CONFIG_TYPE						0x02
#define    USBD_DES_STRING_TYPE						0x03
#define    USBD_DES_INTERFACE_TYPE					0x04
#define    USBD_DES_ENDPOINT_TYPE					0x05
#define    USBD_DES_DEVICE_QUALIFIER_TYPE			0x06
#define    USBD_DES_OTHER_SPEED_CONFIGURATION_TYPE	0x07
#define    USBD_DES_INTERFACE_POWER_TYPE			0x08

/* for Class request */
#define    USBD_HID_REQ_GET_IDLE					0x02
#define    USBD_HID_REQ_SET_REPORT					0x09
#define    USBD_HID_REQ_SET_IDLE					0x0A
#define    USBD_DES_HID_TYPE						0x21
#define    USBD_DES_REPORT_TYPE						0x22
#define    USBD_REQ_GET_MAX_LUN_TYPE				0xfe
#define    USBD_REQ_MASS_STORAGE_RESET_TYPE			0xff


/* Event type */
/* Event type, base address = 1 */
#define	   USBD_SETUP_CMD_EVENT			        1
#define    USBD_MISC_RESET_EVENT                2
#define    USBD_MISC_SET_CONF_EVENT             3
#define    USBD_MISC_SET_INTERFACE_EVENT        4
#define    USBD_MISC_GET_SUSPEND_EVENT          5
#define    USBD_MISC_GET_RESUM_EVENT            6
#define    USBD_MISC_MSDC_INSERT_EVENT          7
#define    USBD_MISC_GET_INTERFACE_EVENT        8
#define    USBD_MISC_GET_CONF_EVENT             9
#define    USBD_MISC_SET_ADDRESS_EVENT          10
#define    USBD_MISC_SET_FEATURE_EVENT          11
#define    USBD_MISC_CLEAR_FEATURE_EVENT        12
#define    USBD_MISC_GET_STATUS_EVENT           13
#define	   USBD_MISC_CLR_EP0_STALL_EVENT		14
#define	   USBD_MISC_CLR_EPX_STALL_EVENT		15
#define	   USBD_EP_IN_ACK_EVENT		            21
#define    USBD_EP_OUT_READY_EVENT              22
#define    USBD_EP_STATE_CLEAR_EVENT            23
#define	   USBD_EP_OUT_NULL_EVENT				24
#define	   USBD_BULK_IN_ACK_EVENT	            31
#define    USBD_BULK_IN_DMA_DONE_EVENT	        32
#define    USBD_BULK_IN_DMA_TRANS_DONE_EVENT    33
#define	   USBD_BULK_89_IN_ACK_EVENT		 	34
#define	   USBD_BULK_89_IN_DMA_TRANS_DONE_EVENT 35
#define	   USBD_BULK_AB_IN_ACK_EVENT 			36
#define	   USBD_BULK_AB_IN_DMA_TRANS_DONE_EVENT 37
#define    USBD_BULK_OUT_READY_EVENT            38
#define    USBD_BULK_89_OUT_READY_EVENT         39
#define    USBD_BULK_AB_OUT_READY_EVENT         40
#define    USBD_BULK_OUT_DMA_DONE_EVENT	        41
#define    USBD_BULK_OUT_M_DMA_DONE_EVENT	    42
#define    USBD_BULK_OUT_89_M_DMA_DONE_EVENT    43
#define    USBD_BULK_OUT_AB_M_DMA_DONE_EVENT    44
#define    USBD_ISO_IN_DMA_DONE_EVENT           51
#define    USBD_ISO_IN_PKT_CLEAR_EVENT          52
#define    USBD_ISO_IN_EP5_DMA_DONE_EVENT       53
#define    USBD_ISO_IN_EP5_PKT_CLEAR_EVENT      54
#define    USBD_ISO_IN_EP7_DMA_DONE_EVENT		55
#define    USBD_ISO_IN_EP7_PKT_CLEAR_EVENT      56
#define    USBD_ISO_OUT_PKT_READY_EVENT         57
#define    USBD_ISO_OUT_DMA_DONE_EVENT          58
#define    USBD_ISO_VIDEO_IN_PKT_CLEAR_EVENT    59
#define    USBD_ISO_VIDEO_IN_DMA_DONE_EVENT     60
#define    USBD_INT_IN_NACK_EVENT               61 
#define    USBD_INT_IN_PACKET_CLEAR_EVENT       62 
#define    USBD_INT_EP3_IN_PACKET_CLEAR_EVENT   63 
#define    USBD_INT_EP4_IN_PACKET_CLEAR_EVENT   64 
#define    USBD_INT_EP6_IN_PACKET_CLEAR_EVENT   65 
#define    USBD_STORAG_SDC_PLUG_IN_EVENT        71
#define    USBD_STORAG_SDC_PLUG_OUT_EVENT       72
#define    USBD_STORAG_CF_PLUG_IN_EVENT         73
#define    USBD_STORAG_CF_PLUG_OUT_EVENT        74

/* Transfer type */
#define	   USBD_XFER_CONTROL		0
#define	   USBD_XFER_BULK			1
#define	   USBD_XFER_ISO			2
#define    USBD_XFER_INT			3
#define    USBD_XFER_MISC			4
#define    USBD_XFER_MAX			USBD_XFER_MISC

/* Control transfer function index */
#define    USBD_XFER_CONTROL_EP0_SETUP	0
#define    USBD_XFER_CONTROL_EP0_IN		1
#define    USBD_XFER_CONTROL_EP0_OUT	2
#define    USBD_XFER_CONTROL_MAX		USBD_XFER_CONTROL_EP0_OUT			

/* Bulk transfer function index */
#define	   USBD_XFER_BULK_IN		0
#define    USBD_XFER_BULK_OUT		1
#define    USBD_XFER_BULK_DMA_IN	2
#define    USBD_XFER_BULK_DMA_OUT	3
#define    USBD_XFER_BULK_MAX		USBD_XFER_BULK_DMA_OUT

/* ISO transfer function index */
#define	   USBD_XFER_ISO_IN			0
#define    USBD_XFER_ISO_OUT		1
#define	   USBD_XFER_ISO_MAX		USBD_XFER_ISO_OUT

/* Interrupt transfer function index */
#define    USBD_XFER_INT_IN			0	
#define    USBD_XFER_INT_OUT		1
#define    USBD_XFER_INT_MAX		USBD_XFER_INT_OUT

/* Gereral function index */
#define    USBD_XFER_MISC_HANDLE		    		0
#define    USBD_XFER_MISC_MAX					    USBD_XFER_MISC_HANDLE

/* Decriptor type for registration */
#define  REG_DEVICE_DESCRIPTOR_TYPE             0
#define  REG_CONFIG_DESCRIPTOR_TYPE             1
#define  REG_DEVICE_QUALIFIER_DESCRIPTOR_TYPE   2
#define  REG_REPORT_DESCRIPTOR_TYPE             3       //HID class
#define  REG_STRING0_DESCRIPTOR_TYPE            7
#define  REG_STRING1_DESCRIPTOR_TYPE            8
#define  REG_STRING2_DESCRIPTOR_TYPE            9
#define  REG_STRING3_DESCRIPTOR_TYPE            10
#define  REG_STRING4_DESCRIPTOR_TYPE            11
#define  REG_STRING5_DESCRIPTOR_TYPE            12

/* Definitions of interface number for UAC+HID device */
#define USBD_UAC_CTRL_INTF		0x00
#define USBD_UAC_STREAM_INTF 	0x01
#define USBD_UAC_MIC_INTF		0x02
#define USBD_HID_INTF			0x03
#define USBD_UAC_MIC_UID		0x05
#define USBD_UAC_SPEAKER_UID	0x06

/* Definitions of UAC request code */
#define USBD_UAC_REQ_SET_CUR_CODE	0x01
#define USBD_UAC_REQ_GET_CUR_CODE	0x81
#define USBD_UAC_REQ_SET_MIN_CODE	0x02
#define USBD_UAC_REQ_GET_MIN_CODE	0x82
#define USBD_UAC_REQ_SET_MAX_CODE	0x03
#define USBD_UAC_REQ_GET_MAX_CODE	0x83
#define USBD_UAC_REQ_SET_RES_CODE	0x04
#define USBD_UAC_REQ_GET_RES_CODE	0x84
#define USBD_UAC_REQ_SET_MEM_CODE	0x05
#define USBD_UAC_REQ_GET_MEM_CODE	0x85
#define USBD_UAC_REQ_GET_STAT_CODE	0xFF


/* Definitions of Feature Unit Control Selectors */
#define FU_MUTE_CONTROL		0x01
#define FU_VOLUME_CONTROL	0x02
#define FU_BASS_CONTROL		0x03
#define FU_MID_CONTROL		0x04

/* SETUP packet message */
typedef struct _USBD_SETUP_MSG
{
    INT8U   bmRequestType;
    INT8U   bRequest;
    INT16U  wValue;
    INT16U  wIndex;
    INT16U  wLength;
} USBD_SETUP_MSG;

typedef struct _USBD_CONTROL
{
    INT8U *   dev_desptr;
    INT8U *   config_desptr;
    INT8U *   dev_qualifier_desptr;
    INT8U *   report_desptr;
    INT8U *   str0_desptr;
    INT8U *   str1_desptr;
    INT8U *   str2_desptr;
    INT8U *   str3_desptr;
    INT8U *   str4_desptr;
    INT8U *   str5_desptr;
    INT8U *    dataptr;	
    INT32U   datacnt;
    USBD_SETUP_MSG     setupmsg;
    INT8U   ep0_sendnull;
    INT8U	cur_interface_num;
} USBD_CONTROL;

/***********************Get bmRequestType **************************/
#define  GetSetupBMReqType(w,x,y,z) \
    x = ((w & 0x80) >> 7);  \
    y = ((w & 0x60) >> 5);  \
    z = (w & 0x1f);
#define  GetDescriptorbywVlaue(x,y,z)   \
    y = (x >> 8);   \
    z = (x & 0x00ff);

/**********************Get/Set message from/to L1 *************************/
#define USBD_L2_XFER_TYPE_OFFSET    24
#define USBD_L2_STATE_OFFSET        16
#define USBD_L2_MSG_NULL    0xffffffff

#define USBDGetMsg(w,x,y,z)   \
    x = ((w & 0xff000000) >> USBD_L2_XFER_TYPE_OFFSET); \
    y = ((w & 0x00ff0000) >> USBD_L2_STATE_OFFSET);     \
    z = (w & 0x0000ffff);

#define USBDSetMsg(w,x,y,z)  (w |= (((x << USBD_L2_XFER_TYPE_OFFSET) & 0xff000000) | ((y << USBD_L2_STATE_OFFSET)& 0x00ff0000)  \
                              | (z & 0x0000ffff)))

#define USBD_RET_SUCCESS    0
#define USBD_RET_ERROR     -1
/*********************************************************************
        Functions declaration
**********************************************************************/
extern INT8U Device_Descriptor_TBL[];
extern const INT8U Qualifier_Descriptor_TBL[];
extern INT8U Config_Descriptor_TBL[];
extern const INT8U String0_Descriptor[];
extern const INT8U String1_Descriptor[];
extern const INT8U String2_Descriptor[];
extern USBD_CONTROL ctl_ctlblk;

extern INT32S drv_l2_usbd_ctl_init(void);
extern INT32S drv_l2_usbd_ctl_uninit(void);
extern void drv_l2_usbd_main(void *param);
extern void usbd_l2_main_task_exit(void);
extern void drv_l2_usbd_ctl_reset(void);
extern void drv_l2_usbd_register_state_handler(INT8U transfer, USBD_L2_STATE_FUN setup, USBD_L2_STATE_FUN in, USBD_L2_STATE_FUN out);
extern void drv_l2_usbd_l2_run_handle(INT32U msg);
extern void drv_l2_usbd_get_ctl_blk(USBD_CONTROL* ctl);
extern void drv_l2_usbd_set_ctl_blk(USBD_CONTROL* ctl);
extern void drv_l2_usbd_register_descriptor(INT8U type, INT8U * desptr);
extern void drv_l2_usbd_set_interface_num(INT8U num);
extern void drv_l2_usbd_get_setup_token(void);
extern void drv_l2_usbd_process_setup_request(void);
extern INT8U drv_l2_usbd_check_ep0_state_stage(INT8U datadir);
extern void _check_ep0_data_send_zero(void);
/**** L2 state machine handle ******/
/* Control transfer state */
extern void drv_l2_usbd_state_ctl_ep0_setup(INT32U event);
extern void drv_l2_usbd_state_ctl_ep0_in(INT32U event); 
extern void drv_l2_usbd_state_ctl_ep0_out(INT32U event);

/* Miscellaneous state */
extern void drv_l2_usbd_misc_handle(INT32U event);
#endif  //DRV_L2_USBD_H
