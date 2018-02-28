#include "ap_peripheral_handling.h"
#include "ap_state_config.h"
#include "ap_state_handling.h"
//#include "drv_l1_system.h"
#include "driver_l1.h"
#include "drv_l1_cdsp.h"
#include "state_wifi.h"
//#include "ir.h"

#define LED_STATUS_FLASH	1
#define LED_STATUS_BLINK	2

#define CRAZY_KEY_TEST		0		// Send key events faster than human finger can do
#define LED_ON              1
#define LED_OFF             0

static INT8U led_status;	//0: nothing  1: flash	2: blink
static INT8U led_cnt;
static INT8U nOldWifiSta = 0;
//------------------------------------------------------------------
static INT8U 	g_led_count;
static INT8U 	g_led_r_state;	//0 = OFF;	1=ON;	2=Flicker
static INT8U	g_led_b_state;
static INT8U	g_led_y_state;
static INT8U	g_led_g_state;
static INT8U	g_led_flicker_state;	//0=同时闪烁	1=交替闪烁
static INT8U    led_red_flag;
static INT8U    led_yellow_flag;
static INT8U    led_wifi_flag;
static INT8U    led_blue_flag;
static INT8U    led_ir_flag;
//static INT8U    savefile_led_flag = 0; //保存录像文件时闪灯的标志, =1,闪灯
//------------------------------------------------------------------
extern void led_wifi_on(void);
extern void led_wifi_off(void);

extern INT8U usb_state_get(void);
extern void usb_state_set(INT8U flag);
static INT8U Low_Voltage_Flag = 0;

#if TV_DET_ENABLE
INT8U tv_plug_in_flag;
INT8U tv_debounce_cnt = 0;
#endif
static	INT8U tv = !TV_DET_ACTIVE;
static INT8U backlight_tmr = 0;

#if C_SCREEN_SAVER == CUSTOM_ON
	INT8U auto_off_force_disable = 0;
	void ap_peripheral_auto_off_force_disable_set(INT8U);
#endif

static INT8U led_flash_timerid;
static INT16U config_cnt;
static INT8U nChargingMode = 0;
extern INT8U nWifiSta = 0;
//----------------------------
    typedef struct {
        INT8U byRealVal ;
        INT8U byCalVal;
    } AD_MAP_t;


//----------------------------
	extern void avi_adc_gsensor_data_register(void **msgq_id, INT32U *msg_id);

	INT8U gsensor_lock_flag = 0;
	INT8U gsensor_data[2][32] = {0};
	static void   *gsensor_msgQId0 = 0;
	static INT32U gsensor_msgId0 = 0;

	static INT16U adc_battery_value_new, adc_battery_value_old;
	static INT32U battery_stable_cnt = 0;

	#define C_BATTERY_STABLE_THRESHOLD		4  // Defines threshold number that AD value is deemed stable

#if C_BATTERY_DETECT == CUSTOM_ON
	static INT16U low_voltage_cnt;
    static INT32U battery_value_sum=0;
	static INT8U bat_ck_cnt=0;
#endif



#if USE_ADKEY_NO
	static INT8U ad_detect_timerid;
	static INT16U ad_value;
	static KEYSTATUS ad_key_map[USE_ADKEY_NO+1];

#if KEY_TYPE == KEY_TYPE0
	static INT8U  ad_line_select = 0;
	static INT16U adc_key_release_value_old, adc_key_release_value_new, adc_key_release_value_stable;
	static INT32U key_release_stable_cnt = 0;
#endif

	#define C_RESISTOR_ACCURACY				5//josephhsieh@140418 3			// 2% accuracy
#if GPDV_BOARD_VERSION == GPCV1248_MINI 
	#define C_KEY_PRESS_WATERSHED			150//josephhsieh@140418 175
#else
	#define C_KEY_PRESS_WATERSHED			600//josephhsieh@140418 175
#endif
	#define C_KEY_STABLE_THRESHOLD			4//josephhsieh@140418 3			// Defines threshold number that AD value of key is deemed stable
	#define C_KEY_FAST_JUDGE_THRESHOLD 		40			// Defines threshold number that key is should be judge before it is release. 0=Disable
	#define C_KEY_RELEASE_STABLE_THRESHOLD	4  // Defines threshold number that AD value is deemed stable

	INT16U adc_key_value;

    //static INT8U  ad_value_cnt ;
	INT32U key_pressed_cnt;
	INT8U fast_key_exec_flag;
	INT8U normal_key_exec_flag;
	INT8U long_key_exec_flag;
#endif

//static INT8U tf_state_flag = 0;
static INT32U video_busy_cnt = 0; //时间计数,执行两次MSG_APQ_VIDEO_RECORD_ACTIVE消息之间要间隔至少1秒以上的时间, 否则有可能引起模式混乱

static INT32U wifi_key_snd_flag = 0;		// 硈 Wi-Fi ぃ璶Τ龄
static INT32U key_active_cnt;
static INT8U lcd_bl_sts;
static INT8U power_off_timerid;
static INT8U usbd_detect_io_timerid;
static KEYSTATUS key_map[USE_IOKEY_NO];
static INT8U key_detect_timerid;
static INT16U adp_out_cnt;
static INT16U usbd_cnt;
#if USB_PHY_SUSPEND == 1
static INT16U phy_cnt = 0;
#endif
static INT16U adp_cnt;
 INT8U  adp_status;
static INT8U battery_low_flag = 0;
INT8U  usbd_exit;
volatile INT8U s_usbd_pin;
extern INT8U MODE_KEY_flag;
extern volatile INT8U g_sequence_start_flag;
volatile INT8U sd_flag = 0; //有卡: 2,  无卡: 0,   容量不足: 1
volatile INT8U g_motion_mode_enable = 0;
extern volatile INT8U video_down_flag;
extern volatile  INT8U pic_down_flag;

//	prototypes
void ap_peripheral_key_init(void);
void ap_peripheral_rec_key_exe(INT16U *tick_cnt_ptr);
void ap_peripheral_function_key_exe(INT16U *tick_cnt_ptr);
void ap_peripheral_next_key_exe(INT16U *tick_cnt_ptr);
void ap_peripheral_prev_key_exe(INT16U *tick_cnt_ptr);
void ap_peripheral_ok_key_exe(INT16U *tick_cnt_ptr);
void ap_peripheral_sos_key_exe(INT16U *tick_cnt_ptr);
void ap_peripheral_usbd_plug_out_exe(INT16U *tick_cnt_ptr);
void ap_peripheral_pw_key_exe(INT16U *tick_cnt_ptr);
void ap_peripheral_menu_key_exe(INT16U *tick_cnt_ptr);
#if  C_MOTION_DETECTION == CUSTOM_ON
void ap_peripheral_md_key_exe(INT16U *tick_cnt_ptr);
#endif
#if KEY_FUNTION_TYPE == SAMPLE2
void ap_peripheral_capture_key_exe(INT16U *tick_cnt_ptr);
#endif
void ap_peripheral_wifi_key_exe(INT16U *tick_cnt_ptr);

void ap_peripheral_null_key_exe(INT16U *tick_cnt_ptr);
#if USE_ADKEY_NO
	void ap_peripheral_ad_detect_init(INT8U adc_channel, void (*bat_detect_isr)(INT16U data));
	void ap_peripheral_ad_check_isr(INT16U value);
#endif	

void LED_blanking_isr(void);
extern INT8U gp_mode_flag_get(void);
extern INT32U Wifi_State_Get(void);
//------------------------------------------
//闪灯模式
//= RED_FLIKER: 红灯闪；  
//= BLUE_FLIKER：蓝灯闪；
//= GREEN_FLIKER：绿灯闪
static INT32U LedFilkerStep = 0; 
static INT8U LedFilkerCnt = 0; //闪灯次数
static INT32U LastLedMode = 0xaa; //上一次的闪灯状态，用于恢复原来的LED状态
static INT32U NowLedMode = 0x55;  //本次闪灯状态
INT32U LedFilkerTime = 0;  //LED亮或灭的时间，=128为1秒，闪灯时亮与灭时间相同
static INT8U wifi_connected_ok = 0;

#define RED_FLIKER		0X10 //红灯闪
#define BLUE_FLIKER		0X20 //蓝灯闪
#define YELLOW_FLIKER	0X30 //黄灯闪
#define WIFI_FLIKER		0x40 //WIFI灯闪

void LED_pin_init(void)
{
	INT32U type;
	//led init as ouput pull-low
  	gpio_init_io(LED1, GPIO_OUTPUT);
  	gpio_set_port_attribute(LED1, ATTRIBUTE_HIGH);
  	gpio_write_io(LED1, LED1_ACTIVE^1);
  	
  	gpio_init_io(LED2, GPIO_OUTPUT);
  	gpio_set_port_attribute(LED2, ATTRIBUTE_HIGH);
  	gpio_write_io(LED2, LED2_ACTIVE^1);

  	gpio_init_io(LED3, GPIO_OUTPUT);
  	gpio_set_port_attribute(LED3, ATTRIBUTE_HIGH);
  	gpio_write_io(LED3, LED3_ACTIVE^1);

  	gpio_init_io(WIFI_LED, GPIO_OUTPUT);
  	gpio_set_port_attribute(WIFI_LED, ATTRIBUTE_HIGH);
  	gpio_write_io(WIFI_LED, WIFI_LED_ACTIVE^1);
  	
//====================================================================
//IR 是4个IO并到一起的
  	
  	gpio_init_io(IR_LED1, GPIO_OUTPUT);
  	gpio_set_port_attribute(IR_LED1, ATTRIBUTE_HIGH);
  	gpio_write_io(IR_LED1, IR_LED_ACTIVE^1);
	
	//====================================================================
  	led_red_flag=LED_OFF;
	led_wifi_flag=LED_OFF;
	led_blue_flag = LED_OFF;
	led_yellow_flag = LED_OFF;
	led_ir_flag = LED_OFF;
	//LED init放到其他地方去

	type = LED_INIT;
	msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &type, sizeof(INT32U), MSG_PRI_NORMAL);
	
  	sys_registe_timer_isr(LED_blanking_isr);	//timer base c to start adc convert
}

void ChargeMode_set(INT8U m)
{
	nChargingMode = m;
}

INT8U ChargeMode_get(void)
{
	return nChargingMode;
}

INT8U ap_peripheral_low_vol_get(void)
{
	return Low_Voltage_Flag;
}

INT8U ap_sd_status_get(void)
{
	return sd_flag;
}

void ap_sd_status_set(INT8U flag)
{
	sd_flag = flag;
}

void ap_peripheral_md_mode_set(INT8U sta)
{
	if (g_motion_mode_enable != sta) g_motion_mode_enable = sta;
}

INT8U ap_peripheral_md_mode_get(void)
{
	return g_motion_mode_enable;
}
//========================================================
//没有使用的IO设置为输入下拉, 并将其接到GND上, 有利于散热
const INT16U COOLER_PIN_ARRAY[]={
COOLER_PIN_IO_01,
COOLER_PIN_IO_02,
COOLER_PIN_IO_03,
COOLER_PIN_IO_04,
COOLER_PIN_IO_05,
COOLER_PIN_IO_06,
COOLER_PIN_IO_07,
COOLER_PIN_IO_08,
COOLER_PIN_IO_09,
COOLER_PIN_IO_10,
COOLER_PIN_IO_11,
};

void ap_peripheral_handling_cooler_pin_init(void)
{
	INT8U i;
	for(i=0; i<COOLER_PIN_NUM; i++)
	{
		gpio_init_io(COOLER_PIN_ARRAY[i],GPIO_INPUT);
	  	gpio_set_port_attribute(COOLER_PIN_ARRAY[i], ATTRIBUTE_LOW);
	  	gpio_write_io(COOLER_PIN_ARRAY[i], DATA_LOW);
	}
}
//====================================================
#if 1
static void init_usbstate(void)
{
	static INT8U usb_dete_cnt=0;
	static INT8U err_cnt=0;

	while(++err_cnt<100)
	{
		if(sys_pwr_key1_read())
			usb_dete_cnt++;
		else
		{
			usb_dete_cnt=0;
			break;
		}
		if(usb_dete_cnt > 3) break;
		OSTimeDly(1);
	}
	if(usb_dete_cnt > 3)
	usb_state_set(3);
	err_cnt=0;
}
#endif

