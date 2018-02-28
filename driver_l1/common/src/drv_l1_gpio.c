/******************************************************************************
 * Purpose: GPIO driver/interface  
 * Author: Dominant Yang
 * Date: 2008/01/21
 * Copyright Generalplus Corp. ALL RIGHTS RESERVED.
 ******************************************************************************/
 
/******************************************************************************
 * Paresent Header Include
 ******************************************************************************/
#include "drv_l1_gpio.h"

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#if (defined _DRV_L1_GPIO) && (_DRV_L1_GPIO == 1)                 //
//================================================================//

/******************************************************************************
 * Static Variables
 ******************************************************************************/ 
/*把计程竚 internal ram*/
static INT16U bit_mask_array[16];
static INT16U bit_mask_inv_array[16];
static INT32U gpio_data_out_addr[8];
static INT32U gpio_data_in_addr[8];
static INT8U  gpio_set_array[128];
static INT8U  gpio_setbit_array[128];
/*把计程竚 internal ram*/

/******************************************************************************
 * Function Prototypes
 ******************************************************************************/
void gpio_init(void);
//void gpio_pad_init(void);
void gpio_pad_init2(void);
BOOLEAN gpio_init_io(INT32U port, BOOLEAN direction);
BOOLEAN gpio_read_io(INT32U port);
BOOLEAN gpio_write_io(INT32U port, BOOLEAN data);
BOOLEAN gpio_set_port_attribute(INT32U port, BOOLEAN attribute);
BOOLEAN gpio_get_dir(INT32U port);
BOOLEAN gpio_drving_init_io(GPIO_ENUM port, IO_DRV_LEVEL gpio_driving_level);
void gpio_drving_init(void);
void gpio_drving_uninit(void);

/******************************************************************************
 * Function Body
 ******************************************************************************/
void gpio_IOE_switch_config_from_HDMI_to_GPIO(void)
{
	INT32U HDMI_CTRL = R_SYSTEM_HDMI_CTRL;
	INT32U CLK_EN0 = R_SYSTEM_CLK_EN0;
	R_SYSTEM_HDMI_CTRL &= (~0x8000);	// must
	R_SYSTEM_CLK_EN0  |= 0x8;			// must

	R_SYSTEM_CKGEN_CTRL |= 0x4;
	R_SYSTEM_PLLEN |= 0x80;
	R_HDMITXPHYCONFIG1 |= 0x1;
	R_SYSTEM_CKGEN_CTRL &= (~0x4);
	R_SYSTEM_PLLEN &= (~0x80);

	R_SYSTEM_HDMI_CTRL = HDMI_CTRL;
	R_SYSTEM_CLK_EN0 = CLK_EN0;
}

void gpio_sdram_swith(INT32U port, BOOLEAN data)
{
  INT16U i;
  i =0x00001;
  i= i << port ;
  if (data)
  {
	 R_FUNPOS1 |= i;
  } else
  {
  	 R_FUNPOS1 &= ~i;
  }	 
}
void gpio_init(void)
{
    INT16U i;


    /* initial all gpio */
    for (i=0; i<8;i++)
    {
      //   DRV_Reg32(IOA_ATTRIB_ADDR+(0x20*i)) = 0xffff;  /*Set all gpio attribute to 1 */
         gpio_data_out_addr[i] = (0xC0000004 + (0x20*i));
         gpio_data_in_addr[i] = (0xC0000000 + (0x20*i));
    }
    
    for (i=0; i<16; i++)
    {
        bit_mask_array[i]=1<<i;
        bit_mask_inv_array[i]=~(bit_mask_array[i]);
    }

    for (i=0; i<128; i++)
    {
        gpio_set_array[i]=i/16;
        gpio_setbit_array[i]=i%16;
    }


    gpio_IOE_switch_config_from_HDMI_to_GPIO();    
    /* initial gpio pad */
    gpio_pad_init2();
    /* initial gpio pad driving */
    gpio_drving_init();
    
     gpio_init_io(POWER_EN_PIN, GPIO_OUTPUT);
     gpio_set_port_attribute(POWER_EN_PIN, ATTRIBUTE_HIGH);
     gpio_write_io(POWER_EN_PIN, DATA_HIGH);
}

