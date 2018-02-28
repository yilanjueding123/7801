#ifndef __DRIVER_L1_H__
#define __DRIVER_L1_H__

#include "project.h"
#include "gp_stdlib.h"
#if _OPERATING_SYSTEM != _OS_NONE
#include "os.h"
#endif
#include "driver_l1_cfg.h"
#include "drv_l1_system.h"
#include "drv_l1_uart.h"
#include "drv_l1_interrupt.h"
#include "drv_l1_cache.h"
#include "drv_l1_sdc.h"
#include "drv_l1_dma.h"
#include "drv_l1_scaler.h"
#include "drv_l1_i2c.h"
#include "drv_l1_conv422to420.h"
#include "drv_l1_gsensor.h"
#include "drv_l1_cdsp.h"

#ifndef __ALIGN_DEFINE__
#define __ALIGN_DEFINE__
    #ifdef __ADS_COMPILER__
        #define ALIGN32 __align(32)
        #define ALIGN16 __align(16)
        #define ALIGN8 __align(8)
        #define ALIGN4 __align(4)
    #else
        #ifdef __CS_COMPILER__
        #define ALIGN32 __attribute__ ((aligned(32)))
        #define ALIGN16 __attribute__ ((aligned(16)))
        #define ALIGN8 __attribute__ ((aligned(8)))
        #define ALIGN4 __attribute__ ((aligned(4)))
        #else
        #define ALIGN32 __align(32)
        #define ALIGN16 __align(16)
        #define ALIGN8 __align(8)
        #define ALIGN4 __align(4)
        #endif
    #endif
#endif

#ifndef __CS_COMPILER__
    #define ASM(asm_code)  __asm {asm_code}  /*ADS embeded asm*/
    #define IRQ            __irq
    #define PACKED 			__packed
#else
    #define ASM(asm_code)  asm(#asm_code);  /*ADS embeded asm*/
    #define IRQ            __attribute__ ((interrupt))
    #define PACKED	   __attribute__((__packed__))
#endif

typedef INT32S (*fp_msg_qsend)(void*,INT32U,void*,INT32U,INT32U);

extern void drv_l1_init(void);

// ARM
#define	EXCEPTION_UNDEF     0x1
#define	EXCEPTION_SWI       0x2
#define	EXCEPTION_PABORT    0x3
#define	EXCEPTION_DABORT    0x4
#define	EXCEPTION_IRQ       0x5
#define	EXCEPTION_FIQ       0x6

extern void exception_table_init(void);
extern INT32S register_exception_table(INT32U exception, INT32U handler);
#if _OPERATING_SYSTEM == 0
extern void fiq_enable(void);
extern void	irq_enable(void);
#endif
void mic_init(void);

// system
extern INT32U  MCLK;
extern INT32U  MHZ;

#ifndef _SYS_SDRAM_DEF_
#define _SYS_SDRAM_DEF_
typedef enum {
    CL_2=2,
    CL_3=3
} SDRAM_CL;

typedef enum {
    SD_24MHZ=24,
    SD_48MHZ=48,
    SD_96MHZ=96
} SDRAM_CLK_MHZ;

typedef enum {
    SDRAM_OUT_DLY_LV0=0,
    SDRAM_OUT_DLY_LV1 ,
    SDRAM_OUT_DLY_LV2 ,
    SDRAM_OUT_DLY_LV3 ,
    SDRAM_OUT_DLY_LV4 ,
    SDRAM_OUT_DLY_LV5 ,
    SDRAM_OUT_DLY_LV6 ,
    SDRAM_OUT_DLY_LV7 ,
    SDRAM_OUT_DLY_LV8 ,
    SDRAM_OUT_DLY_LV9 ,
    SDRAM_OUT_DLY_LV10,
    SDRAM_OUT_DLY_LV11,
    SDRAM_OUT_DLY_LV12,
    SDRAM_OUT_DLY_LV13,
    SDRAM_OUT_DLY_LV14,
    SDRAM_OUT_DLY_LV15
} SD_CLK_OUT_DELAY_CELL;

typedef enum {
    SDRAM_IN_DLY_LV0=0,
    SDRAM_IN_DLY_LV1 ,
    SDRAM_IN_DLY_LV2 ,
    SDRAM_IN_DLY_LV3 ,
    SDRAM_IN_DLY_LV4 ,
    SDRAM_IN_DLY_LV5 ,
    SDRAM_IN_DLY_LV6 ,
    SDRAM_IN_DLY_LV7 ,
    SDRAM_IN_DLY_DISABLE
} SD_CLK_IN_DELAY_CELL;

typedef enum {
    SD_CLK_PHASE_0=0,
    SD_CLK_PHASE_180
} SD_CLK_PHASE;

typedef enum {
    SD_DISABLE=0,
    SD_ENABLE
} SD_SWITCH;

typedef enum {
    SD_BUS_16bit=0,
    SD_BUS_32bit
} SD_BUS_WIDTH;


typedef enum {
    SD_BANK_1M=0,
    SD_BANK_4M
} SD_BANK_BOUND;

typedef enum {
    SD_16Mb=0x0,
    SD_2MB=SD_16Mb,
    SD_64Mb=0x1,
    SD_8MB=SD_64Mb,
    SD_128Mb=0x2,
    SD_16MB=SD_128Mb,
    SD_256Mb=0x3,
    SD_32MB=SD_256Mb,
    SD_512Mb=0x4,
    SD_64MB=SD_512Mb,
    SD_1024Mb=0x4,
    SD_128MB=SD_1024Mb
} SD_SIZE;

typedef enum {
    tREFI_3o9u=39,
    tREFI_7o8u=78,    /*suggest: when SDRAM size >= 256Mb*/
    tREFI_15o6u=156,  /*suggest: when SDRAM size < 256Mb*/
    tREFI_31o2u=312
} tREFI_ENUM;

#endif  // _SYS_SDRAM_DEF_

// sys-arbit init
extern void system_bus_arbiter_init(void);

