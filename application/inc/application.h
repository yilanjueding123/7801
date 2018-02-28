#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "gplib.h"
#include "application_cfg.h"
#include "drv_l1_sfr.h"

/* Pseudo Header Include there*/
#include "turnkey_sys_msg.h"
#include "turnkey_drv_msg.h"

/****************************************************************************/
// Definitions
#ifndef DBG_MESSAGE
	#define DBG_MESSAGE 1
#endif

#if DBG_MESSAGE
	#define DEBUG_MSG(x)	{x;}
#else
	#define DEBUG_MSG(x)	{}
#endif

#define C_ACK_SUCCESS		0x00000001
#define C_ACK_FAIL			0x80000000

#define RETURN(x)	{nRet = x; goto Return;}
#define POST_MESSAGE(msg_queue, message, ack_mbox, msc_time, msg, err)\
{\
	msg = (INT32S) OSMboxAccept(ack_mbox);\
	err = OSQPost(msg_queue, (void *)message);\
	if(err != OS_NO_ERR)\
	{\
		DEBUG_MSG(DBG_PRINT("OSQPost Fail!!!\r\n"));\
		RETURN(STATUS_FAIL);\
	}\
	msg = (INT32S) OSMboxPend(ack_mbox, msc_time/10, &err);\
	if(err != OS_NO_ERR || msg == C_ACK_FAIL)\
	{\
		DEBUG_MSG(DBG_PRINT("OSMbox ack Fail!!!\r\n"));\
		RETURN(STATUS_FAIL);\
	}\
}

/****************************************************************************/
 #define GPS_EN 0
 #define GPS_TXT 0
 #define GPS_AVI_DEMO 0
/****************************************************************************/
/* AUDIO Task */
typedef struct
{
	INT8U	*ring_buf;
	INT8S   *work_mem;
	INT8U   reading;
	INT16S  file_handle;
	INT32U  frame_size;
	INT32U  ring_size;
	INT32U  ri;
	INT32U  wi;
	INT32U  f_last;
	INT32U  file_len;
	INT32U  file_cnt;
	INT32U  try_cnt;
	INT32U  read_secs;
	INT32U  file_end;
}AUDIO_CTRL;

typedef struct
{
    INT32S curr_play_time;
    INT32S total_play_time;
} ST_AUDIO_PLAY_TIME;

extern void audio_task_entry(void *p_arg);
extern void audio_send_stop(void);
extern OS_EVENT	*hAudioTaskQ;
extern OS_EVENT    *SemAudio;
extern AUDIO_CTRL		audio_ctrl;		// added by Bruce, 2008/09/18

extern INT32U g_MIDI_index;		// added by Bruce, 2008/10/09
extern void (*decode_end)(INT32U audio_decoder_num);	// modified by Bruce, 2008/11/20
extern INT32S (*audio_move_data)(INT32U buf_addr, INT32U buf_size);	// added by Bruce, 2008/10/27


// File Service Task
#define C_FILE_SERVICE_FLUSH_DONE		0xFF010263

typedef struct {
	INT16U cmd;
	INT16S fd;
	INT32U buf_addr;
	INT32U buf_size;
  #if _OPERATING_SYSTEM == 1	
	OS_EVENT *result_queue;
  #endif
} FILE_SERVICE_STRUCT, *P_FILE_SERVICE_STRUCT;

/* audio */
#define MAX_DAC_BUFFERS   6
#define AUDIO_PRIOR_DYNAMIC_SW 1

/*==================== Turnkey Solution Application header declare */
/* turn key message manager */
#define MSG_PRI_NORMAL		0
#define MSG_PRI_URGENT		1

typedef struct 
{
	INT32U		maxMsgs;			/* max messages that can be queued */
	INT32U		maxMsgLength;		/* max bytes in a message */
	OS_EVENT*	pEvent;
	void*		pMsgQ;
	INT8U*		pMsgPool;
	OS_EVENT*   msg_Q_sem;
} *MSG_Q_ID;

//========================================================
//Function Name:msgQCreate
//Syntax:		MSG_Q_ID msgQCreate(INT32U maxQSize, INT32U maxMsgs, INT32U maxMsgLength)
//Purpose:		create and initialize a message queue
//Note:			
//Parameters:   INT32U maxQSize			/* max queue can be creat */
//				INT32U	maxMsgs			/* max messages that can be queued */
//				INT32U	maxMsgLength	/* max bytes in a message */
//Return:		NULL if faile
//=======================================================
extern MSG_Q_ID msgQCreate(INT32U maxQSize, INT32U maxMsgs, INT32U maxMsgLength);

//========================================================
//Function Name:msgQDelete
//Syntax:		void msgQDelete (MSG_Q_ID msgQId)
//Purpose:		delete a message queue
//Note:			
//Parameters:   MSG_Q_ID msgQId		/* message queue to delete */
//Return:		
//=======================================================
extern void msgQDelete (MSG_Q_ID msgQId);

//========================================================
//Function Name:msgQSend
//Syntax:		INT32S msgQSend(MSG_Q_ID msgQId, INT32U msg_id, void *para, INT32U nParaByte, INT32U priority)
//Purpose:		send a message to a message queue
//Note:			
//Parameters:   MSG_Q_ID msgQId			/* message queue on which to send */
//				INT32U msg_id			/* message id */
//				void *para				/* message to send */
//				INT32U nParaByte		/* byte number of para buffer */
//				INT32U priority			/* MSG_PRI_NORMAL or MSG_PRI_URGENT */
//Return:		-1 if faile 
//				0 success
//=======================================================
extern INT32S msgQSend(MSG_Q_ID msgQId, INT32U msg_id, void *para, INT32U nParaByte, INT32U priority);

