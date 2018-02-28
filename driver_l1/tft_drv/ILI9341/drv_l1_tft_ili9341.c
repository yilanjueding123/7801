#include"drv_l1_tft_ili9341.h"

#if (USE_PANEL_NAME == PANEL_400X240_I80)

void ILI9341_WR_COMD(unsigned int comm)//write command
{    
	 gpio_write_io (LCD_RS, DATA_LOW); 
     gpio_write_io (LCD_RD, DATA_HIGH);
     gpio_write_io (LCD_CS, DATA_LOW);  
     gpio_write_io (LCD_WR, DATA_LOW);
     R_IOA_O_DATA=comm;
     gpio_write_io (LCD_WR, DATA_HIGH);
   /*
     R_TFT_MEM_BUFF_WR = comm;
     drv_msec_wait(1); 
     R_TFT_CTRL = 0x208F;
     drv_msec_wait(1);*/
}

void ILI9341_WR_Parameter(unsigned int data)//write Parameter
{    
     gpio_write_io (LCD_RD, DATA_HIGH);
     gpio_write_io (LCD_RS, DATA_HIGH);
     gpio_write_io (LCD_CS, DATA_LOW);  
     gpio_write_io (LCD_WR, DATA_LOW);
     R_IOA_O_DATA=data; 
     gpio_write_io (LCD_WR, DATA_HIGH);
    /*
     R_TFT_MEM_BUFF_WR = data;
     drv_msec_wait(1); 
     R_TFT_CTRL = 0x20AF;
     drv_msec_wait(1); */
}

void ILI9341_WR_DATD(unsigned int data)//write data
{    
     gpio_write_io (LCD_RD, DATA_HIGH);
     gpio_write_io (LCD_RS, DATA_HIGH);
     gpio_write_io (LCD_CS, DATA_LOW);  
     gpio_write_io (LCD_WR, DATA_LOW);
     R_IOA_O_DATA=data>>8; 
     gpio_write_io (LCD_WR, DATA_HIGH);
     
     gpio_write_io (LCD_WR, DATA_LOW);
     R_IOA_O_DATA=data&0xFF; 
     gpio_write_io (LCD_WR, DATA_HIGH);
  /*
	  R_TFT_MEM_BUFF_WR = data>>8;
	  R_TFT_CTRL = 0x20AF;

	  R_TFT_MEM_BUFF_WR = data&0xFF;
	  R_TFT_CTRL = 0x20AF;*/
}

INT32U ILI9341_RD_Parameter(void)//READ Parameter
{    
     INT32U RD_data;
   
   	 R_IOA_ATTRIB = 0x00FF;
     R_IOA_DIR = 0x0000;
     R_IOA_O_DATA = 0x0000; 
   
     gpio_write_io (LCD_RS, DATA_HIGH);
     gpio_write_io (LCD_CS, DATA_LOW);  
     gpio_write_io (LCD_WR, DATA_HIGH);   
     gpio_write_io (LCD_RD, DATA_LOW);
   
     gpio_write_io (LCD_RD, DATA_HIGH);
     RD_data =R_IOA_O_DATA; 
     gpio_write_io (LCD_RD, DATA_LOW);
    
     gpio_write_io (LCD_RD, DATA_HIGH);
     RD_data =R_IOA_O_DATA;
      /*
    // R_TFT_CTRL = 0x20BF;
     RD_data = R_TFT_MEM_BUFF_RD;
     R_TFT_CTRL = 0x20BF;
     */
     
    R_IOA_ATTRIB = 0x00ff;
    R_IOA_DIR = 0x00ff;
    R_IOA_O_DATA = 0; 
     return RD_data;
     
   
}


void TFT_ILI9341_Clear(unsigned int Color)
{
	INT16U i, j;
	ILI9341_WR_COMD(0x2c);
	for(i = 0; i < 400; i++)
	{
	    drv_msec_wait (1);
		for(j = 0; j < 240; j++)
		   ILI9341_WR_DATD(Color);
		   }
}


