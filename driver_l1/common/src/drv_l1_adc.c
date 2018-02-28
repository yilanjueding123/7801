/*
* Purpose: ADC driver/interface
*
* Author: allenlin
*
* Date: 2008/5/9
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 1.00
* History :
*/

//Include files
#include "drv_l1_adc.h"
#include "drv_l1_mic.h"

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#if (defined _DRV_L1_ADC) && (_DRV_L1_ADC == 1)                   //
//================================================================//

// Constant definitions used in this file only go here

// Type definitions used in this file only go here

// Global inline functions(Marcos) used in this file only go here

// Declaration of global variables and functions defined in other files

// Variables defined in this file
ADC_CTRL adc_ctrl;
void (*adc_user_isr)(INT16U data);
void (*tp_user_isr)(void);
// Functions defined in this file
static void adc_auto_isr(void);
static void adc_manual_isr(void);
void tp_callback_set(void (*user_isr)(void));
void adc_tp_en(BOOLEAN status);
void adc_tp_int_en(BOOLEAN status);

void adc_init(void)
{
	R_ADC_SETUP  = 0x4000;//(GPL32700B)  AD Bias Reference Voltage Enable 
	R_ADC_ASADC_CTRL = 0;
	R_ADC_MADC_CTRL  = 0;
	R_ADC_TP_CTRL    = 0;

		
	R_ADC_USELINEIN = 0x2;//(GPL32700B)//for  2 of AD pad
							//bit0  enable line0
							//bit1  enable line1
							//bit2	enable line2
	R_ADC_PGA_GAIN |= 0x300;
	adc_user_isr = 0;
	
	gp_memset((INT8S *)&adc_ctrl, 0, sizeof(ADC_CTRL)); 
	
	vic_irq_register(VIC_ADCF, adc_auto_isr);	/* register auto smaple isr */
	vic_irq_register(VIC_ADC, adc_manual_isr);	/* register manual smaple isr */
	vic_irq_enable(VIC_ADCF);
	vic_irq_enable(VIC_ADC);
}	

static void adc_auto_isr()
{
    if((R_ADC_ASADC_CTRL & ADC_ASIF) && (R_ADC_ASADC_CTRL & ADC_ASIEN)){                       
	while(((R_ADC_ASADC_CTRL & 0xF) != 0) && (adc_ctrl.count < adc_ctrl.data_len)) {
			adc_ctrl.adc_data[adc_ctrl.count++] = R_ADC_ASADC_DATA;
	}
	R_ADC_ASADC_CTRL |= ADC_ASIF; /* clear flag must after getting data */
	}
	if((R_MIC_ASADC_CTRL & MIC_ASIF) && (R_MIC_ASADC_CTRL & MIC_ASIEN)){                          //add by duxiatang,2009-12-09
	    while(((R_MIC_ASADC_CTRL & 0xF) != 0) && (mic_ctrl.count < mic_ctrl.data_len)) {
			mic_ctrl.mic_data[mic_ctrl.count++] = R_MIC_ASADC_DATA;
		}
		R_MIC_ASADC_CTRL |= MIC_ASIF; /* clear flag must after getting data */	
	}                                                                                             
	
	if (adc_ctrl.count >= adc_ctrl.data_len) {
		adc_auto_int_set(FALSE); /* disable fifo interrupt */
		*adc_ctrl.notify = 1;
	}	
	if (mic_ctrl.count >= mic_ctrl.data_len) {
		mic_auto_int_set(FALSE); /* disable fifo interrupt */
		*mic_ctrl.notify = 1;
	}
}

static void adc_manual_isr()
{
	
	if (R_ADC_TP_CTRL & ADC_TP_IF) {
		R_ADC_TP_CTRL |= ADC_TP_IF;
        (*tp_user_isr)();
	}	
	
	if (R_ADC_MADC_CTRL & ADC_ADCRIF) {
	    R_ADC_MADC_CTRL |= ADC_ADCRIF;
	    (*adc_user_isr)((INT16U)R_ADC_MADC_DATA);
	    R_ADC_MADC_CTRL &= ~ADC_RIEN; /* disable manual interrupt */		
        }
}