//========================================================
//Function Name:msgQReceive
//Syntax:		INT32S msgQReceive(MSG_Q_ID msgQId, INT32U *msg_id, void *para, INT32U maxParaNByte)
//Purpose:		receive a message from a message queue
//Note:			
//Parameters:   MSG_Q_ID msgQId			/* message queue on which to send */
//				INT32U *msg_id			/* message id */
//				void *para				/* message and type received */
//				INT32U maxNByte			/* message size */
//Return:		-1: if faile
//				0: success
//=======================================================
extern INT32S msgQReceive(MSG_Q_ID msgQId, INT32U *msg_id, void *para, INT32U maxParaNByte);

//========================================================
//Function Name:msgQAccept
//Syntax:		INT32S msgQAccept(MSG_Q_ID msgQId, INT32U *msg_id, void *para, INT32U maxParaNByte)
//Purpose:		Check whether a message is available from a message queue
//Note:
//Parameters:   MSG_Q_ID msgQId			/* message queue on which to send */
//				INT32U *msg_id			/* message id */
//				void *para				/* message and type received */
//				INT32U maxNByte			/* message size */
//Return:		-1: queue is empty or fail
//				0: success
//=======================================================
extern INT32S msgQAccept(MSG_Q_ID msgQId, INT32U *msg_id, void *para, INT32U maxParaNByte);

//========================================================
//Function Name:msgQFlush
//Syntax:		void msgQFlush(MSG_Q_ID msgQId)
//Purpose:		flush message queue
//Note:			
//Parameters:   MSG_Q_ID msgQId
//Return:		
//=======================================================
extern void msgQFlush(MSG_Q_ID msgQId);

//========================================================
//Function Name:msgQQuery
//Syntax:		INT8U msgQQuery(MSG_Q_ID msgQId, OS_Q_DATA *pdata)
//Purpose:		get current Q message information
//Note:			
//Parameters:   MSG_Q_ID msgQId, OS_Q_DATA *pdata
//Return:		
//=======================================================
extern INT8U msgQQuery(MSG_Q_ID msgQId, OS_Q_DATA *pdata);

//========================================================
//Function Name:msgQSizeGet
//Syntax:		INT32U msgQSizeGet(MSG_Q_ID msgQId)
//Purpose:		get current Q message number
//Note:			
//Parameters:   MSG_Q_ID msgQId
//Return:		
//=======================================================
extern INT32U msgQSizeGet(MSG_Q_ID msgQId);


/* ap - error call function define */
extern INT32S err_call(INT32S status, INT32S err_define, void(* ErrHandleFunction)(void));
extern INT32S not_OK_call(INT32S status, INT32S ok_define, void(* NotOkHandleFunction)(void));

/* Turn Key Task Message Structure Declare */

/* TurnKey Task entry declare */
/* Turn Key Audio DAC task */
extern void audio_dac_task_entry(void * p_arg);
extern INT32U      last_send_idx;	// added by Bruce, 2008/12/01

/*== Turn Key Audio Decoder task ==*/
typedef struct 
{
    MSG_AUD_ENUM  msg_id; 
    void* pPara1;
} STTkAudioTaskMsg;

typedef struct 
{
    BOOLEAN   mute;
    INT8U     volume;
    INT16S    fd;
    INT32U    src_id;
    INT32U    src_type;
    INT32U    audio_format;
    INT32U    file_len;
    INT32U    play_speed;
    INT32U    reverse;
} STAudioTaskPara;

typedef enum
{
	AUDIO_SRC_TYPE_FS=0,
	AUDIO_SRC_TYPE_SDRAM_LDW,
	AUDIO_SRC_TYPE_SPI,
	AUDIO_SRC_TYPE_RS,			// added by Bruce, 2009/02/19
	AUDIO_SRC_TYPE_USER_DEFINE,	// added by Bruce, 2008/10/27
	AUDIO_SRC_TYPE_FS_RESOURCE_IN_FILE,	// added by Bruce, 2010/01/22
	AUDIO_SRC_TYPE_GPRS,
	AUDIO_SRC_TYPE_APP_RS,
	AUDIO_SRC_TYPE_APP_PACKED_RS,
	AUDIO_SRC_TYPE_MAX
}AUDIO_SOURCE_TYPE;

typedef enum
{
	AUDIO_TYPE_NONE = -1,
	AUDIO_TYPE_WAV,
	AUDIO_TYPE_MP3,
	AUDIO_TYPE_WMA,
	AUDIO_TYPE_AVI,
	AUDIO_TYPE_AVI_MP3,
	AUDIO_TYPE_A1800,//080722
	AUDIO_TYPE_MIDI,//080903
	AUDIO_TYPE_A1600,
	AUDIO_TYPE_A6400,
	AUDIO_TYPE_S880
}AUDIO_TYPE;

typedef struct
{
    MSG_AUD_ENUM result_type;
    AUDIO_SOURCE_TYPE source_type;
    INT32S result;
} STAudioConfirm;

