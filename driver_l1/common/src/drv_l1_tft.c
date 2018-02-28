/* 
* Purpose: TFT driver/interface
*
* Author: Allen Lin
*
* Date: 2008/5/9
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 1.00
* History :
*/

//Include files
#include "drv_l1_tft.h"
#include "application.h"

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#if (defined _DRV_L1_TFT) && (_DRV_L1_TFT == 1)                   //
//================================================================//

INT32U  CSN_n;
INT32U  SCL_n;
INT32U  SDA_n;
INT32U  SDI_n;

void TFT_TXDT240C_5182A_Init(void);
extern void HX8356_Initial(void);
extern void ILI9341_Initial(void);
extern void HX7781_Initial(void);
extern void TFT_D51E5TA8566_Init(void);
void tft_T27P05_ILI8961_init(void);
#if (USE_PANEL_NAME == PANEL_T43)
void tft_T43_init(void);
#endif
void tft_T40P00_ILI9342C_init(void);
#if USE_PANEL_NAME ==  PANEL_T15P06
void tft_T15P06_init(void);
#endif

#if USE_PANEL_NAME == PANEL_T20P82_ST7789V
extern void ap_display_TE_sync_I80_isr(void);
extern void TFT_T20P82_ST7789V_Init(void);
#endif

void tft_AUO_A015AN05_init(void);
void AP_TFT_ClK_144M_set(void)
{
	#if USE_PANEL_NAME == PANEL_T27P05_ILI8961	
		tft_clk_set(TFT_CLK_DIVIDE_8);
	#elif USE_PANEL_NAME ==  PANEL_T43
		tft_clk_set(TFT_CLK_DIVIDE_5);
	#elif (USE_PANEL_NAME == PANEL_400X240_I80)
		tft_clk_set(TFT_CLK_DIVIDE_14);
	#elif (USE_PANEL_NAME == PANEL_T40P00_ILI9342C)
		tft_clk_set(TFT_CLK_DIVIDE_8);
	#elif (USE_PANEL_NAME == PANEL_T20P82_ST7789V)	
		tft_clk_set(TFT_CLK_DIVIDE_4);
	#elif (USE_PANEL_NAME == PANEL_T15P06)	
		tft_clk_set(TFT_CLK_DIVIDE_4);
	#else	// GPCV1248 EVB   
		tft_clk_set(TFT_CLK_DIVIDE_8);
	#endif	

}
void AP_TFT_ClK_48M_set(void)
{
	#if USE_PANEL_NAME == PANEL_T27P05_ILI8961	
		tft_clk_set(TFT_CLK_DIVIDE_2);
	#elif USE_PANEL_NAME ==  PANEL_T43
		tft_clk_set(TFT_CLK_DIVIDE_2);
	#elif (USE_PANEL_NAME == PANEL_400X240_I80)
		tft_clk_set(TFT_CLK_DIVIDE_3);
	#elif (USE_PANEL_NAME == PANEL_T40P00_ILI9342C)
		tft_clk_set(TFT_CLK_DIVIDE_2);
	#elif (USE_PANEL_NAME == PANEL_T20P82_ST7789V)	
		tft_clk_set(TFT_CLK_DIVIDE_2);
	#elif (USE_PANEL_NAME == PANEL_T15P06)	
		tft_clk_set(TFT_CLK_DIVIDE_2);
	#else	// GPCV1248 EVB   
		tft_clk_set(TFT_CLK_DIVIDE_2);
	#endif	

}


void tft_slide_en_set(BOOLEAN status)
{
	if (status == TRUE) {
		R_TFT_TS_MISC |= TFT_SLIDE_EN;
	}
	else {
		R_TFT_TS_MISC &= ~TFT_SLIDE_EN;
	}
}

#define BL_LOW_ACTIVE		0
#define BL_HIGH_ACTIVE		1

