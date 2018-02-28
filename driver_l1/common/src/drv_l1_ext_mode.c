#include "drv_l1_system.h"

static INT32S iRAM_Delay(INT32U cnt)
{
	INT32U i;
	
	for (i=0; i<cnt;++i)
	{
		__asm {NOP};
	}
	return 0;
}

////////////////
// ICE 的 clock 的來源是 SYSCLK
// ICE 要穩定 SYSCLK / ICE_CLK > 7
// 原來 SYSCLK=144MHz,  ICE LCK=4MHz
// 進入 SLOW mode， SYSCLK=12MHz，所以 ICE clk 不能超過 1.7M Hz
// ICE 降至1.3M Hz 以免在切頻時飛掉。
// 切到外部 XTAL 12M
void system_clk_ext_XLAT_12M(void)
{
	R_SYSTEM_MISC_CTRL1 |= 0x01;
	R_SYSTEM_CKGEN_CTRL |= 0x14C;

	{
		INT32U i;
		for (i=0;i<0xF000;++i) {
			R_RANDOM0 = i;		// delay for XLAT clock stable
		}
		
	}
	
	R_SYSTEM_CLK_CTRL &= 0x3fff;		//enter SLOW mode

	while ( (R_SYSTEM_POWER_STATE&0xF)!=1 ) {
		//DBG_PRINT("wait goint to SLOW mode\r\n");
		;		
	}
	
	R_SYSTEM_CKGEN_CTRL |= 0x21;		
	R_SYSTEM_CLK_CTRL |= 0x8000;		//enter FAST mode again

	
	while ( (R_SYSTEM_POWER_STATE&0xF)!=2 ) {
		//DBG_PRINT("wait coming back to FAST mode\r\n");
		;		
	}

	R_SYSTEM_CTRL |= 0x00000902;  // josephhsieh@140519 sensor 前端固定為48MHz
	R_SYSTEM_CTRL &= ~0x1000;	//clear SEL30K
	R_SYSTEM_CKGEN_CTRL |= 0x1F;
}

void setting_by_iRAM(void)
{
	__asm { NOP };
}

void system_clk_alter(INT32U SysClk, INT32U SDramPara)
{	// switch system clock in TFT Vblanking  (ISR)
	SysClk |= (R_SYSTEM_PLLEN & (~0x1F));
	R_MEM_SDRAM_CTRL0 = SDramPara;	
	R_SYSTEM_PLLEN =  SysClk;
	
	iRAM_Delay(16);
}

