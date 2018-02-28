/******************************************************
* drv_l2_usbd_msdc.h
*
* Purpose: usb controller L2 msdc driver/interface
*
* Author: Eugene Hsu
*
* Date: 2011/10/24
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 
* History :
*
*******************************************************/
#ifndef DRV_L2_USBD_MSDC_H
#define DRV_L2_USBD_MSDC_H

/******************************************************
    Definition and structure
******************************************************/
typedef void (*SCSI_VENDOR_CMD_FN)(INT16U); /* for vendor command usage */
typedef INT32S (*SCSI_VENDOR_READ_WRITE_FN)(INT32U, INT32U, void*, void*); /* for vendor command read/write, including NAND, SPI, OTP */
typedef void (*SCSI_VENDOR_CHECK_FN)(void);  /* for vendor check */

#define USBD_MSDC_HIDKEY_DEVICE		1

#if (USBD_MSDC_HIDKEY_DEVICE == 1)
#define CONFIG_DES_TOTAL_INF	0x02	/* MSDC + HID */
#define CONFIG_DES_TOTAL_LEN	0x39
#define USBD_HID_KEYBOARD_INTF	0x01
#define USBD_HID_REPORT_DES_LEN	0x3F	/* The length of REPORT descriptor */

/* keyborad definition */
#define USBD_HID_KEY_A		0x04
#define USBD_HID_KEY_B		0x05
#define USBD_HID_KEY_C		0x06
#define USBD_HID_KEY_D		0x07
#define USBD_HID_KEY_E		0x08
#define USBD_HID_KEY_F		0x09
#define USBD_HID_KEY_G		0x0A
#define USBD_HID_KEY_H		0x0B
#define USBD_HID_KEY_UP		0x52
#define USBD_HID_KEY_DOWN	0x51
#define USBD_HID_KEY_LEFT	0x50
#define USBD_HID_KEY_RIGHT	0x4F
#define USBD_HID_KEY_1		0x1E
#define USBD_HID_KEY_2		0x1F
#define USBD_HID_KEY_3		0x20
#define USBD_HID_KEY_4		0x21
#define USBD_HID_KEY_5		0x22
#define USBD_HID_KEY_6		0x23
#define USBD_HID_KEY_7		0x24
#define USBD_HID_KEY_8		0x25

#else
#define CONFIG_DES_TOTAL_INF	0x01	/* MSDC */
#define CONFIG_DES_TOTAL_LEN	0x20
#endif

#define USBD_STORAGE_NO_WPROTECT	0
#define USBD_STORAGE_WPROTECT		1

/* LUN number, 0~7, total is 8 lun */
#define LUN_NUM_0	0
#define	LUN_NUM_1	1
#define LUN_NUM_2	2
#define LUN_NUM_3	3
#define LUN_NUM_4   4
#define LUN_NUM_5	5
#define LUN_NUM_6	6
#define LUN_NUM_7	7

/* GP17's file system only support 5 storages, NAND/SD/CDROM/NOR/USBH */
#define LUN_NF_TYPE		    0
#define LUN_SDC_TYPE   	    1
#define LUN_CDROM_TYPE 	    2
#define LUN_NOR_TYPE   	    3
#define LUN_RAM_DISK_TYPE 	4

#define BOT_CBW_LEN 			31
#define BOT_CBWCB_LEN 			16    /* CBWCB length */
#define BOT_CSW_LEN 			13
#define BOT_CBW_CBWCB_OFFSET 	15

#define BOT_CBW_SIGN1   0x55
#define BOT_CBW_SIGN2   0x53
#define BOT_CBW_SIGN3   0x42
#define BOT_CBW_SIGN4   0x43

#define BOT_CSW_SIGN1   0x55
#define BOT_CSW_SIGN2   0x53
#define BOT_CSW_SIGN3   0x42
#define BOT_CSW_SIGN4   0x53

#define BOT_TEMP_DATA_LEN   512     /* less than 512 for none DMA mode */

/**** define for bulk IN/OUT DMA buffer transport *****/
#define BOT_DMA_RAM_BUF_ORDER   8   /* 0=512, 1=1024, 2=2048, 3=4096, 4=8192 5=16384*/
#define BOT_DMA_RAM_BUF_SCALE   (1 << BOT_DMA_RAM_BUF_ORDER)
#define BOT_DMA_RAM_BUF_SIZE     (MSDC_BLOCK_SIZE * BOT_DMA_RAM_BUF_SCALE)
#define BOT_DMA_BUF_NUM   2         /* Now support 2 DMA AB buffer */
#define MSDC_BLOCK_SIZE   512
#define CBW_DEVICE_BLOCK_SIZE   512
#define MAX_MSDC_LUN_NUM 5               /* For maximum LUN number */

