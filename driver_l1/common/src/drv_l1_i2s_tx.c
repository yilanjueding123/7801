/*
* Purpose: I2S TX driver/interface
*
* Author: josephhsieh
*
* Date: 2014/4/8
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 1.00
* History :
*/

//Include files
#include "drv_l1_i2s_tx.h"

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#if (defined _DRV_L1_I2S_TX) && (_DRV_L1_I2S_TX == 1)             //
//================================================================//

// Constant definitions used in this file only go here

#define MCLK_PLL_67MHz (R_SYSTEM_CKGEN_CTRL|=0x80)
#define MCLK_PLL_73MHz (R_SYSTEM_CKGEN_CTRL&=(~0x80))
#define MCLK_PLL_DIV_MASK (R_SYSTEM_MISC_CTRL2&=(~0xF000))
#define MCLK_PLL_DIV4 (R_SYSTEM_MISC_CTRL2|=0x3000)
#define MCLK_PLL_DIV6 (R_SYSTEM_MISC_CTRL2|=0x5000)
#define MCLK_PLL_DIV8 (R_SYSTEM_MISC_CTRL2|=0x7000)
#define MCLK_PLL_DIV12 (R_SYSTEM_MISC_CTRL2|=0xB000)
#define MCLK_PLL_DIV16 (R_SYSTEM_MISC_CTRL2|=0xF000)


/* For Tx Control Register (ISRC) */
   enum I2STX_ISCR_REG {
   /* Tx Control Register */
   I2STX_MASK_EN_I2S_TX = 0x1,
   I2STX_MASK_FirstFrameLR = 0x2,
   I2STX_MASK_FramePolarity = 0x4,
   I2STX_MASK_EdgeMode = 0x8,
   I2STX_MASK_SendMode = 0x10,
   I2STX_MASK_NormalModeAlign = 0x20,
   I2STX_MASK_ValidDataMode = 0x1C0,
   I2STX_MASK_SPU_SW = 0x600,
   I2STX_MASK_I2SMode = 0x1800,
   I2STX_MASK_SLVMode = 0x2000,
   I2STX_MASK_IRT_Polarity = 0x4000,
   I2STX_MASK_EN_OVWR_n = 0x8000,
   I2STX_MASK_EN_IRT = 0x10000,
   I2STX_MASK_IRT_FLAG = 0x20000,
   I2STX_MASK_ClrFIFO = 0x40000,
   I2STX_MASK_UndFlow = 0x80000,
   I2STX_MASK_MERGE = 0x100000,
   I2STX_MASK_R_LSB = 0x200000,
   I2STX_MASK_MonMode = 0x400000,
   I2STX_MASK_DIV_MCLK = 0x7000000,
   I2STX_MASK_FrameSizeMode = 0xF0000000,
   
   I2STX_EN = 0x0001,
   I2STX_FirstFrame_Right = 0x0002,		/* Right frame first */
   I2STX_FirstFrame_Left =	0x0000,
   I2STX_FramePolarity_Left = 0x0004,	/* Left frame polarity */
   I2STX_FramePolarity_Right = 0x0000,
   I2STX_EdgeMode_Rising = 0x0008,		/* rising edge */
   I2STX_EdgeMode_falling = 0x0000,		
   I2STX_SendMode_LSB = 0x0010,		/* LSB first */
   I2STX_SendMode_MSB = 0x0000,		/* MSB first */
   I2STX_NormalModeAlign_Left = 0x0020,		/* Left align */
   I2STX_NormalModeAlign_Right = 0x0000,		/* Left align */
   I2STX_WordLength_16BIT = 0x0000,
   I2STX_WordLength_18BIT = 0x0040,
   I2STX_WordLength_20BIT = 0x0080,
   I2STX_WordLength_22BIT = 0x00C0,
   I2STX_WordLength_24BIT = 0x0100,
   I2STX_WordLength_32BIT = 0x0140,
   I2STX_WordLength_08BIT = 0x0180,
   // I2STX_SPU_SW = 0x0600,
   I2STX_Mode_I2Smode = 0x0000,
   I2STX_Mode_Normal = 0x0800,
   I2STX_Mode_DSP = 0x1000,
   I2STX_Mode_DSP_2 = 0x1800,
   I2STX_SLVMode_Tx_Master = 0x0000,
   I2STX_SLVMode_Tx_Slave = 0x2000,
   I2STX_IntPolarity_High = 0x4000,		/* interrupt is high active */
   I2STX_IntPolarity_Low = 0x0000,		/* interrupt is Low active */
   I2STX_EN_OVWR_n = 0x8000,
   I2STX_EN_IRT = 0x10000,		/* HIgh Active: Tx interrupt enable bit */
   I2STX_IRT_FLAG = 0x20000,	/* High Active: Tx interrupt flag bit, write 1 clear */
   I2STX_ClrFIFO = 0x40000,	/* High active Tx FIFO, automatically clear to 0 after FIFO cleared. */
   I2STX_UndFlow = 0x80000,
   I2STX_MERGE = 0x100000,
   I2STX_R_LSB = 0x200000,
   I2STX_MonoMode = 0x400000,
   I2STX_MCLK_DIV2 = 0x2000000,
   I2STX_MCLK_DIV3 = 0x3000000,
   I2STX_MCLK_DIV4 = 0x4000000,
   I2STX_MCLK_DIV6 = 0x6000000,
   I2STX_MCLK_DIV8 = 0x7000000,
   I2STX_FrameSize_16BIT = 0x00000000,
   I2STX_FrameSize_24BIT = 0x10000000,
   I2STX_FrameSize_32BIT = 0x20000000,
   I2STX_FrameSize_48BIT = 0x30000000,
   I2STX_FrameSize_64BIT = 0x40000000,
   I2STX_FrameSize_96BIT = 0x50000000,
   I2STX_FrameSize_128BIT = 0x60000000,	 
   I2STX_FrameSize_176BIT = 0x70000000,	    
   I2STX_FrameSize_192BIT = 0x80000000,
   I2STX_FrameSize_SlaveMode = 0xF0000000
   };

