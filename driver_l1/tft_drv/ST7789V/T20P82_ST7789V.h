#ifndef __T20P82_ST7789V_H__
#define __T20P82_ST7789V_H__

#include "application.h"
#include "drv_l1_sfr.h"

///////////////////////////////////////////////////////
#define	BLACK	0x0000
#define	BLUE	0x001F
#define	RED 	0xF800
#define	GREEN 	0x07E0
#define CYAN	0x07FF
#define MAGENTA 0xF81F
#define YELLOW	0xFFE0
#define WHITE	0xFFFF		

#define LCD_RD      IO_B0
#define LCD_RS      IO_B1
#define LCD_CS      IO_B2
#define LCD_WR      IO_B3

#define LCD_RST     IO_B11//IO_D6

void NV3029_WR_COMD(unsigned int index);//write command
void NV3029_WR_Parameter(unsigned int data);//write Parameter
void NV3029_WR_DATD(unsigned int data);//write data
void TFT_S2006L0_ILI9341_Init(void);
void TFT_S2006L0_ILI9341_Clear(unsigned int Color);
void NV3029_WR_DELAY(INT32U i); //DELAY

#endif 		