void adc_vref_enable_set(BOOLEAN status)
{
	if (status == TRUE) {
		R_ADC_SETUP |= ADC_ADBEN; /* AD Bias Reference Voltage Enable */
	}
		
	else {
		R_ADC_SETUP &= ~ADC_ADBEN; /* AD Bias Reference Voltage Disable */
	}
}

int adc_conv_time_sel(unsigned int value)
{
	int ret = 0;

	if (value > 5)
		ret = -1;
	else {
		R_ADC_SETUP &= ~0x700;
		R_ADC_SETUP |= (value<<8);
	}
	
	return ret;
}

void adc_fifo_level_set(INT8U level)
{
	R_ADC_ASADC_CTRL &= ~ADC_FIFO_LEVEL;
	R_ADC_ASADC_CTRL |= (level << 4); 
}

void adc_auto_ch_set(INT8U ch)
{
	R_ADC_SETUP &= ~(0x7<<4);
	R_ADC_SETUP |= (ch << 4);
}

void adc_manual_ch_set(INT8U ch)
{
	R_ADC_MADC_CTRL &= ~ADC_MANUAL_CH_SEL;
	R_ADC_MADC_CTRL |= ch;
}

void adc_auto_int_set(BOOLEAN status)
{
	if (status == TRUE) {
		R_ADC_ASADC_CTRL |= ADC_AUTO_ASIEN;
	}
	else {
		R_ADC_ASADC_CTRL &= ~ADC_AUTO_ASIEN;
	}
}

void adc_manual_callback_set(void (*user_isr)(INT16U data))
{
	if (user_isr != 0) {
		adc_user_isr = user_isr;
	}
}

INT32S adc_manual_sample_start(void)
{	
	R_ADC_MADC_CTRL |= ADC_RIEN;  /* enable manual interrupt */
	R_ADC_MADC_CTRL |= ADC_MIASE; /* clear error flag */
	R_ADC_MADC_CTRL |= ADC_ASIME; /* clear error flag */
	
	R_ADC_MADC_CTRL |= ADC_STRCNV; /* start manual sample */
#if 0
	while(1) {
		R_ADC_MADC_CTRL |= ADC_STRCNV; /* start manual sample */
		while(!(R_ADC_MADC_CTRL & ADC_ADCRIF)); /* wait ready */
		R_ADC_MADC_CTRL |= ADC_ADCRIF;
		*data = R_ADC_MADC_DATA;
		if ((!(R_ADC_MADC_CTRL & ADC_MIASE)) && (!(R_ADC_MADC_CTRL & ADC_ASIME))) {
			break;
		}
		R_ADC_MADC_CTRL |= ADC_MIASE; /* clear error flag */
		R_ADC_MADC_CTRL |= ADC_ASIME; /* clear error flag */
	}
	//R_ADC_MADC_CTRL &= ~ADC_RIEN; /* disable manual interrupt */
#endif	
	return STATUS_OK;
}

INT32S adc_auto_sample_start(void)
{
	R_ADC_SETUP |= ADC_ADBEN; /* AD Bias Reference Voltage Enable */
	R_ADC_ASADC_CTRL |= ADC_ASIF; /* clear fifo interrupt flag */
	R_ADC_ASADC_CTRL |= ADC_ASADC_DMA; /* DMA mode enable */
	R_ADC_SETUP |= ADC_ASEN;
	R_ADC_SETUP |= ADC_ASMEN; /* start auto sample */
	return STATUS_OK;
}

INT32S adc_auto_data_dma_get(INT16U *data, INT32U len, INT8S *notify)
{
	DMA_STRUCT  dma_struct;
	INT32S      status;
	
	if (len > ADC_BLOCK_LEN) {
		return ADC_ERR_WRONG_LEN;
	}
	
	*notify = C_DMA_STATUS_WAITING;
	dma_struct.s_addr = (INT32U) P_ADC_ASADC_DATA;
	dma_struct.t_addr = (INT32U) data;
	dma_struct.width = DMA_DATA_WIDTH_2BYTE;		// DMA_DATA_WIDTH_1BYTE or DMA_DATA_WIDTH_2BYTE or DMA_DATA_WIDTH_4BYTE
	dma_struct.count = (INT32U) len;
	dma_struct.notify = notify;
	dma_struct.timeout = 0;
	
	status = dma_transfer(&dma_struct);
	if ( status != 0)
		return status;
		
	return STATUS_OK;	
}