void tft_init(void)
{
#if !((USE_PANEL_NAME == PANEL_T43)||(USE_PANEL_NAME == PANEL_400X240_I80)||(USE_PANEL_NAME == PANEL_T20P82_ST7789V))
	INT32U reg;
#endif

	R_PPU_MISC=0x2;
#if ((USE_PANEL_NAME == PANEL_T43)||(USE_PANEL_NAME == PANEL_400X240_I80))

//   	R_PPU_ENABLE= PPU_QVGA_MODE|PPU_FRAME_BASE_MODE|PPU_RGB565_MODE;	 
   	R_PPU_ENABLE= PPU_VGA_MODE|PPU_FRAME_BASE_MODE|PPU_RGB565_MODE;	 
	R_FREE_SIZE = TFT_HEIGHT; //Vertical
	R_FREE_SIZE |= (TFT_WIDTH << 16); //Horizontal
#elif (USE_PANEL_NAME == PANEL_T20P82_ST7789V)
   	R_PPU_ENABLE= PPU_QVGA_MODE|PPU_FRAME_BASE_MODE|PPU_RGB565_MODE;	 
   	//R_PPU_ENABLE= PPU_VGA_MODE|PPU_FRAME_BASE_MODE|PPU_RGB565_MODE;	 
	R_FREE_SIZE = TFT_HEIGHT; //Vertical
	R_FREE_SIZE |= (TFT_WIDTH << 16); //Horizontal
#else

    // dennis: temp
    #define FB_FORMAT 0
    #define FB_MONO   0
    #define YUV_TYPE  2
    #define LCD_RESO_320X240    0

    reg = PPU_FRAME_BASE_MODE;
    reg |= FB_FORMAT << 8;
    reg |= FB_MONO <<10;
    reg |= YUV_TYPE <<20;
    reg |= (1<<24); // TFTLB
    R_PPU_ENABLE = reg | (LCD_RESO_320X240<<16);
#endif


 	R_TFT_CTRL = 0;
	R_SPI0_CTRL = 0;
	tft_slide_en_set(FALSE);
#if USE_PANEL_NAME == PANEL_T27P05_ILI8961	
    tft_T27P05_ILI8961_init();
#elif USE_PANEL_NAME ==  PANEL_T43
    tft_T43_init();
 #elif (USE_PANEL_NAME == PANEL_400X240_I80)
 	TFT_D51E5TA8566_Init();
#elif (USE_PANEL_NAME == PANEL_T40P00_ILI9342C)
 	tft_T40P00_ILI9342C_init();
#elif (USE_PANEL_NAME == PANEL_T20P82_ST7789V)
    TFT_T20P82_ST7789V_Init();
    
    /*gpio_init_io(IO_B9,GPIO_INPUT);
    gpio_set_port_attribute(IO_B9, 0);
    //Enable INT 
 	extab_enable_set(EXTB,TRUE);
	extab_edge_set(EXTB,RISING);
	extab_int_clr(EXTB);  //if EXT happen ,clear the interrupt flag
	extab_user_isr_set(EXTB,ap_display_TE_sync_I80_isr);*/
#elif USE_PANEL_NAME ==  PANEL_T15P06
    tft_T15P06_init();
#else	// GPCV1248 EVB    
    tft_tpo_td025thea7_init();
 #endif	
}

void tft_backlight_en_set(BOOLEAN status)
{
   	 gpio_set_port_attribute(TFT_BL, ATTRIBUTE_HIGH);
	gpio_init_io(TFT_BL, GPIO_OUTPUT);	


#if BL_LOW_ACTIVE
	if (status) {
		gpio_write_io(TFT_BL, DATA_LOW);
	}
	else {
		gpio_write_io(TFT_BL, DATA_HIGH);
	}
#elif BL_HIGH_ACTIVE
	if (status) {
		gpio_write_io(TFT_BL, DATA_HIGH);
	}
	else {
		gpio_write_io(TFT_BL, DATA_LOW);
	}
#endif


}
	
void cmd_delay(INT32U i) 
{
	unsigned int j, cnt;
	cnt = i*2;
	for (j=0;j<cnt;j++);
}

void cmd_longdelay(INT32U i) 
{
	INT32U j, cnt;
	
	cnt = i*100;
	
	for (j=0;j<cnt;j++);	
}

void tft_data_mode_set(INT32U mode)
{
	R_TFT_TS_MISC &= ~TFT_DATA_MODE;
	R_TFT_TS_MISC |= mode;	
}

void tft_mem_unit_set(BOOLEAN status)
{
	if (status == TRUE) {
		R_TFT_CTRL |= TFT_MEM_BYTE_EN;
	}
	else {
		R_TFT_CTRL &= ~TFT_MEM_BYTE_EN; /* word */
	}
}

void tft_mem_mode_ctrl_set(INT32U mem_ctrl)
{
	R_TFT_CTRL &= ~TFT_MODE;
	R_TFT_CTRL |= (mem_ctrl|TFT_EN);		
}

void tft_clk_set(INT32U clk)
{
	R_TFT_CTRL &= ~TFT_CLK_SEL;
	R_TFT_TS_MISC &= ~0xC0;
	
	if (clk < TFT_CLK_DIVIDE_9) {
		R_TFT_CTRL |= clk;
	}
	else {
		R_TFT_CTRL |= clk & 0xF;
		R_TFT_TS_MISC |= (clk & 0x20) << 1;
		R_TFT_TS_MISC |= (clk & 0x10) << 3;
	}	
}	