void gpio_pad_init2(void)
{
    /*initial temp register*/
    INT32U mem_io_ctrl=R_MEM_IO_CTRL;
    INT32U funpos0=R_FUNPOS0;
    INT32U funpos1=R_FUNPOS1;
    INT32U iosrsel=R_IOSRSEL;
    INT32U sysmonictrl=R_SYSMONICTRL;
    INT32U pwm_ctrl=R_PWM_CTRL;
	INT32U funpos2=R_FUNPOS2;
/* BKCS Pad Initial START */
#if (defined BKCS_0_EN) && (BKCS_0_EN == TRUE)
   mem_io_ctrl &= ~(1<<11);   //MEMC0 Disable and become gpio
#endif

#if (defined BKCS_1_EN) && (BKCS_1_EN == TRUE)
   mem_io_ctrl &= ~(1<<12);   //MEMC1 Disable and become gpio
#endif

#if (defined BKCS_2_EN) && (BKCS_2_EN == TRUE)
   mem_io_ctrl &= ~(1<<13);   //MEMC2 Disable and become gpio
#endif

#if (defined BKCS_3_EN) && (BKCS_3_EN == TRUE)
   mem_io_ctrl &= ~(1<<14);   //MEMC3 Disable and become gpio 
#endif
R_MEM_IO_CTRL = mem_io_ctrl;
/* BKCS Pad Initial END */

/* Nand Pad Dispacher START */
#if (_DRV_L1_NAND==1)
   #if (defined NAND_PAGE_TYPE) && (NAND_PAGE_TYPE==NAND_SMALLBLK)  // different page type will decision nand cs out
       #if NAND_CS_POS == NF_CS_AS_BKCS1
          nand_small_page_cs_pin_reg(NAND_CS1);
       #elif NAND_CS_POS == NF_CS_AS_BKCS2
          nand_small_page_cs_pin_reg(NAND_CS2);
       #elif NAND_CS_POS == NF_CS_AS_BKCS3
          nand_small_page_cs_pin_reg(NAND_CS3);
       #endif
   #else
       #if NAND_CS_POS == NF_CS_AS_BKCS1
          funpos0 &= ~(1<<5|1<<4); 
          nand_large_page_cs_reg(NAND_CS1);
       #elif NAND_CS_POS == NF_CS_AS_BKCS2
          funpos0 &= ~(1<<5); funpos0 |= (1<<4);
          nand_large_page_cs_reg(NAND_CS2);
       #elif NAND_CS_POS == NF_CS_AS_BKCS3
          funpos0 |= (1<<5); funpos0 &= ~(1<<4);
          nand_large_page_cs_reg(NAND_CS3);
       #endif
   #endif
   
   #if (defined NAND_WP_IO) && (NAND_WP_IO!=NAND_WP_PIN_NONE)
       nand_wp_pin_reg(NAND_WP_IO);
   #endif
   
   #if NAND_SHARE_MODE == NF_NON_SHARE  /* only effect in nand "non-shard with SDRAM" MODE */
       #if NAND_DATA5_0_POS==NAND_DATA5_0_AS_IOB13_8
           funpos0 &= ~(1<<9|1<<8);                  /*(Fun_POS[9:8] == 2'h0)*/
       #elif NAND_DATA5_0_POS==NAND_DATA5_0_AS_IOD5_0
           funpos0 &= ~(1<<9);  funpos0 |= (1<<8); /*(Fun_POS[9:8] == 2'h1)*/
       #elif NAND_DATA5_0_POS==NAND_DATA5_0_AS_IOE5_0
           funpos0 |= (1<<9);  funpos0 &= ~(1<<8); /*(Fun_POS[9:8] == 2'h2)*/
       #elif NAND_DATA5_0_POS==NAND_DATA5_0_AS_IOA13_8  /* force mode to modify NF_DATA0~7 to IOA8~15*/
           sysmonictrl |= IOSYSMONICTRL_NF_DATA0_7_FORCE_TO_IOA8_15;
       #endif
       
       #if NAND_DATA7_6_POS==NAND_DATA7_6_AS_IOB15_14
           funpos0 &= ~(1<<7|1<<6);                  /*(Fun_POS[7:6] == 2'h0) */
       #elif NAND_DATA7_6_POS==NAND_DATA7_6_AS_IOD7_6
           funpos0 &= ~(1<<7);  funpos0 |= (1<<6); /*((Fun_POS[7:6] == 2'h1)*/
       #elif NAND_DATA7_6_POS==NAND_DATA7_6_AS_IOE7_6
           funpos0 |= (1<<7);  funpos0 &= ~(1<<6); /*((Fun_POS[7:6] == 2'h2)*/
       #elif NAND_DATA7_6_POS==NAND_DATA7_6_AS_IOC5_4
           funpos0 &= ~(1<<7);  funpos0 |= (1<<6); /*((Fun_POS[7:6] == 2'h3)*/
       #elif NAND_DATA7_6_POS==NAND_DATA7_6_AS_IOA15_14  /* force mode to modify NF_DATA0~7 to IOA8~15*/
           sysmonictrl |= IOSYSMONICTRL_NF_DATA0_7_FORCE_TO_IOA8_15;
           #if NAND_DATA5_0_POS != NAND_DATA5_0_AS_IOA13_8
               #error  
           #endif
       #endif
   #endif //NAND_SHARE_MODE == NF_NON_SHARE
#endif //(_DRV_L1_NAND==1)

#if (_DRV_L1_NAND == 1)
     #if (NAND_SHARE_MODE == NF_NON_SHARE)  /* in non-share mode have 2 choice */ 
         #if NAND_CTRL_POS==NF_ALE_IOC12__CLE_IOC13__REB_IOC14__WEB_IOC15
             funpos0 &= ~(1<<3);
             iosrsel &= ~(1<<14);
         #elif NAND_CTRL_POS==NF_ALE_IOC6__CLE_IOC7__REB_IOC8__WEB_IOC9 
             funpos0 |= (1<<3);
             iosrsel &= ~(1<<14);
         #endif
     #elif (NAND_SHARE_MODE == NF_SHARE_MODE) /* in share mode only 2 choices (and sd ram share set)*/
         #if NAND_CTRL_POS==NF_ALE_IOG5__CLE_IOG6__REB_IOG10__WEB_IOG11  
             /* Release MEMA20 & MEMA19 for nand CTRL ALE/CLE */
             iosrsel |= IOSRSEL_NF_ALE_CLE_REB_WEB_IOG5_IOG6_IOG10_IOG11;
         #else
             iosrsel =0;
         #endif
     #endif
#endif  //(_DRV_L1_NAND==1)

#if (_DRV_L1_NAND == 1)   /* Only Nand1 */
    #if (NAND_SHARE_MODE == NF_SHARE_MODE)  
        R_MEM_SDRAM_MISC|=0x10;  /* Disable SDRAM Preload process to avoid system crash */
        //R_NF_CTRL = 0x2000;  /* Enable Nand Share Pad (with SDRAM) */  
        R_MEM_M11_BUS_PRIORITY = 0x0004 ;//shin add for adding others SDRAM bandwidth beside NF
        nand_share_mode_reg(NF_SHARE_PIN_MODE);        
    #elif (NAND_SHARE_MODE == NF_NON_SHARE)
        nand_share_mode_reg(NF_NON_SHARE_MODE);
    #endif

#endif

/* NAND pad Dispacher END*/

/* SD Pad Dispacher START */
	funpos0 &= (~0x1E00000); // Fun_POS bit[24:21]
	switch(SD_POS)
	{
		case SDC0_IOA2_IOA3_IOA4_IOA5_IOA6_IOA7:
			funpos0 |= 0x0000000;
			R_IOA_DIR &= ~0x00F4;
			R_IOA_ATT &= ~0x00F4;
			R_IOA_O_DATA |= 0x00F4;
			break;
		case SDC0_IOB14_IOB15_IOB10_IOB11_IOB12_IOB13:
			funpos0 |= 0x0200000;
			R_IOB_DIR &= ~0x7C00;
			R_IOB_ATT &= ~0x7C00;
			R_IOB_O_DATA |= 0x7C00;
			break;
		case SDC0_IOC6_IOC7_IOC8_IOC9_IOC10_IOC11:
			funpos0 |= 0x0400000;
			R_IOC_DIR &= ~0x0F40;
			R_IOC_ATT &= ~0x0F40;
			R_IOC_O_DATA |= 0x0F40;			
			break;
		case SDC1_IOB0_IOB1_IOB2_IOB3_IOB4_IOB5:
			funpos0 |= 0x0800000;
			R_IOB_DIR &= ~0x003D;
			R_IOB_ATT &= ~0x003D;
			R_IOB_O_DATA |= 0x003D;
			break;
		case SDC1_IOD10_IOD11_IOC13_IOD13_IOC14_IOC15:			
			funpos0 |= 0x1000000;
			R_IOD_DIR &= ~0x2400;
			R_IOD_ATT &= ~0x2400;
			R_IOD_O_DATA |= 0x2C00;
			R_IOC_DIR &= ~0xE000;
			R_IOC_ATT &= ~0xE000;
			R_IOC_O_DATA |= 0xE000;
			break;
		case SDC1_IOD10_IOD11_IOD12_IOD13_IOD14_IOD15:
		default:
			// input pull high IOD10,12,13,14,15
			funpos0 |= 0x0000000;
			R_IOD_DIR &= ~0xF400;
			R_IOD_ATT &= ~0xF400;
			R_IOD_O_DATA |= 0xF400;	
	}
/* SD Pad Dispacher END */

/* UART Pad Dispacher START */
	funpos0 &= ~(0xF);		// Fun_POS bit[3:0]
	switch(UART_TX_RX_POS)
	{
		case UART0_TX_IOB5__RX_IOB4:
			R_IOB_DIR &= ~0x0010;
			R_IOB_ATT &= ~0x0010;
			R_IOB_O_DATA |= 0x0010;
			funpos0 |= 0x00000000;
			break;
		case UART0_TX_IOC12__RX_IOC13:
			R_IOC_DIR &= ~0x2000;
			R_IOC_ATT &= ~0x2000;
			R_IOC_O_DATA |= 0x2000;
			funpos0 |= 0x00000001;
			break;
		case UART0_TX_IOD5__RX_IOD4:
			R_IOD_DIR &= ~0x0010;
			R_IOD_ATT &= ~0x0010;
			R_IOD_O_DATA |= 0x0010;
			funpos0 |= 0x00000002;
			break;
		case UART1_TX_IOB7__RX_IOB6:
			R_IOB_DIR &= ~0x0040;
			R_IOB_ATT &= ~0x0040;
			R_IOB_O_DATA |= 0x0040;
			funpos0 |= 0x00000000;			
			break;
		case UART1_TX_IOC15__RX_IOC14:
			R_IOC_DIR &= ~0x4000;
			R_IOC_ATT &= ~0x4000;
			R_IOC_O_DATA |= 0x4000;
			funpos0 |= 0x00000004;
			break;
		case UART1_TX_IOD9__RX_IOD8:
			R_IOD_DIR &= ~0x0100;
			R_IOD_ATT &= ~0x0100;
			R_IOD_O_DATA |= 0x0100;
			funpos0 |= 0x00000008;
			break;
		case UART1_TX_IOD15__RX_IOD14:
			R_IOD_DIR &= ~0x4000;
			R_IOD_ATT &= ~0x4000;
			R_IOD_O_DATA |= 0x4000;			
			funpos0 |= 0x0000000C;			
			break;
		case UART_TX_NONE__RX_NONE:
		default:
			funpos0 |= 0x00000000;
			// rx pull hign
	}
/* UART Pad Dispacher END */


/* CSI Pad Dispacher START */
	funpos1 &= ~(0x1F);		// Fun_POS1  bit[4:0]
	switch (CMOS_CLK0_POS)
	{
		case CMOS_CLK0__IOD9:
			funpos1 |= 2;			
			break;
		case CMOS_CLK0__IOD7:
			funpos1 |= 1;			
			break;
		case CMOS_CLK0__IOC9:
		default:
			funpos1 |= 0;
	}
	switch (CMOS_CLK1_HSYNC_VSTNC_POS)
	{
		case CMOS_CLK1_HSYNC_VSYNC__IOD6_IOD8_IOD9:
			funpos1 |= 4;
			break;
		case CMOS_CLK1_HSYNC_VSYNC__IOC8_IOC10_IOC11:
		default:
			funpos1 |= 0;
	}
	switch (CMOS_DATA2_9_POS)
	{
		case CMOS_DATA2_9__IOE0_7:
			funpos1 |= 0x10;
			break;
		case CMOS_DATA2_9__IOB8_15:
			funpos1 |= 0x08;
			break;
		case CMOS_DATA2_9__IOC0_7:
		default:
			funpos1 |= 0;
	}
/* CSI Pad Dispacher END */


/* SPI (second) Pad Dispacher START */
	funpos0 &= (~0x0000C000); // Fun_POS bit[15:14]
	switch (SPI0_POS)
	{
		case SPI0_RX_IOC11__CLK_IOC9__TX_IOC10:
			funpos0 |= (1<<14);
			break;
		case SPI0_RX_IOD13__CLK_IOD11__TX_IOD12:
			funpos0 |= (2<<14);
			break;
		case SPI0_RX_IOD9__CLK_IOD7__TX_IOD8:
		default:
			funpos0 |= 0;
	}
 /* SPI Pad Dispacher END */


/* TFT Pad Dispacher START */
	funpos0 &= ~(0x3800);		// Fun_POS bit[13:11]
	switch(TFT_DATA0_7_POS)
	{
		case TFT_DATA0_7__IOE0_7:
			funpos0 |= 0x00000800;
			break;
		case TFT_DATA0_7__IOA0_7:
		default:			
			funpos0 |= 0x00000000;
	}
	switch(TFT_DATA8_15_POS)
	{
		case TFT_DATA8_15__IOE0_7:
			funpos0 |= 0x00001000;
			break;
		case TFT_DATA8_15__IOA8_15:
		default:			
			funpos0 |= 0x00000000;
	}
	switch(TFT_CTRL_POS)
	{
		case TFT_DE_HSYNC_VSYNC_CLK__IOC8_IOC9_IOC10_IOC11:
			funpos0 |= 0x00002000;
			break;
		case TFT_DE_HSYNC_VSYNC_CLK_TE__IOB0_IOB1_IOB2_IOB3_IOB4:
		default:			
			funpos0 |= 0x00000000;
	}
/* TFT Pad Dispacher END */

//External interrupt A~C Pad Dispacher START
//EXT_in_IOB = (Fun_POS[10:9]==2'b00);
//EXT_in_IOC = (Fun_POS[10:9]==2'b01);
//EXT_in_IOD_POS0 = (Fun_POS[10:9]==2'b10);
//EXT_in_IOD_POS1 = (Fun_POS[10:9]==2'b11);
//=========================
    funpos0 &= ~(0x600);		// Fun_POS1  bit[10:9]
	switch (EXT_INT_POS)
	{
		case EXT_INT_ABC_IOC13_15:
			funpos0 |= 0x00000200;			
			break;
		case EXT_INT_ABC_IOD7_9:
			funpos0 |= 0x00000400;		
			break;
		case EXT_INT_ABC_IOD12_14_15:
			funpos0 |= 0x00000600;			
			break;
		case EXT_INT_ABC_IOB8_10:
		default:
			funpos0 |= 0x00000000;
	}
//==========================
//External interrupt A~C Pad Dispacher END

#if TIMER_C_PWM_EN == TRUE
    pwm_ctrl &= ~(1<<11);  /* PWM1 Pin No use */
//  pwm_ctrl &= ~(1<<12);  /* FB1 Pin No use */
//  pwm_ctrl &= ~(1<<13);  /* VC1 Pin No use */
//  pwm_ctrl &= ~(1<<2);   /* PWM1 Function Disable */
#endif
/* Timer PWM Pad END */

/* I2C Pad Dispacher START */
	funpos0 &= ~(0x7 << 25);
	funpos0 |= I2C_POS << 25;
/* I2C Pad Dispacher END */

/* fill register */
R_FUNPOS0 = funpos0;
R_FUNPOS1 = funpos1;
R_IOSRSEL = iosrsel;
R_SYSMONICTRL = sysmonictrl;
R_PWM_CTRL = pwm_ctrl;
R_FUNPOS2 = funpos2;
	
	
#if SDRAM_SIZE == 0x00200000
	R_MEM_IO_CTRL = 0x8C61;			// Release XA12, XA13 and XA14 as GPIO
#elif SDRAM_SIZE==0x00800000 || SDRAM_SIZE==0x01000000
	R_MEM_IO_CTRL = 0x8F61;			// Release XA12 as GPIO
#else
	R_MEM_IO_CTRL = 0x8FE1;
#endif

}

