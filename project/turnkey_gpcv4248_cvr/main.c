#include "main.h"

extern volatile void *NorWakeupVector;
extern volatile void *Wakeup;

#define PERIPHERAL_HANDLING_STACK_SIZE		256
#define STATE_HANDLING_STACK_SIZE			1024
#define DISPLAY_TASK_STACK_SIZE				1024
#define STORAGE_SERVICE_TASK_STACK_SIZE		512
#define USB_TASK_STACK_SIZE					512
#define IDLE_TASK_STACK_SIZE				32
#define C_AUDIO_TASK_STACK_SIZE				1024
#define C_AUDIO_DAC_TASK_STACK_SIZE			256
#define C_FILESRV_TASK_STACK_SIZE	        512

INT32U PeripheralHandlingStack[PERIPHERAL_HANDLING_STACK_SIZE];
INT32U StateHandlingStack[STATE_HANDLING_STACK_SIZE];
INT32U DisplayTaskStack[DISPLAY_TASK_STACK_SIZE];
INT32U AudioTaskStack[C_AUDIO_TASK_STACK_SIZE];
INT32U AudioDacTaskStack[C_AUDIO_DAC_TASK_STACK_SIZE];
INT32U Filesrv[C_FILESRV_TASK_STACK_SIZE];
INT32U StorageServiceTaskStack[STORAGE_SERVICE_TASK_STACK_SIZE];
INT32U USBTaskStack[USB_TASK_STACK_SIZE];
INT32U IdleTaskStack[IDLE_TASK_STACK_SIZE];


void idle_task_entry(void *para)
{
	OS_CPU_SR cpu_sr;
	INT16U i;
	while (1) {
OS_ENTER_CRITICAL();
		i=0x5005;
		R_SYSTEM_WAIT = i;
		i = R_CACHE_CTRL;

		ASM(NOP);
		ASM(NOP);
		ASM(NOP);
		ASM(NOP);
		ASM(NOP);
		ASM(NOP);
		ASM(NOP);
		ASM(NOP);
		ASM(NOP);
		ASM(NOP);
		ASM(NOP);
		ASM(NOP);
		ASM(NOP);
		ASM(NOP);
		ASM(NOP);
		ASM(NOP);
OS_EXIT_CRITICAL();	    
	}
}


#if 0
static int checksum(void)
{
	char *p_src = (char *)0;
	char *p_dst = (char *)0x30000200;
	unsigned int size = (700*1024);
	unsigned int sum1 = 0;
	unsigned int sum2 = 0;
	unsigned int ret = 0;
	unsigned int i;

	// Boot Procedure will copy data from SPI area to SDRAM area.
	// This function want to make sure SDRAM data is correct.
	for (i=0;i<size;++i)
	{
		sum1 += (unsigned int)p_src[i];
		sum2 += (unsigned int)p_dst[i];
	}
	
	if (sum1!=sum2)
		ret = 1;
	return ret;
}
#endif

#if 0
#define MAIN_TASK_PRIORITY		48
#define TaskStackSize 		1024
INT32U MainTaskStack[TaskStackSize];
extern int ssv6xxx_dev_init(int argc, char *argv[]);

void Main_task_entry(void *para)
{
	ssv6xxx_dev_init(0,NULL);
	while(1);
}
#endif

