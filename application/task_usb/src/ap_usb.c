
/*********************************************************************
    Include file
**********************************************************************/
#include <stdio.h>
#include "project.h"

#include "driver_l2.h"
#include "driver_l1_cfg.h"
#include "drv_l1_sfr.h"
#include "gplib_print_string.h"

#include <string.h>
#include "drv_l1_usbd.h"
#include "drv_l2_usbd.h"
#include "drv_l2_usbd_msdc.h"
#include "drv_l2_usbd_uvc.h"

#include "application.h"
#include "ap_state_handling.h"
#include "my_avi_encoder_state.h"
#include "fs_driver.h"

//////////////////////////////////////////////////////////////////////
//				           M    S    D    C                         //
//////////////////////////////////////////////////////////////////////

extern INT8U screen_saver_enable;

/******************************************************
    Extern functions variables declaration
*******************************************************/
//MSDC descriptor data
extern INT8U Default_Device_Descriptor_TBL[];
extern INT8U Default_Qualifier_Descriptor_TBL[];
extern INT8U Default_Config_Descriptor_TBL[];
extern INT8U Default_String0_Descriptor[];
extern INT8U Default_String1_Descriptor[];
extern INT8U Default_String2_Descriptor[];
extern INT8U Default_scsi_inquirydata[];
extern INT8U Default_scsi_inquirydata_CDROM[];

/*****************************************************
    USBD API
    Purpose: For upper layer application 
            or other system module.
*****************************************************/
#define RETRY_NUM 5
#define SD_OUT_SYM	0xFFFFFFFF

enum {
	SD_REMOVE = 0,
	SD_INSERT
};

static OS_EVENT *rw_sem = NULL;
static INT32U sd_status = SD_REMOVE;
static INT32U sd_status_bak = SD_REMOVE;
static INT32U ramcurlba = SD_OUT_SYM;

//static INT8S *gRamDiskPtr = NULL;
//#define	C_RAMDISK_SIZE		0x400000     //4M bytes
#define C_RAMDISK_SECTOR_NUM    (C_RAMDISK_SIZE >> 9)
//ALIGN4 static INT8S ramdiskbuf[C_RAMDISK_SIZE];

/*****************************************************
    Functions API
*****************************************************/
static INT32S usbd_delay(INT32S num)
{
	int i;
	for (i=0;i<num;++i)
		R_RANDOM0 = i;
	return 0;
}

static INT32S SDCARD_Initial_Storage(void* priv)
{
	INT32S ret = 0;
	INT8U err;		

	if (sd_status == SD_REMOVE)  // 在 SDCARD_Insertion 可能已經 init SD
	{
		OSSemPend(rw_sem, 0, &err); 	// 讓 SD 卡同時只被一個線程存取 	
		ret = drvl2_sd_init();
		OSSemPost(rw_sem);
		if (ret==0) {
			DBG_PRINT("[MSDC Mode] SD Card Insert\r\n");
			sd_status = SD_INSERT;
		}
	}

	ramcurlba = SD_OUT_SYM;

	return ret;
}

static INT32S SDCARD_Uninitial_Storage(void* priv)
{
	INT8U err;	

	 OSSemPend(rw_sem, 0, &err);	 // 讓 SD 卡同時只被一個線程存取 
	 drvl2_sd_card_remove();
	 OSSemPost(rw_sem);
	 
	 return 0;
}

static void SDCARD_GetDrvInfo_Storage(void* priv, STORAGE_DRV_INFO *info)
{
	INT32U size;
	INT8U err;	

	OSSemPend(rw_sem, 0, &err); 	// 讓 SD 卡同時只被一個線程存取 
	size = drvl2_sd_sector_number_get();
	OSSemPost(rw_sem);

	DBG_PRINT("total sector %d\r\n", size);
	info->nSectors = size;
	info->nBytesPerSector = 512;
}

