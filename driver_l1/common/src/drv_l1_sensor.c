#include	"drv_l1_sensor.h"

extern void print_string(CHAR *fmt, ...);

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#if (defined _DRV_L1_SENSOR) && (_DRV_L1_SENSOR == 1)             //
//================================================================//
void sccb_delay (INT16U i);
void sccb_start (void);
void sccb_stop (void);
void sccb_w_phase (INT16U value);
INT16U sccb_r_phase (void);
void sccb_init (INT32U nSCL, INT32U nSDA);
void sccb_write (INT16U id, INT16U addr, INT16U data);
INT16U sccb_read (INT16U id, INT16U addr);

void CSI_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag, INT32U uFrmBuf0, INT32U uFrmBuf1);
#ifdef	__OV6680_DRV_C__
void OV6680_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag);
#endif
#ifdef	__OV7680_DRV_C__
extern void OV7680_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag);
#endif
#ifdef	__OV7670_DRV_C__
extern void OV7670_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag);
#endif
#ifdef	__OV7725_DRV_C__
extern void OV7725_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag);
#endif
#ifdef	__OV9655_DRV_C__
extern void OV9655_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag);
#endif
#ifdef	__OV2655_DRV_C__
extern void OV2655_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag);
#endif
#ifdef	__OV3640_DRV_C__
extern void OV3640_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag);
#endif
#ifdef	__OV5642_DRV_C__
extern void OV5642_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag);
#endif
#ifdef	__OV7675_DRV_C__
extern void OV7675_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag);
#endif
#ifdef	__GC0308_DRV_C__
extern void GC0308_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag);
#endif
#ifdef	__SP0838_DRV_C__
extern void SP0838_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag);
#endif
#ifdef	__OV2643_DRV_C__
extern void OV2643_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag);
#endif

void Sensor_Bluescreen_Enable(void);
void Sensor_Cut_Enable(void);


extern INT8U frame_mode_en;

#if VIDEO_ENCODE_USE_MODE ==SENSOR_BUF_FIFO_MODE
extern INT8U csi_fifo_flag;
#endif

//====================================================================================================
// SCCB Control C Code
// using SDA and SDL to control sensor interface
//====================================================================================================

//====================================================================================================
//	Description:	Delay function
//	Syntax:			sccb_delay (
//						INT16U i
//					);
//	Return: 		None
//====================================================================================================
void sccb_delay (
	INT16U i
) {
	INT16U j;
	INT32U cnt;

	cnt = (i<<4)+(i<<3);
	for (j=0; j<cnt; j++) {
		i=i;
	}
}

//===================================================================
//	Description:	Start of data transmission
//	Function:		sccb_start
//	Syntax:			void sccb_start (void)
//	Input Paramter:
//					none
//	Return: 		none
//	Note:			Please refer to SCCB spec
//===================================================================
void sccb_start (void)
{
	gpio_write_io(SCCB_SCL, DATA_HIGH);					//SCL1
	sccb_delay (1);
	gpio_write_io(SCCB_SDA, DATA_HIGH);					//SDA1
	sccb_delay (1);
	gpio_write_io(SCCB_SDA, DATA_LOW);					//SDA0
	sccb_delay (1);
}

//===================================================================
//	Description:	Stop of data transmission
//	Function:		sccb_stop
//	Syntax:			void sccb_stop (void)
//	Input Paramter:
//					none
//	Return: 		none
//	Note:			Please refer to SCCB spec
//===================================================================
void sccb_stop (void)
{
	sccb_delay (1);
	gpio_write_io(SCCB_SDA, DATA_LOW);					//SDA0
	sccb_delay (1);
	gpio_write_io(SCCB_SCL, DATA_HIGH);					//SCL1
	sccb_delay (1);
	gpio_write_io(SCCB_SDA, DATA_HIGH);					//SDA1
	sccb_delay (1);
}

//===================================================================
//	Description:	Phase write process
//	Function:		sccb_w_phase
//	Syntax:			void sccb_w_phase (INT16U value)
//	Input Paramter:
//					INT16U value:	output data
//	Return: 		none
//	Note:			Please refer to SCCB spec
//===================================================================
void sccb_w_phase (INT16U value)
{
	INT16U i;

	for (i=0;i<8;i++)
	{
		gpio_write_io(SCCB_SCL, DATA_LOW);					//SCL0
//		sccb_delay (1);
		if (value & 0x80)
			gpio_write_io(SCCB_SDA, DATA_HIGH);				//SDA1
		else
			gpio_write_io(SCCB_SDA, DATA_LOW);				//SDA0
		sccb_delay (1);
		gpio_write_io(SCCB_SCL, DATA_HIGH);					//SCL1
		sccb_delay(1);
		value <<= 1;
	}
	// The 9th bit transmission
	gpio_write_io(SCCB_SCL, DATA_LOW);						//SCL0
	gpio_init_io(SCCB_SDA, GPIO_INPUT);						//SDA is Hi-Z mode
	sccb_delay(1);
	gpio_write_io(SCCB_SCL, DATA_HIGH);						//SCL1
	sccb_delay(1);
	gpio_write_io(SCCB_SCL, DATA_LOW);						//SCL0
	gpio_init_io(SCCB_SDA, GPIO_OUTPUT);						//SDA is Hi-Z mode
}

//===================================================================
//	Description:	Phase read process
//	Function:		sccb_r_phase
//	Syntax:			INT16U sccb_r_phase (void)
//	Input Paramter: none
//	Return: 		INT16U			: input data
//	Note:			Please refer to SCCB spec
//===================================================================
INT16U sccb_r_phase (void)
{
	INT16U i;
	INT16U data;

	gpio_init_io(SCCB_SDA, GPIO_INPUT);						//SDA is Hi-Z mode
	data = 0x00;
	for (i=0;i<8;i++)
	{
		gpio_write_io(SCCB_SCL, DATA_LOW);					//SCL0
		sccb_delay(1);
		gpio_write_io(SCCB_SCL, DATA_HIGH);					//SCL1
		data <<= 1;
		data |=( gpio_read_io(SCCB_SDA));
		sccb_delay(1);
	}
	// The 9th bit transmission
	gpio_write_io(SCCB_SCL, DATA_LOW);						//SCL0
	gpio_init_io(SCCB_SDA, GPIO_OUTPUT);					//SDA is output mode
	gpio_write_io(SCCB_SDA, DATA_HIGH);						//SDA0, the nighth bit is NA must be 1
	sccb_delay(1);
	gpio_write_io(SCCB_SCL, DATA_HIGH);						//SCL1
	sccb_delay(1);
	gpio_write_io(SCCB_SCL, DATA_LOW);						//SCL0
	return data;
}

//====================================================================================================
//	Description:	SCCB Initialization
//	Syntax:			void sccb_init (
//						INT16S nSCL,				// Clock Port No,
//						INT16S nSDA				// Data Port No
//					);
//	Return: 		None
//====================================================================================================
void sccb_init (
	INT32U nSCL,			// Clock Port No
	INT32U nSDA				// Data Port No
){
	//init IO
	gpio_set_port_attribute(nSCL, ATTRIBUTE_HIGH);
	gpio_set_port_attribute(nSDA, ATTRIBUTE_HIGH);
	gpio_init_io(nSCL, GPIO_OUTPUT);				//set dir
	gpio_init_io(nSDA, GPIO_OUTPUT);				//set dir
	gpio_write_io(nSCL, DATA_HIGH);					//SCL1
	gpio_write_io(nSDA, DATA_HIGH);					//SDA0
}

//====================================================================================================
//	Description:	SCCB write register process
//	Syntax:			void sccb_write (
//						INT16U id,					// Slave ID
//						INT16U addr,				// Register Address
//						INT16U data				// Register Data
//					);
//	Return: 		None
//	Note:			Please refer to SCCB spec
//====================================================================================================
void sccb_write (
	INT16U id,					// Slave ID
	INT16U addr,				// Register Address
	INT16U data				  	// Register Data
) {
	//INT16U read_data;

	// Data re-verification
	id &= 0xFF;
#if	(defined __OV2655_DRV_C__)||(defined __OV3640_DRV_C__)||(defined __OV5642_DRV_C__)
	addr &= 0xFFFF;
#else
	addr &= 0xFF;
#endif
	data &= 0xFF;

	// Serial bus output mode initialization
	gpio_set_port_attribute(SCCB_SCL, ATTRIBUTE_HIGH);
	gpio_set_port_attribute(SCCB_SDA, ATTRIBUTE_HIGH);
	gpio_init_io(SCCB_SCL, GPIO_OUTPUT);				//set dir
	gpio_init_io(SCCB_SDA, GPIO_OUTPUT);				//set dir

	// 3-Phase write transmission cycle is starting now ...
	gpio_write_io(SCCB_SCL, DATA_HIGH);				//SCL1
	gpio_write_io(SCCB_SDA, DATA_LOW);				//SDA0
	sccb_start ();									// Transmission start
	sccb_w_phase (id);								// Phase 1
#if	(defined __OV2655_DRV_C__)||(defined __OV3640_DRV_C__)||(defined __OV5642_DRV_C__)
	sccb_w_phase (addr>>8);							// Phase 2
#endif
	sccb_w_phase (addr);
	sccb_w_phase (data);							// Phase 3
	sccb_stop ();									// Transmission stop
	
	
//----test----
	/*
	read_data = sccb_read(id,addr);
	if(read_data == data)
	{
		DBG_PRINT("addr = 0x%x,write_data 0x%x,read_data = 0x%x \r\n", addr,data,read_data);
	}
	else
	{
		DBG_PRINT("--------------addr = 0x%x,write_data 0x%x,read_data = 0x%x----------------\r\n", addr,data,read_data);
	} */
//----test end

}

//====================================================================================================
//	Description:	SCCB read register process
//	Syntax:			INT16U sccb_read (
//						INT16U id,					// Slave ID
//						INT16U addr				// Register Address
//					);
//	Input Paramter:
//					INT16U id		: slave device id
//					INT16U addr	: register address
//	Return:
//					INT16U 		: register data
//	Note:			Please refer to SCCB spec
//====================================================================================================
INT16U sccb_read (
		INT16U id,					// Slave ID
		INT16U addr					// Register Address
) {
	INT16U data;

	// Data re-verification
	id &= 0xFF;
	addr &= 0xFF;

	// Serial bus output mode initialization
	gpio_set_port_attribute(SCCB_SCL, ATTRIBUTE_HIGH);
	gpio_set_port_attribute(SCCB_SDA, ATTRIBUTE_HIGH);
	gpio_init_io(SCCB_SCL, GPIO_OUTPUT);				//set dir
	gpio_init_io(SCCB_SDA, GPIO_OUTPUT);				//set dir

	// 2-Phase write transmission cycle is starting now ...
	gpio_write_io(SCCB_SCL, DATA_HIGH);				//SCL1
	gpio_write_io(SCCB_SDA, DATA_LOW);				//SDA0
	sccb_start ();									// Transmission start
	sccb_w_phase (id);								// Phase 1
	sccb_w_phase (addr);							// Phase 2
	sccb_stop ();									// Transmission stop

	// 2-Phase read transmission cycle is starting now ...
	sccb_start ();									// Transmission start
	sccb_w_phase (id | 0x01);						// Phase 1 (read)
	data = sccb_r_phase();							// Phase 2
	sccb_stop ();									// Transmission stop
	return data;
}

#ifdef	__OV6680_DRV_C__
//====================================================================================================
//	Description:	OV6680 Initialization
//	Syntax:			void OV6680_Init (
//						INT16S nWidthH,			// Active H Width
//						INT16S nWidthV,			// Active V Width
//						INT16U uFlag				// Flag Type
//					);
//	Return:			None
//====================================================================================================
void OV6680_Init (
	INT16S nWidthH,			// Active H Width
	INT16S nWidthV,			// Active V Width
	INT16U uFlag				// Flag Type
) {
}
#endif

#ifdef	__OV9655_DRV_C__
//====================================================================================================
//	Description:	OV9655 Initialization
//	Syntax:			void OV9655_Init (
//						INT16S nWidthH,			// Active H Width
//						INT16S nWidthV,			// Active V Width
//						INT16U uFlag				// Flag Type
//					);
//	Return:			None
//====================================================================================================
void OV9655_Init (
	INT16S nWidthH,			// Active H Width
	INT16S nWidthV,			// Active V Width
	INT16U uFlag				// Flag Type
) {
	INT16U uCtrlReg1, uCtrlReg2;

	// Enable CSI clock to let sensor initialize at first
#if CSI_CLOCK == CSI_CLOCK_27MHZ
	uCtrlReg2 = CLKOEN | CSI_RGB565 |CLK_SEL27M | CSI_HIGHPRI | CSI_NOSTOP;
#else
	uCtrlReg2 = CLKOEN | CSI_RGB565 |CLK_SEL48M | CSI_HIGHPRI | CSI_NOSTOP;
#endif

	uCtrlReg1 = CSIEN | YUV_YUYV | CAP;									// Default CSI Control Register 1
	if (uFlag & FT_CSI_RGB1555)											// RGB1555
	{
		uCtrlReg2 |= CSI_RGB1555;
	}
	if (uFlag & FT_CSI_CCIR656)										// CCIR656?
	{
		uCtrlReg1 |= CCIR656 | VADD_FALL | VRST_FALL | HRST_FALL;	// CCIR656
		uCtrlReg2 |= D_TYPE1;										// CCIR656
	}
	else
	{
		uCtrlReg1 |= VADD_RISE | VRST_FALL | HRST_RISE | HREF;		// NOT CCIR656
		uCtrlReg2 |= D_TYPE0;										// NOT CCIR656
	}
	if (uFlag & FT_CSI_YUVIN)										// YUVIN?
	{
		uCtrlReg1 |= YUVIN;
	}
	if (uFlag & FT_CSI_YUVOUT)										// YUVOUT?
	{
		uCtrlReg1 |= YUVOUT;
	}

	R_CSI_TG_VL0START = 0x0000;						// Sensor field 0 vertical latch start register.
	R_CSI_TG_VL1START = 0x0000;						//*P_Sensor_TG_V_L1Start = 0x0000;
	R_CSI_TG_HSTART = 0x0000;						// Sensor horizontal start register.

	R_CSI_TG_CTRL0 = 0;								//reset control0
	R_CSI_TG_CTRL1 = CSI_NOSTOP|CLKOEN;				//enable CSI CLKO
	drv_msec_wait(100); 							//wait 100ms for CLKO stable

	// CMOS Sensor Initialization Start...
	sccb_init (SCCB_SCL, SCCB_SDA);
	sccb_delay (200);
	sccb_write (SLAVE_ID, 0x12, 0x80);
	sccb_delay (200);

	sccb_write(SLAVE_ID,0xb5,0x00);
	sccb_write(SLAVE_ID,0x35,0x00);
	sccb_write(SLAVE_ID,0xa8,0xc1);
	sccb_write(SLAVE_ID,0x3a,0x80);
	sccb_write(SLAVE_ID,0x3d,0x99);
	sccb_write(SLAVE_ID,0x77,0x02);
	sccb_write(SLAVE_ID,0x13,0xe7);
	sccb_write(SLAVE_ID,0x26,0x72);
	sccb_write(SLAVE_ID,0x27,0x08);
	sccb_write(SLAVE_ID,0x28,0x08);
	sccb_write(SLAVE_ID,0x29,0x15);
	sccb_write(SLAVE_ID,0x2c,0x08);
	sccb_write(SLAVE_ID,0xab,0x04);
	sccb_write(SLAVE_ID,0x6e,0x00);
	sccb_write(SLAVE_ID,0x6d,0x55);
	sccb_write(SLAVE_ID,0x00,0x32);
	sccb_write(SLAVE_ID,0x10,0x7b);
	sccb_write(SLAVE_ID,0xbb,0xae);


#if CSI_CLOCK == CSI_CLOCK_27MHZ
	sccb_write(SLAVE_ID,0x11,0x40);	// Use external Clock Directly
#else
	sccb_write(SLAVE_ID,0x11,0x00);
#endif

	if(((nWidthH == 320) &&(nWidthV == 240))||((nWidthH == 640) &&(nWidthV == 480)))
	{
		if ((nWidthH == 320) &&(nWidthV == 240))
		{
			sccb_write(SLAVE_ID,0x72,0x11);
			sccb_write(SLAVE_ID,0x3e,0x02);
			sccb_write(SLAVE_ID,0x74,0x10);
			sccb_write(SLAVE_ID,0x76,0x01);
			sccb_write(SLAVE_ID,0x75,0x10);
			sccb_write(SLAVE_ID,0x73,0x01);
			sccb_write(SLAVE_ID,0xc7,0x81);
		}
		else if ((nWidthH == 640) &&(nWidthV == 480))
		{
			sccb_write(SLAVE_ID,0x11,0x00);
			sccb_write(SLAVE_ID,0x72,0x00);
			sccb_write(SLAVE_ID,0x3e,0x0c);
			sccb_write(SLAVE_ID,0x74,0x3a);
			sccb_write(SLAVE_ID,0x76,0x01);
			sccb_write(SLAVE_ID,0x75,0x35);
			sccb_write(SLAVE_ID,0x73,0x00);
			sccb_write(SLAVE_ID,0xc7,0x80);
		}

		sccb_write(SLAVE_ID,0xc3,0x4e);
		sccb_write(SLAVE_ID,0x33,0x00);
		sccb_write(SLAVE_ID,0xa4,0x50);
		sccb_write(SLAVE_ID,0xaa,0x92);
		sccb_write(SLAVE_ID,0xc2,0x01);
		sccb_write(SLAVE_ID,0xc1,0xc8);
		//sccb_write(SLAVE_ID,0x1e,0x04);//del by george 08272007
		sccb_write(SLAVE_ID,0x1e,0x24);//enable mirror effect
		sccb_write(SLAVE_ID,0xa9,0xfa);
		sccb_write(SLAVE_ID,0x0e,0x61);
		sccb_write(SLAVE_ID,0x39,0x57);
		sccb_write(SLAVE_ID,0x0f,0x42);
		sccb_write(SLAVE_ID,0x24,0x3c);
		sccb_write(SLAVE_ID,0x25,0x36);

		if (uFlag & FT_CSI_YUVIN)
			sccb_write(SLAVE_ID,0x12,0x62);
		else
			sccb_write(SLAVE_ID,0x12,0x60);

		if ((nWidthH == 320) &&(nWidthV == 240))
		{
			sccb_write(SLAVE_ID,0x03,0x02);
			sccb_write(SLAVE_ID,0x32,0x12);
			sccb_write(SLAVE_ID,0x17,0x18);
			sccb_write(SLAVE_ID,0x18,0x04);
			sccb_write(SLAVE_ID,0x19,0x01);
			sccb_write(SLAVE_ID,0x1a,0x81);
			sccb_write(SLAVE_ID,0x36,0x3a);
			sccb_write(SLAVE_ID,0x69,0x0a);
			sccb_write(SLAVE_ID,0x8c,0x80);
		}
		else if ((nWidthH == 640) &&(nWidthV == 480))
		{
			sccb_write(SLAVE_ID,0x03,0x12);
			sccb_write(SLAVE_ID,0x32,0xff);
			sccb_write(SLAVE_ID,0x17,0x16);
			sccb_write(SLAVE_ID,0x18,0x02);
			sccb_write(SLAVE_ID,0x19,0x01);
			sccb_write(SLAVE_ID,0x1a,0x3d);
			sccb_write(SLAVE_ID,0x36,0xfa);
			sccb_write(SLAVE_ID,0x69,0x0a);
			sccb_write(SLAVE_ID,0x8c,0x8d);
		}

		sccb_write(SLAVE_ID,0xc0,0xaa);
		sccb_write(SLAVE_ID,0x40,0xc0);

		sccb_write(SLAVE_ID,0xc6,0x85);
		sccb_write(SLAVE_ID,0xcb,0xf0);
		sccb_write(SLAVE_ID,0xcc,0xd8);
		sccb_write(SLAVE_ID,0x71,0x78);
		sccb_write(SLAVE_ID,0xa5,0x68);
		sccb_write(SLAVE_ID,0x6f,0x9e);
		sccb_write(SLAVE_ID,0x42,0xc0);
		sccb_write(SLAVE_ID,0x3f,0x82);
		sccb_write(SLAVE_ID,0x8a,0x23);
		sccb_write(SLAVE_ID,0x14,0x3a);
		sccb_write(SLAVE_ID,0x3b,0xcc);
		sccb_write(SLAVE_ID,0x34,0x3d);

		if ((nWidthH == 320) &&(nWidthV == 240))
			sccb_write(SLAVE_ID,0x41,0x41);
		else if ((nWidthH == 640) &&(nWidthV == 480))
			sccb_write(SLAVE_ID,0x41,0x40);

		sccb_write(SLAVE_ID,0xc9,0xe0);
		sccb_write(SLAVE_ID,0xca,0xe8);
		sccb_write(SLAVE_ID,0xcd,0x93);
		sccb_write(SLAVE_ID,0x7a,0x20);
		sccb_write(SLAVE_ID,0x7b,0x1c);
		sccb_write(SLAVE_ID,0x7c,0x28);
		sccb_write(SLAVE_ID,0x7d,0x3c);
		sccb_write(SLAVE_ID,0x7e,0x5a);
		sccb_write(SLAVE_ID,0x7f,0x68);
		sccb_write(SLAVE_ID,0x80,0x76);
		sccb_write(SLAVE_ID,0x81,0x80);
		sccb_write(SLAVE_ID,0x82,0x88);
		sccb_write(SLAVE_ID,0x83,0x8f);
		sccb_write(SLAVE_ID,0x84,0x96);
		sccb_write(SLAVE_ID,0x85,0xa3);
		sccb_write(SLAVE_ID,0x86,0xaf);
		sccb_write(SLAVE_ID,0x87,0xc4);
		sccb_write(SLAVE_ID,0x88,0xd7);
		sccb_write(SLAVE_ID,0x89,0xe8);
		sccb_write(SLAVE_ID,0x4f,0x98);
		sccb_write(SLAVE_ID,0x50,0x98);
		sccb_write(SLAVE_ID,0x51,0x00);
		sccb_write(SLAVE_ID,0x52,0x28);
		sccb_write(SLAVE_ID,0x53,0x70);
		sccb_write(SLAVE_ID,0x54,0x98);
		sccb_write(SLAVE_ID,0x58,0x1a);

		//sccb_write(SLAVE_ID,0x6b,0x5a);
		sccb_write(SLAVE_ID,0x6b,0x0a);		// Bypass PLL

		sccb_write(SLAVE_ID,0x90,0x86);
		sccb_write(SLAVE_ID,0x91,0x84);
		sccb_write(SLAVE_ID,0x9f,0x75);
		sccb_write(SLAVE_ID,0xa0,0x73);
		sccb_write(SLAVE_ID,0x16,0x24);
		sccb_write(SLAVE_ID,0x2a,0x00);
		sccb_write(SLAVE_ID,0x2b,0x00);

		sccb_write(SLAVE_ID,0xac,0x80);
		sccb_write(SLAVE_ID,0xad,0x80);
		sccb_write(SLAVE_ID,0xae,0x80);
		sccb_write(SLAVE_ID,0xaf,0x80);
		sccb_write(SLAVE_ID,0xb2,0xf2);
		sccb_write(SLAVE_ID,0xb3,0x20);
		sccb_write(SLAVE_ID,0xb4,0x20);
		sccb_write(SLAVE_ID,0xb6,0xaf);
		sccb_write(SLAVE_ID,0xb6,0xaf);

		sccb_write(SLAVE_ID,0x05,0x2b);
		sccb_write(SLAVE_ID,0x06,0x35);
		sccb_write(SLAVE_ID,0x07,0x36);
		sccb_write(SLAVE_ID,0x08,0x3b);
		sccb_write(SLAVE_ID,0x2d,0xf4);
		sccb_write(SLAVE_ID,0x2e,0x01);
		sccb_write(SLAVE_ID,0x2f,0x35);
		sccb_write(SLAVE_ID,0x4a,0xea);
		sccb_write(SLAVE_ID,0x4b,0xe6);
		sccb_write(SLAVE_ID,0x4c,0xe6);
		sccb_write(SLAVE_ID,0x4d,0xe6);
		sccb_write(SLAVE_ID,0x4e,0xe6);
		sccb_write(SLAVE_ID,0x70,0x0b);
		sccb_write(SLAVE_ID,0xa6,0x40);
		sccb_write(SLAVE_ID,0xbc,0x04);
		sccb_write(SLAVE_ID,0xbd,0x01);
		sccb_write(SLAVE_ID,0xbe,0x03);
		sccb_write(SLAVE_ID,0xbf,0x01);
		sccb_write(SLAVE_ID,0xbf,0x01);

		sccb_write(SLAVE_ID,0x43,0x14);
		sccb_write(SLAVE_ID,0x44,0xf0);
		sccb_write(SLAVE_ID,0x45,0x46);
		sccb_write(SLAVE_ID,0x46,0x62);
		sccb_write(SLAVE_ID,0x47,0x2a);
		sccb_write(SLAVE_ID,0x48,0x3c);
		sccb_write(SLAVE_ID,0x59,0x85);
		sccb_write(SLAVE_ID,0x5a,0xa9);
		sccb_write(SLAVE_ID,0x5b,0x64);
		sccb_write(SLAVE_ID,0x5c,0x84);
		sccb_write(SLAVE_ID,0x5d,0x53);
		sccb_write(SLAVE_ID,0x5e,0xe );
		sccb_write(SLAVE_ID,0x6c,0x0c);
		sccb_write(SLAVE_ID,0x6d,0x55);
		sccb_write(SLAVE_ID,0x6e,0x0 );
		sccb_write(SLAVE_ID,0x6f,0x9e);

		sccb_write(SLAVE_ID,0x62,0x0 );
		sccb_write(SLAVE_ID,0x63,0x0 );
		sccb_write(SLAVE_ID,0x64,0x2 );
		sccb_write(SLAVE_ID,0x65,0x20);
		sccb_write(SLAVE_ID,0x66,0x1 );
		sccb_write(SLAVE_ID,0x9d,0x2 );
		sccb_write(SLAVE_ID,0x9e,0x2 );

		sccb_write(SLAVE_ID,0x29,0x15);
		sccb_write(SLAVE_ID,0xa9,0xef);
	}
	else if ((nWidthH == 1280) &&(nWidthV == 1024))
	{	// For SXGA
		sccb_write(SLAVE_ID,0x13,0x00);
		sccb_write(SLAVE_ID,0x00,0x00);
		sccb_write(SLAVE_ID,0x01,0x80);
		sccb_write(SLAVE_ID,0x02,0x80);
		sccb_write(SLAVE_ID,0x03,0x1b);
		sccb_write(SLAVE_ID,0x0e,0x61);
		sccb_write(SLAVE_ID,0x0f,0x42);
		sccb_write(SLAVE_ID,0x10,0xf0);

	//	sccb_write(SLAVE_ID,0x11,0x00);
		sccb_write(SLAVE_ID,0x11,0x40);	// use external clock directly

		if (uFlag & FT_CSI_YUVIN)
			sccb_write(SLAVE_ID,0x12,0x02);
		else
			sccb_write(SLAVE_ID,0x12,0x00);

		sccb_write(SLAVE_ID,0x14,0x3a);
		sccb_write(SLAVE_ID,0x16,0x24);
		sccb_write(SLAVE_ID,0x17,0x1d);
		sccb_write(SLAVE_ID,0x18,0xbd);
		sccb_write(SLAVE_ID,0x19,0x01);
		sccb_write(SLAVE_ID,0x1a,0x81);
		sccb_write(SLAVE_ID,0x1e,0x04);
		sccb_write(SLAVE_ID,0x24,0x3c);
		sccb_write(SLAVE_ID,0x25,0x36);
		sccb_write(SLAVE_ID,0x26,0x72);
		sccb_write(SLAVE_ID,0x27,0x08);
		sccb_write(SLAVE_ID,0x28,0x08);
		sccb_write(SLAVE_ID,0x2a,0x00);
		sccb_write(SLAVE_ID,0x2b,0x00);
		sccb_write(SLAVE_ID,0x2c,0x08);
		sccb_write(SLAVE_ID,0x32,0xff);
		sccb_write(SLAVE_ID,0x33,0x00);
		sccb_write(SLAVE_ID,0x34,0x3d);
		sccb_write(SLAVE_ID,0x35,0x00);
		sccb_write(SLAVE_ID,0x36,0xF0);
		sccb_write(SLAVE_ID,0x39,0x57);
		sccb_write(SLAVE_ID,0x3a,0x80);
		sccb_write(SLAVE_ID,0x3b,0xac);
		sccb_write(SLAVE_ID,0x3d,0x99);
		sccb_write(SLAVE_ID,0x3e,0x0c);
		sccb_write(SLAVE_ID,0x3f,0x42);
		sccb_write(SLAVE_ID,0x41,0x40);
		sccb_write(SLAVE_ID,0x42,0xc0);

		//sccb_write(SLAVE_ID,0x6b,0x5a);
		sccb_write(SLAVE_ID,0x6b,0x0a);		// Bypass PLL

		sccb_write(SLAVE_ID,0x71,0x78);
		sccb_write(SLAVE_ID,0x72,0x00);
		sccb_write(SLAVE_ID,0x73,0x01);
		sccb_write(SLAVE_ID,0x74,0x3a);
		sccb_write(SLAVE_ID,0x75,0x35);
		sccb_write(SLAVE_ID,0x76,0x01);
		sccb_write(SLAVE_ID,0x77,0x02);
		sccb_write(SLAVE_ID,0x7a,0x20);
		sccb_write(SLAVE_ID,0x7b,0x1C);
		sccb_write(SLAVE_ID,0x7c,0x28);
		sccb_write(SLAVE_ID,0x7d,0x3C);
		sccb_write(SLAVE_ID,0x7e,0x5A);
		sccb_write(SLAVE_ID,0x7f,0x68);
		sccb_write(SLAVE_ID,0x80,0x76);
		sccb_write(SLAVE_ID,0x81,0x80);
		sccb_write(SLAVE_ID,0x82,0x88);
		sccb_write(SLAVE_ID,0x83,0x8f);
		sccb_write(SLAVE_ID,0x84,0x96);
		sccb_write(SLAVE_ID,0x85,0xa3);
		sccb_write(SLAVE_ID,0x86,0xaf);
		sccb_write(SLAVE_ID,0x87,0xc4);
		sccb_write(SLAVE_ID,0x88,0xd7);
		sccb_write(SLAVE_ID,0x89,0xe8);
		sccb_write(SLAVE_ID,0x8a,0x23);
		sccb_write(SLAVE_ID,0x8c,0x0d);
		sccb_write(SLAVE_ID,0x90,0x20);
		sccb_write(SLAVE_ID,0x91,0x20);

		sccb_write(SLAVE_ID,0x9f,0x20);
		sccb_write(SLAVE_ID,0xa0,0x20);
		sccb_write(SLAVE_ID,0xa4,0x50);
		sccb_write(SLAVE_ID,0xa5,0x68);
		sccb_write(SLAVE_ID,0xa8,0xc1);
		sccb_write(SLAVE_ID,0xa9,0xfa);
		sccb_write(SLAVE_ID,0xaa,0x92);
		sccb_write(SLAVE_ID,0xab,0x04);
		sccb_write(SLAVE_ID,0xac,0x80);
		sccb_write(SLAVE_ID,0xad,0x80);
		sccb_write(SLAVE_ID,0xae,0x80);
		sccb_write(SLAVE_ID,0xaf,0x80);
		sccb_write(SLAVE_ID,0xb2,0xf2);
		sccb_write(SLAVE_ID,0xb3,0x20);
		sccb_write(SLAVE_ID,0xb4,0x20);
		sccb_write(SLAVE_ID,0xb5,0x52);
		sccb_write(SLAVE_ID,0xb6,0xaf);
		sccb_write(SLAVE_ID,0xbb,0xae);
		sccb_write(SLAVE_ID,0xb5,0x00);
		sccb_write(SLAVE_ID,0xc1,0xc8);
		sccb_write(SLAVE_ID,0xc2,0x01);
		sccb_write(SLAVE_ID,0xc3,0x4e);
		sccb_write(SLAVE_ID,0xC6,0x85);
		sccb_write(SLAVE_ID,0xc7,0x80);
		sccb_write(SLAVE_ID,0xc9,0xe0);
		sccb_write(SLAVE_ID,0xca,0xe8);
		sccb_write(SLAVE_ID,0xcb,0xf0);
		sccb_write(SLAVE_ID,0xcc,0xd8);
		sccb_write(SLAVE_ID,0xcd,0x93);

		sccb_write(SLAVE_ID,0x4f,0x98);
		sccb_write(SLAVE_ID,0x50,0x98);
		sccb_write(SLAVE_ID,0x51,0x00);
		sccb_write(SLAVE_ID,0x52,0x28);
		sccb_write(SLAVE_ID,0x53,0x70);
		sccb_write(SLAVE_ID,0x54,0x98);
		sccb_write(SLAVE_ID,0x3b,0xcc);

		sccb_write(SLAVE_ID,0x43,0x14);
		sccb_write(SLAVE_ID,0x44,0xf0);
		sccb_write(SLAVE_ID,0x45,0x46);
		sccb_write(SLAVE_ID,0x46,0x62);
		sccb_write(SLAVE_ID,0x47,0x2a);
		sccb_write(SLAVE_ID,0x48,0x3c);
		sccb_write(SLAVE_ID,0x59,0x85);
		sccb_write(SLAVE_ID,0x5a,0xa9);
		sccb_write(SLAVE_ID,0x5b,0x64);
		sccb_write(SLAVE_ID,0x5c,0x84);
		sccb_write(SLAVE_ID,0x5d,0x53);
		sccb_write(SLAVE_ID,0x5e,0xe );
		sccb_write(SLAVE_ID,0x6c,0x0c);
		sccb_write(SLAVE_ID,0x6d,0x55);
		sccb_write(SLAVE_ID,0x6e,0x0 );
		sccb_write(SLAVE_ID,0x6f,0x9e);

		sccb_write(SLAVE_ID,0x62,0x0 );
		sccb_write(SLAVE_ID,0x63,0x0 );
		sccb_write(SLAVE_ID,0x64,0x2 );
		sccb_write(SLAVE_ID,0x65,0x20);
		sccb_write(SLAVE_ID,0x66,0x0 );
		sccb_write(SLAVE_ID,0x9d,0x2 );
		sccb_write(SLAVE_ID,0x9e,0x2 );

		sccb_write(SLAVE_ID,0x29,0x15);
		sccb_write(SLAVE_ID,0xa9,0xef);

		sccb_write(SLAVE_ID,0x13,0xe7);
//		sccb_write(SLAVE_ID,0x1e,0x34);//Flip & Mirror
	}
	//Flip & Mirror
	sccb_write(SLAVE_ID,0x1e,0x34);

	R_CSI_TG_CTRL1 = uCtrlReg2;					//*P_Sensor_TG_Ctrl2 = uCtrlReg2;
	R_CSI_TG_CTRL0 = uCtrlReg1;					//*P_Sensor_TG_Ctrl1 = uCtrlReg1;
}
#endif

#ifdef	__OV7680_DRV_C__
//====================================================================================================
//	Description:	OV7680 Initialization
//	Syntax:			void OV7680_Init (
//						INT16S nWidthH,			// Active H Width
//						INT16S nWidthV,			// Active V Width
//						INT16U nFlag				// Flag Type
//					);
//	Return:			None
//====================================================================================================
void OV7680_Init (
	INT16S nWidthH,			// Active H Width
	INT16S nWidthV,			// Active V Width
	INT16U nFlag				// Flag Type
) {
}
#endif
#ifdef	__OV7670_DRV_C__
//====================================================================================================
//	Description:	OV7670 Initialization
//	Syntax:			void OV7670_Init (
//						INT16S nWidthH,			// Active H Width
//						INT16S nWidthV,			// Active V Width
//						INT16U uFlag				// Flag Type
//					);
//	Return:			None
//====================================================================================================
void OV7670_Init (
	INT16S nWidthH,			// Active H Width
	INT16S nWidthV,			// Active V Width
	INT16U uFlag				// Flag Type
) {
	INT16U uCtrlReg1, uCtrlReg2;
	INT32U SPI_Ctrl_temp;
	INT16S nReso;

DBG_PRINT("OV7670_Init()\r\n");

//	csi_fifo_flag = 1;

	// Enable CSI clock to let sensor initialize at first
#if CSI_CLOCK == CSI_CLOCK_27MHZ
	uCtrlReg2 = CLKOEN | CSI_RGB565 |CLK_SEL27M | CSI_HIGHPRI | CSI_NOSTOP;
#else
	uCtrlReg2 = CLKOEN | CSI_RGB565 |CLK_SEL48M | CSI_HIGHPRI | CSI_NOSTOP;
#endif

	uCtrlReg1 = CSIEN | YUV_YUYV | CAP;									// Default CSI Control Register 1
	if (uFlag & FT_CSI_RGB1555)											// RGB1555
	{
		uCtrlReg2 |= CSI_RGB1555;
	}
	if (uFlag & FT_CSI_CCIR656)										// CCIR656?
	{
		uCtrlReg1 |= CCIR656 | VADD_FALL | VRST_FALL | HRST_FALL;	// CCIR656
		uCtrlReg2 |= D_TYPE1;										// CCIR656
	}
	else
	{
		uCtrlReg1 |= VADD_RISE | VRST_FALL | HRST_RISE | HREF;		// NOT CCIR656
		uCtrlReg2 |= D_TYPE0;										// NOT CCIR656
	}
	if (uFlag & FT_CSI_YUVIN)										// YUVIN?
	{
		uCtrlReg1 |= YUVIN;
	}
	if (uFlag & FT_CSI_YUVOUT)										// YUVOUT?
	{
		uCtrlReg1 |= YUVOUT;
	}

	// Whether compression or not?
	nReso = ((nWidthH == 320) && (nWidthV == 240)) ? 1 : 0;
	if (nReso == 1)								// VGA
	{
#ifdef	__TV_QVGA__
		R_CSI_TG_HRATIO = 0x0102;					// Scale to 1/2
		R_CSI_TG_VRATIO = 0x0102;					// Scale to 1/2
		R_CSI_TG_HWIDTH = nWidthH;					// Horizontal frame width
		R_CSI_TG_VHEIGHT = nWidthV*2;				// Vertical frame width
#endif	// __TV_QVGA__
	}
	else
	{
		R_CSI_TG_HRATIO = 0;
		R_CSI_TG_VRATIO = 0;
	}

	R_CSI_TG_VL0START = 0x0000;						// Sensor field 0 vertical latch start register.
	R_CSI_TG_VL1START = 0x0000;						//*P_Sensor_TG_V_L1Start = 0x0000;
	R_CSI_TG_HSTART = 0x0000;						// Sensor horizontal start register.

	R_CSI_TG_CTRL0 = 0;								//reset control0
	R_CSI_TG_CTRL1 = CSI_NOSTOP|CLKOEN;				//enable CSI CLKO
	drv_msec_wait(100); 							//wait 100ms for CLKO stable

	SPI_Ctrl_temp = R_SPI0_CTRL;
	R_SPI0_CTRL = 0;

	// CMOS Sensor Initialization Start...
	sccb_init (SCCB_SCL, SCCB_SDA);
	sccb_delay (200);
	sccb_write (SLAVE_ID, 0x12, 0x80);
	//sccb_write (SLAVE_ID, 0x09, 0x03);
	//testREG = sccb_read(SLAVE_ID, 0xB);
	sccb_delay (200);

	//if((nWidthH == 640) &&(nWidthV == 480))
	{
#if CSI_FPS	== CSI_30FPS
		sccb_write(SLAVE_ID, 0x11, 0x02);	 // 30fps
		sccb_write(SLAVE_ID, 0x6b, 0x8a);	// pclk*6
#elif CSI_FPS	== CSI_27FPS
		sccb_write(SLAVE_ID, 0x11, 0x04);    // 27fps
		sccb_write(SLAVE_ID, 0x6b, 0xca);	// pclk*8
#elif CSI_FPS	== CSI_15FPS
		//sccb_write(SLAVE_ID, 0x11, 0x06);    // 14.5fps
		//sccb_write(SLAVE_ID, 0x11, 0x09);    // 13.5fps
		sccb_write(SLAVE_ID, 0x11, 0x08);    // 15fps
		sccb_write(SLAVE_ID, 0x6b, 0xca);	// pclk*8
#elif CSI_FPS == CSI_10FPS
		sccb_write(SLAVE_ID, 0x11, 0x06);    // 10fps
		sccb_write(SLAVE_ID, 0x6b, 0x4a);
#elif CSI_FPS == CSI_07FPS
		sccb_write(SLAVE_ID, 0x11, 0x09);    // 6.7fps
		sccb_write(SLAVE_ID, 0x6b, 0x4a);
#endif
		sccb_write(SLAVE_ID, 0x3A, 0x04);
		sccb_write(SLAVE_ID, 0x12, 0x00);
		sccb_write(SLAVE_ID, 0x17, 0x13);
		sccb_write(SLAVE_ID, 0x18, 0x01);
		sccb_write(SLAVE_ID, 0x32, 0xB6);
		sccb_write(SLAVE_ID, 0x19, 0x02);
		sccb_write(SLAVE_ID, 0x1A, 0x7A);
		sccb_write(SLAVE_ID, 0x03, 0x0F);
		sccb_write(SLAVE_ID, 0x0C, 0x00);
		sccb_write(SLAVE_ID, 0x3E, 0x00);
		sccb_write(SLAVE_ID, 0x70, 0x3A);
		sccb_write(SLAVE_ID, 0x71, 0x35);
		sccb_write(SLAVE_ID, 0x72, 0x11);
		sccb_write(SLAVE_ID, 0x73, 0xF0);
		sccb_write(SLAVE_ID, 0xA2, 0x3B);
		sccb_write(SLAVE_ID, 0x1E, 0x0F);
		sccb_write(SLAVE_ID, 0x7a, 0x20);
		sccb_write(SLAVE_ID, 0x7b, 0x03);
		sccb_write(SLAVE_ID, 0x7c, 0x0a);
		sccb_write(SLAVE_ID, 0x7d, 0x1a);
		sccb_write(SLAVE_ID, 0x7e, 0x3f);
		sccb_write(SLAVE_ID, 0x7f, 0x4e);
		sccb_write(SLAVE_ID, 0x80, 0x5b);
		sccb_write(SLAVE_ID, 0x81, 0x68);
		sccb_write(SLAVE_ID, 0x82, 0x75);
		sccb_write(SLAVE_ID, 0x83, 0x7f);
		sccb_write(SLAVE_ID, 0x84, 0x89);
		sccb_write(SLAVE_ID, 0x85, 0x9a);
		sccb_write(SLAVE_ID, 0x86, 0xa6);
		sccb_write(SLAVE_ID, 0x87, 0xbd);
		sccb_write(SLAVE_ID, 0x88, 0xd3);
		sccb_write(SLAVE_ID, 0x89, 0xe8);
		sccb_write(SLAVE_ID, 0x13, 0xE0);
		sccb_write(SLAVE_ID, 0x00, 0x00);
		sccb_write(SLAVE_ID, 0x10, 0x00);
		sccb_write(SLAVE_ID, 0x0D, 0x50);
		sccb_write(SLAVE_ID, 0x42, 0x40);
		sccb_write(SLAVE_ID, 0x14, 0x28);
		sccb_write(SLAVE_ID, 0xA5, 0x03);
		sccb_write(SLAVE_ID, 0xAB, 0x03);
		sccb_write(SLAVE_ID, 0x24, 0x50);
		sccb_write(SLAVE_ID, 0x25, 0x43);
		sccb_write(SLAVE_ID, 0x26, 0xa3);
		sccb_write(SLAVE_ID, 0x9F, 0x78);
		sccb_write(SLAVE_ID, 0xA0, 0x68);
		sccb_write(SLAVE_ID, 0xA1, 0x03);
		sccb_write(SLAVE_ID, 0xA6, 0xd2);
		sccb_write(SLAVE_ID, 0xA7, 0xd2);
		sccb_write(SLAVE_ID, 0xA8, 0xF0);
		sccb_write(SLAVE_ID, 0xA9, 0x80);
		sccb_write(SLAVE_ID, 0xAA, 0x14);
		sccb_write(SLAVE_ID, 0x13, 0xE5);
		sccb_write(SLAVE_ID, 0x0E, 0x61);
		sccb_write(SLAVE_ID, 0x0F, 0x4B); 	// Flip (bit4) & Mirror (bit5)
		sccb_write(SLAVE_ID, 0x16, 0x02);
		sccb_write(SLAVE_ID, 0x21, 0x02);
		sccb_write(SLAVE_ID, 0x22, 0x91);
		sccb_write(SLAVE_ID, 0x29, 0x07);
		sccb_write(SLAVE_ID, 0x33, 0x0B);
		sccb_write(SLAVE_ID, 0x35, 0x0B);
		sccb_write(SLAVE_ID, 0x37, 0x1D);
		sccb_write(SLAVE_ID, 0x38, 0x71);
		sccb_write(SLAVE_ID, 0x39, 0x2A);
		sccb_write(SLAVE_ID, 0x3C, 0x78);
		sccb_write(SLAVE_ID, 0x4D, 0x40);
		sccb_write(SLAVE_ID, 0x4E, 0x20);
		sccb_write(SLAVE_ID, 0x69, 0x00);

		sccb_write(SLAVE_ID, 0x74, 0x10);
		sccb_write(SLAVE_ID, 0x8D, 0x4F);
		sccb_write(SLAVE_ID, 0x8E, 0x00);
		sccb_write(SLAVE_ID, 0x8F, 0x00);
		sccb_write(SLAVE_ID, 0x90, 0x00);
		sccb_write(SLAVE_ID, 0x91, 0x00);
		sccb_write(SLAVE_ID, 0x96, 0x00);
		sccb_write(SLAVE_ID, 0x9A, 0x80);
		sccb_write(SLAVE_ID, 0xB0, 0x84);
		sccb_write(SLAVE_ID, 0xB1, 0x0C);
		sccb_write(SLAVE_ID, 0xB2, 0x0E);
		sccb_write(SLAVE_ID, 0xB3, 0x82);
		sccb_write(SLAVE_ID, 0xB8, 0x0A);
		sccb_write(SLAVE_ID, 0x43, 0x02);
		sccb_write(SLAVE_ID, 0x44, 0xf2);
		sccb_write(SLAVE_ID, 0x45, 0x46);
		sccb_write(SLAVE_ID, 0x46, 0x63);
		sccb_write(SLAVE_ID, 0x47, 0x32);
		sccb_write(SLAVE_ID, 0x48, 0x3b);
		sccb_write(SLAVE_ID, 0x59, 0x92);
		sccb_write(SLAVE_ID, 0x5a, 0x9b);
		sccb_write(SLAVE_ID, 0x5b, 0xa5);
		sccb_write(SLAVE_ID, 0x5c, 0x7a);
		sccb_write(SLAVE_ID, 0x5d, 0x4a);
		sccb_write(SLAVE_ID, 0x5e, 0x0a);
		sccb_write(SLAVE_ID, 0x6c, 0x0a);
		sccb_write(SLAVE_ID, 0x6d, 0x55);
		sccb_write(SLAVE_ID, 0x6e, 0x11);
		sccb_write(SLAVE_ID, 0x6f, 0x9e);
		sccb_write(SLAVE_ID, 0x6A, 0x40);
		sccb_write(SLAVE_ID, 0x01, 0x40);
		sccb_write(SLAVE_ID, 0x02, 0x40);
		sccb_write(SLAVE_ID, 0x13, 0xf7);
		sccb_write(SLAVE_ID, 0x4f, 0x9c);
		sccb_write(SLAVE_ID, 0x50, 0x99);
		sccb_write(SLAVE_ID, 0x51, 0x02);
		sccb_write(SLAVE_ID, 0x52, 0x29);
		sccb_write(SLAVE_ID, 0x53, 0x8b);
		sccb_write(SLAVE_ID, 0x54, 0xb5);
		sccb_write(SLAVE_ID, 0x58, 0x1e);
		sccb_write(SLAVE_ID, 0x62, 0x08);
		sccb_write(SLAVE_ID, 0x63, 0x10);
		sccb_write(SLAVE_ID, 0x64, 0x04);
		sccb_write(SLAVE_ID, 0x65, 0x00);
		sccb_write(SLAVE_ID, 0x66, 0x05);
		sccb_write(SLAVE_ID, 0x94, 0x04);
		sccb_write(SLAVE_ID, 0x95, 0x06);
		sccb_write(SLAVE_ID, 0x41, 0x08);
		sccb_write(SLAVE_ID, 0x3F, 0x00);
		sccb_write(SLAVE_ID, 0x75, 0x44);
		sccb_write(SLAVE_ID, 0x76, 0xe1);
		sccb_write(SLAVE_ID, 0x4C, 0x00);
		sccb_write(SLAVE_ID, 0x77, 0x01);
		sccb_write(SLAVE_ID, 0x3D, 0xC0);
		sccb_write(SLAVE_ID, 0x4B, 0x09);
		sccb_write(SLAVE_ID, 0xC9, 0x60);
		sccb_write(SLAVE_ID, 0x41, 0x38);
		sccb_write(SLAVE_ID, 0x56, 0x40);
		sccb_write(SLAVE_ID, 0x34, 0x11);
		sccb_write(SLAVE_ID, 0x3b, 0x02);
		sccb_write(SLAVE_ID, 0xa4, 0x88);	//disable  night mode
		sccb_write(SLAVE_ID, 0x92, 0x00);
		sccb_write(SLAVE_ID, 0x96, 0x00);
		sccb_write(SLAVE_ID, 0x97, 0x30);
		sccb_write(SLAVE_ID, 0x98, 0x20);
		sccb_write(SLAVE_ID, 0x99, 0x20);
		sccb_write(SLAVE_ID, 0x9A, 0x84);
		sccb_write(SLAVE_ID, 0x9B, 0x29);
		sccb_write(SLAVE_ID, 0x9C, 0x03);
		sccb_write(SLAVE_ID, 0x9D, 0x99);
		sccb_write(SLAVE_ID, 0x9E, 0x7F);
		sccb_write(SLAVE_ID, 0x78, 0x00);
		sccb_write(SLAVE_ID, 0x79, 0x01);
		sccb_write(SLAVE_ID, 0xc8, 0xf0);
		sccb_write(SLAVE_ID, 0x79, 0x0f);
		sccb_write(SLAVE_ID, 0xc8, 0x00);
		sccb_write(SLAVE_ID, 0x79, 0x10);
		sccb_write(SLAVE_ID, 0xc8, 0x7e);
		sccb_write(SLAVE_ID, 0x79, 0x0a);
		sccb_write(SLAVE_ID, 0xc8, 0x80);
		sccb_write(SLAVE_ID, 0x79, 0x0b);
		sccb_write(SLAVE_ID, 0xc8, 0x01);
		sccb_write(SLAVE_ID, 0x79, 0x0c);
		sccb_write(SLAVE_ID, 0xc8, 0x0f);
		sccb_write(SLAVE_ID, 0x79, 0x0d);
		sccb_write(SLAVE_ID, 0xc8, 0x20);
		sccb_write(SLAVE_ID, 0x79, 0x09);
		sccb_write(SLAVE_ID, 0xc8, 0x80);
		sccb_write(SLAVE_ID, 0x79, 0x02);
		sccb_write(SLAVE_ID, 0xc8, 0xc0);
		sccb_write(SLAVE_ID, 0x79, 0x03);
		sccb_write(SLAVE_ID, 0xc8, 0x40);
		sccb_write(SLAVE_ID, 0x79, 0x05);
		sccb_write(SLAVE_ID, 0xc8, 0x30);
		sccb_write(SLAVE_ID, 0x79, 0x26);
		sccb_write(SLAVE_ID, 0x3b, 0x02);
		sccb_write(SLAVE_ID, 0x43, 0x02);
		sccb_write(SLAVE_ID, 0x44, 0xf2);
		sccb_write(SLAVE_ID, 0x30, 0x4F);
		
		sccb_write(SLAVE_ID, 0x09, 0x00); 
	}
	R_SPI0_CTRL = SPI_Ctrl_temp;

	R_CSI_TG_CTRL1 = uCtrlReg2;					//*P_Sensor_TG_Ctrl2 = uCtrlReg2;
#if 0
#if CSI_MODE == CSI_PPU_FRAME_MODE
	R_CSI_TG_CTRL0 = uCtrlReg1;					//*P_Sensor_TG_Ctrl1 = uCtrlReg1;
#elif CSI_MODE == CSI_TG_FRAME_MODE
	R_CSI_TG_CTRL0 = uCtrlReg1|0x010000;
#elif CSI_MODE == CSI_FIFO_8_MODE
	R_CSI_TG_CTRL0 = uCtrlReg1|0x110000;
#elif CSI_MODE == CSI_FIFO_16_MODE
	R_CSI_TG_CTRL0 = uCtrlReg1|0x210000;
#elif CSI_MODE == CSI_FIFO_32_MODE
	R_CSI_TG_CTRL0 = uCtrlReg1|0x310000;
#endif
#endif

	if (frame_mode_en == 1) { // CSI_MODE == CSI_TG_FRAME_MODE
		R_CSI_TG_CTRL0 = uCtrlReg1|0x010000;
	} else { // CSI_MODE == CSI_FIFO_32_MODE
		R_CSI_TG_CTRL0 = uCtrlReg1|0x310000;
	}
}
#endif

#ifdef	__OV2643_DRV_C__
//====================================================================================================
//	Description:	OV2643 Initialization
//	Syntax:			void OV2643_Init (
//						INT16S nWidthH,			// Active H Width
//						INT16S nWidthV,			// Active V Width
//						INT16U uFlag				// Flag Type
//					);
//	Return:			None
//====================================================================================================
void OV2643_Init (
	INT16S nWidthH,			// Active H Width
	INT16S nWidthV,			// Active V Width
	INT16U uFlag				// Flag Type
) {
	INT16U uCtrlReg1, uCtrlReg2;
	
	// Enable CSI clock to let sensor initialize at first
	// input clock : 6~27MHz or 56MHz 
	uCtrlReg2 = CLKOEN | CSI_RGB565 |CLK_SEL27M | CSI_HIGHPRI | CSI_NOSTOP;
	uCtrlReg1 = CSIEN | YUV_YUYV | CAP;									// Default CSI Control Register 1
	if (uFlag & FT_CSI_RGB1555)											// RGB1555
	{
		uCtrlReg2 |= CSI_RGB1555;
	}
	if (uFlag & FT_CSI_CCIR656)										// CCIR656?
	{
		uCtrlReg1 |= CCIR656 | VADD_FALL | VRST_FALL | HRST_FALL;	// CCIR656
		uCtrlReg2 |= D_TYPE1;										// CCIR656
	}
	else
	{
		uCtrlReg1 |= VADD_RISE | VRST_FALL | HRST_RISE | HREF;		// NOT CCIR656
		uCtrlReg2 |= D_TYPE0;										// NOT CCIR656
	}
	if (uFlag & FT_CSI_YUVIN)										// YUVIN?
	{
		uCtrlReg1 |= YUVIN;
	}
	if (uFlag & FT_CSI_YUVOUT)						// YUVOUT?
	{
		uCtrlReg1 |= YUVOUT;
	}
	
	R_CSI_TG_HRATIO = 0;							//no scaler					
	R_CSI_TG_VRATIO = 0;

	R_CSI_TG_VL0START = 0x0000;						// Sensor field 0 vertical latch start register.
	R_CSI_TG_VL1START = 0x0000;						//*P_Sensor_TG_V_L1Start = 0x0000;				
	R_CSI_TG_HSTART = 0x0000;						// Sensor horizontal start register.
	
	R_CSI_TG_CTRL0 = 0;								//reset control0
	R_CSI_TG_CTRL1 = CSI_NOSTOP|CLKOEN;				//enable CSI CLKO
	drv_msec_wait(100); 							//wait 100ms for CLKO stable

	// CMOS Sensor Initialization Start...
	sccb_init (SCCB_SCL, SCCB_SDA);
	sccb_delay (200);
	sccb_write (SLAVE_ID, 0x12, 0x80);
	sccb_delay (200);
				
	if (nWidthH == 400 && nWidthV == 300)
	{	//CIF 400x300, 30fps 
		sccb_write (SLAVE_ID, 0xc3, 0x1f);
		sccb_write (SLAVE_ID, 0xc4, 0xff);
		sccb_write (SLAVE_ID, 0x3d, 0x48); 
		sccb_write (SLAVE_ID, 0xdd, 0xa5);
		sccb_write (SLAVE_ID, 0x0e, 0x10);
		sccb_write (SLAVE_ID, 0x10, 0x0a);
		sccb_write (SLAVE_ID, 0x11, 0x00);
		sccb_write (SLAVE_ID, 0x0f, 0x14);
		sccb_write (SLAVE_ID, 0x0e, 0x10);
		
		sccb_write (SLAVE_ID, 0x20, 0x01);
		sccb_write (SLAVE_ID, 0x21, 0x98);
		sccb_write (SLAVE_ID, 0x22, 0x00);
		sccb_write (SLAVE_ID, 0x23, 0x06);
		sccb_write (SLAVE_ID, 0x24, 0x32);
		sccb_write (SLAVE_ID, 0x26, 0x25);
		sccb_write (SLAVE_ID, 0x27, 0x84);
		sccb_write (SLAVE_ID, 0x29, 0x05);
		sccb_write (SLAVE_ID, 0x2a, 0xdc);
		sccb_write (SLAVE_ID, 0x2b, 0x03);
		sccb_write (SLAVE_ID, 0x2c, 0x20);
		sccb_write (SLAVE_ID, 0x1d, 0x04);
		sccb_write (SLAVE_ID, 0x25, 0x04);
		sccb_write (SLAVE_ID, 0x27, 0x84);
		sccb_write (SLAVE_ID, 0x28, 0x40);
		sccb_write (SLAVE_ID, 0x12, 0x3a);	//mirror and flip
		sccb_write (SLAVE_ID, 0x39, 0xd0);
		sccb_write (SLAVE_ID, 0xcd, 0x13);	
		
		sccb_write (SLAVE_ID, 0x13, 0xff);
		sccb_write (SLAVE_ID, 0x14, 0xa7);
		sccb_write (SLAVE_ID, 0x15, 0x42);
		sccb_write (SLAVE_ID, 0x3c, 0xa4);
		sccb_write (SLAVE_ID, 0x18, 0x60);
		sccb_write (SLAVE_ID, 0x19, 0x50);
		sccb_write (SLAVE_ID, 0x1a, 0xe2);
		sccb_write (SLAVE_ID, 0x37, 0xe8);
		sccb_write (SLAVE_ID, 0x16, 0x90);
		sccb_write (SLAVE_ID, 0x43, 0x00);
		sccb_write (SLAVE_ID, 0x40, 0xfb);
		sccb_write (SLAVE_ID, 0xa9, 0x44);
		sccb_write (SLAVE_ID, 0x2f, 0xec);
		sccb_write (SLAVE_ID, 0x35, 0x10);
		sccb_write (SLAVE_ID, 0x36, 0x10);
		sccb_write (SLAVE_ID, 0x0c, 0x00);
		sccb_write (SLAVE_ID, 0x0d, 0x00);
		sccb_write (SLAVE_ID, 0xd0, 0x93);
		sccb_write (SLAVE_ID, 0xdc, 0x2b);
		sccb_write (SLAVE_ID, 0xd9, 0x41);
		sccb_write (SLAVE_ID, 0xd3, 0x02);
		
		sccb_write (SLAVE_ID, 0xde, 0x7c);
		
		sccb_write (SLAVE_ID, 0x3d, 0x08);
		sccb_write (SLAVE_ID, 0x0c, 0x00);
		sccb_write (SLAVE_ID, 0x18, 0x2c);
		sccb_write (SLAVE_ID, 0x19, 0x24);
		sccb_write (SLAVE_ID, 0x1a, 0x71);
		sccb_write (SLAVE_ID, 0x9b, 0x69);
		sccb_write (SLAVE_ID, 0x9c, 0x7d);
		sccb_write (SLAVE_ID, 0x9d, 0x7d);
		sccb_write (SLAVE_ID, 0x9e, 0x69);
		sccb_write (SLAVE_ID, 0x35, 0x04);
		sccb_write (SLAVE_ID, 0x36, 0x04);
		sccb_write (SLAVE_ID, 0x65, 0x12);
		sccb_write (SLAVE_ID, 0x66, 0x20);
		sccb_write (SLAVE_ID, 0x67, 0x39);
		sccb_write (SLAVE_ID, 0x68, 0x4e);
		sccb_write (SLAVE_ID, 0x69, 0x62);
		sccb_write (SLAVE_ID, 0x6a, 0x74);
		sccb_write (SLAVE_ID, 0x6b, 0x85);
		sccb_write (SLAVE_ID, 0x6c, 0x92);
		sccb_write (SLAVE_ID, 0x6d, 0x9e);
		sccb_write (SLAVE_ID, 0x6e, 0xb2);
		sccb_write (SLAVE_ID, 0x6f, 0xc0);
		sccb_write (SLAVE_ID, 0x70, 0xcc);
		sccb_write (SLAVE_ID, 0x71, 0xe0);
		sccb_write (SLAVE_ID, 0x72, 0xee);
		sccb_write (SLAVE_ID, 0x73, 0xf6);
		sccb_write (SLAVE_ID, 0x74, 0x11);
		sccb_write (SLAVE_ID, 0xab, 0x20);
		sccb_write (SLAVE_ID, 0xac, 0x5b);
		sccb_write (SLAVE_ID, 0xad, 0x05);
		sccb_write (SLAVE_ID, 0xae, 0x1b);
		sccb_write (SLAVE_ID, 0xaf, 0x76);
		sccb_write (SLAVE_ID, 0xb0, 0x90);
		sccb_write (SLAVE_ID, 0xb1, 0x90);
		sccb_write (SLAVE_ID, 0xb2, 0x8c);
		sccb_write (SLAVE_ID, 0xb3, 0x04);
		sccb_write (SLAVE_ID, 0xb4, 0x98);
		sccb_write (SLAVE_ID, 0xbC, 0x03);
		sccb_write (SLAVE_ID, 0x4d, 0x30);
		sccb_write (SLAVE_ID, 0x4e, 0x02);
		sccb_write (SLAVE_ID, 0x4f, 0x5c);
		sccb_write (SLAVE_ID, 0x50, 0x56);
		sccb_write (SLAVE_ID, 0x51, 0x00);
		sccb_write (SLAVE_ID, 0x52, 0x66);
		sccb_write (SLAVE_ID, 0x53, 0x03);
		sccb_write (SLAVE_ID, 0x54, 0x30);
		sccb_write (SLAVE_ID, 0x55, 0x02);
		sccb_write (SLAVE_ID, 0x56, 0x5c);
		sccb_write (SLAVE_ID, 0x57, 0x40);
		sccb_write (SLAVE_ID, 0x58, 0x00);
		sccb_write (SLAVE_ID, 0x59, 0x66);
		sccb_write (SLAVE_ID, 0x5a, 0x03);
		sccb_write (SLAVE_ID, 0x5b, 0x20);
		sccb_write (SLAVE_ID, 0x5c, 0x02);
		sccb_write (SLAVE_ID, 0x5d, 0x5c);
		sccb_write (SLAVE_ID, 0x5e, 0x3a);
		sccb_write (SLAVE_ID, 0x5f, 0x00);
		
		sccb_write (SLAVE_ID, 0x60, 0x66);
		sccb_write (SLAVE_ID, 0x41, 0x1f);
		sccb_write (SLAVE_ID, 0xb5, 0x01);
		sccb_write (SLAVE_ID, 0xb6, 0x02);
		sccb_write (SLAVE_ID, 0xb9, 0x40);
		sccb_write (SLAVE_ID, 0xba, 0x28);
		sccb_write (SLAVE_ID, 0xbf, 0x0c);
		sccb_write (SLAVE_ID, 0xc0, 0x3e);
		sccb_write (SLAVE_ID, 0xa3, 0x0a);
		sccb_write (SLAVE_ID, 0xa4, 0x0f);
		sccb_write (SLAVE_ID, 0xa5, 0x09);
		sccb_write (SLAVE_ID, 0xa6, 0x16);
		sccb_write (SLAVE_ID, 0x9f, 0x0a);
		sccb_write (SLAVE_ID, 0xa0, 0x0f);
		sccb_write (SLAVE_ID, 0xa7, 0x0a);
		sccb_write (SLAVE_ID, 0xa8, 0x0f);
		sccb_write (SLAVE_ID, 0xa1, 0x10);
		sccb_write (SLAVE_ID, 0xa2, 0x04);
		sccb_write (SLAVE_ID, 0xa9, 0x04);
		sccb_write (SLAVE_ID, 0xaa, 0xa6);
		sccb_write (SLAVE_ID, 0x75, 0x6a);
		sccb_write (SLAVE_ID, 0x76, 0x11);
		sccb_write (SLAVE_ID, 0x77, 0x92);
		sccb_write (SLAVE_ID, 0x78, 0x21);
		sccb_write (SLAVE_ID, 0x79, 0xe1);
		sccb_write (SLAVE_ID, 0x7a, 0x02);
		sccb_write (SLAVE_ID, 0x7c, 0x05);
		sccb_write (SLAVE_ID, 0x7d, 0x08);
		sccb_write (SLAVE_ID, 0x7e, 0x08);
		sccb_write (SLAVE_ID, 0x7f, 0x7c);
		sccb_write (SLAVE_ID, 0x80, 0x58);
		sccb_write (SLAVE_ID, 0x81, 0x2a);
		sccb_write (SLAVE_ID, 0x82, 0xc5);
		sccb_write (SLAVE_ID, 0x83, 0x46);
		sccb_write (SLAVE_ID, 0x84, 0x3a);
		sccb_write (SLAVE_ID, 0x85, 0x54);
		sccb_write (SLAVE_ID, 0x86, 0x44);
		sccb_write (SLAVE_ID, 0x87, 0xf8);
		sccb_write (SLAVE_ID, 0x88, 0x08);
		sccb_write (SLAVE_ID, 0x89, 0x70);
		sccb_write (SLAVE_ID, 0x8a, 0xf0);
		sccb_write (SLAVE_ID, 0x8b, 0xf0); 
	}
	else if(nWidthH == 800 && nWidthV == 600)
	{	//SVGA
		sccb_write (SLAVE_ID, 0xc3, 0x1f);
		sccb_write (SLAVE_ID, 0xc4, 0xff);
		sccb_write (SLAVE_ID, 0x3d, 0x48); 
		sccb_write (SLAVE_ID, 0xdd, 0xa5);
		sccb_write (SLAVE_ID, 0x0e, 0x10);
		sccb_write (SLAVE_ID, 0x10, 0x0a);
		sccb_write (SLAVE_ID, 0x11, 0x00);
		sccb_write (SLAVE_ID, 0x0f, 0x14);
		sccb_write (SLAVE_ID, 0x0e, 0x10);
		
		sccb_write (SLAVE_ID, 0x20, 0x01);
		sccb_write (SLAVE_ID, 0x21, 0x98);
		sccb_write (SLAVE_ID, 0x22, 0x00);
		sccb_write (SLAVE_ID, 0x23, 0x06);
		sccb_write (SLAVE_ID, 0x24, 0x32);
		sccb_write (SLAVE_ID, 0x26, 0x25);
		sccb_write (SLAVE_ID, 0x27, 0x84);
		sccb_write (SLAVE_ID, 0x29, 0x05);
		sccb_write (SLAVE_ID, 0x2a, 0xdc);
		sccb_write (SLAVE_ID, 0x2b, 0x03);
		sccb_write (SLAVE_ID, 0x2c, 0x20);
		sccb_write (SLAVE_ID, 0x1d, 0x04);
		sccb_write (SLAVE_ID, 0x25, 0x04);
		sccb_write (SLAVE_ID, 0x27, 0x84);
		sccb_write (SLAVE_ID, 0x28, 0x40);
		sccb_write (SLAVE_ID, 0x12, 0x39);	//mirror and flip
		sccb_write (SLAVE_ID, 0x39, 0xd0);
		sccb_write (SLAVE_ID, 0xcd, 0x13);	
		
		sccb_write (SLAVE_ID, 0x13, 0xff);
		sccb_write (SLAVE_ID, 0x14, 0xa7);
		sccb_write (SLAVE_ID, 0x15, 0x42);
		sccb_write (SLAVE_ID, 0x3c, 0xa4);
		sccb_write (SLAVE_ID, 0x18, 0x60);
		sccb_write (SLAVE_ID, 0x19, 0x50);
		sccb_write (SLAVE_ID, 0x1a, 0xe2);
		sccb_write (SLAVE_ID, 0x37, 0xe8);
		sccb_write (SLAVE_ID, 0x16, 0x90);
		sccb_write (SLAVE_ID, 0x43, 0x00);
		sccb_write (SLAVE_ID, 0x40, 0xfb);
		sccb_write (SLAVE_ID, 0xa9, 0x44);
		sccb_write (SLAVE_ID, 0x2f, 0xec);
		sccb_write (SLAVE_ID, 0x35, 0x10);
		sccb_write (SLAVE_ID, 0x36, 0x10);
		sccb_write (SLAVE_ID, 0x0c, 0x00);
		sccb_write (SLAVE_ID, 0x0d, 0x00);
		sccb_write (SLAVE_ID, 0xd0, 0x93);
		sccb_write (SLAVE_ID, 0xdc, 0x2b);
		sccb_write (SLAVE_ID, 0xd9, 0x41);
		sccb_write (SLAVE_ID, 0xd3, 0x02);
		
		sccb_write (SLAVE_ID, 0xde, 0x7c);
		
		sccb_write (SLAVE_ID, 0x3d, 0x08);
		sccb_write (SLAVE_ID, 0x0c, 0x00);
		sccb_write (SLAVE_ID, 0x18, 0x2c);
		sccb_write (SLAVE_ID, 0x19, 0x24);
		sccb_write (SLAVE_ID, 0x1a, 0x71);
		sccb_write (SLAVE_ID, 0x9b, 0x69);
		sccb_write (SLAVE_ID, 0x9c, 0x7d);
		sccb_write (SLAVE_ID, 0x9d, 0x7d);
		sccb_write (SLAVE_ID, 0x9e, 0x69);
		sccb_write (SLAVE_ID, 0x35, 0x04);
		sccb_write (SLAVE_ID, 0x36, 0x04);
		sccb_write (SLAVE_ID, 0x65, 0x12);
		sccb_write (SLAVE_ID, 0x66, 0x20);
		sccb_write (SLAVE_ID, 0x67, 0x39);
		sccb_write (SLAVE_ID, 0x68, 0x4e);
		sccb_write (SLAVE_ID, 0x69, 0x62);
		sccb_write (SLAVE_ID, 0x6a, 0x74);
		sccb_write (SLAVE_ID, 0x6b, 0x85);
		sccb_write (SLAVE_ID, 0x6c, 0x92);
		sccb_write (SLAVE_ID, 0x6d, 0x9e);
		sccb_write (SLAVE_ID, 0x6e, 0xb2);
		sccb_write (SLAVE_ID, 0x6f, 0xc0);
		sccb_write (SLAVE_ID, 0x70, 0xcc);
		sccb_write (SLAVE_ID, 0x71, 0xe0);
		sccb_write (SLAVE_ID, 0x72, 0xee);
		sccb_write (SLAVE_ID, 0x73, 0xf6);
		sccb_write (SLAVE_ID, 0x74, 0x11);
		sccb_write (SLAVE_ID, 0xab, 0x20);
		sccb_write (SLAVE_ID, 0xac, 0x5b);
		sccb_write (SLAVE_ID, 0xad, 0x05);
		sccb_write (SLAVE_ID, 0xae, 0x1b);
		sccb_write (SLAVE_ID, 0xaf, 0x76);
		sccb_write (SLAVE_ID, 0xb0, 0x90);
		sccb_write (SLAVE_ID, 0xb1, 0x90);
		sccb_write (SLAVE_ID, 0xb2, 0x8c);
		sccb_write (SLAVE_ID, 0xb3, 0x04);
		sccb_write (SLAVE_ID, 0xb4, 0x98);
		sccb_write (SLAVE_ID, 0xbC, 0x03);
		sccb_write (SLAVE_ID, 0x4d, 0x30);
		sccb_write (SLAVE_ID, 0x4e, 0x02);
		sccb_write (SLAVE_ID, 0x4f, 0x5c);
		sccb_write (SLAVE_ID, 0x50, 0x56);
		sccb_write (SLAVE_ID, 0x51, 0x00);
		sccb_write (SLAVE_ID, 0x52, 0x66);
		sccb_write (SLAVE_ID, 0x53, 0x03);
		sccb_write (SLAVE_ID, 0x54, 0x30);
		sccb_write (SLAVE_ID, 0x55, 0x02);
		sccb_write (SLAVE_ID, 0x56, 0x5c);
		sccb_write (SLAVE_ID, 0x57, 0x40);
		sccb_write (SLAVE_ID, 0x58, 0x00);
		sccb_write (SLAVE_ID, 0x59, 0x66);
		sccb_write (SLAVE_ID, 0x5a, 0x03);
		sccb_write (SLAVE_ID, 0x5b, 0x20);
		sccb_write (SLAVE_ID, 0x5c, 0x02);
		sccb_write (SLAVE_ID, 0x5d, 0x5c);
		sccb_write (SLAVE_ID, 0x5e, 0x3a);
		sccb_write (SLAVE_ID, 0x5f, 0x00);
		
		sccb_write (SLAVE_ID, 0x60, 0x66);
		sccb_write (SLAVE_ID, 0x41, 0x1f);
		sccb_write (SLAVE_ID, 0xb5, 0x01);
		sccb_write (SLAVE_ID, 0xb6, 0x02);
		sccb_write (SLAVE_ID, 0xb9, 0x40);
		sccb_write (SLAVE_ID, 0xba, 0x28);
		sccb_write (SLAVE_ID, 0xbf, 0x0c);
		sccb_write (SLAVE_ID, 0xc0, 0x3e);
		sccb_write (SLAVE_ID, 0xa3, 0x0a);
		sccb_write (SLAVE_ID, 0xa4, 0x0f);
		sccb_write (SLAVE_ID, 0xa5, 0x09);
		sccb_write (SLAVE_ID, 0xa6, 0x16);
		sccb_write (SLAVE_ID, 0x9f, 0x0a);
		sccb_write (SLAVE_ID, 0xa0, 0x0f);
		sccb_write (SLAVE_ID, 0xa7, 0x0a);
		sccb_write (SLAVE_ID, 0xa8, 0x0f);
		sccb_write (SLAVE_ID, 0xa1, 0x10);
		sccb_write (SLAVE_ID, 0xa2, 0x04);
		sccb_write (SLAVE_ID, 0xa9, 0x04);
		sccb_write (SLAVE_ID, 0xaa, 0xa6);
		sccb_write (SLAVE_ID, 0x75, 0x6a);
		sccb_write (SLAVE_ID, 0x76, 0x11);
		sccb_write (SLAVE_ID, 0x77, 0x92);
		sccb_write (SLAVE_ID, 0x78, 0x21);
		sccb_write (SLAVE_ID, 0x79, 0xe1);
		sccb_write (SLAVE_ID, 0x7a, 0x02);
		sccb_write (SLAVE_ID, 0x7c, 0x05);
		sccb_write (SLAVE_ID, 0x7d, 0x08);
		sccb_write (SLAVE_ID, 0x7e, 0x08);
		sccb_write (SLAVE_ID, 0x7f, 0x7c);
		sccb_write (SLAVE_ID, 0x80, 0x58);
		sccb_write (SLAVE_ID, 0x81, 0x2a);
		sccb_write (SLAVE_ID, 0x82, 0xc5);
		sccb_write (SLAVE_ID, 0x83, 0x46);
		sccb_write (SLAVE_ID, 0x84, 0x3a);
		sccb_write (SLAVE_ID, 0x85, 0x54);
		sccb_write (SLAVE_ID, 0x86, 0x44);
		sccb_write (SLAVE_ID, 0x87, 0xf8);
		sccb_write (SLAVE_ID, 0x88, 0x08);
		sccb_write (SLAVE_ID, 0x89, 0x70);
		sccb_write (SLAVE_ID, 0x8a, 0xf0);
		sccb_write (SLAVE_ID, 0x8b, 0xf0);
		sccb_write (SLAVE_ID, 0x0f, 0x24); //15fps
		//driving capability for 30fps
		//sccb_write (SLAVE_ID, 0xc3, 0xdf); 
	}
	else if(nWidthH == 1280 && nWidthV == 720)
	{	//HD720P
		sccb_write (SLAVE_ID, 0xc3, 0x1f);
		sccb_write (SLAVE_ID, 0xc4, 0xff);
		sccb_write (SLAVE_ID, 0x3d, 0x48); 
		sccb_write (SLAVE_ID, 0xdd, 0xa5);
		sccb_write (SLAVE_ID, 0x0e, 0x10);
		sccb_write (SLAVE_ID, 0x10, 0x0a);
		sccb_write (SLAVE_ID, 0x11, 0x00);
		sccb_write (SLAVE_ID, 0x0f, 0x14);
		sccb_write (SLAVE_ID, 0x0e, 0x10);
		
		sccb_write (SLAVE_ID, 0x20, 0x01);
		sccb_write (SLAVE_ID, 0x21, 0x25);
		sccb_write (SLAVE_ID, 0x22, 0x00);
		sccb_write (SLAVE_ID, 0x23, 0x0c);
		sccb_write (SLAVE_ID, 0x24, 0x50);
		sccb_write (SLAVE_ID, 0x26, 0x2d);
		sccb_write (SLAVE_ID, 0x27, 0x04);
		sccb_write (SLAVE_ID, 0x29, 0x06);
		sccb_write (SLAVE_ID, 0x2a, 0x40);
		sccb_write (SLAVE_ID, 0x2b, 0x02);
		sccb_write (SLAVE_ID, 0x2c, 0xee);
		sccb_write (SLAVE_ID, 0x1d, 0x04);
		sccb_write (SLAVE_ID, 0x25, 0x04);
		sccb_write (SLAVE_ID, 0x27, 0x04);
		sccb_write (SLAVE_ID, 0x28, 0x40);
		sccb_write (SLAVE_ID, 0x12, 0x58);	//mirror and flip 0x78
		sccb_write (SLAVE_ID, 0x39, 0x10);
		sccb_write (SLAVE_ID, 0xcd, 0x12);	
		
		sccb_write (SLAVE_ID, 0x13, 0xff);
		sccb_write (SLAVE_ID, 0x14, 0xa7);
		sccb_write (SLAVE_ID, 0x15, 0x42);
		sccb_write (SLAVE_ID, 0x3c, 0xa4);
		sccb_write (SLAVE_ID, 0x18, 0x60);
		sccb_write (SLAVE_ID, 0x19, 0x50);
		sccb_write (SLAVE_ID, 0x1a, 0xe2);
		sccb_write (SLAVE_ID, 0x37, 0xe8);
		sccb_write (SLAVE_ID, 0x16, 0x90);
		sccb_write (SLAVE_ID, 0x43, 0x00);
		sccb_write (SLAVE_ID, 0x40, 0xfb);
		sccb_write (SLAVE_ID, 0xa9, 0x44);
		sccb_write (SLAVE_ID, 0x2f, 0xec);
		sccb_write (SLAVE_ID, 0x35, 0x10);
		sccb_write (SLAVE_ID, 0x36, 0x10);
		sccb_write (SLAVE_ID, 0x0c, 0x00);
		sccb_write (SLAVE_ID, 0x0d, 0x00);
		sccb_write (SLAVE_ID, 0xd0, 0x93);
		sccb_write (SLAVE_ID, 0xdc, 0x2b);
		sccb_write (SLAVE_ID, 0xd9, 0x41);
		sccb_write (SLAVE_ID, 0xd3, 0x02);
		
		sccb_write (SLAVE_ID, 0x3d, 0x08);
		sccb_write (SLAVE_ID, 0x0c, 0x00);
		sccb_write (SLAVE_ID, 0x18, 0x2c);
		sccb_write (SLAVE_ID, 0x19, 0x24);
		sccb_write (SLAVE_ID, 0x1a, 0x71);
		sccb_write (SLAVE_ID, 0x9b, 0x69);
		sccb_write (SLAVE_ID, 0x9c, 0x7d);
		sccb_write (SLAVE_ID, 0x9d, 0x7d);
		sccb_write (SLAVE_ID, 0x9e, 0x69);
		sccb_write (SLAVE_ID, 0x35, 0x04);
		sccb_write (SLAVE_ID, 0x36, 0x04);
		sccb_write (SLAVE_ID, 0x65, 0x12);
		sccb_write (SLAVE_ID, 0x66, 0x20);
		sccb_write (SLAVE_ID, 0x67, 0x39);
		sccb_write (SLAVE_ID, 0x68, 0x4e);
		sccb_write (SLAVE_ID, 0x69, 0x62);
		sccb_write (SLAVE_ID, 0x6a, 0x74);
		sccb_write (SLAVE_ID, 0x6b, 0x85);
		sccb_write (SLAVE_ID, 0x6c, 0x92);
		sccb_write (SLAVE_ID, 0x6d, 0x9e);
		sccb_write (SLAVE_ID, 0x6e, 0xb2);
		sccb_write (SLAVE_ID, 0x6f, 0xc0);
		sccb_write (SLAVE_ID, 0x70, 0xcc);
		sccb_write (SLAVE_ID, 0x71, 0xe0);
		sccb_write (SLAVE_ID, 0x72, 0xee);
		sccb_write (SLAVE_ID, 0x73, 0xf6);
		sccb_write (SLAVE_ID, 0x74, 0x11);
		sccb_write (SLAVE_ID, 0xab, 0x20);
		sccb_write (SLAVE_ID, 0xac, 0x5b);
		sccb_write (SLAVE_ID, 0xad, 0x05);
		sccb_write (SLAVE_ID, 0xae, 0x1b);
		sccb_write (SLAVE_ID, 0xaf, 0x76);
		sccb_write (SLAVE_ID, 0xb0, 0x90);
		sccb_write (SLAVE_ID, 0xb1, 0x90);
		sccb_write (SLAVE_ID, 0xb2, 0x8c);
		sccb_write (SLAVE_ID, 0xb3, 0x04);
		sccb_write (SLAVE_ID, 0xb4, 0x98);
		sccb_write (SLAVE_ID, 0xbC, 0x03);
		sccb_write (SLAVE_ID, 0x4d, 0x30);
		sccb_write (SLAVE_ID, 0x4e, 0x02);
		sccb_write (SLAVE_ID, 0x4f, 0x5c);
		sccb_write (SLAVE_ID, 0x50, 0x56);
		sccb_write (SLAVE_ID, 0x51, 0x00);
		sccb_write (SLAVE_ID, 0x52, 0x66);
		sccb_write (SLAVE_ID, 0x53, 0x03);
		sccb_write (SLAVE_ID, 0x54, 0x30);
		sccb_write (SLAVE_ID, 0x55, 0x02);
		sccb_write (SLAVE_ID, 0x56, 0x5c);
		sccb_write (SLAVE_ID, 0x57, 0x40);
		sccb_write (SLAVE_ID, 0x58, 0x00);
		sccb_write (SLAVE_ID, 0x59, 0x66);
		sccb_write (SLAVE_ID, 0x5a, 0x03);
		sccb_write (SLAVE_ID, 0x5b, 0x20);
		sccb_write (SLAVE_ID, 0x5c, 0x02);
		sccb_write (SLAVE_ID, 0x5d, 0x5c);
		sccb_write (SLAVE_ID, 0x5e, 0x3a);
		sccb_write (SLAVE_ID, 0x5f, 0x00);
		
		sccb_write (SLAVE_ID, 0x60, 0x66);
		sccb_write (SLAVE_ID, 0x41, 0x1f);
		sccb_write (SLAVE_ID, 0xb5, 0x01);
		sccb_write (SLAVE_ID, 0xb6, 0x02);
		sccb_write (SLAVE_ID, 0xb9, 0x40);
		sccb_write (SLAVE_ID, 0xba, 0x28);
		sccb_write (SLAVE_ID, 0xbf, 0x0c);
		sccb_write (SLAVE_ID, 0xc0, 0x3e);
		sccb_write (SLAVE_ID, 0xa3, 0x0a);
		sccb_write (SLAVE_ID, 0xa4, 0x0f);
		sccb_write (SLAVE_ID, 0xa5, 0x09);
		sccb_write (SLAVE_ID, 0xa6, 0x16);
		sccb_write (SLAVE_ID, 0x9f, 0x0a);
		sccb_write (SLAVE_ID, 0xa0, 0x0f);
		sccb_write (SLAVE_ID, 0xa7, 0x0a);
		sccb_write (SLAVE_ID, 0xa8, 0x0f);
		sccb_write (SLAVE_ID, 0xa1, 0x10);
		sccb_write (SLAVE_ID, 0xa2, 0x04);
		sccb_write (SLAVE_ID, 0xa9, 0x04);
		sccb_write (SLAVE_ID, 0xaa, 0xa6);
		sccb_write (SLAVE_ID, 0x75, 0x6a);
		sccb_write (SLAVE_ID, 0x76, 0x11);
		sccb_write (SLAVE_ID, 0x77, 0x92);
		sccb_write (SLAVE_ID, 0x78, 0x21);
		sccb_write (SLAVE_ID, 0x79, 0xe1);
		sccb_write (SLAVE_ID, 0x7a, 0x02);
		sccb_write (SLAVE_ID, 0x7c, 0x05);
		sccb_write (SLAVE_ID, 0x7d, 0x08);
		sccb_write (SLAVE_ID, 0x7e, 0x08);
		sccb_write (SLAVE_ID, 0x7f, 0x7c);
		sccb_write (SLAVE_ID, 0x80, 0x58);
		sccb_write (SLAVE_ID, 0x81, 0x2a);
		sccb_write (SLAVE_ID, 0x82, 0xc5);
		sccb_write (SLAVE_ID, 0x83, 0x46);
		sccb_write (SLAVE_ID, 0x84, 0x3a);
		sccb_write (SLAVE_ID, 0x85, 0x54);
		sccb_write (SLAVE_ID, 0x86, 0x44);
		sccb_write (SLAVE_ID, 0x87, 0xf8);
		sccb_write (SLAVE_ID, 0x88, 0x08);
		sccb_write (SLAVE_ID, 0x89, 0x70);
		sccb_write (SLAVE_ID, 0x8a, 0xf0);
		sccb_write (SLAVE_ID, 0x8b, 0xf0);
		//sccb_write (SLAVE_ID, 0x0f, 0x30); //10fps  0x34
		
	#if INIT_MCLK==96000000	
        sccb_write (SLAVE_ID,0x0E, 0x16); // 1A              // PLL1, [7:6]=bit8div, [5:0]=plldiv 20fps
        sccb_write (SLAVE_ID,0x0F, 0x15); // 15              // PLL2, [7:4]=scalediv, [3:2]=sysdiv, [1:0]=freqdiv 
        sccb_write (SLAVE_ID,0x10, 0x0E); // 0E              // PLL3, [7]=PLL power down, [6]=PLL bypass, [5:3]=PLL charge pump control, [2:0]=indiv 
        sccb_write (SLAVE_ID,0x11, 0x00); // 00             // CLK, [7]=dpllen, [6]=slvpck, [5:0]=div 		
	#elif INIT_MCLK==118000000		
        sccb_write (SLAVE_ID,0x0E, 0x0a);                // PLL1, [7:6]=bit8div, [5:0]=plldiv 20fps
        sccb_write (SLAVE_ID,0x0F, 0x15);                // PLL2, [7:4]=scalediv, [3:2]=sysdiv, [1:0]=freqdiv 
        sccb_write (SLAVE_ID,0x10, 0x0E);                // PLL3, [7]=PLL power down, [6]=PLL bypass, [5:3]=PLL charge pump control, [2:0]=indiv 
        sccb_write (SLAVE_ID,0x11, 0x00);                // CLK, [7]=dpllen, [6]=slvpck, [5:0]=div 		
	    sccb_write (SLAVE_ID,0x2E, 0x08);  
	#endif	
		//driving capability for 30fps
		sccb_write (SLAVE_ID, 0xc3, 0xdf); 
	}
	else if(nWidthH == 1600 && nWidthV == 1200)
	{	//UXGA
		sccb_write (SLAVE_ID, 0xc3, 0x1f);
		sccb_write (SLAVE_ID, 0xc4, 0xff);
		sccb_write (SLAVE_ID, 0x3d, 0x48);
		sccb_write (SLAVE_ID, 0xdd, 0xa5);
		sccb_write (SLAVE_ID, 0x0e, 0x10);
		sccb_write (SLAVE_ID, 0x10, 0x0a);
		sccb_write (SLAVE_ID, 0x11, 0x00);
		sccb_write (SLAVE_ID, 0x0f, 0x14);
		sccb_write (SLAVE_ID, 0x0e, 0x10);
		sccb_write (SLAVE_ID, 0x21, 0x25);
		sccb_write (SLAVE_ID, 0x23, 0x0c);
		sccb_write (SLAVE_ID, 0x12, 0x38);	//mirror and flip
		sccb_write (SLAVE_ID, 0x39, 0x10);
		sccb_write (SLAVE_ID, 0xcd, 0x12);	
		sccb_write (SLAVE_ID, 0x13, 0xff);
		sccb_write (SLAVE_ID, 0x14, 0xa7);
		sccb_write (SLAVE_ID, 0x15, 0x42);
		sccb_write (SLAVE_ID, 0x3c, 0xa4);
		sccb_write (SLAVE_ID, 0x18, 0x60);
		sccb_write (SLAVE_ID, 0x19, 0x50);
		sccb_write (SLAVE_ID, 0x1a, 0xe2);
		sccb_write (SLAVE_ID, 0x37, 0xe8);
		sccb_write (SLAVE_ID, 0x16, 0x90);
		sccb_write (SLAVE_ID, 0x43, 0x00);
		sccb_write (SLAVE_ID, 0x40, 0xfb);
		sccb_write (SLAVE_ID, 0xa9, 0x44);
		sccb_write (SLAVE_ID, 0x2f, 0xec);
		sccb_write (SLAVE_ID, 0x35, 0x10);
		sccb_write (SLAVE_ID, 0x36, 0x10);
		sccb_write (SLAVE_ID, 0x0c, 0x00);
		sccb_write (SLAVE_ID, 0x0d, 0x00);
		sccb_write (SLAVE_ID, 0xd0, 0x93);
		sccb_write (SLAVE_ID, 0xdc, 0x2b);
		sccb_write (SLAVE_ID, 0xd9, 0x41);
		sccb_write (SLAVE_ID, 0xd3, 0x02);
		
		sccb_write (SLAVE_ID, 0x3d, 0x08);
		sccb_write (SLAVE_ID, 0x0c, 0x00);
		sccb_write (SLAVE_ID, 0x18, 0x2c);
		sccb_write (SLAVE_ID, 0x19, 0x24);
		sccb_write (SLAVE_ID, 0x1a, 0x71);
		sccb_write (SLAVE_ID, 0x9b, 0x69);
		sccb_write (SLAVE_ID, 0x9c, 0x7d);
		sccb_write (SLAVE_ID, 0x9d, 0x7d);
		sccb_write (SLAVE_ID, 0x9e, 0x69);
		sccb_write (SLAVE_ID, 0x35, 0x04);
		sccb_write (SLAVE_ID, 0x36, 0x04);
		sccb_write (SLAVE_ID, 0x65, 0x12);
		sccb_write (SLAVE_ID, 0x66, 0x20);
		sccb_write (SLAVE_ID, 0x67, 0x39);
		sccb_write (SLAVE_ID, 0x68, 0x4e);
		sccb_write (SLAVE_ID, 0x69, 0x62);
		sccb_write (SLAVE_ID, 0x6a, 0x74);
		sccb_write (SLAVE_ID, 0x6b, 0x85);
		sccb_write (SLAVE_ID, 0x6c, 0x92);
		sccb_write (SLAVE_ID, 0x6d, 0x9e);
		sccb_write (SLAVE_ID, 0x6e, 0xb2);
		sccb_write (SLAVE_ID, 0x6f, 0xc0);
		sccb_write (SLAVE_ID, 0x70, 0xcc);
		sccb_write (SLAVE_ID, 0x71, 0xe0);
		sccb_write (SLAVE_ID, 0x72, 0xee);
		sccb_write (SLAVE_ID, 0x73, 0xf6);
		sccb_write (SLAVE_ID, 0x74, 0x11);
		sccb_write (SLAVE_ID, 0xab, 0x20);
		sccb_write (SLAVE_ID, 0xac, 0x5b);
		sccb_write (SLAVE_ID, 0xad, 0x05);
		sccb_write (SLAVE_ID, 0xae, 0x1b);
		sccb_write (SLAVE_ID, 0xaf, 0x76);
		sccb_write (SLAVE_ID, 0xb0, 0x90);
		sccb_write (SLAVE_ID, 0xb1, 0x90);
		sccb_write (SLAVE_ID, 0xb2, 0x8c);
		sccb_write (SLAVE_ID, 0xb3, 0x04);
		sccb_write (SLAVE_ID, 0xb4, 0x98);
		sccb_write (SLAVE_ID, 0xbC, 0x03);
		sccb_write (SLAVE_ID, 0x4d, 0x30);
		sccb_write (SLAVE_ID, 0x4e, 0x02);
		sccb_write (SLAVE_ID, 0x4f, 0x5c);
		sccb_write (SLAVE_ID, 0x50, 0x56);
		sccb_write (SLAVE_ID, 0x51, 0x00);
		sccb_write (SLAVE_ID, 0x52, 0x66);
		sccb_write (SLAVE_ID, 0x53, 0x03);
		sccb_write (SLAVE_ID, 0x54, 0x30);
		sccb_write (SLAVE_ID, 0x55, 0x02);
		sccb_write (SLAVE_ID, 0x56, 0x5c);
		sccb_write (SLAVE_ID, 0x57, 0x40);
		sccb_write (SLAVE_ID, 0x58, 0x00);
		sccb_write (SLAVE_ID, 0x59, 0x66);
		sccb_write (SLAVE_ID, 0x5a, 0x03);
		sccb_write (SLAVE_ID, 0x5b, 0x20);
		sccb_write (SLAVE_ID, 0x5c, 0x02);
		sccb_write (SLAVE_ID, 0x5d, 0x5c);
		sccb_write (SLAVE_ID, 0x5e, 0x3a);
		sccb_write (SLAVE_ID, 0x5f, 0x00);
		
		sccb_write (SLAVE_ID, 0x60, 0x66);
		sccb_write (SLAVE_ID, 0x41, 0x1f);
		sccb_write (SLAVE_ID, 0xb5, 0x01);
		sccb_write (SLAVE_ID, 0xb6, 0x02);
		sccb_write (SLAVE_ID, 0xb9, 0x40);
		sccb_write (SLAVE_ID, 0xba, 0x28);
		sccb_write (SLAVE_ID, 0xbf, 0x0c);
		sccb_write (SLAVE_ID, 0xc0, 0x3e);
		sccb_write (SLAVE_ID, 0xa3, 0x0a);
		sccb_write (SLAVE_ID, 0xa4, 0x0f);
		sccb_write (SLAVE_ID, 0xa5, 0x09);
		sccb_write (SLAVE_ID, 0xa6, 0x16);
		sccb_write (SLAVE_ID, 0x9f, 0x0a);
		sccb_write (SLAVE_ID, 0xa0, 0x0f);
		sccb_write (SLAVE_ID, 0xa7, 0x0a);
		sccb_write (SLAVE_ID, 0xa8, 0x0f);
		sccb_write (SLAVE_ID, 0xa1, 0x10);
		sccb_write (SLAVE_ID, 0xa2, 0x04);
		sccb_write (SLAVE_ID, 0xa9, 0x04);
		sccb_write (SLAVE_ID, 0xaa, 0xa6);
		sccb_write (SLAVE_ID, 0x75, 0x6a);
		sccb_write (SLAVE_ID, 0x76, 0x11);
		sccb_write (SLAVE_ID, 0x77, 0x92);
		sccb_write (SLAVE_ID, 0x78, 0x21);
		sccb_write (SLAVE_ID, 0x79, 0xe1);
		sccb_write (SLAVE_ID, 0x7a, 0x02);
		sccb_write (SLAVE_ID, 0x7c, 0x05);
		sccb_write (SLAVE_ID, 0x7d, 0x08);
		sccb_write (SLAVE_ID, 0x7e, 0x08);
		sccb_write (SLAVE_ID, 0x7f, 0x7c);
		sccb_write (SLAVE_ID, 0x80, 0x58);
		sccb_write (SLAVE_ID, 0x81, 0x2a);
		sccb_write (SLAVE_ID, 0x82, 0xc5);
		sccb_write (SLAVE_ID, 0x83, 0x46);
		sccb_write (SLAVE_ID, 0x84, 0x3a);
		sccb_write (SLAVE_ID, 0x85, 0x54);
		sccb_write (SLAVE_ID, 0x86, 0x44);
		sccb_write (SLAVE_ID, 0x87, 0xf8);
		sccb_write (SLAVE_ID, 0x88, 0x08);
		sccb_write (SLAVE_ID, 0x89, 0x70);
		sccb_write (SLAVE_ID, 0x8a, 0xf0);
		sccb_write (SLAVE_ID, 0x8b, 0xf0);
		sccb_write (SLAVE_ID, 0x0f, 0x34); //5fps
		//driving capability for 15fps
		//sccb_write (SLAVE_ID, 0xc3, 0xdf);	
	}
/*			
	R_CSI_TG_CTRL1 = uCtrlReg2;					//P_Sensor_TG_Ctrl2 = uCtrlReg2;
#if CSI_IRQ_MODE == CSI_IRQ_PPU_IRQ	
	R_CSI_TG_CTRL0 = uCtrlReg1;					//P_Sensor_TG_Ctrl1 = uCtrlReg1;
#elif CSI_IRQ_MODE == CSI_IRQ_TG_IRQ
	R_CSI_TG_CTRL0 = uCtrlReg1 | (1 << 16);
#elif CSI_IRQ_MODE == CSI_IRQ_TG_FIFO8_IRQ
	R_CSI_TG_CTRL0 = uCtrlReg1 | (1 << 20) | (1 << 16);
#elif CSI_IRQ_MODE == CSI_IRQ_TG_FIFO16_IRQ
	R_CSI_TG_CTRL0 = uCtrlReg1 | (2 << 20) | (1 << 16);
#elif CSI_IRQ_MODE == CSI_IRQ_TG_FIFO32_IRQ
	R_CSI_TG_CTRL0 = uCtrlReg1 | (3 << 20) | (1 << 16);
#endif	
*/


	R_CSI_TG_CTRL1 = uCtrlReg2;					//*P_Sensor_TG_Ctrl2 = uCtrlReg2;
#if CSI_MODE == CSI_PPU_FRAME_MODE
	R_CSI_TG_CTRL0 = uCtrlReg1;					//*P_Sensor_TG_Ctrl1 = uCtrlReg1;
#elif CSI_MODE == CSI_TG_FRAME_MODE
	R_CSI_TG_CTRL0 = uCtrlReg1|0x010000;
#elif CSI_MODE == CSI_FIFO_8_MODE
	R_CSI_TG_CTRL0 = uCtrlReg1|0x110000;
#elif CSI_MODE == CSI_FIFO_16_MODE
	R_CSI_TG_CTRL0 = uCtrlReg1|0x210000;
#elif CSI_MODE == CSI_FIFO_32_MODE
	R_CSI_TG_CTRL0 = uCtrlReg1|0x310000;
#endif
}
#endif

#ifdef	__OV7725_DRV_C__
//====================================================================================================
//	Description:	OV7725 Initialization
//	Syntax:			void OV7725_Init (
//						INT16S nWidthH,			// Active H Width
//						INT16S nWidthV,			// Active V Width
//						INT16U uFlag				// Flag Type
//					);
//	Return:			None
//====================================================================================================
void OV7725_Init (
	INT16S nWidthH,			// Active H Width
	INT16S nWidthV,			// Active V Width
	INT16U uFlag				// Flag Type
) {
INT32U tmp;
	INT16U uCtrlReg1, uCtrlReg2;
	INT16S nReso;

	// Enable CSI clock to let sensor initialize at first
	#if CSI_CLOCK == CSI_CLOCK_27MHZ
	uCtrlReg2 = CLKOEN | CSI_RGB565 |CLK_SEL27M | CSI_HIGHPRI | CSI_NOSTOP;
	#else
	uCtrlReg2 = CLKOEN | CSI_RGB565 |CLK_SEL48M | CSI_HIGHPRI | CSI_NOSTOP;
	R_SYSTEM_CTRL |= 0x4000;		//24MHz
	#endif

	uCtrlReg1 = CSIEN | YUV_YUYV | CAP; // Default CSI Control Register 1
	if (uFlag & FT_CSI_RGB1555) // RGB1555
	{
	uCtrlReg2 |= CSI_RGB1555;
	}
	if (uFlag & FT_CSI_CCIR656) // CCIR656?
	{
	uCtrlReg1 |= CCIR656 | VADD_FALL | VRST_FALL | HRST_FALL; // CCIR656
	uCtrlReg2 |= D_TYPE1; // CCIR656
	}
	else
	{
	uCtrlReg1 |= VADD_RISE | VRST_FALL | HRST_RISE | HREF; // NOT CCIR656
	uCtrlReg2 |= D_TYPE0; // NOT CCIR656
	}
	if (uFlag & FT_CSI_YUVIN) // YUVIN?
	{
	uCtrlReg1 |= YUVIN;
	}
	if (uFlag & FT_CSI_YUVOUT) // YUVOUT?
	{
	uCtrlReg1 |= YUVOUT;
	}

	// Whether compression or not?
	nReso = ((nWidthH == 320) && (nWidthV == 240)) ? 1 : 0;
	if (nReso == 1) // VGA
	{
	#ifdef __TV_QVGA__
	R_CSI_TG_HRATIO = 0x0102; // Scale to 1/2
	R_CSI_TG_VRATIO = 0x0102; // Scale to 1/2
	R_CSI_TG_HWIDTH = nWidthH; // Horizontal frame width
	R_CSI_TG_VHEIGHT = nWidthV*2; // Vertical frame width
	#endif // __TV_QVGA__
	}
	else
	{
	R_CSI_TG_HRATIO = 0;
	R_CSI_TG_VRATIO = 0;
	}

	R_CSI_TG_VL0START = 0x0000; // Sensor field 0 vertical latch start register.
	R_CSI_TG_VL1START = 0x0000; //*P_Sensor_TG_V_L1Start = 0x0000;
	R_CSI_TG_HSTART = 0x0000; // Sensor horizontal start register.

	R_CSI_TG_CTRL0 = 0; //reset control0
	R_CSI_TG_CTRL1 = CSI_NOSTOP|CLKOEN; //enable CSI CLKO
	drv_msec_wait(100); //wait 100ms for CLKO stable

	// CMOS Sensor Initialization Start...
	tmp = R_SPI0_CTRL;
	R_SPI0_CTRL = 0;

	sccb_init (SCCB_SCL, SCCB_SDA);
	sccb_delay (200);
	sccb_write (SLAVE_ID, 0x12, 0x80);
	sccb_delay (200);

	//AD
	sccb_write(SLAVE_ID, 0x3d, 0x03); //DC offset for analog process

	//ROI : vga
	sccb_write(SLAVE_ID, 0x17, 0x22); //HStart
	sccb_write(SLAVE_ID, 0x18, 0xa4); //HSize
	sccb_write(SLAVE_ID, 0x19, 0x07); //VStart
	sccb_write(SLAVE_ID, 0x1a, 0xf0); //VSize
	sccb_write(SLAVE_ID, 0x32, 0x00); //HREF
	sccb_write(SLAVE_ID, 0x29, 0xa0); //Houtsize 8MSB
	sccb_write(SLAVE_ID, 0x2c, 0xf0); //Voutsize 8MSB
	sccb_write(SLAVE_ID, 0x2a, 0x00); //Houtsize 2MSB,Voutsize 1MSB
	sccb_write(SLAVE_ID, 0x11, 0x01); //internal clock
	sccb_write(SLAVE_ID, 0x33, 0x00);
	sccb_write(SLAVE_ID, 0x34, 0x00);
	sccb_write(SLAVE_ID, 0x15, 0x04);
	sccb_write(SLAVE_ID, 0x0c, 0xe0);

	//DSP control
	sccb_write(SLAVE_ID, 0x42, 0x7f);
	sccb_write(SLAVE_ID, 0x4d, 0x09);
	sccb_write(SLAVE_ID, 0x63, 0xe0);
	sccb_write(SLAVE_ID, 0x64, 0xff);
	sccb_write(SLAVE_ID, 0x65, 0x20); //vertical/horizontal zoom disable
	sccb_write(SLAVE_ID, 0x66, 0x00);
	sccb_write(SLAVE_ID, 0x67, 0x48);

	//FPS adj
	sccb_write(SLAVE_ID, 0x2b, 0x63); //Dummy pixel LSB
	sccb_write(SLAVE_ID, 0x33, 0x00); //Dummy line LSB
	sccb_write(SLAVE_ID, 0x34, 0x00); //Dummy line MSB

	//DE-FLICKER
	sccb_write(SLAVE_ID, 0x22, 0x99); //For 50Hz
	sccb_write(SLAVE_ID, 0x23, 0x02);

	//AGC AEC AWB
	sccb_write(SLAVE_ID, 0x13, 0xe0);
	sccb_write(SLAVE_ID, 0x1f, 0x08); //Fine AEC
	sccb_write(SLAVE_ID, 0x0d, 0x71); //AEC/AGC window
	sccb_write(SLAVE_ID, 0x0f, 0xc5);
	sccb_write(SLAVE_ID, 0x14, 0x17); //AGC 4x & drop frame
	sccb_write(SLAVE_ID, 0x22, 0x98);
	sccb_write(SLAVE_ID, 0x23, 0x02);
	sccb_write(SLAVE_ID, 0x24, 0x2f);
	sccb_write(SLAVE_ID, 0x25, 0x29);
	sccb_write(SLAVE_ID, 0x26, 0x51);
	sccb_write(SLAVE_ID, 0x2b, 0x00); //50Hz
	sccb_write(SLAVE_ID, 0x69, 0x5c);
	sccb_write(SLAVE_ID, 0x6b, 0xa8);
	sccb_write(SLAVE_ID, 0x01, 0x5a);
	sccb_write(SLAVE_ID, 0x02, 0x5c);
	sccb_write(SLAVE_ID, 0x03, 0x40);
	sccb_write(SLAVE_ID, 0x7b, 0x60);
	sccb_write(SLAVE_ID, 0x7c, 0x40);
	sccb_write(SLAVE_ID, 0x7d, 0x70);
	sccb_write(SLAVE_ID, 0x13, 0xef);

	//GAMMA : TEST 3
	sccb_write(SLAVE_ID, 0x7e, 0x0f);
	sccb_write(SLAVE_ID, 0x7f, 0x28);
	sccb_write(SLAVE_ID, 0x80, 0x44);
	sccb_write(SLAVE_ID, 0x81, 0x6c);
	sccb_write(SLAVE_ID, 0x82, 0x78);
	sccb_write(SLAVE_ID, 0x83, 0x86);
	sccb_write(SLAVE_ID, 0x84, 0x91);
	sccb_write(SLAVE_ID, 0x85, 0x9b);
	sccb_write(SLAVE_ID, 0x86, 0xa2);
	sccb_write(SLAVE_ID, 0x87, 0xab);
	sccb_write(SLAVE_ID, 0x88, 0xb8);
	sccb_write(SLAVE_ID, 0x89, 0xc1);
	sccb_write(SLAVE_ID, 0x8A, 0xce);
	sccb_write(SLAVE_ID, 0x8B, 0xd7);
	sccb_write(SLAVE_ID, 0x8C, 0xde);
	sccb_write(SLAVE_ID, 0x8d, 0x0c);

	//C MATRIX : Lancer color 0.82
	sccb_write(SLAVE_ID, 0x94, 0x90);
	sccb_write(SLAVE_ID, 0x95, 0x80);
	sccb_write(SLAVE_ID, 0x96, 0x0f);
	sccb_write(SLAVE_ID, 0x97, 0x12);
	sccb_write(SLAVE_ID, 0x98, 0x64);
	sccb_write(SLAVE_ID, 0x99, 0x76);
	sccb_write(SLAVE_ID, 0x9a, 0x1e);

	//sharpness brightness contrast saturation hue UV
	sccb_write(SLAVE_ID, 0x90, 0x05);
	sccb_write(SLAVE_ID, 0x91, 0x01);
	sccb_write(SLAVE_ID, 0x92, 0x03);
	sccb_write(SLAVE_ID, 0x93, 0x00);
	sccb_write(SLAVE_ID, 0x9b, 0x08);
	sccb_write(SLAVE_ID, 0x9c, 0x20);
	sccb_write(SLAVE_ID, 0x9e, 0x81);
	sccb_write(SLAVE_ID, 0x9f, 0x80);
	sccb_write(SLAVE_ID, 0xa7, 0x40);
	sccb_write(SLAVE_ID, 0xa8, 0x40);
	sccb_write(SLAVE_ID, 0xa6, 0x02);
	sccb_write(SLAVE_ID, 0xac, 0xbf);
	sccb_write(SLAVE_ID, 0x8e, 0x09);
	sccb_write(SLAVE_ID, 0xac, 0xff);
	sccb_write(SLAVE_ID, 0x8f, 0x01);

	//night mode
	sccb_write(SLAVE_ID, 0x0e, 0xd5); 	//night mode on	and max reduction to 1/2 frame rate

	//misc

	R_SPI0_CTRL = tmp;

	R_CSI_TG_CTRL1 = uCtrlReg2; //*P_Sensor_TG_Ctrl2 = uCtrlReg2;
	R_CSI_TG_CTRL1 |= 0x1;
//	#if CSI_MODE == CSI_PPU_FRAME_MODE
//	R_CSI_TG_CTRL0 = uCtrlReg1; //*P_Sensor_TG_Ctrl1 = uCtrlReg1;
//	#elif CSI_MODE == CSI_TG_FRAME_MODE
//	R_CSI_TG_CTRL0 = uCtrlReg1|0x010000;
//	#elif CSI_MODE == CSI_FIFO_8_MODE
//	R_CSI_TG_CTRL0 = uCtrlReg1|0x110000;
//	#elif CSI_MODE == CSI_FIFO_16_MODE
//	R_CSI_TG_CTRL0 = uCtrlReg1|0x210000;
//	#elif CSI_MODE == CSI_FIFO_32_MODE
//	R_CSI_TG_CTRL0 = uCtrlReg1|0x310000;
//	#endif
	if (frame_mode_en == 1) // CSI_MODE == CSI_TG_FRAME_MODE
		R_CSI_TG_CTRL0 = uCtrlReg1|0x010000;
	else	// CSI_MODE == CSI_FIFO_32_MODE
	{
		csi_fifo_flag = 0;
		R_CSI_TG_CTRL0 = uCtrlReg1|0x310000;
	}
}
#endif


#ifdef	__OV2655_DRV_C__
//====================================================================================================
//	Description:	OV2655 Initialization
//	Syntax:			void OV2655_Init (
//						INT16S nWidthH,			// Active H Width
//						INT16S nWidthV,			// Active V Width
//						INT16U uFlag				// Flag Type
//					);
//	Return:			None
//====================================================================================================
void OV2655_Init (
	INT16S nWidthH,			// Active H Width
	INT16S nWidthV,			// Active V Width
	INT16U uFlag				// Flag Type
) {
	INT16U uCtrlReg1, uCtrlReg2;
	INT16S nReso;

	// Enable CSI clock to let sensor initialize at first
//#if CSI_CLOCK == CSI_CLOCK_27MHZ
//	uCtrlReg2 = CLKOEN | CSI_RGB565 |CLK_SEL27M | CSI_HIGHPRI | CSI_NOSTOP;
//#else
	uCtrlReg2 = CLKOEN | CSI_RGB565 |CLK_SEL48M | CSI_HIGHPRI | CSI_NOSTOP;
//#endif

	uCtrlReg1 = CSIEN | YUV_YUYV | CAP;									// Default CSI Control Register 1
	if (uFlag & FT_CSI_RGB1555)											// RGB1555
	{
		uCtrlReg2 |= CSI_RGB1555;
	}
	if (uFlag & FT_CSI_CCIR656)										// CCIR656?
	{
		uCtrlReg1 |= CCIR656 | VADD_FALL | VRST_FALL | HRST_FALL;	// CCIR656
		uCtrlReg2 |= D_TYPE1;										// CCIR656
	}
	else
	{
		uCtrlReg1 |= VADD_RISE | VRST_FALL | HRST_RISE | HREF;		// NOT CCIR656
		uCtrlReg2 |= D_TYPE0;										// NOT CCIR656
	}
	if (uFlag & FT_CSI_YUVIN)										// YUVIN?
	{
		uCtrlReg1 |= YUVIN;
	}
	if (uFlag & FT_CSI_YUVOUT)										// YUVOUT?
	{
		uCtrlReg1 |= YUVOUT;
	}

#if 0
	// Whether compression or not?
	nReso = ((nWidthH == 320) && (nWidthV == 240)) ? 1 : 0;
	if (nReso == 1)								// VGA
	{
#ifdef	__TV_QVGA__
		R_CSI_TG_HRATIO = 0x0101;					// Scale to 1/2
		R_CSI_TG_VRATIO = 0x0101;					// Scale to 1/2
//		R_CSI_TG_HWIDTH = nWidthH;					// Horizontal frame width
//		R_CSI_TG_VHEIGHT = nWidthV*2;				// Vertical frame width
#endif	// __TV_QVGA__
	}
#endif

	R_CSI_TG_VL0START = 0x0000;						// Sensor field 0 vertical latch start register.
	R_CSI_TG_VL1START = 0x0000;						//*P_Sensor_TG_V_L1Start = 0x0000;
	R_CSI_TG_HSTART = 0x0000;						// Sensor horizontal start register.

	R_CSI_TG_CTRL0 = 0;								//reset control0
	R_CSI_TG_CTRL1 = CSI_NOSTOP|CLKOEN;				//enable CSI CLKO
	drv_msec_wait(100); 							//wait 100ms for CLKO stable

	// CMOS Sensor Initialization Start...
	sccb_init (SCCB_SCL, SCCB_SDA);
	sccb_delay (200);
	sccb_write (SLAVE_ID, 0x3012, 0x80);
	sccb_delay (200);

	if ((nWidthH == 320) &&(nWidthV == 240))
	{	//QVGA
		sccb_write(SLAVE_ID, 0x3012, 0x10);
		sccb_write(SLAVE_ID, 0x308c, 0x80);
		sccb_write(SLAVE_ID, 0x308d, 0xe);
		sccb_write(SLAVE_ID, 0x360b, 0x0);
		sccb_write(SLAVE_ID, 0x30b0, 0xff);
		sccb_write(SLAVE_ID, 0x30b1, 0xff);
		sccb_write(SLAVE_ID, 0x30b2, 0x4);
		sccb_write(SLAVE_ID, 0x300e, 0x34);
		sccb_write(SLAVE_ID, 0x300f, 0xa6);
		sccb_write(SLAVE_ID, 0x3010, 0x84);
		sccb_write(SLAVE_ID, 0x3082, 0x1);
		sccb_write(SLAVE_ID, 0x30f4, 0x1);
		sccb_write(SLAVE_ID, 0x3090, 0x3);
		sccb_write(SLAVE_ID, 0x3091, 0xc0);
		sccb_write(SLAVE_ID, 0x30ac, 0x42);
		sccb_write(SLAVE_ID, 0x30d1, 0x8);
		sccb_write(SLAVE_ID, 0x30a8, 0x55);
		sccb_write(SLAVE_ID, 0x3015, 0x2);
		sccb_write(SLAVE_ID, 0x3093, 0x0);
		sccb_write(SLAVE_ID, 0x307e, 0xe5);
		sccb_write(SLAVE_ID, 0x3079, 0x0);
		sccb_write(SLAVE_ID, 0x30aa, 0x32);
		sccb_write(SLAVE_ID, 0x3017, 0x40);
		sccb_write(SLAVE_ID, 0x30f3, 0x83);
		sccb_write(SLAVE_ID, 0x306a, 0xc);
		sccb_write(SLAVE_ID, 0x306d, 0x0);
		sccb_write(SLAVE_ID, 0x336a, 0x3c);
		sccb_write(SLAVE_ID, 0x3076, 0x6a);
		sccb_write(SLAVE_ID, 0x30d9, 0x95);
		sccb_write(SLAVE_ID, 0x3016, 0x82);
		sccb_write(SLAVE_ID, 0x3601, 0x30);
		sccb_write(SLAVE_ID, 0x304e, 0x88);
		sccb_write(SLAVE_ID, 0x30f1, 0x82);
		sccb_write(SLAVE_ID, 0x306f, 0x14);
		sccb_write(SLAVE_ID, 0x302a, 0x2);
		sccb_write(SLAVE_ID, 0x302b, 0x6a);
		sccb_write(SLAVE_ID, 0x3012, 0x10);
		sccb_write(SLAVE_ID, 0x3011, 0x1);
		sccb_write(SLAVE_ID, 0x3013, 0xf7);
		sccb_write(SLAVE_ID, 0x3018, 0x70);
		sccb_write(SLAVE_ID, 0x3019, 0x60);
		sccb_write(SLAVE_ID, 0x301a, 0xd4);
		sccb_write(SLAVE_ID, 0x301c, 0x5);
		sccb_write(SLAVE_ID, 0x301d, 0x6);
		sccb_write(SLAVE_ID, 0x3070, 0x5d);
		sccb_write(SLAVE_ID, 0x3072, 0x4d);
		sccb_write(SLAVE_ID, 0x30af, 0x0);
		sccb_write(SLAVE_ID, 0x3048, 0x1f);
		sccb_write(SLAVE_ID, 0x3049, 0x4e);
		sccb_write(SLAVE_ID, 0x304a, 0x40);
		sccb_write(SLAVE_ID, 0x304f, 0x40);
		sccb_write(SLAVE_ID, 0x304b, 0x2);
		sccb_write(SLAVE_ID, 0x304c, 0x0);
		sccb_write(SLAVE_ID, 0x304d, 0x42);
		sccb_write(SLAVE_ID, 0x304f, 0x40);
		sccb_write(SLAVE_ID, 0x30a3, 0x80);
		sccb_write(SLAVE_ID, 0x3013, 0xf7);
		sccb_write(SLAVE_ID, 0x3014, 0x44);
		sccb_write(SLAVE_ID, 0x3071, 0x0);
		sccb_write(SLAVE_ID, 0x3070, 0x5d);
		sccb_write(SLAVE_ID, 0x3073, 0x0);
		sccb_write(SLAVE_ID, 0x3072, 0x4d);
		sccb_write(SLAVE_ID, 0x301c, 0x5);
		sccb_write(SLAVE_ID, 0x301d, 0x6);
		sccb_write(SLAVE_ID, 0x304d, 0x42);
		sccb_write(SLAVE_ID, 0x304a, 0x40);
		sccb_write(SLAVE_ID, 0x304f, 0x40);
		sccb_write(SLAVE_ID, 0x3095, 0x7);
		sccb_write(SLAVE_ID, 0x3096, 0x16);
		sccb_write(SLAVE_ID, 0x3097, 0x1d);
		sccb_write(SLAVE_ID, 0x3020, 0x1);
		sccb_write(SLAVE_ID, 0x3021, 0x18);
		sccb_write(SLAVE_ID, 0x3022, 0x0);
		sccb_write(SLAVE_ID, 0x3023, 0x6);
		sccb_write(SLAVE_ID, 0x3024, 0x6);
		sccb_write(SLAVE_ID, 0x3025, 0x58);
		sccb_write(SLAVE_ID, 0x3026, 0x2);
		sccb_write(SLAVE_ID, 0x3027, 0x61);
		sccb_write(SLAVE_ID, 0x3088, 0x1);
		sccb_write(SLAVE_ID, 0x3089, 0x40);
		sccb_write(SLAVE_ID, 0x308a, 0x0);
		sccb_write(SLAVE_ID, 0x308b, 0xf0);
		sccb_write(SLAVE_ID, 0x3316, 0x64);
		sccb_write(SLAVE_ID, 0x3317, 0x25);
		sccb_write(SLAVE_ID, 0x3318, 0x80);
		sccb_write(SLAVE_ID, 0x3319, 0x8);
		sccb_write(SLAVE_ID, 0x331a, 0x14);
		sccb_write(SLAVE_ID, 0x331b, 0xf);
		sccb_write(SLAVE_ID, 0x331c, 0x0);
		sccb_write(SLAVE_ID, 0x331d, 0x38);
		sccb_write(SLAVE_ID, 0x3100, 0x0);
		sccb_write(SLAVE_ID, 0x3320, 0xfa);
		sccb_write(SLAVE_ID, 0x3321, 0x11);
		sccb_write(SLAVE_ID, 0x3322, 0x92);
		sccb_write(SLAVE_ID, 0x3323, 0x1);
		sccb_write(SLAVE_ID, 0x3324, 0x97);
		sccb_write(SLAVE_ID, 0x3325, 0x2);
		sccb_write(SLAVE_ID, 0x3326, 0xff);
		sccb_write(SLAVE_ID, 0x3327, 0x10);
		sccb_write(SLAVE_ID, 0x3328, 0x10);
		sccb_write(SLAVE_ID, 0x3329, 0x1f);
		sccb_write(SLAVE_ID, 0x332a, 0x58);
		sccb_write(SLAVE_ID, 0x332b, 0x50);
		sccb_write(SLAVE_ID, 0x332c, 0xbe);
		sccb_write(SLAVE_ID, 0x332d, 0xce);
		sccb_write(SLAVE_ID, 0x332e, 0x2e);
		sccb_write(SLAVE_ID, 0x332f, 0x36);
		sccb_write(SLAVE_ID, 0x3330, 0x4d);
		sccb_write(SLAVE_ID, 0x3331, 0x44);
		sccb_write(SLAVE_ID, 0x3332, 0xf0);
		sccb_write(SLAVE_ID, 0x3333, 0xa);
		sccb_write(SLAVE_ID, 0x3334, 0xf0);
		sccb_write(SLAVE_ID, 0x3335, 0xf0);
		sccb_write(SLAVE_ID, 0x3336, 0xf0);
		sccb_write(SLAVE_ID, 0x3337, 0x40);
		sccb_write(SLAVE_ID, 0x3338, 0x40);
		sccb_write(SLAVE_ID, 0x3339, 0x40);
		sccb_write(SLAVE_ID, 0x333a, 0x0);
		sccb_write(SLAVE_ID, 0x333b, 0x0);
		sccb_write(SLAVE_ID, 0x3380, 0x20);
		sccb_write(SLAVE_ID, 0x3381, 0x5b);
		sccb_write(SLAVE_ID, 0x3382, 0x5);
		sccb_write(SLAVE_ID, 0x3383, 0x22);
		sccb_write(SLAVE_ID, 0x3384, 0x9d);
		sccb_write(SLAVE_ID, 0x3385, 0xc0);
		sccb_write(SLAVE_ID, 0x3386, 0xb6);
		sccb_write(SLAVE_ID, 0x3387, 0xb5);
		sccb_write(SLAVE_ID, 0x3388, 0x2);
		sccb_write(SLAVE_ID, 0x3389, 0x98);
		sccb_write(SLAVE_ID, 0x338a, 0x0);
		sccb_write(SLAVE_ID, 0x3340, 0x9);
		sccb_write(SLAVE_ID, 0x3341, 0x19);
		sccb_write(SLAVE_ID, 0x3342, 0x2f);
		sccb_write(SLAVE_ID, 0x3343, 0x45);
		sccb_write(SLAVE_ID, 0x3344, 0x5a);
		sccb_write(SLAVE_ID, 0x3345, 0x69);
		sccb_write(SLAVE_ID, 0x3346, 0x75);
		sccb_write(SLAVE_ID, 0x3347, 0x7e);
		sccb_write(SLAVE_ID, 0x3348, 0x88);
		sccb_write(SLAVE_ID, 0x3349, 0x96);
		sccb_write(SLAVE_ID, 0x334a, 0xa3);
		sccb_write(SLAVE_ID, 0x334b, 0xaf);
		sccb_write(SLAVE_ID, 0x334c, 0xc4);
		sccb_write(SLAVE_ID, 0x334d, 0xd7);
		sccb_write(SLAVE_ID, 0x334e, 0xe8);
		sccb_write(SLAVE_ID, 0x334f, 0x20);
		sccb_write(SLAVE_ID, 0x3350, 0x32);
		sccb_write(SLAVE_ID, 0x3351, 0x25);
		sccb_write(SLAVE_ID, 0x3352, 0x80);
		sccb_write(SLAVE_ID, 0x3353, 0x1e);
		sccb_write(SLAVE_ID, 0x3354, 0x0);
		sccb_write(SLAVE_ID, 0x3355, 0x84);
		sccb_write(SLAVE_ID, 0x3356, 0x32);
		sccb_write(SLAVE_ID, 0x3357, 0x25);
		sccb_write(SLAVE_ID, 0x3358, 0x80);
		sccb_write(SLAVE_ID, 0x3359, 0x1b);
		sccb_write(SLAVE_ID, 0x335a, 0x0);
		sccb_write(SLAVE_ID, 0x335b, 0x84);
		sccb_write(SLAVE_ID, 0x335c, 0x32);
		sccb_write(SLAVE_ID, 0x335d, 0x25);
		sccb_write(SLAVE_ID, 0x335e, 0x80);
		sccb_write(SLAVE_ID, 0x335f, 0x1b);
		sccb_write(SLAVE_ID, 0x3360, 0x0);
		sccb_write(SLAVE_ID, 0x3361, 0x84);
		sccb_write(SLAVE_ID, 0x3363, 0x70);
		sccb_write(SLAVE_ID, 0x3364, 0x7f);
		sccb_write(SLAVE_ID, 0x3365, 0x0);
		sccb_write(SLAVE_ID, 0x3366, 0x0);
		sccb_write(SLAVE_ID, 0x3301, 0xff);
		sccb_write(SLAVE_ID, 0x338b, 0x1b);
		sccb_write(SLAVE_ID, 0x338c, 0x1f);
		sccb_write(SLAVE_ID, 0x338d, 0x40);
		sccb_write(SLAVE_ID, 0x3370, 0xd0);
		sccb_write(SLAVE_ID, 0x3371, 0x0);
		sccb_write(SLAVE_ID, 0x3372, 0x0);
		sccb_write(SLAVE_ID, 0x3373, 0x40);
		sccb_write(SLAVE_ID, 0x3374, 0x10);
		sccb_write(SLAVE_ID, 0x3375, 0x10);
		sccb_write(SLAVE_ID, 0x3376, 0x4);
		sccb_write(SLAVE_ID, 0x3377, 0x0);
		sccb_write(SLAVE_ID, 0x3378, 0x4);
		sccb_write(SLAVE_ID, 0x3379, 0x80);
		sccb_write(SLAVE_ID, 0x3069, 0x86);
		sccb_write(SLAVE_ID, 0x307c, 0x10);
		sccb_write(SLAVE_ID, 0x3087, 0x2);
		sccb_write(SLAVE_ID, 0x3090, 0x3);
		sccb_write(SLAVE_ID, 0x30aa, 0x32);
		sccb_write(SLAVE_ID, 0x30a3, 0x80);
		sccb_write(SLAVE_ID, 0x30a1, 0x41);
		sccb_write(SLAVE_ID, 0x3300, 0xfc);
		sccb_write(SLAVE_ID, 0x3302, 0x11);
		sccb_write(SLAVE_ID, 0x3400, 0x0);
		sccb_write(SLAVE_ID, 0x3606, 0x20);
		sccb_write(SLAVE_ID, 0x3601, 0x30);
		sccb_write(SLAVE_ID, 0x300e, 0x34);
		sccb_write(SLAVE_ID, 0x30f3, 0x83);
		sccb_write(SLAVE_ID, 0x304e, 0x88);
		sccb_write(SLAVE_ID, 0x363b, 0x1);
		sccb_write(SLAVE_ID, 0x363c, 0xf2);
		sccb_write(SLAVE_ID, 0x3086, 0x0);
		sccb_write(SLAVE_ID, 0x3086, 0x0);
		//30FPS
		sccb_write(SLAVE_ID, 0x3011, 0x0);
		sccb_write(SLAVE_ID, 0x300e, 0x34);
		sccb_write(SLAVE_ID, 0x302c, 0x0);
		sccb_write(SLAVE_ID, 0x3071, 0x0);
		sccb_write(SLAVE_ID, 0x3070, 0xba);
		sccb_write(SLAVE_ID, 0x301c, 0x5);
		sccb_write(SLAVE_ID, 0x3073, 0x0);
		sccb_write(SLAVE_ID, 0x3072, 0x9a);
		sccb_write(SLAVE_ID, 0x301d, 0x7);

	}
	else if((nWidthH == 640) &&(nWidthV == 480))
	{	//VGA
		sccb_write(SLAVE_ID, 0x3012, 0x10);
		sccb_write(SLAVE_ID, 0x308c, 0x80);
		sccb_write(SLAVE_ID, 0x308d, 0xe);
		sccb_write(SLAVE_ID, 0x360b, 0x0);
		sccb_write(SLAVE_ID, 0x30b0, 0xff);
		sccb_write(SLAVE_ID, 0x30b1, 0xff);
		sccb_write(SLAVE_ID, 0x30b2, 0x4);
		sccb_write(SLAVE_ID, 0x300e, 0x34);
		sccb_write(SLAVE_ID, 0x300f, 0xa6);
		sccb_write(SLAVE_ID, 0x3010, 0x82);
		sccb_write(SLAVE_ID, 0x3082, 0x1);
		sccb_write(SLAVE_ID, 0x30f4, 0x1);
		sccb_write(SLAVE_ID, 0x3090, 0x3);
		sccb_write(SLAVE_ID, 0x3091, 0xc0);
		sccb_write(SLAVE_ID, 0x30ac, 0x42);
		sccb_write(SLAVE_ID, 0x30d1, 0x8);
		sccb_write(SLAVE_ID, 0x30a8, 0x55);
		sccb_write(SLAVE_ID, 0x3015, 0x2);
		sccb_write(SLAVE_ID, 0x3093, 0x0);
		sccb_write(SLAVE_ID, 0x307e, 0xe5);
		sccb_write(SLAVE_ID, 0x3079, 0x0);
		sccb_write(SLAVE_ID, 0x30aa, 0x32);
		sccb_write(SLAVE_ID, 0x3017, 0x40);
		sccb_write(SLAVE_ID, 0x30f3, 0x83);
		sccb_write(SLAVE_ID, 0x306a, 0xc);
		sccb_write(SLAVE_ID, 0x306d, 0x0);
		sccb_write(SLAVE_ID, 0x336a, 0x3c);
		sccb_write(SLAVE_ID, 0x3076, 0x6a);
		sccb_write(SLAVE_ID, 0x30d9, 0x95);
		sccb_write(SLAVE_ID, 0x3016, 0x82);
		sccb_write(SLAVE_ID, 0x3601, 0x30);
		sccb_write(SLAVE_ID, 0x304e, 0x88);
		sccb_write(SLAVE_ID, 0x30f1, 0x82);
		sccb_write(SLAVE_ID, 0x306f, 0x14);
		sccb_write(SLAVE_ID, 0x302a, 0x2);
		sccb_write(SLAVE_ID, 0x302b, 0x6a);
		sccb_write(SLAVE_ID, 0x3012, 0x10);
		sccb_write(SLAVE_ID, 0x3011, 0x0);
		sccb_write(SLAVE_ID, 0x3013, 0xf7);
		sccb_write(SLAVE_ID, 0x3018, 0x70);
		sccb_write(SLAVE_ID, 0x3019, 0x60);
		sccb_write(SLAVE_ID, 0x301a, 0xd4);
		sccb_write(SLAVE_ID, 0x301c, 0x5);
		sccb_write(SLAVE_ID, 0x301d, 0x6);
		sccb_write(SLAVE_ID, 0x3070, 0x5d);
		sccb_write(SLAVE_ID, 0x3072, 0x4d);
		sccb_write(SLAVE_ID, 0x30af, 0x0);
		sccb_write(SLAVE_ID, 0x3048, 0x1f);
		sccb_write(SLAVE_ID, 0x3049, 0x4e);
		sccb_write(SLAVE_ID, 0x304a, 0x40);
		sccb_write(SLAVE_ID, 0x304f, 0x40);
		sccb_write(SLAVE_ID, 0x304b, 0x2);
		sccb_write(SLAVE_ID, 0x304c, 0x0);
		sccb_write(SLAVE_ID, 0x304d, 0x42);
		sccb_write(SLAVE_ID, 0x304f, 0x40);
		sccb_write(SLAVE_ID, 0x30a3, 0x80);
		sccb_write(SLAVE_ID, 0x3013, 0xf7);
		sccb_write(SLAVE_ID, 0x3014, 0x44);
		sccb_write(SLAVE_ID, 0x3071, 0x0);
		sccb_write(SLAVE_ID, 0x3070, 0x5d);
		sccb_write(SLAVE_ID, 0x3073, 0x0);
		sccb_write(SLAVE_ID, 0x3072, 0x4d);
		sccb_write(SLAVE_ID, 0x301c, 0x5);
		sccb_write(SLAVE_ID, 0x301d, 0x6);
		sccb_write(SLAVE_ID, 0x304d, 0x42);
		sccb_write(SLAVE_ID, 0x304a, 0x40);
		sccb_write(SLAVE_ID, 0x304f, 0x40);
		sccb_write(SLAVE_ID, 0x3095, 0x7);
		sccb_write(SLAVE_ID, 0x3096, 0x16);
		sccb_write(SLAVE_ID, 0x3097, 0x1d);
		sccb_write(SLAVE_ID, 0x3020, 0x1);
		sccb_write(SLAVE_ID, 0x3021, 0x18);
		sccb_write(SLAVE_ID, 0x3022, 0x0);
		sccb_write(SLAVE_ID, 0x3023, 0x6);
		sccb_write(SLAVE_ID, 0x3024, 0x6);
		sccb_write(SLAVE_ID, 0x3025, 0x58);
		sccb_write(SLAVE_ID, 0x3026, 0x2);
		sccb_write(SLAVE_ID, 0x3027, 0x61);
		sccb_write(SLAVE_ID, 0x3088, 0x2);
		sccb_write(SLAVE_ID, 0x3089, 0x80);
		sccb_write(SLAVE_ID, 0x308a, 0x1);
		sccb_write(SLAVE_ID, 0x308b, 0xe0);
		sccb_write(SLAVE_ID, 0x3316, 0x64);
		sccb_write(SLAVE_ID, 0x3317, 0x25);
		sccb_write(SLAVE_ID, 0x3318, 0x80);
		sccb_write(SLAVE_ID, 0x3319, 0x8);
		sccb_write(SLAVE_ID, 0x331a, 0x28);
		sccb_write(SLAVE_ID, 0x331b, 0x1e);
		sccb_write(SLAVE_ID, 0x331c, 0x0);
		sccb_write(SLAVE_ID, 0x331d, 0x38);
		sccb_write(SLAVE_ID, 0x3100, 0x0);
		sccb_write(SLAVE_ID, 0x3320, 0xfa);
		sccb_write(SLAVE_ID, 0x3321, 0x11);
		sccb_write(SLAVE_ID, 0x3322, 0x92);
		sccb_write(SLAVE_ID, 0x3323, 0x1);
		sccb_write(SLAVE_ID, 0x3324, 0x97);
		sccb_write(SLAVE_ID, 0x3325, 0x2);
		sccb_write(SLAVE_ID, 0x3326, 0xff);
		sccb_write(SLAVE_ID, 0x3327, 0x10);
		sccb_write(SLAVE_ID, 0x3328, 0x10);
		sccb_write(SLAVE_ID, 0x3329, 0x1f);
		sccb_write(SLAVE_ID, 0x332a, 0x58);
		sccb_write(SLAVE_ID, 0x332b, 0x50);
		sccb_write(SLAVE_ID, 0x332c, 0xbe);
		sccb_write(SLAVE_ID, 0x332d, 0xce);
		sccb_write(SLAVE_ID, 0x332e, 0x2e);
		sccb_write(SLAVE_ID, 0x332f, 0x36);
		sccb_write(SLAVE_ID, 0x3330, 0x4d);
		sccb_write(SLAVE_ID, 0x3331, 0x44);
		sccb_write(SLAVE_ID, 0x3332, 0xf0);
		sccb_write(SLAVE_ID, 0x3333, 0xa);
		sccb_write(SLAVE_ID, 0x3334, 0xf0);
		sccb_write(SLAVE_ID, 0x3335, 0xf0);
		sccb_write(SLAVE_ID, 0x3336, 0xf0);
		sccb_write(SLAVE_ID, 0x3337, 0x40);
		sccb_write(SLAVE_ID, 0x3338, 0x40);
		sccb_write(SLAVE_ID, 0x3339, 0x40);
		sccb_write(SLAVE_ID, 0x333a, 0x0);
		sccb_write(SLAVE_ID, 0x333b, 0x0);
		sccb_write(SLAVE_ID, 0x3380, 0x20);
		sccb_write(SLAVE_ID, 0x3381, 0x5b);
		sccb_write(SLAVE_ID, 0x3382, 0x5);
		sccb_write(SLAVE_ID, 0x3383, 0x22);
		sccb_write(SLAVE_ID, 0x3384, 0x9d);
		sccb_write(SLAVE_ID, 0x3385, 0xc0);
		sccb_write(SLAVE_ID, 0x3386, 0xb6);
		sccb_write(SLAVE_ID, 0x3387, 0xb5);
		sccb_write(SLAVE_ID, 0x3388, 0x2);
		sccb_write(SLAVE_ID, 0x3389, 0x98);
		sccb_write(SLAVE_ID, 0x338a, 0x0);
		sccb_write(SLAVE_ID, 0x3340, 0x9);
		sccb_write(SLAVE_ID, 0x3341, 0x19);
		sccb_write(SLAVE_ID, 0x3342, 0x2f);
		sccb_write(SLAVE_ID, 0x3343, 0x45);
		sccb_write(SLAVE_ID, 0x3344, 0x5a);
		sccb_write(SLAVE_ID, 0x3345, 0x69);
		sccb_write(SLAVE_ID, 0x3346, 0x75);
		sccb_write(SLAVE_ID, 0x3347, 0x7e);
		sccb_write(SLAVE_ID, 0x3348, 0x88);
		sccb_write(SLAVE_ID, 0x3349, 0x96);
		sccb_write(SLAVE_ID, 0x334a, 0xa3);
		sccb_write(SLAVE_ID, 0x334b, 0xaf);
		sccb_write(SLAVE_ID, 0x334c, 0xc4);
		sccb_write(SLAVE_ID, 0x334d, 0xd7);
		sccb_write(SLAVE_ID, 0x334e, 0xe8);
		sccb_write(SLAVE_ID, 0x334f, 0x20);
		sccb_write(SLAVE_ID, 0x3350, 0x32);
		sccb_write(SLAVE_ID, 0x3351, 0x25);
		sccb_write(SLAVE_ID, 0x3352, 0x80);
		sccb_write(SLAVE_ID, 0x3353, 0x1e);
		sccb_write(SLAVE_ID, 0x3354, 0x0);
		sccb_write(SLAVE_ID, 0x3355, 0x84);
		sccb_write(SLAVE_ID, 0x3356, 0x32);
		sccb_write(SLAVE_ID, 0x3357, 0x25);
		sccb_write(SLAVE_ID, 0x3358, 0x80);
		sccb_write(SLAVE_ID, 0x3359, 0x1b);
		sccb_write(SLAVE_ID, 0x335a, 0x0);
		sccb_write(SLAVE_ID, 0x335b, 0x84);
		sccb_write(SLAVE_ID, 0x335c, 0x32);
		sccb_write(SLAVE_ID, 0x335d, 0x25);
		sccb_write(SLAVE_ID, 0x335e, 0x80);
		sccb_write(SLAVE_ID, 0x335f, 0x1b);
		sccb_write(SLAVE_ID, 0x3360, 0x0);
		sccb_write(SLAVE_ID, 0x3361, 0x84);
		sccb_write(SLAVE_ID, 0x3363, 0x70);
		sccb_write(SLAVE_ID, 0x3364, 0x7f);
		sccb_write(SLAVE_ID, 0x3365, 0x0);
		sccb_write(SLAVE_ID, 0x3366, 0x0);
		sccb_write(SLAVE_ID, 0x3301, 0xff);
		sccb_write(SLAVE_ID, 0x338b, 0x21);
		sccb_write(SLAVE_ID, 0x338c, 0x1f);
		sccb_write(SLAVE_ID, 0x338d, 0x90);
		sccb_write(SLAVE_ID, 0x3370, 0xd0);
		sccb_write(SLAVE_ID, 0x3371, 0x0);
		sccb_write(SLAVE_ID, 0x3372, 0x0);
		sccb_write(SLAVE_ID, 0x3373, 0x40);
		sccb_write(SLAVE_ID, 0x3374, 0x10);
		sccb_write(SLAVE_ID, 0x3375, 0x10);
		sccb_write(SLAVE_ID, 0x3376, 0x4);
		sccb_write(SLAVE_ID, 0x3377, 0x0);
		sccb_write(SLAVE_ID, 0x3378, 0x4);
		sccb_write(SLAVE_ID, 0x3379, 0x80);
		sccb_write(SLAVE_ID, 0x3069, 0x86);
		sccb_write(SLAVE_ID, 0x307c, 0x10);
		sccb_write(SLAVE_ID, 0x3087, 0x2);
		sccb_write(SLAVE_ID, 0x3090, 0x3);
		sccb_write(SLAVE_ID, 0x30aa, 0x32);
		sccb_write(SLAVE_ID, 0x30a3, 0x80);
		sccb_write(SLAVE_ID, 0x30a1, 0x81);
		sccb_write(SLAVE_ID, 0x3300, 0xfc);
		sccb_write(SLAVE_ID, 0x3302, 0x11);
		sccb_write(SLAVE_ID, 0x3400, 0x0);
		sccb_write(SLAVE_ID, 0x3606, 0x20);
		sccb_write(SLAVE_ID, 0x3601, 0x30);
		sccb_write(SLAVE_ID, 0x300e, 0x34);
		sccb_write(SLAVE_ID, 0x30f3, 0x83);
		sccb_write(SLAVE_ID, 0x304e, 0x88);
		sccb_write(SLAVE_ID, 0x363b, 0x1);
		sccb_write(SLAVE_ID, 0x363c, 0xf2);
		sccb_write(SLAVE_ID, 0x30a1, 0x81);
		sccb_write(SLAVE_ID, 0x338b, 0x21);
		sccb_write(SLAVE_ID, 0x338d, 0x90);
		sccb_write(SLAVE_ID, 0x3086, 0x0);
		sccb_write(SLAVE_ID, 0x3086, 0x0);
		//15FPS
//			sccb_write(SLAVE_ID, 0x3011, 0x1);
		sccb_write(SLAVE_ID, 0x3011, 0x3);// set CLKO=SYS/2 to get 15FPS
		sccb_write(SLAVE_ID, 0x300e, 0x34);
		sccb_write(SLAVE_ID, 0x302c, 0x0);
		sccb_write(SLAVE_ID, 0x3071, 0x0);
		sccb_write(SLAVE_ID, 0x3070, 0x5d);
		sccb_write(SLAVE_ID, 0x301c, 0xc);
		sccb_write(SLAVE_ID, 0x3073, 0x0);
		sccb_write(SLAVE_ID, 0x3072, 0x4d);
		sccb_write(SLAVE_ID, 0x301d, 0xf);
	}
	else
	{	//Capture Test
		//1600X1200 UXGA
		sccb_write(SLAVE_ID,0x3012,0x0);
		sccb_write(SLAVE_ID,0x308c,0x80);
		sccb_write(SLAVE_ID,0x308d,0xe);
		sccb_write(SLAVE_ID,0x360b,0x0);
		sccb_write(SLAVE_ID,0x30b0,0xff);
		sccb_write(SLAVE_ID,0x30b1,0xff);
		sccb_write(SLAVE_ID,0x30b2,0x4);
		sccb_write(SLAVE_ID,0x300e,0x34);
		sccb_write(SLAVE_ID,0x300f,0xa6);
		sccb_write(SLAVE_ID,0x3010,0x81);
		sccb_write(SLAVE_ID,0x3082,0x1);
		sccb_write(SLAVE_ID,0x30f4,0x1);
		sccb_write(SLAVE_ID,0x3090,0x3);
		sccb_write(SLAVE_ID,0x3091,0xc0);
		sccb_write(SLAVE_ID,0x30ac,0x42);
		sccb_write(SLAVE_ID,0x30d1,0x8);
		sccb_write(SLAVE_ID,0x30a8,0x55);
		sccb_write(SLAVE_ID,0x3015,0x2);
		sccb_write(SLAVE_ID,0x3093,0x0);
		sccb_write(SLAVE_ID,0x307e,0xe5);
		sccb_write(SLAVE_ID,0x3079,0x0);
		sccb_write(SLAVE_ID,0x30aa,0x32);
		sccb_write(SLAVE_ID,0x3017,0x40);
		sccb_write(SLAVE_ID,0x30f3,0x83);
		sccb_write(SLAVE_ID,0x306a,0xc);
		sccb_write(SLAVE_ID,0x306d,0x0);
		sccb_write(SLAVE_ID,0x336a,0x3c);
		sccb_write(SLAVE_ID,0x3076,0x6a);
		sccb_write(SLAVE_ID,0x30d9,0x95);
		sccb_write(SLAVE_ID,0x3016,0x82);
		sccb_write(SLAVE_ID,0x3601,0x30);
		sccb_write(SLAVE_ID,0x304e,0x88);
		sccb_write(SLAVE_ID,0x30f1,0x82);
		sccb_write(SLAVE_ID,0x3011,0x2);
		sccb_write(SLAVE_ID,0x3013,0xf7);
		sccb_write(SLAVE_ID,0x3018,0x70);
		sccb_write(SLAVE_ID,0x3019,0x60);
		sccb_write(SLAVE_ID,0x301a,0xd4);
		sccb_write(SLAVE_ID,0x301c,0x12);
		sccb_write(SLAVE_ID,0x301d,0x16);
		sccb_write(SLAVE_ID,0x3070,0x3e);
		sccb_write(SLAVE_ID,0x3072,0x34);
		sccb_write(SLAVE_ID,0x30af,0x0);
		sccb_write(SLAVE_ID,0x3048,0x1f);
		sccb_write(SLAVE_ID,0x3049,0x4e);
		sccb_write(SLAVE_ID,0x304a,0x40);
		sccb_write(SLAVE_ID,0x304f,0x40);
		sccb_write(SLAVE_ID,0x304b,0x2);
		sccb_write(SLAVE_ID,0x304c,0x0);
		sccb_write(SLAVE_ID,0x304d,0x42);
		sccb_write(SLAVE_ID,0x304f,0x40);
		sccb_write(SLAVE_ID,0x30a3,0x80);
		sccb_write(SLAVE_ID,0x3013,0xf7);
		sccb_write(SLAVE_ID,0x3014,0x44);
		sccb_write(SLAVE_ID,0x3071,0x0);
		sccb_write(SLAVE_ID,0x3070,0x3e);
		sccb_write(SLAVE_ID,0x3073,0x0);
		sccb_write(SLAVE_ID,0x3072,0x34);
		sccb_write(SLAVE_ID,0x301c,0x12);
		sccb_write(SLAVE_ID,0x301d,0x16);
		sccb_write(SLAVE_ID,0x304d,0x42);
		sccb_write(SLAVE_ID,0x304a,0x40);
		sccb_write(SLAVE_ID,0x304f,0x40);
		sccb_write(SLAVE_ID,0x3095,0x7);
		sccb_write(SLAVE_ID,0x3096,0x16);
		sccb_write(SLAVE_ID,0x3097,0x1d);
		sccb_write(SLAVE_ID,0x3020,0x1);
		sccb_write(SLAVE_ID,0x3021,0x18);
		sccb_write(SLAVE_ID,0x3022,0x0);
		sccb_write(SLAVE_ID,0x3023,0xc);
		sccb_write(SLAVE_ID,0x3024,0x6);
		sccb_write(SLAVE_ID,0x3025,0x58);
		sccb_write(SLAVE_ID,0x3026,0x4);
		sccb_write(SLAVE_ID,0x3027,0xbc);
		sccb_write(SLAVE_ID,0x3088,0x6);
		sccb_write(SLAVE_ID,0x3089,0x40);
		sccb_write(SLAVE_ID,0x308a,0x4);
		sccb_write(SLAVE_ID,0x308b,0xb0);
		sccb_write(SLAVE_ID,0x3316,0x64);
		sccb_write(SLAVE_ID,0x3317,0x4b);
		sccb_write(SLAVE_ID,0x3318,0x0);
		sccb_write(SLAVE_ID,0x331a,0x64);
		sccb_write(SLAVE_ID,0x331b,0x4b);
		sccb_write(SLAVE_ID,0x331c,0x0);
		sccb_write(SLAVE_ID,0x3100,0x0);
		sccb_write(SLAVE_ID,0x3320,0xfa);
		sccb_write(SLAVE_ID,0x3321,0x11);
		sccb_write(SLAVE_ID,0x3322,0x92);
		sccb_write(SLAVE_ID,0x3323,0x1);
		sccb_write(SLAVE_ID,0x3324,0x97);
		sccb_write(SLAVE_ID,0x3325,0x2);
		sccb_write(SLAVE_ID,0x3326,0xff);
		sccb_write(SLAVE_ID,0x3327,0x10);
		sccb_write(SLAVE_ID,0x3328,0x10);
		sccb_write(SLAVE_ID,0x3329,0x1f);
		sccb_write(SLAVE_ID,0x332a,0x58);
		sccb_write(SLAVE_ID,0x332b,0x50);
		sccb_write(SLAVE_ID,0x332c,0xbe);
		sccb_write(SLAVE_ID,0x332d,0xce);
		sccb_write(SLAVE_ID,0x332e,0x2e);
		sccb_write(SLAVE_ID,0x332f,0x36);
		sccb_write(SLAVE_ID,0x3330,0x4d);
		sccb_write(SLAVE_ID,0x3331,0x44);
		sccb_write(SLAVE_ID,0x3332,0xf0);
		sccb_write(SLAVE_ID,0x3333,0xa);
		sccb_write(SLAVE_ID,0x3334,0xf0);
		sccb_write(SLAVE_ID,0x3335,0xf0);
		sccb_write(SLAVE_ID,0x3336,0xf0);
		sccb_write(SLAVE_ID,0x3337,0x40);
		sccb_write(SLAVE_ID,0x3338,0x40);
		sccb_write(SLAVE_ID,0x3339,0x40);
		sccb_write(SLAVE_ID,0x333a,0x0);
		sccb_write(SLAVE_ID,0x333b,0x0);
		sccb_write(SLAVE_ID,0x3380,0x20);
		sccb_write(SLAVE_ID,0x3381,0x5b);
		sccb_write(SLAVE_ID,0x3382,0x5);
		sccb_write(SLAVE_ID,0x3383,0x22);
		sccb_write(SLAVE_ID,0x3384,0x9d);
		sccb_write(SLAVE_ID,0x3385,0xc0);
		sccb_write(SLAVE_ID,0x3386,0xb6);
		sccb_write(SLAVE_ID,0x3387,0xb5);
		sccb_write(SLAVE_ID,0x3388,0x2);
		sccb_write(SLAVE_ID,0x3389,0x98);
		sccb_write(SLAVE_ID,0x338a,0x0);
		sccb_write(SLAVE_ID,0x3340,0x9);
		sccb_write(SLAVE_ID,0x3341,0x19);
		sccb_write(SLAVE_ID,0x3342,0x2f);
		sccb_write(SLAVE_ID,0x3343,0x45);
		sccb_write(SLAVE_ID,0x3344,0x5a);
		sccb_write(SLAVE_ID,0x3345,0x69);
		sccb_write(SLAVE_ID,0x3346,0x75);
		sccb_write(SLAVE_ID,0x3347,0x7e);
		sccb_write(SLAVE_ID,0x3348,0x88);
		sccb_write(SLAVE_ID,0x3349,0x96);
		sccb_write(SLAVE_ID,0x334a,0xa3);
		sccb_write(SLAVE_ID,0x334b,0xaf);
		sccb_write(SLAVE_ID,0x334c,0xc4);
		sccb_write(SLAVE_ID,0x334d,0xd7);
		sccb_write(SLAVE_ID,0x334e,0xe8);
		sccb_write(SLAVE_ID,0x334f,0x20);
		sccb_write(SLAVE_ID,0x3350,0x32);
		sccb_write(SLAVE_ID,0x3351,0x25);
		sccb_write(SLAVE_ID,0x3352,0x80);
		sccb_write(SLAVE_ID,0x3353,0x1e);
		sccb_write(SLAVE_ID,0x3354,0x0);
		sccb_write(SLAVE_ID,0x3355,0x84);
		sccb_write(SLAVE_ID,0x3356,0x32);
		sccb_write(SLAVE_ID,0x3357,0x25);
		sccb_write(SLAVE_ID,0x3358,0x80);
		sccb_write(SLAVE_ID,0x3359,0x1b);
		sccb_write(SLAVE_ID,0x335a,0x0);
		sccb_write(SLAVE_ID,0x335b,0x84);
		sccb_write(SLAVE_ID,0x335c,0x32);
		sccb_write(SLAVE_ID,0x335d,0x25);
		sccb_write(SLAVE_ID,0x335e,0x80);
		sccb_write(SLAVE_ID,0x335f,0x1b);
		sccb_write(SLAVE_ID,0x3360,0x0);
		sccb_write(SLAVE_ID,0x3361,0x84);
		sccb_write(SLAVE_ID,0x3363,0x70);
		sccb_write(SLAVE_ID,0x3364,0x7f);
		sccb_write(SLAVE_ID,0x3365,0x0);
		sccb_write(SLAVE_ID,0x3366,0x0);
		sccb_write(SLAVE_ID,0x3301,0xff);
		sccb_write(SLAVE_ID,0x338b,0x1b);
		sccb_write(SLAVE_ID,0x338c,0x1f);
		sccb_write(SLAVE_ID,0x338d,0x40);
		sccb_write(SLAVE_ID,0x3370,0xd0);
		sccb_write(SLAVE_ID,0x3371,0x0);
		sccb_write(SLAVE_ID,0x3372,0x0);
		sccb_write(SLAVE_ID,0x3373,0x40);
		sccb_write(SLAVE_ID,0x3374,0x10);
		sccb_write(SLAVE_ID,0x3375,0x10);
		sccb_write(SLAVE_ID,0x3376,0x4);
		sccb_write(SLAVE_ID,0x3377,0x0);
		sccb_write(SLAVE_ID,0x3378,0x4);
		sccb_write(SLAVE_ID,0x3379,0x80);
		sccb_write(SLAVE_ID,0x3069,0x86);
		sccb_write(SLAVE_ID,0x307c,0x10);
		sccb_write(SLAVE_ID,0x3087,0x2);
		sccb_write(SLAVE_ID,0x3090,0x3);
		sccb_write(SLAVE_ID,0x30aa,0x32);
		sccb_write(SLAVE_ID,0x30a3,0x80);
		sccb_write(SLAVE_ID,0x30a1,0x41);
		sccb_write(SLAVE_ID,0x3300,0xfc);
		sccb_write(SLAVE_ID,0x3302,0x1);
		sccb_write(SLAVE_ID,0x3400,0x0);
		sccb_write(SLAVE_ID,0x3606,0x20);
		sccb_write(SLAVE_ID,0x3601,0x30);
		sccb_write(SLAVE_ID,0x300e,0x34);
		sccb_write(SLAVE_ID,0x30f3,0x83);
		sccb_write(SLAVE_ID,0x304e,0x88);
		sccb_write(SLAVE_ID,0x363b,0x1);
		sccb_write(SLAVE_ID,0x363c,0xf2);
		sccb_write(SLAVE_ID,0x3023,0xc);
		sccb_write(SLAVE_ID,0x3319,0x4c);
		sccb_write(SLAVE_ID,0x3086,0x0);
		sccb_write(SLAVE_ID,0x3086,0x0);
//12.5FPS
		sccb_write(SLAVE_ID,0x3011,0x01);
		sccb_write(SLAVE_ID,0x300e,0x36);
		sccb_write(SLAVE_ID,0x302c,0x04);
		sccb_write(SLAVE_ID,0x3071,0x0);
		sccb_write(SLAVE_ID,0x3070,0x98);
		sccb_write(SLAVE_ID,0x301c,0x07);
		sccb_write(SLAVE_ID,0x3073,0x0);
		sccb_write(SLAVE_ID,0x3072,0x81);
		sccb_write(SLAVE_ID,0x301d,0x08);
//		sccb_write(SLAVE_ID,0x300f,0x08); 	//bypass OV2655 internal PLL
	}
		//V Flip
		sccb_write(SLAVE_ID, 0x307c, 0x01);
		//Mirror
		sccb_write(SLAVE_ID, 0x3090, 0x08);

		R_CSI_TG_CTRL1 = uCtrlReg2;					//*P_Sensor_TG_Ctrl2 = uCtrlReg2;
		R_CSI_TG_CTRL0 = uCtrlReg1;					//*P_Sensor_TG_Ctrl1 = uCtrlReg1;
	}
#endif
#ifdef	__OV3640_DRV_C__
//====================================================================================================
//	Description:	OV3640 Initialization
//	Syntax:			void OV3640_Init (
//						INT16S nWidthH,			// Active H Width
//						INT16S nWidthV,			// Active V Width
//						INT16U uFlag				// Flag Type
//					);
//	Return:			None
//====================================================================================================
void OV3640_Init (
	INT16S nWidthH,			// Active H Width
	INT16S nWidthV,			// Active V Width
	INT16U uFlag				// Flag Type
) {
	INT16U uCtrlReg1, uCtrlReg2;
	INT16S nReso;

	// Enable CSI clock to let sensor initialize at first
//#if CSI_CLOCK == CSI_CLOCK_27MHZ
//	uCtrlReg2 = CLKOEN | CSI_RGB565 |CLK_SEL27M | CSI_HIGHPRI | CSI_NOSTOP;
//#else
	uCtrlReg2 = CLKOEN | CSI_RGB565 |CLK_SEL48M | CSI_HIGHPRI | CSI_NOSTOP;
//#endif

	uCtrlReg1 = CSIEN | YUV_YUYV | CAP;									// Default CSI Control Register 1
	if (uFlag & FT_CSI_RGB1555)											// RGB1555
	{
		uCtrlReg2 |= CSI_RGB1555;
	}
	if (uFlag & FT_CSI_CCIR656)										// CCIR656?
	{
		uCtrlReg1 |= CCIR656 | VADD_FALL | VRST_FALL | HRST_FALL;	// CCIR656
		uCtrlReg2 |= D_TYPE1;										// CCIR656
	}
	else
	{
		uCtrlReg1 |= VADD_RISE | VRST_FALL | HRST_RISE | HREF;		// NOT CCIR656
		uCtrlReg2 |= D_TYPE0;										// NOT CCIR656
	}
	if (uFlag & FT_CSI_YUVIN)										// YUVIN?
	{
		uCtrlReg1 |= YUVIN;
	}
	if (uFlag & FT_CSI_YUVOUT)										// YUVOUT?
	{
		uCtrlReg1 |= YUVOUT;
	}

#if 0
	// Whether compression or not?
	nReso = ((nWidthH == 320) && (nWidthV == 240)) ? 1 : 0;
	if (nReso == 1)								// VGA
	{
#ifdef	__TV_QVGA__
		R_CSI_TG_HRATIO =0x0101;					// Scale to 1/2
		R_CSI_TG_VRATIO =0x0101;					// Scale to 1/2
//		R_CSI_TG_HWIDTH = nWidthH;					// Horizontal frame width
//		R_CSI_TG_VHEIGHT = nWidthV*2;				// Vertical frame width
#endif	// __TV_QVGA__
	}
#endif

	R_CSI_TG_VL0START = 0x0000;						// Sensor field 0 vertical latch start register.
	R_CSI_TG_VL1START = 0x0000;						//*P_Sensor_TG_V_L1Start = 0x0000;
	R_CSI_TG_HSTART = 0x0000;						// Sensor horizontal start register.

	R_CSI_TG_CTRL0 = 0;								//reset control0
	R_CSI_TG_CTRL1 = CSI_NOSTOP|CLKOEN;				//enable CSI CLKO
	drv_msec_wait(100); 							//wait 100ms for CLKO stable

	// CMOS Sensor Initialization Start...
	sccb_init (SCCB_SCL, SCCB_SDA);
	sccb_delay (200);
	sccb_write (SLAVE_ID, 0x3012, 0x80);
	sccb_delay (200);

	// 3640 QVGA YUV
	if((nWidthH == 320) &&(nWidthV == 240))
	{	//QVGA
		sccb_write(SLAVE_ID,0x3012,0x80);
		sccb_write(SLAVE_ID,0x304d,0x45);
		sccb_write(SLAVE_ID,0x30a7,0x5e);
		sccb_write(SLAVE_ID,0x3087,0x16);
		sccb_write(SLAVE_ID,0x309C,0x1a);
		sccb_write(SLAVE_ID,0x30a2,0xe4);
		sccb_write(SLAVE_ID,0x30aa,0x42);
		sccb_write(SLAVE_ID,0x30b0,0xff);
		sccb_write(SLAVE_ID,0x30b1,0xff);
		sccb_write(SLAVE_ID,0x30b2,0x10);
		//24Mhz
		sccb_write(SLAVE_ID,0x300e,0x32);
		sccb_write(SLAVE_ID,0x300f,0x21);
		sccb_write(SLAVE_ID,0x3010,0x20);
		sccb_write(SLAVE_ID,0x3011,0x00);
		sccb_write(SLAVE_ID,0x304c,0x84);
		sccb_write(SLAVE_ID,0x30d7,0x10);
		//
		sccb_write(SLAVE_ID,0x30d9,0x0d);
		sccb_write(SLAVE_ID,0x30db,0x08);
		sccb_write(SLAVE_ID,0x3016,0x82);

		//aec/agc,0xauto,0xsetting
		sccb_write(SLAVE_ID,0x3018,0x38);
		sccb_write(SLAVE_ID,0x3019,0x30);
		sccb_write(SLAVE_ID,0x301a,0x61);
		sccb_write(SLAVE_ID,0x307d,0x00);
		sccb_write(SLAVE_ID,0x3087,0x02);
		sccb_write(SLAVE_ID,0x3082,0x20);

		sccb_write(SLAVE_ID,0x3015,0x12);
		sccb_write(SLAVE_ID,0x3014,0x04);
		sccb_write(SLAVE_ID,0x3013,0xf7);

		//aecweight;06142007
		sccb_write(SLAVE_ID,0x303c,0x08);
		sccb_write(SLAVE_ID,0x303d,0x18);
		sccb_write(SLAVE_ID,0x303e,0x06);
		sccb_write(SLAVE_ID,0x303F,0x0c);
		sccb_write(SLAVE_ID,0x3030,0x62);
		sccb_write(SLAVE_ID,0x3031,0x26);
		sccb_write(SLAVE_ID,0x3032,0xe6);
		sccb_write(SLAVE_ID,0x3033,0x6e);
		sccb_write(SLAVE_ID,0x3034,0xea);
		sccb_write(SLAVE_ID,0x3035,0xae);
		sccb_write(SLAVE_ID,0x3036,0xa6);
		sccb_write(SLAVE_ID,0x3037,0x6a);

		//ISP,0xCommon,0x
		sccb_write(SLAVE_ID,0x3104,0x02);
		sccb_write(SLAVE_ID,0x3105,0xfd);
		sccb_write(SLAVE_ID,0x3106,0x00);
		sccb_write(SLAVE_ID,0x3107,0xff);

		sccb_write(SLAVE_ID,0x3300,0x12);
		sccb_write(SLAVE_ID,0x3301,0xde);

		//ISP,0xsetting
		sccb_write(SLAVE_ID,0x3302,0xcf);

		//AWB
		sccb_write(SLAVE_ID,0x3312,0x26);
		sccb_write(SLAVE_ID,0x3314,0x42);
		sccb_write(SLAVE_ID,0x3313,0x2b);
		sccb_write(SLAVE_ID,0x3315,0x42);
		sccb_write(SLAVE_ID,0x3310,0xd0);
		sccb_write(SLAVE_ID,0x3311,0xbd);
		sccb_write(SLAVE_ID,0x330c,0x18);
		sccb_write(SLAVE_ID,0x330d,0x18);
		sccb_write(SLAVE_ID,0x330e,0x56);
		sccb_write(SLAVE_ID,0x330f,0x5c);
		sccb_write(SLAVE_ID,0x330b,0x1c);
		sccb_write(SLAVE_ID,0x3306,0x5c);
		sccb_write(SLAVE_ID,0x3307,0x11);

		//Lens,0xcorrection
		sccb_write(SLAVE_ID,0x336a,0x52);
		sccb_write(SLAVE_ID,0x3370,0x46);
		sccb_write(SLAVE_ID,0x3376,0x38);
		sccb_write(SLAVE_ID,0x3300,0x13);

		//UV,0xadjust,0x
		sccb_write(SLAVE_ID,0x30b8,0x20);
		sccb_write(SLAVE_ID,0x30b9,0x17);
		sccb_write(SLAVE_ID,0x30ba,0x04);
		sccb_write(SLAVE_ID,0x30bb,0x08);

		//Compression
		sccb_write(SLAVE_ID,0x3507,0x06);
		sccb_write(SLAVE_ID,0x350a,0x4f);

		//Output,0xformat
		sccb_write(SLAVE_ID,0x3100,0x32);
		sccb_write(SLAVE_ID,0x3304,0x00);
		sccb_write(SLAVE_ID,0x3400,0x02);
		sccb_write(SLAVE_ID,0x3404,0x22);
		sccb_write(SLAVE_ID,0x3500,0x00);
		sccb_write(SLAVE_ID,0x3600,0xc0);
		sccb_write(SLAVE_ID,0x3610,0x60);
		sccb_write(SLAVE_ID,0x350a,0x4f);

		//DVP,0xQXGA
		sccb_write(SLAVE_ID,0x3088,0x08);
		sccb_write(SLAVE_ID,0x3089,0x00);
		sccb_write(SLAVE_ID,0x308a,0x06);
		sccb_write(SLAVE_ID,0x308b,0x00);

		sccb_write(SLAVE_ID,0x3302,0xef);
		sccb_write(SLAVE_ID,0x335f,0x68);
		sccb_write(SLAVE_ID,0x3360,0x18);
		sccb_write(SLAVE_ID,0x3361,0x0c);
		sccb_write(SLAVE_ID,0x3362,0x01);
		sccb_write(SLAVE_ID,0x3363,0x48);
		sccb_write(SLAVE_ID,0x3364,0xf4);
		sccb_write(SLAVE_ID,0x3403,0x42);
		sccb_write(SLAVE_ID,0x3088,0x01);
		sccb_write(SLAVE_ID,0x3089,0x40);
		sccb_write(SLAVE_ID,0x308a,0x00);
		sccb_write(SLAVE_ID,0x308b,0xf0);

		sccb_write(SLAVE_ID,0x3100,0x02);
		sccb_write(SLAVE_ID,0x3304,0x00);
		sccb_write(SLAVE_ID,0x3400,0x00);
		sccb_write(SLAVE_ID,0x3404,0x00);
		sccb_write(SLAVE_ID,0x3600,0xc0);

		sccb_write(SLAVE_ID,0x308d,0x04);
		sccb_write(SLAVE_ID,0x3086,0x03);
		sccb_write(SLAVE_ID,0x3086,0x00);
	}

	if((nWidthH == 640) &&(nWidthV == 480))
	{
		//VGA
		sccb_write(SLAVE_ID,0x3012, 0x0);
		sccb_write(SLAVE_ID,0x304d, 0x45);
		sccb_write(SLAVE_ID,0x30a7, 0x5e);
		sccb_write(SLAVE_ID,0x3087, 0x2);
		sccb_write(SLAVE_ID,0x309c, 0x1a);
		sccb_write(SLAVE_ID,0x30a2, 0xe4);
		sccb_write(SLAVE_ID,0x30aa, 0x42);
		sccb_write(SLAVE_ID,0x30b0, 0xff);
		sccb_write(SLAVE_ID,0x30b1, 0xff);
		sccb_write(SLAVE_ID,0x30b2, 0x10);
		sccb_write(SLAVE_ID,0x300e, 0x32);
		sccb_write(SLAVE_ID,0x300f, 0x21);
		sccb_write(SLAVE_ID,0x3010, 0x20);
		sccb_write(SLAVE_ID,0x3011, 0x0);
		sccb_write(SLAVE_ID,0x304c, 0x84);
		sccb_write(SLAVE_ID,0x30d7, 0x10);
		sccb_write(SLAVE_ID,0x30d9, 0xd);
		sccb_write(SLAVE_ID,0x30db, 0x8);
		sccb_write(SLAVE_ID,0x3016, 0x82);
		sccb_write(SLAVE_ID,0x3018, 0x38);
		sccb_write(SLAVE_ID,0x3019, 0x30);
		sccb_write(SLAVE_ID,0x301a, 0x61);
		sccb_write(SLAVE_ID,0x307d, 0x0);
		sccb_write(SLAVE_ID,0x3087, 0x2);
		sccb_write(SLAVE_ID,0x3082, 0x20);
		sccb_write(SLAVE_ID,0x3015, 0x12);
		sccb_write(SLAVE_ID,0x3014, 0xc);
		sccb_write(SLAVE_ID,0x3013, 0xf7);
		sccb_write(SLAVE_ID,0x303c, 0x8);
		sccb_write(SLAVE_ID,0x303d, 0x18);
		sccb_write(SLAVE_ID,0x303e, 0x6);
		sccb_write(SLAVE_ID,0x303f, 0xc);
		sccb_write(SLAVE_ID,0x3030, 0x62);
		sccb_write(SLAVE_ID,0x3031, 0x26);
		sccb_write(SLAVE_ID,0x3032, 0xe6);
		sccb_write(SLAVE_ID,0x3033, 0x6e);
		sccb_write(SLAVE_ID,0x3034, 0xea);
		sccb_write(SLAVE_ID,0x3035, 0xae);
		sccb_write(SLAVE_ID,0x3036, 0xa6);
		sccb_write(SLAVE_ID,0x3037, 0x6a);
		sccb_write(SLAVE_ID,0x3104, 0x2);
		sccb_write(SLAVE_ID,0x3105, 0xfd);
		sccb_write(SLAVE_ID,0x3106, 0x0);
		sccb_write(SLAVE_ID,0x3107, 0xff);
		sccb_write(SLAVE_ID,0x3300, 0x13);
		sccb_write(SLAVE_ID,0x3301, 0xde);
		sccb_write(SLAVE_ID,0x3302, 0xef);
		sccb_write(SLAVE_ID,0x3312, 0x26);
		sccb_write(SLAVE_ID,0x3314, 0x42);
		sccb_write(SLAVE_ID,0x3313, 0x2b);
		sccb_write(SLAVE_ID,0x3315, 0x42);
		sccb_write(SLAVE_ID,0x3310, 0xd0);
		sccb_write(SLAVE_ID,0x3311, 0xbd);
		sccb_write(SLAVE_ID,0x330c, 0x18);
		sccb_write(SLAVE_ID,0x330d, 0x18);
		sccb_write(SLAVE_ID,0x330e, 0x56);
		sccb_write(SLAVE_ID,0x330f, 0x5c);
		sccb_write(SLAVE_ID,0x330b, 0x1c);
		sccb_write(SLAVE_ID,0x3306, 0x5c);
		sccb_write(SLAVE_ID,0x3307, 0x11);
		sccb_write(SLAVE_ID,0x336a, 0x52);
		sccb_write(SLAVE_ID,0x3370, 0x46);
		sccb_write(SLAVE_ID,0x3376, 0x38);
		sccb_write(SLAVE_ID,0x3300, 0x13);
		sccb_write(SLAVE_ID,0x30b8, 0x20);
		sccb_write(SLAVE_ID,0x30b9, 0x17);
		sccb_write(SLAVE_ID,0x30ba, 0x4);
		sccb_write(SLAVE_ID,0x30bb, 0x8);
		sccb_write(SLAVE_ID,0x3507, 0x6);
		sccb_write(SLAVE_ID,0x350a, 0x4f);
		sccb_write(SLAVE_ID,0x3100, 0x2);
		sccb_write(SLAVE_ID,0x3304, 0x0);
		sccb_write(SLAVE_ID,0x3400, 0x0);
		sccb_write(SLAVE_ID,0x3404, 0x0);
		sccb_write(SLAVE_ID,0x3500, 0x0);
		sccb_write(SLAVE_ID,0x3600, 0xc0);
		sccb_write(SLAVE_ID,0x3610, 0x60);
		sccb_write(SLAVE_ID,0x350a, 0x4f);
		sccb_write(SLAVE_ID,0x3088, 0x1);
		sccb_write(SLAVE_ID,0x3089, 0x40);
		sccb_write(SLAVE_ID,0x308a, 0x0);
		sccb_write(SLAVE_ID,0x308b, 0xf0);
		sccb_write(SLAVE_ID,0x3302, 0xef);
		sccb_write(SLAVE_ID,0x335f, 0x68);
		sccb_write(SLAVE_ID,0x3360, 0x18);
		sccb_write(SLAVE_ID,0x3361, 0xc);
		sccb_write(SLAVE_ID,0x3362, 0x1);
		sccb_write(SLAVE_ID,0x3363, 0x48);
		sccb_write(SLAVE_ID,0x3364, 0xf4);
		sccb_write(SLAVE_ID,0x3403, 0x42);
		sccb_write(SLAVE_ID,0x3088, 0x1);
		sccb_write(SLAVE_ID,0x3089, 0x40);
		sccb_write(SLAVE_ID,0x308a, 0x0);
		sccb_write(SLAVE_ID,0x308b, 0xf0);
		sccb_write(SLAVE_ID,0x3100, 0x2);
		sccb_write(SLAVE_ID,0x3304, 0x0);
		sccb_write(SLAVE_ID,0x3400, 0x0);
		sccb_write(SLAVE_ID,0x3404, 0x0);
		sccb_write(SLAVE_ID,0x3600, 0xc0);
		sccb_write(SLAVE_ID,0x308d, 0x4);
		sccb_write(SLAVE_ID,0x3086, 0x0);
		sccb_write(SLAVE_ID,0x3086, 0x0);

		//15FPS
		sccb_write(SLAVE_ID, 0x300e, 0x32);
		sccb_write(SLAVE_ID, 0x3011, 0x0);
		sccb_write(SLAVE_ID, 0x3010, 0x81);
		sccb_write(SLAVE_ID, 0x3014, 0x4);
		sccb_write(SLAVE_ID, 0x302e, 0x0);
		sccb_write(SLAVE_ID, 0x302d, 0x0);

	if((nWidthH == 640) &&(nWidthV == 480))
	{
		//VGA
		sccb_write(SLAVE_ID, 0x3012, 0x10);
		sccb_write(SLAVE_ID, 0x3023, 0x6);
		sccb_write(SLAVE_ID, 0x3026, 0x3);
		sccb_write(SLAVE_ID, 0x3027, 0x4);
		sccb_write(SLAVE_ID, 0x302a, 0x3);
		sccb_write(SLAVE_ID, 0x302b, 0x10);
		sccb_write(SLAVE_ID, 0x3075, 0x24);
		sccb_write(SLAVE_ID, 0x300d, 0x1);
		sccb_write(SLAVE_ID, 0x30d7, 0x90);
		sccb_write(SLAVE_ID, 0x3069, 0x4);
		sccb_write(SLAVE_ID, 0x303e, 0x0);
		sccb_write(SLAVE_ID, 0x303f, 0xc0);
		sccb_write(SLAVE_ID, 0x3302, 0xef);
		sccb_write(SLAVE_ID, 0x335f, 0x34);
		sccb_write(SLAVE_ID, 0x3360, 0xc);
		sccb_write(SLAVE_ID, 0x3361, 0x4);
		sccb_write(SLAVE_ID, 0x3362, 0x12);
		sccb_write(SLAVE_ID, 0x3363, 0x88);
		sccb_write(SLAVE_ID, 0x3364, 0xe4);
		sccb_write(SLAVE_ID, 0x3403, 0x42);
		sccb_write(SLAVE_ID, 0x3088, 0x12);
		sccb_write(SLAVE_ID, 0x3089, 0x80);
		sccb_write(SLAVE_ID, 0x308a, 0x1);
		sccb_write(SLAVE_ID, 0x308b, 0xe0);
		sccb_write(SLAVE_ID, 0x3100, 0x2);
		sccb_write(SLAVE_ID, 0x3301, 0xde);
		sccb_write(SLAVE_ID, 0x3304, 0x0);
		sccb_write(SLAVE_ID, 0x3400, 0x0);
		sccb_write(SLAVE_ID, 0x3404, 0x0);
		sccb_write(SLAVE_ID, 0x304c, 0x82);
		sccb_write(SLAVE_ID, 0x3011, 0x3);// set CLKO=SYS/2 to get 15FPS
	}

	if((nWidthH == 2048) &&(nWidthV == 1536))
	{	//Capture Test
		//QXGA 2048X1536
		sccb_write(SLAVE_ID,0x3012,0x0);
		sccb_write(SLAVE_ID,0x304d,0x45);
		sccb_write(SLAVE_ID,0x30a7,0x5e);
		sccb_write(SLAVE_ID,0x3087,0x2);
		sccb_write(SLAVE_ID,0x309c,0x1a);
		sccb_write(SLAVE_ID,0x30a2,0xe4);
		sccb_write(SLAVE_ID,0x30aa,0x42);
		sccb_write(SLAVE_ID,0x30b0,0xff);
		sccb_write(SLAVE_ID,0x30b1,0xff);
		sccb_write(SLAVE_ID,0x30b2,0x10);
		sccb_write(SLAVE_ID,0x300e,0x32);
		sccb_write(SLAVE_ID,0x300f,0x21);
		sccb_write(SLAVE_ID,0x3010,0x20);
		sccb_write(SLAVE_ID,0x3011,0x1);
		sccb_write(SLAVE_ID,0x304c,0x82);
		sccb_write(SLAVE_ID,0x30d7,0x10);
		sccb_write(SLAVE_ID,0x30d9,0xd);
		sccb_write(SLAVE_ID,0x30db,0x8);
		sccb_write(SLAVE_ID,0x3016,0x82);
		sccb_write(SLAVE_ID,0x3018,0x38);
		sccb_write(SLAVE_ID,0x3019,0x30);
		sccb_write(SLAVE_ID,0x301a,0x61);
		sccb_write(SLAVE_ID,0x307d,0x0);
		sccb_write(SLAVE_ID,0x3087,0x2);
		sccb_write(SLAVE_ID,0x3082,0x20);
		sccb_write(SLAVE_ID,0x3015,0x12);
		sccb_write(SLAVE_ID,0x3014,0xc);
		sccb_write(SLAVE_ID,0x3013,0xf7);
		sccb_write(SLAVE_ID,0x303c,0x8);
		sccb_write(SLAVE_ID,0x303d,0x18);
		sccb_write(SLAVE_ID,0x303e,0x6);
		sccb_write(SLAVE_ID,0x303f,0xc);
		sccb_write(SLAVE_ID,0x3030,0x62);
		sccb_write(SLAVE_ID,0x3031,0x26);
		sccb_write(SLAVE_ID,0x3032,0xe6);
		sccb_write(SLAVE_ID,0x3033,0x6e);
		sccb_write(SLAVE_ID,0x3034,0xea);
		sccb_write(SLAVE_ID,0x3035,0xae);
		sccb_write(SLAVE_ID,0x3036,0xa6);
		sccb_write(SLAVE_ID,0x3037,0x6a);
		sccb_write(SLAVE_ID,0x3104,0x2);
		sccb_write(SLAVE_ID,0x3105,0xfd);
		sccb_write(SLAVE_ID,0x3106,0x0);
		sccb_write(SLAVE_ID,0x3107,0xff);
		sccb_write(SLAVE_ID,0x3300,0x13);
		sccb_write(SLAVE_ID,0x3301,0xde);
		sccb_write(SLAVE_ID,0x3302,0xcf);
		sccb_write(SLAVE_ID,0x3312,0x26);
		sccb_write(SLAVE_ID,0x3314,0x42);
		sccb_write(SLAVE_ID,0x3313,0x2b);
		sccb_write(SLAVE_ID,0x3315,0x42);
		sccb_write(SLAVE_ID,0x3310,0xd0);
		sccb_write(SLAVE_ID,0x3311,0xbd);
		sccb_write(SLAVE_ID,0x330c,0x18);
		sccb_write(SLAVE_ID,0x330d,0x18);
		sccb_write(SLAVE_ID,0x330e,0x56);
		sccb_write(SLAVE_ID,0x330f,0x5c);
		sccb_write(SLAVE_ID,0x330b,0x1c);
		sccb_write(SLAVE_ID,0x3306,0x5c);
		sccb_write(SLAVE_ID,0x3307,0x11);
		sccb_write(SLAVE_ID,0x336a,0x52);
		sccb_write(SLAVE_ID,0x3370,0x46);
		sccb_write(SLAVE_ID,0x3376,0x38);
		sccb_write(SLAVE_ID,0x3300,0x13);
		sccb_write(SLAVE_ID,0x30b8,0x20);
		sccb_write(SLAVE_ID,0x30b9,0x17);
		sccb_write(SLAVE_ID,0x30ba,0x4);
		sccb_write(SLAVE_ID,0x30bb,0x8);
		sccb_write(SLAVE_ID,0x3507,0x6);
		sccb_write(SLAVE_ID,0x350a,0x4f);
		sccb_write(SLAVE_ID,0x3100,0x32);
		sccb_write(SLAVE_ID,0x3304,0x0);
		sccb_write(SLAVE_ID,0x3400,0x2);
		sccb_write(SLAVE_ID,0x3404,0x22);
		sccb_write(SLAVE_ID,0x3500,0x0);
		sccb_write(SLAVE_ID,0x3600,0xc0);
		sccb_write(SLAVE_ID,0x3610,0x60);
		sccb_write(SLAVE_ID,0x350a,0x4f);
		sccb_write(SLAVE_ID,0x3088,0x8);
		sccb_write(SLAVE_ID,0x3089,0x0);
		sccb_write(SLAVE_ID,0x308a,0x6);
		sccb_write(SLAVE_ID,0x308b,0x0);
		sccb_write(SLAVE_ID,0x308d,0x4);
		sccb_write(SLAVE_ID,0x3086,0x0);
		sccb_write(SLAVE_ID,0x3086,0x0);

		//QXGA YUV
		sccb_write(SLAVE_ID,0x3012,0x0);
		sccb_write(SLAVE_ID,0x3020,0x1);
		sccb_write(SLAVE_ID,0x3021,0x1d);
		sccb_write(SLAVE_ID,0x3022,0x0);
		sccb_write(SLAVE_ID,0x3023,0xa);
		sccb_write(SLAVE_ID,0x3024,0x8);
		sccb_write(SLAVE_ID,0x3025,0x18);
		sccb_write(SLAVE_ID,0x3026,0x6);
		sccb_write(SLAVE_ID,0x3027,0xc);
		sccb_write(SLAVE_ID,0x302a,0x6);
		sccb_write(SLAVE_ID,0x302b,0x20);
		sccb_write(SLAVE_ID,0x3075,0x44);
		sccb_write(SLAVE_ID,0x300d,0x0);
		sccb_write(SLAVE_ID,0x30d7,0x10);
		sccb_write(SLAVE_ID,0x3069,0x44);
		sccb_write(SLAVE_ID,0x303e,0x1);
		sccb_write(SLAVE_ID,0x303f,0x80);
		sccb_write(SLAVE_ID,0x3302,0xef);
		sccb_write(SLAVE_ID,0x335f,0x68);
		sccb_write(SLAVE_ID,0x3360,0x18);
		sccb_write(SLAVE_ID,0x3361,0xc);
		sccb_write(SLAVE_ID,0x3362,0x68);
		sccb_write(SLAVE_ID,0x3363,0x8);
		sccb_write(SLAVE_ID,0x3364,0x4);
		sccb_write(SLAVE_ID,0x3403,0x42);
		sccb_write(SLAVE_ID,0x3088,0x8);
		sccb_write(SLAVE_ID,0x3089,0x0);
		sccb_write(SLAVE_ID,0x308a,0x6);
		sccb_write(SLAVE_ID,0x308b,0x0);
		sccb_write(SLAVE_ID,0x3100,0x2);
		sccb_write(SLAVE_ID,0x3301,0xde);
		sccb_write(SLAVE_ID,0x3304,0x0);
		sccb_write(SLAVE_ID,0x3400,0x0);
		sccb_write(SLAVE_ID,0x3404,0x0);
		sccb_write(SLAVE_ID,0x304c,0x81);
		sccb_write(SLAVE_ID,0x3011,0x5);

#if 0
		//correct yellowish effect in QXGA
		sccb_write(SLAVE_ID,0x3012,0x00);
		sccb_write(SLAVE_ID,0x3020,0x01);
		sccb_write(SLAVE_ID,0x3021,0x1d);
		sccb_write(SLAVE_ID,0x3022,0x00);
		sccb_write(SLAVE_ID,0x3023,0x0a);
		sccb_write(SLAVE_ID,0x3024,0x08);
		sccb_write(SLAVE_ID,0x3025,0x18);
		sccb_write(SLAVE_ID,0x3026,0x06);
		sccb_write(SLAVE_ID,0x3027,0x0c);
		sccb_write(SLAVE_ID,0x302a,0x06);
		sccb_write(SLAVE_ID,0x302b,0x20);
		sccb_write(SLAVE_ID,0x3075,0x44);
		sccb_write(SLAVE_ID,0x300d,0x00);
		sccb_write(SLAVE_ID,0x30d7,0x00);
		sccb_write(SLAVE_ID,0x3069,0x40);
		sccb_write(SLAVE_ID,0x303e,0x01);
		sccb_write(SLAVE_ID,0x303f,0x80);

		sccb_write(SLAVE_ID,0x3302,0x20);
		sccb_write(SLAVE_ID,0x335f,0x68);
		sccb_write(SLAVE_ID,0x3360,0x18);
		sccb_write(SLAVE_ID,0x3361,0x0c);
		sccb_write(SLAVE_ID,0x3362,0x68);
		sccb_write(SLAVE_ID,0x3363,0x08);
		sccb_write(SLAVE_ID,0x3364,0x04);
		sccb_write(SLAVE_ID,0x3403,0x42);
		sccb_write(SLAVE_ID,0x3088,0x08);
		sccb_write(SLAVE_ID,0x3089,0x00);
		sccb_write(SLAVE_ID,0x308a,0x06);
		sccb_write(SLAVE_ID,0x308b,0x00);
#endif

//Color Saturation
//		sccb_write(SLAVE_ID,0x3302,0xef);
//		sccb_write(SLAVE_ID,0x3358,0x40);
//		sccb_write(SLAVE_ID,0x3359,0x40);
//		sccb_write(SLAVE_ID,0x3355,0x0);
	}
	//V Flip & Mirror
	sccb_write(SLAVE_ID,0x307c, 0x13);
	sccb_write(SLAVE_ID,0x3023, 0x09);
	sccb_write(SLAVE_ID,0x3090, 0xc9);

	R_CSI_TG_CTRL1 = uCtrlReg2;					//*P_Sensor_TG_Ctrl2 = uCtrlReg2;
	R_CSI_TG_CTRL0 = uCtrlReg1;					//*P_Sensor_TG_Ctrl1 = uCtrlReg1;
}
#endif
#ifdef	__OV5642_DRV_C__
//====================================================================================================
//	Description:	OV5642 Initialization
//	Syntax:			void OV5642_Init (
//						INT16S nWidthH,			// Active H Width
//						INT16S nWidthV,			// Active V Width
//						INT16U uFlag				// Flag Type
//					);
//	Return:			None
//====================================================================================================
void OV5642_Init (
	INT16S nWidthH,			// Active H Width
	INT16S nWidthV,			// Active V Width
	INT16U uFlag				// Flag Type
) {
	INT16U uCtrlReg1, uCtrlReg2;
	INT16S nReso;

	// Enable CSI clock to let sensor initialize at first
#if CSI_CLOCK == CSI_CLOCK_27MHZ
	uCtrlReg2 = CLKOEN | CSI_RGB565 |CLK_SEL27M | CSI_HIGHPRI | CSI_NOSTOP;
#else
	uCtrlReg2 = CLKOEN | CSI_RGB565 |CLK_SEL48M | CSI_HIGHPRI | CSI_NOSTOP;
#endif

	uCtrlReg1 = CSIEN | YUV_YUYV | CAP;									// Default CSI Control Register 1
	if (uFlag & FT_CSI_RGB1555)											// RGB1555
	{
		uCtrlReg2 |= CSI_RGB1555;
	}
	if (uFlag & FT_CSI_CCIR656)										// CCIR656?
	{
		uCtrlReg1 |= CCIR656 | VADD_FALL | VRST_FALL | HRST_FALL;	// CCIR656
		uCtrlReg2 |= D_TYPE1;										// CCIR656
	}
	else
	{
		uCtrlReg1 |= VADD_RISE | VRST_FALL | HRST_RISE | HREF;		// NOT CCIR656
		uCtrlReg2 |= D_TYPE0;										// NOT CCIR656
	}
	if (uFlag & FT_CSI_YUVIN)										// YUVIN?
	{
		uCtrlReg1 |= YUVIN;
	}
	if (uFlag & FT_CSI_YUVOUT)										// YUVOUT?
	{
		uCtrlReg1 |= YUVOUT;
	}

#if 0
	// Whether compression or not?
	nReso = ((nWidthH == 320) && (nWidthV == 240)) ? 1 : 0;
	if (nReso == 1)								// VGA
	{
#ifdef	__TV_QVGA__
		R_CSI_TG_HRATIO =0x0101;					// Scale to 1/2
		R_CSI_TG_VRATIO =0x0101;					// Scale to 1/2
//		R_CSI_TG_HWIDTH = nWidthH;					// Horizontal frame width
//		R_CSI_TG_VHEIGHT = nWidthV*2;				// Vertical frame width
#endif	// __TV_QVGA__
	}
#endif

	R_CSI_TG_VL0START = 0x0000;						// Sensor field 0 vertical latch start register.
	R_CSI_TG_VL1START = 0x0000;						//*P_Sensor_TG_V_L1Start = 0x0000;
	R_CSI_TG_HSTART = 0x0000;						// Sensor horizontal start register.

	R_CSI_TG_CTRL1 = CSI_NOSTOP|CLKOEN;				//enable CSI CLKO
	drv_msec_wait(100); 							//wait 100ms for CLKO stable


	// CMOS Sensor Initialization Start...
	sccb_init (SCCB_SCL, SCCB_SDA);
//	sccb_delay (200);
//	sccb_write (SLAVE_ID, 0x3012, 0x80);
//	sccb_delay (200);

	if((nWidthH == 320) && (nWidthV == 240))
	{
		//QVGA
		sccb_write(SLAVE_ID, 0x3103, 0x3);
		sccb_write(SLAVE_ID, 0x3008, 0x2);
		sccb_write(SLAVE_ID, 0x3017, 0x7f);
		sccb_write(SLAVE_ID, 0x3018, 0xfc);
		sccb_write(SLAVE_ID, 0x3810, 0xc0);
		sccb_write(SLAVE_ID, 0x3615, 0xf0);
		sccb_write(SLAVE_ID, 0x3000, 0xf8);
		sccb_write(SLAVE_ID, 0x3001, 0x48);
		sccb_write(SLAVE_ID, 0x3002, 0x5c);
		sccb_write(SLAVE_ID, 0x3003, 0x2);
		sccb_write(SLAVE_ID, 0x3000, 0xf8);
		sccb_write(SLAVE_ID, 0x3001, 0x48);
		sccb_write(SLAVE_ID, 0x3002, 0x5c);
		sccb_write(SLAVE_ID, 0x3003, 0x2);
		sccb_write(SLAVE_ID, 0x3004, 0xff);
		sccb_write(SLAVE_ID, 0x3005, 0xb7);
		sccb_write(SLAVE_ID, 0x3006, 0x43);
		sccb_write(SLAVE_ID, 0x3007, 0x37);
		sccb_write(SLAVE_ID, 0x3011, 0x9);
		sccb_write(SLAVE_ID, 0x3012, 0x2);
		sccb_write(SLAVE_ID, 0x3010, 0x0);
		sccb_write(SLAVE_ID, 0x460c, 0x20);
		sccb_write(SLAVE_ID, 0x3815, 0x4);
		sccb_write(SLAVE_ID, 0x370d, 0x2);
		sccb_write(SLAVE_ID, 0x370c, 0xa0);
		sccb_write(SLAVE_ID, 0x3602, 0xfc);
		sccb_write(SLAVE_ID, 0x3612, 0xff);
		sccb_write(SLAVE_ID, 0x3634, 0xc0);
		sccb_write(SLAVE_ID, 0x3613, 0x0);
		sccb_write(SLAVE_ID, 0x3605, 0x4);
		sccb_write(SLAVE_ID, 0x3621, 0xc7);
		sccb_write(SLAVE_ID, 0x3622, 0x0);
		sccb_write(SLAVE_ID, 0x3604, 0x40);
		sccb_write(SLAVE_ID, 0x3603, 0x27);
		sccb_write(SLAVE_ID, 0x3603, 0x27);
		sccb_write(SLAVE_ID, 0x4000, 0x21);
		sccb_write(SLAVE_ID, 0x401d, 0x2);
		sccb_write(SLAVE_ID, 0x3600, 0x54);
		sccb_write(SLAVE_ID, 0x3605, 0x4);
		sccb_write(SLAVE_ID, 0x3606, 0x3f);
		sccb_write(SLAVE_ID, 0x3c01, 0x80);
		sccb_write(SLAVE_ID, 0x5000, 0x4f);
		sccb_write(SLAVE_ID, 0x5020, 0x4);
		sccb_write(SLAVE_ID, 0x5181, 0x79);
		sccb_write(SLAVE_ID, 0x5182, 0x0);
		sccb_write(SLAVE_ID, 0x5185, 0x22);
		sccb_write(SLAVE_ID, 0x5197, 0x1);
		sccb_write(SLAVE_ID, 0x5001, 0x7f);
		sccb_write(SLAVE_ID, 0x5500, 0xa);
		sccb_write(SLAVE_ID, 0x5504, 0x0);
		sccb_write(SLAVE_ID, 0x5505, 0x7f);
		sccb_write(SLAVE_ID, 0x5080, 0x8);
		sccb_write(SLAVE_ID, 0x300e, 0x18);
		sccb_write(SLAVE_ID, 0x4610, 0x0);
		sccb_write(SLAVE_ID, 0x471d, 0x5);
		sccb_write(SLAVE_ID, 0x4708, 0x6);
		sccb_write(SLAVE_ID, 0x3710, 0x10);
		sccb_write(SLAVE_ID, 0x3632, 0x41);
		sccb_write(SLAVE_ID, 0x3702, 0x10);
		sccb_write(SLAVE_ID, 0x3620, 0x52);
		sccb_write(SLAVE_ID, 0x3631, 0x1);
		sccb_write(SLAVE_ID, 0x3808, 0x1);
		sccb_write(SLAVE_ID, 0x3809, 0x40);
		sccb_write(SLAVE_ID, 0x380a, 0x0);
		sccb_write(SLAVE_ID, 0x380b, 0xf0);
		sccb_write(SLAVE_ID, 0x380e, 0x3);
		sccb_write(SLAVE_ID, 0x380f, 0xe8);
		sccb_write(SLAVE_ID, 0x501f, 0x0);
		sccb_write(SLAVE_ID, 0x5000, 0x4f);
		sccb_write(SLAVE_ID, 0x4300, 0x30);
		sccb_write(SLAVE_ID, 0x3503, 0x0);
		sccb_write(SLAVE_ID, 0x3501, 0x3e);
		sccb_write(SLAVE_ID, 0x3502, 0x40);
		sccb_write(SLAVE_ID, 0x350b, 0x12);
		sccb_write(SLAVE_ID, 0x3503, 0x0);
		sccb_write(SLAVE_ID, 0x3824, 0x11);
		sccb_write(SLAVE_ID, 0x3501, 0x3e);
		sccb_write(SLAVE_ID, 0x3502, 0x40);
		sccb_write(SLAVE_ID, 0x350b, 0x12);
		sccb_write(SLAVE_ID, 0x380c, 0x7);
		sccb_write(SLAVE_ID, 0x380d, 0xa);
		sccb_write(SLAVE_ID, 0x380e, 0x3);
		sccb_write(SLAVE_ID, 0x380f, 0xe8);
		sccb_write(SLAVE_ID, 0x3a0d, 0x4);
		sccb_write(SLAVE_ID, 0x3a0e, 0x3);
//		sccb_write(SLAVE_ID, 0x3818, 0xc1);
		sccb_write(SLAVE_ID, 0x3818, 0xa1);	//VFlip set/clear 0x3818[5]
		sccb_write(SLAVE_ID, 0x3705, 0xdb);
		sccb_write(SLAVE_ID, 0x370a, 0x81);
		sccb_write(SLAVE_ID, 0x3801, 0x50);
//		sccb_write(SLAVE_ID, 0x3621, 0xc7);
		sccb_write(SLAVE_ID, 0x3621, 0xe7);	//Mirror
		sccb_write(SLAVE_ID, 0x3801, 0x50);
		sccb_write(SLAVE_ID, 0x3803, 0x8);
		sccb_write(SLAVE_ID, 0x3827, 0x8);
		sccb_write(SLAVE_ID, 0x3810, 0xc0);
		sccb_write(SLAVE_ID, 0x3804, 0x5);
		sccb_write(SLAVE_ID, 0x3805, 0x0);
		sccb_write(SLAVE_ID, 0x5682, 0x5);
		sccb_write(SLAVE_ID, 0x5683, 0x0);
		sccb_write(SLAVE_ID, 0x3806, 0x3);
		sccb_write(SLAVE_ID, 0x3807, 0xc0);
		sccb_write(SLAVE_ID, 0x5686, 0x3);
		sccb_write(SLAVE_ID, 0x5687, 0xc0);
		sccb_write(SLAVE_ID, 0x3a00, 0x78);
		sccb_write(SLAVE_ID, 0x3a1a, 0x4);
		sccb_write(SLAVE_ID, 0x3a13, 0x30);
		sccb_write(SLAVE_ID, 0x3a18, 0x0);
		sccb_write(SLAVE_ID, 0x3a19, 0x7c);
		sccb_write(SLAVE_ID, 0x3a08, 0x12);
		sccb_write(SLAVE_ID, 0x3a09, 0xc0);
		sccb_write(SLAVE_ID, 0x3a0a, 0xf);
		sccb_write(SLAVE_ID, 0x3a0b, 0xa0);
		sccb_write(SLAVE_ID, 0x3004, 0xff);
		sccb_write(SLAVE_ID, 0x350c, 0x3);
		sccb_write(SLAVE_ID, 0x350d, 0xe8);
		sccb_write(SLAVE_ID, 0x3500, 0x0);
		sccb_write(SLAVE_ID, 0x3501, 0x3e);
		sccb_write(SLAVE_ID, 0x3502, 0x40);
		sccb_write(SLAVE_ID, 0x350a, 0x0);
		sccb_write(SLAVE_ID, 0x350b, 0x12);
		sccb_write(SLAVE_ID, 0x3503, 0x0);
		sccb_write(SLAVE_ID, 0x528a, 0x2);
		sccb_write(SLAVE_ID, 0x528b, 0x6);
		sccb_write(SLAVE_ID, 0x528c, 0x20);
		sccb_write(SLAVE_ID, 0x528d, 0x30);
		sccb_write(SLAVE_ID, 0x528e, 0x40);
		sccb_write(SLAVE_ID, 0x528f, 0x50);
		sccb_write(SLAVE_ID, 0x5290, 0x60);
		sccb_write(SLAVE_ID, 0x5292, 0x0);
		sccb_write(SLAVE_ID, 0x5293, 0x2);
		sccb_write(SLAVE_ID, 0x5294, 0x0);
		sccb_write(SLAVE_ID, 0x5295, 0x4);
		sccb_write(SLAVE_ID, 0x5296, 0x0);
		sccb_write(SLAVE_ID, 0x5297, 0x8);
		sccb_write(SLAVE_ID, 0x5298, 0x0);
		sccb_write(SLAVE_ID, 0x5299, 0x10);
		sccb_write(SLAVE_ID, 0x529a, 0x0);
		sccb_write(SLAVE_ID, 0x529b, 0x20);
		sccb_write(SLAVE_ID, 0x529c, 0x0);
		sccb_write(SLAVE_ID, 0x529d, 0x28);
		sccb_write(SLAVE_ID, 0x529e, 0x0);
		sccb_write(SLAVE_ID, 0x529f, 0x30);
		sccb_write(SLAVE_ID, 0x3a0f, 0x3c);
		sccb_write(SLAVE_ID, 0x3a10, 0x30);
		sccb_write(SLAVE_ID, 0x3a1b, 0x3c);
		sccb_write(SLAVE_ID, 0x3a1e, 0x30);
		sccb_write(SLAVE_ID, 0x3a11, 0x70);
		sccb_write(SLAVE_ID, 0x3a1f, 0x10);
		sccb_write(SLAVE_ID, 0x3030, 0x2b);
		sccb_write(SLAVE_ID, 0x3a02, 0x0);
		sccb_write(SLAVE_ID, 0x3a03, 0x7d);
		sccb_write(SLAVE_ID, 0x3a04, 0x0);
		sccb_write(SLAVE_ID, 0x3a14, 0x0);
		sccb_write(SLAVE_ID, 0x3a15, 0x7d);
		sccb_write(SLAVE_ID, 0x3a16, 0x0);
		sccb_write(SLAVE_ID, 0x3a00, 0x78);
		sccb_write(SLAVE_ID, 0x3a08, 0x12);
		sccb_write(SLAVE_ID, 0x3a09, 0xc0);
		sccb_write(SLAVE_ID, 0x3a0a, 0xf);
		sccb_write(SLAVE_ID, 0x3a0b, 0xa0);
		sccb_write(SLAVE_ID, 0x3a0d, 0x4);
		sccb_write(SLAVE_ID, 0x3a0e, 0x3);
		sccb_write(SLAVE_ID, 0x5193, 0x70);
		sccb_write(SLAVE_ID, 0x3620, 0x52);
		sccb_write(SLAVE_ID, 0x3703, 0xb2);
		sccb_write(SLAVE_ID, 0x3704, 0x18);
		sccb_write(SLAVE_ID, 0x589b, 0x4);
		sccb_write(SLAVE_ID, 0x589a, 0xc5);
		sccb_write(SLAVE_ID, 0x528a, 0x2);
		sccb_write(SLAVE_ID, 0x528b, 0x6);
		sccb_write(SLAVE_ID, 0x528c, 0x20);
		sccb_write(SLAVE_ID, 0x528d, 0x30);
		sccb_write(SLAVE_ID, 0x528e, 0x40);
		sccb_write(SLAVE_ID, 0x528f, 0x50);
		sccb_write(SLAVE_ID, 0x5290, 0x60);
		sccb_write(SLAVE_ID, 0x5293, 0x2);
		sccb_write(SLAVE_ID, 0x5295, 0x4);
		sccb_write(SLAVE_ID, 0x5297, 0x8);
		sccb_write(SLAVE_ID, 0x5299, 0x10);
		sccb_write(SLAVE_ID, 0x529b, 0x20);
		sccb_write(SLAVE_ID, 0x529d, 0x28);
		sccb_write(SLAVE_ID, 0x529f, 0x30);
		sccb_write(SLAVE_ID, 0x5300, 0x0);
		sccb_write(SLAVE_ID, 0x5301, 0x20);
		sccb_write(SLAVE_ID, 0x5302, 0x0);
		sccb_write(SLAVE_ID, 0x5303, 0x7c);
		sccb_write(SLAVE_ID, 0x530c, 0x0);
		sccb_write(SLAVE_ID, 0x530d, 0xc);
		sccb_write(SLAVE_ID, 0x530e, 0x20);
		sccb_write(SLAVE_ID, 0x530f, 0x80);
		sccb_write(SLAVE_ID, 0x5310, 0x20);
		sccb_write(SLAVE_ID, 0x5311, 0x80);
		sccb_write(SLAVE_ID, 0x5380, 0x1);
		sccb_write(SLAVE_ID, 0x5381, 0x0);
		sccb_write(SLAVE_ID, 0x5382, 0x0);
		sccb_write(SLAVE_ID, 0x5383, 0x4e);
		sccb_write(SLAVE_ID, 0x5384, 0x0);
		sccb_write(SLAVE_ID, 0x5385, 0xf);
		sccb_write(SLAVE_ID, 0x5386, 0x0);
		sccb_write(SLAVE_ID, 0x5387, 0x0);
		sccb_write(SLAVE_ID, 0x5388, 0x1);
		sccb_write(SLAVE_ID, 0x5389, 0x15);
		sccb_write(SLAVE_ID, 0x538a, 0x0);
		sccb_write(SLAVE_ID, 0x538b, 0x31);
		sccb_write(SLAVE_ID, 0x538c, 0x0);
		sccb_write(SLAVE_ID, 0x538d, 0x0);
		sccb_write(SLAVE_ID, 0x538e, 0x0);
		sccb_write(SLAVE_ID, 0x538f, 0xf);
		sccb_write(SLAVE_ID, 0x5390, 0x0);
		sccb_write(SLAVE_ID, 0x5391, 0xab);
		sccb_write(SLAVE_ID, 0x5392, 0x0);
		sccb_write(SLAVE_ID, 0x5393, 0xa2);
		sccb_write(SLAVE_ID, 0x5394, 0x8);
		sccb_write(SLAVE_ID, 0x5480, 0x14);
		sccb_write(SLAVE_ID, 0x5481, 0x21);
		sccb_write(SLAVE_ID, 0x5482, 0x36);
		sccb_write(SLAVE_ID, 0x5483, 0x57);
		sccb_write(SLAVE_ID, 0x5484, 0x65);
		sccb_write(SLAVE_ID, 0x5485, 0x71);
		sccb_write(SLAVE_ID, 0x5486, 0x7d);
		sccb_write(SLAVE_ID, 0x5487, 0x87);
		sccb_write(SLAVE_ID, 0x5488, 0x91);
		sccb_write(SLAVE_ID, 0x5489, 0x9a);
		sccb_write(SLAVE_ID, 0x548a, 0xaa);
		sccb_write(SLAVE_ID, 0x548b, 0xb8);
		sccb_write(SLAVE_ID, 0x548c, 0xcd);
		sccb_write(SLAVE_ID, 0x548d, 0xdd);
		sccb_write(SLAVE_ID, 0x548e, 0xea);
		sccb_write(SLAVE_ID, 0x548f, 0x10);
		sccb_write(SLAVE_ID, 0x5490, 0x5);
		sccb_write(SLAVE_ID, 0x5491, 0x0);
		sccb_write(SLAVE_ID, 0x5492, 0x4);
		sccb_write(SLAVE_ID, 0x5493, 0x20);
		sccb_write(SLAVE_ID, 0x5494, 0x3);
		sccb_write(SLAVE_ID, 0x5495, 0x60);
		sccb_write(SLAVE_ID, 0x5496, 0x2);
		sccb_write(SLAVE_ID, 0x5497, 0xb8);
		sccb_write(SLAVE_ID, 0x5498, 0x2);
		sccb_write(SLAVE_ID, 0x5499, 0x86);
		sccb_write(SLAVE_ID, 0x549a, 0x2);
		sccb_write(SLAVE_ID, 0x549b, 0x5b);
		sccb_write(SLAVE_ID, 0x549c, 0x2);
		sccb_write(SLAVE_ID, 0x549d, 0x3b);
		sccb_write(SLAVE_ID, 0x549e, 0x2);
		sccb_write(SLAVE_ID, 0x549f, 0x1c);
		sccb_write(SLAVE_ID, 0x54a0, 0x2);
		sccb_write(SLAVE_ID, 0x54a1, 0x4);
		sccb_write(SLAVE_ID, 0x54a2, 0x1);
		sccb_write(SLAVE_ID, 0x54a3, 0xed);
		sccb_write(SLAVE_ID, 0x54a4, 0x1);
		sccb_write(SLAVE_ID, 0x54a5, 0xc5);
		sccb_write(SLAVE_ID, 0x54a6, 0x1);
		sccb_write(SLAVE_ID, 0x54a7, 0xa5);
		sccb_write(SLAVE_ID, 0x54a8, 0x1);
		sccb_write(SLAVE_ID, 0x54a9, 0x6c);
		sccb_write(SLAVE_ID, 0x54aa, 0x1);
		sccb_write(SLAVE_ID, 0x54ab, 0x41);
		sccb_write(SLAVE_ID, 0x54ac, 0x1);
		sccb_write(SLAVE_ID, 0x54ad, 0x20);
		sccb_write(SLAVE_ID, 0x54ae, 0x0);
		sccb_write(SLAVE_ID, 0x54af, 0x16);
		sccb_write(SLAVE_ID, 0x3633, 0x7);
		sccb_write(SLAVE_ID, 0x3702, 0x10);
		sccb_write(SLAVE_ID, 0x3703, 0xb2);
		sccb_write(SLAVE_ID, 0x3704, 0x18);
		sccb_write(SLAVE_ID, 0x370b, 0x40);
		sccb_write(SLAVE_ID, 0x370d, 0x2);
		sccb_write(SLAVE_ID, 0x3620, 0x52);
		sccb_write(SLAVE_ID, 0x3808, 0x1);
		sccb_write(SLAVE_ID, 0x3809, 0x40);
		sccb_write(SLAVE_ID, 0x380a, 0x0);
		sccb_write(SLAVE_ID, 0x380b, 0xf0);
		//Mirror
//		sccb_write(SLAVE_ID, 0x3621, 0xe7);
//		sccb_write(SLAVE_ID, 0x3824, 0x01);
	}

	if((nWidthH == 640) && (nWidthV == 480))
	{
		//VGA 17FPS
		sccb_write(SLAVE_ID,0x3103,0x3);
		sccb_write(SLAVE_ID,0x3008,0x2);
		sccb_write(SLAVE_ID,0x3017,0x7f);
		sccb_write(SLAVE_ID,0x3018,0xfc);
		sccb_write(SLAVE_ID,0x3810,0xc0);
		sccb_write(SLAVE_ID,0x3615,0xf0);
		sccb_write(SLAVE_ID,0x3000,0xf8);
		sccb_write(SLAVE_ID,0x3001,0x48);
		sccb_write(SLAVE_ID,0x3002,0x5c);
		sccb_write(SLAVE_ID,0x3003,0x2);
		sccb_write(SLAVE_ID,0x3000,0xf8);
		sccb_write(SLAVE_ID,0x3001,0x48);
		sccb_write(SLAVE_ID,0x3002,0x5c);
		sccb_write(SLAVE_ID,0x3003,0x2);
		sccb_write(SLAVE_ID,0x3004,0xff);
		sccb_write(SLAVE_ID,0x3005,0xb7);
		sccb_write(SLAVE_ID,0x3006,0x43);
		sccb_write(SLAVE_ID,0x3007,0x37);
		sccb_write(SLAVE_ID,0x3011,0x8);
		sccb_write(SLAVE_ID,0x3010,0x10);
		sccb_write(SLAVE_ID,0x460c,0x22);
		sccb_write(SLAVE_ID,0x3815,0x4);
		sccb_write(SLAVE_ID,0x370d,0x2);
		sccb_write(SLAVE_ID,0x370c,0xa0);
		sccb_write(SLAVE_ID,0x3602,0xfc);
		sccb_write(SLAVE_ID,0x3612,0xff);
		sccb_write(SLAVE_ID,0x3634,0xc0);
		sccb_write(SLAVE_ID,0x3613,0x0);
		sccb_write(SLAVE_ID,0x3605,0x4);
		sccb_write(SLAVE_ID,0x3621,0xc7);
		sccb_write(SLAVE_ID,0x3622,0x0);
		sccb_write(SLAVE_ID,0x3604,0x40);
		sccb_write(SLAVE_ID,0x3603,0x27);
		sccb_write(SLAVE_ID,0x3603,0x27);
		sccb_write(SLAVE_ID,0x4000,0x21);
		sccb_write(SLAVE_ID,0x401d,0x2);
		sccb_write(SLAVE_ID,0x3600,0x54);
		sccb_write(SLAVE_ID,0x3605,0x4);
		sccb_write(SLAVE_ID,0x3606,0x3f);
		sccb_write(SLAVE_ID,0x3c01,0x80);
		sccb_write(SLAVE_ID,0x5000,0x4f);
		sccb_write(SLAVE_ID,0x5020,0x4);
		sccb_write(SLAVE_ID,0x5181,0x79);
		sccb_write(SLAVE_ID,0x5182,0x0);
		sccb_write(SLAVE_ID,0x5185,0x22);
		sccb_write(SLAVE_ID,0x5197,0x1);
		sccb_write(SLAVE_ID,0x5001,0x7f);
		sccb_write(SLAVE_ID,0x5500,0xa);
		sccb_write(SLAVE_ID,0x5504,0x0);
		sccb_write(SLAVE_ID,0x5505,0x7f);
		sccb_write(SLAVE_ID,0x5080,0x8);
		sccb_write(SLAVE_ID,0x300e,0x18);
		sccb_write(SLAVE_ID,0x4610,0x0);
		sccb_write(SLAVE_ID,0x471d,0x5);
		sccb_write(SLAVE_ID,0x4708,0x6);
		sccb_write(SLAVE_ID,0x3710,0x10);
		sccb_write(SLAVE_ID,0x3632,0x41);
		sccb_write(SLAVE_ID,0x3702,0x10);
		sccb_write(SLAVE_ID,0x3620,0x52);
		sccb_write(SLAVE_ID,0x3631,0x1);
		sccb_write(SLAVE_ID,0x3808,0x2);
		sccb_write(SLAVE_ID,0x3809,0x80);
		sccb_write(SLAVE_ID,0x380a,0x1);
		sccb_write(SLAVE_ID,0x380b,0xe0);
		sccb_write(SLAVE_ID,0x380e,0x3);
		sccb_write(SLAVE_ID,0x380f,0xe8);
		sccb_write(SLAVE_ID,0x501f,0x0);
		sccb_write(SLAVE_ID,0x5000,0x4f);
		sccb_write(SLAVE_ID,0x4300,0x30);
		sccb_write(SLAVE_ID,0x3503,0x0);
		sccb_write(SLAVE_ID,0x3501,0x3e);
		sccb_write(SLAVE_ID,0x3502,0x40);
		sccb_write(SLAVE_ID,0x350b,0x12);
		sccb_write(SLAVE_ID,0x3503,0x0);
		sccb_write(SLAVE_ID,0x3824,0x11);
		sccb_write(SLAVE_ID,0x3501,0x3e);
		sccb_write(SLAVE_ID,0x3502,0x40);
		sccb_write(SLAVE_ID,0x350b,0x12);
		sccb_write(SLAVE_ID,0x380c,0xc);
		sccb_write(SLAVE_ID,0x380d,0x80);
		sccb_write(SLAVE_ID,0x380e,0x3);
		sccb_write(SLAVE_ID,0x380f,0xe8);
		sccb_write(SLAVE_ID,0x3a0d,0x8);
		sccb_write(SLAVE_ID,0x3a0e,0x6);
		sccb_write(SLAVE_ID,0x3818,0xc1);
		sccb_write(SLAVE_ID,0x3705,0xdb);
		sccb_write(SLAVE_ID,0x370a,0x81);
		sccb_write(SLAVE_ID,0x3801,0x50);
		sccb_write(SLAVE_ID,0x3621,0xc7);
		sccb_write(SLAVE_ID,0x3801,0x50);
		sccb_write(SLAVE_ID,0x3803,0x8);
		sccb_write(SLAVE_ID,0x3827,0x8);
		sccb_write(SLAVE_ID,0x3810,0xc0);
		sccb_write(SLAVE_ID,0x3804,0x5);
		sccb_write(SLAVE_ID,0x3805,0x0);
		sccb_write(SLAVE_ID,0x5682,0x5);
		sccb_write(SLAVE_ID,0x5683,0x0);
		sccb_write(SLAVE_ID,0x3806,0x3);
		sccb_write(SLAVE_ID,0x3807,0xc0);
		sccb_write(SLAVE_ID,0x5686,0x3);
		sccb_write(SLAVE_ID,0x5687,0xc0);
		sccb_write(SLAVE_ID,0x3a00,0x7c);
		sccb_write(SLAVE_ID,0x3a1a,0x4);
		sccb_write(SLAVE_ID,0x3a13,0x30);
		sccb_write(SLAVE_ID,0x3a18,0x0);
		sccb_write(SLAVE_ID,0x3a19,0x7c);
		sccb_write(SLAVE_ID,0x3a08,0x9);
		sccb_write(SLAVE_ID,0x3a09,0x60);
		sccb_write(SLAVE_ID,0x3a0a,0x7);
		sccb_write(SLAVE_ID,0x3a0b,0xd0);
		sccb_write(SLAVE_ID,0x3004,0xff);
		sccb_write(SLAVE_ID,0x350c,0x3);
		sccb_write(SLAVE_ID,0x350d,0xe8);
		sccb_write(SLAVE_ID,0x3500,0x0);
		sccb_write(SLAVE_ID,0x3501,0x3e);
		sccb_write(SLAVE_ID,0x3502,0x40);
		sccb_write(SLAVE_ID,0x350a,0x0);
		sccb_write(SLAVE_ID,0x350b,0x12);
		sccb_write(SLAVE_ID,0x3503,0x0);
		sccb_write(SLAVE_ID,0x528a,0x2);
		sccb_write(SLAVE_ID,0x528b,0x6);
		sccb_write(SLAVE_ID,0x528c,0x20);
		sccb_write(SLAVE_ID,0x528d,0x30);
		sccb_write(SLAVE_ID,0x528e,0x40);
		sccb_write(SLAVE_ID,0x528f,0x50);
		sccb_write(SLAVE_ID,0x5290,0x60);
		sccb_write(SLAVE_ID,0x5292,0x0);
		sccb_write(SLAVE_ID,0x5293,0x2);
		sccb_write(SLAVE_ID,0x5294,0x0);
		sccb_write(SLAVE_ID,0x5295,0x4);
		sccb_write(SLAVE_ID,0x5296,0x0);
		sccb_write(SLAVE_ID,0x5297,0x8);
		sccb_write(SLAVE_ID,0x5298,0x0);
		sccb_write(SLAVE_ID,0x5299,0x10);
		sccb_write(SLAVE_ID,0x529a,0x0);
		sccb_write(SLAVE_ID,0x529b,0x20);
		sccb_write(SLAVE_ID,0x529c,0x0);
		sccb_write(SLAVE_ID,0x529d,0x28);
		sccb_write(SLAVE_ID,0x529e,0x0);
		sccb_write(SLAVE_ID,0x529f,0x30);
		sccb_write(SLAVE_ID,0x3a0f,0x3c);
		sccb_write(SLAVE_ID,0x3a10,0x30);
		sccb_write(SLAVE_ID,0x3a1b,0x3c);
		sccb_write(SLAVE_ID,0x3a1e,0x30);
		sccb_write(SLAVE_ID,0x3a11,0x70);
		sccb_write(SLAVE_ID,0x3a1f,0x10);
		sccb_write(SLAVE_ID,0x3030,0x2b);
		sccb_write(SLAVE_ID,0x3a02,0x0);
		sccb_write(SLAVE_ID,0x3a03,0x7d);
		sccb_write(SLAVE_ID,0x3a04,0x0);
		sccb_write(SLAVE_ID,0x3a14,0x0);
		sccb_write(SLAVE_ID,0x3a15,0x7d);
		sccb_write(SLAVE_ID,0x3a16,0x0);
		sccb_write(SLAVE_ID,0x3a00,0x7c);
		sccb_write(SLAVE_ID,0x3a08,0x9);
		sccb_write(SLAVE_ID,0x3a09,0x60);
		sccb_write(SLAVE_ID,0x3a0a,0x7);
		sccb_write(SLAVE_ID,0x3a0b,0xd0);
		sccb_write(SLAVE_ID,0x3a0d,0x8);
		sccb_write(SLAVE_ID,0x3a0e,0x6);
		sccb_write(SLAVE_ID,0x5193,0x70);
		sccb_write(SLAVE_ID,0x3620,0x52);
		sccb_write(SLAVE_ID,0x3703,0xb2);
		sccb_write(SLAVE_ID,0x3704,0x18);
		sccb_write(SLAVE_ID,0x589b,0x4);
		sccb_write(SLAVE_ID,0x589a,0xc5);
		sccb_write(SLAVE_ID,0x528a,0x2);
		sccb_write(SLAVE_ID,0x528b,0x6);
		sccb_write(SLAVE_ID,0x528c,0x20);
		sccb_write(SLAVE_ID,0x528d,0x30);
		sccb_write(SLAVE_ID,0x528e,0x40);
		sccb_write(SLAVE_ID,0x528f,0x50);
		sccb_write(SLAVE_ID,0x5290,0x60);
		sccb_write(SLAVE_ID,0x5293,0x2);
		sccb_write(SLAVE_ID,0x5295,0x4);
		sccb_write(SLAVE_ID,0x5297,0x8);
		sccb_write(SLAVE_ID,0x5299,0x10);
		sccb_write(SLAVE_ID,0x529b,0x20);
		sccb_write(SLAVE_ID,0x529d,0x28);
		sccb_write(SLAVE_ID,0x529f,0x30);
		sccb_write(SLAVE_ID,0x5300,0x0);
		sccb_write(SLAVE_ID,0x5301,0x20);
		sccb_write(SLAVE_ID,0x5302,0x0);
		sccb_write(SLAVE_ID,0x5303,0x7c);
		sccb_write(SLAVE_ID,0x530c,0x0);
		sccb_write(SLAVE_ID,0x530d,0xc);
		sccb_write(SLAVE_ID,0x530e,0x20);
		sccb_write(SLAVE_ID,0x530f,0x80);
		sccb_write(SLAVE_ID,0x5310,0x20);
		sccb_write(SLAVE_ID,0x5311,0x80);
		sccb_write(SLAVE_ID,0x5380,0x1);
		sccb_write(SLAVE_ID,0x5381,0x0);
		sccb_write(SLAVE_ID,0x5382,0x0);
		sccb_write(SLAVE_ID,0x5383,0x4e);
		sccb_write(SLAVE_ID,0x5384,0x0);
		sccb_write(SLAVE_ID,0x5385,0xf);
		sccb_write(SLAVE_ID,0x5386,0x0);
		sccb_write(SLAVE_ID,0x5387,0x0);
		sccb_write(SLAVE_ID,0x5388,0x1);
		sccb_write(SLAVE_ID,0x5389,0x15);
		sccb_write(SLAVE_ID,0x538a,0x0);
		sccb_write(SLAVE_ID,0x538b,0x31);
		sccb_write(SLAVE_ID,0x538c,0x0);
		sccb_write(SLAVE_ID,0x538d,0x0);
		sccb_write(SLAVE_ID,0x538e,0x0);
		sccb_write(SLAVE_ID,0x538f,0xf);
		sccb_write(SLAVE_ID,0x5390,0x0);
		sccb_write(SLAVE_ID,0x5391,0xab);
		sccb_write(SLAVE_ID,0x5392,0x0);
		sccb_write(SLAVE_ID,0x5393,0xa2);
		sccb_write(SLAVE_ID,0x5394,0x8);
		sccb_write(SLAVE_ID,0x5480,0x14);
		sccb_write(SLAVE_ID,0x5481,0x21);
		sccb_write(SLAVE_ID,0x5482,0x36);
		sccb_write(SLAVE_ID,0x5483,0x57);
		sccb_write(SLAVE_ID,0x5484,0x65);
		sccb_write(SLAVE_ID,0x5485,0x71);
		sccb_write(SLAVE_ID,0x5486,0x7d);
		sccb_write(SLAVE_ID,0x5487,0x87);
		sccb_write(SLAVE_ID,0x5488,0x91);
		sccb_write(SLAVE_ID,0x5489,0x9a);
		sccb_write(SLAVE_ID,0x548a,0xaa);
		sccb_write(SLAVE_ID,0x548b,0xb8);
		sccb_write(SLAVE_ID,0x548c,0xcd);
		sccb_write(SLAVE_ID,0x548d,0xdd);
		sccb_write(SLAVE_ID,0x548e,0xea);
		sccb_write(SLAVE_ID,0x548f,0x10);
		sccb_write(SLAVE_ID,0x5490,0x5);
		sccb_write(SLAVE_ID,0x5491,0x0);
		sccb_write(SLAVE_ID,0x5492,0x4);
		sccb_write(SLAVE_ID,0x5493,0x20);
		sccb_write(SLAVE_ID,0x5494,0x3);
		sccb_write(SLAVE_ID,0x5495,0x60);
		sccb_write(SLAVE_ID,0x5496,0x2);
		sccb_write(SLAVE_ID,0x5497,0xb8);
		sccb_write(SLAVE_ID,0x5498,0x2);
		sccb_write(SLAVE_ID,0x5499,0x86);
		sccb_write(SLAVE_ID,0x549a,0x2);
		sccb_write(SLAVE_ID,0x549b,0x5b);
		sccb_write(SLAVE_ID,0x549c,0x2);
		sccb_write(SLAVE_ID,0x549d,0x3b);
		sccb_write(SLAVE_ID,0x549e,0x2);
		sccb_write(SLAVE_ID,0x549f,0x1c);
		sccb_write(SLAVE_ID,0x54a0,0x2);
		sccb_write(SLAVE_ID,0x54a1,0x4);
		sccb_write(SLAVE_ID,0x54a2,0x1);
		sccb_write(SLAVE_ID,0x54a3,0xed);
		sccb_write(SLAVE_ID,0x54a4,0x1);
		sccb_write(SLAVE_ID,0x54a5,0xc5);
		sccb_write(SLAVE_ID,0x54a6,0x1);
		sccb_write(SLAVE_ID,0x54a7,0xa5);
		sccb_write(SLAVE_ID,0x54a8,0x1);
		sccb_write(SLAVE_ID,0x54a9,0x6c);
		sccb_write(SLAVE_ID,0x54aa,0x1);
		sccb_write(SLAVE_ID,0x54ab,0x41);
		sccb_write(SLAVE_ID,0x54ac,0x1);
		sccb_write(SLAVE_ID,0x54ad,0x20);
		sccb_write(SLAVE_ID,0x54ae,0x0);
		sccb_write(SLAVE_ID,0x54af,0x16);
		sccb_write(SLAVE_ID,0x3633,0x7);
		sccb_write(SLAVE_ID,0x3702,0x10);
		sccb_write(SLAVE_ID,0x3703,0xb2);
		sccb_write(SLAVE_ID,0x3704,0x18);
		sccb_write(SLAVE_ID,0x370b,0x40);
		sccb_write(SLAVE_ID,0x370d,0x2);
		sccb_write(SLAVE_ID,0x3620,0x52);

	}
	if((nWidthH == 2592) && (nWidthV == 1944))
	{
		//2592X1944
		sccb_write(SLAVE_ID,0x3103,0x03);
		sccb_write(SLAVE_ID,0x3008,0x82);
		sccb_write(SLAVE_ID,0x3017,0x7f);
		sccb_write(SLAVE_ID,0x3018,0xfc);
		sccb_write(SLAVE_ID,0x3810,0xc2);
		sccb_write(SLAVE_ID,0x3615,0xf0);
		sccb_write(SLAVE_ID,0x3000,0x00);
		sccb_write(SLAVE_ID,0x3001,0x00);
		sccb_write(SLAVE_ID,0x3002,0x00);
		sccb_write(SLAVE_ID,0x3003,0x00);
		sccb_write(SLAVE_ID,0x3030,0x2b);
		sccb_write(SLAVE_ID,0x3011,0x08);
		sccb_write(SLAVE_ID,0x3010,0x70);
		sccb_write(SLAVE_ID,0x3604,0x60);
		sccb_write(SLAVE_ID,0x3622,0x08);
		sccb_write(SLAVE_ID,0x3621,0x27);
		sccb_write(SLAVE_ID,0x3709,0x00);
		sccb_write(SLAVE_ID,0x4000,0x21);
		sccb_write(SLAVE_ID,0x401d,0x02);
		sccb_write(SLAVE_ID,0x3600,0x54);
		sccb_write(SLAVE_ID,0x3605,0x04);
		sccb_write(SLAVE_ID,0x3606,0x3f);
		sccb_write(SLAVE_ID,0x3c01,0x80);
		sccb_write(SLAVE_ID,0x300d,0x21);
		sccb_write(SLAVE_ID,0x3623,0x22);
		sccb_write(SLAVE_ID,0x5000,0x4f);
		sccb_write(SLAVE_ID,0x5020,0x04);
		sccb_write(SLAVE_ID,0x5181,0x79);
		sccb_write(SLAVE_ID,0x5182,0x00);
		sccb_write(SLAVE_ID,0x5185,0x22);
		sccb_write(SLAVE_ID,0x5197,0x01);
		sccb_write(SLAVE_ID,0x5500,0x0a);
		sccb_write(SLAVE_ID,0x5504,0x00);
		sccb_write(SLAVE_ID,0x5505,0x7f);
		sccb_write(SLAVE_ID,0x5080,0x08);
		sccb_write(SLAVE_ID,0x300e,0x18);
		sccb_write(SLAVE_ID,0x4610,0x00);
		sccb_write(SLAVE_ID,0x471d,0x05);
		sccb_write(SLAVE_ID,0x4708,0x06);
		sccb_write(SLAVE_ID,0x3710,0x10);
		sccb_write(SLAVE_ID,0x370d,0x0e);
		sccb_write(SLAVE_ID,0x3632,0x41);
		sccb_write(SLAVE_ID,0x3702,0x40);
		sccb_write(SLAVE_ID,0x3620,0x37);
		sccb_write(SLAVE_ID,0x3631,0x01);
		sccb_write(SLAVE_ID,0x370c,0xa0);
		sccb_write(SLAVE_ID,0x3808,0x0a);
		sccb_write(SLAVE_ID,0x3809,0x20);
		sccb_write(SLAVE_ID,0x380a,0x07);
		sccb_write(SLAVE_ID,0x380b,0x98);
		sccb_write(SLAVE_ID,0x380c,0x0c);
		sccb_write(SLAVE_ID,0x380d,0x80);
		sccb_write(SLAVE_ID,0x380e,0x07);
		sccb_write(SLAVE_ID,0x380f,0xd0);
		sccb_write(SLAVE_ID,0x3801,0x8a);
		sccb_write(SLAVE_ID,0x501f,0x00);
		sccb_write(SLAVE_ID,0x5000,0x4f);
		sccb_write(SLAVE_ID,0x5001,0x4f);
		sccb_write(SLAVE_ID,0x4300,0x30);
		sccb_write(SLAVE_ID,0x4300,0x30);
		sccb_write(SLAVE_ID,0x460b,0x35);
		sccb_write(SLAVE_ID,0x471d,0x00);
		sccb_write(SLAVE_ID,0x3002,0x0c);
		sccb_write(SLAVE_ID,0x3002,0x00);
		sccb_write(SLAVE_ID,0x4713,0x03);
		sccb_write(SLAVE_ID,0x471c,0x50);
		sccb_write(SLAVE_ID,0x460c,0x22);
		sccb_write(SLAVE_ID,0x3815,0x44);
		sccb_write(SLAVE_ID,0x3503,0x07);
		sccb_write(SLAVE_ID,0x3501,0x73);
		sccb_write(SLAVE_ID,0x3502,0x80);
		sccb_write(SLAVE_ID,0x350b,0x00);
		sccb_write(SLAVE_ID,0x3818,0xc8);
		sccb_write(SLAVE_ID,0x3621,0x27);
		sccb_write(SLAVE_ID,0x3801,0x8a);
		sccb_write(SLAVE_ID,0x3a00,0x78);
		sccb_write(SLAVE_ID,0x3a1a,0x04);
		sccb_write(SLAVE_ID,0x3a13,0x30);
		sccb_write(SLAVE_ID,0x3a18,0x00);
		sccb_write(SLAVE_ID,0x3a19,0x7c);
		sccb_write(SLAVE_ID,0x3a08,0x12);
		sccb_write(SLAVE_ID,0x3a09,0xc0);
		sccb_write(SLAVE_ID,0x3a0a,0x0f);
		sccb_write(SLAVE_ID,0x3a0b,0xa0);
		sccb_write(SLAVE_ID,0x3004,0xff);
		sccb_write(SLAVE_ID,0x350c,0x07);
		sccb_write(SLAVE_ID,0x350d,0xd0);
		sccb_write(SLAVE_ID,0x3a0d,0x08);
		sccb_write(SLAVE_ID,0x3a0e,0x06);
		sccb_write(SLAVE_ID,0x3500,0x00);
		sccb_write(SLAVE_ID,0x3501,0x00);
		sccb_write(SLAVE_ID,0x3502,0x00);
		sccb_write(SLAVE_ID,0x350a,0x00);
		sccb_write(SLAVE_ID,0x350b,0x00);
		sccb_write(SLAVE_ID,0x3503,0x00);
		sccb_write(SLAVE_ID,0x3a0f,0x3c);
		sccb_write(SLAVE_ID,0x3a10,0x32);
		sccb_write(SLAVE_ID,0x3a1b,0x3c);
		sccb_write(SLAVE_ID,0x3a1e,0x32);
		sccb_write(SLAVE_ID,0x3a11,0x80);
		sccb_write(SLAVE_ID,0x3a1f,0x20);
		sccb_write(SLAVE_ID,0x3030,0x2b);
		sccb_write(SLAVE_ID,0x3a02,0x00);
		sccb_write(SLAVE_ID,0x3a03,0x7d);
		sccb_write(SLAVE_ID,0x3a04,0x00);
		sccb_write(SLAVE_ID,0x3a14,0x00);
		sccb_write(SLAVE_ID,0x3a15,0x7d);
		sccb_write(SLAVE_ID,0x3a16,0x00);
		sccb_write(SLAVE_ID,0x3a00,0x78);
		sccb_write(SLAVE_ID,0x3a08,0x09);
		sccb_write(SLAVE_ID,0x3a09,0x60);
		sccb_write(SLAVE_ID,0x3a0a,0x07);
		sccb_write(SLAVE_ID,0x3a0b,0xd0);
		sccb_write(SLAVE_ID,0x3a0d,0x10);
		sccb_write(SLAVE_ID,0x3a0e,0x0d);
		sccb_write(SLAVE_ID,0x4407,0x04);
		sccb_write(SLAVE_ID,0x5193,0x70);
		sccb_write(SLAVE_ID,0x3620,0x57);
		sccb_write(SLAVE_ID,0x3703,0x98);
		sccb_write(SLAVE_ID,0x3704,0x1c);
		sccb_write(SLAVE_ID,0x589b,0x00);
		sccb_write(SLAVE_ID,0x589a,0xc0);
		sccb_write(SLAVE_ID,0x528a,0x02);
		sccb_write(SLAVE_ID,0x528b,0x06);
		sccb_write(SLAVE_ID,0x528c,0x20);
		sccb_write(SLAVE_ID,0x528d,0x30);
		sccb_write(SLAVE_ID,0x528e,0x40);
		sccb_write(SLAVE_ID,0x528f,0x50);
		sccb_write(SLAVE_ID,0x5290,0x60);
		sccb_write(SLAVE_ID,0x5293,0x02);
		sccb_write(SLAVE_ID,0x5295,0x04);
		sccb_write(SLAVE_ID,0x5297,0x08);
		sccb_write(SLAVE_ID,0x5299,0x10);
		sccb_write(SLAVE_ID,0x529b,0x20);
		sccb_write(SLAVE_ID,0x529d,0x28);
		sccb_write(SLAVE_ID,0x529f,0x30);
		sccb_write(SLAVE_ID,0x5300,0x00);
		sccb_write(SLAVE_ID,0x5301,0x20);
		sccb_write(SLAVE_ID,0x5302,0x00);
		sccb_write(SLAVE_ID,0x5303,0x7c);
		sccb_write(SLAVE_ID,0x530c,0x00);
		sccb_write(SLAVE_ID,0x530d,0x0c);
		sccb_write(SLAVE_ID,0x530e,0x20);
		sccb_write(SLAVE_ID,0x530f,0x80);
		sccb_write(SLAVE_ID,0x5310,0x20);
		sccb_write(SLAVE_ID,0x5311,0x80);
		sccb_write(SLAVE_ID,0x5380,0x01);
		sccb_write(SLAVE_ID,0x5381,0x00);
		sccb_write(SLAVE_ID,0x5382,0x00);
		sccb_write(SLAVE_ID,0x5383,0x4e);
		sccb_write(SLAVE_ID,0x5384,0x00);
		sccb_write(SLAVE_ID,0x5385,0x0f);
		sccb_write(SLAVE_ID,0x5386,0x00);
		sccb_write(SLAVE_ID,0x5387,0x00);
		sccb_write(SLAVE_ID,0x5388,0x01);
		sccb_write(SLAVE_ID,0x5389,0x15);
		sccb_write(SLAVE_ID,0x538a,0x00);
		sccb_write(SLAVE_ID,0x538b,0x31);
		sccb_write(SLAVE_ID,0x538c,0x00);
		sccb_write(SLAVE_ID,0x538d,0x00);
		sccb_write(SLAVE_ID,0x538e,0x00);
		sccb_write(SLAVE_ID,0x538f,0x0f);
		sccb_write(SLAVE_ID,0x5390,0x00);
		sccb_write(SLAVE_ID,0x5391,0xab);
		sccb_write(SLAVE_ID,0x5392,0x00);
		sccb_write(SLAVE_ID,0x5393,0xa2);
		sccb_write(SLAVE_ID,0x5394,0x08);
		sccb_write(SLAVE_ID,0x5480,0x14);
		sccb_write(SLAVE_ID,0x5481,0x21);
		sccb_write(SLAVE_ID,0x5482,0x36);
		sccb_write(SLAVE_ID,0x5483,0x57);
		sccb_write(SLAVE_ID,0x5484,0x65);
		sccb_write(SLAVE_ID,0x5485,0x71);
		sccb_write(SLAVE_ID,0x5486,0x7d);
		sccb_write(SLAVE_ID,0x5487,0x87);
		sccb_write(SLAVE_ID,0x5488,0x91);
		sccb_write(SLAVE_ID,0x5489,0x9a);
		sccb_write(SLAVE_ID,0x548a,0xaa);
		sccb_write(SLAVE_ID,0x548b,0xb8);
		sccb_write(SLAVE_ID,0x548c,0xcd);
		sccb_write(SLAVE_ID,0x548d,0xdd);
		sccb_write(SLAVE_ID,0x548e,0xea);
		sccb_write(SLAVE_ID,0x548f,0x10);
		sccb_write(SLAVE_ID,0x5490,0x05);
		sccb_write(SLAVE_ID,0x5491,0x00);
		sccb_write(SLAVE_ID,0x5492,0x04);
		sccb_write(SLAVE_ID,0x5493,0x20);
		sccb_write(SLAVE_ID,0x5494,0x03);
		sccb_write(SLAVE_ID,0x5495,0x60);
		sccb_write(SLAVE_ID,0x5496,0x02);
		sccb_write(SLAVE_ID,0x5497,0xb8);
		sccb_write(SLAVE_ID,0x5498,0x02);
		sccb_write(SLAVE_ID,0x5499,0x86);
		sccb_write(SLAVE_ID,0x549a,0x02);
		sccb_write(SLAVE_ID,0x549b,0x5b);
		sccb_write(SLAVE_ID,0x549c,0x02);
		sccb_write(SLAVE_ID,0x549d,0x3b);
		sccb_write(SLAVE_ID,0x549e,0x02);
		sccb_write(SLAVE_ID,0x549f,0x1c);
		sccb_write(SLAVE_ID,0x54a0,0x02);
		sccb_write(SLAVE_ID,0x54a1,0x04);
		sccb_write(SLAVE_ID,0x54a2,0x01);
		sccb_write(SLAVE_ID,0x54a3,0xed);
		sccb_write(SLAVE_ID,0x54a4,0x01);
		sccb_write(SLAVE_ID,0x54a5,0xc5);
		sccb_write(SLAVE_ID,0x54a6,0x01);
		sccb_write(SLAVE_ID,0x54a7,0xa5);
		sccb_write(SLAVE_ID,0x54a8,0x01);
		sccb_write(SLAVE_ID,0x54a9,0x6c);
		sccb_write(SLAVE_ID,0x54aa,0x01);
		sccb_write(SLAVE_ID,0x54ab,0x41);
		sccb_write(SLAVE_ID,0x54ac,0x01);
		sccb_write(SLAVE_ID,0x54ad,0x20);
		sccb_write(SLAVE_ID,0x54ae,0x00);
		sccb_write(SLAVE_ID,0x54af,0x16);
		sccb_write(SLAVE_ID,0x3633,0x07);
		sccb_write(SLAVE_ID,0x3702,0x10);
		sccb_write(SLAVE_ID,0x3703,0xb2);
		sccb_write(SLAVE_ID,0x3704,0x18);
		sccb_write(SLAVE_ID,0x370b,0x40);
		sccb_write(SLAVE_ID,0x370d,0x02);
		sccb_write(SLAVE_ID,0x3620,0x52);

		//;YUV
		sccb_write(SLAVE_ID,0x3002,0x1c);
		sccb_write(SLAVE_ID,0x460c,0x20);
		sccb_write(SLAVE_ID,0x471c,0xd0);
		sccb_write(SLAVE_ID,0x3815,0x01);
		sccb_write(SLAVE_ID,0x501f,0x00);
		sccb_write(SLAVE_ID,0x5002,0xe0);
		sccb_write(SLAVE_ID,0x4300,0x30);
		sccb_write(SLAVE_ID,0x3818,0xc0);
		sccb_write(SLAVE_ID,0x3810,0xc2);

		//;auto sharp+1
		sccb_write(SLAVE_ID,0x530c,0x4);
		sccb_write(SLAVE_ID,0x530d,0x18);
		sccb_write(SLAVE_ID,0x5312,0x20);

		//;@@ Auto -1
		//;de-noise Y
		sccb_write(SLAVE_ID,0x528a,0x00);
		sccb_write(SLAVE_ID,0x528b,0x02);
		sccb_write(SLAVE_ID,0x528c,0x08);
		sccb_write(SLAVE_ID,0x528d,0x10);
		sccb_write(SLAVE_ID,0x528e,0x20);
		sccb_write(SLAVE_ID,0x528f,0x28);
		sccb_write(SLAVE_ID,0x5290,0x30);
		//;de-noise UV
		sccb_write(SLAVE_ID,0x5292,0x00);
		sccb_write(SLAVE_ID,0x5293,0x00);
		sccb_write(SLAVE_ID,0x5294,0x00);
		sccb_write(SLAVE_ID,0x5295,0x02);
		sccb_write(SLAVE_ID,0x5296,0x00);
		sccb_write(SLAVE_ID,0x5297,0x08);
		sccb_write(SLAVE_ID,0x5298,0x00);
		sccb_write(SLAVE_ID,0x5299,0x10);
		sccb_write(SLAVE_ID,0x529a,0x00);
		sccb_write(SLAVE_ID,0x529b,0x20);
		sccb_write(SLAVE_ID,0x529c,0x00);
		sccb_write(SLAVE_ID,0x529d,0x28);
		sccb_write(SLAVE_ID,0x529e,0x00);
		sccb_write(SLAVE_ID,0x529f,0x30);
		sccb_write(SLAVE_ID,0x5282,0x00);

		//;de-noise cip
		sccb_write(SLAVE_ID,0x5304,0x00);
		sccb_write(SLAVE_ID,0x5305,0x00);
		sccb_write(SLAVE_ID,0x5314,0x03);
		sccb_write(SLAVE_ID,0x5315,0x04);
		sccb_write(SLAVE_ID,0x5319,0x02);
	}
	//V Flip & Mirror
	sccb_write(SLAVE_ID, 0x3818, 0xa1);	//VFlip set/clear 0x3818[5]
	sccb_write(SLAVE_ID, 0x3621, 0xe7);	//Mirror
	R_CSI_TG_CTRL1 = uCtrlReg2;					//*P_Sensor_TG_Ctrl2 = uCtrlReg2;
	R_CSI_TG_CTRL0 = uCtrlReg1;					//*P_Sensor_TG_Ctrl1 = uCtrlReg1;

}
#endif


#ifdef	__OV7675_DRV_C__

//====================================================================================================
//	Description:	OV7675 Initialization
//	Syntax:			void OV7675_Init (
//						INT16S nWidthH,			// Active H Width
//						INT16S nWidthV,			// Active V Width
//						INT16U uFlag				// Flag Type
//					);
//	Return:			None
//====================================================================================================
void OV7675_Init (
	INT16S nWidthH,			// Active H Width
	INT16S nWidthV,			// Active V Width
	INT16U uFlag				// Flag Type
) {
	INT16U uCtrlReg1, uCtrlReg2;
	INT32U SPI_Ctrl_temp;
	INT16S nReso;

	csi_fifo_flag = 1;

	// Enable CSI clock to let sensor initialize at first
#if CSI_CLOCK == CSI_CLOCK_27MHZ
	uCtrlReg2 = CLKOEN | CSI_RGB565 |CLK_SEL27M | CSI_HIGHPRI | CSI_NOSTOP;
#else
	uCtrlReg2 = CLKOEN | CSI_RGB565 |CLK_SEL48M | CSI_HIGHPRI | CSI_NOSTOP;
#endif

	uCtrlReg1 = CSIEN | YUV_YUYV | CAP;									// Default CSI Control Register 1
	if (uFlag & FT_CSI_RGB1555)											// RGB1555
	{
		uCtrlReg2 |= CSI_RGB1555;
	}
	if (uFlag & FT_CSI_CCIR656)										// CCIR656?
	{
		uCtrlReg1 |= CCIR656 | VADD_FALL | VRST_FALL | HRST_FALL;	// CCIR656
		uCtrlReg2 |= D_TYPE1;										// CCIR656
	}
	else
	{
		uCtrlReg1 |= VADD_RISE | VRST_FALL | HRST_RISE | HREF;		// NOT CCIR656
		uCtrlReg2 |= D_TYPE0;										// NOT CCIR656
	}
	if (uFlag & FT_CSI_YUVIN)										// YUVIN?
	{
		uCtrlReg1 |= YUVIN;
	}
	if (uFlag & FT_CSI_YUVOUT)										// YUVOUT?
	{
		uCtrlReg1 |= YUVOUT;
	}

	// Whether compression or not?
	nReso = ((nWidthH == 320) && (nWidthV == 240)) ? 1 : 0;
	if (nReso == 1)								// VGA
	{
#ifdef	__TV_QVGA__
		R_CSI_TG_HRATIO = 0x0102;					// Scale to 1/2
		R_CSI_TG_VRATIO = 0x0102;					// Scale to 1/2
		R_CSI_TG_HWIDTH = nWidthH;					// Horizontal frame width
		R_CSI_TG_VHEIGHT = nWidthV*2;				// Vertical frame width
#endif	// __TV_QVGA__
	}
	else
	{
		R_CSI_TG_HRATIO = 0;
		R_CSI_TG_VRATIO = 0;
	}

	R_CSI_TG_VL0START = 0x0000;						// Sensor field 0 vertical latch start register.
	R_CSI_TG_VL1START = 0x0000;						//*P_Sensor_TG_V_L1Start = 0x0000;
	R_CSI_TG_HSTART = 0x0000;						// Sensor horizontal start register.

	R_CSI_TG_CTRL0 = 0;								//reset control0
	R_CSI_TG_CTRL1 = CSI_NOSTOP|CLKOEN;				//enable CSI CLKO
	drv_msec_wait(100); 							//wait 100ms for CLKO stable

	SPI_Ctrl_temp = R_SPI0_CTRL;
	R_SPI0_CTRL = 0;

	// CMOS Sensor Initialization Start...
	sccb_init (SCCB_SCL, SCCB_SDA);
	sccb_delay (200);

	if(nWidthH == 320 && nWidthV == 240)
	{
		//QVGA, YUV, 27Mhz, 60fps
		sccb_write(SLAVE_ID, 0x09, 0x10);
		sccb_write(SLAVE_ID, 0xC1, 0x7F);
#if CSI_FPS	== CSI_15FPS
		sccb_write(SLAVE_ID, 0x11, 0x82);   // 15fps
#elif CSI_FPS == CSI_30FPS
		sccb_write(SLAVE_ID, 0x11, 0x81);	// 30fps
#else		
		sccb_write(SLAVE_ID, 0x11, 0x80);	// 60fps
#endif		
		sccb_write(SLAVE_ID, 0x3A, 0x0C);
		sccb_write(SLAVE_ID, 0x3D, 0xC0);
		sccb_write(SLAVE_ID, 0x12, 0x00);
		sccb_write(SLAVE_ID, 0x15, 0x40);
		sccb_write(SLAVE_ID, 0x17, 0x13);
		sccb_write(SLAVE_ID, 0x18, 0x01);
		sccb_write(SLAVE_ID, 0x32, 0xBF);
		sccb_write(SLAVE_ID, 0x19, 0x02);
		sccb_write(SLAVE_ID, 0x1A, 0x7A);
		sccb_write(SLAVE_ID, 0x03, 0x0A);
		sccb_write(SLAVE_ID, 0x0C, 0x00);
		sccb_write(SLAVE_ID, 0x3E, 0x00);
		sccb_write(SLAVE_ID, 0x70, 0x3A);
		sccb_write(SLAVE_ID, 0x71, 0x35);
		sccb_write(SLAVE_ID, 0x72, 0x11);
		sccb_write(SLAVE_ID, 0x73, 0xF0);
		                     
		sccb_write(SLAVE_ID, 0xA2, 0x02);
		sccb_write(SLAVE_ID, 0x7A, 0x20);
		sccb_write(SLAVE_ID, 0x7B, 0x03);
		sccb_write(SLAVE_ID, 0x7C, 0x0A);
		sccb_write(SLAVE_ID, 0x7D, 0x1A);
		sccb_write(SLAVE_ID, 0x7E, 0x3F);
		sccb_write(SLAVE_ID, 0x7F, 0x4E);
		sccb_write(SLAVE_ID, 0x80, 0x5B);
		sccb_write(SLAVE_ID, 0x81, 0x68);
		sccb_write(SLAVE_ID, 0x82, 0x75);
		sccb_write(SLAVE_ID, 0x83, 0x7F);
		sccb_write(SLAVE_ID, 0x84, 0x89);
		sccb_write(SLAVE_ID, 0x85, 0x9A);
		sccb_write(SLAVE_ID, 0x86, 0xA6);
		sccb_write(SLAVE_ID, 0x87, 0xBD);
		sccb_write(SLAVE_ID, 0x88, 0xD3);
		sccb_write(SLAVE_ID, 0x89, 0xE8);
		sccb_write(SLAVE_ID, 0x13, 0xE0);
		sccb_write(SLAVE_ID, 0x00, 0x00);
		sccb_write(SLAVE_ID, 0x10, 0x00);
		sccb_write(SLAVE_ID, 0x0D, 0x40);
		sccb_write(SLAVE_ID, 0x14, 0x28);
		sccb_write(SLAVE_ID, 0xA5, 0x02);
		sccb_write(SLAVE_ID, 0xAB, 0x02);
		sccb_write(SLAVE_ID, 0x24, 0x68);
		sccb_write(SLAVE_ID, 0x25, 0x58);
		sccb_write(SLAVE_ID, 0x26, 0xC2);
		sccb_write(SLAVE_ID, 0x9F, 0x78);
		sccb_write(SLAVE_ID, 0xA0, 0x68);
		sccb_write(SLAVE_ID, 0xA1, 0x03);
		                     
		sccb_write(SLAVE_ID, 0xA6, 0xD8);
		sccb_write(SLAVE_ID, 0xA7, 0xD8);
		sccb_write(SLAVE_ID, 0xA8, 0xF0);
		sccb_write(SLAVE_ID, 0xA9, 0x90);
		sccb_write(SLAVE_ID, 0xAA, 0x14);
		sccb_write(SLAVE_ID, 0x13, 0xE5);
		sccb_write(SLAVE_ID, 0x0E, 0x61);
		sccb_write(SLAVE_ID, 0x0F, 0x4B);
		sccb_write(SLAVE_ID, 0x16, 0x02);
		sccb_write(SLAVE_ID, 0x1e, 0x37); 	// Flip & Mirror
		sccb_write(SLAVE_ID, 0x21, 0x02);
		sccb_write(SLAVE_ID, 0x22, 0x91);
		sccb_write(SLAVE_ID, 0x29, 0x07);
		sccb_write(SLAVE_ID, 0x33, 0x0B);
		sccb_write(SLAVE_ID, 0x35, 0x0B);
		sccb_write(SLAVE_ID, 0x37, 0x1D);
		sccb_write(SLAVE_ID, 0x38, 0x71);
		sccb_write(SLAVE_ID, 0x39, 0x2A);
		sccb_write(SLAVE_ID, 0x3C, 0x78);
		sccb_write(SLAVE_ID, 0x4D, 0x40);
		sccb_write(SLAVE_ID, 0x4E, 0x20);
		sccb_write(SLAVE_ID, 0x69, 0x00);
		sccb_write(SLAVE_ID, 0x6B, 0x0A);
		sccb_write(SLAVE_ID, 0x74, 0x10);
		sccb_write(SLAVE_ID, 0x8D, 0x4F);
		sccb_write(SLAVE_ID, 0x8E, 0x00);
		sccb_write(SLAVE_ID, 0x8F, 0x00);
		sccb_write(SLAVE_ID, 0x90, 0x00);
		sccb_write(SLAVE_ID, 0x91, 0x00);
		sccb_write(SLAVE_ID, 0x96, 0x00);
		                    
		sccb_write(SLAVE_ID, 0x9A, 0x80);
		sccb_write(SLAVE_ID, 0xB0, 0x84);
		sccb_write(SLAVE_ID, 0xB1, 0x0C);
		sccb_write(SLAVE_ID, 0xB2, 0x0E);
		sccb_write(SLAVE_ID, 0xB3, 0x82);
		sccb_write(SLAVE_ID, 0xB8, 0x10); //0x08 for 1.8 volts, 0x10 for 2.8 volts to DOVDD	
		sccb_write(SLAVE_ID, 0x43, 0x0A);
		sccb_write(SLAVE_ID, 0x44, 0xF2);
		sccb_write(SLAVE_ID, 0x45, 0x39);
		sccb_write(SLAVE_ID, 0x46, 0x62);
		sccb_write(SLAVE_ID, 0x47, 0x3D);
		sccb_write(SLAVE_ID, 0x48, 0x55);
		sccb_write(SLAVE_ID, 0x59, 0x83);
		sccb_write(SLAVE_ID, 0x5A, 0x0D);
		sccb_write(SLAVE_ID, 0x5B, 0xCD);
		sccb_write(SLAVE_ID, 0x5C, 0x8C);
		sccb_write(SLAVE_ID, 0x5D, 0x77);
		sccb_write(SLAVE_ID, 0x5E, 0x16);
		sccb_write(SLAVE_ID, 0x6C, 0x0A);
		sccb_write(SLAVE_ID, 0x6D, 0x65);
		sccb_write(SLAVE_ID, 0x6E, 0x11);
		sccb_write(SLAVE_ID, 0x6F, 0x9E);
		sccb_write(SLAVE_ID, 0x6A, 0x40);
		sccb_write(SLAVE_ID, 0x01, 0x56);
		sccb_write(SLAVE_ID, 0x02, 0x44);
		sccb_write(SLAVE_ID, 0x13, 0xE7);
		sccb_write(SLAVE_ID, 0x4F, 0x88);
		sccb_write(SLAVE_ID, 0x50, 0x8B);
		sccb_write(SLAVE_ID, 0x51, 0x04);
		sccb_write(SLAVE_ID, 0x52, 0x11);
		sccb_write(SLAVE_ID, 0x53, 0x8C);
		sccb_write(SLAVE_ID, 0x54, 0x9D);
		                     
		sccb_write(SLAVE_ID, 0x55, 0x00);
		sccb_write(SLAVE_ID, 0x56, 0x40);
		sccb_write(SLAVE_ID, 0x57, 0x80);
		sccb_write(SLAVE_ID, 0x58, 0x9A);
		sccb_write(SLAVE_ID, 0x41, 0x08);
		sccb_write(SLAVE_ID, 0x3F, 0x00);
		sccb_write(SLAVE_ID, 0x75, 0x04);
		sccb_write(SLAVE_ID, 0x76, 0x60);
		sccb_write(SLAVE_ID, 0x4C, 0x00);
		sccb_write(SLAVE_ID, 0x77, 0x01);
		sccb_write(SLAVE_ID, 0x3D, 0xC2);
		sccb_write(SLAVE_ID, 0x4D, 0x09);
		sccb_write(SLAVE_ID, 0xC9, 0x30);
		sccb_write(SLAVE_ID, 0x41, 0x38);
		sccb_write(SLAVE_ID, 0x56, 0x40);
		sccb_write(SLAVE_ID, 0x34, 0x11);
		sccb_write(SLAVE_ID, 0x3B, 0x12);
		sccb_write(SLAVE_ID, 0xA4, 0x88);
		sccb_write(SLAVE_ID, 0x96, 0x00);
		sccb_write(SLAVE_ID, 0x97, 0x30);
		sccb_write(SLAVE_ID, 0x98, 0x20);
		sccb_write(SLAVE_ID, 0x99, 0x30);
		sccb_write(SLAVE_ID, 0x9A, 0x84);
		sccb_write(SLAVE_ID, 0x9B, 0x29);
		sccb_write(SLAVE_ID, 0x9C, 0x03);
		sccb_write(SLAVE_ID, 0x9D, 0x99);
		sccb_write(SLAVE_ID, 0x9E, 0x7F);
		                     
		sccb_write(SLAVE_ID, 0x78, 0x04);
		sccb_write(SLAVE_ID, 0x79, 0x01);
		sccb_write(SLAVE_ID, 0xC8, 0xF0);
		sccb_write(SLAVE_ID, 0x79, 0x0F);
		sccb_write(SLAVE_ID, 0xC8, 0x00);
		sccb_write(SLAVE_ID, 0x79, 0x10);
		sccb_write(SLAVE_ID, 0xC8, 0x7E);
		sccb_write(SLAVE_ID, 0x79, 0x0A);
		sccb_write(SLAVE_ID, 0xC8, 0x80);
		sccb_write(SLAVE_ID, 0x79, 0x0B);
		sccb_write(SLAVE_ID, 0xC8, 0x01);
		sccb_write(SLAVE_ID, 0x79, 0x0C);
		sccb_write(SLAVE_ID, 0xC8, 0x0F);
		sccb_write(SLAVE_ID, 0x79, 0x0D);
		sccb_write(SLAVE_ID, 0xC8, 0x20);
		sccb_write(SLAVE_ID, 0x79, 0x09);
		sccb_write(SLAVE_ID, 0xC8, 0x80);
		sccb_write(SLAVE_ID, 0x79, 0x02);
		sccb_write(SLAVE_ID, 0xC8, 0xC0);
		sccb_write(SLAVE_ID, 0x79, 0x03);
		sccb_write(SLAVE_ID, 0xC8, 0x40);
		sccb_write(SLAVE_ID, 0x79, 0x05);
		sccb_write(SLAVE_ID, 0xC8, 0x30);
		sccb_write(SLAVE_ID, 0x79, 0x26);
		                     
		sccb_write(SLAVE_ID, 0x62, 0x00);
		sccb_write(SLAVE_ID, 0x63, 0x00);
		sccb_write(SLAVE_ID, 0x64, 0x06);
		sccb_write(SLAVE_ID, 0x65, 0x00);
		sccb_write(SLAVE_ID, 0x66, 0x05);
		sccb_write(SLAVE_ID, 0x94, 0x05);
		sccb_write(SLAVE_ID, 0x95, 0x09);
		sccb_write(SLAVE_ID, 0x2A, 0x10);
		sccb_write(SLAVE_ID, 0x2B, 0xC2);
		sccb_write(SLAVE_ID, 0x15, 0x00);
		sccb_write(SLAVE_ID, 0x3A, 0x04);
		sccb_write(SLAVE_ID, 0x3D, 0xC3);
		sccb_write(SLAVE_ID, 0x19, 0x03);
		sccb_write(SLAVE_ID, 0x1A, 0x7B);
		sccb_write(SLAVE_ID, 0x2A, 0x00);
		sccb_write(SLAVE_ID, 0x2B, 0x00);
		sccb_write(SLAVE_ID, 0x18, 0x01);
		sccb_write(SLAVE_ID, 0x92, 0x88);
		sccb_write(SLAVE_ID, 0x93, 0x00);
		sccb_write(SLAVE_ID, 0xB9, 0x30);
		sccb_write(SLAVE_ID, 0x19, 0x02);
		sccb_write(SLAVE_ID, 0x1A, 0x3E);
		sccb_write(SLAVE_ID, 0x17, 0x13);
		sccb_write(SLAVE_ID, 0x18, 0x3B);
		sccb_write(SLAVE_ID, 0x03, 0x0A);
		sccb_write(SLAVE_ID, 0xE6, 0x05);
		sccb_write(SLAVE_ID, 0xD2, 0x1C);
		sccb_write(SLAVE_ID, 0x66, 0x05);
		sccb_write(SLAVE_ID, 0x62, 0x10);
		sccb_write(SLAVE_ID, 0x63, 0x0B);
		sccb_write(SLAVE_ID, 0x65, 0x07);
		                     
		sccb_write(SLAVE_ID, 0x64, 0x0F);
		sccb_write(SLAVE_ID, 0x94, 0x0E);
		sccb_write(SLAVE_ID, 0x95, 0x10);
		sccb_write(SLAVE_ID, 0x4F, 0x87);
		sccb_write(SLAVE_ID, 0x50, 0x68);
		sccb_write(SLAVE_ID, 0x51, 0x1E);
		sccb_write(SLAVE_ID, 0x52, 0x15);
		sccb_write(SLAVE_ID, 0x53, 0x7C);
		sccb_write(SLAVE_ID, 0x54, 0x91);
		sccb_write(SLAVE_ID, 0x58, 0x1E);
		sccb_write(SLAVE_ID, 0x41, 0x38);
		sccb_write(SLAVE_ID, 0x76, 0xE0);
		sccb_write(SLAVE_ID, 0x24, 0x40);
		sccb_write(SLAVE_ID, 0x25, 0x38);
		sccb_write(SLAVE_ID, 0x26, 0x91);
		sccb_write(SLAVE_ID, 0x7A, 0x09);
		sccb_write(SLAVE_ID, 0x7B, 0x0C);
		sccb_write(SLAVE_ID, 0x7C, 0x16);
		sccb_write(SLAVE_ID, 0x7D, 0x28);
		sccb_write(SLAVE_ID, 0x7E, 0x48);
		sccb_write(SLAVE_ID, 0x7F, 0x57);
		sccb_write(SLAVE_ID, 0x80, 0x64);
		sccb_write(SLAVE_ID, 0x81, 0x71);
		sccb_write(SLAVE_ID, 0x82, 0x7E);
		sccb_write(SLAVE_ID, 0x83, 0x89);
		sccb_write(SLAVE_ID, 0x84, 0x94);
		sccb_write(SLAVE_ID, 0x85, 0xA8);
		sccb_write(SLAVE_ID, 0x86, 0xBA);
		sccb_write(SLAVE_ID, 0x87, 0xD7);
		sccb_write(SLAVE_ID, 0x88, 0xEC);
		sccb_write(SLAVE_ID, 0x89, 0xF9);
		sccb_write(SLAVE_ID, 0x09, 0x00);
		sccb_write(SLAVE_ID, 0x92, 0x3F);	
	}           
	else if(nWidthH == 640 && nWidthV == 480)
	{           
		//VGA, YUV, 27Mhz, 30fps
		sccb_write(SLAVE_ID, 0x09, 0x10);
		sccb_write(SLAVE_ID, 0xC1, 0x7F);
#if CSI_FPS	== CSI_15FPS
		sccb_write(SLAVE_ID, 0x11, 0x81);   // 15fps
#else
		sccb_write(SLAVE_ID, 0x11, 0x80);	// 30fps
#endif
		sccb_write(SLAVE_ID, 0x3A, 0x0C);
		sccb_write(SLAVE_ID, 0x3D, 0xC0);
		sccb_write(SLAVE_ID, 0x12, 0x00);
		sccb_write(SLAVE_ID, 0x15, 0x40);
		sccb_write(SLAVE_ID, 0x17, 0x13);
		sccb_write(SLAVE_ID, 0x18, 0x01);
		sccb_write(SLAVE_ID, 0x32, 0xBF);
		sccb_write(SLAVE_ID, 0x19, 0x02);
		sccb_write(SLAVE_ID, 0x1A, 0x7A);
		sccb_write(SLAVE_ID, 0x03, 0x0A);
		sccb_write(SLAVE_ID, 0x0C, 0x00);
		sccb_write(SLAVE_ID, 0x3E, 0x00);
		sccb_write(SLAVE_ID, 0x70, 0x3A);
		sccb_write(SLAVE_ID, 0x71, 0x35);
		sccb_write(SLAVE_ID, 0x72, 0x11);
		sccb_write(SLAVE_ID, 0x73, 0xF0);
		sccb_write(SLAVE_ID, 0xA2, 0x02);
		sccb_write(SLAVE_ID, 0x7A, 0x20);
		sccb_write(SLAVE_ID, 0x7B, 0x03);
		sccb_write(SLAVE_ID, 0x7C, 0x0A);
		sccb_write(SLAVE_ID, 0x7D, 0x1A);
		sccb_write(SLAVE_ID, 0x7E, 0x3F);
		sccb_write(SLAVE_ID, 0x7F, 0x4E);
		sccb_write(SLAVE_ID, 0x80, 0x5B);
		sccb_write(SLAVE_ID, 0x81, 0x68);
		sccb_write(SLAVE_ID, 0x82, 0x75);
		                     
		sccb_write(SLAVE_ID, 0x83, 0x7F);
		sccb_write(SLAVE_ID, 0x84, 0x89);
		sccb_write(SLAVE_ID, 0x85, 0x9A);
		sccb_write(SLAVE_ID, 0x86, 0xA6);
		sccb_write(SLAVE_ID, 0x87, 0xBD);
		sccb_write(SLAVE_ID, 0x88, 0xD3);
		sccb_write(SLAVE_ID, 0x89, 0xE8);
		sccb_write(SLAVE_ID, 0x13, 0xE0);
		sccb_write(SLAVE_ID, 0x00, 0x00);
		sccb_write(SLAVE_ID, 0x10, 0x00);
		sccb_write(SLAVE_ID, 0x0D, 0x40);
		sccb_write(SLAVE_ID, 0x14, 0x28);
		sccb_write(SLAVE_ID, 0xA5, 0x02);
		sccb_write(SLAVE_ID, 0xAB, 0x02);
		sccb_write(SLAVE_ID, 0x24, 0x68);
		sccb_write(SLAVE_ID, 0x25, 0x58);
		sccb_write(SLAVE_ID, 0x26, 0xC2);
		sccb_write(SLAVE_ID, 0x9F, 0x78);
		sccb_write(SLAVE_ID, 0xA0, 0x68);
		sccb_write(SLAVE_ID, 0xA1, 0x03);
		sccb_write(SLAVE_ID, 0xA6, 0xD8);
		sccb_write(SLAVE_ID, 0xA7, 0xD8);
		sccb_write(SLAVE_ID, 0xA8, 0xF0);
		sccb_write(SLAVE_ID, 0xA9, 0x90);
		sccb_write(SLAVE_ID, 0xAA, 0x14);
		sccb_write(SLAVE_ID, 0x13, 0xE5);
		sccb_write(SLAVE_ID, 0x0E, 0x61);
		sccb_write(SLAVE_ID, 0x0F, 0x4B);
		sccb_write(SLAVE_ID, 0x16, 0x02);
		
		sccb_write(SLAVE_ID, 0x1e, 0x37); 	// Flip & Mirror 0x37
		                     
		sccb_write(SLAVE_ID, 0x21, 0x02);
		sccb_write(SLAVE_ID, 0x22, 0x91);
		sccb_write(SLAVE_ID, 0x29, 0x07);
		sccb_write(SLAVE_ID, 0x33, 0x0B);
		sccb_write(SLAVE_ID, 0x35, 0x0B);
		sccb_write(SLAVE_ID, 0x37, 0x1D);
		sccb_write(SLAVE_ID, 0x38, 0x71);
		sccb_write(SLAVE_ID, 0x39, 0x2A);
		sccb_write(SLAVE_ID, 0x3C, 0x78);
		sccb_write(SLAVE_ID, 0x4D, 0x40);
		sccb_write(SLAVE_ID, 0x4E, 0x20);
		sccb_write(SLAVE_ID, 0x69, 0x00);
		sccb_write(SLAVE_ID, 0x6B, 0x0A);
		sccb_write(SLAVE_ID, 0x74, 0x10);
		sccb_write(SLAVE_ID, 0x8D, 0x4F);
		sccb_write(SLAVE_ID, 0x8E, 0x00);
		sccb_write(SLAVE_ID, 0x8F, 0x00);
		sccb_write(SLAVE_ID, 0x90, 0x00);
		sccb_write(SLAVE_ID, 0x91, 0x00);
		sccb_write(SLAVE_ID, 0x96, 0x00);
		sccb_write(SLAVE_ID, 0x9A, 0x80);
		sccb_write(SLAVE_ID, 0xB0, 0x84);
		sccb_write(SLAVE_ID, 0xB1, 0x0C);
		sccb_write(SLAVE_ID, 0xB2, 0x0E);
		sccb_write(SLAVE_ID, 0xB3, 0x82);
		sccb_write(SLAVE_ID, 0xB8, 0x10); //0x08 for 1.8 volts, 0x10 for 2.8 volts to DOVDD	
		sccb_write(SLAVE_ID, 0x43, 0x0A);
		sccb_write(SLAVE_ID, 0x44, 0xF2);
		sccb_write(SLAVE_ID, 0x45, 0x39);
		sccb_write(SLAVE_ID, 0x46, 0x62);
		sccb_write(SLAVE_ID, 0x47, 0x3D);
		sccb_write(SLAVE_ID, 0x48, 0x55);
		sccb_write(SLAVE_ID, 0x59, 0x83);
		sccb_write(SLAVE_ID, 0x5A, 0x0D);
		sccb_write(SLAVE_ID, 0x5B, 0xCD);
		sccb_write(SLAVE_ID, 0x5C, 0x8C);
		sccb_write(SLAVE_ID, 0x5D, 0x77);
		sccb_write(SLAVE_ID, 0x5E, 0x16);
		sccb_write(SLAVE_ID, 0x6C, 0x0A);
		sccb_write(SLAVE_ID, 0x6D, 0x65);
		sccb_write(SLAVE_ID, 0x6E, 0x11);
		sccb_write(SLAVE_ID, 0x6F, 0x9E);
		sccb_write(SLAVE_ID, 0x6A, 0x40);
		sccb_write(SLAVE_ID, 0x01, 0x56);
		sccb_write(SLAVE_ID, 0x02, 0x44);
		sccb_write(SLAVE_ID, 0x13, 0xE7);
		sccb_write(SLAVE_ID, 0x4F, 0x88);
		sccb_write(SLAVE_ID, 0x50, 0x8B);
		sccb_write(SLAVE_ID, 0x51, 0x04);
		sccb_write(SLAVE_ID, 0x52, 0x11);
		sccb_write(SLAVE_ID, 0x53, 0x8C);
		sccb_write(SLAVE_ID, 0x54, 0x9D);
		sccb_write(SLAVE_ID, 0x55, 0x00);
		sccb_write(SLAVE_ID, 0x56, 0x40);
		sccb_write(SLAVE_ID, 0x57, 0x80);
		sccb_write(SLAVE_ID, 0x58, 0x9A);
		sccb_write(SLAVE_ID, 0x41, 0x08);
		sccb_write(SLAVE_ID, 0x3F, 0x00);
		sccb_write(SLAVE_ID, 0x75, 0x04);
		sccb_write(SLAVE_ID, 0x76, 0x60);
		sccb_write(SLAVE_ID, 0x4C, 0x00);
		sccb_write(SLAVE_ID, 0x77, 0x01);
		sccb_write(SLAVE_ID, 0x3D, 0xC2);
		sccb_write(SLAVE_ID, 0x4D, 0x09);
		sccb_write(SLAVE_ID, 0xC9, 0x30);
		sccb_write(SLAVE_ID, 0x41, 0x38);
		sccb_write(SLAVE_ID, 0x56, 0x40);
		sccb_write(SLAVE_ID, 0x34, 0x11);
		sccb_write(SLAVE_ID, 0x3B, 0x12);
		sccb_write(SLAVE_ID, 0xA4, 0x88);
		sccb_write(SLAVE_ID, 0x96, 0x00);
		sccb_write(SLAVE_ID, 0x97, 0x30);
		sccb_write(SLAVE_ID, 0x98, 0x20);
		sccb_write(SLAVE_ID, 0x99, 0x30);
		sccb_write(SLAVE_ID, 0x9A, 0x84);
		sccb_write(SLAVE_ID, 0x9B, 0x29);
		sccb_write(SLAVE_ID, 0x9C, 0x03);
		sccb_write(SLAVE_ID, 0x9D, 0x99);
		sccb_write(SLAVE_ID, 0x9E, 0x7F);
		                     
		sccb_write(SLAVE_ID, 0x78, 0x04);
		sccb_write(SLAVE_ID, 0x79, 0x01);
		sccb_write(SLAVE_ID, 0xC8, 0xF0);
		sccb_write(SLAVE_ID, 0x79, 0x0F);
		sccb_write(SLAVE_ID, 0xC8, 0x00);
		sccb_write(SLAVE_ID, 0x79, 0x10);
		sccb_write(SLAVE_ID, 0xC8, 0x7E);
		sccb_write(SLAVE_ID, 0x79, 0x0A);
		sccb_write(SLAVE_ID, 0xC8, 0x80);
		sccb_write(SLAVE_ID, 0x79, 0x0B);
		sccb_write(SLAVE_ID, 0xC8, 0x01);
		sccb_write(SLAVE_ID, 0x79, 0x0C);
		sccb_write(SLAVE_ID, 0xC8, 0x0F);
		sccb_write(SLAVE_ID, 0x79, 0x0D);
		sccb_write(SLAVE_ID, 0xC8, 0x20);
		sccb_write(SLAVE_ID, 0x79, 0x09);
		sccb_write(SLAVE_ID, 0xC8, 0x80);
		sccb_write(SLAVE_ID, 0x79, 0x02);
		sccb_write(SLAVE_ID, 0xC8, 0xC0);
		sccb_write(SLAVE_ID, 0x79, 0x03);
		sccb_write(SLAVE_ID, 0xC8, 0x40);
		sccb_write(SLAVE_ID, 0x79, 0x05);
		sccb_write(SLAVE_ID, 0xC8, 0x30);
		sccb_write(SLAVE_ID, 0x79, 0x26);
		          
		sccb_write(SLAVE_ID, 0x62, 0x00);
		sccb_write(SLAVE_ID, 0x63, 0x00);
		sccb_write(SLAVE_ID, 0x64, 0x06);
		sccb_write(SLAVE_ID, 0x65, 0x00);
		sccb_write(SLAVE_ID, 0x66, 0x05);
		sccb_write(SLAVE_ID, 0x94, 0x05);
		sccb_write(SLAVE_ID, 0x95, 0x09);
		sccb_write(SLAVE_ID, 0x2A, 0x10);
		sccb_write(SLAVE_ID, 0x2B, 0xC2);
		sccb_write(SLAVE_ID, 0x15, 0x00);
		sccb_write(SLAVE_ID, 0x3A, 0x04);
		sccb_write(SLAVE_ID, 0x3D, 0xC3);
		sccb_write(SLAVE_ID, 0x19, 0x03);
		sccb_write(SLAVE_ID, 0x1A, 0x7B);
		sccb_write(SLAVE_ID, 0x2A, 0x00);
		sccb_write(SLAVE_ID, 0x2B, 0x00);
		sccb_write(SLAVE_ID, 0x18, 0x01);
		                    
		sccb_write(SLAVE_ID, 0x66, 0x05);
		sccb_write(SLAVE_ID, 0x62, 0x10);
		sccb_write(SLAVE_ID, 0x63, 0x0B);
		sccb_write(SLAVE_ID, 0x65, 0x07);
		sccb_write(SLAVE_ID, 0x64, 0x0F);
		sccb_write(SLAVE_ID, 0x94, 0x0E);
		sccb_write(SLAVE_ID, 0x95, 0x10);
		sccb_write(SLAVE_ID, 0x4F, 0x87);
		sccb_write(SLAVE_ID, 0x50, 0x68);
		sccb_write(SLAVE_ID, 0x51, 0x1E);
		sccb_write(SLAVE_ID, 0x52, 0x15);
		sccb_write(SLAVE_ID, 0x53, 0x7C);
		sccb_write(SLAVE_ID, 0x54, 0x91);
		sccb_write(SLAVE_ID, 0x58, 0x1E);
		                     
		sccb_write(SLAVE_ID, 0x41, 0x38);
		sccb_write(SLAVE_ID, 0x76, 0xE0);
		sccb_write(SLAVE_ID, 0x24, 0x40);
		sccb_write(SLAVE_ID, 0x25, 0x38);
		sccb_write(SLAVE_ID, 0x26, 0x91);
		sccb_write(SLAVE_ID, 0x7A, 0x09);
		sccb_write(SLAVE_ID, 0x7B, 0x0C);
		sccb_write(SLAVE_ID, 0x7C, 0x16);
		sccb_write(SLAVE_ID, 0x7D, 0x28);
		sccb_write(SLAVE_ID, 0x7E, 0x48);
		sccb_write(SLAVE_ID, 0x7F, 0x57);
                         
		sccb_write(SLAVE_ID, 0x80, 0x64);
		sccb_write(SLAVE_ID, 0x81, 0x71);
		sccb_write(SLAVE_ID, 0x82, 0x7E);
		sccb_write(SLAVE_ID, 0x83, 0x89);
		sccb_write(SLAVE_ID, 0x84, 0x94);
		sccb_write(SLAVE_ID, 0x85, 0xA8);
		sccb_write(SLAVE_ID, 0x86, 0xBA);
		sccb_write(SLAVE_ID, 0x87, 0xD7);
		sccb_write(SLAVE_ID, 0x88, 0xEC);
		sccb_write(SLAVE_ID, 0x89, 0xF9);
		sccb_write(SLAVE_ID, 0x09, 0x00);
		sccb_write(SLAVE_ID, 0x92, 0x3F);
	}           
	R_SPI0_CTRL = SPI_Ctrl_temp;

	R_CSI_TG_CTRL1 = uCtrlReg2;					//*P_Sensor_TG_Ctrl2 = uCtrlReg2;
//#if CSI_MODE == CSI_PPU_FRAME_MODE
//	R_CSI_TG_CTRL0 = uCtrlReg1;					//*P_Sensor_TG_Ctrl1 = uCtrlReg1;
//#elif CSI_MODE == CSI_TG_FRAME_MODE
//	R_CSI_TG_CTRL0 = uCtrlReg1|0x010000;
//#elif CSI_MODE == CSI_FIFO_8_MODE
//	R_CSI_TG_CTRL0 = uCtrlReg1|0x110000;
//#elif CSI_MODE == CSI_FIFO_16_MODE
//	R_CSI_TG_CTRL0 = uCtrlReg1|0x210000;
//#elif CSI_MODE == CSI_FIFO_32_MODE
//	R_CSI_TG_CTRL0 = uCtrlReg1|0x310000;
//#endif

	if (frame_mode_en == 1) { // CSI_MODE == CSI_TG_FRAME_MODE
		R_CSI_TG_CTRL0 = uCtrlReg1|0x010000;
	} else { // CSI_MODE == CSI_FIFO_32_MODE
		R_CSI_TG_CTRL0 = uCtrlReg1|0x310000;
	}
}

#endif


#ifdef	__GC0308_DRV_C__
//====================================================================================================
//	Description:	GC0308 Initialization
//	Syntax:			void GC0308_Init (
//						INT16S nWidthH,			// Active H Width
//						INT16S nWidthV,			// Active V Width
//						INT16U uFlag				// Flag Type
//					);
//	Return:			None
//====================================================================================================
void GC0308_Init (
	INT16S nWidthH,			// Active H Width
	INT16S nWidthV,			// Active V Width
	INT16U uFlag				// Flag Type
) {
	INT16U uCtrlReg1, uCtrlReg2;
	INT16S nReso;
	INT32U SPI_Ctrl_temp;

	// Enable CSI clock to let sensor initialize at first
	csi_fifo_flag = 1;

#if CSI_CLOCK == CSI_CLOCK_SYS_CLK_DIV2
	uCtrlReg2 = CLKOEN | CSI_RGB565 |CLK_SEL48M | CSI_HIGHPRI| CSI_NOSTOP;
	R_SYSTEM_CTRL &= ~0x4000;
#elif CSI_CLOCK == CSI_CLOCK_27MHZ
	uCtrlReg2 = CLKOEN | CSI_RGB565 |CLK_SEL27M | CSI_HIGHPRI| CSI_NOSTOP;
	R_SYSTEM_CTRL &= ~0x4000;
#elif CSI_CLOCK == CSI_CLOCK_SYS_CLK_DIV4
	uCtrlReg2 = CLKOEN | CSI_RGB565 |CLK_SEL48M | CSI_HIGHPRI| CSI_NOSTOP;
	R_SYSTEM_CTRL |= 0x4000;
#elif CSI_CLOCK == CSI_CLOCK_13_5MHz
	uCtrlReg2 = CLKOEN | CSI_RGB565 |CLK_SEL27M | CSI_HIGHPRI| CSI_NOSTOP;
	R_SYSTEM_CTRL |= 0x4000;
#endif

	uCtrlReg1 = CSIEN | YUV_YUYV | CAP;									// Default CSI Control Register 1
	if (uFlag & FT_CSI_RGB1555)											// RGB1555
	{
		uCtrlReg2 |= CSI_RGB1555;
	}
	if (uFlag & FT_CSI_CCIR656)										// CCIR656?
	{
		uCtrlReg1 |= CCIR656 | VADD_FALL | VRST_FALL | HRST_FALL;	// CCIR656
		uCtrlReg2 |= D_TYPE1;										// CCIR656
	}
	else
	{
		uCtrlReg1 |= VADD_RISE | VRST_FALL | HRST_RISE | HREF;		// NOT CCIR656
		uCtrlReg2 |= D_TYPE0;										// NOT CCIR656
	}
	if (uFlag & FT_CSI_YUVIN)										// YUVIN?
	{
		uCtrlReg1 |= YUVIN;
	}
	if (uFlag & FT_CSI_YUVOUT)										// YUVOUT?
	{
		uCtrlReg1 |= YUVOUT;
	}

	// Whether compression or not?
	nReso = ((nWidthH == 320) && (nWidthV == 240)) ? 1 : 0;
	if (nReso == 1)								// VGA
	{
#ifdef	__TV_QVGA__
		R_CSI_TG_HRATIO = 0x0102;					// Scale to 1/2
		R_CSI_TG_VRATIO = 0x0102;					// Scale to 1/2
		R_CSI_TG_HWIDTH = nWidthH;					// Horizontal frame width
		R_CSI_TG_VHEIGHT = nWidthV*2;				// Vertical frame width
#endif	// __TV_QVGA__
	}
	else
	{
		R_CSI_TG_HRATIO = 0;
		R_CSI_TG_VRATIO = 0;
	}

	R_CSI_TG_VL0START = 0x0000;						// Sensor field 0 vertical latch start register.
	R_CSI_TG_VL1START = 0x0000;						//*P_Sensor_TG_V_L1Start = 0x0000;
	R_CSI_TG_HSTART = 0x0000;						// Sensor horizontal start register.

	R_CSI_TG_CTRL0 = 0;								//reset control0
	R_CSI_TG_CTRL1 = CSI_NOSTOP|CLKOEN;				//enable CSI CLKO
	drv_msec_wait(100); 							//wait 100ms for CLKO stable
	
	SPI_Ctrl_temp = R_SPI0_CTRL;
	R_SPI0_CTRL = 0;
	// CMOS Sensor Initialization Start...
	sccb_init (SCCB_SCL, SCCB_SDA);
	sccb_delay (200);
	//if((nWidthH == 640) &&(nWidthV == 480))
	{

		sccb_write(SLAVE_ID,0XFE,0X80);
		sccb_write(SLAVE_ID,0XFE,0X00);
		sccb_write(SLAVE_ID,0XD2,0X10);   // close AEC
		sccb_write(SLAVE_ID,0X22,0X55);   
		sccb_write(SLAVE_ID,0X5A,0X56); 
		sccb_write(SLAVE_ID,0X5B,0X40); 
		sccb_write(SLAVE_ID,0X5C,0X4A); 		
		sccb_write(SLAVE_ID,0X22,0X57);   // Open AWB
#if 1
	/*
        //27MHz								//30 fps
		sccb_write(SLAVE_ID,0x01,0x56);
		sccb_write(SLAVE_ID,0x02,0x1f);
		sccb_write(SLAVE_ID,0x0f,0x00);
		sccb_write(SLAVE_ID,0xe2,0x00);
		sccb_write(SLAVE_ID,0xe3,0xad);//0xad		
		sccb_write(SLAVE_ID,0xe4,0x02);
		sccb_write(SLAVE_ID,0xe5,0x07);
		sccb_write(SLAVE_ID,0xe6,0x02);
		sccb_write(SLAVE_ID,0xe7,0x07);
		sccb_write(SLAVE_ID,0xe8,0x02);
		sccb_write(SLAVE_ID,0xe9,0x07);
		sccb_write(SLAVE_ID,0xea,0x02);
		sccb_write(SLAVE_ID,0xeb,0x07);
		sccb_write(SLAVE_ID,0xec,0x20);	
	*/
	
	/*	
		// 27M								//12fps
	     sccb_write(SLAVE_ID, 0x0f,0x02);
		 sccb_write(SLAVE_ID, 0x01,0x2c);       // 27M
         sccb_write(SLAVE_ID, 0x02,0xa0);
         sccb_write(SLAVE_ID, 0xe2,0x00);  //anti-flicker step [11:8]
         sccb_write(SLAVE_ID, 0xe3,0x6c);  //anti-flicker step [7:0]
         
				
		 sccb_write(SLAVE_ID, 0xe4,0x03);//exp level 1  16.67fps
		 sccb_write(SLAVE_ID, 0xe5,0x60);
		 sccb_write(SLAVE_ID, 0xe6,0x03);//exp level 2  12.5fps
		 sccb_write(SLAVE_ID, 0xe7,0x60);
		 sccb_write(SLAVE_ID, 0xe8,0x03);//exp level 3  8.33fps 
		 sccb_write(SLAVE_ID, 0xe9,0x60);
		 sccb_write(SLAVE_ID, 0xea,0x03);//exp level 4  4.00fps
		 sccb_write(SLAVE_ID, 0xeb,0x60);
    	 sccb_write(SLAVE_ID, 0xec,0x00);
	*/	

		//24MHz
		sccb_write(SLAVE_ID,0x01,0x6a);
	    sccb_write(SLAVE_ID,0x02,0x70);
	    sccb_write(SLAVE_ID,0x0f,0x00);
	    sccb_write(SLAVE_ID,0xe2,0x00);
	    sccb_write(SLAVE_ID,0xe3,0x96);
	    sccb_write(SLAVE_ID,0xe4,0x02);
	    sccb_write(SLAVE_ID,0xe5,0x58);
	    sccb_write(SLAVE_ID,0xe6,0x02);
	    sccb_write(SLAVE_ID,0xe7,0x58);
	    sccb_write(SLAVE_ID,0xe8,0x02);
	    sccb_write(SLAVE_ID,0xe9,0x58);
	    sccb_write(SLAVE_ID,0xea,0x02);
	    sccb_write(SLAVE_ID,0xeb,0x58);
	    sccb_write(SLAVE_ID,0xec,0x20);
#else			
		sccb_write(SLAVE_ID,0x01,0xce);
		sccb_write(SLAVE_ID,0x02,0x70);
		sccb_write(SLAVE_ID,0x0f,0x00);
		sccb_write(SLAVE_ID,0xe2,0x00);
		sccb_write(SLAVE_ID,0xe3,0x96);
		sccb_write(SLAVE_ID,0xe4,0x02);
		sccb_write(SLAVE_ID,0xe5,0x58);
		sccb_write(SLAVE_ID,0xe6,0x02);//eb
		sccb_write(SLAVE_ID,0xe7,0xee);
		sccb_write(SLAVE_ID,0xe8,0x03);
		sccb_write(SLAVE_ID,0xe9,0x84);
		sccb_write(SLAVE_ID,0xea,0x0c);
		sccb_write(SLAVE_ID,0xeb,0xbe);
		sccb_write(SLAVE_ID,0xec,0x20);
#endif
		sccb_write(SLAVE_ID,0x05,0x00);
		sccb_write(SLAVE_ID,0x06,0x00);
		sccb_write(SLAVE_ID,0x07,0x00);
		sccb_write(SLAVE_ID,0x08,0x00);
		sccb_write(SLAVE_ID,0x09,0x01);
		sccb_write(SLAVE_ID,0x0a,0xe8);
		sccb_write(SLAVE_ID,0x0b,0x02);
		sccb_write(SLAVE_ID,0x0c,0x88);
		sccb_write(SLAVE_ID,0x0d,0x02);
		sccb_write(SLAVE_ID,0x0e,0x02);
		sccb_write(SLAVE_ID,0x10,0x26);
		sccb_write(SLAVE_ID,0x11,0x0d);
		sccb_write(SLAVE_ID,0x12,0x2a);
		sccb_write(SLAVE_ID,0x13,0x00);
		sccb_write(SLAVE_ID,0x15,0x0a);
		sccb_write(SLAVE_ID,0x16,0x05);
		sccb_write(SLAVE_ID,0x17,0x01);
		sccb_write(SLAVE_ID,0x18,0x44);
		sccb_write(SLAVE_ID,0x19,0x44);
		sccb_write(SLAVE_ID,0x1a,0x2a);
		sccb_write(SLAVE_ID,0x1b,0x00);
		sccb_write(SLAVE_ID,0x1c,0x49);
		sccb_write(SLAVE_ID,0x1d,0x9a);
		sccb_write(SLAVE_ID,0x1e,0x61);
		sccb_write(SLAVE_ID,0x1f,0x16);
		sccb_write(SLAVE_ID,0x20,0xff);
		sccb_write(SLAVE_ID,0x21,0xf8);
		sccb_write(SLAVE_ID,0x22,0x57);
		sccb_write(SLAVE_ID,0x24,0xa2);
		sccb_write(SLAVE_ID,0x25,0x0f);
		sccb_write(SLAVE_ID,0x26,0x02);
		sccb_write(SLAVE_ID,0x2f,0x01);
		sccb_write(SLAVE_ID,0x30,0xf7);
		sccb_write(SLAVE_ID,0x31,0x50);
		sccb_write(SLAVE_ID,0x32,0x00);
		sccb_write(SLAVE_ID,0x39,0x04);
		sccb_write(SLAVE_ID,0x3a,0x20);
		sccb_write(SLAVE_ID,0x3b,0x20);
	#if 1
		sccb_write(SLAVE_ID,0x3c,0x02);
		sccb_write(SLAVE_ID,0x3d,0x00);
		sccb_write(SLAVE_ID,0x3e,0x02);
		sccb_write(SLAVE_ID,0x3f,0x02);
	#else
		sccb_write(SLAVE_ID,0x3c,0x00);
		sccb_write(SLAVE_ID,0x3d,0x00);
		sccb_write(SLAVE_ID,0x3e,0x00);
		sccb_write(SLAVE_ID,0x3f,0x00);
	#endif
		sccb_write(SLAVE_ID,0x50,0x12);
		sccb_write(SLAVE_ID,0x53,0x84);
		sccb_write(SLAVE_ID,0x54,0x80);
		sccb_write(SLAVE_ID,0x55,0x80);
		sccb_write(SLAVE_ID,0x56,0x84);
		
		sccb_write(SLAVE_ID,0x57,0x80);
		sccb_write(SLAVE_ID,0x58,0x80);
		sccb_write(SLAVE_ID,0x59,0x80);
		sccb_write(SLAVE_ID,0x8b,0x20);
		sccb_write(SLAVE_ID,0x8c,0x20);
		sccb_write(SLAVE_ID,0x8d,0x20);
		sccb_write(SLAVE_ID,0x8e,0x14);
		sccb_write(SLAVE_ID,0x8f,0x10);
		sccb_write(SLAVE_ID,0x90,0x14);
		sccb_write(SLAVE_ID,0x91,0x3c);
		sccb_write(SLAVE_ID,0x92,0x50);
		sccb_write(SLAVE_ID,0x5d,0x12);
		sccb_write(SLAVE_ID,0x5e,0x1a);
		sccb_write(SLAVE_ID,0x5f,0x24);
		sccb_write(SLAVE_ID,0x60,0x07);
		sccb_write(SLAVE_ID,0x61,0x15);
		sccb_write(SLAVE_ID,0x62,0x08);
		sccb_write(SLAVE_ID,0x64,0x03);
		sccb_write(SLAVE_ID,0x66,0xe8);
		sccb_write(SLAVE_ID,0x67,0x86);
		sccb_write(SLAVE_ID,0x68,0xa2);
		sccb_write(SLAVE_ID,0x69,0x18);
		sccb_write(SLAVE_ID,0x6a,0x0f);
		sccb_write(SLAVE_ID,0x6b,0x00);
		sccb_write(SLAVE_ID,0x6c,0x5f);
		sccb_write(SLAVE_ID,0x6d,0x8f);
		sccb_write(SLAVE_ID,0x6e,0x55);
		sccb_write(SLAVE_ID,0x6f,0x38);
		sccb_write(SLAVE_ID,0x70,0x15);
		sccb_write(SLAVE_ID,0x71,0x33);
		sccb_write(SLAVE_ID,0x72,0xdc);
		sccb_write(SLAVE_ID,0x73,0x80);
		sccb_write(SLAVE_ID,0x74,0x02);
		sccb_write(SLAVE_ID,0x75,0x3f);
		sccb_write(SLAVE_ID,0x76,0x02);
		sccb_write(SLAVE_ID,0x77,0x36);
		sccb_write(SLAVE_ID,0x78,0x88);
		sccb_write(SLAVE_ID,0x79,0x81);
		sccb_write(SLAVE_ID,0x7a,0x81);
		sccb_write(SLAVE_ID,0x7b,0x22);
		sccb_write(SLAVE_ID,0x7c,0xff);
		
		sccb_write(SLAVE_ID,0x93,0x48);
		sccb_write(SLAVE_ID,0x94,0x00);
		sccb_write(SLAVE_ID,0x95,0x04);
		sccb_write(SLAVE_ID,0x96,0xE0);
		sccb_write(SLAVE_ID,0x97,0x46);
		sccb_write(SLAVE_ID,0x98,0xF3);
  // gamma
  #if 1   
		sccb_write(SLAVE_ID,0x9F,0x14);
		sccb_write(SLAVE_ID,0xA0,0x28);
		sccb_write(SLAVE_ID,0xA1,0x44);
		sccb_write(SLAVE_ID,0xA2,0x5D);
		sccb_write(SLAVE_ID,0xA3,0x72);
		sccb_write(SLAVE_ID,0xA4,0x86);
		sccb_write(SLAVE_ID,0xA5,0x95);
		sccb_write(SLAVE_ID,0xA6,0xB1);
		sccb_write(SLAVE_ID,0xA7,0xC6);
		sccb_write(SLAVE_ID,0xA8,0xD5);
		sccb_write(SLAVE_ID,0xA9,0xE1);
		sccb_write(SLAVE_ID,0xAA,0xEA);
		sccb_write(SLAVE_ID,0xAB,0xF1);
		sccb_write(SLAVE_ID,0xAC,0xF5);
		sccb_write(SLAVE_ID,0xAD,0xFB);
		sccb_write(SLAVE_ID,0xAE,0xFE);
		sccb_write(SLAVE_ID,0xAF,0xFF);
  #else
		sccb_write(SLAVE_ID,0x9F,0x10);
		sccb_write(SLAVE_ID,0xA0,0x20);
		sccb_write(SLAVE_ID,0xA1,0x38);
		sccb_write(SLAVE_ID,0xA2,0x4e);
		sccb_write(SLAVE_ID,0xA3,0x63);
		sccb_write(SLAVE_ID,0xA4,0x76);
		sccb_write(SLAVE_ID,0xA5,0x87);
		sccb_write(SLAVE_ID,0xA6,0xa2);
		sccb_write(SLAVE_ID,0xA7,0xb8);
		sccb_write(SLAVE_ID,0xA8,0xca);
		sccb_write(SLAVE_ID,0xA9,0xd8);
		sccb_write(SLAVE_ID,0xAA,0xe3);
		sccb_write(SLAVE_ID,0xAB,0xEb);
		sccb_write(SLAVE_ID,0xAC,0xf0);
		sccb_write(SLAVE_ID,0xAD,0xF8);
		sccb_write(SLAVE_ID,0xAE,0xFD);
		sccb_write(SLAVE_ID,0xAF,0xFF);
  #endif
		sccb_write(SLAVE_ID,0xb1 , 0x40);
		sccb_write(SLAVE_ID,0xb2 , 0x40);
		sccb_write(SLAVE_ID,0xbd , 0x3c);
		sccb_write(SLAVE_ID,0xbe , 0x36);
		sccb_write(SLAVE_ID,0xd0 , 0xcb);
		sccb_write(SLAVE_ID,0xd1 , 0x10);
		sccb_write(SLAVE_ID,0xd3 , 0x50);// 0X80//brightness
		sccb_write(SLAVE_ID,0xd5 , 0xf2);
		sccb_write(SLAVE_ID,0xd6 , 0x10);
		sccb_write(SLAVE_ID,0xdb , 0x92);
		sccb_write(SLAVE_ID,0xdc , 0xa5);
		sccb_write(SLAVE_ID,0xdf , 0x23);
		sccb_write(SLAVE_ID,0xd9 , 0x00);
		sccb_write(SLAVE_ID,0xda , 0x00);
		sccb_write(SLAVE_ID,0xe0 , 0x09);
		sccb_write(SLAVE_ID,0xec , 0x20);
		sccb_write(SLAVE_ID,0xed , 0x04);
		sccb_write(SLAVE_ID,0xee , 0xa0);
		sccb_write(SLAVE_ID,0xef , 0x40);
		sccb_write(SLAVE_ID,0x80 , 0x03);
		sccb_write(SLAVE_ID,0x80 , 0x03);
		sccb_write(SLAVE_ID,0xc0 , 0x00);
		sccb_write(SLAVE_ID,0xc1 , 0x10);
		sccb_write(SLAVE_ID,0xc2 , 0x1C);
		sccb_write(SLAVE_ID,0xc3 , 0x30);
		sccb_write(SLAVE_ID,0xc4 , 0x43);
		sccb_write(SLAVE_ID,0xc5 , 0x54);
		sccb_write(SLAVE_ID,0xc6 , 0x65);
		sccb_write(SLAVE_ID,0xc7 , 0x75);
		sccb_write(SLAVE_ID,0xc8 , 0x93);
		
		sccb_write(SLAVE_ID,0xc9 , 0xB0);
		sccb_write(SLAVE_ID,0xca , 0xCB);
		sccb_write(SLAVE_ID,0xcb , 0xE6);
		sccb_write(SLAVE_ID,0xcc , 0xFF);
		sccb_write(SLAVE_ID,0xf0 , 0x02);
		sccb_write(SLAVE_ID,0xf1 , 0x01);
		sccb_write(SLAVE_ID,0xf2 , 0x03);
		sccb_write(SLAVE_ID,0xf3 , 0x30);
		sccb_write(SLAVE_ID,0xf9 , 0x9f);
		sccb_write(SLAVE_ID,0xfa , 0x78);
		sccb_write(SLAVE_ID,0xfe , 0x01);
		sccb_write(SLAVE_ID,0x00 , 0xf5);  //high_low limit
		sccb_write(SLAVE_ID,0x02 , 0x20);  //y2c 
		sccb_write(SLAVE_ID,0x04 , 0x10);  
		sccb_write(SLAVE_ID,0x05 , 0x10);  
		sccb_write(SLAVE_ID,0x06 , 0x20);  
		sccb_write(SLAVE_ID,0x08 , 0x15);  //kim 0x0a,
		sccb_write(SLAVE_ID,0x0a , 0xa0);  // number limit
		sccb_write(SLAVE_ID,0x0b , 0x64);  //kim 0x60,// skip_mode
		sccb_write(SLAVE_ID,0x0c , 0x08);                               
		sccb_write(SLAVE_ID,0x0e , 0x4c);   // width step
		sccb_write(SLAVE_ID,0x0f , 0x39);   // height step
		sccb_write(SLAVE_ID,0x10 , 0x00);  //kim 0x41,
		sccb_write(SLAVE_ID,0x11 , 0x37);    // 0x3f
		sccb_write(SLAVE_ID,0x12 , 0x24);  //kim 0x72,                  
		sccb_write(SLAVE_ID,0x13 , 0x39);  //0x19//13//smooth 2
		sccb_write(SLAVE_ID,0x14 , 0x45);  //kim 0x42,//R_5k_gain_base
		sccb_write(SLAVE_ID,0x15 , 0x45);  //kim 0x43,//B_5k_gain_base
		sccb_write(SLAVE_ID,0x16 , 0xc2);  //kim 0xbe, //c2//sinT
		sccb_write(SLAVE_ID,0x17 , 0xa8);  //kim 0xac, //a8//a8 //a8//co
		sccb_write(SLAVE_ID,0x18 , 0x18);  //X1 thd
		sccb_write(SLAVE_ID,0x19 , 0x40);  //X2 thd
		sccb_write(SLAVE_ID,0x1a , 0xcc);  //kim 0xd8,//e4//d0//Y1 thd
		sccb_write(SLAVE_ID,0x1b , 0xf5);  //Y2 thd
		sccb_write(SLAVE_ID,0x70 , 0x40);   // A R2G low	
		sccb_write(SLAVE_ID,0x71 , 0x58);   // A R2G high
		sccb_write(SLAVE_ID,0x72 , 0x30);   // A B2G low
		                                   
		sccb_write(SLAVE_ID,0x73 , 0x48);   // A B2G high
		sccb_write(SLAVE_ID,0x74 , 0x20);   // A G low
		sccb_write(SLAVE_ID,0x75 , 0x60);   // A G high  
		sccb_write(SLAVE_ID,0x77 , 0x20);                               
		sccb_write(SLAVE_ID,0x78 , 0x32);                               
		sccb_write(SLAVE_ID,0x30 , 0x03);  //[1]HSP_en [0]sa_curve_en
		sccb_write(SLAVE_ID,0x31 , 0x40);
		sccb_write(SLAVE_ID,0x32 , 0x10);
		sccb_write(SLAVE_ID,0x33 , 0xe0);
		sccb_write(SLAVE_ID,0x34 , 0xe0);
		sccb_write(SLAVE_ID,0x35 , 0x00);
		sccb_write(SLAVE_ID,0x36 , 0x80);
		sccb_write(SLAVE_ID,0x37 , 0x00);
		sccb_write(SLAVE_ID,0x38 , 0x04);//sat1, at8   
		sccb_write(SLAVE_ID,0x39 , 0x09);
		sccb_write(SLAVE_ID,0x3a , 0x12);
		sccb_write(SLAVE_ID,0x3b , 0x1C);
		sccb_write(SLAVE_ID,0x3c , 0x28);
		sccb_write(SLAVE_ID,0x3d , 0x31);
		sccb_write(SLAVE_ID,0x3e , 0x44);
		sccb_write(SLAVE_ID,0x3f , 0x57);
		sccb_write(SLAVE_ID,0x40 , 0x6C);
		sccb_write(SLAVE_ID,0x41 , 0x81);
		sccb_write(SLAVE_ID,0x42 , 0x94);
		sccb_write(SLAVE_ID,0x43 , 0xA7);
		sccb_write(SLAVE_ID,0x44 , 0xB8);
		sccb_write(SLAVE_ID,0x45 , 0xD6);
		sccb_write(SLAVE_ID,0x46 , 0xEE); //sat15,at224
		sccb_write(SLAVE_ID,0x47 , 0x0d);//blue_edge_dec_ratio
		sccb_write(SLAVE_ID,0xfe , 0x00);
		sccb_write(SLAVE_ID,0xb6 , 0xe0);
		
		sccb_write(SLAVE_ID,0x14 , 0x13);//0x10//0x13//Flip (bit1) & Mirror (bit0)
		sccb_write(SLAVE_ID,0x23 , 0x00);
		sccb_write(SLAVE_ID,0x2d , 0x0a);
		sccb_write(SLAVE_ID,0x20 , 0xff);
		sccb_write(SLAVE_ID,0xd2 , 0x90);
		sccb_write(SLAVE_ID,0x73 , 0x00);
		sccb_write(SLAVE_ID,0x77 , 0x33);
		sccb_write(SLAVE_ID,0xb3 , 0x40);
		sccb_write(SLAVE_ID,0xb4 , 0x80);//Contrast center
		sccb_write(SLAVE_ID,0xb9 , 0xe3);
		sccb_write(SLAVE_ID,0xba , 0x00);
		sccb_write(SLAVE_ID,0xbb , 0x00);
			
	}
	R_SPI0_CTRL = SPI_Ctrl_temp;

	R_CSI_TG_CTRL1 = uCtrlReg2;					//*P_Sensor_TG_Ctrl2 = uCtrlReg2;
//#if CSI_MODE == CSI_PPU_FRAME_MODE
//	R_CSI_TG_CTRL0 = uCtrlReg1;					//*P_Sensor_TG_Ctrl1 = uCtrlReg1;
//#elif CSI_MODE == CSI_TG_FRAME_MODE
//	R_CSI_TG_CTRL0 = uCtrlReg1|0x010000;
//#elif CSI_MODE == CSI_FIFO_8_MODE
//	R_CSI_TG_CTRL0 = uCtrlReg1|0x110000;
//#elif CSI_MODE == CSI_FIFO_16_MODE
//	R_CSI_TG_CTRL0 = uCtrlReg1|0x210000;
//#elif CSI_MODE == CSI_FIFO_32_MODE
//	R_CSI_TG_CTRL0 = uCtrlReg1|0x310000;
//#endif
	if (frame_mode_en == 1) { // CSI_MODE == CSI_TG_FRAME_MODE
		R_CSI_TG_CTRL0 = uCtrlReg1|0x010000;
	} else { // CSI_MODE == CSI_FIFO_32_MODE
		R_CSI_TG_CTRL0 = uCtrlReg1|0x310000;
	}
}
#endif

#ifdef	__SP0838_DRV_C__
//====================================================================================================
//	Description:	OV7670 Initialization
//	Syntax:			void OV7670_Init (
//						INT16S nWidthH,			// Active H Width
//						INT16S nWidthV,			// Active V Width
//						INT16U uFlag				// Flag Type
//					);
//	Return:			None
//====================================================================================================
void SP0838_Init (
	INT16S nWidthH,			// Active H Width
	INT16S nWidthV,			// Active V Width
	INT16U uFlag				// Flag Type
) {
	INT16U uCtrlReg1, uCtrlReg2;
	INT16S nReso;

	// Enable CSI clock to let sensor initialize at first
#if CSI_CLOCK == CSI_CLOCK_27MHZ
	uCtrlReg2 = CLKOEN | CSI_RGB565 |CLK_SEL27M | CSI_HIGHPRI | CSI_NOSTOP;
#else
	uCtrlReg2 = CLKOEN | CSI_RGB565 |CLK_SEL48M | CSI_HIGHPRI | CSI_NOSTOP;
#endif

	uCtrlReg1 = CSIEN | YUV_YUYV | CAP;									// Default CSI Control Register 1
	if (uFlag & FT_CSI_RGB1555)											// RGB1555
	{
		uCtrlReg2 |= CSI_RGB1555;
	}
	if (uFlag & FT_CSI_CCIR656)										// CCIR656?
	{
		uCtrlReg1 |= CCIR656 | VADD_FALL | VRST_FALL | HRST_FALL;	// CCIR656
		uCtrlReg2 |= D_TYPE1;										// CCIR656
	}
	else
	{
		uCtrlReg1 |= VADD_RISE | VRST_FALL | HRST_RISE | HREF;		// NOT CCIR656
		uCtrlReg2 |= D_TYPE0;										// NOT CCIR656
	}
	if (uFlag & FT_CSI_YUVIN)										// YUVIN?
	{
		uCtrlReg1 |= YUVIN;
	}
	if (uFlag & FT_CSI_YUVOUT)										// YUVOUT?
	{
		uCtrlReg1 |= YUVOUT;
	}

	// Whether compression or not?
	nReso = ((nWidthH == 320) && (nWidthV == 240)) ? 1 : 0;
	if (nReso == 1)								// VGA
	{
#ifdef	__TV_QVGA__
		R_CSI_TG_HRATIO = 0x0102;					// Scale to 1/2
		R_CSI_TG_VRATIO = 0x0102;					// Scale to 1/2
		R_CSI_TG_HWIDTH = nWidthH;					// Horizontal frame width
		R_CSI_TG_VHEIGHT = nWidthV*2;				// Vertical frame width
#endif	// __TV_QVGA__
	}
	else
	{
		R_CSI_TG_HRATIO = 0;
		R_CSI_TG_VRATIO = 0;
	}

	R_CSI_TG_VL0START = 0x0000;						// Sensor field 0 vertical latch start register.
	R_CSI_TG_VL1START = 0x0000;						//P_Sensor_TG_V_L1Start = 0x0000;
	R_CSI_TG_HSTART = 0x0000;						// Sensor horizontal start register.

	R_CSI_TG_CTRL0 = 0;								//reset control0
	R_CSI_TG_CTRL1 = CSI_NOSTOP|CLKOEN;				//enable CSI CLKO
	drv_msec_wait(100); 							//wait 100ms for CLKO stable

	// CMOS Sensor Initialization Start...
	sccb_init (SCCB_SCL, SCCB_SDA);
	sccb_delay (200);
	//if((nWidthH == 640) &&(nWidthV == 480))
	/////////////////guobin 20111124/////////////////////
    	{
                

                    //[Sensor]
            sccb_write(SLAVE_ID,0xfd, 0x00);   //  ;P0
            sccb_write(SLAVE_ID,0x1B, 0x02);//12//02
            //sccb_write(SLAVE_ID,0x1C, 0x07); 
            sccb_write(SLAVE_ID,0x12, 0x2f);
            sccb_write(SLAVE_ID,0x15, 0x3f);   
            sccb_write(SLAVE_ID,0x27, 0xe8);   	
            sccb_write(SLAVE_ID,0x28, 0x0b);//03     
            sccb_write(SLAVE_ID,0x32, 0x00);  
            sccb_write(SLAVE_ID,0x22, 0xc0);  
            sccb_write(SLAVE_ID,0x26, 0x10);  
            sccb_write(SLAVE_ID,0x31, 0x10);  //  ;Upside/mirr/Pclk inv/sub
            sccb_write(SLAVE_ID,0x5f, 0x11);  //  ;Bayer order
            sccb_write(SLAVE_ID,0xfd, 0x01);  //  ;P1
            sccb_write(SLAVE_ID,0x25, 0x1a);  //  ;Awb start
            sccb_write(SLAVE_ID,0x26, 0xfb);  
            sccb_write(SLAVE_ID,0x28, 0x75);//0x61
            sccb_write(SLAVE_ID,0x29, 0x4e);//0x49
            sccb_write(SLAVE_ID,0xfd, 0x00);//
            sccb_write(SLAVE_ID,0xe7, 0x03);//
            sccb_write(SLAVE_ID,0xe7, 0x00);//
            sccb_write(SLAVE_ID,0xfd, 0x01);// 
            sccb_write(SLAVE_ID,0x31, 0x60);//;64
            sccb_write(SLAVE_ID,0x32, 0x18);//
            sccb_write(SLAVE_ID,0x4d, 0xdc);//
            sccb_write(SLAVE_ID,0x4e, 0x53);//0x6b
            sccb_write(SLAVE_ID,0x41, 0x8c);//
            sccb_write(SLAVE_ID,0x42, 0x57);//0x66
            sccb_write(SLAVE_ID,0x55, 0xff);//
            sccb_write(SLAVE_ID,0x56, 0x00);//
            sccb_write(SLAVE_ID,0x59, 0x82);//
            sccb_write(SLAVE_ID,0x5a, 0x00);//
            sccb_write(SLAVE_ID,0x5d, 0xff);//
            sccb_write(SLAVE_ID,0x5e, 0x6f);//
            sccb_write(SLAVE_ID,0x57, 0xff);//
            sccb_write(SLAVE_ID,0x58, 0x00);//
            sccb_write(SLAVE_ID,0x5b, 0xff);//
            sccb_write(SLAVE_ID,0x5c, 0xa8);//
            sccb_write(SLAVE_ID,0x5f, 0x75);//
            sccb_write(SLAVE_ID,0x60, 0x00);//
            sccb_write(SLAVE_ID,0x2d, 0x00);//
            sccb_write(SLAVE_ID,0x2e, 0x00);//
            sccb_write(SLAVE_ID,0x2f, 0x00);//
            sccb_write(SLAVE_ID,0x30, 0x00);//
            sccb_write(SLAVE_ID,0x33, 0x00);//
            sccb_write(SLAVE_ID,0x34, 0x00);//
            sccb_write(SLAVE_ID,0x37, 0x00);//
            sccb_write(SLAVE_ID,0x38, 0x00);// ;awb end
            sccb_write(SLAVE_ID,0xfd, 0x00);// ;P0
            sccb_write(SLAVE_ID,0x33, 0x6f);// ;LSC BPC EN
            sccb_write(SLAVE_ID,0x51, 0x3f);// ;BPC debug start
            sccb_write(SLAVE_ID,0x52, 0x09);// 
            sccb_write(SLAVE_ID,0x53, 0x00);// 
            sccb_write(SLAVE_ID,0x54, 0x00);//
            sccb_write(SLAVE_ID,0x55, 0x10);// ;BPC debug end
            sccb_write(SLAVE_ID,0x4f, 0x08);//ff// 08;blueedge
            sccb_write(SLAVE_ID,0x50, 0x08);//ff// 08
            sccb_write(SLAVE_ID,0x57, 0x10);//40//10 ;Raw filter debut start
            sccb_write(SLAVE_ID,0x58, 0x10);//40// 10;4
            sccb_write(SLAVE_ID,0x59, 0x10);// ;4
            sccb_write(SLAVE_ID,0x56, 0x70);//
            sccb_write(SLAVE_ID,0x5a, 0x02);//
            sccb_write(SLAVE_ID,0x5b, 0x02);//
            sccb_write(SLAVE_ID,0x5c, 0x20);// ;Raw filter debut end 
            sccb_write(SLAVE_ID,0x65, 0x03);//03 ;Sharpness debug start
            sccb_write(SLAVE_ID,0x66, 0x01);//
            sccb_write(SLAVE_ID,0x67, 0x03);//
            sccb_write(SLAVE_ID,0x68, 0x43);//
            sccb_write(SLAVE_ID,0x69, 0x7f);//
            sccb_write(SLAVE_ID,0x6a, 0x01);//
            sccb_write(SLAVE_ID,0x6b, 0x05);//
            sccb_write(SLAVE_ID,0x6c, 0x01);//
            sccb_write(SLAVE_ID,0x6d, 0x03);// ;Edge gain normal
            sccb_write(SLAVE_ID,0x6e, 0x43);// ;Edge gain normal
            sccb_write(SLAVE_ID,0x6f, 0x7f);//
            sccb_write(SLAVE_ID,0x70, 0x01);//
            sccb_write(SLAVE_ID,0x71, 0x08);//05
            sccb_write(SLAVE_ID,0x72, 0x01);//10//01
            sccb_write(SLAVE_ID,0x73, 0x03);//
            sccb_write(SLAVE_ID,0x74, 0x43);//
            sccb_write(SLAVE_ID,0x75, 0x7f);//
            sccb_write(SLAVE_ID,0x76, 0x01);// ;Sharpness debug end
            sccb_write(SLAVE_ID,0xcb, 0x07);// ;HEQ&Saturation debug start 
            sccb_write(SLAVE_ID,0xcc, 0x04);//
            sccb_write(SLAVE_ID,0xce, 0xff);//
            sccb_write(SLAVE_ID,0xcf, 0x10);//
            sccb_write(SLAVE_ID,0xd0, 0x20);//
            sccb_write(SLAVE_ID,0xd1, 0x00);//
            sccb_write(SLAVE_ID,0xd2, 0x1c);//
            sccb_write(SLAVE_ID,0xd3, 0x16);//
            sccb_write(SLAVE_ID,0xd4, 0x00);//
            sccb_write(SLAVE_ID,0xd6, 0x1c);//
            sccb_write(SLAVE_ID,0xd7, 0x16);//
            sccb_write(SLAVE_ID,0xdd, 0x70);// ;Contrast
            sccb_write(SLAVE_ID,0xde, 0x90);// ;HEQ&Saturation debug end
            sccb_write(SLAVE_ID,0x7f, 0xd7);//   ;Color Correction start 
            sccb_write(SLAVE_ID,0x80, 0xbc);// 
            sccb_write(SLAVE_ID,0x81, 0xed);// 
            sccb_write(SLAVE_ID,0x82, 0xd7);// 
            sccb_write(SLAVE_ID,0x83, 0xd4);//
            sccb_write(SLAVE_ID,0x84, 0xd6);//
            sccb_write(SLAVE_ID,0x85, 0xff);//
            sccb_write(SLAVE_ID,0x86, 0x89);//55 
            sccb_write(SLAVE_ID,0x87, 0xf8);//2c 
            sccb_write(SLAVE_ID,0x88, 0x3c);//
            sccb_write(SLAVE_ID,0x89, 0x33);//
            sccb_write(SLAVE_ID,0x8a, 0x0f);//
            sccb_write(SLAVE_ID,0x8b, 0x0 );// ;gamma start
            sccb_write(SLAVE_ID,0x8c, 0x1a);//
            sccb_write(SLAVE_ID,0x8d, 0x29);//
            sccb_write(SLAVE_ID,0x8e, 0x41);//
            sccb_write(SLAVE_ID,0x8f, 0x62);//
            sccb_write(SLAVE_ID,0x90, 0x7c);//
            sccb_write(SLAVE_ID,0x91, 0x90);//
            sccb_write(SLAVE_ID,0x92, 0xa2);//
            sccb_write(SLAVE_ID,0x93, 0xaf);//
            sccb_write(SLAVE_ID,0x94, 0xbc);//
            sccb_write(SLAVE_ID,0x95, 0xc5);//
            sccb_write(SLAVE_ID,0x96, 0xcd);//
            sccb_write(SLAVE_ID,0x97, 0xd5);//
            sccb_write(SLAVE_ID,0x98, 0xdd);//
            sccb_write(SLAVE_ID,0x99, 0xe5);//
            sccb_write(SLAVE_ID,0x9a, 0xed);//
            sccb_write(SLAVE_ID,0x9b, 0xf5);//
            sccb_write(SLAVE_ID,0xfd, 0x01);// ;P1
            sccb_write(SLAVE_ID,0x8d, 0xfd);//
            sccb_write(SLAVE_ID,0x8e, 0xff);// ;gamma end
            sccb_write(SLAVE_ID,0xfd, 0x00);// ;P0
            sccb_write(SLAVE_ID,0xca, 0xcf);//
            sccb_write(SLAVE_ID,0xd8, 0x50);//0x65  ;UV outdoor
            sccb_write(SLAVE_ID,0xd9, 0x50);//0x65  ;UV indoor 
            sccb_write(SLAVE_ID,0xda, 0x48);//0x65  ;UV dummy
            sccb_write(SLAVE_ID,0xdb, 0x40);//0x50  ;UV lowlight
            sccb_write(SLAVE_ID,0xb9, 0x00);// ;Ygamma start
            sccb_write(SLAVE_ID,0xba, 0x04);//
            sccb_write(SLAVE_ID,0xbb, 0x08);//
            sccb_write(SLAVE_ID,0xbc, 0x10);//
            sccb_write(SLAVE_ID,0xbd, 0x20);//
            sccb_write(SLAVE_ID,0xbe, 0x30);//
            sccb_write(SLAVE_ID,0xbf, 0x40);//
            sccb_write(SLAVE_ID,0xc0, 0x50);//
            sccb_write(SLAVE_ID,0xc1, 0x60);//
            sccb_write(SLAVE_ID,0xc2, 0x70);//
            sccb_write(SLAVE_ID,0xc3, 0x80);//
            sccb_write(SLAVE_ID,0xc4, 0x90);//
            sccb_write(SLAVE_ID,0xc5, 0xA0);//
            sccb_write(SLAVE_ID,0xc6, 0xB0);//
            sccb_write(SLAVE_ID,0xc7, 0xC0);//
            sccb_write(SLAVE_ID,0xc8, 0xD0);//
            sccb_write(SLAVE_ID,0xc9, 0xE0);//
            sccb_write(SLAVE_ID,0xfd, 0x01);// ;P1
            sccb_write(SLAVE_ID,0x89, 0xf0);//
            sccb_write(SLAVE_ID,0x8a, 0xff);// ;Ygamma end
            sccb_write(SLAVE_ID,0xfd, 0x00);// ;P0
            sccb_write(SLAVE_ID,0xe8, 0x30);// ;AEdebug start
            sccb_write(SLAVE_ID,0xe9, 0x30);//
            sccb_write(SLAVE_ID,0xea, 0x40);// ;Alc Window sel
            sccb_write(SLAVE_ID,0xf4, 0x1b);// ;outdoor mode sel
            sccb_write(SLAVE_ID,0xf5, 0x80);//
            sccb_write(SLAVE_ID,0xf7, 0x88);//78 ;AE target 
            sccb_write(SLAVE_ID,0xf8, 0x73);//63 
            sccb_write(SLAVE_ID,0xf9, 0x78);// ;AE target 
            sccb_write(SLAVE_ID,0xfa, 0x63);//
            sccb_write(SLAVE_ID,0xfd, 0x01);// ;P1
            sccb_write(SLAVE_ID,0x09, 0x31);// ;AE Step 3.0
            sccb_write(SLAVE_ID,0x0a, 0x85);//
            sccb_write(SLAVE_ID,0x0b, 0x0b);// ;AE Step 3.0
            sccb_write(SLAVE_ID,0x14, 0x20);//20
            sccb_write(SLAVE_ID,0x15, 0x0f);//

        //27M 50Hz 25-25fps gain:0x70
          sccb_write(SLAVE_ID,0xfd,0x00);
          sccb_write(SLAVE_ID,0x05,0x0 );
          sccb_write(SLAVE_ID,0x06,0x0 );
          sccb_write(SLAVE_ID,0x09,0x0 );
          sccb_write(SLAVE_ID,0x0a,0xfc);
          sccb_write(SLAVE_ID,0xf0,0x7b);
          sccb_write(SLAVE_ID,0xf1,0x0 );
          sccb_write(SLAVE_ID,0xf2,0x65);
          sccb_write(SLAVE_ID,0xf5,0x7f);
          sccb_write(SLAVE_ID,0xfd,0x01);
          sccb_write(SLAVE_ID,0x00,0xa4);
          sccb_write(SLAVE_ID,0x0f,0x66);
          sccb_write(SLAVE_ID,0x16,0x66);
          sccb_write(SLAVE_ID,0x17,0x94);
          sccb_write(SLAVE_ID,0x18,0x9c);
          sccb_write(SLAVE_ID,0x1b,0x66);
          sccb_write(SLAVE_ID,0x1c,0x9c);
          sccb_write(SLAVE_ID,0xb4,0x21);
          sccb_write(SLAVE_ID,0xb5,0x3b);
          sccb_write(SLAVE_ID,0xb6,0x73);
          sccb_write(SLAVE_ID,0xb9,0x40);
          sccb_write(SLAVE_ID,0xba,0x4f);
          sccb_write(SLAVE_ID,0xbb,0x47);
          sccb_write(SLAVE_ID,0xbc,0x45);
          sccb_write(SLAVE_ID,0xbd,0x70);
          sccb_write(SLAVE_ID,0xbe,0x42);
          sccb_write(SLAVE_ID,0xbf,0x42);
          sccb_write(SLAVE_ID,0xc0,0x42);
          sccb_write(SLAVE_ID,0xc1,0x41);
          sccb_write(SLAVE_ID,0xc2,0x41);
          sccb_write(SLAVE_ID,0xc3,0x41);
          sccb_write(SLAVE_ID,0xc4,0x41);
          sccb_write(SLAVE_ID,0xc5,0x41);
          sccb_write(SLAVE_ID,0xc6,0x41);
          sccb_write(SLAVE_ID,0xca,0x70);
          sccb_write(SLAVE_ID,0xcb,0x4 );
          sccb_write(SLAVE_ID,0xfd,0x00);



        /*
        //27M 50Hz 20-20fps gain:0x60
          sccb_write(SLAVE_ID,0xfd,0x00);
          sccb_write(SLAVE_ID,0x05,0x0 );
          sccb_write(SLAVE_ID,0x06,0x0 );
          sccb_write(SLAVE_ID,0x09,0x2 );
          sccb_write(SLAVE_ID,0x0a,0xf );
          sccb_write(SLAVE_ID,0xf0,0x62);
          sccb_write(SLAVE_ID,0xf1,0x0 );
          sccb_write(SLAVE_ID,0xf2,0x5f);
          sccb_write(SLAVE_ID,0xf5,0x78);
          sccb_write(SLAVE_ID,0xfd,0x01);
          sccb_write(SLAVE_ID,0x00,0x91);
          sccb_write(SLAVE_ID,0x0f,0x60);
          sccb_write(SLAVE_ID,0x16,0x60);
          sccb_write(SLAVE_ID,0x17,0x91);
          sccb_write(SLAVE_ID,0x18,0x1 );
          sccb_write(SLAVE_ID,0x1b,0x60);
          sccb_write(SLAVE_ID,0x1c,0xd0);
          sccb_write(SLAVE_ID,0xb4,0x20);
          sccb_write(SLAVE_ID,0xb5,0x3a);
          sccb_write(SLAVE_ID,0xb6,0x5e);
          sccb_write(SLAVE_ID,0xb9,0x40);
          sccb_write(SLAVE_ID,0xba,0x4f);
          sccb_write(SLAVE_ID,0xbb,0x47);
          sccb_write(SLAVE_ID,0xbc,0x45);
          sccb_write(SLAVE_ID,0xbd,0x43);
          sccb_write(SLAVE_ID,0xbe,0x60);
          sccb_write(SLAVE_ID,0xbf,0x42);
          sccb_write(SLAVE_ID,0xc0,0x42);
          sccb_write(SLAVE_ID,0xc1,0x41);
          sccb_write(SLAVE_ID,0xc2,0x41);
          sccb_write(SLAVE_ID,0xc3,0x41);
          sccb_write(SLAVE_ID,0xc4,0x41);
          sccb_write(SLAVE_ID,0xc5,0x41);
          sccb_write(SLAVE_ID,0xc6,0x41);
          sccb_write(SLAVE_ID,0xca,0x60);
          sccb_write(SLAVE_ID,0xcb,0x5 );
          sccb_write(SLAVE_ID,0xfd,0x00);
        */

        /*
        //27M 50Hz 20-8fps gain:0x60
          sccb_write(SLAVE_ID,0xfd,0x00);
          sccb_write(SLAVE_ID,0x05,0x0 );
          sccb_write(SLAVE_ID,0x06,0x0 );
          sccb_write(SLAVE_ID,0x09,0x2 );
          sccb_write(SLAVE_ID,0x0a,0x10);
          sccb_write(SLAVE_ID,0xf0,0x62);
          sccb_write(SLAVE_ID,0xf1,0x0 );
          sccb_write(SLAVE_ID,0xf2,0x5f);
          sccb_write(SLAVE_ID,0xf5,0x78);
          sccb_write(SLAVE_ID,0xfd,0x01);
          sccb_write(SLAVE_ID,0x00,0xa2);
          sccb_write(SLAVE_ID,0x0f,0x60);
          sccb_write(SLAVE_ID,0x16,0x60);
          sccb_write(SLAVE_ID,0x17,0xa2);
          sccb_write(SLAVE_ID,0x18,0x1 );
          sccb_write(SLAVE_ID,0x1b,0x60);
          sccb_write(SLAVE_ID,0x1c,0xd0);
          sccb_write(SLAVE_ID,0xb4,0x20);
          sccb_write(SLAVE_ID,0xb5,0x3a);
          sccb_write(SLAVE_ID,0xb6,0x5e);
          sccb_write(SLAVE_ID,0xb9,0x40);
          sccb_write(SLAVE_ID,0xba,0x4f);
          sccb_write(SLAVE_ID,0xbb,0x47);
          sccb_write(SLAVE_ID,0xbc,0x45);
          sccb_write(SLAVE_ID,0xbd,0x43);
          sccb_write(SLAVE_ID,0xbe,0x42);
          sccb_write(SLAVE_ID,0xbf,0x42);
          sccb_write(SLAVE_ID,0xc0,0x42);
          sccb_write(SLAVE_ID,0xc1,0x41);
          sccb_write(SLAVE_ID,0xc2,0x41);
          sccb_write(SLAVE_ID,0xc3,0x41);
          sccb_write(SLAVE_ID,0xc4,0x41);
          sccb_write(SLAVE_ID,0xc5,0x60);
          sccb_write(SLAVE_ID,0xc6,0x41);
          sccb_write(SLAVE_ID,0xca,0x60);
          sccb_write(SLAVE_ID,0xcb,0xc );
          sccb_write(SLAVE_ID,0xfd,0x00);
        */

         /*
         //27M 50Hz 15-8fps gain:0x60
        sccb_write(SLAVE_ID,0xfd, 0x00);
        sccb_write(SLAVE_ID,0x05, 0x0 );
        sccb_write(SLAVE_ID,0x06, 0x0 );
        sccb_write(SLAVE_ID,0x09, 0x3 );
        sccb_write(SLAVE_ID,0x0a, 0xdb);
        sccb_write(SLAVE_ID,0xf0, 0x49);
        sccb_write(SLAVE_ID,0xf1, 0x0 );
        sccb_write(SLAVE_ID,0xf2, 0x5a);
        sccb_write(SLAVE_ID,0xf5, 0x73);
        sccb_write(SLAVE_ID,0xfd, 0x01);
        sccb_write(SLAVE_ID,0x00, 0x9d);
        sccb_write(SLAVE_ID,0x0f, 0x5b);
        sccb_write(SLAVE_ID,0x16, 0x5b);
        sccb_write(SLAVE_ID,0x17, 0x9d);
        sccb_write(SLAVE_ID,0x18, 0x1 );
        sccb_write(SLAVE_ID,0x1b, 0x5b);
        sccb_write(SLAVE_ID,0x1c, 0xd0);
        sccb_write(SLAVE_ID,0xb4, 0x21);
        sccb_write(SLAVE_ID,0xb5, 0x3d);
        sccb_write(SLAVE_ID,0xb6, 0x45);
        sccb_write(SLAVE_ID,0xb9, 0x40);
        sccb_write(SLAVE_ID,0xba, 0x4f);
        sccb_write(SLAVE_ID,0xbb, 0x47);
        sccb_write(SLAVE_ID,0xbc, 0x45);
        sccb_write(SLAVE_ID,0xbd, 0x43);
        sccb_write(SLAVE_ID,0xbe, 0x42);
        sccb_write(SLAVE_ID,0xbf, 0x42);
        sccb_write(SLAVE_ID,0xc0, 0x42);
        sccb_write(SLAVE_ID,0xc1, 0x41);
        sccb_write(SLAVE_ID,0xc2, 0x41);
        sccb_write(SLAVE_ID,0xc3, 0x41);
        sccb_write(SLAVE_ID,0xc4, 0x41);
        sccb_write(SLAVE_ID,0xc5, 0x60);
        sccb_write(SLAVE_ID,0xc6, 0x41);
        sccb_write(SLAVE_ID,0xca, 0x60);
        sccb_write(SLAVE_ID,0xcb, 0xc );
        sccb_write(SLAVE_ID,0xfd, 0x00);
        */



        /* //27M 50Hz 15-8fps gain:0x70
          sccb_write(SLAVE_ID,0xfd, 0x00);
          sccb_write(SLAVE_ID,0x05, 0x0 );
          sccb_write(SLAVE_ID,0x06, 0x0 );
          sccb_write(SLAVE_ID,0x09, 0x3 );
          sccb_write(SLAVE_ID,0x0a, 0xda);
          sccb_write(SLAVE_ID,0xf0, 0x4a);
          sccb_write(SLAVE_ID,0xf1, 0x0 );
          sccb_write(SLAVE_ID,0xf2, 0x59);
          sccb_write(SLAVE_ID,0xf5, 0x72);
          sccb_write(SLAVE_ID,0xfd, 0x01);
          sccb_write(SLAVE_ID,0x00, 0xac);
          sccb_write(SLAVE_ID,0x0f, 0x5a);
          sccb_write(SLAVE_ID,0x16, 0x5a);
          sccb_write(SLAVE_ID,0x17, 0x9c);
          sccb_write(SLAVE_ID,0x18, 0xa4);
          sccb_write(SLAVE_ID,0x1b, 0x5a);
          sccb_write(SLAVE_ID,0x1c, 0xa4);
          sccb_write(SLAVE_ID,0xb4, 0x20);
          sccb_write(SLAVE_ID,0xb5, 0x3a);
          sccb_write(SLAVE_ID,0xb6, 0x46);
          sccb_write(SLAVE_ID,0xb9, 0x40);
          sccb_write(SLAVE_ID,0xba, 0x4f);
          sccb_write(SLAVE_ID,0xbb, 0x47);
          sccb_write(SLAVE_ID,0xbc, 0x45);
          sccb_write(SLAVE_ID,0xbd, 0x43);
          sccb_write(SLAVE_ID,0xbe, 0x42);
          sccb_write(SLAVE_ID,0xbf, 0x42);
          sccb_write(SLAVE_ID,0xc0, 0x42);
          sccb_write(SLAVE_ID,0xc1, 0x41);
          sccb_write(SLAVE_ID,0xc2, 0x41);
          sccb_write(SLAVE_ID,0xc3, 0x41);
          sccb_write(SLAVE_ID,0xc4, 0x41);
          sccb_write(SLAVE_ID,0xc5, 0x70);
          sccb_write(SLAVE_ID,0xc6, 0x41);
          sccb_write(SLAVE_ID,0xca, 0x70);
          sccb_write(SLAVE_ID,0xcb, 0xc );
          sccb_write(SLAVE_ID,0xfd, 0x00);
        */

            sccb_write(SLAVE_ID,0xfd, 0x00);// ;P0
            sccb_write(SLAVE_ID,0x32, 0x15);// ;Auto_mode set
            sccb_write(SLAVE_ID,0x34, 0x66);// ;Isp_mode set
            sccb_write(SLAVE_ID,0x35, 0x40);// ;out format
                                                                       
    	}
	R_CSI_TG_CTRL1 = uCtrlReg2;					//*P_Sensor_TG_Ctrl2 = uCtrlReg2;
#if CSI_MODE == CSI_PPU_FRAME_MODE
	R_CSI_TG_CTRL0 = uCtrlReg1;					//*P_Sensor_TG_Ctrl1 = uCtrlReg1;
#elif CSI_MODE == CSI_TG_FRAME_MODE
	R_CSI_TG_CTRL0 = uCtrlReg1|0x010000;
#elif CSI_MODE == CSI_FIFO_8_MODE
	R_CSI_TG_CTRL0 = uCtrlReg1|0x110000;
#elif CSI_MODE == CSI_FIFO_16_MODE
	R_CSI_TG_CTRL0 = uCtrlReg1|0x210000;
#elif CSI_MODE == CSI_FIFO_32_MODE
	R_CSI_TG_CTRL0 = uCtrlReg1|0x310000;
#endif
}
#endif

//====================================================================================================
//	Description:	Initial CMOS sensor
//	Function:		CSI_Init (nWidthH, nWidthV, uFlag, uFrmBuf0, uFrmBuf1, uFrmBuf2)
//	Syntax:			void CSI_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag, INT32U uFrmBuf0, INT32U uFrmBuf1, INT32U uFrmBuf2)
//	Input Paramter:	INT16S nWidthH: Active horizontal width
//					INT16S nWidthV: Active vertical width
//					INT16U uFlag: Flag type selection
//					INT32U uFrmBuf0: Frame buffer index 0
//	Return: 		none
//====================================================================================================
void CSI_Init (INT16S nWidthH, INT16S nWidthV, INT16U uFlag, INT32U uFrmBuf0, INT32U uFrmBuf1)
{
	// Setup sensor frame buffer start address
	*P_CSI_TG_FBSADDR = uFrmBuf0;
	if(frame_mode_en == 0) {
		*P_CSI_TG_FBSADDR_B = uFrmBuf1;
	}

	// Setup sensor frame size
	R_CSI_TG_HWIDTH = nWidthH;				// Horizontal frame width
  #if SENSOR_WIDTH==640 && AVI_WIDTH==720	// VGA(640x480) to D1(720x480) engine is used
	R_CSI_TG_VHEIGHT = 448;				// Vertical frame width

	R_CSI_TG_CUTSTART = 0x0100;
	R_CSI_TG_CUTSIZE = 0x1B28;
  #else
	R_CSI_TG_VHEIGHT = nWidthV;			// Vertical frame width
  #endif
	//	CMOS Sensor Interface (CSI) H/W Initialization
	//
#ifdef	__OV6680_DRV_C__
	OV6680_Init (nWidthH, nWidthV, uFlag);
#endif
#ifdef	__OV7680_DRV_C__
	OV7680_Init (nWidthH, nWidthV, uFlag);
#endif
#ifdef	__OV7670_DRV_C__
	OV7670_Init (nWidthH, nWidthV, uFlag);
#endif
#ifdef	__OV7725_DRV_C__
	OV7725_Init (nWidthH, nWidthV, uFlag);
#endif
#ifdef	__OV9655_DRV_C__
	OV9655_Init (nWidthH, nWidthV, uFlag);
#endif
#ifdef	__OV2655_DRV_C__
	OV2655_Init (nWidthH, nWidthV, uFlag);
#endif
#ifdef	__OV3640_DRV_C__
	OV3640_Init (nWidthH, nWidthV, uFlag);
#endif
#ifdef	__OV5642_DRV_C__
	OV5642_Init (nWidthH, nWidthV, uFlag);
#endif
#ifdef	__OV7675_DRV_C__
	OV7675_Init (nWidthH, nWidthV, uFlag);
#endif
#ifdef	__GC0308_DRV_C__
	GC0308_Init (nWidthH, nWidthV, uFlag);
#endif
#ifdef	__SP0838_DRV_C__
	SP0838_Init (nWidthH, nWidthV, uFlag);
#endif
#ifdef  __OV2643_DRV_C__
    OV2643_Init (nWidthH, nWidthV, uFlag);
#endif

#if SENSOR_WIDTH==640 && AVI_WIDTH==720		// VGA(640x480) to D1(720x480) engine is used
	R_CSI_TG_CTRL0 |= (1<<17);				// Enable VGA to D1 scale up
	R_CSI_TG_CTRL1 |= 0x0100;				// Enable Cut function for ratio calibration
#endif

}
#endif
//====================================================================================================
//	Description:	Wait capture delay complete
//	Function:		CSI_WaitCaptureComplete ()
//	Syntax:			void CSI_WaitCaptureComplete (void)
//	Input Paramter:	none
//	Return: 		none
//====================================================================================================
void CSI_WaitCaptureComplete (void)
{
	while ((R_PPU_IRQ_STATUS & 0x40) == 0);
	R_PPU_IRQ_STATUS = 0x40;
}

void Sensor_Bluescreen_Enable(void) {
	R_CSI_TG_CTRL0 |= 0x0080;
	R_CSI_TG_BSUPPER = 0x001f;
	R_CSI_TG_BSLOWER = 0x0000;
}
void Sensor_Bluescreen_Disable() {
	R_CSI_TG_CTRL0 &= 0xFF7F;
}

void Sensor_Cut_Enable(void) {
	R_CSI_TG_CTRL1 |= 0x0100;	//Enable Cut function
	R_CSI_TG_CUTSTART = 0x0808;
	R_CSI_TG_CUTSIZE  = 0x0808;
}
void Sensor_Cut_Disable(void) {
	R_CSI_TG_CTRL1 &= 0xFEFF;
}

void Sensor_Black_Enable(void) {
	R_CSI_TG_HSTART = 0x50;
	R_CSI_TG_VSTART = 0x28;
	R_CSI_TG_HEND = 0xf0;
	R_CSI_TG_VEND = 0xc8;
}

void Sensor_Black_Disable(void) {
	R_CSI_TG_HSTART = 0x0000;
	R_CSI_TG_VSTART = 0x0000;
	R_CSI_TG_HEND = 0x0140;
	R_CSI_TG_VEND = 0x00f0;
}
#if C_MOTION_DETECTION == CUSTOM_ON
void Sensor_MotionDection_Inital(INT32U buff)
{ 
   	R_CSI_MD_CTRL = MD_VGA|MD_FRAME_2|MD_THRESHOLD|MD_MODE_SINGLEIIR;
   	*P_CSI_MD_FBADDR = buff; 		
}

void Sensor_MotionDection_stop(void)
{
	R_CSI_MD_CTRL &= (~MD_EN);
	R_PPU_IRQ_EN &= ~0x80;
 	R_PPU_IRQ_STATUS |= 0x80;
}
void Sensor_MotionDection_start(void)
{
	R_CSI_MD_CTRL |= (MD_EN);
	R_PPU_IRQ_STATUS |= 0x80;
	R_PPU_IRQ_EN |= 0x80;
}
//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#endif //(defined _DRV_L1_SENSOR) && (_DRV_L1_SENSOR == 1)        //
//================================================================//