void i80_write(INT8U index,INT8U *data,INT8U len)
{
	INT32U i;
	
	R_TFT_MEM_BUFF_WR = index;
	tft_mem_mode_ctrl_set(TFT_MODE_MEM_CMD_WR);

	for (i=0;i<len;i++) {
		R_TFT_MEM_BUFF_WR = *data++;
		tft_mem_mode_ctrl_set(TFT_MODE_MEM_DATA_WR);
	}
}
//driver fd54122
void tft_g200_init(void)
{
	INT8U data[16] = {0};
	
	R_TFT_CTRL = 0;
	R_TFT_V_PERIOD = 128-1;
    R_TFT_VS_WIDTH = 5;  //blanking time
    R_TFT_V_START = 5;    //setup time                                                                              
    R_TFT_H_PERIOD = 128-1; 
    R_TFT_HS_WIDTH = 1;   //RD/WR plus low
    R_TFT_H_START = 1;	  //RD/WR plus hi
    
   
	tft_mem_unit_set(TRUE); // Byte mode
	tft_clk_set(TFT_CLK_DIVIDE_2);
	tft_data_mode_set(TFT_DATA_MODE_8);
	
	
	gpio_init_io (IO_F12, GPIO_OUTPUT);
	gpio_set_port_attribute(IO_F12, 1);
	
	drv_msec_wait(5);
	gpio_write_io(IO_F12,0);
	drv_msec_wait(5);
	gpio_write_io(IO_F12,1);
	drv_msec_wait(5);

	i80_write(0x11,0,0); /* sleep out */
	drv_msec_wait(200);
	
	
    data[0] = 0x4;             
	i80_write(0x26,data,1); /* gamma 2.5 */
	
	data[0] = 0xe;
	data[1] = 0x14;
	i80_write(0xb1,data,2); /* frame control */
	
	data[0] = 0x8;
	data[1] = 0x0;
	i80_write(0xc0,data,2); /* GVDD */
	
	data[0] = 0x5;             
	i80_write(0xc1,data,1); //AVDD VGL VGL VCL VGH = 16.5V  VGL=-8.25V 
	
	//data[0] = 0x3a;
	//data[1] = 0x2d;
	data[0] = 0x64;
	data[1] = 0x64;
	i80_write(0xc5,data,2); //SET VCOMH & VCOML	
	
	data[0] = 0x3f;
	i80_write(0xc6,data,1); 
	
	data[0] = 0x5;
	i80_write(0x3A,data,1); //format
	
	
	data[0] = 0x00;
	i80_write(0xb4,data,1); //all line inversion

	data[0] = 0xc8; 
	i80_write(0x36,data,1); /* pixel odred is BGR */
	
	data[0] = 0;
	data[1] = 2;
	data[2] = 0;
	data[3] = 129;
	
	i80_write(0x2a,data,4); /* column address range */
	
	data[0] = 0;
	data[1] = 1+2;
	data[2] = 0;
	data[3] = 128+2;
	
	i80_write(0x2b,data,4); /* row address range */
	
	i80_write(0x29,0,0); /* display on */
	
	
	R_TFT_MEM_BUFF_WR = 0x2C;
    tft_mem_mode_ctrl_set(TFT_MODE_MEM_CMD_WR);
    
    R_TFT_CTRL = 0x20d3; //byte mode and update once
}

void tft_mem_buf_update(void)
{
	//R_TFT_MEM_BUFF_WR = 0x2C;
    //tft_mem_mode_ctrl_set(TFT_MODE_MEM_CMD_WR);
	R_TFT_CTRL = 0x20d3;	
}

void tft_tft_en_set(BOOLEAN status)
{
	if (status == TRUE) {
		R_TFT_CTRL |= TFT_EN;
	}		
	else {
		R_TFT_CTRL &= ~TFT_EN;
	}
}

void tft_mode_set(INT32U mode)
{
	R_TFT_CTRL &= ~TFT_MODE;
	R_TFT_CTRL |= mode;		
}

void tft_signal_inv_set(INT32U mask, INT32U value)
{
	/*set vsync,hsync,dclk and DE inv */
	R_TFT_CTRL &= ~mask;
	R_TFT_CTRL |= (mask & value);		
}