typedef enum
{
	AUDIO_ERR_NONE,
	AUDIO_ERR_FAILED,
	AUDIO_ERR_INVALID_FORMAT,
	AUDIO_ERR_OPEN_FILE_FAIL,
	AUDIO_ERR_GET_FILE_FAIL,
	AUDIO_ERR_DEC_FINISH,
	AUDIO_ERR_DEC_FAIL,
	AUDIO_ERR_READ_FAIL,
	AUDIO_ERR_MEM_ALLOC_FAIL
}AUDIO_STATUS;

extern void audio_task_entry(void * p_arg); 
extern MSG_Q_ID	AudioTaskQ;
extern MSG_Q_ID	AudioBGTaskQ;
extern OS_EVENT	*audio_wq;
extern OS_EVENT	*audio_bg_wq;
extern INT16S   *pcm_out[MAX_DAC_BUFFERS];
extern INT32U    pcm_len[MAX_DAC_BUFFERS];
extern INT16S   *pcm_bg_out[MAX_DAC_BUFFERS];
extern INT32U    pcm_bg_len[MAX_DAC_BUFFERS];

// added by Bruce, 2008/10/09
extern OS_EVENT	*midi_wq;
extern MSG_Q_ID MIDITaskQ;
extern INT16S   *pcm_midi_out[MAX_DAC_BUFFERS];
extern INT32U    pcm_midi_len[MAX_DAC_BUFFERS];
// added by Bruce, 2008/10/09
extern INT32U g_audio_data_length;

/*== Turn Key File Server Task ==*/
typedef struct 
{
    MSG_FILESRV_ENUM  msg_id; 
    void* pPara1;
} STTkFileSrvTaskMsg;

typedef struct {
	INT8U   src_id;
	INT8U   src_name[10];
	INT16U  sec_offset;
	INT16U  sec_cnt;
} TK_FILE_SERVICE_SPI_STRUCT, *P_TK_FILE_SERVICE_SPI_STRUCT;


typedef struct {
	INT16S fd;
	INT32U buf_addr;
	INT32U buf_size;
	INT32U rev_seek;
	INT8U  FB_seek;
	TK_FILE_SERVICE_SPI_STRUCT spi_para;
	OS_EVENT *result_queue;
} TK_FILE_SERVICE_STRUCT, *P_TK_FILE_SERVICE_STRUCT;

typedef struct 
{
	INT16S disk;
	OS_EVENT *result_queue;
} STMountPara;

typedef struct 
{
	struct STFileNodeInfo *pstFNodeInfo;
	OS_EVENT *result_queue;
} STScanFilePara;

extern void filesrv_task_entry(void * p_arg);

//========================================================
//Function Name:WaitScanFile
//Syntax:		void ScanFileWait(struct STFileNodeInfo *pstFNodeInfo, INT32S index)
//Purpose:		wait for search file
//Note:			
//Parameters:   pstFNodeInfo	/* the point to file node information struct */
//				index			/* the file index you want to find */
//Return:		
//=======================================================
extern INT32S ScanFileWait(struct STFileNodeInfo *pstFNodeInfo, INT32S index);

/*== Turn Key Image processing task ==*/
#define IMAGE_CMD_STATE_MASK					0xFC000000
#define IMAGE_CMD_STATE_SHIFT_BITS				26

typedef enum {
	TK_IMAGE_SOURCE_TYPE_FILE = 0x0,  
	TK_IMAGE_SOURCE_TYPE_BUFFER,
	TK_IMAGE_SOURCE_TYPE_NVRAM,
	TK_IMAGE_SOURCE_TYPE_MAX
} TK_IMAGE_SOURCE_TYPE_ENUM;

typedef enum {
	TK_IMAGE_TYPE_JPEG = 0x1,  
	TK_IMAGE_TYPE_PROGRESSIVE_JPEG,
	TK_IMAGE_TYPE_MOTION_JPEG,
	TK_IMAGE_TYPE_BMP,
	TK_IMAGE_TYPE_GIF,
	TK_IMAGE_TYPE_PNG,
	TK_IMAGE_TYPE_GPZP,
	TK_IMAGE_TYPE_WAV,
	//#ifdef _DPF_PROJECT
	TK_IMAGE_TYPE_MOV_JPEG,		//mov	// added by Bruce, 2009/02/19
	//#endif
	TK_IMAGE_TYPE_MAX
} TK_IMAGE_TYPE_ENUM;


typedef struct {
	INT32U cmd_id;
	INT32S image_source;          	// File handle/resource handle/pointer
	INT32U source_size;             // File size or buffer size
	INT8U source_type;              // 0=File System, 1=SDRAM, 2=NVRAM
	INT8S decode_status;            // 0=ok, others=fail
	INT8U output_format;
	INT8U output_ratio;             // 0=Fit to output_buffer_width and output_buffer_height, 1=Maintain ratio and fit to output_buffer_width or output_buffer_height, 2=Same as 1 but without scale up, 3=Special case for thumbnail show
	INT16U output_buffer_width;
	INT16U output_buffer_height;
	INT16U output_image_width;
	INT16U output_image_height;
	INT32U out_of_boundary_color;
	INT32U output_buffer_pointer;
} IMAGE_DECODE_STRUCT, *P_IMAGE_DECODE_STRUCT;