// sys-PLL setting
extern void system_set_pll(INT32U PLL_CLK);
extern void system_enter_halt_mode(void);
extern void system_power_on_ctrl(void);
extern void system_da_ad_pll_en_set(BOOLEAN status);
extern INT32U system_rtc_counter_get(void);
extern void system_rtc_counter_set(INT32U rtc_cnt);
extern void sys_weak_6M_set(BOOLEAN status);
extern void sys_reg_sleep_mode_set(BOOLEAN status);
extern void sys_ir_opcode_clear(void);
extern INT32U sys_ir_opcode_get(void);

/*iRAM system functions*/
#ifndef _SDRAM_DRV
#define _SDRAM_DRV
typedef enum {
    SDRAM_DRV_4mA=0x0000,
    SDRAM_DRV_8mA=0x5555,
    SDRAM_DRV_12mA=0xAAAA,
    SDRAM_DRV_16mA=0xFFFF
} SDRAM_DRV;
#endif

#ifndef _NEG_SAMPLE
#define _NEG_SAMPLE
typedef enum {
    NEG_OFF=0x0,
    NEG_ON=0x2
} NEG_SAMPLE;
#endif


// Power Key
enum {
	PWR_KEY0 = 0x100,
	PWR_KEY1
};

// GPIO
/* Direction Register Input/Output definition */
#define GPIO_INPUT              0       /* IO in input */
#define GPIO_OUTPUT             1       /* IO in output */

/* Attribution Register High/Low definition */
#define ATTRIBUTE_HIGH          1
#define INPUT_NO_RESISTOR       1
#define OUTPUT_UNINVERT_CONTENT 1
#define ATTRIBUTE_LOW           0
#define INPUT_WITH_RESISTOR     0
#define OUTPUT_INVERT_CONTENT   0
#define DATA_HIGH               1
#define DATA_LOW                0
#define BL_ON                   1
#define BL_OFF                  0

#ifndef __GPIO_TYPEDEF__
#define __GPIO_TYPEDEF__

typedef enum {
    IO_A0=0,
    IO_A1 ,
    IO_A2 ,
    IO_A3 ,
    IO_A4 ,
    IO_A5 ,
    IO_A6 ,
    IO_A7 ,
    IO_A8 ,
    IO_A9 ,
    IO_A10,
    IO_A11,
    IO_A12,
    IO_A13,
    IO_A14,
    IO_A15,
    IO_B0 ,
    IO_B1 ,
    IO_B2 ,
    IO_B3 ,
    IO_B4 ,
    IO_B5 ,
    IO_B6 ,
    IO_B7 ,
    IO_B8 ,
    IO_B9 ,
    IO_B10,
    IO_B11,
    IO_B12,
    IO_B13,
    IO_B14,
    IO_B15,
    IO_C0 ,
    IO_C1 ,
    IO_C2 ,
    IO_C3 ,
    IO_C4 ,
    IO_C5 ,
    IO_C6 ,
    IO_C7 ,
    IO_C8 ,
    IO_C9 ,
    IO_C10,
    IO_C11,
    IO_C12,
    IO_C13,
    IO_C14,
    IO_C15,
    IO_D0 ,
    IO_D1 ,
    IO_D2 ,
    IO_D3 ,
    IO_D4 ,
    IO_D5 ,
    IO_D6 ,
    IO_D7 ,
    IO_D8 ,
    IO_D9 ,
    IO_D10,
    IO_D11,
    IO_D12,
    IO_D13,
    IO_D14,
    IO_D15,
    IO_E0 ,
    IO_E1 ,
    IO_E2 ,
    IO_E3 ,
    IO_E4 ,
    IO_E5 ,
    IO_E6 ,
    IO_E7 ,
    IO_E8 ,
    IO_E9 ,
    IO_E10,
    IO_E11,
    IO_E12,
    IO_E13,
    IO_E14,
    IO_E15,
    IO_F0 ,
    IO_F1 ,
    IO_F2 ,
    IO_F3 ,
    IO_F4 ,
    IO_F5 ,
    IO_F6 ,
    IO_F7 ,
    IO_F8 ,
    IO_F9 ,
    IO_F10,
    IO_F11,
    IO_F12,
    IO_F13,
    IO_F14,
    IO_F15,
    IO_G0 ,
    IO_G1 ,
    IO_G2 ,
    IO_G3 ,
    IO_G4 ,
    IO_G5 ,
    IO_G6 ,
    IO_G7 ,
    IO_G8 ,
    IO_G9 ,
    IO_G10,
    IO_G11,
    IO_G12,
    IO_G13,
    IO_G14,
    IO_G15,
    IO_H0 ,
    IO_H1 ,
    IO_H2 ,
    IO_H3 ,
    IO_H4 ,
    IO_H5 ,
    IO_H6 ,
    IO_H7 ,
    IO_H8 ,
    IO_H9 ,
    IO_H10,
    IO_H11,
    IO_H12,
    IO_H13
} GPIO_ENUM;

#endif


#ifndef _GPIO_DRVING_DEF_
#define _GPIO_DRVING_DEF_

typedef enum {
    IOA_DRV_4mA=0x0,
    IOA_DRV_8mA=0x1,
    IOA_DRV_12mA=0x2,
    IOA_DRV_16mA=0x3,
/* IOB Driving Options */
    IOB_DRV_4mA=0x0,
    IOB_DRV_8mA=0x1,
    IOB_DRV_12mA=0x2,
    IOB_DRV_16mA=0x3,
/* IOC Driving Options */
    IOC_DRV_4mA=0x0,
    IOC_DRV_8mA=0x1,
    IOC_DRV_12mA=0x2,
    IOCB_DRV_16mA=0x3,
/* IOD Driving Options */
    IOD_DRV_4mA=0x0,
    IOD_DRV_8mA=0x1,
    IOD_DRV_12mA=0x2,
    IOD_DRV_16mA=0x3,
/* IOE Driving Options */
    IOE_DRV_4mA=0x0,
    IOE_DRV_8mA=0x1,
    IOE_DRV_12mA=0x2,
    IOE_DRV_16mA=0x3
} IO_DRV_LEVEL;


#endif //_GPIO_DRVING_DEF_