void serial_cmd_1(INT32U cmd) {

	INT32S i;
	
	gpio_write_io(CSN_n,1);//CS=1
	gpio_write_io(SCL_n,0);//SCL=0
	gpio_write_io(SDA_n,0);//SDA
	
	// set csn low
	gpio_write_io(CSN_n,0);//CS=0
	cmd_delay(1);
	for (i=0;i<16;i++) {
		// shift data
		gpio_write_io(SDA_n, ((cmd&0x8000)>>15)); /* SDA */
		cmd = (cmd<<1);
		cmd_delay(1);
		// toggle clock
		gpio_write_io(SCL_n,1);//SCL=0
		cmd_delay(1);
		gpio_write_io(SCL_n,0);//SCL=0		
		cmd_delay(1);
	}
	
	// set csn high
	gpio_write_io(CSN_n,1);//CS=1
	
	cmd_delay(1);
			
}

void tft_AUO_A015AN05_init(void)
{
	/* 320*240 */
#if 0
	INT32U spi_cs = IO_E0;
	INT8U send[2];
	
	spi_cs_by_internal_set(SPI_0,spi_cs, 0);
	spi_clk_set(SPI_0,SYSCLK_8); 
	spi_pha_pol_set(SPI_0,PHA0_POL0);
	
	send[0] = 0x08;
	send[1] = 0;
	spi_transceive(SPI_0,send, 2, NULL, 0);
#else
#if GPDV_BOARD_VERSION == DVP_V1_0
	CSN_n = IO_C9;
	SDA_n = IO_C8;
    SCL_n = IO_C6;
#else
	CSN_n = IO_G7;
	SDA_n = IO_C8;
    SCL_n = IO_C9;
#endif
	
    gpio_init_io (CSN_n, GPIO_OUTPUT);
	gpio_init_io (SCL_n, GPIO_OUTPUT);
	gpio_init_io (SDA_n, GPIO_OUTPUT);
    gpio_init_io (TFT_BL, GPIO_OUTPUT);
	
	gpio_set_port_attribute(CSN_n, 1);
	gpio_set_port_attribute(SCL_n, 1);
	gpio_set_port_attribute(SDA_n, 1);
	gpio_set_port_attribute(TFT_BL, 1);
/*	
    serial_cmd_1(0x400C);//R2
    serial_cmd_1(0x4000);//R2
    drv_msec_wait(30);
    serial_cmd_1(0x400C);//R2
    serial_cmd_1(0x0001);//R0
    serial_cmd_1(0x2001);//R1
    serial_cmd_1(0x6001); //R3
    serial_cmd_1(0x8006); //R4
    serial_cmd_1(0xA000); //R5
*/

#endif   
	/*
	R_TFT_LINE_RGB_ORDER = 0x00;
	R_TFT_V_PERIOD = 262;
	R_TFT_V_START = 22-1;
	R_TFT_V_END	= 22+240-1;
	
	R_TFT_H_PERIOD = 1560;
	R_TFT_H_START = 240;
	R_TFT_H_END	= 240+1280;
	*/
	R_TFT_HS_WIDTH			= 1;				//	1		=HPW
	R_TFT_H_START			= 1+248;			//	240		=HPW+HBP
	R_TFT_H_END				= 1+248+1280;	//	1520	=HPW+HBP+HDE
	R_TFT_H_PERIOD			= 1+248+1280+31;	//	1560	=HPW+HBP+HDE+HFP
	R_TFT_VS_WIDTH			= 1;				//	1		=VPW				(DCLK)
	R_TFT_V_START			= 1+23;			//	21		=VPW+VBP			(LINE)
	R_TFT_V_END				= 1+23+220;		//	261		=VPW+VBP+VDE		(LINE)
	R_TFT_V_PERIOD			= 1+23+220+20;		//	262		=VPW+VBP+VDE+VFP	(LINE)
	R_TFT_LINE_RGB_ORDER    = 0x00;
/*
	R_TFT_HS_WIDTH			= 25;				//	1		=HPW
	R_TFT_H_START			= 25+249;			//	240		=HPW+HBP
	R_TFT_H_END				= 25+249+1280;		//	1520	=HPW+HBP+HDE
	R_TFT_H_PERIOD			= 25+249+1280+31;	//	1560	=HPW+HBP+HDE+HFP
	R_TFT_VS_WIDTH			= 3;				//	1		=VPW				(DCLK)
	R_TFT_V_START			= 3+23;				//	21		=VPW+VBP			(LINE)
	R_TFT_V_END				= 3+23+220;			//	261		=VPW+VBP+VDE		(LINE)
	R_TFT_V_PERIOD			= 3+23+220+20;		//	262		=VPW+VBP+VDE+VFP	(LINE)
	R_TFT_LINE_RGB_ORDER    = 0x00;
*/	
	tft_signal_inv_set(TFT_VSYNC_INV|TFT_HSYNC_INV, (TFT_ENABLE & TFT_VSYNC_INV)|(TFT_ENABLE & TFT_HSYNC_INV));
	tft_mode_set(TFT_MODE_UPS052);
	tft_data_mode_set(TFT_DATA_MODE_8);
	tft_clk_set(TFT_CLK_DIVIDE_5); /* FS=66 */
#if 0
	spi_cs_by_external_set(SPI_0);
	spi_clk_set(SPI_0,SYSCLK_8); 
#endif
//	tft_tft_en_set(TRUE);
}