void gpio_drving_init(void)
{
    INT8U i;
    gpio_drving_uninit();
    
/* TFT Pad Driving START */
	switch(TFT_DATA0_7_POS)
	{
		case TFT_DATA0_7__IOE0_7:
			for (i=IO_E0 ; i<=IO_E7; i++)
			{
				gpio_drving_init_io(i,(IO_DRV_LEVEL) TFT_DATA0_7_DRIVING);
			}				
			break;
		case TFT_DATA0_7__IOA0_7:
		default:
			for (i=IO_A0 ; i<=IO_A7; i++)
			{
				gpio_drving_init_io(i,(IO_DRV_LEVEL) TFT_DATA0_7_DRIVING);
			}		
	}
	switch(TFT_DATA8_15_POS)
	{
		case TFT_DATA8_15__IOE0_7:
			for (i=IO_E0 ; i<=IO_E7; i++)
			{
				gpio_drving_init_io(i,(IO_DRV_LEVEL) TFT_DATA8_15_DRIVING);
			}
			break;
		case TFT_DATA8_15__IOA8_15:
		default:
			for (i=IO_A8 ; i<=IO_A15; i++)
			{
				gpio_drving_init_io(i,(IO_DRV_LEVEL) TFT_DATA8_15_DRIVING);
			}
	}
	switch(TFT_CTRL_POS)
	{
		case TFT_DE_HSYNC_VSYNC_CLK__IOC8_IOC9_IOC10_IOC11:
			gpio_drving_init_io(IO_C8,(IO_DRV_LEVEL) TFT_CTRL_DRIVING);
			gpio_drving_init_io(IO_C9,(IO_DRV_LEVEL) TFT_CTRL_DRIVING);
			gpio_drving_init_io(IO_C10,(IO_DRV_LEVEL) TFT_CTRL_DRIVING);
			gpio_drving_init_io(IO_C11,(IO_DRV_LEVEL) TFT_CTRL_DRIVING);
			break;
		case TFT_DE_HSYNC_VSYNC_CLK_TE__IOB0_IOB1_IOB2_IOB3_IOB4:
		default:
			gpio_drving_init_io(IO_B0,(IO_DRV_LEVEL) TFT_CTRL_DRIVING);
			gpio_drving_init_io(IO_B1,(IO_DRV_LEVEL) TFT_CTRL_DRIVING);
			gpio_drving_init_io(IO_B2,(IO_DRV_LEVEL) TFT_CTRL_DRIVING);
			gpio_drving_init_io(IO_B3,(IO_DRV_LEVEL) TFT_CTRL_DRIVING);
			gpio_drving_init_io(IO_B4,(IO_DRV_LEVEL) TFT_CTRL_DRIVING);
	}
/* TFT Pad Driving END */


/* SPI (second) Pad Dispacher START */
 /* SPI Pad Dispacher END */


/* CSI Pad Dispacher START */
	switch (CMOS_CLK0_POS)
	{
		case CMOS_CLK0__IOD9:
			gpio_drving_init_io(IO_D9,(IO_DRV_LEVEL) CMOS_DRIVING);
			break;
		case CMOS_CLK0__IOD7:
			gpio_drving_init_io(IO_D7,(IO_DRV_LEVEL) CMOS_DRIVING);
			break;
		case CMOS_CLK0__IOC9:
		default:
			gpio_drving_init_io(IO_C9,(IO_DRV_LEVEL) CMOS_DRIVING);
	}
	switch (CMOS_CLK1_HSYNC_VSTNC_POS)
	{
		case CMOS_CLK1_HSYNC_VSYNC__IOD6_IOD8_IOD9:
			gpio_drving_init_io(IO_D8,(IO_DRV_LEVEL) CMOS_DRIVING);
			gpio_drving_init_io(IO_D9,(IO_DRV_LEVEL) CMOS_DRIVING);
			break;
		case CMOS_CLK1_HSYNC_VSYNC__IOC8_IOC10_IOC11:
		default:
			gpio_drving_init_io(IO_C10,(IO_DRV_LEVEL) CMOS_DRIVING);
			gpio_drving_init_io(IO_C11,(IO_DRV_LEVEL) CMOS_DRIVING);			
	}
/* CSI Pad Dispacher END */


/* UART Pad Dispacher START */  
	switch(UART_TX_RX_POS)
	{
		case UART0_TX_IOB5__RX_IOB4:
			gpio_drving_init_io(IO_B4,(IO_DRV_LEVEL) UART_TX_RX_DRIVING);
			gpio_drving_init_io(IO_B5,(IO_DRV_LEVEL) UART_TX_RX_DRIVING);
			break;
		case UART0_TX_IOC12__RX_IOC13:
			gpio_drving_init_io(IO_C12,(IO_DRV_LEVEL) UART_TX_RX_DRIVING);
			gpio_drving_init_io(IO_C13,(IO_DRV_LEVEL) UART_TX_RX_DRIVING);
			break;
		case UART0_TX_IOD5__RX_IOD4:
			gpio_drving_init_io(IO_D4,(IO_DRV_LEVEL) UART_TX_RX_DRIVING);
			gpio_drving_init_io(IO_D5,(IO_DRV_LEVEL) UART_TX_RX_DRIVING);
			break;
		case UART1_TX_IOB7__RX_IOB6:
			gpio_drving_init_io(IO_B6,(IO_DRV_LEVEL) UART_TX_RX_DRIVING);
			gpio_drving_init_io(IO_B7,(IO_DRV_LEVEL) UART_TX_RX_DRIVING);
			break;
		case UART1_TX_IOC15__RX_IOC14:
			gpio_drving_init_io(IO_C14,(IO_DRV_LEVEL) UART_TX_RX_DRIVING);
			gpio_drving_init_io(IO_C15,(IO_DRV_LEVEL) UART_TX_RX_DRIVING);
			break;
		case UART1_TX_IOD9__RX_IOD8:
			gpio_drving_init_io(IO_D8,(IO_DRV_LEVEL) UART_TX_RX_DRIVING);
			gpio_drving_init_io(IO_D9,(IO_DRV_LEVEL) UART_TX_RX_DRIVING);
			break;
		case UART1_TX_IOD15__RX_IOD14:
			gpio_drving_init_io(IO_D14,(IO_DRV_LEVEL) UART_TX_RX_DRIVING);
			gpio_drving_init_io(IO_D15,(IO_DRV_LEVEL) UART_TX_RX_DRIVING);
			break;
	}
/* UART Pad Dispacher END */


/* SD Pad Dispacher START */
	switch(SD_POS)
	{
		case SDC0_IOA2_IOA3_IOA4_IOA5_IOA6_IOA7:
			gpio_drving_init_io(IO_A2,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_A3,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_A4,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_A5,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_A6,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_A7,(IO_DRV_LEVEL) SD_DRIVING);
			break;
		case SDC0_IOB14_IOB15_IOB10_IOB11_IOB12_IOB13:
			gpio_drving_init_io(IO_B14,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_B15,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_B10,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_B11,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_B12,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_B13,(IO_DRV_LEVEL) SD_DRIVING);
			break;
		case SDC0_IOC6_IOC7_IOC8_IOC9_IOC10_IOC11:
			gpio_drving_init_io(IO_C6,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_C7,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_C8,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_C9,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_C10,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_C11,(IO_DRV_LEVEL) SD_DRIVING);
			break;
		case SDC1_IOB0_IOB1_IOB2_IOB3_IOB4_IOB5:
			gpio_drving_init_io(IO_B0,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_B1,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_B2,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_B3,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_B4,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_B5,(IO_DRV_LEVEL) SD_DRIVING);
			break;
		case SDC1_IOD10_IOD11_IOC13_IOD13_IOC14_IOC15:
			gpio_drving_init_io(IO_D10,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_D11,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_C13,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_D13,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_C14,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_C15,(IO_DRV_LEVEL) SD_DRIVING);
			break;
		case SDC1_IOD10_IOD11_IOD12_IOD13_IOD14_IOD15:
		default:
			gpio_drving_init_io(IO_D10,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_D11,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_D12,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_D13,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_D14,(IO_DRV_LEVEL) SD_DRIVING);
			gpio_drving_init_io(IO_D15,(IO_DRV_LEVEL) SD_DRIVING);
	}
/* SD Pad Dispacher END */


/* Nand Pad Dispacher START */
#if (_DRV_L1_NAND==1)

#if NAND_CS_POS == NF_CS_AS_BKCS1
   gpio_drving_init_io(IO_F1,(IO_DRV_LEVEL) NAND_CS_DRIVING);
#elif NAND_CS_POS == NF_CS_AS_BKCS2
   gpio_drving_init_io(IO_F2,(IO_DRV_LEVEL) NAND_CS_DRIVING);
#elif NAND_CS_POS == NF_CS_AS_BKCS3
   gpio_drving_init_io(IO_F3,(IO_DRV_LEVEL) NAND_CS_DRIVING);
#endif

#if NAND_SHARE_MODE == NF_NON_SHARE  /* only effect in nand "non-shard with SDRAM" MODE */
    #if NAND_DATA5_0_POS==NAND_DATA5_0_AS_IOB13_8
        for (i=IO_B8 ; i<=IO_B13; i++)
        {
            gpio_drving_init_io(i,(IO_DRV_LEVEL) NAND_DATA_DRIVING);
        }
    #elif NAND_DATA5_0_POS==NAND_DATA5_0_AS_IOD5_0
        for (i=IO_D0 ; i<=IO_D5; i++)
        {
            gpio_drving_init_io(i,(IO_DRV_LEVEL) NAND_DATA_DRIVING);
        }
    #elif NAND_DATA5_0_POS==NAND_DATA5_0_AS_IOE5_0
        for (i=IO_E0 ; i<=IO_E5; i++)
        {
            gpio_drving_init_io(i,(IO_DRV_LEVEL) NAND_DATA_DRIVING);
        }
    #elif NAND_DATA5_0_POS==NAND_DATA5_0_AS_IOA13_8  /* force mode to modify NF_DATA0~7 to IOA8~15*/
        for (i=IO_A8 ; i<=IO_A13; i++)
        {
            gpio_drving_init_io(i,(IO_DRV_LEVEL) NAND_DATA_DRIVING);
        }
    #endif
    
    #if NAND_DATA7_6_POS==NAND_DATA7_6_AS_IOB15_14
        for (i=IO_B14 ; i<=IO_B15; i++)
        {
            gpio_drving_init_io(i,(IO_DRV_LEVEL) NAND_DATA_DRIVING);
        }
    #elif NAND_DATA7_6_POS==NAND_DATA7_6_AS_IOD7_6
        for (i=IO_D6 ; i<=IO_D7; i++)
        {
            gpio_drving_init_io(i,(IO_DRV_LEVEL) NAND_DATA_DRIVING);
        }
    #elif NAND_DATA7_6_POS==NAND_DATA7_6_AS_IOE7_6
        for (i=IO_E6 ; i<=IO_E7; i++)
        {
            gpio_drving_init_io(i,(IO_DRV_LEVEL) NAND_DATA_DRIVING);
        }
    #elif NAND_DATA7_6_POS==NAND_DATA7_6_AS_IOC5_4
        for (i=IO_C4 ; i<=IO_C5; i++)
        {
            gpio_drving_init_io(i,(IO_DRV_LEVEL) NAND_DATA_DRIVING);
        }
    #elif NAND_DATA7_6_POS==NAND_DATA7_6_AS_IOA15_14  /* force mode to modify NF_DATA0~7 to IOA8~15*/
        for (i=IO_A14 ; i<=IO_A15; i++)
        {
            gpio_drving_init_io(i,(IO_DRV_LEVEL) NAND_DATA_DRIVING);
        }
    #endif
#endif //NAND_SHARE_MODE == NF_NON_SHARE

#if NAND_CTRL_POS==NF_ALE_IOC12__CLE_IOC13__REB_IOC14__WEB_IOC15
        for (i=IO_C12 ; i<=IO_C15; i++)
        {
            gpio_drving_init_io(i,(IO_DRV_LEVEL) NAND_CTRL_DRIVING);
        }
#elif NAND_CTRL_POS==NF_ALE_IOC6__CLE_IOC7__REB_IOC8__WEB_IOC9 
        for (i=IO_C6 ; i<=IO_C9; i++)
        {
            //gpio_drving_init_io(i,(IO_DRV_LEVEL) NAND_CTRL_DRIVING);
        }
#elif NAND_CTRL_POS==NF_ALE_IOG5__CLE_IOG6__REB_IOG10__WEB_IOG11  
        gpio_drving_init_io(IO_G5,(IO_DRV_LEVEL) NAND_CTRL_DRIVING);
        gpio_drving_init_io(IO_G6,(IO_DRV_LEVEL) NAND_CTRL_DRIVING);
        gpio_drving_init_io(IO_G10,(IO_DRV_LEVEL) NAND_CTRL_DRIVING);
        gpio_drving_init_io(IO_G11,(IO_DRV_LEVEL) NAND_CTRL_DRIVING);
#endif

#endif
/* Nand Pad Dispacher END */  
}