// Type definitions used in this file only go here

// Variables defined in this file
static INT32U tx_hz_bak = 0;

// Global inline functions(Marcos) used in this file only go here
INT32S i2s_tx_sample_rate_set(INT32U hz)
{
	//(*((volatile INT32U *) 0xC0000108)) &= (~0x20000000);
	//(*((volatile INT32U *) 0xC0000108)) |= 0x10000000;	// 用第二組 I2S_TX 出 PIN

	if (hz != tx_hz_bak)
	{
		MCLK_PLL_DIV_MASK;
		switch(hz)
		{
			case 48000:
				MCLK_PLL_73MHz;
				MCLK_PLL_DIV4;
				break;
			case 44100:
				MCLK_PLL_67MHz;
				MCLK_PLL_DIV4;
				break;
			case 32000:
				MCLK_PLL_73MHz;
				MCLK_PLL_DIV6;
				break;
			case 24000:
				MCLK_PLL_73MHz;
				MCLK_PLL_DIV8;
				break;
			case 22050:
				MCLK_PLL_67MHz;
				MCLK_PLL_DIV8;
				break;
			case 16000:
				MCLK_PLL_73MHz;
				MCLK_PLL_DIV12;
				break;
			case 12000:
				MCLK_PLL_73MHz;
				MCLK_PLL_DIV16;
				break;
			case 11025:
				MCLK_PLL_67MHz;
				MCLK_PLL_DIV16;
				break;
			default:
				MCLK_PLL_67MHz;
				MCLK_PLL_DIV8;			
		}
		tx_hz_bak = hz;
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

void i2s_tx_init(void)
{
	R_I2STX_CTRL = 0x14004A20;		// Reset Value  (FrameSize=24-bit)
	R_I2STX_CTRL |= I2STX_MCLK_DIV8; // BCLK = MCLK/8 （固定）
	R_I2STX_CTRL &= (~I2STX_MASK_I2SMode);	// I2S Mode
	R_I2STX_CTRL |= I2STX_MERGE;  // MERGE Mode
}	

void i2s_tx_exit(void)
{
}	



INT32S i2s_tx_mono_ch_set(void)
{
	R_I2STX_CTRL |= I2STX_MonoMode;	// MONO mode
	return STATUS_OK;
}

void i2s_tx_fifo_clear(void)
{
	// FIFO 若有殘存值，則下次錄音可能左右聲道會互換
	// 使用時機： enable I2S TX 之前或用完後
	R_I2STX_CTRL |= I2STX_ClrFIFO;	// clear FIFO
	R_I2STX_CTRL &= (~I2STX_ClrFIFO); // clear FIFO
}

// 此時 DMA 已經啟動
INT32S i2s_tx_start(void)
{
	INT32S ret = STATUS_OK;
#if _OPERATING_SYSTEM == _OS_NONE	
	INT32U cnt = 0;
#endif

	// FIFO 若有殘存值，則下次錄音可能左右聲道會互換
	R_I2STX_CTRL |= I2STX_ClrFIFO;	// clear FIFO
	R_I2STX_CTRL &= (~I2STX_ClrFIFO); // clear FIFO

	R_I2STX_CTRL |= I2STX_EN; 		// Disable I2S_RX
	// I2S TX 的 enable/disable，雖然以 SYS_CLK 寫入，但完成時間會參考 LRCK，所以會慢 1/(sample rate) 秒	
#if _OPERATING_SYSTEM == _OS_NONE
	while ((R_I2STX_CTRL&I2STX_EN)==0)
	{	
		if (cnt>0x100000)
		{
			ret = STATUS_FAIL;
			break;
		}
		cnt++;
	}
#else
		OSTimeDly(1);
#endif
}

// 此時 DMA 已經停止
INT32S i2s_tx_stop(INT32U status)
{
	INT32S ret = STATUS_OK;
#if _OPERATING_SYSTEM == _OS_NONE	
	INT32U cnt = 0;
#endif

	R_I2STX_CTRL &= (~I2STX_EN); 		// Disable I2S_TX
	// I2S TX 的 enable/disable，雖然以 SYS_CLK 寫入，但完成時間會參考 LRCK，所以會慢 1/(sample rate) 秒	
#if _OPERATING_SYSTEM == _OS_NONE
	while ((R_I2STX_CTRL&I2STX_EN)==1)
	{	
		if (cnt>0x100000)
		{
			ret = STATUS_FAIL;
			break;
		}
		cnt++;
	}
#else
	OSTimeDly(1);
#endif
	// FIFO 若有殘存值，則下次錄音可能左右聲道會互換
	R_I2STX_CTRL |= I2STX_ClrFIFO;	// clear FIFO
	R_I2STX_CTRL &= (~I2STX_ClrFIFO); // clear FIFO			

	return ret;
}

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#endif //(defined _DRV_L1_I2S_TX) && (_DRV_L1_I2S_TX == 1)        //
//================================================================//