void tft_tpo_td025thea7_init(void)
{
	/* 320*240 */

	CSN_n = TFT_CSN_PIN;
	SDA_n = TFT_SDA_PIN;
	SCL_n = TFT_SCL_PIN;		
	
    gpio_init_io (CSN_n, GPIO_OUTPUT);
	gpio_init_io (SCL_n, GPIO_OUTPUT);
	gpio_init_io (SDA_n, GPIO_OUTPUT);
	
	gpio_set_port_attribute(CSN_n, 1);
	gpio_set_port_attribute(SCL_n, 1);
	gpio_set_port_attribute(SDA_n, 1);
	
    serial_cmd_1(0x0800);



	/*
	R_TFT_LINE_RGB_ORDER = 0x00;
	R_TFT_V_PERIOD = 262;
	R_TFT_V_START = 22-1;
	R_TFT_V_END	= 22+240-1;
	
	R_TFT_H_PERIOD = 1560;
	R_TFT_H_START = 240;
	R_TFT_H_END	= 240+1280;
	*/

	R_TFT_HS_WIDTH			= 0;				//	1		=HPW
	R_TFT_H_START			= 1+239;			//	240		=HPW+HBP
	R_TFT_H_END				= 1+239+1280;	//	1520	=HPW+HBP+HDE
	R_TFT_H_PERIOD			= 1+239+1280+40;	//	1560	=HPW+HBP+HDE+HFP
	R_TFT_VS_WIDTH			= 0;				//	1		=VPW				(DCLK)
	R_TFT_V_START			= 21;			//	21		=VPW+VBP			(LINE)
	R_TFT_V_END				= 21+240;		//	261		=VPW+VBP+VDE		(LINE)
	R_TFT_V_PERIOD			= 21+240+1;		//	262		=VPW+VBP+VDE+VFP	(LINE)
	R_TFT_LINE_RGB_ORDER    = 0x00;
	
	tft_signal_inv_set(TFT_VSYNC_INV|TFT_HSYNC_INV, (TFT_ENABLE & TFT_VSYNC_INV)|(TFT_ENABLE & TFT_HSYNC_INV));
	tft_mode_set(TFT_MODE_UPS052);
	tft_data_mode_set(TFT_DATA_MODE_8);
	tft_clk_set(TFT_CLK_DIVIDE_8); /* FS=66 */	
	
/*	
// dennis:old version
	R_TFT_HS_WIDTH			= 0;				//	1		=HPW
	R_TFT_H_START			= 1+239;			//	240		=HPW+HBP
//	R_TFT_H_END				= 1+239+1280;	//	1520	=HPW+HBP+HDE
//	R_TFT_H_PERIOD			= 1+239+1280+40;	//	1560	=HPW+HBP+HDE+HFP
	R_TFT_H_END				= 1+239+1120;	//	1520	=HPW+HBP+HDE
	R_TFT_H_PERIOD			= 1+239+1120+40;	//	1560	=HPW+HBP+HDE+HFP
	R_TFT_VS_WIDTH			= 0;				//	1		=VPW				(DCLK)
	R_TFT_V_START			= 21;			//	21		=VPW+VBP			(LINE)
	R_TFT_V_END				= 21+240;		//	261		=VPW+VBP+VDE		(LINE)
	R_TFT_V_PERIOD			= 21+240+1;		//	262		=VPW+VBP+VDE+VFP	(LINE)
	R_TFT_LINE_RGB_ORDER    = 0x00;
	
	tft_signal_inv_set(TFT_VSYNC_INV|TFT_HSYNC_INV, (TFT_ENABLE & TFT_VSYNC_INV)|(TFT_ENABLE & TFT_HSYNC_INV));
	tft_mode_set(TFT_MODE_UPS052);
	tft_data_mode_set(TFT_DATA_MODE_8);
	tft_clk_set(TFT_CLK_DIVIDE_5); // FS=66 
*/	
#if 0
	spi_cs_by_external_set(SPI_0);
	spi_clk_set(SPI_0,SYSCLK_8); 
#endif
//	tft_tft_en_set(TRUE);
}