static INT32S SDCARD_ReadSectorDMA(void* priv, INT32U *buf,INT8U ifwait,INT32U seccount)
{	
	INT32S ret;
	INT32S i;
	INT8U err;
	
	for (i=0;i<RETRY_NUM;++i)
	{
		OSSemPend(rw_sem, 0, &err);		// 讓 SD 卡同時只被一個線程存取	
		ret = drvl2_sd_read(ramcurlba, buf, seccount);
		OSSemPost(rw_sem);
		if (ret == 0) {
			break;			
		}
		usbd_delay(0x1);
	}

    if(ret == 0)
    {
        //DBG_PRINT("Read lba addr %d\r\n", ramcurlba);
		ramcurlba += seccount;		
        return 0;    
    }
    else
    {
    	DBG_PRINT("Read lba(%d) return -1\r\n", ramcurlba);
		ramcurlba = SD_OUT_SYM;    
		sd_status = SD_REMOVE;		
        return -1;
    }
}

static INT32S SDCARD_WriteSectorDMA(void* priv, INT32U *buf, INT8U ifwait, INT32U seccount)
{
	INT32S ret;
	INT32S i;
	INT8U err;

	for (i=0;i<RETRY_NUM;++i)
	{	
		OSSemPend(rw_sem, 0, &err);		// 讓 SD 卡同時只被一個線程存取
		ret = drvl2_sd_write(ramcurlba, buf, seccount);
		OSSemPost(rw_sem);
		if (ret == 0) {
			break;
		}
		usbd_delay(0x1);
	}

    if(ret == 0)
    {
        //DBG_PRINT("Write lba addr %d\r\n", ramcurlba);
		ramcurlba += seccount;		
		return 0;
    }
    else
    {
    	DBG_PRINT("Write lba(%d) return -1\r\n", ramcurlba);
		ramcurlba = SD_OUT_SYM;    
		sd_status = SD_REMOVE;		
        return -1;
    }
}

static INT32S	SDCARD_ReadCmdPhase(void* priv, INT32U lba,INT32U seccount)
{

    //DBG_PRINT("Read start lba %d\r\n", lba);
    ramcurlba = lba;    
    return 0;
}

static INT32S 	SDCARD_ReadEndCmdPhase(void* priv)
{
    //DBG_PRINT("Read end\r\n\r\n");
    ramcurlba = SD_OUT_SYM;    
    return 0;
}

static INT32S	SDCARD_WriteCmdPhase(void* priv, INT32U lba, INT32U seccount)
{
    //DBG_PRINT("Write start lba %d\r\n", lba);
    ramcurlba = lba;
    return 0;
}

static INT32S 	SDCARD_WriteEndCmdPhase(void* priv)
{
   //DBG_PRINT("Write end\r\n\r\n");
   ramcurlba = SD_OUT_SYM;
    return 0;
}

static INT32S  SDCARD_CheckDmaCheckState(void* priv)
{
    //DBG_PRINT("RAMDISK_CheckDmaCheckState\r\n");
    return STORAGE_CHECK_READY;
}

static INT32S  SDCARD_Insertion(void* priv)
{
	INT32S ret = -1;
	INT8U err;

	OSSemPend(rw_sem, 0, &err);		// 讓 SD 卡同時只被一個線程存取
	if (ramcurlba==SD_OUT_SYM)
	{
		if (sd_status==SD_INSERT) {
			ret = drvl2_sdc_live_response();
		}
		else {
			ret = drvl2_sd_init();
		}
	}
	else
	{
		ret = 0;	// 正在讀寫
	}
	OSSemPost(rw_sem);	

	sd_status_bak = sd_status;
	if (ret == 0)	sd_status = SD_INSERT;
	else			sd_status = SD_REMOVE;

	if (sd_status != sd_status_bak)
	{
		// 若螢幕保護開時，要點亮背光
		if(screen_saver_enable) {
			screen_saver_enable = 0;
	      		ap_state_handling_lcd_backlight_switch(1);
			DBG_PRINT("turn on backlight\r\n");
		}	
	}

	return ret;
}

/* RAM disk access functions table */
static MDSC_LUN_STORAGE_DRV const SDCARD_Access =
{
	NULL,						//private data
    SDCARD_Initial_Storage,    //init
    SDCARD_Uninitial_Storage,  //uninit
    SDCARD_Insertion,          //insert event
    SDCARD_GetDrvInfo_Storage, //get driver info
    SDCARD_ReadCmdPhase,       //read command phase
    SDCARD_ReadSectorDMA,      //read DMA phase
    SDCARD_ReadEndCmdPhase,    //read command end phase
    SDCARD_WriteCmdPhase,      //write command phase
    SDCARD_WriteSectorDMA,     //write DMA phase
    SDCARD_WriteEndCmdPhase,   //write command end phase
    SDCARD_CheckDmaCheckState  //check DMA buffer state
};