extern void gpio_init(void);
extern BOOLEAN gpio_init_io(INT32U port, BOOLEAN direction);
extern BOOLEAN gpio_read_io(INT32U port);
extern BOOLEAN gpio_write_io(INT32U port, BOOLEAN data);
extern BOOLEAN gpio_init_io(INT32U port, BOOLEAN direction);
extern BOOLEAN gpio_set_port_attribute(INT32U port, BOOLEAN attribute);
extern BOOLEAN gpio_drving_init_io(GPIO_ENUM port, IO_DRV_LEVEL gpio_driving_level);
extern void gpio_drving_init(void);
extern void gpio_drving_uninit(void);
extern void gpio_set_ice_en(BOOLEAN status);
extern void gpio_sdram_swith(INT32U port, BOOLEAN data);
extern void gpio_IOE_switch_config_from_HDMI_to_GPIO(void);
// TIMER

#ifndef __TIMER_TYPEDEF__
#define __TIMER_TYPEDEF__

typedef enum
{
	TIMER_SOURCE_256HZ=0,
	TIMER_SOURCE_1024HZ,
	TIMER_SOURCE_2048HZ,
	TIMER_SOURCE_4096HZ,
	TIMER_SOURCE_8192HZ,
	TIMER_SOURCE_32768HZ,
	TIMER_SOURCE_MCLK_DIV_256,
	TIMER_SOURCE_MCLK_DIV_2,
	TIMER_SOURCE_MAX
} TIMER_SOURCE_ENUM;


typedef enum
{
	TMBAS_1HZ=1,
	TMBAS_2HZ,
	TMBAS_4HZ,
	TMBBS_8HZ=0,
	TMBBS_16HZ,
	TMBBS_32HZ,
	TMBBS_64HZ,
	TMBCS_128HZ=0,
	TMBCS_256HZ,
	TMBCS_512HZ,
	TMBCS_1024HZ,
	TMBXS_MAX
} TMBXS_ENUM;

typedef enum
{
	TIMER_A=0,
	TIMER_B,
	TIMER_C,
	TIMER_D,
	TIMER_E,
	TIMER_F,
	TIMER_RTC,
	TIMER_ID_MAX
} TIMER_ID_ENUM;

typedef enum
{
	TIME_BASE_A=0,
	TIME_BASE_B,
	TIME_BASE_C,
	TIME_BASE_ID_MAX
} TIME_BASE_ID_ENUM;

typedef enum
{
	PWM_NRO_OUTPUT=0,
	PWM_NRZ_OUTPUT
} PWMXSEL_ENUM;

#endif  //__TIMER_TYPEDEF__

/* Each Tiny count time is 2.666... us*/
/* Eample TINY_WHILE((reg_flag==0x00000001),0xE000)*/
#define TINY_WHILE(BREAK_CONDITION,TIMEOUT_TINY_COUNT)  \
{\
   INT32S ttt1 = tiny_counter_get();\
   INT32S dt=0;\
   while(!((BREAK_CONDITION) || (dt>TIMEOUT_TINY_COUNT)))\
   {\
        dt = ((tiny_counter_get() | 0x10000) - ttt1) & (0xFFFF);\
   }\
}

#define MINY_WHILE(BREAK_CONDITION,TIMEOUT_MINY_COUNT)  \
{\
   INT32S dtm=0;\
   INT32S dtn = tiny_counter_get();\
   while(!((BREAK_CONDITION) || (dtm>TIMEOUT_MINY_COUNT)))\
   {\
        if ((((tiny_counter_get() | 0x10000) - dtn) & (0xFFFF))>=375)\
        {dtm++;dtn=tiny_counter_get();}\
   }\
}

#ifndef _SW_TIMER_ID
#define _SW_TIMER_ID

typedef enum
{
    SW_TIMER_0=0,
    SW_TIMER_1,
    SW_TIMER_2,
    SW_TIMER_3,
    SW_TIMER_4,
    SW_TIMER_5,
    SW_TIMER_6,
    SW_TIMER_7,
    SW_TIMER_8,
    SW_TIMER_9,
    SW_TIMER_MAX
} SW_TIMER_ID;

#endif


extern INT32S timer_pwm_setup(INT32U timer_id, INT32U freq_hz, INT8U pwm_duty_cycle_percent, PWMXSEL_ENUM NRO_NRZ);
extern void timer_init(void);
extern INT32S timer_freq_setup(INT32U timer_id, INT32U freq_hz, INT32U times_counter, void(* TimerIsrFunction)(void));
extern INT32S dac_timer_freq_setup(INT32U freq_hz);
extern INT32S adc_timer_freq_setup(INT32U timer_id, INT32U freq_hz);
extern INT32S timer_stop(INT32U timer_id);
extern INT32U TimerCountReturn(INT32U Timer_Id);
extern void TimerCountSet(INT32U Timer_Id, INT32U set_count);
extern void time_base_reset(void);
extern INT32S timer_start_without_isr(INT32U timer_id, TIMER_SOURCE_ENUM timer_source);
extern INT32S timer_msec_setup(INT32U timer_id, INT32U msec, INT32U times_counter, void(* TimerIsrFunction)(void));
extern void timer_counter_reset(INT32U timer_id);
extern void timer_init(void);
/* if times_counter == 0 , then isr will repeat witoout limit, until stop the timer*/
extern INT32S timer_freq_setup(INT32U timer_id, INT32U freq_hz, INT32U times_counter, void(* TimerIsrFunction)(void));
extern INT32S timer_msec_setup(INT32U timer_id, INT32U msec, INT32U times_counter,void (*TimerIsrFunction)(void));
extern INT32S dac_timer_freq_setup(INT32U freq_hz);  /* for dac timer (TimerE)*/
extern INT32S timer_stop(INT32U timer_id);  /* only used for timer A,B,C and Timer E and F -> no effect*/
extern INT32S time_base_setup(INT32U time_base_id, TMBXS_ENUM tmbxs_enum_hz, INT32U times_counter, void(* TimeBaseIsrFunction)(void));
extern INT32S time_base_stop(INT32U time_base_id);
extern void sw_timer_ms_delay(INT32U msec);
extern void sw_timer_us_delay(INT32U usec);
extern INT32U sw_timer_init(INT32U TimerId, INT32U freq_hz);
extern INT32U sw_timer_get_counter_L(void);
extern INT32U sw_timer_get_counter_H(void);
extern void  drv_msec_wait(INT32U m_sec) ;
extern INT32S timerD_counter_init(void);
extern INT32S tiny_counter_get(void);