void TPO_SPI_WriteCmd1 (INT32S nCmdType, INT32U uCmd)
{
	INT32S	nBits;
	INT32S	i;
	
 	R_FUNPOS1 |= 0x80; //IOB0 not used as DE signal		
	CSN_n = IO_B0;
	SDA_n = IO_C2;
    SCL_n = IO_G7;

	//	Initial 3-wire GPIO interface, Initial SPI
	gpio_set_port_attribute(SDA_n, ATTRIBUTE_HIGH);
	gpio_set_port_attribute(SCL_n, ATTRIBUTE_HIGH);
	gpio_set_port_attribute(CSN_n, ATTRIBUTE_HIGH);
	gpio_init_io(SDA_n, GPIO_OUTPUT);					 	
	gpio_init_io(SCL_n, GPIO_OUTPUT);
	gpio_init_io(CSN_n, GPIO_OUTPUT);	
	gpio_write_io (SDA_n, DATA_LOW);
	gpio_write_io (SCL_n, DATA_LOW);
	gpio_write_io (CSN_n, DATA_HIGH);
	cmd_delay (5);
	
	nBits = (nCmdType + 2) << 3;
	uCmd &= ~((((INT32U)1) << (nBits - 9 * nCmdType)) - 1);
	
	//	Activate CS low to start transaction
	gpio_write_io (CSN_n, DATA_LOW);
	cmd_delay (2);
	for (i=0;i<nBits;i++) {
		//	Activate SPI data
		if ((uCmd & 0x80000000) == 0x80000000) {
			gpio_write_io (SDA_n, DATA_HIGH);
		}
		if ((uCmd & 0x80000000) == DATA_LOW) {
			gpio_write_io (SDA_n, DATA_LOW);
		}
		cmd_delay (2);
		//	Toggle SPI clock
		gpio_write_io (SCL_n, DATA_HIGH);
		cmd_delay (2);
		gpio_write_io (SCL_n, DATA_LOW);
		cmd_delay (2);
		uCmd <<= 1;
	}
	//	Pull low data
	gpio_write_io (SDA_n, DATA_LOW);
	//	Activate CS high to stop transaction
	gpio_write_io (CSN_n, DATA_HIGH);
	cmd_delay (2);
}

void TFT_TXDT240C_5182A_Init(void)
{   	
	//	Reset TFT control register at first
   // R_TFT_CTRL=0;
    
	TPO_SPI_WriteCmd1 (0, 0x000D0000);
	cmd_delay (5);
	TPO_SPI_WriteCmd1 (0, 0x00050000);
	cmd_delay (5);
	TPO_SPI_WriteCmd1 (0, 0x00050000);
	cmd_delay (5);
	TPO_SPI_WriteCmd1 (0, 0x000D0000);
	cmd_delay (5);
	//while(1)
	TPO_SPI_WriteCmd1 (0, 0x60010000);
	cmd_delay (5);
	TPO_SPI_WriteCmd1 (0, 0x40030000);
	TPO_SPI_WriteCmd1 (0, 0xA0080000);
	TPO_SPI_WriteCmd1 (0, 0x50680000);
	
	R_TFT_HS_WIDTH			= 1;				//	1		=HPW
	R_TFT_H_START			= 1+252;			//	240		=HPW+HBP
	R_TFT_H_END				= 1+252+1280;	    //	1520	=HPW+HBP+HDE
	R_TFT_H_PERIOD			= 1+252+1280+28;	//	1560	=HPW+HBP+HDE+HFP
	R_TFT_VS_WIDTH			= 1;				//	1		=VPW				(DCLK)
	
	R_TFT_V_START			= 1+21;				//	21		=VPW+VBP			(LINE)
	R_TFT_V_END				= 1+21+240;		    //	261		=VPW+VBP+VDE		(LINE)
	R_TFT_V_PERIOD			= 1+21+240+9;		//	262		=VPW+VBP+VDE+VFP	(LINE)
	R_TFT_CTRL           	= 0x8319;
	R_TFT_TS_MISC		   &= 0xFFF3FF; 	
}