void ap_peripheral_init(void)
{
#if TV_DET_ENABLE
	INT32U i;
#endif	

	power_off_timerid = usbd_detect_io_timerid = led_flash_timerid = 0xFF;
	key_detect_timerid = 0xFF;
  	
  	led_status = 0;
  	led_cnt = 0;

#if TV_DET_ENABLE
	gpio_init_io(AV_IN_DET,GPIO_INPUT);
  	gpio_set_port_attribute(AV_IN_DET, ATTRIBUTE_LOW);
  	gpio_write_io(AV_IN_DET, !TV_DET_ACTIVE);	//pull high or low

	tv_plug_in_flag = 0;
	for(i=0; i<5; i++) {
		if(gpio_read_io(AV_IN_DET) == !TV_DET_ACTIVE) {
			break;
		}
		OSTimeDly(1);
	}

	if(i==5) {
		tv = TV_DET_ACTIVE;
		tv_plug_in_flag = 1;
	}
#endif
/*
	gpio_init_io(HDMI_IN_DET,GPIO_INPUT);
  	gpio_set_port_attribute(HDMI_IN_DET, ATTRIBUTE_LOW);
  	gpio_write_io(HDMI_IN_DET, 0);	//pull low

	gpio_init_io(SPEAKER_EN,GPIO_OUTPUT);
  	gpio_set_port_attribute(SPEAKER_EN, ATTRIBUTE_HIGH);
*/

/*
#if TV_DET_ENABLE
	if(tv_plug_in_flag) {
	  	gpio_write_io(SPEAKER_EN, 0); //mute local speaker
	} else 
#endif	
	{
	  	gpio_write_io(SPEAKER_EN, 0); //mute local speaker
	}
*/
	LED_pin_init();
	init_usbstate();
#if C_IR_REMOTE
	ir_init();
#endif
	ap_peripheral_key_init();
	
	gpio_init_io(CHARGE_DETECTION_PIN,GPIO_INPUT);
  	gpio_set_port_attribute(CHARGE_DETECTION_PIN, ATTRIBUTE_LOW);
  	gpio_write_io(CHARGE_DETECTION_PIN, DATA_HIGH);	//pull high

#if USE_ADKEY_NO
	ad_detect_timerid = 0xFF;
	ap_peripheral_ad_detect_init(AD_KEY_DETECT_PIN, ap_peripheral_ad_check_isr);
#else
	adc_init();
#endif
	config_cnt = 0;
	MODE_KEY_flag = 2;

	gsensor_lock_flag = 0;


}

INT8U wifi_status_get(void)
{
	return wifi_key_snd_flag;
}
//========================================================================
void led_power_off(void)
{	
	INT32U t;

	wifi_connected_ok = 0;
	wifi_key_snd_flag = 0;
	nWifiSta = 0x10;
	led_all_off();
	led_wifi_off();
	led_ir_off();
	for(t=0; t<5; t++)
	{
		OSTimeDly(20);
		led_red_on();
		OSTimeDly(20);
		led_red_off();
	}
	DBG_PRINT("Power off LED ON...\r\n");
}

static void charge_flash_pro(void)
{
	if(gpio_read_io(CHARGE_DETECTION_PIN)== 0)
	{
		led_all_off();
		led_ir_off();
		led_wifi_off();
		g_led_r_state=2;
	}
	else
	{
		led_all_off();
		led_ir_off();
		led_wifi_off();
		led_red_on();
	}
	DBG_PRINT("CHARGE LED RESET...\r\n");
}

//static INT8U ir_power_flag = 0;
extern INT32U g_wifi_notify_pic_done;
void set_led_mode(LED_MODE_ENUM mode)
{
	INT8U i;
	
	g_led_r_state = 0;
	g_led_y_state = 0;
	g_led_g_state = 0;
	g_led_b_state = 0; //避免边充边录时， 因其它灯的消息使用充电灯复位
	g_led_flicker_state = 0;
	LastLedMode = NowLedMode;
	
	switch((INT32U)mode)
	{
		case LED_INIT://初始化
			led_all_off();
			led_ir_off();
			led_wifi_on();
			led_blue_on();
			led_red_off();
			DBG_PRINT("led_type = LED_INIT\r\n");
			break;
		case LED_UPDATE_PROGRAM://开始升级
			g_led_r_state=6;
			DBG_PRINT("led_type = LED_UPDATE_PROGRAM\r\n");
			break;
		case LED_UPDATE_FINISH://升级完成
			g_led_r_state = 1;
			DBG_PRINT("led_type = LED_UPDATE_FINISH\r\n");
			break;
		case LED_UPDATE_FAIL://升级失败
			sys_release_timer_isr(LED_blanking_isr);
			led_all_off();
			for(i=0;i<2;i++)
			{
				OSTimeDly(15);
				led_red_on();
				OSTimeDly(15);
				led_red_off();
			}
			DBG_PRINT("led_type = LED_UPDATE_FAIL\r\n");
			sys_registe_timer_isr(LED_blanking_isr);
			break;
		case LED_CAPTURE_FAIL://拍照失败
		case LED_WAITING_CAPTURE://拍照待机
			if(usb_state_get() == 1)
		   	{
		    	charge_flash_pro();
		   	}
			else
			{
				DBG_PRINT("led_type = LED_WAITING_CAPTURE\r\n");
				if (wifi_key_snd_flag ) break;
				led_all_off();
				led_red_on();
			}
			break;
		case LED_CAPTURE://拍照
			led_all_off();
			DBG_PRINT("led_type = LED_CAPTURE\r\n");
			if (wifi_key_snd_flag ) break;
			led_blue_on();
			break;			
		case LED_WAITING_RECORD://录像待机
			if(usb_state_get() == 1)
		   	{
		    	charge_flash_pro();
		   	}
			else
			{
				DBG_PRINT("led_type = LED_WAITING_RECORD\r\n");
				nOldWifiSta = 0xFF;
				led_all_off();
				led_ir_off();
				if (wifi_key_snd_flag == 0) 
				{
					led_red_on();
				}
			}
			break;
		case LED_RECORD_READY:
			if (ap_sd_status_get() == 1) break; //卡满或容量不足时不闪灯, 防止灯状态被覆盖
			LedFilkerTime = 26; //125ms
			LedFilkerCnt = 3;
			LedFilkerStep = RED_FLIKER;
			DBG_PRINT("led_type = LED_RECORD_READY\r\n");
			break;	
		case LED_RECORD://录像
			DBG_PRINT("led_type = LED_RECORD\r\n");
			if (wifi_key_snd_flag ) break;
			led_all_off();
			g_led_b_state = 3;
			g_led_flicker_state = 0;
			break;
		case LED_RECORD_IR_ENABLE:
			if (led_ir_flag == LED_OFF)
			{
				led_all_off();
				led_ir_on();
				led_wifi_off();
			}
			else
			{
				led_ir_off();
				if (nWifiSta == 0)
				{
					if (ap_video_record_sts_get() & 0x2)
					{
						g_led_b_state = 3;
						g_led_flicker_state = 0;
					}
				}
				else
				{
					nOldWifiSta = 0xFF;
				}
			}
			DBG_PRINT("led_type = LED_RECORD_IR\r\n");
			break;
		case LED_WAITING_AUDIO_RECORD://录音待机		
			led_red_on();
			led_blue_off();
			DBG_PRINT("led_type = LED_WAITING_AUDIO_RECORD\r\n");
			break;
		case LED_AUDIO_RECORD://录音
			led_red_off();
			DBG_PRINT("led_type = LED_AUDIO_RECORD\r\n");
			break;
		case LED_MOTION_READY:	
			DBG_PRINT("led_type = LED_MOTION_READY\r\n");
			LedFilkerTime = 32; //320ms
			LedFilkerCnt = 1;
			LedFilkerStep = BLUE_FLIKER;	
			break;
		case LED_MOTION_WAITING://移动侦测待机		
			led_red_off();
			g_led_flicker_state = 0;
			DBG_PRINT("led_type = LED_MOTION_WAITING\r\n");
			break;
		case LED_MOTION_DETECTION://移动侦测	
			g_led_r_state = 3;
			g_led_flicker_state = 0;
			DBG_PRINT("led_type = LED_MOTION_DETECTION\r\n");
			break;
		case LED_CARD_DETE_SUC://检测到卡
			sd_flag = 2;
			if(storage_sd_upgrade_file_flag_get() == 2) break;
			if(usb_state_get() == 1)
		   	{
		    	charge_flash_pro();
		   	}
			else
			{
				LedFilkerTime = 26; //320ms
				LedFilkerCnt = 1;
				LedFilkerStep = RED_FLIKER|0x1; //red
				DBG_PRINT("led_type = LED_CARD_DETE_SUC\r\n");
			}
			break;
		case LED_USB_CONNECT://连接USB	
			if(LastLedMode != mode)
			{
				g_led_count=0;
			}
			led_all_off();
			led_red_on();

			DBG_PRINT("led_type = LED_USB_CONNECT\r\n");
			break;
		case LED_NO_SDC://无卡
			sd_flag = 0;
			if (ap_peripheral_low_vol_get()) break; 
			if(usb_state_get() == 1)
		   	{
		    	charge_flash_pro();
		   	}
			else
			{
				DBG_PRINT("led_type = LED_NO_SDC\r\n");
				if (wifi_key_snd_flag || (usb_state_get()>=2))
				{
					led_all_off();
					led_ir_off();
					led_wifi_off();
					g_led_r_state = 5;	
				}
				else
				{
					LedFilkerTime = 16; //125ms
					LedFilkerCnt = 5;
					LedFilkerStep = RED_FLIKER;	
				}
			}
			break;
		case LED_SDC_FULL://卡满
		case LED_CARD_NO_SPACE://卡内剩余空间不够
			if(storage_sd_upgrade_file_flag_get() == 2) break;
			sd_flag = 1;
			if((usb_state_get() == 1)||(usb_state_get() == 3))
		   	{
		    	charge_flash_pro();
		   	}
			else
			{
				if (wifi_key_snd_flag || (usb_state_get()>=2))
				{
					
					led_all_off();
					led_ir_off();
					led_wifi_off();
					g_led_r_state = 5;	
				}
				else
				{
					LedFilkerTime = 16; //125ms
					LedFilkerCnt = 5;
					LedFilkerStep = RED_FLIKER|0x1;	
				}
				DBG_PRINT("%s, led_type = LED_SDC_FULL\r\n", __func__);
			}
			break;      
		case LED_CHARGE_FULL://电已充满
			led_all_off();
			led_ir_off();
			led_wifi_off();
			led_red_on();
			DBG_PRINT("led_type = LED_CHARGE_FULL\r\n");
			break;
		case LED_CHARGEING://正在充电
			if((LastLedMode == LED_CHARGE_FULL)||(LastLedMode ==LED_CHARGEING))
		   	{
		   		if(usb_state_get() == 1)
		   	    {
		        	charge_flash_pro();
		   	    }
		   		break;
		   	}
			led_all_off();
			led_ir_off();
			led_wifi_off();
			g_led_r_state=2;
			g_led_flicker_state = 0;
			DBG_PRINT("led_type = LED_CHARGEING\r\n");
			break;
		case LED_STATUS_INDICATORS: //状态提示
			DBG_PRINT("led_type = LED_STATUS_INDICATORS\r\n");
			break;
		case LED_WIFI_ENABLE:
			if (usb_state_get() && (sd_flag == 0))
			{
				led_all_off();
				led_ir_off();
				led_wifi_off();
				nWifiSta = 0;
				g_led_r_state = 5;	
			}
			else
			{
				led_wifi_on();
				led_red_off();
				nWifiSta = 1; 
			}
			DBG_PRINT("led_type = LED_WIFI_ENABLE\r\n");
			break;	
		case LED_WIFI_DISABLE:
			if (usb_state_get() && (sd_flag == 0))
			{
				led_all_off();
				led_ir_off();
				led_wifi_off();
				g_led_r_state = 5;	
			}
			else
			{
				led_wifi_off();
				led_red_on();
			}
			nWifiSta = 0;

			DBG_PRINT("led_type = LED_WIFI_DISABLE\r\n");
			break;	
		case LED_IR_STATUS:
			LedFilkerTime = 26;//200ms
			LedFilkerCnt = 1;
			LedFilkerStep = WIFI_FLIKER;
			DBG_PRINT("led_type = LED_IR_STATUS\r\n");
			break;
		case LED_WIFI_ON_SUCC:
			g_led_g_state = 1;
			DBG_PRINT("%s,led_type = LED_WIFI_ON_SUCC\r\n", __func__);
			break;
	}
	NowLedMode = mode;
}

void led_red_on(void)
{
  if(led_red_flag != LED_ON)
    {
	  gpio_write_io(LED2, LED2_ACTIVE^0);
	  led_red_flag=LED_ON;
    }
}

void led_ir_on(void)
{
    if(led_ir_flag != LED_ON)
    {
	  #if IR_PIN_TYPE
	  sys_ldo28_ctrl(1, LDO_28_3v0);
	  #else   
	  gpio_write_io(IR_LED1, IR_LED_ACTIVE^0);
	  #endif
	  led_ir_flag=LED_ON;
    }
    led_yellow_on();
}

void led_blue_on(void)
{
    if(led_blue_flag != LED_ON)
    {
	  gpio_write_io(LED1, LED1_ACTIVE^0);
	  led_blue_flag=LED_ON;
    }
}

void led_yellow_on(void)
{
    if(led_yellow_flag != LED_ON)
    {
	  gpio_write_io(LED3, LED3_ACTIVE^0);
	  led_yellow_flag=LED_ON;
    }
}

void led_wifi_on(void)
{
    if(led_wifi_flag != LED_ON)
    {
	  gpio_write_io(WIFI_LED, WIFI_LED_ACTIVE^0);
	  led_wifi_flag=LED_ON;
    }
}
void led_all_off(void)
{   

	if(led_blue_flag != LED_OFF)
    {
	   gpio_write_io(LED1, LED1_ACTIVE^1);
	   led_blue_flag=LED_OFF;
    }
    
    if(led_red_flag != LED_OFF)
    {
	   gpio_write_io(LED2, LED2_ACTIVE^1);
	   led_red_flag=LED_OFF;
    }
}

void led_ir_off(void)
{
	if(led_ir_flag != LED_OFF)
	{
	  #if IR_PIN_TYPE
	  sys_ldo28_ctrl(0, LDO_28_3v0);
	  #else
	  gpio_write_io(IR_LED1,IR_LED_ACTIVE^1);
	  #endif
	  led_ir_flag=LED_OFF;
	}
	led_yellow_off();
}