/*== Turn Key SYSTEM Task ==*/
typedef enum
{
	SCAN_FILE_COMPLETE,
	SCAN_FILE_NOT_COMPLETE
}
SCAN_FILE_STATUS_ENUM;

typedef struct 
{
	INT16S fd;
	INT8U  f_name[256];
	INT8U  f_extname[4];
	INT32U f_size;
	INT16U f_time;
	INT16U f_date;
}STORAGE_FINFO;

typedef struct
{
  INT8U time_enable;
  INT8U time_hour;
  INT8U time_minute;
}ALARM_FORMAT;

#define ALARM_NUM 2

typedef struct
{
  ALARM_FORMAT power_on_time;
  ALARM_FORMAT power_off_time;
  ALARM_FORMAT alarm_time[ALARM_NUM];
}ALARM_INFO;


typedef struct
{
  INT8U alarm_onoff;
  ALARM_FORMAT alarm_time[7];
}ALARM_PARAMETER;

#define NO_STORAGE 0xFF

extern OS_EVENT	*hAudioDacTaskQ;
extern OS_EVENT	*aud_send_q;
extern OS_EVENT *aud_bg_send_q;
extern OS_EVENT *aud_right_q;
/*============== State Declare ==============*/
#define AP_QUEUE_MSG_MAX_LEN	40
extern MSG_Q_ID ApQ;
extern MSG_Q_ID	Audio_FG_status_Q, Audio_BG_status_Q, MIDI_status_Q;
extern MSG_Q_ID StorageServiceQ;
extern MSG_Q_ID PeripheralTaskQ;
extern OS_EVENT *scaler_task_q;

extern OS_EVENT *DisplayTaskQ;
extern OS_EVENT *display_task_ack_m;

extern OS_EVENT *USBTaskQ;
extern INT8U  ApQ_para[AP_QUEUE_MSG_MAX_LEN];

extern volatile INT8U s_usbd_pin;

extern void audio_confirm_handler(STAudioConfirm *aud_con);

// message q id define
extern MSG_Q_ID	fs_msg_q_id;
extern MSG_Q_ID	fs_scan_msg_q_id;

// MutilMedia Codec //////////////////////////////////////////////////////////////////////////////////////////

/* define task pority use  */
#define DAC_PRIORITY					0
#define AE_SERVICE_TASK_PRIORITY		1
#define AVI_ENC_PRIORITY				2
#define AUD_DEC_HIGH_PRIORITY   		4
#define AUD_DEC_BG_PRIORITY				6
//#define SYS_SOFTWARE_TIMER_TASK_PRIORITY    7  //defined in system_timer.c
#define FS_PRIORITY						8
#define USBDL2TASKPRIORITY				9
#define AVI_PACKER0_PRIORITY			10
#define AVI_DEC_PRIORITY  				11
#define AUD_ENC_PRIORITY				14
#define VIDEO_PASER_PRIORITY			15
#define AUD_REC_ENC_PRIORITY			16
#define AUDIO_DECODE_PRIORITY   		17

#define MJPG_STREAMER_PRIORITY			32
#define SOCKET_CMD_PRIORITY				33
#define RTSP_TASK_PRIORITY				MJPG_STREAMER_PRIO
#define RTP_TASK_PRIORITY				VDO_ENC_PRIO


#define STATE_HANDLING_PRIORITY			34	//19
#define VIDEO_DECODE_PRIORITY			35	//20
#define STORAGE_SERVICE_PRIORITY		36	//21
//#define AVI_PACKER0_PRIORITY			37	//22
#define VID_DEC_STATE_PRIORITY			38	//23
#define PERIPHERAL_HANDLING_PRIORITY	39	//27
#define TSK_PRI_FILE_SRV        		40	//28
#define DISPLAY_TASK_PRIORITY			41	//29
#define AUD_DEC_PRIORITY        		42	//30
#define USB_DEVICE_PRIORITY				43	//31

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define USB_CAM_PRIORITY        13  //add by erichan for usb cam
#define USB_AUDIO_PRIORITY      12
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//===============     Timer_C Used ID     ===============
//#define POWER_OFF_TIMER_ID					0
#define STORAGE_SERVICE_MOUNT_TIMER_ID			1
#define STORAGE_SERVICE_FREESIZE_TIMER_ID		2
#define VIDEO_RECORD_CYCLE_TIMER_ID				3
#define AD_DETECT_TIMER_ID						4
#define VIDEO_PREVIEW_TIMER_ID					6

//===============     Key Mode     ===============
#define DISABLE_KEY		0x0
#define GENERAL_KEY		0x1
#define USBD_DETECT		0x2
#define USBD_DETECT2	0x3
#define BETTERY_LOW_STATUS_KEY	0x4
#define WIFI_MODE_KEY	0x5
#define REGISTER_KEY	0x6

//===============     Type for draw string     ===============
#define YUYV_DRAW		0x0
#define YUV420_DRAW		0x1
#define RGB565_DRAW		0x2
#define UYVY_DRAW		0x3
#define DRAW_DATE_ONLY	0x10
#define DRAW_DATE_TIME	0x20


//========================= Media Format Defintion ============================
// including Audio, Image, Video
//=============================================================================
typedef enum
{
		AUD_AUTOFORMAT=0,
		MIDI,
		WMA,
		MP3,
		WAV,
		A1800,
		S880,
		A6400,
		A1600, 
		IMA_ADPCM,
		MICROSOFT_ADPCM
} AUDIO_FORMAT;