/*****************************************************
    RAMDISK API
*****************************************************/
#if (defined RAMDISK_EN) && (RAMDISK_EN == 1)
static INT32S RAMDISK_Initial_Storage(void* priv)
{
	gRamDiskPtr = (INT8S *)gRamDisk;//gp_malloc_align(C_RAMDISK_SIZE, 32);

	ramcurlba = 0;
	if(gRamDiskPtr == NULL) return -1;
	else return 0;
}

static INT32S RAMDISK_Uninitial_Storage(void* priv)
{
	if(gRamDiskPtr) {
		//gp_free(gRamDiskPtr);
		gRamDiskPtr = NULL;
	}
	return 0;
}

static void RAMDISK_GetDrvInfo_Storage(void* priv, STORAGE_DRV_INFO *info)
{
	//DBG_PRINT("total sector %d\r\n", size);
	info->nSectors = C_RAMDISK_SECTOR_NUM;
	info->nBytesPerSector = 512;
}

static INT32S RAMDISK_ReadSector(void* priv, INT32U *buf,INT8U ifwait,INT32U seccount)
{	
    if(seccount < C_RAMDISK_SECTOR_NUM)
    {
        //DBG_PRINT("Read DMA addr 0x%x, len 0x%x\r\n", (gRamDiskPtr + (ramcurlba << 9)), (seccount << 9));
        //gp_memcpy((INT8S *)buf, (INT8S *)gRamDiskPtr+(ramcurlba<<9), (seccount<<9));
        memcpy(buf, gRamDiskPtr+(ramcurlba<<9), seccount<<9);
        ramcurlba += seccount;
        return 0;    
    }
    else
    {
    	DBG_PRINT("Read DMA return -1\r\n");
        return -1;
    }
}

static INT32S RAMDISK_WriteSector(void* priv, INT32U *buf, INT8U ifwait, INT32U seccount)
{
    if(seccount < C_RAMDISK_SECTOR_NUM)
    {
        //DBG_PRINT("Write DMA addr 0x%x, len 0x%x\r\n", (gRamDiskPtr + (ramcurlba << 9)), (seccount << 9));
		//gp_memcpy((INT8S *)gRamDiskPtr+(ramcurlba<<9), (INT8S *)buf, (seccount<<9));
		memcpy(gRamDiskPtr+(ramcurlba<<9), buf, seccount<<9);
        ramcurlba += seccount;
        return 0;
    }
    else
    {
    	DBG_PRINT("Write DMA return -1\r\n");
        return -1;
    }
}

static INT32S	RAMDISK_ReadCmdPhase(void* priv, INT32U lba,INT32U seccount)
{

    //DBG_PRINT("Read start lba %d\r\n", lba);
    ramcurlba = lba;
    return 0;
}

static INT32S 	RAMDISK_ReadEndCmdPhase(void* priv)
{
    //DBG_PRINT("Read end\r\n\r\n");
    ramcurlba = 0;
    return 0;
}

static INT32S	RAMDISK_WriteCmdPhase(void* priv, INT32U lba, INT32U seccount)
{
    //DBG_PRINT("Write start lba %d\r\n", lba);
    ramcurlba = lba;
    return 0;
}

static INT32S 	RAMDISK_WriteEndCmdPhase(void* priv)
{
   //DBG_PRINT("Write end\r\n\r\n");
    ramcurlba = 0;
    return 0;
}

static INT32S  RAMDISK_CheckDmaCheckState(void* priv)
{
    //DBG_PRINT("RAMDISK_CheckDmaCheckState\r\n");
    return STORAGE_CHECK_READY;
}