/* SCSI Inquiry length definition */
#define BOT_SCSI_InquiryLength 44 

/* Bulk only transport stage */
#define BULK_CBW_STA		0
#define BULK_DATA_STA	    1
#define BULK_CSW_STA		2

#define	CBW_DATA_DIR_OUT	0
#define CBW_DATA_DIR_IN		1

/* For CBW command status */
#define	CSW_CMD_PASS_STA		0x00
#define	CSW_CMD_FAILED_STA 		0x01
#define CSW_CMD_PHASE_ERROR_STA	0x02

/* For SCSI command opcode */
#define SCSI_TESTUNITREADY_OPCODE		0x00
#define SCSI_REQUSETSENSE_OPCODE			0x03
#define SCSI_INQUIRY_OPCODE				0x12
#define SCSI_MODESENSE6_OPCODE			0x1A
#define SCSI_STARTSTOP_OPCODE			0x1B
#define SCSI_MEDIUMREMOVAL_OPCODE		0x1E
#define SCSI_READFORMATCAPACITY_OPCODE	0x23
#define SCSI_READCAPACITY_OPCODE			0x25
#define SCSI_READ10_OPCODE				0x28
#define SCSI_WRITE10_OPCODE				0x2A
#define SCSI_VERIFY_OPCODE				0x2F
#define SCSI_READTOC_OPCODE				0x43
#define SCSI_MODESENSE10_OPCODE			0x5A
#define SCSI_GP_VENDOR_START_OPCODE      0xF0
#define SCSI_GP_VENDOR_END_OPCODE        0xFF

/* For LUN status */
#define LUNSTS_NORMAL				0x0000
#define LUNSTS_NOMEDIA				0x0001
#define LUNSTS_WRITEPROTECT			0x0002
#define LUNSTS_MEDIACHANGE			0x0004
#define LUNSTS_START_STOP_NOMEDIA	0xFFAA

/* For DMA buffer state */
#define SCSI_DMA_BUF_NOT_USED_STA    0
#define SCSI_DMA_BUF_READING_STA     1
#define SCSI_DMA_BUF_READ_DONE_STA   2    
#define SCSI_DMA_BUF_WRITING_STA     3    
#define SCSI_DMA_BUF_WRITE_DONE_STA  4    
#define SCSI_DMA_BUF_INING_STA       5
#define SCSI_DMA_BUF_IN_DONE_STA     6
#define SCSI_DMA_BUF_OUTING_STA      7
#define SCSI_DMA_BUF_OUT_DONE_STA    8

/* For USB MSDC to check status of storage */
#define STORAGE_CHECK_READY          0
#define STORAGE_CHECK_NOT_READY      1
#define STORAGE_CHECK_FAILED         2

/* For SCSI read/write state */
#define DO_SCSI_NONE           0     
#define DO_SCSI_READ_10        1
#define DO_SCSI_WRITE_10       2
#define DO_SCSI_VENDOR_READ    3
#define DO_SCSI_VENDOR_WRITE   4

/* For GP SCSI vendor cmd state */
#define GP_VENDER_NO_AUTH       0
#define GP_VENDER_START_AUTH    1
#define GP_VENDER_OK_AUTH       2

#define DMA_BULK_IN_MODE	1
#define DMA_BULK_OUT_MODE	2

typedef struct _STORAGE_DRV_INFO
{
	INT32U nSectors;
	INT32U nBytesPerSector;
} STORAGE_DRV_INFO;