void tft_T27P05_ILI8961_init(void)
{
	/* 320*240 */

	CSN_n = IO_A11;
	SDA_n = IO_A10;
    SCL_n = IO_A9;

    gpio_init_io (CSN_n, GPIO_OUTPUT);
	gpio_init_io (SCL_n, GPIO_OUTPUT);
	gpio_init_io (SDA_n, GPIO_OUTPUT);
	
	gpio_set_port_attribute(CSN_n, 1);
	gpio_set_port_attribute(SCL_n, 1);
	gpio_set_port_attribute(SDA_n, 1);

	serial_cmd_1(0x055F);
	cmd_delay(5);
	serial_cmd_1(0x051F);//reset
	cmd_delay(10);
	serial_cmd_1(0x055F);
	cmd_delay(50);
	serial_cmd_1(0x2B01);//exit standby mode
	serial_cmd_1(0x0009);//VCOMAC  
	serial_cmd_1(0x019F);//VCOMDC  

	serial_cmd_1(0x0368);//brightness 
	serial_cmd_1(0x0D40);//contrast 
	
	serial_cmd_1(0x041B);//8-bit RGB interface//0x0409//0x041B
	serial_cmd_1(0x1604);//Default Gamma setting  2.2 
	//serial_cmd_1(0x2f71);//Default Gamma setting  2.2 
 


	R_TFT_HS_WIDTH			= 0;				//	1		=HPW
	R_TFT_H_START			= 1+240;			//	240		=HPW+HBP
	R_TFT_H_END				= 1+240+1280;	//	1520	=HPW+HBP+HDE
	R_TFT_H_PERIOD			= 1+240+1280+39;	//	1560	=HPW+HBP+HDE+HFP
	R_TFT_VS_WIDTH			= 0;				//	1		=VPW				(DCLK)
	R_TFT_V_START			= 21;			//	21		=VPW+VBP			(LINE)
	R_TFT_V_END				= 21+240;		//	261		=VPW+VBP+VDE		(LINE)
	R_TFT_V_PERIOD			= 21+240+1;		//	262		=VPW+VBP+VDE+VFP	(LINE)
	R_TFT_LINE_RGB_ORDER    = 0x00;
	
	tft_signal_inv_set(TFT_VSYNC_INV|TFT_HSYNC_INV, (TFT_ENABLE & TFT_VSYNC_INV)|(TFT_ENABLE & TFT_HSYNC_INV));
	tft_mode_set(TFT_MODE_UPS052);
	tft_data_mode_set(TFT_DATA_MODE_8);
	AP_TFT_ClK_144M_set();

	tft_tft_en_set(TRUE);
}

#if USE_PANEL_NAME ==  PANEL_T15P06
static int sent_command(unsigned char adr,unsigned char value)
{
	int   i=16;

   
	gpio_write_io(CSN_n,1);//CS=1
	gpio_write_io(SCL_n,1);//SCL=0
	gpio_write_io(SDA_n,1);//SDA
	

	gpio_write_io(CSN_n,0);//CS=0
		cmd_delay(2);

	for(i=0;i<16;i++)
	{
		if(i<5)
		{
			
			{
				if(adr&0x80)
					gpio_write_io(SDA_n,1);
				else
					gpio_write_io(SDA_n,0);
				
				adr = adr << 1;
			}
			
			cmd_delay(2);
			gpio_write_io(SCL_n,0);
			cmd_delay(2);
			gpio_write_io(SCL_n,1);
			cmd_delay(2);
		}
		else if((i>=5)&&(i<=7))
		{
			gpio_write_io(SDA_n,0);

			cmd_delay(2);
			gpio_write_io(SCL_n,0);
			cmd_delay(2);
			gpio_write_io(SCL_n,1);
			cmd_delay(2);
		}
		else
		{
			if(value&0x80)
				gpio_write_io(SDA_n,1);
			else
				gpio_write_io(SDA_n,0);

			value = value <<1;

			cmd_delay(2);
			gpio_write_io(SCL_n,0);
			cmd_delay(2);
			gpio_write_io(SCL_n,1);
			cmd_delay(2);
		}
		
	}
	cmd_delay(500);
    gpio_write_io(CSN_n,1);
	cmd_delay(500);

	return 0;
}