typedef enum
{
		IMG_AUTOFORMAT=0,
		JPEG,
		JPEG_P,		// JPEG Progressive
		MJPEG_S,	// Single JPEG from M-JPEG video
		GIF,
		BMP
} IMAGE_FORMAT;

typedef enum
{
		VID_AUTOFORMAT=0,
		MJPEG,
		MPEG4
} VIDEO_FORMAT;

//====================== Media Information Defintion ==========================
// including Audio, Image, Video
//=============================================================================
typedef struct {
		AUDIO_FORMAT	AudFormat;
		char		AudSubFormat[6];
		
		INT32U		AudBitRate;				// unit: bit-per-second
		INT32U		AudSampleRate;			// unit: Hz
		INT8U		AudChannel;				// if 1, Mono, if 2 Stereo
		INT16U		AudFrameSize;			// unit: sample per single frame

		VIDEO_FORMAT	VidFormat;
		char		VidSubFormat[4];
		INT8U		VidFrameRate;			// unit: FPS
		INT32U		Width;					// unit: pixel
		INT32U		Height;					// unit: pixel

		INT32U		TotalDuration;			// unit: second
		//INT32U	FileSize;				// unit: byte
		//char		*FileDate;				// string pointer
		//char		*FileName;				// file name
} VIDEO_INFO;

#define DISPLAY_TV              		0
#define DISPLAY_TFT						1
#define	DISPLAY_DEVICE				    DISPLAY_TFT

#define	QVGA_MODE			      		0
#define VGA_MODE						1
#define D1_MODE							2
#define TFT_320x240_MODE				3
#define TFT_800x480_MODE				4
#define TV_TFT_MODE						VGA_MODE

#if USE_PANEL_NAME == PANEL_400X240_I80
	#define DISPLAY_BUF_NUM					1
#else
	#define DISPLAY_BUF_NUM					3
#endif

#define TV_DISPLAY_BUF_NUM				2
#define HDMI_DISPLAY_BUF_NUM			3
#define DISPLAY_BUF_NUM_MAX				3

#define	PPU_YUYV_TYPE3					(3<<20)
#define	PPU_YUYV_TYPE2					(2<<20)
#define	PPU_YUYV_TYPE1					(1<<20)
#define	PPU_YUYV_TYPE0					(0<<20)

#define	PPU_RGBG_TYPE3					(3<<20)
#define	PPU_RGBG_TYPE2					(2<<20)
#define	PPU_RGBG_TYPE1					(1<<20)
#define	PPU_RGBG_TYPE0					(0<<20)

#define PPU_LB							(1<<19)
#define	PPU_YUYV_MODE					(1<<10)
#define	PPU_RGBG_MODE			        (0<<10)

#define TFT_SIZE_800X480                (5<<16)
#define TFT_SIZE_720x480				(4<<16)
#define TFT_SIZE_640X480                (1<<16)
#define TFT_SIZE_320X240                (0<<16)

#define	PPU_YUYV_RGBG_FORMAT_MODE		(1<<8)
#define	PPU_RGB565_MODE			        (0<<8)

#define	PPU_FRAME_BASE_MODE			    (1<<7)
#define	PPU_VGA_NONINTL_MODE			(0<<5)

#define	PPU_VGA_MODE					(1<<4)
#define	PPU_QVGA_MODE					(0<<4)


//=============================================================================
//======================== Media Argument Defintion ===========================
// including Audio, Image, Video
//=============================================================================
typedef enum
{
	LED_INIT 	= 0,
	LED_UPDATE_PROGRAM,
	LED_UPDATE_FINISH,
	LED_UPDATE_FAIL,
	LED_RECORD,
	LED_RECORD_IR_ENABLE,
	LED_WAITING_RECORD,
	LED_AUDIO_RECORD,
	LED_WAITING_AUDIO_RECORD,
	LED_CAPTURE,
	LED_WAITING_CAPTURE,
	LED_MOTION_DETECTION,
	LED_NO_SDC,
	LED_SDC_FULL,
	LED_CARD_DETE_SUC,
	LED_CAPTURE_FAIL,
	LED_CARD_NO_SPACE,
	LED_TELL_CARD,
	LED_USB_CONNECT,
	LED_POWER_OFF,
	LED_WIFI_ENABLE,
	LED_WIFI_DISABLE,
	LED_CHARGE_FULL,
	LED_CHARGEING,
	LED_RECORD_READY,	   //录像准备灯
	LED_AUTO_RECORD_READY, //自动录像准备灯
	LED_STATUS_INDICATORS, //状态提示灯
	LED_IR_STATUS,		   //红外灯指示
	LED_MOTION_WAITING,    //移动侦测等待     
	LED_MOTION_MODE,       //移动侦测模式  
	LED_MOTION_READY,   
	LED_SAVE_LED,          //录像保存闪灯
	LED_WIFI_ON_SUCC,
	LED_MODE_MAX
}LED_MODE_ENUM;

