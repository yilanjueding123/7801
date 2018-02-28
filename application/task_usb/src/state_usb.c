#include "ap_usb.h"
#include "state_usb.h"
#include "udc_s220.h"
#include "drv_l1_system.h"
#include "drv_l1_usbd.h"
#include "drv_l2_usbd_msdc.h"
#include "avi_encoder_app.h"   // 它會送 event 進 UVC, UAC 的 QUEUE
#include "ap_state_handling.h"

extern INT8U screen_saver_enable;
static INT8U usb_state_flag=0;//0:拸USB 拸鳶籟  1:USB  2:鳶籟  3:usb or 鳶籟 4: USB麼鳶籟筍祥輛�蕔SB耀宒
/******************************************************
    Definition and variable declaration
*******************************************************/
#if 1
	#define USB_PRINT DBG_PRINT
#else
	#define USB_PRINT
#endif

///////////////// USB ////////////////////
enum {
	USB_IDLE = 0,
	USB_MSDC,
	USB_UVC,
	USB_EXIT, 
	USB_ERR
};
#define USB_TASK_QUEUE_MAX	32
OS_EVENT *USBTaskQ;
void *usb_task_q_stack[USB_TASK_QUEUE_MAX];
INT32U usb_status;

///////////////// USB_UVC Task ////////////////
#define C_USB_CAM_STATE_STACK_SIZE   128
INT32U  USBCamStateStack[C_USB_CAM_STATE_STACK_SIZE];
OS_EVENT  *USBCamApQ;
#define USB_CAM_QUEUE_MAX_LEN    	 20
void *USBCamApQ_Stack[USB_CAM_QUEUE_MAX_LEN];
///////////////// USB_UAC Task ////////////////
#define C_USB_AUDIO_STACK_SIZE			128
INT32U  USBAudioStack[C_USB_AUDIO_STACK_SIZE];
OS_EVENT *USBAudioApQ;
#define USB_AUDIO_QUEUE_MAX_LEN			8
void *USBAudioQ_Stack[USB_AUDIO_QUEUE_MAX_LEN];

//////////////////////////////////////////////
extern void drv_l1_usb_soft_disconnect(void);
extern INT32S drv_l2_usbd_ctl_uninit(void);
extern void usbd_l2_main_task_exit(void);
void usb_uninitial_switch(void);
///////////////// CHARGE ////////////////

/******************************************************
						W E B C A M
*******************************************************/
static void uac_entry(void* para1)
{
    ISOTaskMsg  *isosend;
    INT8U err;

	while (1)
	{
		isosend = (ISOTaskMsg*) OSQPend(USBAudioApQ, 0, &err);
		usb_send_audio((INT8U*)(isosend->AddrFrame), isosend->FrameSize);
	}
}
static INT32S usb_uac_task_create(void)
{
	INT8U err;
	
	USBAudioApQ = OSQCreate(USBAudioQ_Stack, USB_AUDIO_QUEUE_MAX_LEN);
    if(!USBAudioApQ) 
		return STATUS_FAIL;

    err = OSTaskCreate(uac_entry, (void*)NULL, &USBAudioStack[C_USB_AUDIO_STACK_SIZE - 1], USB_AUDIO_PRIORITY);
	if(err != OS_NO_ERR)
		return STATUS_FAIL;

	return STATUS_OK;
}
static INT32S usb_uac_task_del(void)
{
	INT8U   err;

	OSTaskSuspend(USB_AUDIO_PRIORITY); 
	OSTimeDly(1);
	OSTaskDelReq(USB_AUDIO_PRIORITY);	
	OSTaskDel(USB_AUDIO_PRIORITY);
	OSQFlush(USBAudioApQ);
	OSQDel(USBAudioApQ, OS_DEL_ALWAYS, &err);

	return STATUS_OK;
}

static void uvc_entry(void* para1)
{
    ISOTaskMsg  *isosend;
    INT8U err;

	while (1)
	{
		isosend = (ISOTaskMsg*) OSQPend(USBCamApQ, 0, &err);
		usb_send_video((INT8U*)(isosend->AddrFrame), isosend->FrameSize);
	}
}
static INT32S usb_uvc_task_create(void)
{
	INT8U err;
	//INT32S nRet;
	//creat usb cam message
	USBCamApQ = OSQCreate(USBCamApQ_Stack, USB_CAM_QUEUE_MAX_LEN);
    if(!USBCamApQ) 
		return STATUS_FAIL;
	
    //creat usb cam task
    err = OSTaskCreate(uvc_entry,  (void*)NULL,&USBCamStateStack[C_USB_CAM_STATE_STACK_SIZE - 1], USB_CAM_PRIORITY);
	if(err != OS_NO_ERR)
		 return STATUS_FAIL;

	return STATUS_OK;
}
static INT32S usb_uvc_task_del(void)
{

	INT8U   err;
    //INT32S  nRet;

	OSTaskSuspend(USB_CAM_PRIORITY); 
	OSTimeDly(1);
	OSTaskDelReq(USB_CAM_PRIORITY);
	OSTaskDel(USB_CAM_PRIORITY);
	OSQFlush(USBCamApQ);
	OSQDel(USBCamApQ, OS_DEL_ALWAYS, &err);
	
	 return STATUS_OK;
}