/* This interface is for the application layer to initail the GPIO direction*/
/* init_io will not only modyfy the direction but also control the attribute value */
BOOLEAN gpio_init_io(INT32U port, BOOLEAN direction)
{
    INT16U gpio_set;
    INT16U gpio_set_num;
    //INT32U trace;
    OS_CPU_SR cpu_sr;
    
    gpio_set = port / EACH_REGISTER_GPIO_NUMS;
    gpio_set_num = port % EACH_REGISTER_GPIO_NUMS;
    direction &= LOWEST_BIT_MASK;
    if (direction == GPIO_OUTPUT) {
		OS_ENTER_CRITICAL();
        DRV_Reg32(IOA_ATTRIB_ADDR+(EACH_DIR_REG_OFFSET*gpio_set)) |= (1 << gpio_set_num);  /*Set attribute to 0 for input */
        DRV_Reg32((IOA_DIR_ADDR+EACH_DIR_REG_OFFSET*gpio_set)) |= (1 << gpio_set_num);
        OS_EXIT_CRITICAL();
    }
    else if (direction == GPIO_INPUT) {
		OS_ENTER_CRITICAL();
        DRV_Reg32(IOA_ATTRIB_ADDR+(EACH_DIR_REG_OFFSET*gpio_set)) &= ~(1 << gpio_set_num);  /*Set attribute to 1 for output */
        DRV_Reg32((IOA_DIR_ADDR+EACH_DIR_REG_OFFSET*gpio_set)) &= ~(1 << gpio_set_num);
        OS_EXIT_CRITICAL();
    }
    else { return GPIO_FAIL; }
    
    return GPIO_OK;
}