typedef enum
{
		IMAGE_OUTPUT_FORMAT_RGB1555=0,
		IMAGE_OUTPUT_FORMAT_RGB565,
		IMAGE_OUTPUT_FORMAT_RGBG,
		IMAGE_OUTPUT_FORMAT_GRGB,
		IMAGE_OUTPUT_FORMAT_YUYV,
     	IMAGE_OUTPUT_FORMAT_UYVY,	
	    IMAGE_OUTPUT_FORMAT_YUYV8X32,		
        IMAGE_OUTPUT_FORMAT_YUYV8X64,		
        IMAGE_OUTPUT_FORMAT_YUYV16X32,		
        IMAGE_OUTPUT_FORMAT_YUYV16X64,
        IMAGE_OUTPUT_FORMAT_YUYV32X32,	
        IMAGE_OUTPUT_FORMAT_YUYV64X64,
        IMAGE_OUTPUT_FORMAT_YUV422,
        IMAGE_OUTPUT_FORMAT_YUV420,
        IMAGE_OUTPUT_FORMAT_YUV411,
        IMAGE_OUTPUT_FORMAT_YUV444,        
        IMAGE_OUTPUT_FORMAT_Y_ONLY        
} IMAGE_OUTPUT_FORMAT;

typedef struct {
		INT8U 		bSensorDoInit;
		INT8U		bScaler;
		//INT8U		bUseDefBuf;			//video decode use user define buffer or not 
		//INT8U		*AviDecodeBuf1;		//video decode user define buffer address 
		//INT8U		*AviDecodeBuf2;		//video decode user define buffer address
		INT16U       bEnterApMode;
		INT16U		TargetWidth;		//video encode use
		INT16U		TargetHeight;		//video encode use
		INT16U      ClipWidth;        //video encode use
		INT16U      ClipHeight;       //video encode use
		INT16U      SensorWidth;        //video encode use
		INT16U      SensorHeight;       //video encode use
		INT16U      DisplayWidth;       
		INT16U      DisplayHeight;      
		INT16U		DisplayBufferWidth;
		INT16U 		DisplayBufferHeight;
		INT32U      VidFrameRate;       //for avi encode only
		INT32U      AudSampleRate;      //for avi encode only
		IMAGE_OUTPUT_FORMAT	OutputFormat;
} VIDEO_ARGUMENT;


typedef enum {
	EXIT_RESUME=0,  /*no exit*/
    EXIT_SUSPEND,  /* prepare to entry state suspend mode */   /* after exit, the next state is decision by UMI Task*/
    EXIT_PAUSE_SUSPEND,  /* no release resource suspend, like as suspend for storage INT */  /* after exit, the next state is decision by UMI Task*/
    EXIT_STG_SUSPEND,  /* no release resource suspend, only for storage INT */  /* after exit, the next state is decision by UMI Task*/
    EXIT_USB_SUSPEND,  /* no release resource suspend, only for storage INT */  /* after exit, the next state is decision by UMI Task*/
    EXIT_TO_SHORTCUT,
    EXIT_ALARM_SUSPEND,
    EXIT_BREAK  /* after exit, the next state is define by current state*/
}EXIT_FLAG_ENUM;

//=============================================================================
//======================== Media Status Defintion ============================
// including Audio, Image, Video
//=============================================================================
typedef enum
{
		START_OK=0,
		RESOURCE_NO_FOUND_ERROR,
		RESOURCE_READ_ERROR,
		RESOURCE_WRITE_ERROR,
		CHANNEL_ASSIGN_ERROR,
		REENTRY_ERROR,
		AUDIO_ALGORITHM_NO_FOUND_ERROR,
		CODEC_START_STATUS_ERROR_MAX
} CODEC_START_STATUS;

typedef enum
{
		AUDIO_CODEC_PROCESSING=0,					// Decoding or Encoding
		AUDIO_CODEC_PROCESS_END,					// Decoded or Encoded End
		AUDIO_CODEC_BREAK_OFF,						// Due to unexpended card-plug-in-out
		AUDIO_CODEC_PROCESS_PAUSED,
		AUDIO_CODEC_STATUS_MAX
} AUDIO_CODEC_STATUS;


//=============================================================================
//========================= Media Source Defintion ============================
// including Audio, Image, Video
//=============================================================================
typedef enum
{
		SOURCE_TYPE_FS=0,
		SOURCE_TYPE_SDRAM,
		SOURCE_TYPE_NVRAM,
		SOURCE_TYPE_USER_DEFINE,
		SOURCE_TYPE_FS_RESOURCE_IN_FILE,	// added by Bruce, 2010/01/22
		SOURCE_TYPE_MAX
} SOURCE_TYPE;


typedef struct {
		SOURCE_TYPE type;					//0: GP FAT16/32 File System by File SYSTEM 
											//1: Directly from Memory Mapping (SDRAM)
											//2: Directly from Memory Mapping (NVRAM)
											//3: User Defined defined by call out function:audio_encoded_data_read								
		
		struct User							//Source File handle and memory address
		{
				INT16S		FileHandle;		//File Number by File System or user Define	
				INT32S      temp;			//Reserve for special use 
				INT8U       *memptr;		//Memory start address						
		}type_ID;
		
		union SourceFormat					//Source File Format
		{
				AUDIO_FORMAT	AudioFormat;		//if NULL,auto detect
				IMAGE_FORMAT	ImageFormat;		//if NULL,auto detect
				VIDEO_FORMAT	VideoFormat;		//if NULL,auto detect
		}Format;
} MEDIA_SOURCE;