/* RAM disk access functions table */
static MDSC_LUN_STORAGE_DRV const RAMDISK_Access =
{
	NULL,						//private data
    RAMDISK_Initial_Storage,    //init
    RAMDISK_Uninitial_Storage,  //uninit
    NULL,                       //insert event
    RAMDISK_GetDrvInfo_Storage, //get driver info
    RAMDISK_ReadCmdPhase,       //read command phase
    RAMDISK_ReadSector,         //read DMA phase
    RAMDISK_ReadEndCmdPhase,    //read command end phase
    RAMDISK_WriteCmdPhase,      //write command phase
    RAMDISK_WriteSector,        //write DMA phase
    RAMDISK_WriteEndCmdPhase,   //write command end phase
    RAMDISK_CheckDmaCheckState  //check DMA buffer state
};
#endif
extern INT8U ap_state_handling_storage_id_get(void);
void USBD_MSDC_Init(void)
{
    INT32S ret;
    #if SUPPORT_MODIFY_VOLUME_NAME == CUSTOM_ON
    INT16S tc_ret;
    STVolume volume_s;
    #endif

	// Init MSDC variables
	sd_status = SD_REMOVE;
	ramcurlba = SD_OUT_SYM;	
 
 	/* switch to USB device mode  bit8 = 0 */
 	rSYS_CTRL_NEW &= ~(1 << 8);
 	DBG_PRINT("USBD_MSDC_Init, rSYS_CTRL_NEW =0x%x\r\n", rSYS_CTRL_NEW);
    /* Init USBD L2 protocol layer first, including control/bulk/ISO/interrupt transfers */
    /******************************* Control transfer ************************************/
    ret = drv_l2_usbd_ctl_init();
    if(ret == STATUS_FAIL)
    {
        DBG_PRINT("drv_l2_usbd_ctl_init failed!\r\n");
        return;
    }
    
    /* Register new descriptor table here, this action must be done after drv_l2_usbd_ctl_init() */
    drv_l2_usbd_register_descriptor(REG_DEVICE_DESCRIPTOR_TYPE, (INT8U*)Default_Device_Descriptor_TBL);
    drv_l2_usbd_register_descriptor(REG_CONFIG_DESCRIPTOR_TYPE, (INT8U*)Default_Config_Descriptor_TBL);
    drv_l2_usbd_register_descriptor(REG_DEVICE_QUALIFIER_DESCRIPTOR_TYPE, (INT8U*)Default_Qualifier_Descriptor_TBL);
    drv_l2_usbd_register_descriptor(REG_STRING0_DESCRIPTOR_TYPE, (INT8U*)Default_String0_Descriptor);
    drv_l2_usbd_register_descriptor(REG_STRING1_DESCRIPTOR_TYPE, (INT8U*)Default_String1_Descriptor);
    drv_l2_usbd_register_descriptor(REG_STRING2_DESCRIPTOR_TYPE, (INT8U*)Default_String2_Descriptor);

#if (defined RAMDISK_EN) && (RAMDISK_EN == 1)
	if (ap_state_handling_storage_id_get() == NO_STORAGE) {
		drv_l2_usbd_msdc_set_lun(LUN_RAM_DISK_TYPE, LUN_NUM_0,USBD_STORAGE_NO_WPROTECT, &RAMDISK_Access);
	} else {
		drv_l2_usbd_msdc_set_lun(LUN_SDC_TYPE, LUN_NUM_0,USBD_STORAGE_NO_WPROTECT, &SDCARD_Access);
	}
#else
	drv_l2_usbd_msdc_set_lun(LUN_SDC_TYPE, LUN_NUM_0,USBD_STORAGE_NO_WPROTECT, &SDCARD_Access);
#endif

   	/* Init MSDC driver */
    ret = drv_l2_usbd_msdc_init();
    if(ret == STATUS_FAIL)
    {
        /* Init failed, do uninit procedures */
        drv_l2_usbd_msdc_uninit();
        DBG_PRINT("drv_l2_usbd_msdc_uninit failed!\r\n");
        return;
    }
    //add volume name judge and modify 20160901 jintao  
    #if SUPPORT_MODIFY_VOLUME_NAME == CUSTOM_ON
    tc_ret = get_volume(MINI_DVR_STORAGE_TYPE,&volume_s);
    if(tc_ret==0)
    {
    	char dst_name[] = "UNIDADKL203";//猁韜靡腔棠攫靡趼ㄛ酗僅祥夔閉徹11跺趼睫酗僅
    	
    	DBG_PRINT("volume_name = %s\r\n",volume_s.name);
    	if(!strcmp((char *)volume_s.name,dst_name))
    	{
    		DBG_PRINT("same name!\r\n");
    	}
    	else
    	{
    		DBG_PRINT("different name!\r\n");
    		set_volume(MINI_DVR_STORAGE_TYPE,(INT8U *)dst_name);
    	}
    }
    #endif
    
    /* Register SCSI inquiry data pointer, it must be done after drv_l2_usbd_msdc_init() */
    drv_l2_sbd_msdc_register_scsi_inquiry_data((INT8U*)Default_scsi_inquirydata, (INT8U*)Default_scsi_inquirydata_CDROM);
    
    /* Init USBD L1 register layer */
    ret = drv_l1_usbd_init();
    if(ret == STATUS_FAIL)
    {
        DBG_PRINT("drv_l1_usbd_init failed!\r\n");
        return;
    }
    
	/* register USBD ISR handler */
	drv_l1_usbd_enable_isr(drv_l1_usbd_isr_handle);
    	
    DBG_PRINT("USB MSDC device init completed!\r\n");
	if(rw_sem == NULL)
	{
	    rw_sem = OSSemCreate(1);
	}
}