BOOLEAN gpio_read_io(INT32U port)
{
#if 0
    if (DRV_Reg32(gpio_data_in_addr[gpio_set_array[port]])&(bit_mask_array[gpio_setbit_array[port]]))
    {return 1;}
    else 
    {return 0;}
#else
    INT16U gpio_set; 
    INT16U gpio_set_num;
    /*debug k*/
    //INT32U k;
    
    gpio_set = port / EACH_REGISTER_GPIO_NUMS;
    gpio_set_num = port % EACH_REGISTER_GPIO_NUMS;
    //k = DRV_Reg32(IOA_DATA_ADDR+EACH_GPIO_DATA_REG_OFFSET*gpio_set) ;
    return ((DRV_Reg32(IOA_DATA_ADDR+EACH_GPIO_DATA_REG_OFFSET*gpio_set) >> gpio_set_num) & LOWEST_BIT_MASK);  
#endif

}

BOOLEAN gpio_write_io(INT32U port, BOOLEAN data)
{
	OS_CPU_SR cpu_sr;

#if 0
    if ((data&LOWEST_BIT_MASK))
    {
        DRV_Reg32(gpio_data_out_addr[gpio_set_array[port]]) |= bit_mask_array[gpio_setbit_array[port]];
    }
    else
    {
        DRV_Reg32(gpio_data_out_addr[gpio_set_array[port]]) &= bit_mask_inv_array[gpio_setbit_array[port]];
    }
    return GPIO_OK;
#else   

    INT16U gpio_set; 
    INT16U gpio_set_num;
    //INT32U trace;
    gpio_set = port / EACH_REGISTER_GPIO_NUMS;  // gpio_set_array
    gpio_set_num = port % EACH_REGISTER_GPIO_NUMS; // gpio_setbit_array
    
    data &= LOWEST_BIT_MASK;
    if (data == DATA_HIGH){    
		OS_ENTER_CRITICAL();
        DRV_Reg32((IOA_BUFFER_ADDR+EACH_GPIO_DATA_REG_OFFSET*gpio_set)) |= (1 << gpio_set_num);
        OS_EXIT_CRITICAL();
    }
    else if (data == DATA_LOW){
		OS_ENTER_CRITICAL();
        DRV_Reg32((IOA_BUFFER_ADDR+EACH_GPIO_DATA_REG_OFFSET*gpio_set)) &= ~(1 << gpio_set_num);
        OS_EXIT_CRITICAL();
    }
    else  {return GPIO_FAIL; }

    return GPIO_OK;

#endif
    return GPIO_OK;
}