//=============================================================================
extern void ap_display_TE_sync_I80_start(void);
extern void TEST_Display_Data_180_degree_set(INT8U enable);
void TFT_D51E5TA8566_Init(void)
{     
//-- if LCD_RST = IOB5
	R_I2C_MISC = 0;
//--
  
	R_IOA_ATTRIB |= 0x00ff;
    R_IOA_DIR    |= 0x00ff;
    R_IOA_O_DATA &= 0xff00; 
	  
	gpio_set_port_attribute(LCD_CS, ATTRIBUTE_HIGH);
	gpio_set_port_attribute(LCD_RS, ATTRIBUTE_HIGH);
	gpio_set_port_attribute(LCD_WR, ATTRIBUTE_HIGH);
	gpio_set_port_attribute(LCD_RD, ATTRIBUTE_HIGH);
	
	gpio_init_io(LCD_CS, GPIO_OUTPUT);					 	
	gpio_init_io(LCD_RS, GPIO_OUTPUT);
	gpio_init_io(LCD_WR, GPIO_OUTPUT);	
	gpio_init_io(LCD_RD, GPIO_OUTPUT);					 	
		
	gpio_write_io (LCD_CS, DATA_LOW);
	gpio_write_io (LCD_RS, DATA_LOW);
	gpio_write_io (LCD_WR, DATA_LOW);
	gpio_write_io (LCD_RD, DATA_LOW);

	   
//=======initializing funciton 1 ===============================

	// sleep out and wait at least 5ms for the next operations
	//ILI9341_WR_COMD(0x11); //Exit Sleep
	//drv_msec_wait(120);

	ILI9341_WR_COMD(0xba);
	ILI9341_WR_Parameter(0x85);
	ILI9341_WR_Parameter(0x66);

	ILI9341_WR_COMD(0xb6);
	ILI9341_WR_Parameter(0x26);
	ILI9341_WR_Parameter(0x03);
	ILI9341_WR_Parameter(0x10);

	ILI9341_WR_COMD(0xb7);
	ILI9341_WR_Parameter(0x66);
	ILI9341_WR_Parameter(0x44);
	ILI9341_WR_Parameter(0x01);

	ILI9341_WR_COMD(0xb8);
	ILI9341_WR_Parameter(0x68); //70 duibidu: 0x68 5.496v   
	ILI9341_WR_Parameter(0x54);
	ILI9341_WR_Parameter(0x11);
	ILI9341_WR_Parameter(0x00);


	ILI9341_WR_COMD(0xb0);
	ILI9341_WR_Parameter(0x00);
	ILI9341_WR_Parameter(0x00);
	ILI9341_WR_Parameter(0x0a);
	ILI9341_WR_Parameter(0x0f);

	ILI9341_WR_COMD(0xb1);
	ILI9341_WR_Parameter(0x01);
	ILI9341_WR_Parameter(0x90); // b0

	ILI9341_WR_COMD(0xb2);
	ILI9341_WR_Parameter(0x53);
	ILI9341_WR_Parameter(0x4f);
	ILI9341_WR_Parameter(0x11);
	ILI9341_WR_Parameter(0x13);
	ILI9341_WR_Parameter(0x51);
	ILI9341_WR_Parameter(0x20);
	ILI9341_WR_Parameter(0x66);
	ILI9341_WR_Parameter(0x12);
	ILI9341_WR_Parameter(0x36);

	ILI9341_WR_COMD(0xb3);
	ILI9341_WR_Parameter(0xb4);
	ILI9341_WR_Parameter(0x88);
	ILI9341_WR_Parameter(0x00);
	ILI9341_WR_Parameter(0x00);

/*
	ILI9341_WR_COMD(0xE0); //Set Gamma
	ILI9341_WR_Parameter(0x0F);
	ILI9341_WR_Parameter(0x20);
	ILI9341_WR_Parameter(0x1E);
	ILI9341_WR_Parameter(0x07);
	ILI9341_WR_Parameter(0x0A);
	ILI9341_WR_Parameter(0x03);
	ILI9341_WR_Parameter(0x52);
	ILI9341_WR_Parameter(0x63);
	ILI9341_WR_Parameter(0x44);
	ILI9341_WR_Parameter(0x08);
	ILI9341_WR_Parameter(0x17);
	ILI9341_WR_Parameter(0x09);
	ILI9341_WR_Parameter(0x19);
	ILI9341_WR_Parameter(0x13);
	ILI9341_WR_Parameter(0x00);

	ILI9341_WR_COMD(0xE1); //Set Gamma
	ILI9341_WR_Parameter(0x00);
	ILI9341_WR_Parameter(0x16);
	ILI9341_WR_Parameter(0x19);
	ILI9341_WR_Parameter(0x02);
	ILI9341_WR_Parameter(0x0F);
	ILI9341_WR_Parameter(0x03);
	ILI9341_WR_Parameter(0x2F);
	ILI9341_WR_Parameter(0x13);
	ILI9341_WR_Parameter(0x40);
	ILI9341_WR_Parameter(0x01);
	ILI9341_WR_Parameter(0x08);
	ILI9341_WR_Parameter(0x07);
	ILI9341_WR_Parameter(0x2E);
	ILI9341_WR_Parameter(0x3C);
	ILI9341_WR_Parameter(0x0F);
*/

	ILI9341_WR_COMD(0xc0);
	ILI9341_WR_Parameter(0x00);
	ILI9341_WR_Parameter(0x23);
	ILI9341_WR_Parameter(0x00);
	ILI9341_WR_Parameter(0x16);
	ILI9341_WR_Parameter(0x14);
	ILI9341_WR_Parameter(0x27);
	ILI9341_WR_Parameter(0x2e);
	ILI9341_WR_Parameter(0x2e);
	ILI9341_WR_Parameter(0x24);
	ILI9341_WR_Parameter(0x21);
	ILI9341_WR_Parameter(0x1d);
	ILI9341_WR_Parameter(0x1e);
	ILI9341_WR_Parameter(0x0d);
	ILI9341_WR_Parameter(0x08);
	ILI9341_WR_Parameter(0x3c);
	ILI9341_WR_Parameter(0x1c);
	ILI9341_WR_Parameter(0x3f);
	ILI9341_WR_Parameter(0x3f);
	ILI9341_WR_Parameter(0x3c);
	ILI9341_WR_Parameter(0x30);
	ILI9341_WR_Parameter(0x28);
	ILI9341_WR_Parameter(0x1e);
	ILI9341_WR_Parameter(0x16);
	ILI9341_WR_Parameter(0x08);
	ILI9341_WR_Parameter(0x09);
	ILI9341_WR_Parameter(0x0d);
	ILI9341_WR_Parameter(0x1f);
	ILI9341_WR_Parameter(0x2a);
	ILI9341_WR_Parameter(0x30);
	ILI9341_WR_Parameter(0x3f);
	ILI9341_WR_Parameter(0x00);
	ILI9341_WR_Parameter(0xc1);
	
	ILI9341_WR_COMD(0xc1);
	ILI9341_WR_Parameter(0x00);
	ILI9341_WR_Parameter(0x23);
	ILI9341_WR_Parameter(0x00);
	ILI9341_WR_Parameter(0x16);
	ILI9341_WR_Parameter(0x14);
	ILI9341_WR_Parameter(0x27);
	ILI9341_WR_Parameter(0x2e);
	ILI9341_WR_Parameter(0x2e);
	ILI9341_WR_Parameter(0x24);
	ILI9341_WR_Parameter(0x21);
	ILI9341_WR_Parameter(0x1d);
	ILI9341_WR_Parameter(0x1e);
	ILI9341_WR_Parameter(0x0d);
	ILI9341_WR_Parameter(0x08);
	ILI9341_WR_Parameter(0x3c);
	ILI9341_WR_Parameter(0x1c);
	ILI9341_WR_Parameter(0x3f);
	ILI9341_WR_Parameter(0x3f);
	ILI9341_WR_Parameter(0x3c);
	ILI9341_WR_Parameter(0x30);
	ILI9341_WR_Parameter(0x28);
	ILI9341_WR_Parameter(0x1e);
	ILI9341_WR_Parameter(0x16);
	ILI9341_WR_Parameter(0x08);
	ILI9341_WR_Parameter(0x09);
	ILI9341_WR_Parameter(0x0d);
	ILI9341_WR_Parameter(0x1f);
	ILI9341_WR_Parameter(0x2a);
	ILI9341_WR_Parameter(0x30);
	ILI9341_WR_Parameter(0x3f);
	ILI9341_WR_Parameter(0x00);
	ILI9341_WR_Parameter(0xc1);
	
	ILI9341_WR_COMD(0xc2);
	ILI9341_WR_Parameter(0x00);
	ILI9341_WR_Parameter(0x23);
	ILI9341_WR_Parameter(0x00);
	ILI9341_WR_Parameter(0x16);
	ILI9341_WR_Parameter(0x14);
	ILI9341_WR_Parameter(0x27);
	ILI9341_WR_Parameter(0x2e);
	ILI9341_WR_Parameter(0x2e);
	ILI9341_WR_Parameter(0x24);
	ILI9341_WR_Parameter(0x21);
	ILI9341_WR_Parameter(0x1d);
	ILI9341_WR_Parameter(0x1e);
	ILI9341_WR_Parameter(0x0d);
	ILI9341_WR_Parameter(0x08);
	ILI9341_WR_Parameter(0x3c);
	ILI9341_WR_Parameter(0x1c);
	ILI9341_WR_Parameter(0x3f);
	ILI9341_WR_Parameter(0x3f);
	ILI9341_WR_Parameter(0x3c);
	ILI9341_WR_Parameter(0x30);
	ILI9341_WR_Parameter(0x28);
	ILI9341_WR_Parameter(0x1e);
	ILI9341_WR_Parameter(0x16);
	ILI9341_WR_Parameter(0x08);
	ILI9341_WR_Parameter(0x09);
	ILI9341_WR_Parameter(0x0d);
	ILI9341_WR_Parameter(0x1f);
	ILI9341_WR_Parameter(0x2a);
	ILI9341_WR_Parameter(0x30);
	ILI9341_WR_Parameter(0x3f);
	ILI9341_WR_Parameter(0x00);
	ILI9341_WR_Parameter(0xc1);


	//ILI9341_WR_COMD(0xba); 
	//ILI9341_WR_Parameter(0x00);
	//ILI9341_WR_Parameter(0x00);
/*
	ILI9341_WR_COMD(0x3A); // Memory Access Control
	ILI9341_WR_Parameter(0x55); //66

	ILI9341_WR_COMD(0x36); // Memory Access Control
	ILI9341_WR_Parameter(0x48); // 0x48 0x88

	//ILI9341_WR_COMD(0x35); //Set TE ON  //ADD
	//ILI9341_WR_Parameter(0x00);
*/
/*
	ILI9341_WR_COMD(0xef); 
	ILI9341_WR_Parameter (0x03);
	ILI9341_WR_Parameter (0x80);
	ILI9341_WR_Parameter (0x02);
	ILI9341_WR_COMD(0xcf); 
	ILI9341_WR_Parameter (0x00);
	ILI9341_WR_Parameter (0xaa);
	ILI9341_WR_Parameter (0xb0);
	ILI9341_WR_COMD(0xed); 
	ILI9341_WR_Parameter (0x67);
	ILI9341_WR_Parameter (0x03);
	ILI9341_WR_Parameter (0x12);
	ILI9341_WR_Parameter (0x81);
	ILI9341_WR_COMD(0xcb); 
	ILI9341_WR_Parameter (0x39);
	ILI9341_WR_Parameter (0x2c);
	ILI9341_WR_Parameter (0x00);
	ILI9341_WR_Parameter (0x34);
	ILI9341_WR_Parameter (0x02);
	ILI9341_WR_COMD(0xea); 
	ILI9341_WR_Parameter (0x00);
	ILI9341_WR_Parameter (0x00);
	ILI9341_WR_COMD(0xe8); 
	ILI9341_WR_Parameter (0x85);
	ILI9341_WR_Parameter (0x0a);
	ILI9341_WR_Parameter (0x78);
*/
	/*
	ILI9341_WR_COMD(0xC0); //Power control
	ILI9341_WR_Parameter (0x26); //VRH[5:0]
	ILI9341_WR_COMD(0xC1); //Power control
	ILI9341_WR_Parameter (0x01); //SAP[2:0];BT[3:0]
	ILI9341_WR_COMD(0xC5); //VCM control
	ILI9341_WR_Parameter (0x2b);
	ILI9341_WR_Parameter (0x2C);
	ILI9341_WR_COMD(0xc7); 
	ILI9341_WR_Parameter (0xc3);
	*/

	ILI9341_WR_COMD(0x3A); 
	ILI9341_WR_Parameter (0x55);
	
	ILI9341_WR_COMD(0x36); // Memory Access Control
//	ILI9341_WR_Parameter (0x08);    // 屏幕旋转 my,mx,mv,ml,BGR,mh,0.0 
	ILI9341_WR_Parameter (0x28);    // 屏幕旋转 my,mx,mv,ml,BGR,mh,0.0 
	                                //if MADCTL BIT5=1, EC[15:0]=0x013f,28/68/A8/E8 
                                    //if MADCTL BIT5=0, EC[15:0]=0x00ef,08/48/88/c8	

	
	/*ILI9341_WR_COMD(0xB1); // Frame Rate Control
	ILI9341_WR_Parameter (0x00);
	ILI9341_WR_Parameter (0x18);
	ILI9341_WR_COMD(0xB6); // Display Function Control
	ILI9341_WR_Parameter (0x0a);
	ILI9341_WR_Parameter (0xa2);*/
	/*
	ILI9341_WR_COMD(0xb6);
	ILI9341_WR_Parameter(0x26);
	ILI9341_WR_Parameter(0x03);
	ILI9341_WR_Parameter(0x10);
	ILI9341_WR_COMD(0xb1);
	ILI9341_WR_Parameter(0x01);
	ILI9341_WR_Parameter(0x90);
	ILI9341_WR_COMD(0xF2); // 3Gamma Function Disable
	ILI9341_WR_Parameter (0x00);
	ILI9341_WR_COMD(0x26); //Gamma curve selected
	ILI9341_WR_Parameter (0x01);
	
	ILI9341_WR_COMD(0xE0); //Set Gamma
	ILI9341_WR_Parameter (0x0f);
	ILI9341_WR_Parameter (0x1d);
	ILI9341_WR_Parameter (0x1a);
	ILI9341_WR_Parameter (0x09);
	ILI9341_WR_Parameter (0x0f);
	ILI9341_WR_Parameter (0x09);
	ILI9341_WR_Parameter (0x46);
	ILI9341_WR_Parameter (0x88);
	ILI9341_WR_Parameter (0x39);
	ILI9341_WR_Parameter (0x05);
	ILI9341_WR_Parameter (0x0f);
	ILI9341_WR_Parameter (0x03);
	ILI9341_WR_Parameter (0x07);
	ILI9341_WR_Parameter (0x05);
	ILI9341_WR_Parameter (0x00);

	ILI9341_WR_COMD(0xE1); //Set Gamma
	ILI9341_WR_Parameter (0x00);
	ILI9341_WR_Parameter (0x22);
	ILI9341_WR_Parameter (0x25);
	ILI9341_WR_Parameter (0x06);
	ILI9341_WR_Parameter (0x10);
	ILI9341_WR_Parameter (0x06);
	ILI9341_WR_Parameter (0x39);
	ILI9341_WR_Parameter (0x22);
	ILI9341_WR_Parameter (0x4a);
	ILI9341_WR_Parameter (0x0a);
	ILI9341_WR_Parameter (0x10);
	ILI9341_WR_Parameter (0x0c);
	ILI9341_WR_Parameter (0x38);
	ILI9341_WR_Parameter (0x3a);
	ILI9341_WR_Parameter (0x0F);
	*/
	ILI9341_WR_COMD(0x2a);          // column set 
	ILI9341_WR_Parameter(0x00);          //if MADCTL BIT5=0, EC[15:0]=0x00ef
	ILI9341_WR_Parameter(0x00);          //if MADCTL BIT5=1, EC[15:0]=0x013f
	ILI9341_WR_Parameter(0x01); 	//01
	ILI9341_WR_Parameter(0x8f); 	//8f	//400
	
	ILI9341_WR_COMD(0x2b);        	// page address set 
	ILI9341_WR_Parameter(0x00);          //if MADCTL BIT5=0, EP[15:0]=0x013f
	ILI9341_WR_Parameter(0x00);          //if MADCTL BIT5=1, EP[15:0]=0x00ef
	ILI9341_WR_Parameter(0x00); 	//00
	ILI9341_WR_Parameter(0xef); 	//ef	//240

	ILI9341_WR_COMD(0xba);  //---
	ILI9341_WR_Parameter (0x85);
	ILI9341_WR_Parameter (0x66);
	ILI9341_WR_COMD(0xb3);  //---
	ILI9341_WR_Parameter (0xb4);
	ILI9341_WR_Parameter (0x80); //0x80 EQ
	ILI9341_WR_Parameter (0x00);
	ILI9341_WR_Parameter (0x00);	

	ILI9341_WR_COMD(0xb7);
		ILI9341_WR_Parameter(0x64); //70 duibidu: 0x68 5.496v	
		ILI9341_WR_Parameter(0x44);
		ILI9341_WR_Parameter(0x03);

	ILI9341_WR_COMD(0xb8);
		ILI9341_WR_Parameter(0x68); //70 duibidu: 0x68 5.496v	
		ILI9341_WR_Parameter(0x54);
		ILI9341_WR_Parameter(0x11);
		ILI9341_WR_Parameter(0x00);

	ILI9341_WR_COMD(0x35);  //---
	ILI9341_WR_Parameter (0x00);	
/*
    //HCK
    ILI9341_WR_COMD(0xb2);  //---
    ILI9341_WR_Parameter (0xff);
*/

    
    ILI9341_WR_COMD(0xb0);  //---
    ILI9341_WR_Parameter (0x00);
    ILI9341_WR_Parameter (0x00);
    ILI9341_WR_Parameter (0x7f); //越大切线越少
    ILI9341_WR_Parameter (0x7f); //越大切线越少

	ILI9341_WR_COMD(0x36); // Memory Access Control
	ILI9341_WR_Parameter (0x08);    // 屏幕旋转 my,mx,mv,ml,BGR,mh,0.0 
	                                //if MADCTL BIT5=1, EC[15:0]=0x013f,28/68/A8/E8 
                                    //if MADCTL BIT5=0, EC[15:0]=0x00ef,08/48/88/c8	
	ILI9341_WR_COMD(0x2a);          // column set 
	ILI9341_WR_Parameter(0x00);          //if MADCTL BIT5=0, EC[15:0]=0x00ef
	ILI9341_WR_Parameter(0x00);          //if MADCTL BIT5=1, EC[15:0]=0x013f
	ILI9341_WR_Parameter(0x00); 	//00
	ILI9341_WR_Parameter(0xef); 	//ef	//240
	
	ILI9341_WR_COMD(0x2b);        	// page address set 
	ILI9341_WR_Parameter(0x00);          //if MADCTL BIT5=0, EP[15:0]=0x013f
	ILI9341_WR_Parameter(0x00);          //if MADCTL BIT5=1, EP[15:0]=0x00ef
	ILI9341_WR_Parameter(0x01); 	//01
	ILI9341_WR_Parameter(0x8f); 	//8f	//400



	ILI9341_WR_COMD(0x11); //Exit Sleep
	drv_msec_wait(120);
	ILI9341_WR_COMD(0x29); //Display on
	ILI9341_WR_COMD(0x2c); //memory write


//==================================================================

    //TFT_ILI9341_Clear(0xffff);
    //TFT_ILI9341_Clear(0x07E0);
    //TFT_ILI9341_Clear(0xf800);
    DBG_PRINT("TFT_D51E5TA8566_Clear\r\n");

	//tft_backlight_en_set(TRUE);
  
 	R_TFT_VS_WIDTH = 20;	//0xD0500148
	R_TFT_V_PERIOD = 399;	//0xD0500144
	R_TFT_V_START = 5;		//0xD050014C

	R_TFT_HS_WIDTH = 4;		//0xD0500158
	R_TFT_H_PERIOD = 239;	//0xD0500154
	R_TFT_H_START = 4;		//0xD050015C
/*
	R_TFT_VS_START = 00;		//0xD05001B0
	R_TFT_VS_END = 240;		//0xD05001B4
	R_TFT_HS_START = 0;		//0xD05001B8
	R_TFT_HS_END = 399;		//0xD05001BC
*/
	R_TFT_TS_MISC = 0x0000;
	R_TFT_CTRL = 0x20D2; 
	AP_TFT_ClK_144M_set();
	ap_display_TE_sync_I80_start();	
	TEST_Display_Data_180_degree_set(0);
}

#endif