// SPI
typedef enum
{
    SPI_LBM_NORMAL, /* no loop back */
    SPI_LBM_LOOP_BACK
} SPI_LOOP_BACK_MODE;

typedef enum
{
    SPI_0,
    SPI_1,
    SPI_MAX_NUMS
} SPI_CHANNEL;

/* master mode clock selection */
typedef enum
{
    SYSCLK_2,   /* SYSCLK/2 */
    SYSCLK_4,
    SYSCLK_8,
    SYSCLK_16,
    SYSCLK_32,
    SYSCLK_64,
    SYSCLK_128
} SPI_CLK;

typedef enum
{
    PHA0_POL0,
    PHA0_POL1,
    PHA1_POL0,
    PHA1_POL1
} SPI_CLK_PHA_POL;

extern void spi_init(INT8U spi_num);
extern void spi_lbm_set(INT8U spi_num, INT8S status);
extern INT32S spi_clk_set(INT8U spi_num, INT8S spi_clk);
extern INT32S spi_transceive(INT8U spi_num, INT8U *tx_data, INT32U tx_len, INT8U *rx_data, INT32U rx_len);
extern INT32S spi_disable(INT8U spi_num);
extern INT32S spi_pha_pol_set(INT8U spi_num, INT8U pha_pol);
extern void spi_cs_by_internal_set(INT8U spi_num,INT8U pin_num, INT8U active);
extern void spi_cs_by_external_set(INT8U spi_num);
extern INT32S spi_transceive_cpu(INT8U spi_num, INT8U *tx_data, INT32U tx_len, INT8U *rx_data, INT32U rx_len);
extern INT32S spi_clk_rate_set(INT8U spi_num, INT32U clk_rate);
// RTC
typedef struct __rtc
{
	INT32U		rtc_sec;    /* seconds [0,59]  */
	INT32U		rtc_min;    /* minutes [0,59]  */
	INT32U		rtc_hour;   /* hours [0,23]  */
	INT32U      rtc_day;    /* gpy0200 day count,[ 0,4095] */
} t_rtc;

typedef enum
{
	RTC_ALM_INT_INDEX,
	RTC_SCH_INT_INDEX,
	RTC_HR_INT_INDEX,
	RTC_MIN_INT_INDEX,
	RTC_SEC_INT_INDEX,
	RTC_HSEC_INT_INDEX,
	RTC_DAY_INT_INDEX,
	EXT_RTC_SEC_INT_INDEX,
	EXT_RTC_ALARM_INT_INDEX,	
	EXT_RTC_WAKEUP_INT_INDEX,	
	RTC_INT_MAX
} RTC_INT_INDEX;

typedef enum
{
	RTC_SCH_16HZ,
	RTC_SCH_32HZ,
	RTC_SCH_64HZ,
	RTC_SCH_128HZ,
	RTC_SCH_256HZ,
	RTC_SCH_512HZ,
	RTC_SCH_1024HZ,
	RTC_SCH_2048HZ
} RTC_SCH_PERIOD;

#define RTC_ALMEN     (1 << 10)  /* alarm function enable */
#define RTC_HMSEN     (1 <<  9)  /* H/M/S function enable */
#define RTC_SCHEN     (1 <<  8)  /* scheduler function enbale */

#define RTC_ALM_IEN       (1 << 10)  /* alarm interrupt enbale */
#define RTC_SCH_IEN       (1 <<  8)  /* scheduler interrupt enbale */
#define RTC_DAY_IEN      (1 <<  4)  /* hour interrupt enbale */
#define RTC_HR_IEN        (1 <<  3)  /* hour interrupt enbale */
#define RTC_MIN_IEN       (1 <<  2)  /* min interrupt enbale */
#define RTC_SEC_IEN       (1 <<  1)  /* alarm interrupt enbale */
#define RTC_HALF_SEC_IEN  (1 <<  0)  /* alarm interrupt enbale */

#define RTC_EN     0xFFFFFFFF
#define RTC_DIS    0

#define RTC_EN_MASK     0xFF

extern void rtc_init(void);
extern INT32S rtc_callback_set(INT8U int_idx, void (*user_isr)(void));
extern INT32S rtc_callback_clear(INT8U int_idx);
extern void rtc_alarm_set(t_rtc *rtc_time);
extern void rtc_time_get(t_rtc *rtc_time);
extern void rtc_time_set(t_rtc *rtc_time);
extern void rtc_alarm_get(t_rtc *rtc_time);

//extern void rtc_function_set(INT8U mask, INT8U value);
extern void rtc_int_set(INT8U mask, INT8U value);
extern void rtc_schedule_enable(INT8U freq);
extern void rtc_schedule_disable(void);
extern void rtc_day_get(t_rtc *rtc_time);
//extern void rtc_day_set(t_rtc *rtc_time);
extern void rtc_ext_to_int_set(void);

//#define old_RTC_driver

//#ifdef old_RTC_driver
//extern void gpx_rtc_write(INT8U addr,INT8U data);
//#else
//extern INT32S gpx_rtc_write(INT8U addr,INT8U data);
//#endif

#define GPX_RTC_ALMOEN    (1 << 0)  /* alarm output singnal enable */
#define GPX_RTC_EN        (1 << 3)  /* RTC function enable */
#define GPX_RTC_VAEN      (1 << 4)  /* Voltage comparator enbale */
#define GPX_RTC_PWREN     (1 << 5)  /* set alarm output signal to high */