//////////////////////////////////////////////////////////////////////
//				           U    V    C                              //
//////////////////////////////////////////////////////////////////////
#include "avi_encoder_app.h"

#define UAC_RETRY					1
#define UVC_RETRY					1

#define ISO_STATE_IDLE				0
#define ISO_STATE_SENDING_VIDEO		1
#define ISO_STATE_SENDING_AUDIO		2
#define ISO_STATE_SEND_VIDEO_DONE	3
#define ISO_STATE_SEND_AUDIO_DONE	4

#define UVC_SIZE		(JPEG_UVC_BUFFER_LARGE_CNT+JPEG_UVC_BUFFER_MIDDLE_CNT+JPEG_UVC_BUFFER_SMALL_CNT)
#define UAC_SIZE	(AVI_ENCODE_PCM_BUFFER_NO-1)	// 要減去正在DMA的2塊 buffer

typedef struct _WebCamBuf {
	INT8U *addr;
	INT32U size;
}WebCamBuf;

static INT8U isostatus = 0;
// video 重送機制
static WebCamBuf vid_cur_buf;
static WebCamBuf vid_buf[UVC_SIZE];
INT32U vid_buf_addr[UVC_SIZE];
static int vid_wi;
static int vid_ri;
int usb_send_video(INT8U *jpeg_addr, INT32U jpeg_size);

extern INT8U USB_UVC_DeviceDescriptor[];
extern INT8U USB_UVC_Qualifier_Descriptor_TBL[];
extern INT8U USB_UVC_ConfigDescriptor[];
extern INT8U UVC_String0_Descriptor[];
extern INT8U UVC_String1_Descriptor[];
extern INT8U UVC_String2_Descriptor[];
extern INT8U UVC_String3_Descriptor[];
extern INT8U UVC_String4_Descriptor[];
extern INT8U UVC_String5_Descriptor[];

extern INT32S drv_l2_usbd_uvc_init(void);
extern INT32S drv_l1_usbd_uvc_init(void);
extern void drv_l2_usbd_resigter_uvc_frame_done_cbk(ISO_FRAME_TRANSFER_DONE cbk);
extern void drv_l2_usbd_resigter_uac_frame_done_cbk(ISO_FRAME_TRANSFER_DONE cbk);

// audio 重送機制
static WebCamBuf aud_cur_buf, aud_dma_done;
static WebCamBuf aud_buf[UAC_SIZE];
static int aud_wi;
static int aud_ri;
int usb_send_audio(INT8U *audio_addr, INT32U audio_size);

#if UAC_RETRY
static int get_aud_idx(void)
{
	aud_ri++;
	if (aud_ri>=UAC_SIZE)
		aud_ri = 0;
	return aud_ri;
}

