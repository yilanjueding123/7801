#include"T20P82_ST7789V.h"

#if USE_PANEL_NAME == PANEL_T20P82_ST7789V

void ST7789V_WR_DELAY(INT32U i) //DELAY
{
	unsigned int j, cnt;
	cnt = i*2;
	for (j=0;j<cnt;j++);
}

void ST7789V_WR_COMD(unsigned int comm)//write command
{    
     gpio_write_io (LCD_CS, DATA_LOW); 
	 gpio_write_io (LCD_RS, DATA_LOW); 
     gpio_write_io (LCD_RD, DATA_HIGH); 
     gpio_write_io (LCD_WR, DATA_LOW);
     R_IOA_O_DATA=comm;
     gpio_write_io (LCD_WR, DATA_HIGH);
}

void ST7789V_WR_Parameter(unsigned int data)//write Parameter
{   
     gpio_write_io (LCD_RD, DATA_HIGH);
     gpio_write_io (LCD_RS, DATA_HIGH); 
     gpio_write_io (LCD_WR, DATA_LOW);
     R_IOA_O_DATA=data; 
     gpio_write_io (LCD_WR, DATA_HIGH);
}

void ST7789V_WR_DATD(unsigned int data)//write data
{    
     gpio_write_io (LCD_CS, DATA_LOW);
     gpio_write_io (LCD_RD, DATA_HIGH);
     gpio_write_io (LCD_RS, DATA_HIGH);
      
     gpio_write_io (LCD_WR, DATA_LOW);
     R_IOA_O_DATA=data; 
     gpio_write_io (LCD_WR, DATA_HIGH);
}

void TFT_T20P82_ST7789V_Clear(unsigned int Color)
{
	INT16U i, j;
	for(i = 0; i < 320; i++)
		for(j = 0; j < 240; j++)
		   {ST7789V_WR_DATD(Color>>8);
		   ST7789V_WR_DATD(Color);}
}

//============================================================================
  /**************INIT***********/