INT8U PowerOn_Low_Check=0;
extern INT8U Check_PowerOn_Battery_Value(void);
extern INT8U ap_peripheral_poweron_usb_det(void);
extern INT8U ap_peripheral_poweron_pwrkey_det(void);
extern void ap_peripheral_handling_cooler_pin_init(void);
void Main(void *free_memory)
{
	INT32U free_memory_start, free_memory_end;
	//INT8U usb_charge_status;
	//INT32U chksum_flag = checksum();

	// Touch those sections so that they won't be removed by linker
	if (!NorWakeupVector && !Wakeup) {
		*((INT32U *) free_memory+1) = 0x0;
	}

	free_memory_start = ((INT32U) free_memory + 3) & (~0x3);	// Align to 4-bytes boundry
	free_memory_end = (INT32U) SDRAM_END_ADDR & ~(0x3);

	// Initiate Operating System
	OSInit();
 
	// Initiate drvier layer 1 modules
	setting_by_iRAM();
    drv_l1_init();
    ap_peripheral_handling_cooler_pin_init();
    
    #if((GPDV_BOARD_VERSION != DVP_V1_0)&&(GPDV_BOARD_VERSION != GPCV4247_WIFI))
    usb_charge_status = ap_peripheral_poweron_usb_det();
    if(usb_charge_status==0)
    {
    	DBG_PRINT("usb dont plug in---\r\n");
    	if(ap_peripheral_poweron_pwrkey_det()==0)
    	{
    		DBG_PRINT("pwr key short press---\r\n");
    		R_SYSTEM_WATCHDOG_CTRL = 0x8002;
    		while(1);
    	}
    }
    #endif
    

    {	// turn on WiFi module
       gpio_init_io(WIFI_LDO_EN, GPIO_OUTPUT);
       gpio_set_port_attribute(WIFI_LDO_EN, ATTRIBUTE_HIGH);
       gpio_write_io(WIFI_LDO_EN, DATA_HIGH); //RAY :default low.
    }
    
    #if POWERON_BATTERY_CHECK ==1
    PowerOn_Low_Check =Check_PowerOn_Battery_Value();
    #endif

	timer_freq_setup(TIMER_A, OS_TICKS_PER_SEC, 0, OSTimeTick);

	// Initiate driver layer 2 modules
	drv_l2_init();

	// Initiate gplib layer modules
	gplib_init(free_memory_start, free_memory_end);

	#if 0
	OSTaskCreate(Main_task_entry, (void *) 0, &MainTaskStack[TaskStackSize - 1], MAIN_TASK_PRIORITY);	
	#else	
	// Initiate applications here
	OSTaskCreate(task_peripheral_handling_entry, (void *) 0, &PeripheralHandlingStack[PERIPHERAL_HANDLING_STACK_SIZE - 1], PERIPHERAL_HANDLING_PRIORITY);
	OSTaskCreate(state_handling_entry, (void *) 0, &StateHandlingStack[STATE_HANDLING_STACK_SIZE - 1], STATE_HANDLING_PRIORITY);
	#if GPDV_BOARD_VERSION != GPCV4247_WIFI	
	OSTaskCreate(task_display_entry, (void *) 0, &DisplayTaskStack[DISPLAY_TASK_STACK_SIZE - 1], DISPLAY_TASK_PRIORITY);
	#endif
	OSTaskCreate(task_storage_service_entry, (void *) 0, &StorageServiceTaskStack[STORAGE_SERVICE_TASK_STACK_SIZE - 1], STORAGE_SERVICE_PRIORITY);
	OSTaskCreate(state_usb_entry, (void *) 0, &USBTaskStack[USB_TASK_STACK_SIZE - 1], USB_DEVICE_PRIORITY);

	OSTaskCreate(audio_dac_task_entry, (void *) 0, &AudioDacTaskStack[C_AUDIO_DAC_TASK_STACK_SIZE - 1], DAC_PRIORITY);
	OSTaskCreate(audio_task_entry, (void *) 0, &AudioTaskStack[C_AUDIO_TASK_STACK_SIZE - 1], AUD_DEC_PRIORITY);
	OSTaskCreate(filesrv_task_entry,(void *) 0, &Filesrv[C_FILESRV_TASK_STACK_SIZE - 1], TSK_PRI_FILE_SRV);
	#endif
/*
#if SUPPORT_JTAG == CUSTOM_OFF
	OSTaskCreate(idle_task_entry, (void *) 0, &IdleTaskStack[IDLE_TASK_STACK_SIZE - 1], (OS_LOWEST_PRIO - 2));
#endif
*/
	DBG_PRINT("CPU: %d MHz\r\n", INIT_MHZ);
    DBG_PRINT("SDRAM: %d MB\r\n", SDRAM_SIZE>>20);	
    
	OSStart();
}