#define GPX_RTC_ALM_IEN       (1 <<  4)  /* alarm interrupt enbale */
#define GPX_RTC_DAY_IEN       (1 <<  5)  /* hour interrupt enbale */
#define GPX_RTC_HR_IEN        (1 <<  3)  /* hour interrupt enbale */
#define GPX_RTC_MIN_IEN       (1 <<  2)  /* min interrupt enbale */
#define GPX_RTC_SEC_IEN       (1 <<  1)  /* alarm interrupt enbale */
#define GPX_RTC_HALF_SEC_IEN  (1 <<  0)  /* alarm interrupt enbale */

//MATRE KEYSCAN
//extern void matre_keyscaninit(void);
//extern void matre_scaninit(void);

// DAC
extern void dac_init(void);
extern void dac_enable_set(BOOLEAN status);
extern void dac_disable(void);
extern void dac_cha_data_put(INT16S *data,INT32U len, INT8S *notify);
extern void dac_chb_data_put(INT16S *data,INT32U len, INT8S *notify);
extern INT32S dac_cha_data_dma_put(INT16S *data,INT32U len, INT8S *notify);
extern INT32S dac_chb_data_dma_put(INT16S *data,INT32U len, INT8S *notify);
#if _OPERATING_SYSTEM == 1
extern INT32S dac_cha_dbf_put(INT16S *data,INT32U len, OS_EVENT *os_q);
extern INT32S dac_cha_dbf_set(INT16S *data,INT32U len);
extern void dac_dbf_free(void);
extern INT32S dac_dbf_status_get(void);
extern INT32S dac_dma_status_get(void);
#endif
extern void dac_fifo_level_set(INT8U cha_level,INT8U chb_level);
extern void dac_sample_rate_set(INT32U hz);
extern void dac_mono_set(void);
extern void dac_stereo_set(void);
extern void dac_stereo_indi_buffer_set(void);
extern void dac_timer_stop(void);
extern void dac_pga_set(INT8U gain);
extern void dac_vref_set(BOOLEAN status);
extern INT8U dac_pga_get(void);

//ADC
/*
#define ADC_AS_TIMER_C     0
#define ADC_AS_TIMER_D     1
#define ADC_AS_TIMER_E     2
#define ADC_AS_TIMER_F     3
*/
#define ADC_AS_TIMER_A     0
#define ADC_AS_TIMER_B     1
#define ADC_AS_TIMER_C     2
#define ADC_AS_TIMER_D     3
#define ADC_AS_TIMER_E     4
#define ADC_AS_TIMER_F     5

#define ADC_LINE_0         0
#define ADC_LINE_1         1
#define ADC_LINE_2         2
#define ADC_LINE_3         3
#define ADC_LINE_4         4
#define ADC_LINE_5         5

extern void adc_init(void);
extern void adc_fifo_level_set(INT8U level);
extern void adc_auto_ch_set(INT8U ch);
extern void adc_manual_ch_set(INT8U ch);
extern void adc_manual_callback_set(void (*user_isr)(INT16U data));
extern INT32S adc_manual_sample_start(void);
extern INT32S adc_auto_sample_start(void);
extern INT32S adc_auto_data_dma_get(INT16U *data, INT32U len, INT8S *notify);
extern INT32S adc_auto_data_get(INT16U *data, INT32U len, INT8S *notify);
extern void   adc_auto_sample_stop(void);
extern INT32S adc_sample_rate_set(INT8U timer_id, INT32U hz);
extern INT32S adc_timer_stop(INT8U timer_id);
extern void adc_fifo_clear(void);
extern INT32S adc_user_line_in_en(INT8U line_id,BOOLEAN status);
extern void tp_callback_set(void (*user_isr)(void));
extern void adc_tp_en(BOOLEAN status);
extern void adc_tp_int_en(BOOLEAN status);
extern void adc_vref_enable_set(BOOLEAN status);
extern int adc_conv_time_sel(unsigned int value);

extern void i2s_adc_init(int mode);
extern void i2s_rx_init(int mode);
extern void i2s_rx_exit(void);
extern INT32S i2s_rx_sample_rate_set(INT32U hz);
extern INT32S i2s_rx_mono_ch_set(void);
extern void i2s_rx_fifo_clear(void);
extern INT32S i2s_rx_start(void);
extern INT32S i2s_rx_stop(INT32U status);


//CMOS SENSOR
extern void CSI_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag, INT32U uFrmBuf0, INT32U uFrmBuf1);
extern void sccb_init (INT32U nSCL, INT32U nSDA);
extern void Sensor_Bluescreen_Enable(void);
extern void Sensor_Cut_Enable(void);
#if C_MOTION_DETECTION == CUSTOM_ON
	extern void Sensor_MotionDection_Inital(INT32U buff);
	extern void Sensor_MotionDection_stop(void);
	extern void Sensor_MotionDection_start(void);
#endif

//TFT
#define TFT_ENABLE    0xFFFFFFFF
#define TFT_DIS       0

#define TFT_CLK_DIVIDE_1        0x0
#define TFT_CLK_DIVIDE_2        0x2
#define TFT_CLK_DIVIDE_3        0x4
#define TFT_CLK_DIVIDE_4        0x6
#define TFT_CLK_DIVIDE_5        0x8
#define TFT_CLK_DIVIDE_6        0xA
#define TFT_CLK_DIVIDE_7        0xC
#define TFT_CLK_DIVIDE_8        0xE
#define TFT_CLK_DIVIDE_9        0x10
#define TFT_CLK_DIVIDE_10       0x12
#define TFT_CLK_DIVIDE_11       0x14
#define TFT_CLK_DIVIDE_12       0x16
#define TFT_CLK_DIVIDE_13       0x18
#define TFT_CLK_DIVIDE_14       0x1A
#define TFT_CLK_DIVIDE_15       0x1C
#define TFT_CLK_DIVIDE_16       0x1E
#define TFT_CLK_DIVIDE_17       0x20
#define TFT_CLK_DIVIDE_18       0x22
#define TFT_CLK_DIVIDE_19       0x24
#define TFT_CLK_DIVIDE_20       0x26
#define TFT_CLK_DIVIDE_21       0x28
#define TFT_CLK_DIVIDE_22       0x2A
#define TFT_CLK_DIVIDE_23       0x2C
#define TFT_CLK_DIVIDE_24       0x2E
#define TFT_CLK_DIVIDE_25       0x30
#define TFT_CLK_DIVIDE_26       0x32
#define TFT_CLK_DIVIDE_27       0x34
#define TFT_CLK_DIVIDE_28       0x36
#define TFT_CLK_DIVIDE_29       0x38
#define TFT_CLK_DIVIDE_30       0x3A
#define TFT_CLK_DIVIDE_31       0x3C
#define TFT_CLK_DIVIDE_32       0x3E