static int set_aud_idx(INT8U *addr , INT32U size)
{
	int idx = aud_wi;
	idx++;
	if (idx>=UAC_SIZE)
		idx=0;
	if (idx==aud_ri)
	{
		DBG_PRINT("AERR! %d_%d\r\n");
		// skip this frame
		/*
		OSSchedLock();
		aud_wi = 0;
		aud_ri = UAC_SIZE-1;
		OSSchedUnlock();
		*/
		return -1;
	}
	else
	{
		aud_buf[aud_wi].addr = addr;
		aud_buf[aud_wi].size = size;
		aud_wi = idx;
		return 0;
	}
}

static int aud_check(void)
{
	int ret = 0;
	int space = aud_wi - aud_ri;
	
	if (space<0) {
		space += UAC_SIZE;
	}
	if (space>1) {
		ret = space;
	}
	return ret;
}
#endif

static int get_jpeg_idx(INT32U addr)
{
	INT32U i = 0;
	int ret = -1;
	
	for (i=0; i<UVC_SIZE; ++i)
	{
		if (addr == vid_buf_addr[i])
		{
			ret = i;
			break;
		}
	}
	if (ret == -1)
	{
		DBG_PRINT("UVC_ADDR Err (0x%x)\r\n",addr);
	}
	return ret;	
}

#if UVC_RETRY
static int get_vid_idx(void)
{
	vid_ri++;
	if (vid_ri>=UVC_SIZE)
		vid_ri = 0;
	return vid_ri;
}

static int set_vid_idx(INT8U *addr , INT32U size)
{
	int idx = vid_wi;
	idx++;
	if (idx>=UVC_SIZE)
		idx=0;
	if (idx==vid_ri)
	{
		DBG_PRINT("VERR! %d_%d\r\n",vid_wi,vid_ri);
		// drop this frame
		{
			INT32S jpeg_id = get_jpeg_idx((INT32U)addr);
			if (jpeg_id!=-1)	{
				OSQPost(my_AVIEncodeApQ, (void*)(MSG_UVC_BUF_DMA_DOWN|jpeg_id));
			}
		}
		return -1;
	}
	else
	{
		vid_buf[vid_wi].addr = addr;
		vid_buf[vid_wi].size = size;
		vid_wi = idx;
		return 0;
	}
}

static int vid_check(void)
{
	int ret = 0;
	int space = vid_wi - vid_ri;
	
	if (space<0) {
		space += UVC_SIZE;
	}
	if (space>1) {
		ret = space;
	}
	return ret;
}
#endif

static void check_media_frame(int type)
{
	#if UVC_RETRY
	int v_check = vid_check();
	#endif
	#if UAC_RETRY
	int a_check = aud_check();
	#endif

	if (type==0)
	{
		#if UVC_RETRY
		if (v_check)
		{
			int idx = get_vid_idx();
			usb_send_video(vid_buf[idx].addr,vid_buf[idx].size);
			//DBG_PRINT("V"); //DBG_PRINT("V%d_%d ",vid_wi,vid_ri);
			return;
		}
		#endif
		#if UAC_RETRY
		if (a_check)
		{
			int idx = get_aud_idx();
			usb_send_audio(aud_buf[idx].addr, aud_buf[idx].size);
			//DBG_PRINT("A");
			return;
		}
		#endif
	}
	else
	{
		#if UAC_RETRY
		if (a_check)
		{
			int idx = get_aud_idx();
			usb_send_audio(aud_buf[idx].addr, aud_buf[idx].size);
			//DBG_PRINT("A");
			return;
		}
		#endif
		#if UVC_RETRY
		if (v_check)
		{
			int idx = get_vid_idx();
			usb_send_video(vid_buf[idx].addr,vid_buf[idx].size);
			//DBG_PRINT("V"); //DBG_PRINT("V%d_%d ",vid_wi,vid_ri);
			return;
		}
		#endif
	}	
}

static void iso_video_transfer_done(void)
{
	isostatus = ISO_STATE_SEND_VIDEO_DONE;
	drv_l1_usbd_set_frame_end_ep5();
	// 整張傳完	
	{
		INT32S jpeg_id = get_jpeg_idx((INT32U)(vid_cur_buf.addr));
		if (jpeg_id!=-1)	{
			OSQPost(my_AVIEncodeApQ, (void*)(MSG_UVC_BUF_DMA_DOWN|jpeg_id));
		}
	}
	
	//DBG_PRINT("V done\r\n");
	check_media_frame(1);
}	