/******************************************************
						M S D C
*******************************************************/
static INT32U usb_bus_priority = 0;
static INT32U dma_bus_priority = 0;

/******************************************************
						C H A R G E
*******************************************************/
#define USB_PHY_SWITCH 1

void usb_phy_clk_off(void)
{
#if USB_PHY_SWITCH
	rSYS_CTRL_NEW &= ~(1 << 8);	
	rUDCCS_UDC &= (~(BIT26 | BIT27 | BIT28));
	__asm {NOP};
	__asm {NOP};
	__asm {NOP};
	__asm {NOP};
	__asm {NOP};
	__asm {NOP};
	__asm {NOP};
		
	rUDLC_SET0 &= (~(BIT2|BIT3));
	rUDLC_SET0 |= (BIT5|BIT4|BIT0);
	rUDCCS_UDC |= BIT31;
	rUDCCS_UDC &= (~(BIT29|BIT28));
	R_SYSTEM_CLK_EN1 &= (~0x4);		// USB device clk off	
#else
	rSYS_CTRL_NEW &= ~(1 << 8);	
	R_SYSTEM_CLK_EN1 |= 0x4; 	// USB device clk on
#endif
}

void usb_phy_clk_on(void)
{
#if USB_PHY_SWITCH
	R_SYSTEM_CLK_EN1 |= 0x4; 	// USB device clk on
	rSYS_CTRL_NEW &= ~(1 << 8);		
	 rUDCCS_UDC |= (BIT26 | BIT27 | BIT28|BIT29);	
#else
	rSYS_CTRL_NEW &= ~(1 << 8);	
	R_SYSTEM_CLK_EN1 |= 0x4; 	// USB device clk on
#endif
}

static void state_usb_init(void)
{
	usb_phy_clk_on();
	OSTimeDly(10);
	drv_l1_usbd_init();
}

INT8U usb_state_get(void)
{
 return usb_state_flag;
}
void usb_state_set(INT8U flag)
{
 usb_state_flag=flag;
}

static void state_usb_isr(void)
{
	//USB_PRINT("rDLCIF_UDLC=0x%x, rSTANDARD_REQ_IF=0x%x, rUDC_IRQ_FLAG=0x%x, rNEWEP_IF=0x%x\r\n",rDLCIF_UDLC,rSTANDARD_REQ_IF,rUDC_IRQ_FLAG,rNEWEP_IF);
	if  ( rDLCIF_UDLC & MASK_USBD_UDLC_IF_RESET )  // 偵測 Reset 信號
	{
		usb_state_flag=1;
		//drv_l1_usb_soft_disconnect();  // 把 D+, D- 壓下去，不丟信號給PC
		rUDCCS_UDC |= MASK_USBD_UDC_CS_SYS_DISCONNECT;			
		vic_irq_disable(VIC_USB);
		vic_irq_unregister(VIC_USB);		
		msgQSend(ApQ, MSG_APQ_CONNECT_TO_PC, NULL, NULL, MSG_PRI_NORMAL);
		
		usb_bus_priority	 = R_MEM_M7_BUS_PRIORITY;
		dma_bus_priority = R_MEM_M16_BUS_PRIORITY;
	}

	// clear interrupt
	rSTANDARD_REQ_IF = rSTANDARD_REQ_IF;
	rDLCIF_UDLC = rDLCIF_UDLC;
	rUDC_IRQ_FLAG = rUDC_IRQ_FLAG;
	rNEWEP_IF = rNEWEP_IF;
}