typedef struct _MDSC_LUN_STORAGE_DRV
{
    void* 	priv;
    INT32S	(*usbd_msdc_init)(void* priv);
    INT32S	(*usbd_msdc_uninit)(void *priv);
    INT32S	(*usbd_msdc_insert_event)(void* priv);
    void 	(*usbd_msdc_getdrvinfo)(void* priv, STORAGE_DRV_INFO* info);
    INT32S	(*usbd_msdc_read_cmd_phase)(void* priv, INT32U lba,INT32U seccount);
    INT32S	(*usbd_msdc_read_dma_phase)(void* priv, INT32U *buf,INT8U ifwait,INT32U seccount);
    INT32S 	(*usbd_msdc_read_cmdend_phase)(void* priv);
    INT32S	(*usbd_msdc_write_cmd_phase)(void* priv, INT32U lba,INT32U seccount);
    INT32S	(*usbd_msdc_write_dma_phase)(void* priv, INT32U *buf,INT8U ifwait,INT32U seccount);
    INT32S 	(*usbd_msdc_write_cmdend_phase)(void* priv);
    INT32S 	(*usbd_msdc_storage_dma_check_state)(void* priv);
} MDSC_LUN_STORAGE_DRV;


typedef struct _MSDC_LOGIC_UNIT 
{
    INT32U  senseidx;               /* Indicate which sense code in the table */              
    INT32U 	secSize;		        /* flash size sector */
    INT32U  wprotect;				/* write protect */
    INT16U 	status; 			    /* Status, write protect, no media, media change. */
    INT8U   isused;                 /* This lun is set by application */
    INT8U   luntype;                     /* not useful now */   
    MDSC_LUN_STORAGE_DRV storageaccess;  /* storage access functions */
} MSDC_LOGIC_UNIT;

typedef struct _USBD_MSDC_DMA_BUF
{
    INT8U * dataptr;
    struct _USBD_MSDC_DMA_BUF* next;
    INT32U usedbuffsize;            /* How many bytes of this RAM buffer are used */
    volatile INT32U DMABufState;
} USBD_MSDC_DMA_BUF;

typedef struct _SCSI_SENSE_SET
{
    INT8U sensekey;
    INT8U asc;
    INT8U ascq;
    INT8U Reserve;           /* for structure alignment */
} SCSI_SENSE_SET;

typedef union _CBW_CB
{
    INT8U cbw_raw[BOT_CBW_LEN];
    struct CBW
    {
        INT8U dCBWSignature[4];
        INT8U dCBWTag[4];
        INT8U dCBWDataTransferLength[4];
        INT8U bmCBWFlags;
        INT8U bCBWLUN;
        INT8U bCBWCBLength;
        INT8U CBWCB[BOT_CBWCB_LEN];
    } cbw_cb;
} CBW_CB;

typedef struct _CSW_CB
{
    INT32U dCSWDataResidue;
    INT8U bCSWStatus;   /* Passed, failed, error */
    INT8U reserve[3];   /* for structure alignment */
} CSW_CB;

typedef struct _USBD_MSDC_CTL_BLK
{
    USBD_MSDC_DMA_BUF* dmaptr;      /* for DMA usage */
    MSDC_LOGIC_UNIT*  lunptr;       /* for LUN usage */
    INT8U *   dataptr;                /* for SCSI command usage */
    INT32U  rambufcnt;				/* how many DMA ram buffer for standard SCSI command */
    INT32U  rambufres;				/* not enough for a DMA ram buffer for standard SCSI command */
    INT32U  secnum;
    INT32U  cbwdatatcnt;
    INT32U  cswdataresidue;
    INT8U *   SCSIinquirydataptr;     /* for SCSI inquiry data */
    INT8U *   SCSIinquiryCDdataptr;   /* for SCSI inquiry data, CDROM */
    volatile INT32U   doingSCSIreadwrite;
    volatile INT8U   prostate;      /* CBW, Data, CSW */
    INT8U   senseidx;               /* SCSI sense index */
    INT8U   nodmadata;
    INT8U   maxlun;                 /* maximum lun number, 0 base */
    CBW_CB  cbw;                    /* CBW control */
    CSW_CB  csw;                    /* CSW control */
    SCSI_VENDOR_CMD_FN    process_scsi_vendor_cmd;
} USBD_MSDC_CTL_BLK;

typedef struct _USBD_MSDC_GP_VENDOR_CTL_BLK
{
    SCSI_VENDOR_READ_WRITE_FN   vnedor_read_fn;
    SCSI_VENDOR_READ_WRITE_FN   vnedor_write_fn;
    SCSI_VENDOR_CHECK_FN   vnedor_check_fn;
    INT8U*  vendordataptr;
    INT32U  isauthrized; 
    INT32U  scsi_vendor_lba;
    INT32U  scsi_vendor_len;
} USBD_MSDC_GP_VENDOR_CTL_BLK;

