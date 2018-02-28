#ifndef __DVR_TFT_ILI9341_H__
#define __DVR_TFT_ILI9341_H__



#include "application.h"
#include "drv_l1_sfr.h"
#if (USE_PANEL_NAME == PANEL_400X240_I80)
///////////////////////////////////////////////////////
// YELLOW 0xFD28 
// GREEN  0x21B7
// BLACK  0x0845
//
////////////////////////////////////////////////////////
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

#define LCD_RST  IO_D8 //IO_B5
//#define LCD_BL  IO_G14

void ILI9341_WR_COMD(unsigned int index);//write command
void ILI9341_WR_Parameter(unsigned int data);//write Parameter
void ILI9341_WR_DATD(unsigned int data);//write data
void TFT_D51E5TA8566_Init(void);
void TFT_ILI9341_Clear(unsigned int Color);


#endif //#if (USE_PANEL_NAME == PANEL_400X240_I80)

#endif 		/* __DVR_TFT_ILI9341_H__ */