INT32S adc_auto_data_get(INT16U *data, INT32U len, INT8S *notify)
{
	if (len > ADC_BLOCK_LEN) {
		return ADC_ERR_WRONG_LEN;
	}
	R_ADC_ASADC_CTRL &= ~ADC_ASADC_DMA; /* DMA mode disable */
	
	adc_ctrl.adc_data = data;
	adc_ctrl.data_len = len;
	adc_ctrl.count = 0;
	adc_ctrl.notify = notify;
	*adc_ctrl.notify = 0;
	
	if (len != 0) {
		adc_auto_int_set(TRUE); /* enable fifo interrupt */
	}
	return STATUS_OK;	
}

void adc_auto_sample_stop(void)
{
	R_ADC_SETUP &= ~ADC_ASMEN;	
}

INT32S adc_sample_rate_set(INT8U timer_id, INT32U hz)
{
	INT32U as_timer;
/*	
	switch(timer_id) {
		case ADC_AS_TIMER_C:
			as_timer = TIMER_C;
			break;
		case ADC_AS_TIMER_D:
			as_timer = TIMER_D;
			break;
		case ADC_AS_TIMER_E:
			as_timer = TIMER_E;
			break;
		case ADC_AS_TIMER_F:
			as_timer = TIMER_F;
			break;
		default:
			return STATUS_FAIL;
	}
	
	R_ADC_SETUP &= ~ADC_ASTMS;
*/
	as_timer = timer_id;
	R_ADC_SETUP &= ~0x07;
	R_ADC_SETUP |= timer_id;
	adc_timer_freq_setup(as_timer, hz);
	
	return STATUS_OK;
}

INT32S adc_timer_stop(INT8U timer_id)
{
	INT32U as_timer;
/*	
	switch(timer_id) {
		case ADC_AS_TIMER_C:
			as_timer = TIMER_C;
			break;
		case ADC_AS_TIMER_D:
			as_timer = TIMER_D;
			break;
		case ADC_AS_TIMER_E:
			as_timer = TIMER_E;
			break;
		case ADC_AS_TIMER_F:
			as_timer = TIMER_F;
			break;
		default:
			return STATUS_FAIL;
	}
*/
	as_timer = timer_id;
	timer_stop(as_timer);	
	return STATUS_OK;
}

void adc_fifo_clear(void)
{
	INT16U dummy;
	while((R_ADC_ASADC_CTRL & 0xF) != 0) {
		dummy = R_ADC_ASADC_DATA;	
	}	
}

void adc_tp_en(BOOLEAN status)
{
	if (status == TRUE) {
		R_ADC_TP_CTRL |= ADC_TP_EN;
		//R_ADC_TP_CTRL |= ADC_TP_MODE;
	}
	else {
		R_ADC_TP_CTRL &= ~ADC_TP_EN;
		//R_ADC_TP_CTRL &= ~ADC_TP_MODE;
	}
}

void adc_tp_int_en(BOOLEAN status)
{
	if (status == TRUE) {
		R_ADC_TP_CTRL |= ADC_TP_INT_EN;
	}
	else {
		R_ADC_TP_CTRL &= ~ADC_TP_INT_EN;
	}
}

INT32S adc_user_line_in_en(INT8U line_id,BOOLEAN status)
{
	switch(line_id) {
		case ADC_LINE_0:
		case ADC_LINE_1:
		case ADC_LINE_2:
		case ADC_LINE_3:
			break;
		default:
			return STATUS_FAIL;
	}
	
	if (status == TRUE) {
		R_ADC_USELINEIN |= line_id; 
	}
	else {
		R_ADC_USELINEIN &= ~line_id; 
	}
	
	return STATUS_OK;
}


void tp_callback_set(void (*user_isr)(void))
{
	if (user_isr != 0) {
		tp_user_isr = user_isr;
	}
}


//////////////////////////////// I2S RX //////////////////////////////