static void iso_audio_transfer_done(void)
{
	aud_dma_done.addr = aud_cur_buf.addr;
	isostatus = ISO_STATE_SEND_AUDIO_DONE;

	//DBG_PRINT("A done\r\n");
	check_media_frame(0);
}

int usb_send_video(INT8U *jpeg_addr, INT32U jpeg_size)
{
	int ret = -1;

	if ( drv_l1_usbd_iso_ep5_get_alt_intf_num()==0 )
	{	// drop video
		OSSchedLock();
		vid_wi = 0;
		vid_ri = UVC_SIZE-1;
		OSSchedUnlock();
		OSQPost(my_AVIEncodeApQ, (void*)(MSG_UVC_BUF_FLUSH));				

		if(drv_l2_usbd_uvc_check_ep5_flush())
		{
			isostatus = ISO_STATE_SEND_VIDEO_DONE;
			DBG_PRINT("Vid EP5 flush\r\n");
			drv_l1_usbd_iso_ep5_flush();
		}

		if(drv_l2_usbd_uvc_check_ep7_flush())
		{
			isostatus = ISO_STATE_SEND_AUDIO_DONE;
			DBG_PRINT("Vid EP7 flush\r\n");
			drv_l1_usbd_iso_ep7_flush();
		}
		
		return 0;
	}

	// normal flow
	if (isostatus != ISO_STATE_SENDING_VIDEO && isostatus != ISO_STATE_SENDING_AUDIO)
	{
		if ( drv_l1_usbd_iso_ep5_get_alt_intf_num() )
		{
			//DBG_PRINT("V(0x%x)\r\n", (INT32U)jpeg_addr);
			isostatus = ISO_STATE_SENDING_VIDEO;	ret = 0;
			vid_cur_buf.addr = jpeg_addr;
			vid_cur_buf.size = jpeg_size;
			drv_l1_usbd_frame_iso_ep5_in((void *) jpeg_addr, jpeg_size, 1);
		}
	}
	else
	{
		if(drv_l2_usbd_uvc_check_ep5_flush())
		{
			isostatus = ISO_STATE_SEND_VIDEO_DONE;
			DBG_PRINT("Vid EP5 flush\r\n");
			drv_l1_usbd_iso_ep5_flush();
		}

		if(drv_l2_usbd_uvc_check_ep7_flush())
		{
			isostatus = ISO_STATE_SEND_AUDIO_DONE;
			DBG_PRINT("Vid EP7 flush\r\n");
			drv_l1_usbd_iso_ep7_flush();
		}
		// 沒送成功，要重傳
		#if UVC_RETRY		
		OSSchedLock();
		set_vid_idx(jpeg_addr, jpeg_size);
		OSSchedUnlock();
		#endif
	}

	return ret;
}

int usb_send_audio(INT8U *audio_addr, INT32U audio_size)
{
	int ret = -1;

	return 0;	// disable UAC

	if(drv_l1_usbd_iso_ep7_get_alt_intf_num()==0)
	{	//drop audio
		OSSchedLock();
		aud_wi = 0;
		aud_ri = UAC_SIZE-1;
		OSSchedUnlock();

		if(drv_l2_usbd_uvc_check_ep5_flush())
		{
			isostatus = ISO_STATE_SEND_VIDEO_DONE;
			DBG_PRINT("Aud EP5 flush\r\n");
			drv_l1_usbd_iso_ep5_flush();
		}

		if(drv_l2_usbd_uvc_check_ep7_flush())
		{
			isostatus = ISO_STATE_SEND_AUDIO_DONE;
			DBG_PRINT("Aud EP7 flush\r\n");
			drv_l1_usbd_iso_ep7_flush();
		}

		return 0;
	}

	// normal flow
	if (isostatus != ISO_STATE_SENDING_VIDEO && isostatus != ISO_STATE_SENDING_AUDIO)
	{
		if(drv_l1_usbd_iso_ep7_get_alt_intf_num())
		{
			// DBG_PRINT("A(0x%x)\r\n", audio_addr);
			isostatus = ISO_STATE_SENDING_AUDIO;	ret = 0;
			aud_cur_buf.addr = audio_addr;
			aud_cur_buf.size = audio_size;			
			drv_l1_usbd_dma_iso_ep7_in((void *) audio_addr, audio_size);
		}
	}
	else
	{
		if(drv_l2_usbd_uvc_check_ep5_flush())
		{
			isostatus = ISO_STATE_SEND_VIDEO_DONE;
			DBG_PRINT("Aud EP5 flush\r\n");
			drv_l1_usbd_iso_ep5_flush();
		}

		if(drv_l2_usbd_uvc_check_ep7_flush())
		{
			isostatus = ISO_STATE_SEND_AUDIO_DONE;
			DBG_PRINT("Aud EP7 flush\r\n");
			drv_l1_usbd_iso_ep7_flush();
		}
		// 沒送成功，要重傳
		#if UAC_RETRY
		OSSchedLock();
		set_aud_idx(audio_addr, audio_size);
		OSSchedUnlock();
		#endif
	}

	return ret;
}

