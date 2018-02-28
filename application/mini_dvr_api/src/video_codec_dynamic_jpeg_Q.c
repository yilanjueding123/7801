#include "my_avi_encoder_state.h"

#define Q_Y_STEP 2
#define Q_UV_STEP 2
#define Q_DISTANCE 0

// 1080P/1080FHD
#define Q_LARGE_Y_MAX 		70
#define Q_LARGE_Y_MIN		15//10
#define Q_LARGE_Y_EMER		20//15

#define Q_LARGE_UV_MAX 	(Q_LARGE_Y_MAX-Q_DISTANCE)
#define Q_LARGE_UV_MIN 	    15//10
#define Q_LARGE_UV_EMER 	20//15
// 720P/WVGA/VGA
#define Q_SMALL_Y_MAX		80
#define Q_SMALL_Y_MIN		15
#define Q_SMALL_Y_EMER		20

#define Q_SMALL_UV_MAX		(Q_SMALL_Y_MAX-Q_DISTANCE)
#define Q_SMALL_UV_MIN		15
#define Q_SMALL_UV_EMER		20


extern INT32S current_Y_Q_value;
extern INT32S current_UV_Q_value;

extern INT32U max_VLC_size;

extern INT32U JPEG_DISP_OUT_BUFFER_SIZE;
extern INT32S disp_current_Y_Q_value;

INT32U JPEG_DISP_OUT_BUFFER_SIZE_640_360 = (40*1024);
INT32U JPEG_DISP_OUT_BUFFER_SIZE_480_272 = (40*1024);	//(30*1024);

#define JPEG_DISP_RESET_Q_SIZE  JPEG_DISP_OUT_BUFFER_SIZE - (10*1024)    
#define JPEG_DISP_MAX_SIZE		JPEG_DISP_OUT_BUFFER_SIZE-(15*1024)			
#define JPEG_DISP_MIN_SIZE		JPEG_DISP_OUT_BUFFER_SIZE-(25*1024)	

static INT16S RiseQ_Cnt = 0;
extern INT16S get_lossframe_cnt(void);
/****************************************************************************/
/*
 *	Dynamic_Tune_Q
 */