void led_blue_off(void)
{
	if(led_blue_flag != LED_OFF)
	{
	  gpio_write_io(LED1,LED1_ACTIVE^1);
	  led_blue_flag=LED_OFF;
	}
}

void led_yellow_off(void)
{
	if(led_yellow_flag != LED_OFF)
	{
	  gpio_write_io(LED3,LED3_ACTIVE^1);
	  led_yellow_flag=LED_OFF;
	}
}

void led_red_off(void)
{
	if(led_red_flag != LED_OFF)
	{
	 gpio_write_io(LED2, LED2_ACTIVE^1);
	 led_red_flag=LED_OFF;
	}
}

void led_wifi_off(void)
{
	if(led_wifi_flag != LED_OFF)
	{
	 gpio_write_io(WIFI_LED, WIFI_LED_ACTIVE^1);
	 led_wifi_flag=LED_OFF;
	}
}

INT8U run_led_fliker(void)
{
	static INT8U LedCnt = 0;
    static INT8U LedTime = 0;
    INT32U type;

	if(LedFilkerStep)
	{	
		//DBG_PRINT("....run_led_fliker....\r\n");
		switch(LedFilkerStep)
		{
			case 0x10:
				led_red_off();
				LedFilkerStep = 0x12;
				break;
			case 0x11:
				led_red_on();
				LedFilkerStep = 0x13;
				break;
			case 0x20:
				led_blue_off();
				LedFilkerStep = 0x22;
				break;
			case 0x21:
				led_blue_on();
				LedFilkerStep = 0x23;
				break;
				/*
			case 0x30:
				led_yellow_off();
				LedFilkerStep = 0x32;
				break;
			case 0x31:
				led_yellow_on();
				LedFilkerStep = 0x33;
				break;
			*/
			case 0x40:
				led_wifi_off();
				LedFilkerStep = 0x42;
				break;
			case 0x41:
				led_wifi_on();
				LedFilkerStep = 0x43;
				break;
			case 0x12:
			case 0x22:
			//case 0x32:
			case 0x42:
			case 0x13:
			case 0x23:
			//case 0x33:
			case 0x43:
				if (++LedTime > LedFilkerTime) //200ms
				{
					LedTime = 0;
					switch(LedFilkerStep)
					{
						case 0x12:
							LedFilkerStep = 0x11;
							break;
						case 0x13:
							LedFilkerStep = 0x10;
							break;
						case 0x22:
							LedFilkerStep = 0x21;
							break;
						case 0x23:
							LedFilkerStep = 0x20;
							break;
							/*
						case 0x32:
							LedFilkerStep = 0x31;
							break;
						case 0x33:
							LedFilkerStep = 0x30;
							break;
							*/
						case 0x42:
							LedFilkerStep = 0x41;
							break;
						case 0x43:
							LedFilkerStep = 0x40;
							break;
					}
					if (++LedCnt > (LedFilkerCnt<<1))
					{
						LedCnt = 0;
						LedFilkerStep = 0xFAA;
					}
				}
				break;
			case 0xFAA:	
				switch(NowLedMode)
				{
					case LED_NO_SDC:
						//break;
					case LED_SDC_FULL:
						//break;
					case LED_CARD_NO_SPACE:
						if ((wifi_key_snd_flag==0)&&(usb_state_get()==0)) //防止卡有问题时, 录像会关机
						{
							DBG_PRINT("SDC ERROR POWER OFF...\r\n");
							type = 0;
							msgQSend(ApQ, MSG_APQ_POWER_KEY_ACTIVE, &type, sizeof(INT32U), MSG_PRI_NORMAL);
						}
						break;
					case LED_CARD_DETE_SUC:
						led_red_on();
						//led_blue_off();
						break;	
					case LED_POWER_OFF:
						type = 1;
						msgQSend(ApQ, MSG_APQ_POWER_KEY_ACTIVE, &type, sizeof(INT32U), MSG_PRI_NORMAL);
						break;
					case LED_IR_STATUS:
						//type = LED_WAITING_RECORD;
			    		//msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &type, sizeof(INT32U), MSG_PRI_NORMAL);
						break;
					case LED_RECORD_READY:
						type = LED_RECORD;
			    		msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &type, sizeof(INT32U), MSG_PRI_NORMAL);
						break;
					case LED_WIFI_ENABLE:	
					case LED_WIFI_DISABLE:
					/*
						if ((usb_state_get()==2)&&(sd_flag!=2))
						{
							led_all_off();
							g_led_r_state = 5;	
						}
						else
						{
							type = LED_WAITING_RECORD;
				    		msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &type, sizeof(INT32U), MSG_PRI_NORMAL);	
						}
						*/
						break;
					case LED_MOTION_READY:
						//led_blue_off();
						//type = LED_MOTION_WAITING;
			    		//msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &type, sizeof(INT32U), MSG_PRI_NORMAL);
						break;
					default:
						//type = LastLedMode;
			    		//msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &type, sizeof(INT32U), MSG_PRI_NORMAL);
						break;		
				}
				LedFilkerStep = 0;
				break;	
			default:
				break;	
		}
	}
	if (LedFilkerStep) return 1;
	else return 0;
}

INT32U video_busy_cnt_get(void)
{
	return video_busy_cnt;
}
void video_busy_cnt_clr(void)
{
	video_busy_cnt = 0;
}

void LED_blanking_isr(void)
{	
   INT32U type=NULL;
   static INT8U red_flash_flag=0;
   static INT8U flash_flag=0;
   static INT8U Cnt = 0;
   static INT8U nWifiLedEnable = 0;

#if C_IR_REMOTE
	if (!s_usbd_pin) F_128Hz_IR_Service(); //红外信号检测
#endif

	if (++video_busy_cnt >= 1000) video_busy_cnt = 1000;
	if(++g_led_count >= 128){
		g_led_count = 0;
		if ((led_ir_flag == LED_OFF)&&(sd_flag != 0))
		{
			if(usb_state_get() == 1) nWifiSta = 0;
			if (nOldWifiSta != nWifiSta)
			{
				switch(nWifiSta)
				{
					case 0:
						led_wifi_off();
						//DBG_PRINT("wifi disable...\r\n");
						break;
					case 1:
						led_wifi_on();
						//DBG_PRINT("wifi enable...\r\n");
						break;
					case 2:
						if (nWifiLedEnable) 
						{
							led_wifi_on();
							nWifiLedEnable = 0;
						}
						else
						{
							led_wifi_off();
							nWifiLedEnable = 1;
						}
						//DBG_PRINT("wifi connect...\r\n");
						break;
					default:
						led_wifi_off();
						break;	
				}
				nOldWifiSta == nWifiSta;
			}
		}
	}
	if ((!wifi_key_snd_flag)&&(sd_flag!=2)&&(g_led_r_state == 5))
	{//WIFI已关闭的情况下, 无卡或卡满都要关机
		if (usb_state_get()==0)
		{	
			type = 0;
			msgQSend(ApQ, MSG_APQ_POWER_KEY_ACTIVE, &type, sizeof(INT32U), MSG_PRI_NORMAL);
			return;
		}
	}
	
	//+++++++++++++++++++++++++++++++++++++++
	if (run_led_fliker()) return; 
	//++++++++++++++++++++++++++++++++++++++++	
	switch(g_led_r_state)
	{
		case 1:
			led_red_on();
			break;
		case 2:
			if(g_led_count == 0)
			{
				if (++Cnt >= 2)
				{
					Cnt = 0;
					if (red_flash_flag)
					{
						red_flash_flag = 0;
						led_red_on();
					}
					else
					{
						led_red_off();
						red_flash_flag = 1;
					}
				}
			}
			break;
		case 3:
			if(g_led_count < 64)
			led_red_on();
			else
			led_red_off();
 			break;
		case 4:
			if (g_led_count%32 == 0)
			{
				if (red_flash_flag)
				{
					red_flash_flag = 0;
					led_red_on();
				}
				else
				{
					red_flash_flag = 1;
					led_red_off();
				}
			}
 			break;
		case 5:
			if (g_led_count%16 == 0)
			{
				if (red_flash_flag)
				{
					red_flash_flag = 0;
					led_red_on();
				}
				else
				{
					red_flash_flag = 1;
					led_red_off();
				}
			}
 			break;
		case 6:
			if (g_led_count%8 == 0)
			{
				if (red_flash_flag)
				{
					red_flash_flag = 0;
					led_red_on();
				}
				else
				{
					red_flash_flag = 1;
					led_red_off();
				}
			}
			break;
	}

	switch(g_led_b_state)
	{
		case 1:
			led_blue_on();
			break;
		case 2:
			if(g_led_count == 0)
			{
				if (flash_flag)
				{
					if (g_led_flicker_state == 0) 
						led_blue_on();
					else
						led_blue_off();
					flash_flag = 0;
				}
				else
				{
					if (g_led_flicker_state == 0)
						led_blue_off();
					else
						led_blue_on();
					flash_flag = 1;
				}
			}
			break;
		case 3:
			if(g_led_count/64 == g_led_flicker_state)
				led_blue_on();
			else
				led_blue_off();
			break;
		case 4:
			if (g_led_count%32 == 0)
			{
				if (flash_flag)
				{
					flash_flag = 0;
					if (g_led_flicker_state == 0)
						led_blue_on();
					else
						led_blue_off();
				}
				else
				{
					flash_flag = 1;
					if (g_led_flicker_state == 0)
						led_blue_off();
					else
						led_blue_on();
				}
			}
			break;
		case 5:
			if (g_led_count%16 == 0)
			{
				if (flash_flag)
				{
					flash_flag = 0;
					if (g_led_flicker_state == 0)
						led_blue_on();
					else
						led_blue_off();
				}
				else
				{
					flash_flag = 1;
					if (g_led_flicker_state == 0)
						led_blue_off();
					else
						led_blue_on();
				}
			}
			break;
		case 6:
			if (g_led_count%8 == 0)
			{
				if (flash_flag)
				{
					flash_flag = 0;
					if (g_led_flicker_state == 0)
						led_blue_on();
					else
						led_blue_off();
				}
				else
				{
					flash_flag = 1;
					if (g_led_flicker_state == 0)
						led_blue_off();
					else
						led_blue_on();
				}
			}
			break;
	}

	if(1 == g_led_g_state)
	{
		if(g_led_count % 64 ==0)
 		{
 		 	if(flash_flag == 0)
 		 	{
			 	led_wifi_on();
				flash_flag=1;
 		 	}
		 	else
		 	{
			 	led_wifi_off();
				flash_flag=0;
		 	}
		 	
 		}
	}

}

//=================================================================
/*
void ap_peripheral_led_set(INT8U type)
{
#ifdef PWM_CTR_LED
	INT8U byPole;
	INT16U wPeriod=0;
	INT16U wPreload=0;
	INT8U byEnable;
	if(type){		//high
		ap_peripheral_PWM_LED_high();
	}else{			//low
		ap_peripheral_PWM_LED_low();
	}

#else
	if (LED1_ACTIVE)
		gpio_write_io(LED1,type);
	else
		gpio_write_io(LED1,type^1);
	led_status = 0;
	led_cnt = 0;
#endif
}

void ap_peripheral_led_flash_set(void)
{
#ifdef PWM_CTR_LED
	ap_peripheral_PWM_LED_high();
	led_status = LED_STATUS_FLASH;
	led_cnt = 0;

#else
	gpio_write_io(LED1, LED1_ACTIVE^0);
	led_status = LED_STATUS_FLASH;
	led_cnt = 0;
#endif
}

void ap_peripheral_led_blink_set(void)
{

#ifdef PWM_CTR_LED
    INT8U byPole = 1; 
    INT16U wPeriod = 0x6000 ; 
    INT16U wPreload = 0x2fff ; 
	INT8U byEnable = TRUE;
    ext_rtc_pwm0_enable(byPole, wPeriod, wPreload,byEnable ) ; 
//    ext_rtc_pwm1_enable(byPole, wPeriod, wPreload, byEnable) ; 
    DBG_PRINT("PWM0/1 blink on  750ms,380ms \r\n"); 

#else
	gpio_write_io(LED1, LED1_ACTIVE^0);
	led_status = LED_STATUS_BLINK;
	led_cnt = 0;
#endif
}

void LED_service(void)
{
	if(led_cnt)
	{
		led_cnt--;
	}

	switch(led_status & 0x000f)
	{
	case LED_STATUS_FLASH:	//1
		if(led_cnt == 0)
		{
			led_status ^= 0x0010;
			if(led_status & 0x0010)
			{
				#ifdef PWM_CTR_LED
			   	ap_peripheral_PWM_LED_high();
				#else 
				gpio_write_io(LED1, LED1_ACTIVE^1);
				#endif
				led_cnt = PERI_TIME_LED_FLASH_ON;
			} else {
				#ifdef PWM_CTR_LED
				ap_peripheral_PWM_LED_low();
				#else 
				gpio_write_io(LED1, LED1_ACTIVE^0);
				#endif
				led_status = 0;
			}
		}
		break;
	
	case LED_STATUS_BLINK:	//2
		if(led_cnt == 0)
		{
			led_status ^= 0x0010;
			if(led_status & 0x0010)
			{
				gpio_write_io(LED1, LED1_ACTIVE^1);
				if (tf_state_flag)
					led_cnt = PERI_TIME_LED_BLINK_ON;
				else
					led_cnt = 8; //   32/PERI_TIME_INTERVAL_AD_DETECT = 250ms	
			} else {
				gpio_write_io(LED1, LED1_ACTIVE^0);
				if (tf_state_flag)
					led_cnt = PERI_TIME_LED_BLINK_OFF;
				else
					led_cnt = 8; //   32/PERI_TIME_INTERVAL_AD_DETECT = 250ms	
			}
		}
		break;
		
	default:
		break;
	}
}
*/