/******************************************************
    Variables, functions definitions
******************************************************/
extern const INT8U scsi_inquirydata[];
extern const INT8U scsi_inquirydata_CDROM[];
extern MSDC_LOGIC_UNIT* drv_l2_usbd_msdc_get_lun_by_type(INT8U type);
extern INT8U drv_l2_usbd_msdc_get_max_lun(void);
extern INT8S drv_l2_usbd_msdc_check_lun_status(void);
extern INT32S drv_l2_usbd_msdc_init(void);
extern INT32S drv_l2_usbd_msdc_set_lun(INT8U type, INT8U lunnum, INT32U wprotect, const MDSC_LUN_STORAGE_DRV* plun);
extern INT32S drv_l2_usbd_msdc_reset_lun(INT8U lunnum);
extern INT32S drv_l2_usbd_msdc_get_cbw_data(void);
extern void drv_l2_usbd_msdc_uninit(void);
extern void drv_l2_usbd_msdc_reset(void);
extern void drv_l2_usbd_msdc_send_csw_data(void);
extern void drv_l2_usbd_msdc_do_read_scsi_cmd(void);
extern void drv_l2_usbd_msdc_get_read_lba_len(INT32U* lba, INT32U* len, INT8U type);
extern void drv_l2_usbd_msdc_process_storage(void);
extern void drv_l2_usbd_msdc_process_insertion(void);
extern void drv_l2_usbd_msdc_check_start_stop_unit(void);
extern void drv_l2_sbd_msdc_register_scsi_inquiry_data(INT8U * dataptr, INT8U * cddataptr);
extern void drv_l2_usbd_msdc_register_scsi_vendor_cmd_fn(SCSI_VENDOR_CMD_FN fn);

extern USBD_MSDC_CTL_BLK msdc_ctlblk;
extern USBD_MSDC_DMA_BUF msdc_dma_buf[];   /* for BULK DMA IN/OUT */

/* msdc state handler */
extern void drv_l2_usbd_msdc_state_bot_in(INT32U event);
extern void drv_l2_usbd_msdc_state_bot_out(INT32U event);

/* SCSI command handler */
extern void drv_l2_usbd_scsi_command_process(void);
extern void drv_l2_usbd_scsi_cmd_inquiry(void);
extern void drv_l2_usbd_scsi_cmd_testunitready(void);
extern void drv_l2_usbd_scsi_cmd_modesense6(void);
extern void drv_l2_usbd_scsi_cmd_read10(void);
extern void drv_l2_usbd_scsi_cmd_write10(void);
extern void drv_l2_usbd_scsi_cmd_mediumremoval(void);
extern void drv_l2_usbd_scsi_cmd_requestsense(void);
extern void drv_l2_usbd_scsi_cmd_readcapacity(void);
extern void drv_l2_usbd_scsi_cmd_formatcapacity(void);
extern void drv_l2_usbd_scsi_cmd_verify(void);
extern void drv_l2_usbd_scsi_cmd_startstop(void);

/* SCSI vendor command handler */
extern void drv_l2_usbd_scsi_vendor_cmd(INT16U cmdval);
extern void drv_l2_usbd_msdc_porcess_vendor_readwrite(void);
extern void drv_l2_usbd_msdc_register_vendor_read_write_fn(SCSI_VENDOR_READ_WRITE_FN read, SCSI_VENDOR_READ_WRITE_FN write);
extern INT32S drv_l2_usbd_msdc_gp_vendor_ram_read(INT32U addr, INT32U len, void* buf, void* pri);
extern INT32S drv_l2_usbd_msdc_gp_vendor_ram_write(INT32U addr, INT32U len, void* buf, void* pri);
extern void gp_scsi_vendor_cmd_set_vendor_ID(void);
extern void gp_scsi_vendor_cmd_get_driver_stauts(void);
extern void gp_scsi_vendor_cmd_get_ic_version(void);
extern void gp_scsi_vendor_cmd_goto_ram(void);
extern void gp_scsi_vendor_cmd_reg_read(void);
extern void gp_scsi_vendor_cmd_reg_write(void);
extern void gp_scsi_vendor_cmd_read(void);
extern void gp_scsi_vendor_cmd_write(void);

extern void scsi_send_stall_null(void);
extern void scsi_command_fail(INT32U senseidx);
#endif// end of DRV_L2_USBD_MSDC_H