extern void tft_init(void);
extern void tft_clk_set(INT32U clk);
extern void tft_slide_en_set(BOOLEAN status);
extern void tft_tft_en_set(BOOLEAN status);
extern void tft_backlight_en_set(BOOLEAN status);
extern void AP_TFT_ClK_144M_set(void);
extern void AP_TFT_ClK_48M_set(void);

//CMOS SENSOR
// Select CMOS Sensor Type
#define		__TV_VGA__
//#define	__TV_QVGA__

#ifdef	__OV6680_DRV_C__
#define	SLAVE_ID				0xC0
#endif

#ifdef	__OV7680_DRV_C__
#define SLAVE_ID				0x42
#endif

#ifdef	__OV7670_DRV_C__
#define SLAVE_ID				0x42
#endif

#ifdef	__OV7725_DRV_C__
#define SLAVE_ID				0x42
#endif

#ifdef	__OV7675_DRV_C__
#define SLAVE_ID				0x42
#endif

#ifdef	__GC0308_DRV_C__
#define SLAVE_ID				0x42
#endif

#ifdef	__SP0838_DRV_C__
#define SLAVE_ID				0x30
#endif

#ifdef	__OV9655_DRV_C__
#define SLAVE_ID				0x60
#endif

#ifdef	__OV2655_DRV_C__
#define SLAVE_ID				0x60
#endif

#ifdef	__OV3640_DRV_C__
#define SLAVE_ID				0x78
#endif

#ifdef	__OV5642_DRV_C__
#define SLAVE_ID				0x78
#endif

#ifdef __OV2643_DRV_C__
#define SLAVE_ID				0x60
#endif

// CSI Flag Type Definitions
#define	FT_CSI_CCIR656			(1<<0)
#define	FT_CSI_YUVIN			(1<<1)
#define	FT_CSI_YUVOUT			(1<<2)
#define FT_CSI_RGB1555			(1<<3)

extern void CSI_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag, INT32U uFrmBuf0, INT32U uFrmBuf1);
extern void Sensor_Bluescreen_Enable(void);
extern void Sensor_Cut_Enable(void);

extern void sccb_init (INT32U nSCL, INT32U nSDA);
extern void sccb_write (INT16U id, INT16U addr, INT16U data);
extern INT16U sccb_read (INT16U id,INT16U addr);
extern void sccb_start (void);
extern void sccb_stop (void);
extern void sccb_w_phase (INT16U value);
extern INT16U sccb_r_phase (void);

// JPEG
#define C_JPG_CTRL_YUV444				0x00000000
#define C_JPG_CTRL_YUV422				0x00000010
#define C_JPG_CTRL_YUV420				0x00000020
#define C_JPG_CTRL_YUV411				0x00000030
#define C_JPG_CTRL_GRAYSCALE			0x00000040
#define C_JPG_CTRL_YUV422V				0x00000050
#define C_JPG_CTRL_YUV411V				0x00000060
#define C_JPG_CTRL_YUV420H2				0x00000120
#define C_JPG_CTRL_YUV420V2				0x00000220
#define C_JPG_CTRL_YUV411H2				0x00000130
#define C_JPG_CTRL_YUV411V2				0x00000260

#define C_JPG_FIFO_DISABLE				0x00000000
#define C_JPG_FIFO_16LINE				0x00000009
#define C_JPG_FIFO_32LINE				0x00000001
#define C_JPG_FIFO_64LINE				0x00000003
#define C_JPG_FIFO_128LINE				0x00000005
#define C_JPG_FIFO_256LINE				0x00000007

#define C_JPG_STATUS_DECODING			0x00000001
#define C_JPG_STATUS_ENCODING			0x00000002
#define C_JPG_STATUS_INPUT_EMPTY		0x00000004
#define C_JPG_STATUS_OUTPUT_FULL		0x00000008
#define C_JPG_STATUS_DECODE_DONE		0x00000010
#define C_JPG_STATUS_ENCODE_DONE		0x00000020
#define C_JPG_STATUS_STOP				0x00000040
#define C_JPG_STATUS_TIMEOUT			0x00000080
#define C_JPG_STATUS_INIT_ERR			0x00000100
#define C_JPG_STATUS_RST_VLC_DONE		0x00000200
#define C_JPG_STATUS_RST_MARKER_ERR		0x00000400
#define C_JPG_STATUS_SCALER_DONE		0x00008000


typedef enum {
	ENUM_JPG_NO_SCALE_DOWN = 0x0,			// No scale-down
	ENUM_JPG_DIV2,							// Sampling mode=YUV422/YUV422V/YUV420/YUV444/YUV411. Output format=YUYV
	ENUM_JPG_DIV4,							// Sampling mode=YUV422/YUV422V/YUV420/YUV444/YUV411. Output format=YUYV
	ENUM_JPG_H_DIV23,						// Sampling mode=YUV420. Output format=GP420
	ENUM_JPG_V_DIV23,						// Sampling mode=YUV420. Output format=GP420
	ENUM_JPG_HV_DIV23						// Sampling mode=YUV420. Output format=GP420
} JPEG_SCALE_DOWN_MODE_ENUM;

// JPEG init API
extern void jpeg_init(void);

// JPEG APIs for setting Quantization table
extern INT32S jpeg_quant_luminance_set(INT32U offset, INT32U len, INT16U *val);
extern INT32S jpeg_quant_chrominance_set(INT32U offset, INT32U len, INT16U *val);