void ap_peripheral_charge_det(void)
{
	INT16U pin_state=0;
	static INT16U prev_pin_state=0;
	INT16U led_type;
	static INT8U loop_cnt=0;
	static INT16U prev_ledtype=0;
	
	pin_state=gpio_read_io(CHARGE_DETECTION_PIN);
	//DBG_PRINT("pin_state=%d",pin_state);

	if(pin_state == prev_pin_state)
		loop_cnt++;
	else
		loop_cnt=0;

	if(loop_cnt >=3)
	{
		loop_cnt=3;
		if(pin_state)
			led_type=LED_CHARGE_FULL;
		else
			led_type=LED_CHARGEING;  
		if(prev_ledtype != led_type)
		{
			prev_ledtype=led_type;
			{
				if(storage_sd_upgrade_file_flag_get() != 2)
				msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
			}
		}
	}

	prev_pin_state=pin_state;
}

#if C_MOTION_DETECTION == CUSTOM_ON

void ap_peripheral_motion_detect_judge(void)
{
	INT32U result;
	
	result = hwCdsp_MD_get_result();
	DBG_PRINT("MD_result = 0x%x\r\n",result);
	if(result>0x40){
		msgQSend(ApQ, MSG_APQ_MOTION_DETECT_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
	}
}

void ap_peripheral_motion_detect_start(void)
{
	motion_detect_status_set(MOTION_DETECT_STATUS_START);
}

void ap_peripheral_motion_detect_stop(void)
{
	motion_detect_status_set(MOTION_DETECT_STATUS_STOP);
}
#endif

#if USE_ADKEY_NO
void ap_peripheral_ad_detect_init(INT8U adc_channel, void (*ad_detect_isr)(INT16U data))
{

   	battery_value_sum=0;
	bat_ck_cnt=0;
//	ad_value_cnt = 0;
	
	adc_init();
	adc_vref_enable_set(TRUE);
	adc_conv_time_sel(4);
	adc_manual_ch_set(adc_channel);
	adc_manual_callback_set(ad_detect_isr);
	if (ad_detect_timerid == 0xFF) {
		ad_detect_timerid = AD_DETECT_TIMER_ID;
		sys_set_timer((void*)msgQSend, (void*) PeripheralTaskQ, MSG_PERIPHERAL_TASK_AD_DETECT_CHECK, ad_detect_timerid, PERI_TIME_INTERVAL_AD_DETECT);
	}
}

void ap_peripheral_ad_check_isr(INT16U value)
{
	ad_value = value;
}

INT16U adc_key_release_calibration(INT16U value)
{
	return value;
}

void ap_peripheral_clr_screen_saver_timer(void)
{
	key_active_cnt = 0;
}

#if  (KEY_TYPE == KEY_TYPE1)||(KEY_TYPE==KEY_TYPE2)||(KEY_TYPE==KEY_TYPE3)||(KEY_TYPE==KEY_TYPE4)||(KEY_TYPE==KEY_TYPE5)

/*
       1/10 だ溃
	4.1v =>  2417  //2460
	4.05v => 2383
	4.0v =>  2334
	3.95v => 2300 
	3.9v =>  2272  //2364
	3.85v => 2240 
	3.8v =>  2207  //2319
	3.75v => 2174
	3.7v =>  2144
	3.65v => 2108 
	3.6v =>  2076  //2200
	3.55v => 2048 
	3.5v => 
	3.4v => 
	场絬隔琌 1/2 だ溃
*/

enum {
	BATTERY_CNT = 8,
	/*
	BATTERY_Lv4 = 2460*BATTERY_CNT,
	BATTERY_Lv3 = 2364*BATTERY_CNT,
	BATTERY_Lv2 = 2300*BATTERY_CNT,
	BATTERY_Lv1 = 2200*BATTERY_CNT
	*/
	BATTERY_Lv4 = 2334*BATTERY_CNT,
	BATTERY_Lv3 = 2240*BATTERY_CNT,
	BATTERY_Lv2 = 2144*BATTERY_CNT,
	BATTERY_Lv1 = 2048*BATTERY_CNT
};

enum {
	DEBUNCE_TIME = 3,
	LONG_KEY_TIME = 35
};

enum {
   C_KEY_NULL = 0,
   C_KEY_KEY1 = 1,
   C_KEY_KEY2 = 2,
   C_KEY_KEY3 = 3,
   C_KEY_KEY4 = 4,
   C_KEY_KEY5 = 5,
   C_KEY_KEY6 = 6,
   C_KEY_HOLD = 0x80
};

/*
static short Median_Filter(short adc_value)
{
   static short D1 = 0;
   static short D2 = 0;
   short max, mid, min;

   if (D2>D1) {
      max = D2;
      min = D1;
   }
   else {
      max = D1;
      min = D2;
   }

   if (adc_value>max) {
      mid = max;
   }
   else {
      mid = adc_value;
   }

   if (mid < min) {
      mid = min;
   }
   
   D2 = D1;
   D1 = adc_value;
   return mid;
}

#if  (KEY_TYPE == KEY_TYPE1)
static char AD_Key_Select(short adc_value)
{
   char key;

   adc_value = Median_Filter(adc_value);
   //DBG_PRINT("Filter_key = %d \r\n",adc_value);
   
   if (adc_value>2200)  //2200
   {  
      key = C_KEY_NULL;
   }
   else if (adc_value>475)
   {  // 
      key = C_KEY_KEY2;
   }
   else
   {
      key =C_KEY_KEY1 ;
   }

   return key;
}
#elif  (KEY_TYPE == KEY_TYPE2)
static char AD_Key_Select(short adc_value)
{
   char key;

   adc_value = Median_Filter(adc_value);
  // DBG_PRINT("Filter_key = %d \r\n",adc_value);
   if (adc_value>1650)
   {  
      key = C_KEY_KEY6;
   }
   else if (adc_value>1150)
   {  // 
      key = C_KEY_KEY5;
   }
   else if (adc_value>960)
   {  // 
      key = C_KEY_KEY4;
   }
   else if (adc_value>560)
   {  // 
      key = C_KEY_KEY3;
   }
   else if (adc_value>420)
   {  // 
      key = C_KEY_KEY2;
   }
   else if (adc_value>200)
   {  // 
      key = C_KEY_KEY1;
   }
   else
   {
      key = C_KEY_NULL;
   }
   return key;
} 
#elif  (KEY_TYPE == KEY_TYPE4)
static char AD_Key_Select(short adc_value)
{
   char key;

   adc_value = Median_Filter(adc_value);
  // DBG_PRINT("Filter_key = %d \r\n",adc_value);
   if (adc_value>1850)  //2200
   {  
      key = C_KEY_KEY6;
   }
   else if (adc_value>1600)//1900
   {  // 
      key = C_KEY_KEY5;
   }
   else if (adc_value>1100)//1400
   {  // 
      key = C_KEY_KEY4;
   }
   else if (adc_value>900)//1000
   {  // 
      key = C_KEY_KEY3;
   }
   else if (adc_value>420)//500
   {  // 
      key = C_KEY_KEY2;
   }
   else if (adc_value>200)
   {  // 
      key = C_KEY_KEY1;
   }
   else
   {
      key = C_KEY_NULL;
   }

   return key;
} 
#elif  (KEY_TYPE == KEY_TYPE5)
static char AD_Key_Select(short adc_value)
{
   char key;

   adc_value = Median_Filter(adc_value);
  // DBG_PRINT("Filter_key = %d \r\n",adc_value);
   if (adc_value>1850)  //2200
   {  
      key = C_KEY_KEY6;
   }
   else if (adc_value>1600)//1900
   {  // 
      key = C_KEY_KEY5;
   }
   else if (adc_value>1100)//1400
   {  // 
      key = C_KEY_KEY4;
   }
   else if (adc_value>900)//1000
   {  // 
      key = C_KEY_KEY3;
   }
   else if (adc_value>420)//500
   {  // 
      key = C_KEY_KEY2;
   }
   else if (adc_value>200)
   {  // 
      key = C_KEY_KEY1;
   }
   else
   {
      key = C_KEY_NULL;
   }

   return key;
} 
#else
static char AD_Key_Select(short adc_value)
{
   char key;

   adc_value = Median_Filter(adc_value);
  // DBG_PRINT("Filter_key = %d \r\n",adc_value);
   if (adc_value>0x893)
   {  
      key = C_KEY_KEY5;
   }
   else if (adc_value>0x655)
   {  // 
      key = C_KEY_KEY4;
   }
   else if (adc_value>0x371)
   {  // 
      key = C_KEY_KEY3;
   }
   else if (adc_value>0x1A5)
   {  // 
      key = C_KEY_KEY2;
   }
   else if (adc_value>0xD2)
   {  // 
      key = C_KEY_KEY1;
   }
   else
   {  // 
      key = C_KEY_NULL;
   }

   return key;
} 
#endif

static char AD_Key_Get(short adc_value)
{
   static char gc_KeyValue = C_KEY_NULL;
   static char gc_Key_Pressed = C_KEY_NULL;
   static char gc_Key_PrevPressed = C_KEY_NULL;
   static char gc_KeyFinalValue = C_KEY_NULL;
   static char gb_KeyStart = 0;
   static short gw_KeyTimeDebunce = DEBUNCE_TIME;
   static short gw_LongKeyCount = 0;
   static short gw_LongKeyCnt = LONG_KEY_TIME;
   char ret = C_KEY_NULL;

////////////////////////////////////////////////////////////
//          Key Selection Algorithm (ML decision)
///////////////////////////////////////////////////////////

   gc_Key_Pressed = AD_Key_Select(adc_value);

////////////////////////////////////////////////////////////
//          Key Detection Algorithm
///////////////////////////////////////////////////////////

// key estimation
   if((gc_KeyFinalValue < gc_Key_Pressed) && (gc_Key_Pressed != C_KEY_NULL)) 
   {
      gc_KeyFinalValue = gc_Key_Pressed;
   }

// debunce or discard current key
   if(gb_KeyStart && (gc_Key_PrevPressed != C_KEY_NULL))
   {
      if(gw_KeyTimeDebunce)
         gw_KeyTimeDebunce--;	
      if(gw_KeyTimeDebunce && (gc_Key_PrevPressed != gc_Key_Pressed))
      {		//  Debunce 筁祘いΩぃ单硂Ωち常フ禣
         gc_Key_PrevPressed = C_KEY_NULL;
         gb_KeyStart = 0;
      }
   }									

///////// key detect start  /////////
   if ((gc_Key_PrevPressed == C_KEY_NULL)&&(gc_Key_Pressed != C_KEY_NULL))		// 穝龄
   {
   	// 1. Ч⊿Τ龄龄
    // 2. Debunce 筁祘い斌玡龄穝璸计硂龄
      gc_Key_PrevPressed=gc_Key_Pressed;
      gw_KeyTimeDebunce = DEBUNCE_TIME;
      gc_KeyFinalValue = C_KEY_NULL;
      gb_KeyStart = 1;	
      gw_LongKeyCount = 0;
   }

 ///////// key detect end  /////////
   if (gw_KeyTimeDebunce == 0)
   {
      if(gc_Key_PrevPressed  != gc_KeyFinalValue)
      {	// キ铆琿籔箇代ぃ,メ奔Key
         gc_Key_PrevPressed = gc_Key_Pressed;
         gw_KeyTimeDebunce = DEBUNCE_TIME;
         gc_KeyFinalValue = C_KEY_NULL;
         gw_LongKeyCount = 0;					
      }
		else if ((gc_Key_Pressed==C_KEY_NULL)&&(gc_Key_PrevPressed!=C_KEY_NULL) )
		{
		gc_Key_PrevPressed = gc_Key_Pressed;
		gw_KeyTimeDebunce = DEBUNCE_TIME;
		gc_KeyFinalValue = C_KEY_NULL;
		gw_LongKeyCount = 0; 
		}
		else if ( (gc_Key_Pressed == gc_Key_PrevPressed)&&(gw_LongKeyCount<gw_LongKeyCnt) ) // 平?段且長按
      {
         gw_LongKeyCount++;
         gc_KeyValue = gc_KeyFinalValue;
         if(gw_LongKeyCount >= gw_LongKeyCnt)   // Detect Long Key
         {
		ad_key_map[gc_KeyValue].key_cnt = 100;	// 竒磅︽筁
		ad_key_map[gc_KeyValue].key_function(&(ad_key_map[gc_KeyValue].key_cnt)); // ㊣龄				            
         
		gc_KeyValue = gc_KeyFinalValue|C_KEY_HOLD;	// Long Key pressed
         }	
      }
     //If user hold long key contiueously, it will do nothing.}
   }

// release key
   if ((gc_KeyValue!=C_KEY_NULL)&&(gc_Key_Pressed==C_KEY_NULL))
   {
      ret = gc_KeyValue;
      gw_LongKeyCount = 0;
      gw_KeyTimeDebunce = DEBUNCE_TIME;
      gc_KeyValue = C_KEY_NULL;
   }

   return ret;
}
*/
#define BATTERY_PEROID 0xF
static unsigned int battery_period_cnt = 0;
void ap_peripheral_ad_key_judge(void)
{
	//char key;

	if ((battery_period_cnt&BATTERY_PEROID)==0)
	{
		unsigned int diff;
		 //DBG_PRINT("bat=0x%x\r\n",ad_value>>4);
		// Battery Detect
		adc_battery_value_old = adc_battery_value_new;
		adc_battery_value_new = ad_value>>4;

		if (adc_battery_value_new >= adc_battery_value_old) {
			diff = adc_battery_value_new - adc_battery_value_old;
		} else {
			diff = adc_battery_value_old - adc_battery_value_new;
		}
		if (!diff || (100*diff <= C_RESISTOR_ACCURACY*adc_battery_value_old)) {
			if (battery_stable_cnt < C_BATTERY_STABLE_THRESHOLD) battery_stable_cnt++;
		} else {
			battery_stable_cnt = 1;

			bat_ck_cnt = 0;
			battery_value_sum = 0;
		}

		if (battery_stable_cnt >= C_BATTERY_STABLE_THRESHOLD) {

		  #if C_BATTERY_DETECT == CUSTOM_ON
        	ap_peripheral_battery_check_calculate();
		  #endif

		}
	}
	else
	{	
	/*
		// AD-Key  Detect
		//DBG_PRINT("adc=0x%x\r\n",ad_value>>4);
		key = AD_Key_Get(ad_value>>4);
		if (key != C_KEY_NULL)
		{

			#if C_SCREEN_SAVER == CUSTOM_ON
#if TV_DET_ENABLE
			if (!tv_plug_in_flag)
#endif
			{
				key_active_cnt = 0;
				ap_peripheral_lcd_backlight_set(BL_ON);
			}
			#endif

   			//DBG_PRINT("key = 0x%x\r\n",(unsigned char)key);
			switch(key)
			{
				case 1:
					DBG_PRINT("PREV\r\n");					
					break;
				case 2:
					DBG_PRINT("NEXT\r\n");					
					break;
				case 3:
					DBG_PRINT("FUNC\r\n");
					break;
				case 4:
					DBG_PRINT("MENU\r\n");
					break;
				case 5:
					DBG_PRINT("OK\r\n");					
					break;
				default:
					DBG_PRINT("Wrong Key\r\n");
			}
   			
			if (key&C_KEY_HOLD)
			{
				//key &= (~C_KEY_HOLD);
				//ad_key_map[key].key_cnt = 100;	// 竒磅︽筁
				//ad_key_map[key].key_function(&(ad_key_map[key].key_cnt)); // ㊣龄				
				__asm {NOP};
			}	
			else
			{
				ad_key_map[key].key_cnt = 3;		// 祏
				ad_key_map[key].key_function(&(ad_key_map[key].key_cnt)); // ㊣龄
			}
		}
			*/	
	}

	//DBG_PRINT("cnt=0x%x\r\n",battery_period_cnt);
	battery_period_cnt++;
	// next SAE ADC manual mode
	if ((battery_period_cnt&BATTERY_PEROID)==0) {
		adc_manual_ch_set(AD_BAT_DETECT_PIN);
	}
	else {
		adc_manual_ch_set(AD_KEY_DETECT_PIN);
	}
	adc_manual_sample_start();
}

#else

/*
	0.41v => 495
	0.39v =>
	0.38v => 460
	0.37v => 
	0.36v => 440
	0.35v =>
	0.34v =>

enum {
	BATTERY_CNT = 8,	
	BATTERY_Lv4 = 495*BATTERY_CNT,		
	BATTERY_Lv3 = 460*BATTERY_CNT,
	BATTERY_Lv2 = 450*BATTERY_CNT,
	BATTERY_Lv1 = 440*BATTERY_CNT
};
*/

#if GPDV_BOARD_VERSION == GPCV1248_MINI 
static INT32U adc_key_factor_table[USE_ADKEY_NO] = {	// x1000
	// 6 AD-keys
	//680K, 300K, 150K, 68K, 39K, 22K
	9225, 23935, 41258, 51096, 73032, 87612
};
#else
static INT32U adc_key_factor_table[USE_ADKEY_NO] = {	// x1000
	// 6 AD-keys
	//680K, 300K, 150K, 68K, 39K, 22K
	1969, 2933, 4182, 5924, 7104, 8102
};
#endif
static INT32U ad_time_stamp;

INT32U adc_key_judge(INT32U adc_value)
{
	INT32U candidate_key;
	INT32U candidate_diff;
	INT32U i, temp1, temp2, temp3, diff;

	candidate_key = USE_ADKEY_NO;
	candidate_diff = 0xFFFFFFFF;
	temp1 = 1000 * adc_value;   // to avoid "decimal point"

	temp2 = adc_key_release_calibration(adc_key_release_value_stable); // adc_battery_value_stable = stable adc value got when no key press
	                                                           // temp2: adc theoretical value 
	for (i=0; i<USE_ADKEY_NO; i++) {
		temp3 = temp2 * adc_key_factor_table[i];              // temp3: the calculated delimiter   
		if (temp1 >= temp3) {
			diff = temp1 - temp3;
		} else {
			diff = temp3 - temp1;
		}
		// DBG_PRINT("adc:[%d], bat:[%d], diff:[%d]\r\n", temp1, temp3, diff);

		if (diff > candidate_diff) {
			ASM(NOP);
			ASM(NOP);
			ASM(NOP);
			ASM(NOP);
			ASM(NOP);
			ASM(NOP);
			
			break;
		}

		candidate_key = i;
		candidate_diff = diff;
	}

	if (candidate_key < USE_ADKEY_NO) {
		DBG_PRINT("\r\nKey %d", candidate_key+1);
//		power_off_time_beep_1 = 0;
//		power_off_time_beep_2 = 0;
//		power_off_time_beep_3 = 0;
#if C_SCREEN_SAVER == CUSTOM_ON
		key_active_cnt = 0;
		ap_peripheral_lcd_backlight_set(BL_ON);
#endif
	}
	
	return candidate_key;
}


void ap_peripheral_ad_key_judge(void)
{
	INT32U  t;
	INT32U diff;
	INT16U adc_current_value;

	//DBG_PRINT("ap_peripheral_ad_key_judge()~~~~\r\n");
	//DBG_PRINT(".");

	t = OSTimeGet();
	if ((t - ad_time_stamp) < 2) {
		return;
	}
	ad_time_stamp = t;

	ad_line_select++;
	if (ad_line_select & 0x1F) {
		adc_manual_ch_set(AD_KEY_DETECT_PIN);
	} else {
		adc_manual_ch_set(AD_BAT_DETECT_PIN);
		ad_line_select = 0;
	}
	//adc_manual_sample_start();

	if (ad_line_select == 1) { //battery detection

		adc_battery_value_old = adc_battery_value_new;
		adc_battery_value_new = ad_value >> 4;

		if (adc_battery_value_new >= adc_battery_value_old) {
			diff = adc_battery_value_new - adc_battery_value_old;
		} else {
			diff = adc_battery_value_old - adc_battery_value_new;
		}
		if (!diff || (100*diff <= C_RESISTOR_ACCURACY*adc_battery_value_old)) {
			if(battery_stable_cnt < C_BATTERY_STABLE_THRESHOLD) battery_stable_cnt++;
		} else {
			battery_stable_cnt = 1;

			bat_ck_cnt = 0;
			battery_value_sum = 0;
		}

		if (battery_stable_cnt >= C_BATTERY_STABLE_THRESHOLD) {

			//DBG_PRINT("%d,", adc_battery_value_new);

		  #if C_BATTERY_DETECT == CUSTOM_ON
        	ap_peripheral_battery_check_calculate();
		  #endif

		}
		adc_manual_sample_start();
		return;
	}

#if 0
	adc_current_value = ad_value >> 6;
#else
	// josephhsieh140408
	adc_current_value = (ad_value>>4);
	//DBG_PRINT("%d\r\n",adc_current_value);
#endif

	//DBG_PRINT("ad_value = 0x%x \r\n",ad_value);
	//print_string("ad_value = %d \r\n",ad_value);

	if (adc_current_value < C_KEY_PRESS_WATERSHED) { // Key released
		if (adc_key_value) {  // adc_key_value: last ADC(KEY) value
			// Key released
			if (!fast_key_exec_flag&&!long_key_exec_flag && key_pressed_cnt>=C_KEY_STABLE_THRESHOLD) {
				INT32U pressed_key;

				pressed_key = adc_key_judge(adc_key_value);
				if (pressed_key<USE_ADKEY_NO && ad_key_map[pressed_key].key_function) {
					ad_key_map[pressed_key].key_function(&(ad_key_map[pressed_key].key_cnt));
					//DBG_PRINT("key_function key%d \r\n",pressed_key);
				}
			}

			adc_key_value = 0;
			key_pressed_cnt = 0;
			fast_key_exec_flag = 0;
			normal_key_exec_flag = 0;
			long_key_exec_flag = 0;
		}
		
		adc_key_release_value_old = adc_key_release_value_new;
		adc_key_release_value_new = adc_current_value;

		if (adc_key_release_value_new >= adc_key_release_value_old) {
			diff = adc_key_release_value_new - adc_key_release_value_old;
		} else {
			diff = adc_key_release_value_old - adc_key_release_value_new;
		}
		if (!diff || (100*diff <= C_RESISTOR_ACCURACY*adc_key_release_value_old)) {
			key_release_stable_cnt++;
		} else {
			key_release_stable_cnt = 1;
		}

		if (key_release_stable_cnt >= C_KEY_RELEASE_STABLE_THRESHOLD) {
			adc_key_release_value_stable = (adc_key_release_value_new + adc_key_release_value_old) >> 1;
			
			//DBG_PRINT("%d,", adc_key_release_value_stable);
			key_release_stable_cnt = 1;
		}
		
	} else { // Key pressed
		if (adc_current_value >= adc_key_value) {
			diff = adc_current_value - adc_key_value;
		} else {
			diff = adc_key_value - adc_current_value;
		}

		if (!diff || (100*diff <= C_RESISTOR_ACCURACY*adc_key_value)) {
			key_pressed_cnt++;
			// TBD: Handle zoom function
			//if (zoom_key_flag && ad_key_cnt>2 && (adkey_lvl == PREVIOUS_KEY || adkey_lvl == NEXT_KEY)) {

		  #if C_KEY_FAST_JUDGE_THRESHOLD != 0
			if (key_pressed_cnt == C_KEY_FAST_JUDGE_THRESHOLD) {   // fast key(long pressed),more than 2 sec
				INT32U pressed_key;
				
				pressed_key = adc_key_judge(adc_key_value);
				if (pressed_key<USE_ADKEY_NO && ad_key_map[pressed_key].fast_key_fun) {
					ad_key_map[pressed_key].fast_key_fun(&(ad_key_map[pressed_key].key_cnt));
					fast_key_exec_flag = 1;
				}
			}
		  #endif
//#if KEY_FUNTION_TYPE == SAMPLE2
		  
//			DBG_PRINT("key_pressed_cnt = %d  \r\n",key_pressed_cnt);
		  	if(key_pressed_cnt ==40){  //long key
				INT32U pressed_key;
				pressed_key = adc_key_judge(adc_key_value);
				if (pressed_key<USE_ADKEY_NO && ad_key_map[pressed_key].key_function) {
					ad_key_map[pressed_key].key_cnt = key_pressed_cnt;
					//DBG_PRINT("long key cnt = %d  \r\n",ad_key_map[pressed_key].key_cnt);
					ad_key_map[pressed_key].key_function(&(ad_key_map[pressed_key].key_cnt));
					
					long_key_exec_flag = 1;
					adc_key_value = 0;
					key_pressed_cnt = 0;
					fast_key_exec_flag = 0;
					normal_key_exec_flag = 0;
				}
		  	}
//#endif

		} else {
			// Key released or not stable
			if (adc_key_value && !fast_key_exec_flag && !normal_key_exec_flag &&!long_key_exec_flag&& key_pressed_cnt>=C_KEY_STABLE_THRESHOLD) {
				INT32U pressed_key;
				
				pressed_key = adc_key_judge(adc_key_value);
				if (pressed_key<USE_ADKEY_NO && ad_key_map[pressed_key].key_function) {
					//ad_key_map[pressed_key].key_function(&(ad_key_map[pressed_key].key_cnt));
					normal_key_exec_flag = 1;
					//DBG_PRINT("key_function 2\r\n");
				}
			}

			if(!fast_key_exec_flag && !normal_key_exec_flag) {
				adc_key_value = adc_current_value;
				key_pressed_cnt = 1;
				//fast_key_exec_flag = 0;
			}
		}
	}
	adc_manual_sample_start();
}
#endif // AD-Key
#endif

#if C_BATTERY_DETECT == CUSTOM_ON

INT32U previous_direction = 0;
//extern void ap_state_handling_led_off(void);
extern INT8U display_str_battery_low;


#define BATTERY_GAP 5*BATTERY_CNT
INT8U bat_lvl_cal_bak = (INT8U)BATTERY_Lv4;
static INT8U ap_peripheral_smith_trigger_battery_level(INT32U direction)
{
	INT8U bat_lvl_cal; 

	//DBG_PRINT("(%d)\r\n", battery_value_sum);
	if (battery_value_sum >= BATTERY_Lv4) {
		bat_lvl_cal = 4;
	} else if ((battery_value_sum < BATTERY_Lv4)&&(battery_value_sum >= BATTERY_Lv3)) {
		bat_lvl_cal = 3;
	} else if ((battery_value_sum < BATTERY_Lv3) && (battery_value_sum >= BATTERY_Lv2)) {
		bat_lvl_cal = 2;
	} else if ((battery_value_sum < BATTERY_Lv2) && (battery_value_sum >= BATTERY_Lv1)) {
		bat_lvl_cal = 1;
	} else if (battery_value_sum < BATTERY_Lv1) {
		bat_lvl_cal = 0;
	}


	if  ( (direction==0)&&(bat_lvl_cal>bat_lvl_cal_bak) )
	{
		if (battery_value_sum >= BATTERY_Lv4+BATTERY_GAP) {
			bat_lvl_cal = 4;
		} else if ((battery_value_sum < BATTERY_Lv4+BATTERY_GAP)&&(battery_value_sum >= BATTERY_Lv3+BATTERY_GAP) ) {
			bat_lvl_cal = 3;
		} else if ((battery_value_sum < BATTERY_Lv3+BATTERY_GAP) && (battery_value_sum >= BATTERY_Lv2+BATTERY_GAP)) {
			bat_lvl_cal = 2;
		} else if ((battery_value_sum < BATTERY_Lv2+BATTERY_GAP) && (battery_value_sum >= BATTERY_Lv1+BATTERY_GAP)) {
			bat_lvl_cal = 1;
		} else if (battery_value_sum < BATTERY_Lv1+BATTERY_GAP) {
			bat_lvl_cal = 0;
		}
	}


	if  ( (direction==1)&&(bat_lvl_cal<bat_lvl_cal_bak) )
	{
		if (battery_value_sum >= BATTERY_Lv4-BATTERY_GAP) {
			bat_lvl_cal = 4;
		} else if ((battery_value_sum < BATTERY_Lv4-BATTERY_GAP)&&(battery_value_sum >= BATTERY_Lv3-BATTERY_GAP) ) {
			bat_lvl_cal = 3;
		} else if ((battery_value_sum < BATTERY_Lv3-BATTERY_GAP) && (battery_value_sum >= BATTERY_Lv2-BATTERY_GAP)) {
			bat_lvl_cal = 2;
		} else if ((battery_value_sum < BATTERY_Lv2-BATTERY_GAP) && (battery_value_sum >= BATTERY_Lv1-BATTERY_GAP)) {
			bat_lvl_cal = 1;
		} else if (battery_value_sum < BATTERY_Lv1-BATTERY_GAP) {
			bat_lvl_cal = 0;
		}
	}


	bat_lvl_cal_bak = bat_lvl_cal;
	return bat_lvl_cal;
	
}


void ap_peripheral_battery_check_calculate(void)
{
	INT8U bat_lvl_cal;
	INT32U direction = 0;

	if (adp_status == 0) {
		//unkown state
		return;
	}
	else if (adp_status == 1) {
		//adaptor in state
		direction = 1; //low voltage to high voltage
		if(previous_direction != direction){
			msgQSend(ApQ, MSG_APQ_BATTERY_CHARGED_SHOW, NULL, NULL, MSG_PRI_NORMAL);
		}
		previous_direction = direction;
	}
	else {		
		//adaptor out state
		direction = 0; //high voltage to low voltage
		if(previous_direction != direction){
			msgQSend(ApQ, MSG_APQ_BATTERY_CHARGED_CLEAR, NULL, NULL, MSG_PRI_NORMAL);
		}
		previous_direction = direction;
	}

	battery_value_sum += (ad_value >> 4);
	//DBG_PRINT("\r\n%d, \r\n",(ad_value>>4));
	bat_ck_cnt++;
	if (bat_ck_cnt >= BATTERY_CNT) {

		bat_lvl_cal = ap_peripheral_smith_trigger_battery_level(direction);
		//DBG_PRINT("%d,", bat_lvl_cal);

		if(!battery_low_flag){
			msgQSend(ApQ, MSG_APQ_BATTERY_LVL_SHOW, &bat_lvl_cal, sizeof(INT8U), MSG_PRI_NORMAL);
		}

		if (bat_lvl_cal == 0 && direction == 0) {
			low_voltage_cnt++;
			if (low_voltage_cnt > 5) {
			    low_voltage_cnt = 0;
				//ap_state_handling_led_off();
#if C_BATTERY_LOW_POWER_OFF == CUSTOM_ON
				if(!battery_low_flag){
					battery_low_flag = 1;
					{
						INT8U type;
						Low_Voltage_Flag = 1;
						msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_STOP, NULL, NULL, MSG_PRI_NORMAL);
						type = FALSE;
						msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_FREESIZE_CHECK_SWITCH, &type, sizeof(INT8U), MSG_PRI_URGENT);
						type = BETTERY_LOW_STATUS_KEY;
						msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_KEY_REGISTER, &type, sizeof(INT8U), MSG_PRI_NORMAL);	
						msgQSend(ApQ, MSG_APQ_BATTERY_LOW_SHOW, NULL, sizeof(INT8U), MSG_PRI_NORMAL);
						g_led_r_state = 0;
						g_led_b_state = 0;
					}
				}
#endif
				//OSTimeDly(100);
				//display_str_battery_low = 1;
				//msgQSend(ApQ, MSG_APQ_POWER_KEY_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
			}
		} else {
			if(battery_low_flag){
				INT8U type;
				battery_low_flag =0;
				msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
				type = GENERAL_KEY;
				msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_KEY_REGISTER, &type, sizeof(INT8U), MSG_PRI_NORMAL);
			}		
			low_voltage_cnt = 0;
		}

		bat_ck_cnt = 0;
		battery_value_sum = 0;
	}
}

