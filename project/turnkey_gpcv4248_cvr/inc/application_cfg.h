/*****************************************************************************
 *               Copyright Generalplus Corp. All Rights Reserved.
 *
 * FileName:       application_cfg.h
 * Author:         Lichuanyue
 * Description:    Created
 * Version:        1.0
 * Function List:
 *                 Null
 * History:
 *                 1>. 2008/7/15 Created
 *****************************************************************************/

#ifndef __APPLICATION_CFG_H__
#define __APPLICATION_CFG_H__

#define _DPF_SUPPORT_ENGLISH    1
#define _DPF_SUPPORT_SCHINESE   1
#define _DPF_SUPPORT_TCHINESE   1
#define _DPF_SUPPORT_FRENCH     0
#define _DPF_SUPPORT_ITALIAN    0
#define _DPF_SUPPORT_SPANISH    0
#define _DPF_SUPPORT_PORTUGUESE 0
#define _DPF_SUPPORT_GERMAN     0
#define _DPF_SUPPORT_RUSSIAN    0
#define _DPF_SUPPORT_SPANISH    0
#define _DPF_SUPPORT_TURKISH    0
#define _DPF_SUPPORT_WWEDISH    0

/*
*  Language code
*/
typedef enum {
	#if _DPF_SUPPORT_ENGLISH == 1
	LCD_EN = 0,  //English
	#endif
	#if _DPF_SUPPORT_TCHINESE == 1
	LCD_TCH,	//Traditional Chinese
 	#endif
	#if _DPF_SUPPORT_SCHINESE == 1
	LCD_SCH,    //Simple Chinese
	#endif
 	#if _DPF_SUPPORT_FRENCH == 1
    LCD_FR,     //French
 	#endif
	#if _DPF_SUPPORT_ITALIAN == 1
    LCD_IT, 	//Italian
 	#endif
	#if _DPF_SUPPORT_PORTUGUESE == 1
    LCD_ES,     // Spanish
 	#endif
 	#if _DPF_SUPPORT_GERMAN == 1
	LCD_PT,     // Portuguese
 	#endif
	#if _DPF_SUPPORT_RUSSIAN == 1
	LCD_DE,     // German
 	#endif
	#if _DPF_SUPPORT_SPANISH == 1
	LCD_RU,     // Russian
 	#endif
	#if _DPF_SUPPORT_TURKISH == 1
	LCD_TR,     // Turkish
 	#endif
	#if _DPF_SUPPORT_WWEDISH == 1
	LCD_SV,     //  Swedish
 	#endif
 	LCD_MAX
}_DPF_LCD;


#define APP_G_MIDI_DECODE_EN	1

//define the special effect when use video encode
#define AUDIO_SFX_HANDLE	0
#define VIDEO_SFX_HANDLE	0

//define audio algorithm
#define APP_WAV_CODEC_EN		1	//mudt enable when use Audio Encode and AVI Decode 	
#define APP_MP3_DECODE_EN		0
#define APP_MP3_ENCODE_EN		0
#define APP_A1800_DECODE_EN		0
#define APP_A1800_ENCODE_EN		0
#define APP_WMA_DECODE_EN		0
#define APP_A1600_DECODE_EN		0
#define APP_A6400_DECODE_EN		0
#define APP_S880_DECODE_EN		0

#define	APP_VOICE_CHANGER_EN	0
#define APP_UP_SAMPLE_EN		0
#define	APP_DOWN_SAMPLE_EN		0
#define A1800_DOWN_SAMPLE_EN	0

#if APP_WAV_CODEC_EN
	#define APP_WAV_CODEC_FG_EN		1	//must enable when use Audio Encode and AVI Decode 	
	#define APP_WAV_CODEC_BG_EN     1
#else
	#define APP_WAV_CODEC_FG_EN		0
	#define APP_WAV_CODEC_BG_EN     0
#endif

#if APP_MP3_DECODE_EN
	#define APP_MP3_DECODE_FG_EN	1
	#define APP_MP3_DECODE_BG_EN	1
#else
	#define APP_MP3_DECODE_FG_EN	0
	#define APP_MP3_DECODE_BG_EN	0
#endif




#if APP_A1800_DECODE_EN
	#define APP_A1800_DECODE_FG_EN	1
	#define APP_A1800_DECODE_BG_EN  1
#else
	#define APP_A1800_DECODE_FG_EN	0
	#define APP_A1800_DECODE_BG_EN  0	
#endif

#if APP_WMA_DECODE_EN
	#define APP_WMA_DECODE_FG_EN	1
	#define APP_WMA_DECODE_BG_EN	1
#else
	#define APP_WMA_DECODE_FG_EN	0
	#define APP_WMA_DECODE_BG_EN	0
#endif

#if APP_A1600_DECODE_EN
	#define APP_A1600_DECODE_FG_EN	1
	#define APP_A1600_DECODE_BG_EN	1
#else
	#define APP_A1600_DECODE_FG_EN	0
	#define APP_A1600_DECODE_BG_EN	0
#endif

#if APP_A6400_DECODE_EN
	#define APP_A6400_DECODE_FG_EN	1
	#define APP_A6400_DECODE_BG_EN	1
#else
	#define APP_A6400_DECODE_FG_EN	0
	#define APP_A6400_DECODE_BG_EN	0
#endif

#if APP_S880_DECODE_EN
	#define APP_S880_DECODE_FG_EN	1
	#define APP_S880_DECODE_BG_EN	1
#else
	#define APP_S880_DECODE_FG_EN	0
	#define APP_S880_DECODE_BG_EN	0
#endif

#endif		// __APPLICATION_CFG_H__