BOOLEAN gpio_drving_init_io(GPIO_ENUM port, IO_DRV_LEVEL gpio_driving_level)
{

    INT16U gpio_set;
    INT16U gpio_set_num;
    INT32U drv_level;
	OS_CPU_SR cpu_sr;
    //INT32U trace;
    
    gpio_set = port / EACH_REGISTER_GPIO_NUMS;
    gpio_set_num = port % EACH_REGISTER_GPIO_NUMS;
    drv_level = (INT32U) gpio_driving_level;


    if (port < IO_E8)  // IOA/B/C/D and E(8-bit) 
    {
        if (drv_level == 0)
        {
			OS_ENTER_CRITICAL();
            DRV_Reg32(IOA_DRV+(EACH_DIR_REG_OFFSET*gpio_set)) &= ~(1 << gpio_set_num);  
            OS_EXIT_CRITICAL();
        }
        else //if (drv_level == 1)
        {
			OS_ENTER_CRITICAL();
            DRV_Reg32(IOA_DRV+(EACH_DIR_REG_OFFSET*gpio_set)) |= (1 << gpio_set_num);  
            OS_EXIT_CRITICAL();
        }
    }
    else 
    {
          return GPIO_FAIL;
    }

	
    return GPIO_OK;
}

void gpio_drving_uninit(void)
{
    R_IOA_DRV = 0;
    R_IOB_DRV = 0;
    R_IOC_DRV = 0;
    R_IOD_DRV = 0;
    R_IOE_DRV = 0;
}



