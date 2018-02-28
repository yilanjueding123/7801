/************************************************************
* jpg_list.c
*
* Purpose: A list of JPG files for streaming test
*
* Author: Craig Yang
*
* Date: 2015/10/06
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version :
* History :
*
************************************************************/
#include "project.h"

/**********************************************
*	Definitions
**********************************************/
#define C_MJPG_IMAGE_NUM	60	// Number of JPG files to stream to client

/**********************************************
*	Extern variables and functions
**********************************************/
// JPEG files are stored as resources below
extern INT8U *RES_MJPG_DOG_100_START;
extern INT8U *RES_MJPG_DOG_101_START;
extern INT8U *RES_MJPG_DOG_102_START;
extern INT8U *RES_MJPG_DOG_103_START;
extern INT8U *RES_MJPG_DOG_104_START;
extern INT8U *RES_MJPG_DOG_105_START;
extern INT8U *RES_MJPG_DOG_106_START;
extern INT8U *RES_MJPG_DOG_107_START;
extern INT8U *RES_MJPG_DOG_108_START;
extern INT8U *RES_MJPG_DOG_109_START;
extern INT8U *RES_MJPG_DOG_110_START;
extern INT8U *RES_MJPG_DOG_111_START;
extern INT8U *RES_MJPG_DOG_112_START;
extern INT8U *RES_MJPG_DOG_113_START;
extern INT8U *RES_MJPG_DOG_114_START;
extern INT8U *RES_MJPG_DOG_115_START;
extern INT8U *RES_MJPG_DOG_116_START;
extern INT8U *RES_MJPG_DOG_117_START;
extern INT8U *RES_MJPG_DOG_118_START;
extern INT8U *RES_MJPG_DOG_119_START;
extern INT8U *RES_MJPG_DOG_120_START;
extern INT8U *RES_MJPG_DOG_121_START;
extern INT8U *RES_MJPG_DOG_122_START;
extern INT8U *RES_MJPG_DOG_123_START;
extern INT8U *RES_MJPG_DOG_124_START;
extern INT8U *RES_MJPG_DOG_125_START;
extern INT8U *RES_MJPG_DOG_126_START;
extern INT8U *RES_MJPG_DOG_127_START;
extern INT8U *RES_MJPG_DOG_128_START;
extern INT8U *RES_MJPG_DOG_129_START;
extern INT8U *RES_MJPG_DOG_130_START;
extern INT8U *RES_MJPG_DOG_131_START;
extern INT8U *RES_MJPG_DOG_132_START;
extern INT8U *RES_MJPG_DOG_133_START;
extern INT8U *RES_MJPG_DOG_134_START;
extern INT8U *RES_MJPG_DOG_135_START;
extern INT8U *RES_MJPG_DOG_136_START;
extern INT8U *RES_MJPG_DOG_137_START;
extern INT8U *RES_MJPG_DOG_138_START;
extern INT8U *RES_MJPG_DOG_139_START;
extern INT8U *RES_MJPG_DOG_140_START;
extern INT8U *RES_MJPG_DOG_141_START;
extern INT8U *RES_MJPG_DOG_142_START;
extern INT8U *RES_MJPG_DOG_143_START;
extern INT8U *RES_MJPG_DOG_144_START;
extern INT8U *RES_MJPG_DOG_145_START;
extern INT8U *RES_MJPG_DOG_146_START;
extern INT8U *RES_MJPG_DOG_147_START;
extern INT8U *RES_MJPG_DOG_148_START;
extern INT8U *RES_MJPG_DOG_149_START;
extern INT8U *RES_MJPG_DOG_150_START;
extern INT8U *RES_MJPG_DOG_151_START;
extern INT8U *RES_MJPG_DOG_152_START;
extern INT8U *RES_MJPG_DOG_153_START;
extern INT8U *RES_MJPG_DOG_154_START;
extern INT8U *RES_MJPG_DOG_155_START;
extern INT8U *RES_MJPG_DOG_156_START;
extern INT8U *RES_MJPG_DOG_157_START;
extern INT8U *RES_MJPG_DOG_158_START;
extern INT8U *RES_MJPG_DOG_159_START;
extern INT8U *RES_MJPG_END;

/*********************************************
*	Variables declaration
*********************************************/
static INT16U curr_jpg = 0;
static INT32U mjpeg_image_addr[C_MJPG_IMAGE_NUM+1];