#endif




#if C_SCREEN_SAVER == CUSTOM_ON

void ap_peripheral_auto_off_force_disable_set(INT8U auto_off_disable)
{
	auto_off_force_disable = auto_off_disable;
}

void ap_peripheral_lcd_backlight_set(INT8U type)
{
	if (type == BL_ON) {
		if (lcd_bl_sts) {
			lcd_bl_sts = 0;
			tft_backlight_en_set(TRUE);
			DBG_PRINT("LCD ON\r\n");
		}
	} else {
		if (!lcd_bl_sts) {
			lcd_bl_sts = 1;
			tft_backlight_en_set(FALSE);
			DBG_PRINT("LCD OFF\r\n");
		}
	}
}
#endif

void ap_peripheral_night_mode_set(INT8U type)
{
	if(type) {
	  	gpio_write_io(IR_CTRL, 1);
	} else {
	  	gpio_write_io(IR_CTRL, 0);
	}
}

void ap_peripheral_key_init(void)
{
	INT32U i;

	gp_memset((INT8S *) &key_map, NULL, sizeof(KEYSTATUS));
	ap_peripheral_key_register(REGISTER_KEY);
	
	for (i=0 ; i<USE_IOKEY_NO ; i++) {
	  		key_map[i].key_cnt = 0;

	  		if (key_map[i].key_io == PWR_KEY0) {
				while (sys_pwr_key0_read()) {
					OSTimeDly(5);
				}
	  		}
	  		else
	  		{
				key_map[0].key_long_flag = 0;
	  			gpio_init_io(key_map[i].key_io, GPIO_INPUT);
				gpio_set_port_attribute(key_map[i].key_io, ATTRIBUTE_LOW);
		  		gpio_write_io(key_map[i].key_io, key_map[i].key_active^1);
	  		}
	  	}
}

