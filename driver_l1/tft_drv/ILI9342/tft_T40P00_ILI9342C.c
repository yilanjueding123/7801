#include "drv_l1_tft.h"
#include "application.h"

#if USE_PANEL_NAME == PANEL_T40P00_ILI9342C

static INT32U  CSN_n;
static INT32U  SCL_n;
static INT32U  SDA_n;
//static INT32U  SDI_n;

static void cmd_delay(INT32U i)
{
	unsigned int j, cnt;
	cnt = i * 2;
	for (j = 0; j < cnt; j++);
}

static void cmd_longdelay(INT32U i)
{
	INT32U j, cnt;

	cnt = i * 100;

	for (j = 0; j < cnt; j++);
}

static void delayms(INT32U i)
{
	cmd_longdelay(i * 5);
}

static void serial_cmd_231(INT32U value)
{
	INT32S i;

	gpio_write_io(CSN_n, 1); //CS=1
	gpio_write_io(SCL_n, 1); //SCL=1
	gpio_write_io(SDA_n, 1); //SDA=1

	cmd_delay(5);
	// set csn low
	gpio_write_io(CSN_n, 0); //CS=0
	cmd_delay(1);
	for (i = 0; i < 9; i++) {
		cmd_delay(5);
		gpio_write_io(SCL_n, 0); //SCL=0
		if (value & 0x100) {
			gpio_write_io(SDA_n, 1);    /* SDA = 1 */
		} else {
			gpio_write_io(SDA_n, 0);    /* SDA = 0 */
		}
		value <<= 1;
		cmd_delay(5);
		gpio_write_io(SCL_n, 1); //SCL=1
	}
	cmd_delay(5);
	cmd_delay(5);
	gpio_write_io(SDA_n, 1);  /* SDA = 1 */
	gpio_write_io(SCL_n, 1);  //SCL=1
}

static void Write_COMMAND16i(INT32U data)
{
	serial_cmd_231(data);
}

static void Write_DATA16i(INT32U data)
{
	serial_cmd_231(0x100 | data);
}