void tft_T15P06_init(void)
{
	/* 320*240 */

	CSN_n = IO_A11;
	SDA_n = IO_A10;
    SCL_n = IO_A9;

    gpio_init_io (CSN_n, GPIO_OUTPUT);
	gpio_init_io (SCL_n, GPIO_OUTPUT);
	gpio_init_io (SDA_n, GPIO_OUTPUT);
	
	gpio_set_port_attribute(CSN_n, 1);
	gpio_set_port_attribute(SCL_n, 1);
	gpio_set_port_attribute(SDA_n, 1);

	sent_command(0x00,0x0f);    	
	sent_command(0x00,0x05);
	cmd_delay(5000);
	sent_command(0x00,0x0f);  
	sent_command(0x00,0x05);
	cmd_delay(5000);
    sent_command(0x00,0x0f);     
	sent_command(0x50,0x00);

	sent_command(0x20,0x03);
	sent_command(0x30,0x08);

	sent_command(0x40,0x03);
	 
	sent_command(0x70,0x40);
	sent_command(0xc0,0x05);   
	sent_command(0xe0,0x13); 
	sent_command(0x60,0x01); 
 

	#if 0
	R_TFT_HS_WIDTH			= 0;				//	1		=HPW
	R_TFT_H_START			= 1+240;			//	240		=HPW+HBP
	R_TFT_H_END				= 1+240+1280;	//	1520	=HPW+HBP+HDE
	R_TFT_H_PERIOD			= 1+240+1280+39;	//	1560	=HPW+HBP+HDE+HFP
	R_TFT_VS_WIDTH			= 0;				//	1		=VPW				(DCLK)
	R_TFT_V_START			= 21;			//	21		=VPW+VBP			(LINE)
	R_TFT_V_END				= 21+240;		//	261		=VPW+VBP+VDE		(LINE)
	R_TFT_V_PERIOD			= 21+240+1;		//	262		=VPW+VBP+VDE+VFP	(LINE)
	R_TFT_LINE_RGB_ORDER    = 0x00;
	#else
	R_TFT_HS_WIDTH			= 4;				//	1		=HPW
	R_TFT_H_START			= 4+253;			//	240		=HPW+HBP
	R_TFT_H_END				= 4+253+1280;	//	1520	=HPW+HBP+HDE
	R_TFT_H_PERIOD			= 4+253+1280+56;	//	1560	=HPW+HBP+HDE+HFP
	R_TFT_VS_WIDTH			= 3;				//	1		=VPW				(DCLK)
	R_TFT_V_START			= 3+13;			//	21		=VPW+VBP			(LINE)
	R_TFT_V_END				= 3+13+240;		//	261		=VPW+VBP+VDE		(LINE)
	R_TFT_V_PERIOD			= 3+13+240+9;		//	262		=VPW+VBP+VDE+VFP	(LINE)
	R_TFT_LINE_RGB_ORDER    = 0x00;
	#endif
	
	tft_signal_inv_set(TFT_VSYNC_INV|TFT_HSYNC_INV, (TFT_ENABLE & TFT_VSYNC_INV)|(TFT_ENABLE & TFT_HSYNC_INV));
	tft_mode_set(TFT_MODE_UPS052);
	tft_data_mode_set(TFT_DATA_MODE_8);
	AP_TFT_ClK_144M_set();

	tft_tft_en_set(TRUE);
}
#endif

#if (USE_PANEL_NAME == PANEL_T43)
void tft_T43_init(void)
{
	/* 480*272 */


	R_TFT_HS_WIDTH			= 1;				//	1		=HPW
	R_TFT_H_START			= 1+44;			//	240		=HPW+HBP
	R_TFT_H_END				= 1+44+1440;	//	1520	=HPW+HBP+HDE
	R_TFT_H_PERIOD			= 1+44+1440+168;	//	1560	=HPW+HBP+HDE+HFP
	R_TFT_VS_WIDTH			= 10;				//	1		=VPW				(DCLK)
	R_TFT_V_START			= 10+1;			//	21		=VPW+VBP			(LINE)
	R_TFT_V_END				= 10+1+272;		//	261		=VPW+VBP+VDE		(LINE)
	R_TFT_V_PERIOD			= 10+1+272+3;		//	262		=VPW+VBP+VDE+VFP	(LINE)
	R_TFT_LINE_RGB_ORDER    = 0x00;

	tft_signal_inv_set(TFT_VSYNC_INV|TFT_HSYNC_INV,0/* (TFT_ENABLE & TFT_VSYNC_INV)|(TFT_ENABLE & TFT_HSYNC_INV)*/);
	tft_mode_set(TFT_MODE_UPS051);
	tft_data_mode_set(TFT_DATA_MODE_8);
	AP_TFT_ClK_144M_set();

	tft_tft_en_set(TRUE);
}

#endif
//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#endif //(defined _DRV_L1_TFT) && (_DRV_L1_TFT == 1)                   //
//================================================================//