/* For Rx Control Register (ISRC) */
   enum I2SRX_ISCR_REG {
   /* Rx Control Register */
   I2SRX_MASK_EN_I2S_RX = 0x1,
   I2SRX_MASK_FirstFrameLR = 0x2,
   I2SRX_MASK_FramePolarity = 0x4,
   I2SRX_MASK_EdgeMode = 0x8,
   I2SRX_MASK_SendMode = 0x10,
   I2SRX_MASK_NormalModeAlign = 0x20,
   I2SRX_MASK_ValidDataMode = 0x1C0,
   I2SRX_MASK_FrameSizeMode = 0x600,
   I2SRX_MASK_I2SMode = 0x1800,
   I2SRX_MASK_MSTMode = 0x2000,
   I2SRX_MASK_IRT_Polarity = 0x4000,
   I2SRX_MASK_OVF = 0x8000,
   I2SRX_MASK_EN_IRT = 0x10000,
   I2SRX_MASK_IRT_PEND = 0x20000,
   I2SRX_MASK_ClrFIFO = 0x40000,
   I2SRX_MASK_MERGE = 0x80000,
   I2SRX_MASK_R_LSB = 0x100000,
   I2SRX_MASK_MonoMode = 0x200000,
   
   I2SRX_EN_RX = 0x0001,
   I2SRX_FirstFrame_Right = 0x0002,		/* Right frame first */
   I2SRX_FirstFrame_Left = 0x0000,
   I2SRX_FramePolarity_Left = 0x0004,	/* Left frame polarity */
   I2SRX_FramePolarity_Right = 0x0000,
   I2SRX_EdgeMode_Rising = 0x0008,		/* rising edge */
   I2SRX_EdgeMode_falling = 0x0000,		
   I2SRX_SendMode_LSB = 0x0010,		/* LSB first */
   I2SRX_SendMode_MSB = 0x0000,		/* MSB first */
   I2SRX_NormalModeAlign_Left = 0x0020,		/* Left align */
   I2SRX_NormalModeAlign_Right = 0x0000,		/* Left align */
   I2SRX_WordLength_16BIT = 0x0000,		
   I2SRX_WordLength_18BIT = 0x0040,		
   I2SRX_WordLength_20BIT = 0x0080,		
   I2SRX_WordLength_22BIT = 0x00C0,		
   I2SRX_WordLength_24BIT = 0x0100,		
   I2SRX_WordLength_32BIT = 0x0140,
   I2SRX_FrameSize_16BIT = 0x0000, 	/* frame size switch */
   I2SRX_FrameSize_24BIT = 0x0200,
   I2SRX_FrameSize_32BIT = 0x0400,
   I2SRX_FrameSize_Slave = 0x0600,
   I2SRX_Mode_I2Smode = 0x0000,
   I2SRX_Mode_Normal = 0x0800,
   I2SRX_Mode_DSP = 0x1000,
   I2SRX_Mode_DSP_2 = 0x1800,
   I2SRX_MSTMode_Master = 0x2000,
   I2SRX_MSTMode_Slave = 0x0000,
   I2SRX_IntPolarity_High = 0x4000,		/* interrupt is high active */
   I2SRX_IntPolarity_Low = 0x0000,		/* interrupt is Low active */
   I2SRX_OVF = 0x8000,        /* High active FIFO overflow flag, write 1 clear. */
   I2SRX_EN_IRT = 0x10000,    /* High active I2S-bus Rx interrupt enable bit */
   I2SRX_IRT_PEND = 0x20000,    /* Interrupt status (read only) */
   I2SRX_ClearFIFO = 0x40000,   /* High active FIFO clear bit, user need set 0 after clear(always on) */
   I2SRX_MERGE = 0x80000,	  /* 32 bit merge select */
   I2SRX_R_LSB = 0x100000,   /* Right channel data in LSB of RX_Data or not, only valid when word length is 16-bit wide */   
   I2SRX_Mono = 0x200000   /* Right channel data in LSB of RX_Data or not, only valid when word length is 16-bit wide */
   };