// JPEG APIs for setting Huffman table
extern INT32S jpeg_huffman_dc_lum_mincode_set(INT32U offset, INT32U len, INT16U *val);
extern INT32S jpeg_huffman_dc_lum_valptr_set(INT32U offset, INT32U len, INT8U *val);
extern INT32S jpeg_huffman_dc_lum_huffval_set(INT32U offset, INT32U len, INT8U *val);
extern INT32S jpeg_huffman_dc_chrom_mincode_set(INT32U offset, INT32U len, INT16U *val);
extern INT32S jpeg_huffman_dc_chrom_valptr_set(INT32U offset, INT32U len, INT8U *val);
extern INT32S jpeg_huffman_dc_chrom_huffval_set(INT32U offset, INT32U len, INT8U *val);
extern INT32S jpeg_huffman_ac_lum_mincode_set(INT32U offset, INT32U len, INT16U *val);
extern INT32S jpeg_huffman_ac_lum_valptr_set(INT32U offset, INT32U len, INT8U *val);
extern INT32S jpeg_huffman_ac_lum_huffval_set(INT32U offset, INT32U len, INT8U *val);
extern INT32S jpeg_huffman_ac_chrom_mincode_set(INT32U offset, INT32U len, INT16U *val);
extern INT32S jpeg_huffman_ac_chrom_valptr_set(INT32U offset, INT32U len, INT8U *val);
extern INT32S jpeg_huffman_ac_chrom_huffval_set(INT32U offset, INT32U len, INT8U *val);

// JPEG APIs for setting image relative parameters
extern INT32S jpeg_restart_interval_set(INT16U interval);
extern INT32S jpeg_image_size_set(INT16U hsize, INT16U vsize);		// hsize<=8000, vsize<=8000, Decompression: hsize must be multiple of 16, vsize must be at least multiple of 8
extern INT32S jpeg_yuv_sampling_mode_set(INT32U mode);				// mode=C_JPG_CTRL_YUV422/C_JPG_CTRL_YUV420/C_JPG_CTRL_YUV444/C_JPG_CTRL_YUV411/C_JPG_CTRL_GRAYSCALE/C_JPG_CTRL_YUV422V/C_JPG_CTRL_YUV411V/C_JPG_CTRL_YUV420H2/C_JPG_CTRL_YUV420V2/C_JPG_CTRL_YUV411H2/C_JPG_CTRL_YUV411V2
extern INT32S jpeg_clipping_mode_enable(void);						// Decompression: When clipping mode is enabled, JPEG will output image data according to jpeg_image_clipping_range_set()
extern INT32S jpeg_clipping_mode_disable(void);
extern INT32S jpeg_image_clipping_range_set(INT16U start_x, INT16U start_y, INT16U width, INT16U height);	// x, y, width, height must be at lease 8-byte alignment
extern INT32S jpeg_decode_scale_down_set(JPEG_SCALE_DOWN_MODE_ENUM mode);

// JPEG APIs for setting Y/Cb/Cr or YUYV data
extern INT32S jpeg_yuv_addr_set(INT32U y_addr, INT32U u_addr, INT32U v_addr);	// Compression: input addresses(8-byte alignment for normal compression, 16-bytes alignment for YUYV data compression); Decompression: output addresses(8-byte alignment)
extern INT32S jpeg_multi_yuv_input_init(INT32U len);				// Compression: When YUYV mode is used, it should be the length of first YUYV data that will be compressed. It should be sum of Y,U and V data when YUV separate mode is used

// JPEG APIs for setting entropy encoded data address and length
extern INT32S jpeg_vlc_addr_set(INT32U addr);						// Compression: output VLC address, addr must be 16-byte alignment. Decompression: input VLC address
extern INT32S jpeg_vlc_maximum_length_set(INT32U len);				// Decompression: JPEG engine stops when maximum VLC bytes are read(Maximum=0x03FFFFFF)
extern INT32S jpeg_multi_vlc_input_init(INT32U len);				// Decompression: length(Maximum=0x000FFFFF) of first VLC buffer

// JPEG APIs for setting output FIFO line
extern INT32S jpeg_fifo_line_set(INT32U line);						// Decompression: FIFO line number(C_JPG_FIFO_DISABLE/C_JPG_FIFO_ENABLE/C_JPG_FIFO_16LINE/C_JPG_FIFO_32LINE/C_JPG_FIFO_64LINE/C_JPG_FIFO_128LINE/C_JPG_FIFO_256LINE)

// JPEG start, restart and stop function APIs
extern INT32S jpeg_compression_start(void (*callback)(INT32S event, INT32U count));
extern INT32S jpeg_multi_yuv_input_restart(INT32U y_addr, INT32U u_addr, INT32U v_addr, INT32U len);	// Compression: Second, third, ... YUV addresses(8-byte alignment for normal compression, 16-bytes alignment for YUYV data compression) and lengths(Maximum=0x000FFFFF)
extern INT32S jpeg_vlc_output_restart(INT32U addr, INT32U len);

extern INT32S jpeg_decompression_start(void (*callback)(INT32S event, INT32U count));
extern INT32S jpeg_multi_vlc_input_restart(INT32U addr, INT32U len);// Decompression: Second, third, ... VLC addresses(16-byte alignment) and lengths(Maximum=0x000FFFFF)
extern INT32S jpeg_multi_vlc_input_output_restart(INT32U addr, INT32U len);// Decompression: Second, third, ... VLC addresses(16-byte alignment) and lengths(Maximum=0x000FFFFF) && Decompression: Restart JPEG engine when FIFO is full
extern INT32S jpeg_yuv_output_restart(void);						// Decompression: Restart JPEG engine when FIFO is full
extern void jpeg_stop(void);

// JPEG status polling API
extern INT32S jpeg_device_protect(void);
extern void jpeg_device_unprotect(INT32S mask);
extern INT32S jpeg_status_polling(INT32U wait);						// If wait is 0, this function returns immediately. Return value:C_JPG_STATUS_INPUT_EMPTY/C_JPG_STATUS_OUTPUT_FULL/C_JPG_STATUS_DECODE_DONE/C_JPG_STATUS_ENCODE_DONE/C_JPG_STATUS_STOP/C_JPG_STATUS_TIMEOUT/C_JPG_STATUS_INIT_ERR
extern INT32U jpeg_compress_vlc_cnt_get(void);						// This API returns the total bytes of VLC data stream that JPEG compression has been generated