void ap_peripheral_key_register(INT8U type)
{
	INT32U i;
	
	if (type == REGISTER_KEY)
	{
		wifi_key_snd_flag = 0;		
		//================================================================
		key_map[0].key_io = PW_KEY;
		key_map[0].key_function = (KEYFUNC) ap_peripheral_pw_key_exe;
		key_map[0].key_active = 1;

		key_map[1].key_io = OK_KEY;
		key_map[1].key_function = (KEYFUNC) ap_peripheral_ok_key_exe;
		key_map[1].key_active = OK_KEY_ACTIVE;

		//================================================================
		ad_key_map[0].key_io = NULL;
		ad_key_map[0].key_function = (KEYFUNC) ap_peripheral_null_key_exe;
	}
	else if (type == GENERAL_KEY) {
		wifi_key_snd_flag = 0;		
		//================================================================
		key_map[0].key_io = PW_KEY;
		key_map[0].key_function = (KEYFUNC) ap_peripheral_pw_key_exe;

		key_map[1].key_io = OK_KEY;
		key_map[1].key_function = (KEYFUNC) ap_peripheral_ok_key_exe;
		key_map[1].key_active = OK_KEY_ACTIVE;

		//================================================================
		ad_key_map[0].key_io = NULL;
		ad_key_map[0].key_function = (KEYFUNC) ap_peripheral_null_key_exe;
	} else if (type == USBD_DETECT) {
#if USE_IOKEY_NO
		for (i=0 ; i<USE_IOKEY_NO ; i++) {
			if (key_map[i].key_io != PW_KEY)
			{ 
				key_map[i].key_io = NULL;
				key_map[i].key_function = ap_peripheral_null_key_exe;
			}
		}
#endif
#if USE_ADKEY_NO		
		for (i=0 ; i<USE_ADKEY_NO ; i++) {
			ad_key_map[i].key_function = ap_peripheral_null_key_exe;
		}
#endif
	} else if (type == DISABLE_KEY) {
#if USE_IOKEY_NO	
		for (i=0 ; i<USE_IOKEY_NO ; i++) {
			key_map[i].key_io = NULL;
		}
#endif

#if USE_ADKEY_NO		
		for (i=0 ; i<USE_ADKEY_NO ; i++) {
			ad_key_map[i].key_function = ap_peripheral_null_key_exe;
		}
#endif
	} else if (type == BETTERY_LOW_STATUS_KEY){
		key_map[0].key_io = PW_KEY;
		key_map[0].key_function = (KEYFUNC) ap_peripheral_pw_key_exe;
#if USE_ADKEY_NO		
		for (i=0 ; i<USE_ADKEY_NO ; i++) {
			ad_key_map[i].key_function = ap_peripheral_null_key_exe;
		}
#endif
	}
#if DUAL_STREAM_FUNC_ENABLE
	else if (type == WIFI_MODE_KEY)
	{
		wifi_key_snd_flag = 1;
		key_map[0].key_io = PW_KEY;
		key_map[0].key_function = (KEYFUNC) ap_peripheral_pw_key_exe;

		key_map[1].key_io = OK_KEY;
		key_map[1].key_function = (KEYFUNC) ap_peripheral_ok_key_exe;
		//key_map[1].key_function = (KEYFUNC) ap_peripheral_prev_key_exe;
		key_map[1].key_active = OK_KEY_ACTIVE;
/*
		key_map[2].key_io = WIFI_KEY;
		key_map[2].key_function = (KEYFUNC) ap_peripheral_wifi_key_exe;
		//key_map[1].key_function = (KEYFUNC) ap_peripheral_prev_key_exe;
		key_map[2].key_active = WIFI_KEY_ACTIVE;
*/
		#if USE_ADKEY_NO		
		for (i=0 ; i<USE_ADKEY_NO ; i++) {
			ad_key_map[i].key_function = ap_peripheral_null_key_exe;
		}
		#endif
	}
#endif	
}