void Dynamic_Tune_Q(INT32U vlcSize)
{

   if(vlcSize > (max_VLC_size - 10*1024)) { // 如果 JPEG > (max_VLC_size - 5KB) 或 爆掉，馬上 Q 值調低 
		if((my_pAviEncVidPara->encode_width == AVI_WIDTH_1080FHD) || (my_pAviEncVidPara->encode_width == AVI_WIDTH_1080P))
		{  // 1080P 
			if((current_Y_Q_value > Q_LARGE_Y_EMER) || (current_UV_Q_value > Q_LARGE_UV_EMER)) {
				current_Y_Q_value = Q_LARGE_Y_EMER;
				current_UV_Q_value = Q_LARGE_UV_EMER;
			} else {
				current_Y_Q_value = Q_LARGE_Y_MIN;
				current_UV_Q_value = Q_LARGE_UV_MIN;
			}
		} else { // 720P 
			if((current_Y_Q_value > Q_SMALL_Y_EMER) || (current_UV_Q_value > Q_SMALL_UV_EMER)) {
				current_Y_Q_value = Q_SMALL_Y_EMER;
				current_UV_Q_value = Q_SMALL_UV_EMER;
			} else {
				current_Y_Q_value = Q_SMALL_Y_MIN;
				current_UV_Q_value = Q_SMALL_UV_MIN;
			}
		}
	} else {
 		if((my_pAviEncVidPara->encode_width == AVI_WIDTH_1080FHD) ||(my_pAviEncVidPara->encode_width == AVI_WIDTH_1080P)) {
			if(vlcSize > (/*115*/200*1024)) {
				if((current_Y_Q_value - current_UV_Q_value) >= Q_DISTANCE) {
					current_Y_Q_value -= Q_Y_STEP;
					if(current_Y_Q_value < Q_LARGE_Y_MIN) {
						current_Y_Q_value = Q_LARGE_Y_MIN;
					}
				} else {
					if(current_UV_Q_value > Q_LARGE_UV_MIN) {
						current_UV_Q_value -= Q_UV_STEP;
						if(current_UV_Q_value < Q_LARGE_UV_MIN) {
							current_UV_Q_value = Q_LARGE_UV_MIN;
						}
					} else {
						current_Y_Q_value -= Q_Y_STEP;
						if(current_Y_Q_value < Q_LARGE_Y_MIN) {
							current_Y_Q_value = Q_LARGE_Y_MIN;
						}
					}
				}
			} else if (vlcSize < (/*70*/150*1024)) {
				if ((current_Y_Q_value - current_UV_Q_value) >= Q_DISTANCE) {
					current_UV_Q_value += Q_UV_STEP;
					if(current_UV_Q_value > Q_LARGE_Y_MAX) {
						current_UV_Q_value = Q_LARGE_Y_MAX;
					}
				} else {
					if(current_Y_Q_value < Q_LARGE_Y_MAX) {
						current_Y_Q_value += Q_Y_STEP;
						if(current_Y_Q_value > Q_LARGE_Y_MAX) {
							current_Y_Q_value = Q_LARGE_Y_MAX;
						}
					} else {
						current_UV_Q_value += Q_UV_STEP;
						if(current_UV_Q_value > Q_LARGE_UV_MAX) {
							current_UV_Q_value = Q_LARGE_UV_MAX;
						}
					}
				}
			}
 		} else { //other size (720P, WVGA, VGA)
			if(vlcSize > (120*1024)) {
				if ((current_Y_Q_value - current_UV_Q_value) >= Q_DISTANCE) {
					current_Y_Q_value -= Q_Y_STEP;
					if(current_Y_Q_value < Q_SMALL_Y_MIN) {
						current_Y_Q_value = Q_SMALL_Y_MIN;
					}
				} else {
					if(current_UV_Q_value > Q_SMALL_UV_MIN) {
						current_UV_Q_value -= Q_UV_STEP;
						if(current_UV_Q_value < Q_SMALL_UV_MIN) {
							current_UV_Q_value = Q_SMALL_UV_MIN;
						}
					} else {
						current_Y_Q_value -= Q_Y_STEP;
						if(current_Y_Q_value < Q_SMALL_Y_MIN) {
							current_Y_Q_value = Q_SMALL_Y_MIN;
						}
					}
				}
			} else if (vlcSize < (/*70*/80*1024)) {
				if ((current_Y_Q_value - current_UV_Q_value) >= Q_DISTANCE) {
					current_UV_Q_value += Q_UV_STEP;
					if(current_UV_Q_value > Q_SMALL_UV_MAX) {
						current_UV_Q_value = Q_SMALL_UV_MAX;
					}
				} else {
					if(current_Y_Q_value < Q_SMALL_Y_MAX) {
						current_Y_Q_value += Q_Y_STEP;
						if(current_Y_Q_value > Q_SMALL_Y_MAX) {
							current_Y_Q_value = Q_SMALL_Y_MAX;
						}
					} else {
						current_UV_Q_value += Q_UV_STEP;
						if(current_UV_Q_value > Q_SMALL_UV_MAX) {
							current_UV_Q_value = Q_SMALL_UV_MAX;
						}
					}
				}
			}
		}
	}
	
	//DBG_PRINT("\r\nvlcSize:%d\r\n", vlcSize/1024);
	//DBG_PRINT("Q:%d\r\n", disp_current_Y_Q_value);
}

void Display_Dynamic_Tune_Q(INT32U vlcSize,INT32U disp_fullsize_cnt)
{
	//disp_current_Y_Q_value = 10;
/*
	if (disp_fullsize_cnt)
	{
		disp_current_Y_Q_value -= 30;//20	
	}

*/
/*
	//if ((get_lossframe_cnt() != 0)||(vlcSize > JPEG_DISP_RESET_Q_SIZE)) 
	if ((get_lossframe_cnt() != 0)|| disp_fullsize_cnt) 
	{
		if (disp_fullsize_cnt)
		{
			disp_current_Y_Q_value -= 30;
		}
		else if (get_lossframe_cnt() != 0)
		{
			RiseQ_Cnt = 5;
			disp_current_Y_Q_value = 20;
		}
	}	
	*/
	if (disp_fullsize_cnt)
	{
		disp_current_Y_Q_value -= 30;
	}
	/*
	else if (get_lossframe_cnt() != 0)
	{
		RiseQ_Cnt = 5;
		disp_current_Y_Q_value = 20;
	}
	*/
	else if (vlcSize > JPEG_DISP_MAX_SIZE)  //25
	{
		disp_current_Y_Q_value -= 4;
	}
	else if (vlcSize < JPEG_DISP_MIN_SIZE)  //15
	{
		/*
		if (RiseQ_Cnt != 0) 
		{
			RiseQ_Cnt--;
		}
		else
		{
			disp_current_Y_Q_value += 1;
		}
		*/
		disp_current_Y_Q_value += 1;
	}

	if(disp_current_Y_Q_value < 10)
	{
		disp_current_Y_Q_value = 10;
	}

	if(disp_current_Y_Q_value >= 50)
	{
		disp_current_Y_Q_value = 50;
	}
	//DBG_PRINT("FullSize:%d\r\n", disp_fullsize_cnt);
	//DBG_PRINT("vlcSize:%d\r\n", vlcSize/1024);
	//DBG_PRINT("Disp Q:%d\r\n", disp_current_Y_Q_value);
}