//=============================================================================
//audio record sampling rate
#define AUD_SAMPLING_RATE_32K			32000
#define AUD_REC_SAMPLING_RATE			AUD_SAMPLING_RATE_32K //AUD_SAMPLING_RATE_32K

#define AUD_FORMAT_WAV                 	0     
#define AUD_FORMAT_MP3                 	1
#define AUD_REC_FORMAT                 	AUD_FORMAT_WAV

#if AUD_REC_FORMAT == AUD_FORMAT_WAV
	#define AUD_FILE_MAX_RECORD_SIZE	300*AUD_REC_SAMPLING_RATE*2	//2GB
#elif AUD_REC_FORMAT == AUD_FORMAT_MP3
	#define AUD_FILE_MAX_RECORD_SIZE	300*16000
#endif

extern INT32S audio_effect_play(INT32U effect_type); //wwj add
// Audio Encode ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void audio_encode_entrance(void);
void audio_encode_exit(void);
CODEC_START_STATUS audio_encode_start(MEDIA_SOURCE src, INT16U SampleRate, INT32U BitRate);
void audio_encode_stop(void);
AUDIO_CODEC_STATUS audio_encode_status(void);
CODEC_START_STATUS audio_encode_set_downsample(INT8U bEnable, INT8U DownSampleFactor);

// Video decode /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern void video_decode_end(void);		//call-back
extern void video_decode_FrameReady(INT8U *FrameBufPtr);

// Video encode /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
extern void video_encode_entrance(void);										//used
extern void video_encode_exit(void);											//used
extern CODEC_START_STATUS video_encode_preview_start(VIDEO_ARGUMENT arg);		//used
extern CODEC_START_STATUS video_encode_preview_stop(void);						//used
extern CODEC_START_STATUS video_encode_start(MEDIA_SOURCE src, INT16S txt_handle);					//used
CODEC_START_STATUS video_encode_fast_stop_and_start(MEDIA_SOURCE src, INT16S next_txt_handle);			//used
extern CODEC_START_STATUS video_encode_stop(void);								//used
extern CODEC_START_STATUS video_encode_auto_switch_csi_frame(void);
extern CODEC_START_STATUS video_encode_auto_switch_csi_fifo_end(INT8U flag);
extern CODEC_START_STATUS video_encode_auto_switch_csi_frame_end(INT8U flag);
extern CODEC_START_STATUS video_encode_set_zoom_scaler(FP32 zoom_ratio);
extern INT32U video_encode_sensor_start(INT32U csi_frame1, INT32U csi_frame2);
extern INT32U video_encode_sensor_stop(void);
extern INT32S video_encode_display_frame_ready(INT32U frame_buffer);
extern void video_encode_end(void *workmem);									//used
extern CODEC_START_STATUS video_encode_capture_picture(MEDIA_SOURCE src);		//used
extern CODEC_START_STATUS video_encode_fast_switch_stop_and_start(MEDIA_SOURCE src);
#if AUDIO_SFX_HANDLE
extern INT32U video_encode_audio_sfx(INT16U *PCM_Buf, INT32U cbLen);
#endif

// display /////////////////////////////////////////////////////////////////////////////////////////////
extern void video_codec_show_image(INT8U TV_TFT, INT32U BUF,INT32U DISPLAY_MODE ,INT32U SHOW_TYPE);
extern void user_defined_video_codec_entrance(void);

extern INT16S unlink2(CHAR *filename);
extern void state_usb_entry(void* para1);
extern INT32S usb_webcam_start(void);
extern INT32S usb_webcam_stop(void);
extern void timer_counter_force_display(INT8U force_en);
extern void ap_peripheral_auto_off_force_disable_set(INT8U auto_off_disable);	//wwj add

typedef struct 
{
    INT16S file_handle;
    #if GPS_TXT
    INT16S txt_handle;
    #endif
    INT16S storage_free_size;
    INT32U file_path_addr;
    #if GPS_TXT
    INT32U txt_path_addr;
    #endif
} STOR_SERV_FILEINFO;

typedef enum
{
	STOR_SERV_SEARCH_INIT = 0,
	STOR_SERV_SEARCH_ORIGIN,
	STOR_SERV_SEARCH_PREV,
	STOR_SERV_SEARCH_NEXT,
	STOR_SERV_SEARCH_GIVEN
} STOR_SERV_PLAYSEARCH;

typedef enum
{
	STOR_SERV_IDLE = 0,
	STOR_SERV_OPEN_FAIL,
	STOR_SERV_OPEN_OK,
	STOR_SERV_DECODE_ALL_FAIL,
	STOR_SERV_NO_MEDIA
} STOR_SERV_PLAYSTS;

typedef struct 
{
    INT16S file_handle;
    INT16S play_index;
    INT16U total_file_number;
    INT16U deleted_file_number;
    INT8U file_type;
    INT8U search_type;
    INT8S err_flag;
    INT32U file_size;
    INT32U file_path_addr;
} STOR_SERV_PLAYINFO;

typedef struct 
{
    INT16S file_handle;
    INT16S play_index;
    INT32U file_size;
    INT32U file_path_addr;
    INT32U file_offset;
} STOR_SERV_DOWNLOAD_FILEINFO;