void tft_T40P00_ILI9342C_init(void)
{
	/* 320*240 */

	CSN_n = IO_A11;
	SDA_n = IO_A10;
	SCL_n = IO_A9;


#if 1
	gpio_init_io(CSN_n, GPIO_OUTPUT);
	gpio_init_io(SCL_n, GPIO_OUTPUT);
	gpio_init_io(SDA_n, GPIO_OUTPUT);

	gpio_set_port_attribute(CSN_n, 1);
	gpio_set_port_attribute(SCL_n, 1);
	gpio_set_port_attribute(SDA_n, 1);

	Write_COMMAND16i(0x00);
	delayms(50);

	Write_COMMAND16i(0x00);
	delayms(100);

	Write_COMMAND16i(0xC8);       //Set EXTC
	Write_DATA16i(0xFF);
	Write_DATA16i(0x93);
	Write_DATA16i(0x42);

	Write_COMMAND16i(0x36);       //Memory Access Control
	Write_DATA16i(0xC8);       //MY,MX,MV,ML,BGR,MH

	Write_COMMAND16i(0x3A);       //Pixel Format Set
	Write_DATA16i(0x66);       //DPI [2:0],DBI [2:0]

	Write_COMMAND16i(0xC0);       //Power Control 1
	Write_DATA16i(0x14);       //VRH[5:0]
	Write_DATA16i(0x0E);       //VC[3:0]
	delayms(100);

	Write_COMMAND16i(0xC1);       //Power Control 2
	Write_DATA16i(0x01);       //SAP[2:0],BT[3:0]

	Write_COMMAND16i(0xC5);       //VCOM
	Write_DATA16i(0xF6);

	Write_COMMAND16i(0xB1);
	Write_DATA16i(0x00);
	Write_DATA16i(0x1B);
	Write_COMMAND16i(0xB4);
	Write_DATA16i(0x02);

	Write_COMMAND16i(0xE0);
	Write_DATA16i(0x00);//P01-VP63
	Write_DATA16i(0x0E);//P02-VP62
	Write_DATA16i(0x18);//P03-VP61
	Write_DATA16i(0x05);//P04-VP59
	Write_DATA16i(0x13);//P05-VP57
	Write_DATA16i(0x09);//P06-VP50
	Write_DATA16i(0x3D);//P07-VP43
	Write_DATA16i(0x89);//P08-VP27,36
	Write_DATA16i(0x4A);//P09-VP20
	Write_DATA16i(0x08);//P10-VP13
	Write_DATA16i(0x0E);//P11-VP6
	Write_DATA16i(0x0A);//P12-VP4
	Write_DATA16i(0x1B);//P13-VP2
	Write_DATA16i(0x1B);//P14-VP1
	Write_DATA16i(0x0F);//P15-VP0

	Write_COMMAND16i(0xE1);
	Write_DATA16i(0x00);//P01
	Write_DATA16i(0x21);//P02
	Write_DATA16i(0x24);//P03
	Write_DATA16i(0x03);//P04
	Write_DATA16i(0x0F);//P05
	Write_DATA16i(0x06);//P06
	Write_DATA16i(0x37);//P07
	Write_DATA16i(0x36);//P08
	Write_DATA16i(0x47);//P09
	Write_DATA16i(0x02);//P10
	Write_DATA16i(0x09);//P11
	Write_DATA16i(0x07);//P12
	Write_DATA16i(0x2E);//P13
	Write_DATA16i(0x33);//P14
	Write_DATA16i(0x0F);//P15

	Write_COMMAND16i(0xB0);
	Write_DATA16i(0xe0);//e0

	Write_COMMAND16i(0xf6);
	Write_DATA16i(0x01);
	Write_DATA16i(0x00);
	Write_DATA16i(0x03);//03

	Write_COMMAND16i(0x11);//Exit Sleep
	delayms(200);

	Write_COMMAND16i(0x29);//Display On

	delayms(180);

	R_TFT_HS_WIDTH			= 0;				//	1		=HPW
	R_TFT_H_START			= 30;			//	240		=HPW+HBP
	R_TFT_H_END				= 30 + 960;	//	1520	=HPW+HBP+HDE
	R_TFT_H_PERIOD			= 30 + 960 + 10;	//	1560	=HPW+HBP+HDE+HFP
	R_TFT_VS_WIDTH			= 0;				//	1		=VPW				(DCLK)
	R_TFT_V_START			= 4;			//	21		=VPW+VBP			(LINE)
	R_TFT_V_END				= 4 + 240;		//	261		=VPW+VBP+VDE		(LINE)
	R_TFT_V_PERIOD			= 4 + 240 + 4;		//	262		=VPW+VBP+VDE+VFP	(LINE)
	R_TFT_LINE_RGB_ORDER    = 0x03 | (0x3 << 4) ;


	R_TFT_CTRL           	= 0x8319  | (0x1 << 10) ;
	R_TFT_TS_MISC		   &= 0xFFF3FF;
	tft_mode_set(TFT_MODE_UPS051);
	
 #else
 	gpio_init_io(CSN_n, GPIO_OUTPUT);
	gpio_init_io(SCL_n, GPIO_OUTPUT);
	gpio_init_io(SDA_n, GPIO_OUTPUT);
	gpio_init_io (IO_A15, GPIO_OUTPUT);

	gpio_set_port_attribute(CSN_n, 1);
	gpio_set_port_attribute(SCL_n, 1);
	gpio_set_port_attribute(SDA_n, 1);
	gpio_set_port_attribute(IO_A15, 1);

	gpio_write_io(CSN_n,1);
	gpio_write_io(SCL_n,0);
	gpio_write_io(SDA_n,0);

	gpio_write_io(IO_A15,1);
	drv_msec_wait(5);
	gpio_write_io(IO_A15,0);
	drv_msec_wait(10);
	gpio_write_io(IO_A15,1);
	drv_msec_wait(5);

   	
   	Write_COMMAND16i(0x00);
	delayms(50);

	Write_COMMAND16i(0x00);
	delayms(100);

   	Write_COMMAND16i(0xC8);
	Write_DATA16i(0xFF);
	Write_DATA16i(0x93);
	Write_DATA16i(0x42);

   	Write_COMMAND16i(0xC0);
   	Write_DATA16i(0x05); /*0x0a 0x0f µ÷Ð¡±äÁÁ*/
   	Write_DATA16i(0x05); /*0x0a 0x0f*/

   	Write_COMMAND16i(0xC1);
   	Write_DATA16i(0x05);/*0x01*/

   	Write_COMMAND16i(0xC5);
   	Write_DATA16i(0xDB);

   	Write_COMMAND16i(0x36);
 /*#ifdef _MIRROR_VFLIP_
   	Write_DATA16i(0x08);
 //#else
 //Write_DATA16i(0xC8);
 //#endif*/
  	Write_DATA16i(0xC8);
   	
  	Write_COMMAND16i(0x3A);
   	Write_DATA16i(0x66);     //18BIT PIX FOMART

   	Write_COMMAND16i(0xB0);
   	Write_DATA16i(0xE0);

   	Write_COMMAND16i(0xB4);
   	Write_DATA16i(0x02);
	
   	Write_COMMAND16i(0xB7);
   	Write_DATA16i(0x07);

   	Write_COMMAND16i(0xF6);
   	Write_DATA16i(0x01);
   	Write_DATA16i(0x00);
   	Write_DATA16i(0x07);

   	Write_COMMAND16i(0xE0); //Set Gamma
   	Write_DATA16i(0x00);
   	Write_DATA16i(0x05);
   	Write_DATA16i(0x08);
   	Write_DATA16i(0x02);
   	Write_DATA16i(0x1A);
   	Write_DATA16i(0x0C);
   	Write_DATA16i(0x42);
   	Write_DATA16i(0x7A);
   	Write_DATA16i(0x54);
   	Write_DATA16i(0x08);
   	Write_DATA16i(0x0D);
   	Write_DATA16i(0x0C);
   	Write_DATA16i(0x23);
   	Write_DATA16i(0x25);
   	Write_DATA16i(0x0F);

   	Write_COMMAND16i(0xE1); //Set Gamma
   	Write_DATA16i(0x00);
   	Write_DATA16i(0x29);
   	Write_DATA16i(0x2F);
   	Write_DATA16i(0x03);
   	Write_DATA16i(0x0F);
   	Write_DATA16i(0x05);
   	Write_DATA16i(0x42);
   	Write_DATA16i(0x55);
   	Write_DATA16i(0x53);
   	Write_DATA16i(0x06);
   	Write_DATA16i(0x0F);
   	Write_DATA16i(0x0C);
   	Write_DATA16i(0x38);
   	Write_DATA16i(0x3A);
   	Write_DATA16i(0x0F);

   	Write_COMMAND16i(0x20);

   	Write_COMMAND16i(0x11); //Exit Sleep
   	delayms(200);
   	Write_COMMAND16i(0x29); //Display on

   	//Write_COMMAND16i(0x2C);
	delayms(180);

	R_TFT_HS_WIDTH			= 0;				//	1		=HPW
	R_TFT_H_START			= 24;			//	240		=HPW+HBP
	R_TFT_H_END				= 24+960;		//	1520	=HPW+HBP+HDE
	R_TFT_H_PERIOD			= 24+960+10;	//	1560	=HPW+HBP+HDE+HFP
	R_TFT_VS_WIDTH			= 0;				//	1		=VPW				(DCLK)
	R_TFT_V_START			= 3;			//	21		=VPW+VBP			(LINE)
	R_TFT_V_END				= 3+240;		//	261		=VPW+VBP+VDE		(LINE)
	R_TFT_V_PERIOD			= 3+240+4;		//	262		=VPW+VBP+VDE+VFP	(LINE)
	R_TFT_LINE_RGB_ORDER     = 0x03 | (0x3 << 4) ;

	R_TFT_CTRL           		= 0x8319  | (0x1 << 10) ;
	R_TFT_TS_MISC		   	&= 0xFFF3FF;
	tft_mode_set(TFT_MODE_UPS051);

 #endif
 
 AP_TFT_ClK_144M_set();
}

#endif