BOOLEAN gpio_set_port_attribute(INT32U port, BOOLEAN attribute)
{
    INT16U gpio_set;
    INT16U gpio_set_num;
    OS_CPU_SR cpu_sr;
    
    gpio_set = port / EACH_REGISTER_GPIO_NUMS;
    gpio_set_num = port % EACH_REGISTER_GPIO_NUMS;
    attribute &= LOWEST_BIT_MASK;
    if (attribute == ATTRIBUTE_HIGH) {
		OS_ENTER_CRITICAL();
        DRV_Reg32((IOA_ATTRIB_ADDR+EACH_ATTRIB_REG_OFFSET*gpio_set)) |= (1 << gpio_set_num);
        OS_EXIT_CRITICAL();
    }
    else if (attribute == ATTRIBUTE_LOW) {
		OS_ENTER_CRITICAL();
        DRV_Reg32((IOA_ATTRIB_ADDR+EACH_ATTRIB_REG_OFFSET*gpio_set)) &= ~(1 << gpio_set_num);
        OS_EXIT_CRITICAL();
    }
    else { return GPIO_FAIL; }

    return GPIO_OK; 
}

BOOLEAN gpio_get_dir(INT32U port)
{
    INT16U gpio_set; 
    INT16U gpio_set_num;
    
    gpio_set = port / EACH_REGISTER_GPIO_NUMS;
    gpio_set_num = port % EACH_REGISTER_GPIO_NUMS;
    
    return ((DRV_Reg32(IOA_DIR_ADDR+EACH_DIR_REG_OFFSET*gpio_set) >> gpio_set_num) & LOWEST_BIT_MASK);  
}

void gpio_set_ice_en(BOOLEAN status)
{
	if (status == TRUE) {
	  R_GPIOCTRL |= 1; /* enable */
	}
	else  {
	  R_GPIOCTRL &= ~1; /* disable */
	}	
}


#if 0
static BOOLEAN gpio_get_attrib(INT32U port)
{
    INT16U gpio_set; 
    INT16U gpio_set_num;
    
    gpio_set = port / EACH_REGISTER_GPIO_NUMS;
    gpio_set_num = port % EACH_REGISTER_GPIO_NUMS;
    
    return ((DRV_Reg32(IOA_ATTRIB_ADDR+EACH_ATTRIB_REG_OFFSET*gpio_set) >> gpio_set_num) & LOWEST_BIT_MASK);  
}
#endif



//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#endif //(defined _DRV_L1_GPIO) && (_DRV_L1_GPIO == 1)            //
//================================================================//