typedef struct{
    INT16U font_color;
    INT16U font_type;
    INT16S pos_x;
    INT16S pos_y;
    INT16U buff_w;
    INT16U buff_h;
    INT16U language;
    INT16U str_idx;
	INT16U font_offset_h_start;
	INT16U font_offset_h_width;
}STRING_INFO;

typedef struct 
{
    INT16U w;
    INT16U h;
    INT32U addr;
} STR_ICON;

typedef struct 
{
    INT8U num;
    INT16U w;
    INT16U h;
    INT32U addr;
	INT16U pos_x;
	INT16U pos_y;
} STR_ICON_EXT;

extern void cpu_draw_time_osd(TIME_T current_time, INT32U target_buffer, INT8U draw_type, INT8U state,INT32U ImgWidth,INT32U ImgHeight);
extern INT32S ap_state_resource_char_draw(INT16U target_char, INT16U *frame_buff, STRING_INFO *str_info, INT8U type, INT8U num_type);

typedef struct {
	INT16U icon_w;
	INT16U icon_h;
	INT32U transparent;
	INT16U pos_x;
	INT16U pos_y;
} DISPLAY_ICONSHOW;

typedef struct {
	INT16U idx;
	INT16U pos_x;
	INT16U pos_y;
} DISPLAY_ICONMOVE;

extern INT16U ui_background_all[];
extern INT16U ui_current_select[];
extern INT16U icon_playback_movie[];
//extern INT16U thumbnail_video_icon[];
extern INT16U thumbnail_cursor_3x3_96x64[];
extern INT16U thumbnail_cursor_3x3_black_96x64[];
extern INT16U thumbnail_lock_icon[];
extern INT16U ui_up[];
extern INT16U ui_left[];
extern INT16U ui_right[];
extern INT16U ui_down[];

extern void task_peripheral_handling_entry(void *para);
extern void state_handling_entry(void *para);
extern void task_display_entry(void *para);
extern void task_storage_service_entry(void *para);
extern void tft_vblank_isr_register(void (*user_isr)(void));
extern void tv_vblank_isr_register(void (*user_isr)(void));

#if C_MOTION_DETECTION == CUSTOM_ON
	extern void motion_detect_isr_register(void (*user_isr)(void));
#endif

extern INT32S vid_dec_entry(void);
extern INT32S vid_dec_get_file_format(INT8S *pdata);
extern INT32S vid_dec_parser_start(INT16S fd, INT32S FileType, INT64U FileSize);
extern void vid_dec_get_size(INT16U *width, INT16U *height);
extern INT32S vid_dec_start(void);
extern INT32S vid_dec_stop(void);
extern INT32S vid_dec_parser_stop(void);
extern INT32S vid_dec_exit(void);
extern INT32S vid_dec_pause(void);
extern INT32S vid_dec_resume(void);
extern INT32S vid_dec_get_status(void);
extern INT32S vid_dec_nth_frame(INT32U nth);

typedef struct
{
	struct STFileNodeInfo audio;
	INT8S  ext_name[24];//080903
	INT8U  scan_status;
}st_storage_file_node_info;

#define SD_SLOT_ID				0
#define MAX_SLOT_NUMS			1
extern  st_storage_file_node_info FNodeInfo[MAX_SLOT_NUMS];
#define USB_PHY_SUSPEND			1
extern INT8U storage_sd_upgrade_file_flag_get(void);

extern void VdoFramNumsLowBoundReg(void *WorkMem, unsigned int fix_frame_cnts);
extern unsigned long current_movie_Byte_size_get(void *WorkMem);
extern void AviPacker_Break_Set(void *WorkMem, INT8U Break1_Work0);

extern INT8U frame_mode_en;

#define AUDIO_PROGRESS_SUPPORT	1
#define AUDIO_BG_DECODE_EN		0


#define DISPLAY_DATE_TIME_RECORD		1
#define DISPLAY_DATE_TIME_RECORD2		2
#define DISPLAY_DATE_TIME_BROWSE		3

#define BLUE_COLOR		0x001F
#define BLUE1_COLOR		0x87BE
#define RED_COLOR		0xF800
#define WHITE_COLOR		0xFFFF

#define WARNING_STR_COLOR	WHITE_COLOR

/****************************************************************************/
extern CODEC_START_STATUS video_encode_preview_off(void);
extern CODEC_START_STATUS video_encode_preview_on(void);

/***************** LDW audio resouce *****************/
#define LDW_SIZE 3
typedef struct _LDW_Fmt
{
	INT32U start_addr;
	INT32U cur_pos;
	INT32U length;
}LDW_Fmt;


//#define HDMI_JPG_DECODE_AS_GP420
#define HDMI_JPG_DECODE_AS_YUV422


// MJPEG STREAM
typedef enum MJPEG_EVENT_E
{
	MJPEG_SEND_EVENT = 0,
	MJPEG_STOP_EVENT = 1,	
	MJPEG_NETWORK_BUSY_EVENT
} MJPEG_EVENT_T;

typedef struct mjpeg_write_data_s
{


	INT32U msg_id;
	INT32U mjpeg_addr;
	INT32U mjpeg_size;
	INT32U running_app;
	INT8U mjpeg_addr_idx;	
}mjpeg_write_data_t;


/****************************************************************************/
#define DUAL_STREAM_FUNC_ENABLE		1
#define PREVIEW_TCPIP		1		// 1:HTTP   /   0:RSTP


#endif 		// __APPLICATION_H__