//=============================================================================
extern void ap_display_TE_sync_I80_start(void);
void TFT_T20P82_ST7789V_Init(void)
{   

	R_I2C_MISC = 0;
	
	R_IOA_ATTRIB = 0xffff;
    R_IOA_DIR = 0xffff;
    R_IOA_O_DATA = 0; 

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

//============================================================	
	gpio_set_port_attribute(LCD_RST, ATTRIBUTE_HIGH);
	gpio_init_io(LCD_RST, DATA_LOW);//gpio_init_io(LCD_RST, GPIO_OUTPUT);
//==============================================================
#if 1		
	gpio_write_io (LCD_RST, DATA_HIGH);
	ST7789V_WR_DELAY(1);
	    	    
	gpio_write_io (LCD_RST, DATA_LOW);
    ST7789V_WR_DELAY(10);
        
	gpio_write_io (LCD_RST, DATA_HIGH);
	ST7789V_WR_DELAY(120); 
#endif		     
//=======init start ===============================


//-------科通达-力科2.0 40P CPU屏-ST7789V--------//
//ST7789V_WR_COMD(0x11);
//ST7789V_WR_DELAY(120); //Delay 120ms
//----------display and color format setting------//
ST7789V_WR_COMD(0x36);
ST7789V_WR_Parameter(0x00);
ST7789V_WR_COMD(0x3a);
ST7789V_WR_Parameter(0x55);
//------ST7789V Frame rate setting--------------//
ST7789V_WR_COMD(0xb2);
ST7789V_WR_Parameter(0x7f);
ST7789V_WR_Parameter(0x7f);
ST7789V_WR_Parameter(0x00);
ST7789V_WR_Parameter(0xff);
ST7789V_WR_Parameter(0xff);
ST7789V_WR_COMD(0xb7);
ST7789V_WR_Parameter(0x35);
//--------ST7789V Power setting--------------//
ST7789V_WR_COMD(0xbb);
ST7789V_WR_Parameter(0x20);//0x20
ST7789V_WR_COMD(0xc0);
ST7789V_WR_Parameter(0x2c);
ST7789V_WR_COMD(0xc2);
ST7789V_WR_Parameter(0x01);
ST7789V_WR_COMD(0xc3);
ST7789V_WR_Parameter(0x00);//0x0b 0x08
ST7789V_WR_COMD(0xc4);
ST7789V_WR_Parameter(0x12);//0x20 0x16
ST7789V_WR_COMD(0xc6);
ST7789V_WR_Parameter(0x11);//0f
ST7789V_WR_COMD(0xd0);
ST7789V_WR_Parameter(0xa4);
ST7789V_WR_Parameter(0xa1);

ST7789V_WR_COMD(0x36); // Memory Access Control
ST7789V_WR_Parameter (0x60);// 这个RGB颜色和 调试方向
	                                //if MADCTL BIT5=1, EC[15:0]=0x013f,20/60/A0/E0 
                                    //if MADCTL BIT5=0, EC[15:0]=0x00ef,00/40/80/c0	
ST7789V_WR_COMD(0x2B);
ST7789V_WR_Parameter(0x00)  ;
ST7789V_WR_Parameter(0x00);
ST7789V_WR_Parameter(0x00);
ST7789V_WR_Parameter(0xef);

ST7789V_WR_COMD(0x2A);
ST7789V_WR_Parameter(0x00);
ST7789V_WR_Parameter(0x00);
ST7789V_WR_Parameter(0x01);
ST7789V_WR_Parameter(0x3f);
//-------ST7789V gamma setting----------//
ST7789V_WR_COMD(0xe0);
ST7789V_WR_Parameter(0xd0);
ST7789V_WR_Parameter(0x00);
ST7789V_WR_Parameter(0x03);
ST7789V_WR_Parameter(0x08);
ST7789V_WR_Parameter(0x0a);
ST7789V_WR_Parameter(0x17);
ST7789V_WR_Parameter(0x2e);
ST7789V_WR_Parameter(0x44);
ST7789V_WR_Parameter(0x3f);
ST7789V_WR_Parameter(0x29);
ST7789V_WR_Parameter(0x10);
ST7789V_WR_Parameter(0x0e);
ST7789V_WR_Parameter(0x14);
ST7789V_WR_Parameter(0x18);
ST7789V_WR_COMD(0xe1);
ST7789V_WR_Parameter(0xd0);
ST7789V_WR_Parameter(0x00);
ST7789V_WR_Parameter(0x03);
ST7789V_WR_Parameter(0x08);
ST7789V_WR_Parameter(0x07);
ST7789V_WR_Parameter(0x27);
ST7789V_WR_Parameter(0x2b);
ST7789V_WR_Parameter(0x44);
ST7789V_WR_Parameter(0x41);
ST7789V_WR_Parameter(0x3c);
ST7789V_WR_Parameter(0x1b);
ST7789V_WR_Parameter(0x1d);
ST7789V_WR_Parameter(0x14);
ST7789V_WR_Parameter(0x18);
ST7789V_WR_COMD(0x29);


ST7789V_WR_COMD(0x30);
ST7789V_WR_Parameter(0x00);
//Write PSL; //PSL: Start Line
ST7789V_WR_Parameter(0x00);
//Write PEL; //PEL: End Line
//ST7789V_WR_COMD(0x12);


ST7789V_WR_COMD(0x35); 
ST7789V_WR_Parameter(0x00); 

//ST7789V_WR_COMD(0x44); 		 
//ST7789V_WR_Parameter(0x00);	
//ST7789V_WR_Parameter(0xc0);	 

ST7789V_WR_COMD(0x11); // exit sleep 
ST7789V_WR_DELAY(120); 
 
ST7789V_WR_COMD(0x29); // display on 
ST7789V_WR_DELAY(10);

//V3029_WR_COMD(0x39);

ST7789V_WR_COMD(0x2c);  //memory write 
ST7789V_WR_DELAY(40);


#if 0
    DBG_PRINT("C1\r\n");
	//-----test action start-----
	TFT_T20P82_ST7789V_Clear(0xF800);
    drv_msec_wait(400);
    TFT_T20P82_ST7789V_Clear(0xF81F);
    drv_msec_wait(400);
    TFT_T20P82_ST7789V_Clear(0x0000);
    drv_msec_wait(400);
    TFT_T20P82_ST7789V_Clear(0xFFFF);
    drv_msec_wait(400);
	//while(1);
    DBG_PRINT("C2\r\n");
	//-----test action end-----	
#endif		
//========init end=======================================

	R_TFT_VS_WIDTH = 20;    
  	R_TFT_V_PERIOD = 320-1;
	R_TFT_V_START = 5;
	
	R_TFT_HS_WIDTH = 5;
	R_TFT_H_PERIOD = 240-1;
	R_TFT_H_START = 5;

	R_TFT_VS_START = 0;
	R_TFT_VS_END = 320;
	R_TFT_HS_START = 0;
	R_TFT_HS_END = 240-1;	
	
  	R_TFT_TS_MISC= 0x00;		 
	R_TFT_CTRL = 0x20d3;//c2	 d2

	AP_TFT_ClK_144M_set();	
	ap_display_TE_sync_I80_start();	
}
#endif