#define ADC_PLL_67MHz (R_SYSTEM_CKGEN_CTRL|=0x80)
#define ADC_PLL_73MHz (R_SYSTEM_CKGEN_CTRL&=(~0x80))
#define ADC_PLL_DIV_MASK (R_SYSTEM_MISC_CTRL2&=(~0xF000))
#define ADC_PLL_DIV6 (R_SYSTEM_MISC_CTRL2|=0x5000)
#define ADC_PLL_DIV9 (R_SYSTEM_MISC_CTRL2|=0x8000)
#define ADC_PLL_DIV12 (R_SYSTEM_MISC_CTRL2|=0xB000)

static INT32U rx_hz_bak = 0;

void i2s_adc_init(int mode)
{
	if (  (R_SYSTEM_POWER_CTRL0&0x700)!=0x700  )
	{	// CODEC LDO 3.3v
		R_SYSTEM_POWER_CTRL0 |= 0x700;
	}
	
	R_SYSTEM_CKGEN_CTRL |= 0x10;	//APLL EN  (audio PLL)
	
	// 設定輸入方式 & 音量
	R_MIC_SETUP &= (~0x1F);   // turn off ADC
	if (mode==0)
	{
		//0xC000C0044  bit[4:0]  MIC PGA  (值愈小，音量愈大)
		//0xC000C0044  bit[8]	 MIC boost	（1:ON, 0:OFF)
		R_MIC_SETUP |= 0x17;  // enable I2S RX clock, enable Vref
		//R_MIC_READY &= (~0x100);  // turn off  boost
		R_MIC_READY |= 0x118;  // turn on  boost
	}
	else
	{
		//0xC000C0044  bit[15:11]  LineInR PGA	(值愈小，音量愈大)
		//0xC000C0044  bit[10:6]   LineInL PGA	(值愈小，音量愈大)	
		R_MIC_SETUP |= 0x1B;  // LineIn
		R_MIC_SETUP |= 0x8400;	// adjust volume
	}
}

void i2s_rx_init(int mode)
{
	R_I2SRX_CTRL = 0x4820;		// Reset Value
	R_I2SRX_CTRL &= (~I2SRX_MASK_I2SMode);	// I2S Mode
	R_I2SRX_CTRL |= I2SRX_Mode_Normal;		// must Normal Mode because of front end design.
	R_I2SRX_CTRL |= I2SRX_MERGE;  // MERGE Mode
}	

void i2s_rx_exit(void)
{
	if (  (R_SYSTEM_POWER_CTRL0&0x700)==0x700  )
	{	// turn off CODEC LDO 3.3v
		R_SYSTEM_POWER_CTRL0 &= ~0x700;
	}
	R_SYSTEM_CKGEN_CTRL &= ~0x10;	//APLL EN	
	R_MIC_SETUP &= (~0x1F);   // turn off ADC
}	

#if 1
INT32S i2s_rx_sample_rate_set(INT32U hz)
{
	if (hz != rx_hz_bak)
	{
		ADC_PLL_DIV_MASK;
		switch(hz)
		{
			case 48000:
				ADC_PLL_73MHz;
				ADC_PLL_DIV6;
				break;
			case 44100:
				ADC_PLL_67MHz;
				ADC_PLL_DIV6;
				break;
			case 32000:
				ADC_PLL_73MHz;
				ADC_PLL_DIV9;
				break;
			case 24000:
				ADC_PLL_73MHz;
				ADC_PLL_DIV12;
				break;
			case 22050:
			default:
				ADC_PLL_67MHz;
				ADC_PLL_DIV12;			
		}
		rx_hz_bak = hz;
#if _OPERATING_SYSTEM == _OS_NONE
		{
			INT32U i;
			for (i=0;i<0x100;++i)
			{
				R_RANDOM0 = i;
			}
		}		
#else
		OSTimeDly(1);
#endif
	}

	return STATUS_OK;
}

#else