extern void ap_video_capture_mode_switch(INT8U DoSensorInit, INT16U EnterAPMode);
void state_usb_entry(void* para1)
{
    INT32U msg_id;
    INT16U time_out = 0;
    INT8U err;

	usb_phy_clk_off();
	usb_status = USB_IDLE;
       USBTaskQ = OSQCreate(usb_task_q_stack, USB_TASK_QUEUE_MAX);

	while (1) {

		msg_id = (INT32U) OSQPend(USBTaskQ, time_out, &err);

	    switch (msg_id) {
			case MSG_USBD_INITIAL:
			case MSG_USBCAM_INITIAL:

				// 若螢幕保護開時，要點亮背光
				if(screen_saver_enable) {
					screen_saver_enable = 0;
		        		ap_state_handling_lcd_backlight_switch(1);
				}

				if  ( (usb_status==USB_MSDC)||(usb_status==USB_UVC) )
				{	// 插拔太快，以致於前一狀態還沒結束
					USB_PRINT("USB status error\r\n");
					break;
				}				
				usb_state_flag=2;			
				state_usb_init();
	    			vic_irq_register(VIC_USB, state_usb_isr);
   				vic_irq_enable(VIC_USB);
				usb_status = USB_IDLE;
				time_out = 0;
				USB_PRINT("USB detect\r\n");
				break;
				
			case MSG_USBD_PLUG_IN:
				sys_msdc_clk_active();
				/* 將 USB bus的優先權調低，以免過最危險的區域 */
				R_MEM_M7_BUS_PRIORITY = 0x5F;
				R_MEM_M16_BUS_PRIORITY = 0x01;
				/* 初始化 */
				usb_status = USB_MSDC;				
				USBD_MSDC_Init();
				time_out = 10;
				break;

			case MSG_USBCAM_PLUG_IN:
				usb_status = USB_UVC;
				OSTimeDly(2);
				usb_uvc_start();
				usb_uvc_task_create();
				usb_uac_task_create();
				usb_webcam_start();		// 通知別的線程
				time_out = 10;
				break;
				
			case MSG_USBCAM_PLUG_OUT:
				usb_uninitial();
				usb_webcam_stop();		// 通知別的線程
				usb_uvc_task_del();		// 也許 Video 還繼續送過來，所以要等 stop 後再結束				
				usb_uac_task_del();				
				time_out = 0;
				usb_status = USB_EXIT;
				USB_PRINT("USBCAM Plug Out\r\n");
				break;

			case MSG_USBD_PLUG_OUT:
				drv_l2_usbd_msdc_uninit();
				usb_uninitial();
				sys_msdc_clk_restore();
				time_out = 0;
				usb_status = USB_EXIT;
				USB_PRINT("MSDC Plug Out\r\n");								
				break;
				
		    case MSG_USBD_SWITCH:
		    	if (usb_status==USB_MSDC)
		    	{
		    		drv_l2_usbd_msdc_uninit();
					usb_uninitial_switch();
					sys_msdc_clk_restore();
					time_out = 0;
					ap_video_capture_mode_switch(0, STATE_CONNECT_TO_PC);
		    		OSQPost(USBTaskQ, (void *)MSG_USBCAM_PLUG_IN);
		    	}
		    	else if (usb_status==USB_UVC)
		    	{
		    		usb_uninitial_switch();
					usb_webcam_stop();		// 通知別的線程
					usb_uvc_task_del();		// 也許 Video 還繼續送過來，所以要等 stop 後再結束				
					usb_uac_task_del();				
					time_out = 0;
					vid_enc_disable_sensor_clock();
					video_encode_preview_off();
					OSQPost(USBTaskQ, (void *)MSG_USBD_PLUG_IN);
		    	}
		    	break;

			case MSG_CHARGE_PLUG_OUT:
				if (  (rUDCCS_UDC & MASK_USBD_UDC_CS_SYS_SUSPENDM)&&(usb_status==USB_IDLE) )
				{
					DBG_PRINT("Turn Off USB PHY clk\r\n");
					usb_state_flag=0;
					usb_phy_clk_off();
				}
				break;

			default:	// time out
				if (usb_status==USB_MSDC)
				{
					if (s_usbd_pin == 0) {	// USB 線拔除
						OSQPost(USBTaskQ, (void *)MSG_USBD_PLUG_OUT);
					}
					else {
						// USB_PRINT("SD CARD check\r\n");
						drv_l2_usbd_msdc_process_insertion();
					}
				}
				if (usb_status==USB_UVC)
				{
					if (s_usbd_pin == 0) {	// USB 線拔除
						OSQPost(USBTaskQ, (void *)MSG_USBCAM_PLUG_OUT);
					}
				}
	   	}

	}
}

void usb_disconnect_wait(void)
{    // wait until switch sys_clk succeed.
	INT32U i = 0;
	while (1)
	{
		OSTimeDly(1);
		if (usb_status==USB_EXIT)
		{
			DBG_PRINT("USB disconnect OK\r\n");
			break;
		}	
		if (i==1000)
		{
			DBG_PRINT("USB disconnect fail\r\n");
			break;
		}
		i++;
	}
}

void usb_uninitial(void)
{
	usb_state_flag=0;

	R_MEM_M7_BUS_PRIORITY = usb_bus_priority;
	R_MEM_M16_BUS_PRIORITY = dma_bus_priority;
	usbd_l2_main_task_exit();	
	drv_l2_usbd_ctl_uninit();

	vic_irq_disable(VIC_USB);
	vic_irq_unregister(VIC_USB);

	/* Force USB PHY disconnect to USB bus */
	rUDCCS_UDC &= ~MASK_USBD_UDC_CS_SYS_SUSPENDM;	// PHY clk off
	rUDCCS_UDC |= MASK_USBD_UDC_CS_SYS_DISCONNECT;	// Disable UDC
	usb_phy_clk_off();
}

void usb_uninitial_switch(void)
{
	//R_MEM_M7_BUS_PRIORITY = usb_bus_priority;
	//R_MEM_M16_BUS_PRIORITY = dma_bus_priority;
	usbd_l2_main_task_exit();	
	drv_l2_usbd_ctl_uninit();

	/* Force USB PHY disconnect to USB bus */
	rUDCCS_UDC &= ~MASK_USBD_UDC_CS_SYS_SUSPENDM;	// PHY clk off
	rUDCCS_UDC |= MASK_USBD_UDC_CS_SYS_DISCONNECT;	// Disable UDC
}