extern INT8U ap_state_config_auto_off_get(void);
void ap_peripheral_key_judge(void)
{
	INT32U i,type, key_press = 0;
	INT8U flag;
	INT8U ir_key = 0;

	//LED_service();
	//wifi_led();
	for (i=0 ; i<USE_IOKEY_NO ; i++) {
		if (key_map[i].key_io) {
			if (key_map[i].key_io == PW_KEY) {
				flag = sys_pwr_key0_read()? 1 : 0;
			} else {
			  if (key_map[i].key_active)
				flag = gpio_read_io(key_map[i].key_io);
			  else
				flag = (!gpio_read_io(key_map[i].key_io));
			}
			if(flag){
				if(!key_map[i].key_long_flag) {
					key_map[i].key_cnt += 1;
					if (key_map[i].key_cnt > (128/PERI_TIME_INTERVAL_AD_DETECT)) {
						key_map[i].key_long_flag = 1;
					    ap_peripheral_md_mode_set(0);
						key_map[i].key_function(&(key_map[i].key_cnt));
					}
				}
				else
				{
					key_map[i].key_cnt = 0;
				}
				if (key_map[i].key_cnt == 65535) {
					key_map[i].key_cnt = 64;
				}
#if C_IR_REMOTE				
				g_ir_set(0); //按键有效时, 检测到的红外键值无效处理
#endif
			} else {
				key_map[i].key_long_flag = 0;
				//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if C_IR_REMOTE				
				//第二个遥控器的5个按键编码是: 00-ff-00-ff / 00-ff-05-fa / 00-ff-08-f7 / 00-ff-0a-f5 / 00-ff-0d-f2 / 
				ir_key = g_ir_get();
				g_ir_set(0);
				//DBG_PRINT("ir_key: 0x%02X\r\n", ir_key);
				if (ir_key != 0)
				{//无按键时, 检测到的红外键值实现相应功能
					ap_peripheral_md_mode_set(0);
					switch(ir_key)
					{
						#if  C_MOTION_DETECTION == CUSTOM_ON
						case 0x05://红外 - 移动侦测键
							DBG_PRINT("IR:ACTIVE MOTION...\r\n");
							if (video_busy_cnt_get() >= 128*2)
							{
								ap_peripheral_md_mode_set(1);
								msgQSend(ApQ, MSG_APQ_MD_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
								video_busy_cnt_clr();
							}
							break;					
						#endif	
						case 0x08://红外 - 录像键
							if ((video_busy_cnt_get() >= 128*2)&&(ap_state_config_md_get()==0))
							{//执行两次此消息之间要间隔一段时间, 否则有可能引起模式混乱
								DBG_PRINT("IR:ACTIVE RECORD...\r\n");
								msgQSend(ApQ, MSG_APQ_VIDEO_RECORD_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
								video_busy_cnt_clr();
							}
							break;					
						case 0x0A://红外 - 拍照键
							DBG_PRINT("IR:ACTIVE CAPTURE...\r\n");
							msgQSend(ApQ, MSG_APQ_CAPTURE_KEY_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
							break;					
						case 0x0D://红外 - 单独录音键
							if (ap_state_handling_storage_id_get() != NO_STORAGE)
							{
								msgQSend(ApQ, MSG_APQ_WIFI_SWITCH, NULL, NULL, MSG_PRI_NORMAL);
								DBG_PRINT("IR: MSG_APQ_WIFI_SWITCH\r\n");//MSG_APQ_WIFI_SWITCH
								if (wifi_key_snd_flag == 0)
								{
									type = LED_WIFI_ENABLE;
	    							msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &type, sizeof(INT32U), MSG_PRI_NORMAL);
								}
								else
								{
									type = LED_WIFI_DISABLE;
	    							msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &type, sizeof(INT32U), MSG_PRI_NORMAL);
								}
							}
							break;					
					}
					ir_key = 0;
					//g_ir_set(0);
				}
				else
#endif					
				//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				{
					if(key_map[i].key_cnt > 5) {
						//DBG_PRINT("Short Press....\r\n");
					    ap_peripheral_md_mode_set(0);
						key_map[i].key_function(&(key_map[i].key_cnt));
						key_press = 1;
					}
				}
				key_map[i].key_cnt = 0;
				{
#if C_SCREEN_SAVER == CUSTOM_ON
					INT32U cnt_sec;
					INT32U screen_auto_off, md_status;
					INT32U cnt_clr_flag = 0;

					static INT8U auto_off_force_disable_bak = 0;
					
					if (auto_off_force_disable!=auto_off_force_disable_bak) {	// video recording & video playing
						key_active_cnt = 0;
						auto_off_force_disable_bak = auto_off_force_disable;
					}
					
					switch(ap_state_config_auto_off_get())
					{
						case 0: //OFF
							screen_auto_off = 0;
							break;
						case 1: //1 min
							screen_auto_off = 1;
							break;
						case 2: //2 min
							screen_auto_off = 2;
							break;
						case 3: //5 min
							screen_auto_off = 5;
							break;
					}
					
					//screen_auto_off = ap_state_config_auto_off_get();
					md_status = ap_state_config_md_get();
					if ((screen_auto_off != 0) && !auto_off_force_disable && (usb_state_get()==0) && !md_status&!wifi_key_snd_flag) { //don't auto off under following conditions:
																											 //1. recording & playing avi files(by auto_off_force_disable)
																											 //2. usb connecting 
																											 //3. motion detect on
																											 //4. wifi is open
						//DBG_PRINT("Auto Power off Waitting\r\n");
						/*																				
						if(screen_auto_off == 1) {	//1 min
							screen_auto_off = 1;
						} else if (screen_auto_off == 2) { //3min
							screen_auto_off = 2;
						}
						*/
						if (cnt_clr_flag==0) {
							key_active_cnt += PERI_TIME_INTERVAL_AD_DETECT;//PERI_TIME_INTERVAL_KEY_DETECT;
						}	
						cnt_sec = (key_active_cnt >> 7) / USE_IOKEY_NO;
						if (cnt_sec > screen_auto_off*60*0xff) {
							key_active_cnt = 0;
							type = 0;
							msgQSend(ApQ, MSG_APQ_POWER_KEY_ACTIVE, &type, sizeof(INT32U), MSG_PRI_NORMAL);
							DBG_PRINT("NO active Auto Power off\r\n");
							/*
							if(screen_saver_enable) {
							screen_saver_enable = 0;	
							msgQSend(ApQ, MSG_APQ_KEY_WAKE_UP, NULL, NULL, MSG_PRI_NORMAL);
							}
							DBG_PRINT("NO active Auto Power off\r\n");
							*/
						}
						cnt_clr_flag++;
					} 
					/*
					if(screen_saver_enable&&key_press)
					{
						screen_saver_enable = 0;	
						msgQSend(ApQ, MSG_APQ_KEY_WAKE_UP, NULL, NULL, MSG_PRI_NORMAL);
					}
					*/
					if  (cnt_clr_flag == 0)
					{
						key_active_cnt = 0;
					}
#endif
				}
			}
		}
	}
}

static int ap_peripheral_power_key_read(int pin)
{
	int status;


	#if  (KEY_TYPE == KEY_TYPE2)||(KEY_TYPE == KEY_TYPE3)||(KEY_TYPE == KEY_TYPE4)||(KEY_TYPE == KEY_TYPE5)
		status = gpio_read_io(pin);
	#else
	switch(pin)
	{
		case PWR_KEY0:
			status = sys_pwr_key0_read();
			break;
		case PWR_KEY1:
			status = sys_pwr_key1_read();
			break;
	}
	#endif

	if (status!=0)
		 return 1;
	else return 0;
}

INT8U ap_peripheral_poweron_usb_det(void)
{
	INT8U tmp_cnt_in=0,tmp_cnt_out=0;
	
	while(1)
	{
		if (!ap_peripheral_power_key_read(C_USBDEVICE_PIN))
		{
			tmp_cnt_out++;
			tmp_cnt_in=0;
			if(tmp_cnt_out>10)
			{
				return 0;
			}
		}
		else
		{
			tmp_cnt_out=0;
			tmp_cnt_in++;
			if(tmp_cnt_in>10)
			{
				return 1;
			}
		}
		drv_msec_wait(2);
	}
}

INT8U ap_peripheral_poweron_pwrkey_det(void)
{
	INT16U tmp_cnt_press=0;
	
	while(ap_peripheral_power_key_read(PW_KEY))
	{
		tmp_cnt_press++;
		if(tmp_cnt_press>10)
		{
			return 1;
		}
		drv_msec_wait(10);
	}
	return 0;
}

void ap_peripheral_adaptor_out_judge(void)
{

	INT32U type;

#if USB_PHY_SUSPEND == 1
	if (s_usbd_pin == 0)
	{
		if (!ap_peripheral_power_key_read(C_USBDEVICE_PIN)) {
			if (phy_cnt == PERI_USB_PHY_SUSPEND_TIME) {
				// disable USB PHY CLK for saving power
				DBG_PRINT("MSG_CHARGE_PLUG_OUT\r\n");
				OSQPost(USBTaskQ, (void *) MSG_CHARGE_PLUG_OUT);  // ﹚璶Ω MSG_USBD_INITIAL 癳 USB QUEUE 
				phy_cnt++;	// ヘ琌 Turn Off 暗Ω
			}
			else if (phy_cnt<PERI_USB_PHY_SUSPEND_TIME) {
				phy_cnt++;
			}
		}
		else {
			phy_cnt = 0;
		}
	}
	else phy_cnt = 0;
#endif

	adp_out_cnt++;
	switch(adp_status) {
		case 0: //unkown state
			if (ap_peripheral_power_key_read(ADP_OUT_PIN)) {
				adp_cnt++;
				if (adp_cnt > 16) {
					adp_out_cnt = 0;
					adp_cnt = 0;
					adp_status = 1;
					OSQPost(USBTaskQ, (void *) MSG_USBD_INITIAL);
				  #if C_BATTERY_DETECT == CUSTOM_ON && USE_ADKEY_NO
					//battery_lvl = 1;
				  #endif
				}
			} else {
				adp_cnt = 0;
			}

			if (adp_out_cnt > 24) {
				adp_out_cnt = 0;
				adp_status = 3;
			  #if C_BATTERY_DETECT == CUSTOM_ON && USE_ADKEY_NO
				//battery_lvl = 2;
				low_voltage_cnt = 0;
			  #endif
			}
			break;

		case 1: //adaptor in state
			if (!ap_peripheral_power_key_read(ADP_OUT_PIN)) {
				if (adp_out_cnt > 8) {
					/*
					if(ap_state_config_car_mode_get())
					{
						adp_status = 2;
					}
					else
					{
						adp_status = 3;
						adp_out_cnt = 0;
					}
					*/
					adp_status = 2;
					low_voltage_cnt = 0;
					// 璝棵辊玂臔秨璶翴獹璉
					if(screen_saver_enable) {
						screen_saver_enable = 0;
  						//ap_state_handling_lcd_backlight_switch(1);
					}					
				} 
			} else {
				adp_out_cnt = 0;
			}
			break;

		case 2: //adaptor out state
			if (!ap_peripheral_power_key_read(ADP_OUT_PIN)) {
				//DBG_PRINT("adp_out_cnt = %d\r\n",adp_out_cnt);	
				if ((adp_out_cnt > PERI_ADP_OUT_PWR_OFF_TIME)) {
					//ap_peripheral_pw_key_exe(&adp_out_cnt);
					/*
					if (usb_state_get() == 2)
					{//边充边录
						adp_out_cnt = 0;
						adp_status = 1;
					}
					else
					*/
					{//USB模式
						type = 0;
						msgQSend(ApQ, MSG_APQ_POWER_KEY_ACTIVE, &type, sizeof(INT32U), MSG_PRI_NORMAL);
					}
					
				}
				adp_cnt = 0;
			} else {
				adp_cnt++;
				if (adp_cnt > 3) {
					adp_out_cnt = 0;
					adp_status = 1;
					usbd_exit = 0;
					OSQPost(USBTaskQ, (void *) MSG_USBD_INITIAL);
				}
			}
			break;

		case 3://adaptor initial out state
			if (ap_peripheral_power_key_read(ADP_OUT_PIN)) {
				if (adp_out_cnt > 3) {
					adp_out_cnt = 0;
					adp_status = 1;
					OSQPost(USBTaskQ, (void *) MSG_USBD_INITIAL);
				} 
			} else {
				adp_out_cnt = 0;
			}
			break;
		default:
			break;
	}
	
	if (s_usbd_pin == 1) {
		usbd_cnt++;
		if (!ap_peripheral_power_key_read(C_USBDEVICE_PIN)) {
			if (usbd_cnt > 3) {
				ap_peripheral_usbd_plug_out_exe(&usbd_cnt);	
			} 
		} else {
			usbd_cnt = 0;
		}
	}


}

INT8U Image_Processing_Busy(void)
{
	if ((ap_state_handling_storage_id_get() == NO_STORAGE)||pic_down_flag||video_down_flag)
	{//Error
		return 0;
	}
	else
	{//OK
		return 1;
	}
}

void ap_peripheral_function_key_exe(INT16U *tick_cnt_ptr)
{
	if(*tick_cnt_ptr > 24) {
	}else{
		DBG_PRINT("MODE_ACTIVE\r\n");
		DBG_PRINT("*tick_cnt_ptr=%d\r\n",*tick_cnt_ptr);
		msgQSend(ApQ, MSG_APQ_MODE, NULL, NULL, MSG_PRI_NORMAL);
	}

	*tick_cnt_ptr = 0;
}

void ap_peripheral_next_key_exe(INT16U *tick_cnt_ptr)
{
	INT8U data = 0;

	if(screen_saver_enable) {
		screen_saver_enable = 0;
		msgQSend(ApQ, MSG_APQ_KEY_WAKE_UP, NULL, NULL, MSG_PRI_NORMAL);
	} else {	
		if(*tick_cnt_ptr > 24) {
			msgQSend(ApQ, MSG_APQ_FORWARD_FAST_PLAY, &data, sizeof(INT8U), MSG_PRI_NORMAL);
		}else{
			if  (wifi_key_snd_flag==0) {
				msgQSend(ApQ, MSG_APQ_AUDIO_EFFECT_DOWN, NULL, NULL, MSG_PRI_NORMAL);
			}
			#if SUPORT_GET_JPG_BUF == CUSTOM_ON
			if(present_state == STATE_VIDEO_RECORD)
			{
				msgQSend(ApQ, MSG_APQ_GET_JPEG_ON_RECORDING, NULL, NULL, MSG_PRI_NORMAL);
			}
			else
			{
				msgQSend(ApQ, MSG_APQ_NEXT_KEY_ACTIVE, &data, sizeof(INT8U), MSG_PRI_NORMAL);
			}
			#else
			msgQSend(ApQ, MSG_APQ_NEXT_KEY_ACTIVE, &data, sizeof(INT8U), MSG_PRI_NORMAL);
			#endif
		}
	}

	*tick_cnt_ptr = 0;
}