INT32S i2s_rx_sample_rate_set(INT32U hz)
{	// 這段程式要改寫
	(*((volatile INT32U *) 0xC0000108)) &= (~0x20000000);
	(*((volatile INT32U *) 0xC0000108)) |= 0x10000000;	// 用第二組出 PIN

	// 產生 BCLK, LRCK 給 RX
	R_I2STX_CTRL = 0x10104024; // default
	//R_I2STX_CTRL |= 0x400000;	// MONO mode		
	R_I2STX_CTRL &= (~0x07000000);	//reset BCLK
	R_I2STX_CTRL |= 0x07000000; 	//BCLK = MCLK / 8
	R_I2STX_CTRL |= 0x040000;		// clear FIFO
	R_I2STX_CTRL &= (~0x040000);	// clear FIFO	
	R_I2STX_CTRL |= 0x1;			// Enable I2S_TX (此時才會打出 MCLK)
	// RX register 設定（請設 slave mode，再由跳線把 BCLK, LRCK 接進來
	//R_I2SRX_CTRL |= 0x200000;	// MONO mode
	//R_I2SRX_CTRL = 0x84424;   // josephhsieh	
	R_I2SRX_CTRL|= 0x040000;	// clear FIFO
	R_I2SRX_CTRL&= (~0x040000); // clear FIFO

	return STATUS_OK;
}
#endif

INT32S i2s_rx_mono_ch_set(void)
{
	R_I2SRX_CTRL |= I2SRX_Mono;	// MONO mode
	return STATUS_OK;
}

void i2s_rx_fifo_clear(void)
{
	// FIFO 若有殘存值，則下次錄音可能左右聲道會互換
	// 使用時機： enable I2S RX 之前或用完後
	R_I2SRX_CTRL |= I2SRX_ClearFIFO;	// clear FIFO
	R_I2SRX_CTRL &= (~I2SRX_ClearFIFO); // clear FIFO
}

// 此時 DMA 已經啟動
INT32S i2s_rx_start(void)
{
	INT32S ret = STATUS_OK;
#if _OPERATING_SYSTEM == _OS_NONE	
	INT32U cnt = 0;
#endif

	// FIFO 若有殘存值，則下次錄音可能左右聲道會互換
	R_I2SRX_CTRL |= I2SRX_ClearFIFO;	// clear FIFO
	R_I2SRX_CTRL &= (~I2SRX_ClearFIFO); // clear FIFO

	R_I2SRX_CTRL |= I2SRX_EN_RX; 		// Disable I2S_RX
	// I2S RX 的 enable/disable，雖然以 SYS_CLK 寫入，但完成時間會參考 LRCK，所以會慢 1/(sample rate) 秒	
#if _OPERATING_SYSTEM == _OS_NONE
	while ((R_I2SRX_CTRL&I2SRX_EN_RX)==0)
	{	
		if (cnt>0x100000)
		{
			ret = STATUS_FAIL;
			break;
		}
		cnt++;
	}
#else
	do {
		OSTimeDly(1);
	} while((R_I2SRX_CTRL&I2SRX_EN_RX)==0);	
#endif

	return ret;
}

// 此時 DMA 已經停止
INT32S i2s_rx_stop(INT32U status)
{
	INT32S ret = STATUS_OK;
#if _OPERATING_SYSTEM == _OS_NONE	
	INT32U cnt = 0;
#endif

	R_I2SRX_CTRL &= (~I2SRX_EN_RX); 		// Disable I2S_RX
	// I2S RX 的 enable/disable，雖然以 SYS_CLK 寫入，但完成時間會參考 LRCK，所以會慢 1/(sample rate) 秒	
#if _OPERATING_SYSTEM == _OS_NONE
	while ((R_I2SRX_CTRL&I2SRX_EN_RX)==1)
	{	
		if (cnt>0x100000)
		{
			ret = STATUS_FAIL;
			break;
		}
		cnt++;
	}
#else
	do {
		OSTimeDly(1);
	}while((R_I2SRX_CTRL&I2SRX_EN_RX)==1);
#endif
	// FIFO 若有殘存值，則下次錄音可能左右聲道會互換
	R_I2SRX_CTRL |= I2SRX_ClearFIFO;	// clear FIFO
	R_I2SRX_CTRL &= (~I2SRX_ClearFIFO); // clear FIFO			

	return ret;
}

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#endif //(defined _DRV_L1_ADC) && (_DRV_L1_ADC == 1)              //
//================================================================//
