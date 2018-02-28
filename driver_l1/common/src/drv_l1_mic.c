/*
* Purpose: MIC driver/interface
*
* Author: 
*
* Date: 
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 
* History :
*/

//Include files
#include "drv_l1_mic.h"

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#if (defined _DRV_L1_ADC) && (_DRV_L1_ADC == 1)                   //
//================================================================//

// Constant definitions used in this file only go here

// Type definitions used in this file only go here

// Global inline functions(Marcos) used in this file only go here

// Declaration of global variables and functions defined in other files

// Variables defined in this file
MIC_CTRL mic_ctrl;

void mic_init(void)
{
	R_MIC_SETUP  = 0xC000;
	R_MIC_ASADC_CTRL = 0;
	
	R_MIC_SETUP |= MIC_ADBEN; /* AD Bias Reference Voltage Enable */	
	
	gp_memset((INT8S *)&mic_ctrl, 0, sizeof(MIC_CTRL)); 
	
}	



void mic_auto_int_set(BOOLEAN status)
{
	if (status == TRUE) {
		R_MIC_ASADC_CTRL |= MIC_AUTO_ASIEN;
	}
	else {
		R_MIC_ASADC_CTRL &= ~MIC_AUTO_ASIEN;
	}
}

void mic_fifo_level_set(INT8U level)
{
	R_MIC_ASADC_CTRL &= ~MIC_FIFO_LEVEL;
	R_MIC_ASADC_CTRL |= (level << 4); 
}

void mic_fifo_clear(void)
{
	INT16U dummy;
	while((R_MIC_ASADC_CTRL & 0xF) != 0) {
		dummy = R_MIC_ASADC_DATA;	
	}	
}

INT32S mic_auto_sample_start(void)
{
	R_MIC_SETUP |= MIC_ADBEN; /* AD Bias Reference Voltage Enable */
	R_MIC_SETUP |= MIC_EN; // MIC enable 
	R_MIC_ASADC_CTRL |= MIC_ASIF; /* clear fifo interrupt flag */
	R_MIC_ASADC_CTRL |= MIC_ASADC_DMA; /* DMA mode enable */
	R_MIC_SETUP |= MIC_ASEN;
	R_MIC_SETUP |= MIC_ASMEN; /* start auto sample */
	
	R_MIC_SETUP |= MIC_BITTRANS;
	//B edition IC 
	if((*((volatile INT32U *) 0xF0001FFC))==0x322E3576)
	{
	    R_MIC_SETUP |= MIC_ASIEN;
	}
	
	return STATUS_OK;
}

void mic_auto_sample_stop(void)
{
	R_MIC_SETUP &= ~MIC_ASMEN;	/* stop auto sample */
}

INT32S mic_sample_rate_set(INT8U timer_id, INT32U hz)
{
	INT32U as_timer;
	
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
	
	R_MIC_SETUP &= ~MIC_ASTMS;
	R_MIC_SETUP |= timer_id;
	adc_timer_freq_setup(as_timer, hz);
	
	return STATUS_OK;
}

INT32S mic_timer_stop(INT8U timer_id)
{
	INT32U as_timer;
	
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
	timer_stop(as_timer);	
	return STATUS_OK;
}


void mic_vref_enable_set(BOOLEAN status)
{
	if (status == TRUE) {
		R_MIC_SETUP |= MIC_ADBEN; /* AD Bias Reference Voltage Enable */
	}
		
	else {
		R_MIC_SETUP &= ~MIC_ADBEN; /* AD Bias Reference Voltage Disable */
	}
}

void mic_agc_enable_set(BOOLEAN status)
{
	if (status == TRUE) {
		R_MIC_SETUP |= MIC_AGCEN; /* AGC Enable */
	}
		
	else {
		R_MIC_SETUP &= ~MIC_AGCEN; /* AGC Disable */
	}
}


INT32S mic_auto_data_dma_get(INT16U *data, INT32U len, INT8S *notify)
{
	DMA_STRUCT  dma_struct;
	INT32S      status;
	
	/*if (len > MIC_BLOCK_LEN) {
		return MIC_ERR_WRONG_LEN;
	}
	*/
	*notify = C_DMA_STATUS_WAITING;
	dma_struct.s_addr = (INT32U) P_MIC_ASADC_DATA;
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

INT32S mic_auto_data_get(INT16U *data, INT32U len, INT8S *notify)
{
	/*if (len > MIC_BLOCK_LEN) {
		return MIC_ERR_WRONG_LEN;
	}*/
	R_MIC_ASADC_CTRL &= ~MIC_ASADC_DMA; /* DMA mode disable */
	
	mic_ctrl.mic_data = data;
	mic_ctrl.data_len = len;
	mic_ctrl.count = 0;
	mic_ctrl.notify = notify;
	*mic_ctrl.notify = 0;
	
	if (len != 0) {
		mic_auto_int_set(TRUE); /* enable fifo interrupt */
	}
	return STATUS_OK;	
}



//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#endif //(defined _DRV_L1_ADC) && (_DRV_L1_ADC == 1)              //
//================================================================//