void usb_uvc_start(void)
{	
 	// switch to USB device mode (bit8 = 0)
 	rSYS_CTRL_NEW &= ~(1 << 8);
 
    // Init USBD L2 protocol layer first, including control/bulk/ISO/interrupt transfers
    if (drv_l2_usbd_ctl_init()) {
        //DBG_PRINT("drv_l2_usbd_ctl_init failed!\r\n");
        return;
    }
    
    /* Register new descriptor table here, this action must be done after drv_l2_usbd_ctl_init() */
    drv_l2_usbd_register_descriptor(REG_DEVICE_DESCRIPTOR_TYPE, (INT8U *) USB_UVC_DeviceDescriptor);
    drv_l2_usbd_register_descriptor(REG_CONFIG_DESCRIPTOR_TYPE, (INT8U *) USB_UVC_ConfigDescriptor);
    drv_l2_usbd_register_descriptor(REG_DEVICE_QUALIFIER_DESCRIPTOR_TYPE, (INT8U *) USB_UVC_Qualifier_Descriptor_TBL);
    drv_l2_usbd_register_descriptor(REG_STRING0_DESCRIPTOR_TYPE, (INT8U *) UVC_String0_Descriptor);
    drv_l2_usbd_register_descriptor(REG_STRING1_DESCRIPTOR_TYPE, (INT8U *) UVC_String1_Descriptor);
    drv_l2_usbd_register_descriptor(REG_STRING2_DESCRIPTOR_TYPE, (INT8U *) UVC_String2_Descriptor);
	drv_l2_usbd_register_descriptor(REG_STRING3_DESCRIPTOR_TYPE, (INT8U *) UVC_String3_Descriptor);
	drv_l2_usbd_register_descriptor(REG_STRING4_DESCRIPTOR_TYPE, (INT8U *) UVC_String4_Descriptor);
	drv_l2_usbd_register_descriptor(REG_STRING5_DESCRIPTOR_TYPE, (INT8U *) UVC_String5_Descriptor);

   	// Init USBD L2 of UVC
    if (drv_l2_usbd_uvc_init()) {
        //DBG_PRINT("drv_l2_usbd_uvc_init failed!\r\n");
        return;
    }

    // Init USBD L1 register layer
    if (drv_l1_usbd_uvc_init()) {
        //DBG_PRINT("drv_l1_usbd_uvc_init failed!\r\n");
        return;
    }
    
	// Register USBD ISR handler
	drv_l1_usbd_enable_isr(drv_l1_usbd_isr_handle);
    
    drv_l2_usbd_resigter_uvc_frame_done_cbk(iso_video_transfer_done);
    drv_l2_usbd_resigter_uac_frame_done_cbk(iso_audio_transfer_done);
	
     DBG_PRINT("\r\nusb_uvc_start");
        isostatus = 0;

	#if UVC_RETRY
	vid_wi = 0;
	vid_ri = UVC_SIZE-1;
	#endif
	#if UAC_RETRY	
	aud_wi = 0;
	aud_ri = UAC_SIZE-1;
	#endif
}