extern INT32S jpeg_scaler_up_x1_5(void); // 1.5­¿
extern INT32S jpeg_level2_scaledown_mode_enable(void);
extern INT32S jpeg_level2_scaledown_mode_disable(void);

#if _DRV_L1_GTE == 1
//division
#define R_GTE_DIVA					(*((volatile INT32U *) 0xF60040A0))
#define R_GTE_DIVB					(*((volatile INT32U *) 0xF60040A4))
#define R_GTE_DIVOF					(*((volatile INT32U *) 0xF60040A8))
#define R_GTE_DIVO					(*((volatile INT32U *) 0xF60040AC))
#define R_GTE_DIVR					(*((volatile INT32U *) 0xF60040B0))

#define INT16U_DIVISION(dividend, divisor, quotient, residue) \
{\
  	R_GTE_DIVOF = 7;\
  	R_GTE_DIVA = (INT32U) (dividend);\
  	R_GTE_DIVB = (INT32U) (divisor);\
    (quotient) = (INT16U) R_GTE_DIVO;\
    (residue) = (INT16U) R_GTE_DIVR;\
}

#define INT16S_DIVISION(dividend, divisor, quotient, residue) \
{\
  	R_GTE_DIVOF = 6;\
  	R_GTE_DIVA = (INT32S) (dividend);\
  	R_GTE_DIVB = (INT32S) (divisor);\
    (quotient) = (INT16S) R_GTE_DIVO;\
    (residue) = (INT16S) R_GTE_DIVR;\
}

#define INT32U_DIVISION(dividend, divisor, quotient, residue) \
{\
  	R_GTE_DIVOF = 7;\
  	R_GTE_DIVA = (INT32U) (dividend);\
  	R_GTE_DIVB = (INT32U) (divisor);\
    (quotient) = R_GTE_DIVO;\
    (residue) = R_GTE_DIVR;\
}

#define INT32S_DIVISION(dividend, divisor, quotient, residue) \
{\
  	R_GTE_DIVOF = 6;\
  	R_GTE_DIVA = (INT32S) (dividend);\
  	R_GTE_DIVB = (INT32S) (divisor);\
    (quotient) = (INT32S) R_GTE_DIVO;\
    (residue) = (INT32S) R_GTE_DIVR;\
}
#endif

#define	R_RANDOM0					(*((volatile INT32U *) 0xD0500380))
#define	R_RANDOM1					(*((volatile INT32U *) 0xD0500384))

#define GET_RANDOM0()				R_RANDOM0
#define GET_RANDOM1()				R_RANDOM1


// WatchDog
extern void watchdog_init(void);
extern INT32U watchdog_ctrl(INT32U WDGPD, INT32U WDGS, INT32U WDGEN);
extern INT32U watchdog_clear(void);

//====================================================================================================
//	TV Export Functions - Begin
//====================================================================================================
extern void tv_init(void);
extern void tv_start(INT32S nTvStd, INT32S nResolution, INT32S nNonInterlace);
extern void tv_disable(void);

//  nTvStd
#define TVSTD_NTSC_M                    0
#define TVSTD_NTSC_J                    1
#define TVSTD_NTSC_N                    2
#define TVSTD_PAL_M                     3
#define TVSTD_PAL_B                     4
#define TVSTD_PAL_N                     5
#define TVSTD_PAL_NC                    6
#define TVSTD_NTSC_J_NONINTL            7
#define TVSTD_PAL_B_NONINTL             8
//  nResolution
#define TV_QVGA                         0
#define TV_HVGA                         1
#define TV_D1                           2
//  nNonInterlace
#define TV_INTERLACE                    0
#define TV_NON_INTERLACE                1
//====================================================================================================
//	TV Export Functions - End
//====================================================================================================

//----------------------
#define USE_BYPASS_SCALER1_PATH		0
//----------------------



//USB
extern void usb_uninitial(void);


//external interrupt driver
typedef enum
{
	EXTA,
	EXTB,
	EXTC
}EXTAB_ENUM;

typedef enum
{
	FALLING,
	RISING
}EXTAB_EDGE_ENUM;

extern void ext_int_init(void);
extern void extab_int_clr(INT8U ext_src);
extern void extab_edge_set(INT8U ext_src, INT8U edge_type);
extern void extab_enable_set(INT8U ext_src, BOOLEAN status);
extern void extab_user_isr_set(INT8U ext_src,void (*user_isr)(void));
extern void extab_user_isr_clr(INT8U ext_src);

// drv_l1_tool.c extern
extern INT32S dma_buffer_copy(INT32U s_addr, INT32U t_addr, INT32U byte_count, INT32U s_width, INT32U t_width);
extern INT32S mem_transfer_dma(INT32U src, INT32U dest, INT32U len);

extern void system_clk_ext_XLAT_12M(void);
extern void setting_by_iRAM(void);


// drv_l1_peripheral.c extern
extern void ap_peripheral_hdmi_detect(void);
extern void ap_peripheral_clr_screen_saver_timer(void);

// drv_l1_hdmi.c extern
extern int drvl1_hdmi_init(unsigned int DISPLAY_MODE, unsigned int AUD_FREQ);
extern int drvl1_hdmi_exit(void);
extern void drvl1_hdmi_set_time_cycle(void *RegBase, unsigned int VBack, unsigned int HBlank);
extern void drvl1_hdmi_set_audio_sample_packet(void *RegBase, unsigned int ch);
extern void drvl1_hdmi_config_phy(unsigned int phy1, unsigned int phy2);
extern void drvl1_hdmi_set_acr_packet(void *RegBase, unsigned int N, unsigned int CTS);
extern void drvl1_hdmi_set_general_ctrl_packet(void *RegBase);
extern void drvl1_hdmi_send_packet(void *RegBase, unsigned int ch, void *data, unsigned int blank, unsigned int sendMode);
extern int  drvl1_hdmi_audio_ctrl(unsigned int status);
extern int  drvl1_hdmi_dac_mute(unsigned int status);



#endif 		// __DRIVER_L1_H__