void jpeg_list_init(void)
{
	mjpeg_image_addr[0] = (INT32S)&RES_MJPG_DOG_100_START;
	mjpeg_image_addr[1] = (INT32S)&RES_MJPG_DOG_101_START;
	mjpeg_image_addr[2] = (INT32S)&RES_MJPG_DOG_102_START;
	mjpeg_image_addr[3] = (INT32S)&RES_MJPG_DOG_103_START;
	mjpeg_image_addr[4] = (INT32S)&RES_MJPG_DOG_104_START;
	mjpeg_image_addr[5] = (INT32S)&RES_MJPG_DOG_105_START;
	mjpeg_image_addr[6] = (INT32S)&RES_MJPG_DOG_106_START;
	mjpeg_image_addr[7] = (INT32S)&RES_MJPG_DOG_107_START;
	mjpeg_image_addr[8] = (INT32S)&RES_MJPG_DOG_108_START;
	mjpeg_image_addr[9] = (INT32S)&RES_MJPG_DOG_109_START;
	mjpeg_image_addr[10] = (INT32S)&RES_MJPG_DOG_110_START;
	mjpeg_image_addr[11] = (INT32S)&RES_MJPG_DOG_111_START;
	mjpeg_image_addr[12] = (INT32S)&RES_MJPG_DOG_112_START;
	mjpeg_image_addr[13] = (INT32S)&RES_MJPG_DOG_113_START;
	mjpeg_image_addr[14] = (INT32S)&RES_MJPG_DOG_114_START;
	mjpeg_image_addr[15] = (INT32S)&RES_MJPG_DOG_115_START;
	mjpeg_image_addr[16] = (INT32S)&RES_MJPG_DOG_116_START;
	mjpeg_image_addr[17] = (INT32S)&RES_MJPG_DOG_117_START;
	mjpeg_image_addr[18] = (INT32S)&RES_MJPG_DOG_118_START;
	mjpeg_image_addr[19] = (INT32S)&RES_MJPG_DOG_119_START;
	mjpeg_image_addr[20] = (INT32S)&RES_MJPG_DOG_120_START;
	mjpeg_image_addr[21] = (INT32S)&RES_MJPG_DOG_121_START;
	mjpeg_image_addr[22] = (INT32S)&RES_MJPG_DOG_122_START;
	mjpeg_image_addr[23] = (INT32S)&RES_MJPG_DOG_123_START;
	mjpeg_image_addr[24] = (INT32S)&RES_MJPG_DOG_124_START;
	mjpeg_image_addr[25] = (INT32S)&RES_MJPG_DOG_125_START;
	mjpeg_image_addr[26] = (INT32S)&RES_MJPG_DOG_126_START;
	mjpeg_image_addr[27] = (INT32S)&RES_MJPG_DOG_127_START;
	mjpeg_image_addr[28] = (INT32S)&RES_MJPG_DOG_128_START;
	mjpeg_image_addr[29] = (INT32S)&RES_MJPG_DOG_129_START;
	mjpeg_image_addr[30] = (INT32S)&RES_MJPG_DOG_130_START;
	mjpeg_image_addr[31] = (INT32S)&RES_MJPG_DOG_131_START;
	mjpeg_image_addr[32] = (INT32S)&RES_MJPG_DOG_132_START;
	mjpeg_image_addr[33] = (INT32S)&RES_MJPG_DOG_133_START;
	mjpeg_image_addr[34] = (INT32S)&RES_MJPG_DOG_134_START;
	mjpeg_image_addr[35] = (INT32S)&RES_MJPG_DOG_135_START;
	mjpeg_image_addr[36] = (INT32S)&RES_MJPG_DOG_136_START;
	mjpeg_image_addr[37] = (INT32S)&RES_MJPG_DOG_137_START;
	mjpeg_image_addr[38] = (INT32S)&RES_MJPG_DOG_138_START;
	mjpeg_image_addr[39] = (INT32S)&RES_MJPG_DOG_139_START;
	mjpeg_image_addr[40] = (INT32S)&RES_MJPG_DOG_140_START;
	mjpeg_image_addr[41] = (INT32S)&RES_MJPG_DOG_141_START;
	mjpeg_image_addr[42] = (INT32S)&RES_MJPG_DOG_142_START;
	mjpeg_image_addr[43] = (INT32S)&RES_MJPG_DOG_143_START;
	mjpeg_image_addr[44] = (INT32S)&RES_MJPG_DOG_144_START;
	mjpeg_image_addr[45] = (INT32S)&RES_MJPG_DOG_145_START;
	mjpeg_image_addr[46] = (INT32S)&RES_MJPG_DOG_146_START;
	mjpeg_image_addr[47] = (INT32S)&RES_MJPG_DOG_147_START;
	mjpeg_image_addr[48] = (INT32S)&RES_MJPG_DOG_148_START;
	mjpeg_image_addr[49] = (INT32S)&RES_MJPG_DOG_149_START;
	mjpeg_image_addr[50] = (INT32S)&RES_MJPG_DOG_150_START;
	mjpeg_image_addr[51] = (INT32S)&RES_MJPG_DOG_151_START;
	mjpeg_image_addr[52] = (INT32S)&RES_MJPG_DOG_152_START;
	mjpeg_image_addr[53] = (INT32S)&RES_MJPG_DOG_153_START;
	mjpeg_image_addr[54] = (INT32S)&RES_MJPG_DOG_154_START;
	mjpeg_image_addr[55] = (INT32S)&RES_MJPG_DOG_155_START;
	mjpeg_image_addr[56] = (INT32S)&RES_MJPG_DOG_156_START;
	mjpeg_image_addr[57] = (INT32S)&RES_MJPG_DOG_157_START;
	mjpeg_image_addr[58] = (INT32S)&RES_MJPG_DOG_158_START;
	mjpeg_image_addr[59] = (INT32S)&RES_MJPG_DOG_159_START;
	mjpeg_image_addr[60] = (INT32S)&RES_MJPG_END;
}

// Get next jpg in mjpeg_image_addr[]
void jpeg_list_get_next(INT8U **addr, INT32U *size)
{
	*addr = (INT8U*)mjpeg_image_addr[curr_jpg];
	*size = mjpeg_image_addr[curr_jpg+1] - mjpeg_image_addr[curr_jpg];
	
	curr_jpg = (curr_jpg >= (C_MJPG_IMAGE_NUM-1)) ? 0 : (curr_jpg+1);
}