void ap_peripheral_prev_key_exe(INT16U *tick_cnt_ptr)
{
/*
	INT8U data = 0;

	if(screen_saver_enable) {
		screen_saver_enable = 0;
		msgQSend(ApQ, MSG_APQ_KEY_WAKE_UP, NULL, NULL, MSG_PRI_NORMAL);
	} else {
		if(*tick_cnt_ptr > 24) {
			msgQSend(ApQ, MSG_APQ_BACKWORD_FAST_PLAY, &data, sizeof(INT8U), MSG_PRI_NORMAL);

		}else{
			if  (wifi_key_snd_flag==0) {
				msgQSend(ApQ, MSG_APQ_AUDIO_EFFECT_UP, NULL, NULL, MSG_PRI_NORMAL);
			}
			msgQSend(ApQ, MSG_APQ_PREV_KEY_ACTIVE, &data, sizeof(INT8U), MSG_PRI_NORMAL);
			//msgQSend(ApQ, MSG_APQ_WIFI_SWITCH, &data, sizeof(INT8U), MSG_PRI_NORMAL);
			DBG_PRINT("MSG_APQ_WIFI_SWITCH\r\n");//MSG_APQ_WIFI_SWITCH
		}
	}
	*/
	*tick_cnt_ptr = 0;
}

void ap_peripheral_wifi_key_exe(INT16U *tick_cnt_ptr)
{
	INT32U led_type;
	
	if (!s_usbd_pin)
	{
		if ((ap_video_record_sts_get() & 0x2) == 0)
		{
			msgQSend(ApQ, MSG_APQ_WIFI_SWITCH, NULL, NULL, MSG_PRI_NORMAL);
			DBG_PRINT("MSG_APQ_WIFI_SWITCH\r\n");//MSG_APQ_WIFI_SWITCH
			if (wifi_key_snd_flag)
			{
				led_type = LED_WIFI_DISABLE;
				msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
			}
			else
			{
				led_type = LED_WIFI_ENABLE;
				msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
			}
		}
	}
	*tick_cnt_ptr = 0;
}	

void ap_peripheral_ok_key_exe(INT16U *tick_cnt_ptr)
{
	INT32U led_type;
	
	if (!s_usbd_pin)
    {
		if(*tick_cnt_ptr >= (128/PERI_TIME_INTERVAL_AD_DETECT)) 
		{//开关红外功能
			if ((ap_video_record_sts_get() & 0x2) == 0)
			{
				if (video_busy_cnt_get() >= 128*2)
				{
					msgQSend(ApQ, MSG_APQ_WIFI_SWITCH, NULL, NULL, MSG_PRI_NORMAL);
					DBG_PRINT("MSG_APQ_WIFI_SWITCH\r\n");//MSG_APQ_WIFI_SWITCH
					if (wifi_key_snd_flag)
					{
						led_type = LED_WIFI_DISABLE;
						msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
					}
					else
					{
						led_type = LED_WIFI_ENABLE;
						msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
					}
					video_busy_cnt_clr();
				}
			}
			else
			{
				led_type = LED_RECORD_IR_ENABLE;
				msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
			}
		}
		else
		{
			if (Image_Processing_Busy())
			{
				DBG_PRINT("ACTIVE RECORD...\r\n");
				//msgQSend(ApQ, MSG_APQ_CAPTURE_KEY_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
				msgQSend(ApQ, MSG_APQ_VIDEO_RECORD_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);	
			}
			video_busy_cnt_clr();
		}
	}
	*tick_cnt_ptr = 0;
}

#if  C_MOTION_DETECTION == CUSTOM_ON
void ap_peripheral_md_key_exe(INT16U *tick_cnt_ptr)
{
	if (!s_usbd_pin)
    {
		DBG_PRINT("ACTIVE MOTION...\r\n");
		if (video_busy_cnt_get() >= 128*2)
		{
			ap_peripheral_md_mode_set(1);
			msgQSend(ApQ, MSG_APQ_MD_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
			video_busy_cnt_clr();
		}
	}
	*tick_cnt_ptr = 0;
}
#endif

#if KEY_FUNTION_TYPE == SAMPLE2
void ap_peripheral_capture_key_exe(INT16U *tick_cnt_ptr)
{
	if(screen_saver_enable) {
		screen_saver_enable = 0;
		msgQSend(ApQ, MSG_APQ_KEY_WAKE_UP, NULL, NULL, MSG_PRI_NORMAL);
	} else {
		msgQSend(ApQ, MSG_APQ_CAPTURE_KEY_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
	}
	*tick_cnt_ptr = 0;
}
#endif
void ap_peripheral_sos_key_exe(INT16U *tick_cnt_ptr)
{

	if(screen_saver_enable) {
		screen_saver_enable = 0;
		msgQSend(ApQ, MSG_APQ_KEY_WAKE_UP, NULL, NULL, MSG_PRI_NORMAL);
	} else {
		if(*tick_cnt_ptr > 24) {
		}else{
			msgQSend(ApQ, MSG_APQ_SOS_KEY_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
		}
	}
	*tick_cnt_ptr = 0;
}

void ap_peripheral_usbd_plug_out_exe(INT16U *tick_cnt_ptr)
{
	msgQSend(ApQ, MSG_APQ_DISCONNECT_TO_PC, NULL, NULL, MSG_PRI_NORMAL);
	*tick_cnt_ptr = 0;
}

void ap_peripheral_pw_key_exe(INT16U *tick_cnt_ptr)
{
	INT32U type;

    if (!s_usbd_pin)
    {
    	/*
    	if (ChargeMode_get() == 1)
    	{//充电模式
    		if (ap_state_handling_storage_id_get()!=NO_STORAGE)
    		{
	    		ChargeMode_set(0);
	    		type = LED_INIT;
				msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_SET, &type, sizeof(INT32U), MSG_PRI_NORMAL);
				DBG_PRINT("Enter Normal Mode...\r\n");
			}
    	}
    	else
    	*/
    	{//正常工作模式
			if(*tick_cnt_ptr >= (128/PERI_TIME_INTERVAL_AD_DETECT)) {
				if (usb_state_get() != 2)
				{//正常工作模式
					DBG_PRINT("Power key OFF\r\n");
					type = 1;
					msgQSend(ApQ, MSG_APQ_POWER_KEY_ACTIVE, &type, sizeof(INT32U), MSG_PRI_NORMAL);
				}
			} else {
				//DBG_PRINT("ACTIVE RECORD...\r\n");
				if (Image_Processing_Busy())
				{
					DBG_PRINT("ACTIVE CAPTURE...\r\n");
					msgQSend(ApQ, MSG_APQ_CAPTURE_KEY_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
					//msgQSend(ApQ, MSG_APQ_VIDEO_RECORD_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
				}
				video_busy_cnt_clr();
			}
		}
	}
	else
	{
		if (ap_state_handling_storage_id_get() != NO_STORAGE)
		{
			DBG_PRINT("USB SWITCH...\r\n");
			OSQPost(USBTaskQ, (void *) MSG_USBD_SWITCH);
		}
	}
	*tick_cnt_ptr = 0;
}

void ap_peripheral_menu_key_exe(INT16U *tick_cnt_ptr)
{
	if(*tick_cnt_ptr > 24) {
	}else{
		msgQSend(ApQ, MSG_APQ_MENU_KEY_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
	}
	*tick_cnt_ptr = 0;
}

void ap_peripheral_null_key_exe(INT16U *tick_cnt_ptr)
{
	
}

void ap_TFT_backlight_tmr_check(void)
{
	if(backlight_tmr) {
		backlight_tmr--;
		if((backlight_tmr == 0) && (tv == !TV_DET_ACTIVE))
		{
			//gpio_write_io(TFT_BL, DATA_HIGH);	//turn on LCD backlight
			tft_backlight_en_set(1);
		}
	}
}

//+++ TV_OUT_D1
#if TV_DET_ENABLE
INT8U tv_plug_status_get(void)
{
	return tv_plug_in_flag;
}
#endif
//---

void ap_peripheral_tv_detect(void)
{
#if TV_DET_ENABLE
	INT8U temp;

	temp = ap_state_config_tv_switch_get();//gpio_read_io(AV_IN_DET);
	if(temp != tv) {
		tv_debounce_cnt++;
		if(tv_debounce_cnt > 4) {
			tv_debounce_cnt = 0;
			tv = temp;
			if(tv == !TV_DET_ACTIVE) {  //display use TFT
				//backlight_tmr = PERI_TIME_BACKLIGHT_DELAY;	//delay some time to enable LCD backlight so that no noise shown on LCD
				//gpio_write_io(SPEAKER_EN, DATA_HIGH);	//open local speaker

				//+++ TV_OUT_D1
				tv_plug_in_flag = 0;
				msgQSend(ApQ, MSG_APQ_TV_PLUG_OUT, NULL, NULL, MSG_PRI_NORMAL);
				//---

			} else { //display use TV
				gpio_write_io(SPEAKER_EN, DATA_LOW);	//mute local speaker
				//gpio_write_io(TFT_BL, DATA_LOW);		//turn off LCD backlight

				//+++ TV_OUT_D1
				tv_plug_in_flag = 1;
				msgQSend(ApQ, MSG_APQ_TV_PLUG_IN, NULL, NULL, MSG_PRI_NORMAL);
				//---
			}
		}
	} else {
		tv_debounce_cnt = 0;
	}
#endif
}

void ap_peripheral_gsensor_data_register(void )
{
	avi_adc_gsensor_data_register(&gsensor_msgQId0, (INT32U*)(&gsensor_msgId0));
}

void ap_peripheral_read_gsensor(void)
{
	static INT16U g_idx = 0;
	INT16U temp;

	if(gsensor_lock_flag) return;
	gsensor_lock_flag = 1;

	temp = G_sensor_get_int_active();

	if((temp != 0xff) && (temp & 0x04)) //active int flag
	  { 
        G_sensor_clear_int_flag();
        
		if(ap_state_config_G_sensor_get()) {
			msgQSend(ApQ, MSG_APQ_SOS_KEY_ACTIVE, NULL, NULL, MSG_PRI_NORMAL);
		}

		DBG_PRINT("gsensor int actived\r\n");
	}

	if (gsensor_msgQId0!=NULL)
	{
		G_Sensor_gps_data_get(gsensor_data[g_idx]);
		OSQPost((OS_EVENT*)gsensor_msgQId0, (void*)(gsensor_msgId0|g_idx));
		g_idx ^= 0x1;
	}

	gsensor_lock_flag = 0;
	//DBG_PRINT("gsensor chipid = 0x%x\r\n", temp);
}

void ap_peripheral_config_store(void)
{
	if (config_cnt++ == PERI_COFING_STORE_INTERVAL) {
		config_cnt = 0;
		msgQSend(ApQ, MSG_APQ_USER_CONFIG_STORE, NULL, NULL, MSG_PRI_NORMAL);
	}
}


void ap_peripheral_hdmi_detect(void)
{
	static BOOLEAN  HDMI_StatusBak = 0;
	static BOOLEAN  HDMI_StateBak = 0;  // HDMI_REMOVE
	static unsigned char HDMI_DetCount = 0;
	BOOLEAN cur_status;

	cur_status = gpio_read_io(HDMI_IN_DET);
	// debounce
	if (HDMI_StatusBak != cur_status) {
		HDMI_DetCount = 0;
	}
	else {
		HDMI_DetCount++;
	}

	if (HDMI_DetCount == 0x10)
	{
		if (cur_status != HDMI_StateBak)
		{
			HDMI_DetCount = 0;
			if(cur_status)	// HDM_IN_DET
			{
				msgQSend(ApQ, MSG_APQ_HDMI_PLUG_IN, NULL, NULL, MSG_PRI_NORMAL);
				gpio_write_io(SPEAKER_EN, DATA_LOW);	//mute local speaker
				DBG_PRINT("HDMI Insert\r\n");	// HDMI Insert
			}
			else
			{
				msgQSend(ApQ, MSG_APQ_HDMI_PLUG_OUT, NULL, NULL, MSG_PRI_NORMAL);
				gpio_write_io(SPEAKER_EN, DATA_LOW);//DATA_HIGH
				DBG_PRINT("HDMI Remove\r\n");	// HDMI Remove
			}
		}
		HDMI_StateBak = cur_status;
	}
	HDMI_StatusBak = cur_status;
}

#ifdef SDC_DETECT_PIN

void ap_peripheral_SDC_detect_init(void)
{

	gpio_init_io(SDC_DETECT_PIN,GPIO_INPUT);
  	gpio_set_port_attribute(SDC_DETECT_PIN, ATTRIBUTE_LOW);
  	gpio_write_io(SDC_DETECT_PIN, 1);	//pull high
}

INT32S ap_peripheral_SDC_at_plug_OUT_detect()
{
	INT32S ret;
	BOOLEAN cur_status;
	ap_peripheral_SDC_detect_init();
	cur_status = gpio_read_io(SDC_DETECT_PIN);
//	DBG_PRINT("SDC_DETECT_PIN_=%d\r\n",cur_status);
	if(cur_status){		//plug_out
		ret = -1;
	}else{			//plug_in	
		ret = 0;
	}
	return ret;
}



INT32S ap_peripheral_SDC_at_plug_IN_detect()
{
	INT32S ret;
	BOOLEAN cur_status;
	ap_peripheral_SDC_detect_init();
	cur_status = gpio_read_io(SDC_DETECT_PIN);
//	DBG_PRINT("SDC_DETECT_PIN=%d\r\n",cur_status);
	if(cur_status){		//plug_out
		ret = -1;
	}else{				//plug_in
		ret = 0;
	}
	return ret;
}

#endif
