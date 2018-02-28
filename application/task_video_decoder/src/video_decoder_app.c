#include "application.h"
#include "task_video_decoder.h"
#include "ap_display.h"
#include "drv_l1_jpeg.h"
#include "drv_l1_wrap.h"
#include "my_video_codec_callback.h"
#include "state_wifi.h"


#if DUAL_STREAM_FUNC_ENABLE
#define BROWSE_DISP_JPEG_CNT		5
#define BROWSE_DISP_JPEG_MAX_SIZE	40*1024


typedef struct BROWSE_DISP_s
{
	INT32U Jpeg_Buf;
	INT8U  Jpeg_Buf_Is_Used;
	mjpeg_write_data_t mjpegWriteData;
}BROWSE_DISP_t;

BROWSE_DISP_t Browse_Disp_Jpeg_Buf[BROWSE_DISP_JPEG_CNT];

extern INT32S Encode_Disp_Buf_To_Jpeg(INT32U dispAddr, INT32U jpegAddrs,INT32U jpegWidth,INT32U jpegHeight, INT32U jpegMaxVlcSize, INT32U* retVlcSize);
extern INT32S mjpeg_send_picture(mjpeg_write_data_t* pMjpegWData);

#endif

/* global varaible */
static vid_dec_buf_struct g_audio;
static DMA_STRUCT g_aud_dec_dma_dbf;


// timer and sync
void vid_dec_timer_isr(void)
{
	if(p_vid_dec_para->audio_flag)
	{
		if((p_vid_dec_para->tv - p_vid_dec_para->ta) < p_vid_dec_para->time_range)
		{
			p_vid_dec_para->tv += p_vid_dec_para->tick_time2;
		}
		
		if(p_vid_dec_para->ta - p_vid_dec_para->tv > p_vid_dec_para->time_range)
		{
			// Make video faster if ta is greater tv more than 32ms
			// This branch will only occur when ICE stop on debug point 
			p_vid_dec_para->tv += p_vid_dec_para->tick_time2;
		}
	}
	else
	{
		p_vid_dec_para->tv += p_vid_dec_para->tick_time2;
	}
	
	if((p_vid_dec_para->tv-p_vid_dec_para->Tv) >= 0)
	{
		if(p_vid_dec_para->post_cnt == p_vid_dec_para->pend_cnt)
		{
			OSQPost(vid_dec_q, (void*)MSG_VID_DEC_ONE_FRAME);
			p_vid_dec_para->post_cnt++;
		}
	}
}

void vid_dec_stop_timer(void)
{
	timer_stop(VIDEO_DECODE_TIMER);
}

void vid_dec_start_timer(void)
{
	INT32U temp, freq_hz;
	
	
	if(p_vid_dec_para->audio_flag)
	{
		//temp = 0x10000 -((0x10000 - (R_TIMERE_PRELOAD & 0xFFFF)) * p_vid_dec_para->n);
		temp = (0x10000 - (R_TIMERE_PRELOAD & 0xFFFF)) * p_vid_dec_para->n;
		freq_hz = MCLK/2/temp;
		if(MCLK%(2*temp))	freq_hz++;
	}
	else
		freq_hz = TIME_BASE_TICK_RATE;
	
	timer_freq_setup(VIDEO_DECODE_TIMER, freq_hz, 0, vid_dec_timer_isr);
}

//memory
static void vid_dec_buffer_number_init(void)
{
	gp_memset((INT8S*)&g_audio, 0x00, sizeof(vid_dec_buf_struct));

	//audio
	g_audio.total_number = AUDIO_FRAME_NO; 	
}

static INT32S vid_dec_audio_memory_alloc(void)
{
	INT32S i, nRet;
	
	if(p_vid_dec_para->audio_flag == 0)
		RETURN(0);
		
	for(i=0; i<g_audio.total_number; i++)
	{
		if(p_vid_dec_para->audio_decode_addr[i] == 0)
		{
			p_vid_dec_para->audio_decode_addr[i] = (INT32U)gp_malloc_align(p_vid_dec_para->aud_frame_size, 4);
			if(p_vid_dec_para->audio_decode_addr[i] == 0) RETURN(-1);
			gp_memset((INT8S*)p_vid_dec_para->audio_decode_addr[i], 0x80, p_vid_dec_para->aud_frame_size);	
		}
	}
	
	nRet = STATUS_OK;
Return:
	for(i=0; i<g_audio.total_number; i++)
		DEBUG_MSG(DBG_PRINT("AudioFrame = 0x%x\r\n", p_vid_dec_para->audio_decode_addr[i]));
		
	return nRet;
}

static void vid_dec_audio_memory_free(void)
{
	INT32S i;
	
	for(i=0; i<g_audio.total_number; i++)
	{
		if(p_vid_dec_para->audio_decode_addr[i])
		{
			gp_free((void*)p_vid_dec_para->audio_decode_addr[i]);
			p_vid_dec_para->audio_decode_addr[i] = 0;
		}
	}
}


INT32S vid_dec_memory_alloc(void)
{
	//init index
	vid_dec_buffer_number_init();
	
	if(vid_dec_audio_memory_alloc() < 0)
		return -1;
	return 0;
}

void vid_dec_memory_free(void)
{
	vid_dec_audio_memory_free();
}


INT32U vid_dec_get_next_aud_buffer(void)
{
	INT32U addr;
	
	//do{
		addr = p_vid_dec_para->audio_decode_addr[g_audio.current_index++];
		if(g_audio.current_index >= g_audio.total_number)		
			g_audio.current_index = 0;	
	//}while(addr == g_lock_vid_addr);
	return addr;
}

//status
void vid_dec_set_status(INT32S flag)
{
	p_vid_dec_para->status |= flag;
}

void vid_dec_clear_status(INT32S flag)
{
	p_vid_dec_para->status &= ~flag;
}

INT32S vid_dec_get_status(void)
{
	return p_vid_dec_para->status;
}

//video info
void vid_dec_set_video_flag(INT8S video_flag)
{
	if(video_flag)
		p_vid_dec_para->video_flag = TRUE;
	else
		p_vid_dec_para->video_flag = FALSE;
}

INT8S vid_dec_get_video_flag(void)
{
	return p_vid_dec_para->video_flag;
}

INT32S vid_dec_get_video_format(INT32U biCompression)
{
	INT8U data[4];
	
	data[0] = (biCompression >> 0) & 0xFF; //X
	data[1] = (biCompression >> 8) & 0xFF; //X
	data[2] = (biCompression >> 16) & 0xFF; //X
	data[3] = (biCompression >> 24) & 0xFF; //X
	
	if( (data[0] == 'X' || data[0] == 'x') &&
		(data[1] == 'V' || data[1] == 'v') &&
		(data[2] == 'I' || data[2] == 'i') &&
		(data[3] == 'D' || data[3] == 'd'))
	{
		DEBUG_MSG(DBG_PRINT("VidFormat = C_XVID_FORMAT\r\n"));
		return C_XVID_FORMAT;
	}
	else if((data[0] == 'M' || data[0] == 'm') &&
			(data[1] == '4') &&
			(data[2] == 'S' || data[2] == 's') &&
			(data[3] == '2'))
	{
		DEBUG_MSG(DBG_PRINT("VidFormat = C_M4S2_FORMAT\r\n"));
		return C_M4S2_FORMAT;
	}
	else if((data[0] == 'H' || data[0] == 'h') &&
			(data[1] == '2') &&
			(data[2] == '6') &&
			(data[3] == '3'))
	{
		DEBUG_MSG(DBG_PRINT("VidFormat = C_H263_FORMAT\r\n"));
		return C_H263_FORMAT;
	}
	else if((data[0] == 'M' || data[0] == 'm') &&
			(data[1] == 'J' || data[1] == 'j') &&
			(data[2] == 'P' || data[2] == 'p') &&
			(data[3] == 'G' || data[3] == 'g'))
	{
		DEBUG_MSG(DBG_PRINT("VidFormat = C_MJPG_FORMAT\r\n"));
		return C_MJPG_FORMAT;
	}
	
	DEBUG_MSG(DBG_PRINT("NotSupportVidFormat = 0x%x\r\n", biCompression)); 
	return STATUS_FAIL;	 
}

INT32S vid_dec_get_file_format(INT8S *pdata)
{
	if( (*(pdata+0) == 'A' || *(pdata+0) == 'a') &&
		(*(pdata+1) == 'V' || *(pdata+1) == 'v') &&
		(*(pdata+2) == 'I' || *(pdata+2) == 'i'))
	{
		return FILE_TYPE_AVI;
	}
	else if((*(pdata+0) == 'M' || *(pdata+0) == 'm') &&
			(*(pdata+1) == 'O' || *(pdata+1) == 'o') &&
			(*(pdata+2) == 'V' || *(pdata+2) == 'v'))
	{
		return FILE_TYPE_MOV;
	}
	else if((*(pdata+0) == 'M' || *(pdata+0) == 'm') &&
			(*(pdata+1) == 'P' || *(pdata+1) == 'p') &&
			(*(pdata+2) == '4' || *(pdata+2) == '4'))
	{
		return FILE_TYPE_MOV;
	}
	else if((*(pdata+0) == '3' || *(pdata+0) == '3') &&
			(*(pdata+1) == 'G' || *(pdata+1) == 'g') &&
			(*(pdata+2) == 'P' || *(pdata+2) == 'p'))
	{
		return FILE_TYPE_MOV;
	}
	else if((*(pdata+0) == 'M' || *(pdata+0) == 'm') &&
			(*(pdata+1) == '4' || *(pdata+1) == '4') &&
			(*(pdata+2) == 'A' || *(pdata+2) == 'a'))
	{
		return FILE_TYPE_MOV;
	}
	 
	return STATUS_FAIL;	 
}

void vid_dec_get_size(INT16U *width, INT16U *height)
{
	*width = p_bitmap_info->biWidth;
	*height = p_bitmap_info->biHeight;
}

//audio info
void vid_dec_set_audio_flag(INT8S audio_flag)
{
	if(audio_flag)
		p_vid_dec_para->audio_flag = TRUE;
	else
		p_vid_dec_para->audio_flag = FALSE;
}

INT8S vid_dec_get_audio_flag(void)
{
	return p_vid_dec_para->audio_flag;
}

//audio decoder
INT32S vid_dec_set_aud_dec_work_mem(INT32U work_mem_size)
{
	p_vid_dec_para->work_mem_size = work_mem_size;
	p_vid_dec_para->work_mem_addr = (INT8U *)gp_malloc_align(work_mem_size+16, 4); // Add 16 bytes for fixing audio lib over memory , 
	if(!p_vid_dec_para->work_mem_addr)
		return -1;

	gp_memset((INT8S*)p_vid_dec_para->work_mem_addr, 0, work_mem_size+16);
	return 0;
}

INT32S vid_dec_set_aud_dec_ring_buffer(void) 
{
	p_vid_dec_para->ring_buffer_size = MultiMediaParser_GetAudRingLen(p_vid_dec_para->media_handle);
	p_vid_dec_para->ring_buffer_addr = (INT8U *)MultiMediaParser_GetAudRing(p_vid_dec_para->media_handle);
	if(!p_vid_dec_para->ring_buffer_addr)
		return -1;
		
	return 0;
}

void vid_dec_aud_dec_memory_free(void)
{
	if(p_vid_dec_para->work_mem_addr)
	{
		gp_free((void*)p_vid_dec_para->work_mem_addr);
	}
	p_vid_dec_para->work_mem_size = 0;
	p_vid_dec_para->work_mem_addr = 0;
	p_vid_dec_para->ring_buffer_size = 0;
	p_vid_dec_para->ring_buffer_addr = 0;
}

//audio dac
INT32S aud_dec_double_buffer_put(INT16U *pdata,INT32U cwlen, OS_EVENT *os_q)
{
	INT32S status;
	
	g_aud_dec_dma_dbf.s_addr = (INT32U) pdata;
	g_aud_dec_dma_dbf.t_addr = (INT32U) P_DAC_CHA_DATA;
	g_aud_dec_dma_dbf.width = DMA_DATA_WIDTH_2BYTE;		
	g_aud_dec_dma_dbf.count = (INT32U) cwlen;
	g_aud_dec_dma_dbf.notify = NULL;
	g_aud_dec_dma_dbf.timeout = 0;
	
	status = dma_transfer_with_double_buf(&g_aud_dec_dma_dbf, os_q);
	if (status != 0)
		return status;
		
	return STATUS_OK;
}

INT32U aud_dec_double_buffer_set(INT16U *pdata, INT32U cwlen)
{
	INT32S status;
	
	g_aud_dec_dma_dbf.s_addr = (INT32U) pdata;
	g_aud_dec_dma_dbf.t_addr = (INT32U) P_DAC_CHA_DATA;
	g_aud_dec_dma_dbf.width = DMA_DATA_WIDTH_2BYTE;		
	g_aud_dec_dma_dbf.count = (INT32U) cwlen;
	g_aud_dec_dma_dbf.notify = NULL;
	g_aud_dec_dma_dbf.timeout = 0;
	status = dma_transfer_double_buf_set(&g_aud_dec_dma_dbf);
	if(status != 0)
		return status;

	return STATUS_OK;
}

INT32S aud_dec_dma_status_get(void)
{
	if (g_aud_dec_dma_dbf.channel == 0xff) 
		return -1;
	
	return dma_status_get(g_aud_dec_dma_dbf.channel);	
}

void aud_dec_double_buffer_free(void)
{
	dma_transfer_double_buf_free(&g_aud_dec_dma_dbf);
	g_aud_dec_dma_dbf.channel = 0xff;
}

//dac init
void aud_dec_dac_start(INT8U channel_no, INT32U sample_rate)
{
	if(p_wave_info->wFormatTag == WAVE_FORMAT_MPEGLAYER3) //2 channel
		dac_stereo_set();
	else if(channel_no == 1)
		dac_mono_set();
	else
		dac_stereo_set();
		
	dac_sample_rate_set(sample_rate);
	drvl1_hdmi_audio_ctrl(1);
}

void aud_dec_dac_stop(void)
{
	dac_timer_stop();
	drvl1_hdmi_audio_ctrl(0);
}

void aud_dec_ramp_down_handle(INT8U channel_no)
{
	INT16S  last_ldata,last_rdata;
	INT16S  i, temp;
	
	temp = 0 - DAC_RAMP_DOWN_STEP;
	last_ldata = R_DAC_CHA_DATA;
	last_rdata = R_DAC_CHB_DATA;
	//unsigned to signed 
	last_ldata ^= 0x8000;
	last_rdata ^= 0x8000;
	
	//change timer to 44100
	dac_sample_rate_set(44100);
	while(1)
	{
		if (last_ldata > 0x0) 
		{
			last_ldata -= DAC_RAMP_DOWN_STEP;
		}
		else if (last_ldata < 0x0) 
		{
			last_ldata += DAC_RAMP_DOWN_STEP;
		}
			
		if ((last_ldata < DAC_RAMP_DOWN_STEP)&&(last_ldata > temp)) 
		{ 
			last_ldata = 0;
		}

		if (channel_no == 2) 
		{
			if (last_rdata > 0x0) 
			{
				last_rdata -= DAC_RAMP_DOWN_STEP;
		    }
			else if (last_rdata < 0x0) 
			{
				last_rdata += DAC_RAMP_DOWN_STEP;
		    }
		        
		    if ((last_rdata < DAC_RAMP_DOWN_STEP)&&(last_rdata > temp)) 
		    {  
				last_rdata = 0;
			}
		}
		    
		for(i=0;i<DAC_RAMP_DOWN_STEP_HOLD;i++) 
		{
			if (channel_no == 2)
			{
				while(R_DAC_CHA_FIFO & 0x8000);
				R_DAC_CHA_DATA = last_ldata;
				while(R_DAC_CHB_FIFO & 0x8000);
				R_DAC_CHB_DATA = last_rdata;
			} 
			else 
			{
				while(R_DAC_CHA_FIFO & 0x8000);
				R_DAC_CHA_DATA = last_ldata;
			}
		}
		
		if (channel_no == 2) 
		{
			if ((last_ldata == 0x0) && (last_rdata == 0x0)) 
				break;
		}
		else
		{
			if (last_ldata == 0x0)
				break;
		}
	}
	dac_timer_stop();
}

//
// Frame mode decoded, 
// frame addr : data_addr
// frame size : data_size
// 
#define JPEG_DEC_PIECE_SIZE 65536//(64<<1024) must be 16-byte alignment   // different from FIFO mode, slice read from JPEG file
INT32S jpeg_scaler_fifo_mode_flow(INT8U *data_addr, INT32U data_size)
{
    INT32S jpeg_status=0, scaler_status=0;
    INT8U  scaler_done=0 ;
    INT8U  *jpeg_dec_start ;
    INT32U jpeg_dec_len, jpeg_has_dec_len ;
    
    jpeg_dec_start = data_addr ;
    if ( (INT32U)data_addr & 0xFFFF)  // VLC addr not 16-byte alignment
    {
        jpeg_dec_len = JPEG_DEC_PIECE_SIZE - ((INT32U)data_addr & 0xFFFF) ;
        jpeg_has_dec_len = jpeg_dec_len ;
    }
    else
    {
        if(JPEG_DEC_PIECE_SIZE < data_size) {
            jpeg_dec_len = JPEG_DEC_PIECE_SIZE ;
            jpeg_has_dec_len = JPEG_DEC_PIECE_SIZE ;
        } else {
            jpeg_dec_len = data_size ;
            jpeg_has_dec_len = data_size ;
        }
    }

    jpeg_decode_on_the_fly_start(jpeg_dec_start, jpeg_dec_len); // start part of decode
    
	while(1) {
		// Now query whether JPEG decoding is done
		jpeg_status = jpeg_decode_status_query(1);
		
		if (jpeg_status & C_JPG_STATUS_DECODE_DONE) {

			while(1) {
				scaler_status = scaler_wait_idle(SCALER_1);

		  		if(scaler_status == C_SCALER_STATUS_STOP)
		  		{
					scaler_start(SCALER_1);
				}
				else if(scaler_status & C_SCALER_STATUS_DONE) 
				{	
					break;
				}
				else if(scaler_status & (C_SCALER_STATUS_TIMEOUT|C_SCALER_STATUS_INIT_ERR)) 
				{
					DEBUG_MSG(DBG_PRINT("Scaler failed to finish its job\r\n"));
					break;
				} 
				else if(scaler_status & C_SCALER_STATUS_INPUT_EMPTY) 
				{
		  			scaler_restart(SCALER_1);
		  		}
		  		else 
		  		{
			  		DEBUG_MSG(DBG_PRINT("Un-handled Scaler status!\r\n"));
			  		break;
			  	}
			}		
			break;
		} // if (jpeg_status & C_JPG_STATUS_DECODE_DONE) {
		
		if (jpeg_status & C_JPG_STATUS_INPUT_EMPTY) // 
		{
            jpeg_dec_start = &data_addr[jpeg_has_dec_len] ;
            if(JPEG_DEC_PIECE_SIZE < (data_size-jpeg_has_dec_len)) {
                jpeg_dec_len = JPEG_DEC_PIECE_SIZE ;
                jpeg_has_dec_len += JPEG_DEC_PIECE_SIZE ;
            } else {
                jpeg_dec_len = data_size-jpeg_has_dec_len ;
                jpeg_has_dec_len = data_size ;
            }		    

		    
            if (jpeg_status & C_JPG_STATUS_OUTPUT_FULL) {  // special case!!, C_JPG_STATUS_INPUT_EMPTY & C_JPG_STATUS_OUTPUT_FULL @ the same time
            
                if(!scaler_done) 
                {
    			  	scaler_status = scaler_wait_idle(SCALER_1);
    			  	if(scaler_status == C_SCALER_STATUS_STOP)
    			  	{
    					scaler_start(SCALER_1);
    			  	}
    			  	else if(scaler_status & C_SCALER_STATUS_DONE)
    			  	{
    			  		scaler_done = 1;
    			  	}
    			  	else if(scaler_status & (C_SCALER_STATUS_TIMEOUT|C_SCALER_STATUS_INIT_ERR)) 
    			  	{
    					DEBUG_MSG(DBG_PRINT("Scaler failed to finish its job\r\n"));
    					break;
    				} 
    				else if(scaler_status & C_SCALER_STATUS_INPUT_EMPTY) 
    				{
    			  		scaler_restart(SCALER_1);
    			  	}
    			  	else 
    			  	{
    			  		DEBUG_MSG(DBG_PRINT("Un-handled Scaler status!\r\n"));
    			  		break;
    			  	}
    			}            
                //DBG_PRINT("JPEG decoded: OUTPUT FULL & INPUT EMPTY=================================\r\n");
                jpeg_decode_on_the_fly_start2(jpeg_dec_start, jpeg_dec_len);
                jpeg_status &= ~C_JPG_STATUS_OUTPUT_FULL ;   
                jpeg_status &= ~C_JPG_STATUS_INPUT_EMPTY ; 
            }
            else {// for only C_JPG_STATUS_INPUT_EMPTY
                jpeg_decode_on_the_fly_start(jpeg_dec_start, jpeg_dec_len);
                jpeg_status &= ~C_JPG_STATUS_INPUT_EMPTY ;            
            }
		}
		
		if(jpeg_status & C_JPG_STATUS_OUTPUT_FULL) 
		{	// Start scaler to handle the full output FIFO now
		  	if(!scaler_done) 
		  	{
			  	scaler_status = scaler_wait_idle(SCALER_1);
			  	if(scaler_status == C_SCALER_STATUS_STOP)
			  	{
					scaler_start(SCALER_1);
			  	}
			  	else if(scaler_status & C_SCALER_STATUS_DONE)
			  	{
			  		scaler_done = 1;
			  	}
			  	else if(scaler_status & (C_SCALER_STATUS_TIMEOUT|C_SCALER_STATUS_INIT_ERR)) 
			  	{
					DEBUG_MSG(DBG_PRINT("Scaler failed to finish its job\r\n"));
					break;
				} 
				else if(scaler_status & C_SCALER_STATUS_INPUT_EMPTY) 
				{
			  		scaler_restart(SCALER_1);
			  	}
			  	else 
			  	{
			  		DEBUG_MSG(DBG_PRINT("Un-handled Scaler status!\r\n"));
			  		break;
			  	}
			}
		
	  		// Now restart JPEG to output to next FIFO
	  		if(jpeg_decode_output_restart()) 
	  		{
	  			DEBUG_MSG(DBG_PRINT("Failed to call jpeg_decode_output_restart()\r\n"));
	  			break;
	  		}
		}  // if(jpeg_status & C_JPG_STATUS_OUTPUT_FULL) 		

		if(jpeg_status & C_JPG_STATUS_STOP) 
		{
			DEBUG_MSG(DBG_PRINT("JPEG is not started!\r\n"));
			break;
		}
		if(jpeg_status & C_JPG_STATUS_TIMEOUT)
		{
			DEBUG_MSG(DBG_PRINT("JPEG execution timeout!\r\n"));
			break;
		}
		if(jpeg_status & C_JPG_STATUS_INIT_ERR) 
		{
			DEBUG_MSG(DBG_PRINT("JPEG init error!\r\n"));
			break;
		}
		if(jpeg_status & C_JPG_STATUS_RST_VLC_DONE) 
		{
			DEBUG_MSG(DBG_PRINT("JPEG Restart marker number is incorrect!\r\n"));
			break;
		}
		if(jpeg_status & C_JPG_STATUS_RST_MARKER_ERR) 
		{
			DEBUG_MSG(DBG_PRINT("JPEG Restart marker sequence error!\r\n"));
			break;
		}			
	} // while loop of JPEG decode 			
	return scaler_status ;		
}


// Decode JPEG to RGB565 format by FRAME mode (for AVI only)
INT32S video_decode_jpeg_as_rgb565(INT8U *data_addr, INT32U data_size, INT32U output_width, INT32U output_height, INT32U output_addr)
{
	INT16U img_valid_width, img_valid_height, img_extend_width, img_extend_height;
	INT32U jpg_temp_out_buf;
	INT32S jpeg_status, scaler_status;
	INT8U pad_line_num;
	INT8U auto_config;
	INT16U i;
	INT16U img_src_valid_width, img_src_valid_height;

	if (!data_addr || !output_width || !output_height) {
		return -1;
	}
	
	// Find JPEG header(0xFFD8)
	for (i=0; i<64; i++) {
		if (*data_addr == 0xFF) {
			break;
		} else {
			data_addr++;
		}
	}
	
	// Parse JPEG data header
	jpeg_decode_init();
	if (jpeg_decode_parse_header((INT8U *) data_addr, data_size) != JPEG_PARSE_OK) {
		return -1;
	}	
	img_valid_width = jpeg_decode_image_width_get();
	img_valid_height = jpeg_decode_image_height_get();
	img_extend_width = jpeg_decode_image_extended_width_get();
	img_extend_height = jpeg_decode_image_extended_height_get();
	img_src_valid_width = img_valid_width ;
	img_src_valid_height = img_valid_height ;
		
	// Setup JPEG and Scaler engine for decoding
	scaler_init(SCALER_1);
	//scaler_input_visible_pixels_set(SCALER_1,img_valid_width, img_valid_height-5);
   	//scaler_fifo_line_set(SCALER_1, C_SCALER_CTRL_IN_FIFO_DISABLE);		// Default disable 
	scaler_input_format_set(SCALER_1, C_SCALER_CTRL_IN_YUYV);
	#if DUAL_STREAM_FUNC_ENABLE
	if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
	{
		scaler_output_format_set(SCALER_1, C_SCALER_CTRL_OUT_YUYV);
	}
	else
	#endif
	{
		scaler_output_format_set(SCALER_1, C_SCALER_CTRL_OUT_RGB565);
	}
	scaler_out_of_boundary_mode_set(SCALER_1, 1);	
	scaler_out_of_boundary_color_set(SCALER_1, 0x008080);			// Black
	
	pad_line_num = 0;
	auto_config = 0;
	
	//---------------------------------------------------------------------------
	// to QVGA (TFT)
	//
	if (output_width==320 && output_height==240) {
		if ((img_valid_width==1920 && img_valid_height==1080)   ||  // 16:9, AVI, has FPS concern
            (img_valid_width==1280 && img_valid_height==720)    ||  // 16:9, AVI, has FPS concern			
            (img_valid_width==848  && img_valid_height==480)    ||   
            (img_valid_width==1440 && img_valid_height==1080) ||  // 4:3, AVI, has FPS concern     
            (img_valid_width==640  && img_valid_height==480) )    // 4:3, AVI, has FPS concern        
		{	
			if (img_valid_width>1280) {  // JPEG decode with 1/4 scale-down to 480x270, YUYV, SDRAM
			    jpeg_decode_scale_down_set(ENUM_JPG_DIV4);
			    img_extend_width >>= 2 ;
			    img_extend_height >>= 2 ;
			    img_valid_width >>= 2 ;
			    img_valid_height >>= 2 ;
			} else if (img_valid_width >= 848) { 
			    jpeg_decode_scale_down_set(ENUM_JPG_DIV2);
			    img_extend_width >>= 1 ;
			    img_extend_height >>= 1 ;
			    img_valid_width >>= 1 ;
			    img_valid_height >>= 1 ;
			} 
			
			if ((jpg_temp_out_buf=(INT32U)gp_malloc_align(img_extend_width*img_extend_height*2, 32))==0) {
			    DBG_PRINT("video_decode_jpeg_as_rgb565()_1 failed to allocate memory\r\n");
			    return -1 ;
			}
			scaler_input_pixels_set(SCALER_1, img_extend_width, img_extend_height);
			scaler_input_visible_pixels_set(SCALER_1, img_valid_width, img_valid_height);
			scaler_input_A_addr_set(SCALER_1, jpg_temp_out_buf, NULL, NULL);
			scaler_input_offset_set(SCALER_1,0x8000, 0x8000);

#if USE_PANEL_NAME==PANEL_T27P05_ILI8961 
            if (img_src_valid_width==1440 || img_src_valid_width==640) {  // 4:3
    			scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/240, (img_valid_height<<16)/240, 320, 240);
    			scaler_output_addr_set(SCALER_1, output_addr+(40<<1), NULL, NULL);		
    			
                for (i=0;i<40;i++) {  // clear first 40 pixels to black
                    *((INT16U*)output_addr+i)=0x0;
                }    			
    			
    		}else {  // 16:9 and others => full screen
    			scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/320, (img_valid_height<<16)/240, 320, 240);	
    			scaler_output_addr_set(SCALER_1, output_addr, NULL, NULL);
    			pad_line_num = 0;
    	    }
    		
#else			
            if (img_src_valid_width==1440  || img_src_valid_width==640) {  // 4:3
            	#if DUAL_STREAM_FUNC_ENABLE
       			if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
				{
    				scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/WIFI_JPEG_WIDTH, (img_valid_height<<16)/WIFI_JPEG_HEIGHT, WIFI_JPEG_WIDTH, WIFI_JPEG_HEIGHT);
				}
				else
				#endif
				{	            
    				scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/320, (img_valid_height<<16)/240, 320, 240);
    			}
    			scaler_output_addr_set(SCALER_1, output_addr, NULL, NULL);		
            } else {  // 16:9
            	#if DUAL_STREAM_FUNC_ENABLE
       			if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
				{
	    			scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/WIFI_JPEG_WIDTH, (img_valid_height<<16)/WIFI_JPEG_HEIGHT, WIFI_JPEG_WIDTH, WIFI_JPEG_HEIGHT);	
    				scaler_output_addr_set(SCALER_1, output_addr, NULL, NULL);    			
    				pad_line_num = 0;
				}
				else
				#endif
				{
	    			// 30 black lines(line 181~210) will be padded by Scaler here
	    			scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/320, (img_valid_height<<16)/180, 320, 210);	
	    			scaler_output_addr_set(SCALER_1, output_addr+(output_width*30*2), NULL, NULL);    			
	    			pad_line_num = 30;
	    		}
    		}
#endif			
        }else {
			// Auto config JPEG and Scaler
			auto_config = 1;
		}
	}	
	
	//---------------------------------------------------------------------------
	// to D1 or VGA
	//
	else if (output_width==TV_WIDTH && output_height==480) {
	
		if ((img_valid_width==1920 && img_valid_height==1080)   ||  // 16:9, AVI, has FPS concern
            (img_valid_width==1280 && img_valid_height==720)    ||  // 16:9, AVI, has FPS concern			
            (img_valid_width==848  && img_valid_height==480)    ||   
            (img_valid_width==1440 && img_valid_height==1080) ||  // 4:3, AVI, has FPS concern     
            (img_valid_width==640  && img_valid_height==480) )    // 4:3, AVI, has FPS concern        
		{	
			if (img_valid_width>=1280) {  // JPEG decode with 1/4 scale-down to 480x270, YUYV, SDRAM
			    jpeg_decode_scale_down_set(ENUM_JPG_DIV2);
			    img_extend_width >>= 1 ;
			    img_extend_height >>= 1 ;
			    img_valid_width >>= 1 ;
			    img_valid_height >>= 1 ;
			} 
			
			if ((jpg_temp_out_buf=(INT32U)gp_malloc_align(img_extend_width*img_extend_height*2, 32))==0) {
			    DBG_PRINT("video_decode_jpeg_as_rgb565()_2 failed to allocate memory\r\n");
			    return -1 ;
			}
			scaler_input_pixels_set(SCALER_1, img_extend_width, img_extend_height);
			scaler_input_visible_pixels_set(SCALER_1, img_valid_width, img_valid_height);
			scaler_input_A_addr_set(SCALER_1, jpg_temp_out_buf, NULL, NULL);


#if TV_WIDTH==720
            if (img_src_valid_width==1440 || img_src_valid_width==640) {  // 4:3
    			scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/640, (img_valid_height<<16)/480, TV_WIDTH, 480);
    			scaler_output_addr_set(SCALER_1, output_addr+(40<<1), NULL, NULL);	    			
    			pad_line_num = 30;
    			
                for (i=0;i<40;i++) {  // clear first 40 pixels to black
                    *((INT16U*)output_addr+i)=0x0;
                }    			
    			
            } else {  // 16:9
    			scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/TV_WIDTH, (img_valid_height<<16)/480, TV_WIDTH, 480);	
    			scaler_output_addr_set(SCALER_1, output_addr, NULL, NULL);    			
    		}
#elif TV_WIDTH==640
            if (img_src_valid_width==1920 || img_src_valid_width==1280 || img_src_valid_width==848 ) {  // 16:9 & 848x480
    			scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/640, (img_valid_height<<16)/360, 640, 420);	
    			scaler_output_addr_set(SCALER_1, output_addr+(output_width*60*2), NULL, NULL);
    			pad_line_num = 60;            			
            } else {  // 4:3 => full screen
    			scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/640, (img_valid_height<<16)/480, 640, 480);	
    			scaler_output_addr_set(SCALER_1, output_addr, NULL, NULL);    			
    		}
#endif
        }else {
			// Auto config JPEG and Scaler
			auto_config = 1;
		}	
	} else {
		// Auto config JPEG and Scaler
		auto_config = 1;	
	}
	

	if (!auto_config) {
		if (jpg_temp_out_buf && jpeg_decode_output_set(jpg_temp_out_buf, 0x0, 0x0, 0x0)) {
    		gp_free((void *)jpg_temp_out_buf);
			return -1;
		}
		
		// Start JPEG
		jpeg_decode_once_start((INT8U *) jpeg_decode_image_vlc_addr_get(), data_size<<1);
		// When JPEG is decoding image, we can fill the black padding lines at the same time
		if (pad_line_num) {
			INT32U *pad_ptr;
			INT32U cnt;
			
			pad_ptr = (INT32U *) output_addr;
			cnt = output_width * pad_line_num >> 1;
			while (cnt--) {
				*pad_ptr++ = 0;
			}
		}
		jpeg_status = jpeg_decode_status_query(1);
		
		// Start Scaler
		if (jpeg_status & C_JPG_STATUS_DECODE_DONE) {
			scaler_start(SCALER_1);
			scaler_status = scaler_wait_idle(SCALER_1);
		} else {
			scaler_status = C_SCALER_STATUS_STOP;
		}
	} else if (auto_config) {
		if (img_extend_width>=2000) {
			jpeg_decode_scale_down_set(ENUM_JPG_DIV4);
			img_valid_width >>= 2;
			img_valid_height >>= 2;
			img_extend_width >>= 2;
			img_extend_height >>= 2;
		} else if (img_extend_width>=1280) {
			jpeg_decode_scale_down_set(ENUM_JPG_DIV2);
			img_valid_width >>= 1;
			img_valid_height >>= 1;
			img_extend_width >>= 1;
			img_extend_height >>= 1;
		}
		
		// TBD: use JPEG output iRAM FIFO mode instead SDRAM frame mode here
		//jpg_temp_out_buf = (INT32U) gp_malloc_align(img_extend_width*img_extend_height*2, 32);
		//jpg_temp_out_buf = (INT32U)display_hdmi_frame[0];
		//if (!jpg_temp_out_buf) {
		//	return -1;
		//}
		
		if ((jpg_temp_out_buf=(INT32U)gp_malloc_align(img_extend_width*img_extend_height*2, 32))==0) {
		    DBG_PRINT("video_decode_jpeg_as_rgb565()_10 failed to allocate memory\r\n");
		    return -1 ;
        }
		
		
		// Start JPEG
		if (jpeg_decode_output_set(jpg_temp_out_buf, 0x0, 0x0, 0x0) ||
			jpeg_decode_once_start((INT8U *) jpeg_decode_image_vlc_addr_get(), data_size<<1)) {
			return -1;
		}
			
			
		// When JPEG is decoding image, we can fill the black padding lines at the same time
       	#if DUAL_STREAM_FUNC_ENABLE
		if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
		#endif
		{
			if (3*img_valid_width >= 5*img_valid_height) {
				INT32U *pad_ptr;
				INT32U cnt;
				
				pad_line_num = (output_height - output_width*img_valid_height/img_valid_width) >> 1;
				pad_ptr = (INT32U *) output_addr;
				cnt = output_width * pad_line_num >> 1;
				while (cnt--) {
					*pad_ptr++ = 0;
				}
			}
		}
		
		// Now query whether JPEG decoding is done
		jpeg_status = jpeg_decode_status_query(1);
								
		if (jpeg_status & C_JPG_STATUS_DECODE_DONE) {
			// Config and start Scaler
			scaler_input_pixels_set(SCALER_1, img_extend_width, img_extend_height);
			scaler_input_visible_pixels_set(SCALER_1, img_valid_width, img_valid_height);				
			scaler_input_A_addr_set(SCALER_1, jpg_temp_out_buf, NULL, NULL);
			scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/output_width, (img_valid_height<<16)/(output_height-(pad_line_num<<1)), output_width, output_height-pad_line_num);	
			scaler_output_addr_set(SCALER_1, output_addr+(output_width*pad_line_num<<1), NULL, NULL);
			
			scaler_start(SCALER_1);
			scaler_status = scaler_wait_idle(SCALER_1);
		} else {
			scaler_status = C_SCALER_STATUS_STOP;
		}
	}
	
	if (jpg_temp_out_buf) {
		gp_free((void *)jpg_temp_out_buf);
	}	

	// STOP JPEG and Scaler
	jpeg_decode_stop();
	scaler_stop(SCALER_1);
	
	if (scaler_status & C_SCALER_STATUS_DONE) { 
		return 0;
	}
	
	return -1;
}

INT32S video_decode_jpeg_as_yuv422(INT8U *data_addr, INT32U data_size, INT32U output_width, INT32U output_height, INT32U output_addr)
{
	INT16U img_valid_width, img_valid_height, img_extend_width, img_extend_height;
	INT32U jpg_temp_out_buf;
	INT32S jpeg_status, scaler_status;
	INT8U pad_line_num;
	INT8U auto_config;
	INT16U i;
	INT16U img_src_valid_width, img_src_valid_height;

	if (!data_addr || !output_width || !output_height) {
		return -1;
	}
	
	// Find JPEG header(0xFFD8)
	for (i=0; i<64; i++) {
		if (*data_addr == 0xFF) {
			break;
		} else {
			data_addr++;
		}
	}
	
	// Parse JPEG data header
	jpeg_decode_init();
	if (jpeg_decode_parse_header((INT8U *) data_addr, data_size) != JPEG_PARSE_OK) {
		return -1;
	}	
	img_valid_width = jpeg_decode_image_width_get();
	img_valid_height = jpeg_decode_image_height_get();
	img_extend_width = jpeg_decode_image_extended_width_get();
	img_extend_height = jpeg_decode_image_extended_height_get();
	img_src_valid_width = img_valid_width ;
	img_src_valid_height = img_valid_height ;
		
	// Setup JPEG and Scaler engine for decoding
	scaler_init(SCALER_1);
	//scaler_input_visible_pixels_set(SCALER_1,img_valid_width, img_valid_height-5);
   	//scaler_fifo_line_set(SCALER_1, C_SCALER_CTRL_IN_FIFO_DISABLE);		// Default disable 
	scaler_input_format_set(SCALER_1, C_SCALER_CTRL_IN_YUYV);
	scaler_output_format_set(SCALER_1, C_SCALER_CTRL_OUT_YUYV);
	scaler_out_of_boundary_mode_set(SCALER_1, 1);	
	scaler_out_of_boundary_color_set(SCALER_1, 0x008080);			// Black
	
	pad_line_num = 0;
	auto_config = 0;
	
	//---------------------------------------------------------------------------
	// to QVGA (TFT)
	//
	if (output_width==320 && output_height==240) {
		if ((img_valid_width==1920 && img_valid_height==1080)   ||  // 16:9, AVI, has FPS concern
            (img_valid_width==1280 && img_valid_height==720)    ||  // 16:9, AVI, has FPS concern			
            (img_valid_width==848  && img_valid_height==480)    ||   
            (img_valid_width==1440 && img_valid_height==1080) ||  // 4:3, AVI, has FPS concern     
            (img_valid_width==640  && img_valid_height==480) )    // 4:3, AVI, has FPS concern        
		{	
			if (img_valid_width>1280) {  // JPEG decode with 1/4 scale-down to 480x270, YUYV, SDRAM
			    jpeg_decode_scale_down_set(ENUM_JPG_DIV4);
			    img_extend_width >>= 2 ;
			    img_extend_height >>= 2 ;
			    img_valid_width >>= 2 ;
			    img_valid_height >>= 2 ;
			} else if (img_valid_width >= 848) { 
			    jpeg_decode_scale_down_set(ENUM_JPG_DIV2);
			    img_extend_width >>= 1 ;
			    img_extend_height >>= 1 ;
			    img_valid_width >>= 1 ;
			    img_valid_height >>= 1 ;
			} 
			
			if ((jpg_temp_out_buf=(INT32U)gp_malloc_align(img_extend_width*img_extend_height*2, 32))==0) {
			    DBG_PRINT("video_decode_jpeg_as_rgb565()_1 failed to allocate memory\r\n");
			    return -1 ;
			}
			scaler_input_pixels_set(SCALER_1, img_extend_width, img_extend_height);
			scaler_input_visible_pixels_set(SCALER_1, img_valid_width, img_valid_height);
			scaler_input_A_addr_set(SCALER_1, jpg_temp_out_buf, NULL, NULL);
			scaler_input_offset_set(SCALER_1,0x8000, 0x8000);
	
            //if (img_src_valid_width==1440  || img_src_valid_width==640) {  // 4:3
    			scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/320, (img_valid_height<<16)/240, 320, 240);
    			scaler_output_addr_set(SCALER_1, output_addr, NULL, NULL);		
            /*} else {  // 16:9
    			// 30 black lines(line 181~210) will be padded by Scaler here
    			scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/320, (img_valid_height<<16)/180, 320, 210);	
    			scaler_output_addr_set(SCALER_1, output_addr+(output_width*30*2), NULL, NULL);
    			pad_line_num = 30;
    		}*/		
        }else {
			// Auto config JPEG and Scaler
			auto_config = 1;
		}
	}	
	else {
		// Auto config JPEG and Scaler
		auto_config = 1;	
	}
	

	if (!auto_config) {
		if (jpg_temp_out_buf && jpeg_decode_output_set(jpg_temp_out_buf, 0x0, 0x0, 0x0)) {
    		gp_free((void *)jpg_temp_out_buf);
			return -1;
		}
		
		// Start JPEG
		jpeg_decode_once_start((INT8U *) jpeg_decode_image_vlc_addr_get(), data_size<<1);
		// When JPEG is decoding image, we can fill the black padding lines at the same time
		if (pad_line_num) {
			INT32U *pad_ptr;
			INT32U cnt;
			
			pad_ptr = (INT32U *) output_addr;
			cnt = output_width * pad_line_num >> 1;
			while (cnt--) {
				*pad_ptr++ = 0;
			}
		}
		jpeg_status = jpeg_decode_status_query(1);
		
		// Start Scaler
		if (jpeg_status & C_JPG_STATUS_DECODE_DONE) {
			scaler_start(SCALER_1);
			scaler_status = scaler_wait_idle(SCALER_1);
		} else {
			scaler_status = C_SCALER_STATUS_STOP;
		}
	} else if (auto_config) {
		if (img_extend_width>=2000) {
			jpeg_decode_scale_down_set(ENUM_JPG_DIV4);
			img_valid_width >>= 2;
			img_valid_height >>= 2;
			img_extend_width >>= 2;
			img_extend_height >>= 2;
		} else if (img_extend_width>=1280) {
			jpeg_decode_scale_down_set(ENUM_JPG_DIV2);
			img_valid_width >>= 1;
			img_valid_height >>= 1;
			img_extend_width >>= 1;
			img_extend_height >>= 1;
		}
		
		// TBD: use JPEG output iRAM FIFO mode instead SDRAM frame mode here
		//jpg_temp_out_buf = (INT32U) gp_malloc_align(img_extend_width*img_extend_height*2, 32);
		//jpg_temp_out_buf = (INT32U)display_hdmi_frame[0];
		//if (!jpg_temp_out_buf) {
		//	return -1;
		//}
		
		if ((jpg_temp_out_buf=(INT32U)gp_malloc_align(img_extend_width*img_extend_height*2, 32))==0) {
		    DBG_PRINT("video_decode_jpeg_as_rgb565()_10 failed to allocate memory\r\n");
		    return -1 ;
        }
		
		
		// Start JPEG
		if (jpeg_decode_output_set(jpg_temp_out_buf, 0x0, 0x0, 0x0) ||
			jpeg_decode_once_start((INT8U *) jpeg_decode_image_vlc_addr_get(), data_size<<1)) {
			return -1;
		}
			
			
		// When JPEG is decoding image, we can fill the black padding lines at the same time
		if (3*img_valid_width >= 5*img_valid_height) {
			INT32U *pad_ptr;
			INT32U cnt;
			
			pad_line_num = (output_height - output_width*img_valid_height/img_valid_width) >> 1;
			pad_ptr = (INT32U *) output_addr;
			cnt = output_width * pad_line_num >> 1;
			while (cnt--) {
				*pad_ptr++ = 0;
			}
		}
		
		// Now query whether JPEG decoding is done
		jpeg_status = jpeg_decode_status_query(1);
								
		if (jpeg_status & C_JPG_STATUS_DECODE_DONE) {
			// Config and start Scaler
			scaler_input_pixels_set(SCALER_1, img_extend_width, img_extend_height);
			scaler_input_visible_pixels_set(SCALER_1, img_valid_width, img_valid_height);				
			scaler_input_A_addr_set(SCALER_1, jpg_temp_out_buf, NULL, NULL);
			scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/output_width, (img_valid_height<<16)/(output_height-(pad_line_num<<1)), output_width, output_height-pad_line_num);	
			scaler_output_addr_set(SCALER_1, output_addr+(output_width*pad_line_num<<1), NULL, NULL);
			
			scaler_start(SCALER_1);
			scaler_status = scaler_wait_idle(SCALER_1);
		} else {
			scaler_status = C_SCALER_STATUS_STOP;
		}
	}
	
	if (jpg_temp_out_buf) {
		gp_free((void *)jpg_temp_out_buf);
	}	
	
	// STOP JPEG and Scaler
	jpeg_decode_stop();
	scaler_stop(SCALER_1);
	
	if (scaler_status & C_SCALER_STATUS_DONE) { 
		return 0;
	}
	
	return -1;
}

INT32S jpeg_scaler_fifo_mode_flow_by_piece(INT16S fd, INT32U file_size, INT32U buff_addr)
{
    INT32S jpeg_status=0, scaler_status=0;
    INT8U  scaler_done=0 ;
    INT8U  *jpeg_dec_start ;
    INT32U jpeg_dec_len, jpeg_has_dec_len ;
    INT8U *data_addr ;
    data_addr = (INT8U *) jpeg_decode_image_vlc_addr_get() ;

    // decode 1st data piece of jpeg raw data
    jpeg_dec_start = data_addr ;
    jpeg_dec_len = JPEG_DEC_PIECE_SIZE- ((INT32U)data_addr - buff_addr) ;
    jpeg_has_dec_len = jpeg_dec_len ;
    

    jpeg_decode_on_the_fly_start(jpeg_dec_start, jpeg_dec_len); // start part of decode
    
	while(1) {
		// Now query whether JPEG decoding is done
		jpeg_status = jpeg_decode_status_query(1);
		
		if (jpeg_status & C_JPG_STATUS_DECODE_DONE) {
		
			while(1) {
				scaler_status = scaler_wait_idle(SCALER_1);

		  		if(scaler_status == C_SCALER_STATUS_STOP)
		  		{
					scaler_start(SCALER_1);
				}
				else if(scaler_status & C_SCALER_STATUS_DONE) 
				{				
					break;
				}
				else if(scaler_status & (C_SCALER_STATUS_TIMEOUT|C_SCALER_STATUS_INIT_ERR)) 
				{
					DEBUG_MSG(DBG_PRINT("Scaler failed to finish its job\r\n"));
					break;
				} 
				else if(scaler_status & C_SCALER_STATUS_INPUT_EMPTY) 
				{
		  			scaler_restart(SCALER_1);
		  		}
		  		else 
		  		{
			  		DEBUG_MSG(DBG_PRINT("Un-handled Scaler status!\r\n"));
			  		break;
			  	}
			}		
	
			break;
		} // if (jpeg_status & C_JPG_STATUS_DECODE_DONE) {
		
		if (jpeg_status & C_JPG_STATUS_INPUT_EMPTY) // 
		{
            if(JPEG_DEC_PIECE_SIZE < (file_size-jpeg_has_dec_len)) {
                jpeg_dec_len = JPEG_DEC_PIECE_SIZE ;
                jpeg_has_dec_len += JPEG_DEC_PIECE_SIZE ;
            } else {
                jpeg_dec_len = file_size-jpeg_has_dec_len ;
                jpeg_has_dec_len = file_size ;
            }		    

        	if (read(fd, buff_addr, jpeg_dec_len) <= 0) {
        		DBG_PRINT("jpeg_scaler_fifo_mode_flow_by_piece read jpeg file fail.\r\n");
        		return STATUS_FAIL;
        	}	
            
            if (jpeg_status & C_JPG_STATUS_OUTPUT_FULL) {  // special case!!, C_JPG_STATUS_INPUT_EMPTY & C_JPG_STATUS_OUTPUT_FULL @ the same time
                //DBG_PRINT("JPEG decoded by piece: OUTPUT FULL & INPUT EMPTY=================================\r\n");
                
    		  	if(!scaler_done) 
    		  	{
    			  	scaler_status = scaler_wait_idle(SCALER_1);
    			  	if(scaler_status == C_SCALER_STATUS_STOP)
    			  	{
    					scaler_start(SCALER_1);
    			  	}
    			  	else if(scaler_status & C_SCALER_STATUS_DONE)
    			  	{
    			  		scaler_done = 1;
    			  	}
    			  	else if(scaler_status & (C_SCALER_STATUS_TIMEOUT|C_SCALER_STATUS_INIT_ERR)) 
    			  	{
    					DEBUG_MSG(DBG_PRINT("Scaler failed to finish its job\r\n"));
    					break;
    				} 
    				else if(scaler_status & C_SCALER_STATUS_INPUT_EMPTY) 
    				{
    			  		scaler_restart(SCALER_1);
    			  	}
    			  	else 
    			  	{
    			  		DEBUG_MSG(DBG_PRINT("Un-handled Scaler status!\r\n"));
    			  		break;
    			  	}
    			}                  
                jpeg_decode_on_the_fly_start2((INT8U *)buff_addr, jpeg_dec_len);
                jpeg_status &= ~C_JPG_STATUS_OUTPUT_FULL ;   
                jpeg_status &= ~C_JPG_STATUS_INPUT_EMPTY ;                 
            }
            else {// for only C_JPG_STATUS_INPUT_EMPTY
                jpeg_decode_on_the_fly_start((INT8U *)buff_addr, jpeg_dec_len);
                jpeg_status &= ~C_JPG_STATUS_INPUT_EMPTY ;
            }
		}
		
		if(jpeg_status & C_JPG_STATUS_OUTPUT_FULL) 
		{	// Start scaler to handle the full output FIFO now
		  	if(!scaler_done) 
		  	{
			  	scaler_status = scaler_wait_idle(SCALER_1);
			  	if(scaler_status == C_SCALER_STATUS_STOP)
			  	{
					scaler_start(SCALER_1);
			  	}
			  	else if(scaler_status & C_SCALER_STATUS_DONE)
			  	{
			  		scaler_done = 1;
			  	}
			  	else if(scaler_status & (C_SCALER_STATUS_TIMEOUT|C_SCALER_STATUS_INIT_ERR)) 
			  	{
					DEBUG_MSG(DBG_PRINT("Scaler failed to finish its job\r\n"));
					break;
				} 
				else if(scaler_status & C_SCALER_STATUS_INPUT_EMPTY) 
				{
			  		scaler_restart(SCALER_1);
			  	}
			  	else 
			  	{
			  		DEBUG_MSG(DBG_PRINT("Un-handled Scaler status!\r\n"));
			  		break;
			  	}
			}
		
	  		// Now restart JPEG to output to next FIFO
	  		if(jpeg_decode_output_restart()) 
	  		{
	  			DEBUG_MSG(DBG_PRINT("Failed to call jpeg_decode_output_restart()\r\n"));
	  			break;
	  		}
		}  // if(jpeg_status & C_JPG_STATUS_OUTPUT_FULL) 		

		if(jpeg_status & C_JPG_STATUS_STOP) 
		{
			DEBUG_MSG(DBG_PRINT("JPEG is not started!\r\n"));
			break;
		}
		if(jpeg_status & C_JPG_STATUS_TIMEOUT)
		{
			DEBUG_MSG(DBG_PRINT("JPEG execution timeout!\r\n"));
			break;
		}
		if(jpeg_status & C_JPG_STATUS_INIT_ERR) 
		{
			DEBUG_MSG(DBG_PRINT("JPEG init error!\r\n"));
			break;
		}
		if(jpeg_status & C_JPG_STATUS_RST_VLC_DONE) 
		{
			DEBUG_MSG(DBG_PRINT("JPEG Restart marker number is incorrect!\r\n"));
			break;
		}
		if(jpeg_status & C_JPG_STATUS_RST_MARKER_ERR) 
		{
			DEBUG_MSG(DBG_PRINT("JPEG Restart marker sequence error!\r\n"));
			break;
		}			
	} // while loop of JPEG decode 			
	return scaler_status ;		
}




// Decode JPEG image only, not include AVI.
INT32S video_decode_jpeg_as_rgb565_by_piece(INT16S fd, INT32U file_size, INT32U output_width, INT32U output_height, INT32U output_addr)
{
	INT16U img_valid_width, img_valid_height, img_extend_width, img_extend_height;
	INT32U jpg_temp_out_buf;
	INT32S scaler_status;
	INT8U pad_line_num;
	INT8U auto_config;
	INT16U i;
	INT32U size_to_read;
	INT8U *data_addr;
	INT32U file_buff_addr ;
	INT16U img_src_valid_width, img_src_valid_height ;
	

	INT32U fifo_line;
	SCALER_MAS scaler1_mas;
	//INT8U scaler_done=0 ;

	if (fd<0 || !output_width || !output_height || !output_addr) {
		return -1;
	}
	

	// allocate buffer to decode
	file_buff_addr = (INT32U)gp_malloc_align(JPEG_DEC_PIECE_SIZE, 16);
	if (file_buff_addr==0) {
	    DBG_PRINT("video_decode_jpeg_as_rgb565_by_piece failed to allocate memory\r\n");
	    return -1;
	}
	
	// read file header or entire file
	if (file_size<=JPEG_DEC_PIECE_SIZE)
	    size_to_read = file_size;
	else
	    size_to_read = JPEG_DEC_PIECE_SIZE ;
    
	if (read(fd, file_buff_addr, size_to_read) <= 0) {
		gp_free((void *) file_buff_addr);
		DBG_PRINT("video_decode_jpeg_as_rgb565_by_piece read jpeg file fail.\r\n");
		return STATUS_FAIL;
	}	
	
	data_addr = (INT8U *)file_buff_addr ;
	// Find JPEG header(0xFFD8)
	for (i=0; i<64; i++) {
		if (*data_addr == 0xFF) {
			break;
		} else {
			data_addr++;
		}
	}
	
	// Parse JPEG data header
	jpeg_decode_init();
	if (jpeg_decode_parse_header((INT8U *) data_addr, size_to_read) != JPEG_PARSE_OK) {
		return -1;
	}
	img_valid_width = jpeg_decode_image_width_get();
	img_valid_height = jpeg_decode_image_height_get();
	img_extend_width = jpeg_decode_image_extended_width_get();
	img_extend_height = jpeg_decode_image_extended_height_get();
	img_src_valid_width = img_valid_width ;
	img_src_valid_height = img_valid_height ;
		
	// Setup JPEG and Scaler engine for decoding
	scaler_init(SCALER_1);
   	//scaler_fifo_line_set(SCALER_1, C_SCALER_CTRL_IN_FIFO_DISABLE);		// Default disable 
	scaler_input_format_set(SCALER_1, C_SCALER_CTRL_IN_YUYV);
	#if DUAL_STREAM_FUNC_ENABLE
	if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
	{
		scaler_output_format_set(SCALER_1, C_SCALER_CTRL_OUT_YUYV);
	}
	else
	#endif
	{
		scaler_output_format_set(SCALER_1, C_SCALER_CTRL_OUT_RGB565);
	}
	scaler_out_of_boundary_mode_set(SCALER_1, 1);	
	scaler_out_of_boundary_color_set(SCALER_1, 0x008080);			// Black
	
	pad_line_num = 0;
	auto_config = 0;
	
	//---------------------------------------------------------------------------
	// to QVGA (TFT)
	//
	if (output_width==320 && output_height==240) {

	    if (img_valid_width>2000) {
			jpeg_decode_scale_down_set(ENUM_JPG_DIV4);
			img_valid_width >>= 2;
			img_valid_height >>= 2;
			img_extend_width >>= 2;
			img_extend_height >>= 2;
        } else if ((img_valid_width>=(output_width<<2)) && (img_valid_height>=(output_height<<2))) {
			jpeg_decode_scale_down_set(ENUM_JPG_DIV4);
			img_valid_width >>= 2;
			img_valid_height >>= 2;
			img_extend_width >>= 2;
			img_extend_height >>= 2;
			
		} else if ((img_valid_width>=(output_width<<1)) && (img_valid_height>=(output_height<<1))) {
			jpeg_decode_scale_down_set(ENUM_JPG_DIV2);
			img_valid_width >>= 1;
			img_valid_height >>= 1;
			img_extend_width >>= 1;
			img_extend_height >>= 1;
		}

		fifo_line = 0 ;
		if (img_valid_width<=1718) { // FIFO=32 lines
			fifo_line = 32 ;
		} else if (img_valid_width<=3436) { // FIFO=16 lines
			fifo_line = 16 ;
		} 

		if ((jpg_temp_out_buf=(INT32U)gp_malloc_align(img_extend_width*fifo_line*2*2, 32))==0) {
		    DBG_PRINT("video_decode_jpeg_as_rgb565_by_piece failed to allocate memory\r\n");
		    return -1 ;
        }
		scaler_input_fifo_line_set(SCALER_1,fifo_line) ;
		scaler_input_A_addr_set(SCALER_1,jpg_temp_out_buf, 0, 0) ;
		scaler_input_B_addr_set(SCALER_1,jpg_temp_out_buf+img_extend_width*fifo_line*2, 0, 0) ;
		scaler_input_pixels_set(SCALER_1, img_extend_width, img_extend_height);
		scaler_input_visible_pixels_set(SCALER_1, img_valid_width, img_valid_height);	
        scaler_input_offset_set(SCALER_1,0x8000, 0x8000);
        scaler_output_fifo_line_set(SCALER_1,0) ;
  
#if USE_PANEL_NAME==PANEL_T27P05_ILI8961		
        if (img_src_valid_width==1920 && img_src_valid_height==1080) {  // for 16:9 ratio image, full screen
    		scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/320, (img_valid_height<<16)/240, 320, 240);
	    	scaler_output_addr_set(SCALER_1, output_addr, NULL, NULL);		
	    } else { // for 4:3 and other ratio
    		scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/240, (img_valid_height<<16)/240, 320, 240);
    		scaler_output_addr_set(SCALER_1, output_addr+(40<<1), NULL, NULL);		
        }
#else
        if (img_src_valid_width==1920 && img_src_valid_height==1080) {  // for 16:9 ratio image
        	#if DUAL_STREAM_FUNC_ENABLE
   			if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
			{
				scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/WIFI_JPEG_WIDTH, (img_valid_height<<16)/WIFI_JPEG_HEIGHT, WIFI_JPEG_WIDTH, WIFI_JPEG_HEIGHT);	
				scaler_output_addr_set(SCALER_1, output_addr, NULL, NULL);
				pad_line_num = 0;        
			}
			else
			#endif
			{        
				// 30 black lines(line 181~210) will be padded by Scaler here
				for(i=0; i< (output_width*30*2)>>2; i++)
	    			((INT32U *)output_addr)[i] = 0 ;
				scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/320, (img_valid_height<<16)/180, 320, 210);	
				scaler_output_addr_set(SCALER_1, output_addr+(output_width*30*2), NULL, NULL);
				pad_line_num = 30;        
			}
        }else {  // for 4:3 and other ratio
        	#if DUAL_STREAM_FUNC_ENABLE
   			if(Wifi_State_Get() == WIFI_STATE_FLAG_CONNECT)
			{
    			scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/WIFI_JPEG_WIDTH, (img_valid_height<<16)/WIFI_JPEG_HEIGHT, WIFI_JPEG_WIDTH, WIFI_JPEG_HEIGHT);
			}
			else
			#endif
        	{
    			scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/320, (img_valid_height<<16)/240, 320, 240);
    		}
	    	scaler_output_addr_set(SCALER_1, output_addr, NULL, NULL);		
	    }
#endif    
	}	
	
	//---------------------------------------------------------------------------
	// to D1 or VGA (TV)
	//
	else if (output_width==TV_WIDTH && output_height==480) {
	
	    if (img_valid_width>2000) {
			jpeg_decode_scale_down_set(ENUM_JPG_DIV4);
			img_valid_width >>= 2;
			img_valid_height >>= 2;
			img_extend_width >>= 2;
			img_extend_height >>= 2;
        } else if (img_valid_width>=1280) {
			jpeg_decode_scale_down_set(ENUM_JPG_DIV2);
			img_valid_width >>= 1;
			img_valid_height >>= 1;
			img_extend_width >>= 1;
			img_extend_height >>= 1;
    	}	

		fifo_line = 0 ;
		if (img_valid_width<=1718) { // FIFO=32 lines
			fifo_line = 32 ;
		} else if (img_valid_width<=3436) { // FIFO=16 lines
			fifo_line = 16 ;
		} 

		if ((jpg_temp_out_buf=(INT32U)gp_malloc_align(img_extend_width*fifo_line*2*2, 32))==0) {
		    DBG_PRINT("video_decode_jpeg_as_rgb565_by_piece failed to allocate memory\r\n");
		    return -1 ;
        }
		scaler_input_fifo_line_set(SCALER_1,fifo_line) ;
		scaler_input_A_addr_set(SCALER_1,jpg_temp_out_buf, 0, 0) ;
		scaler_input_B_addr_set(SCALER_1,jpg_temp_out_buf+img_extend_width*fifo_line*2, 0, 0) ;
		scaler_input_pixels_set(SCALER_1, img_extend_width, img_extend_height);
		scaler_input_visible_pixels_set(SCALER_1, img_valid_width, img_valid_height);	
        
        scaler_output_fifo_line_set(SCALER_1,0) ;
  
//		scaler_output_pixels_set(SCALER_1, (img_extend_width<<16)/640, (img_extend_height<<16)/480, TV_WIDTH, 480);
//		scaler_output_addr_set(SCALER_1, output_addr+(40<<1), NULL, NULL);	
		
		
#if TV_WIDTH==720
            if (img_src_valid_width==1440 || img_src_valid_width==640) {  // 4:3
    			scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/640, (img_valid_height<<16)/480, TV_WIDTH, 480);
    			scaler_output_addr_set(SCALER_1, output_addr+(40<<1), NULL, NULL);	    			
    			pad_line_num = 30;
            } else {  // 16:9
    			scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/TV_WIDTH, (img_valid_height<<16)/480, TV_WIDTH, 480);	
    			scaler_output_addr_set(SCALER_1, output_addr, NULL, NULL);    			
    		}
#elif TV_WIDTH==640
            if (img_src_valid_width==1920) {  // 16:9
    			// 30 black lines(line 181~210) will be padded by Scaler here
    			for(i=0; i< (output_width*60*2)>>2; i++)
        			((INT32U *)output_addr)[i] = 0 ;
    			scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/640, (img_valid_height<<16)/360, 640, 420);	
    			scaler_output_addr_set(SCALER_1, output_addr+(output_width*60*2), NULL, NULL);
    			pad_line_num = 30;            			
            } else {  // 4:3 => full screen
    			scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/640, (img_valid_height<<16)/480, 640, 480);	
    			scaler_output_addr_set(SCALER_1, output_addr, NULL, NULL);    			
    		}
#endif    		
    } else { // thumbnail case

		jpeg_decode_scale_down_set(ENUM_JPG_DIV4);
		img_valid_width >>= 2;
		img_valid_height >>= 2;
		img_extend_width >>= 2;
		img_extend_height >>= 2;

		fifo_line = 0 ;
		if (img_valid_width<=1718) { // FIFO=32 lines
			fifo_line = 32 ;
		} else if (img_valid_width<=3436) { // FIFO=16 lines
			fifo_line = 16 ;
		} 

		if ((jpg_temp_out_buf=(INT32U)gp_malloc_align(img_extend_width*fifo_line*2*2, 32))==0) {
		    DBG_PRINT("video_decode_jpeg_as_rgb565_by_piece failed to allocate memory\r\n");
		    return -1 ;
        }
		scaler_input_fifo_line_set(SCALER_1,fifo_line) ;
		scaler_input_A_addr_set(SCALER_1,jpg_temp_out_buf, 0, 0) ;
		scaler_input_B_addr_set(SCALER_1,jpg_temp_out_buf+img_extend_width*fifo_line*2, 0, 0) ;
		scaler_input_pixels_set(SCALER_1, img_extend_width, img_extend_height);
		scaler_input_visible_pixels_set(SCALER_1, img_valid_width, img_valid_height);	
        
        scaler_output_fifo_line_set(SCALER_1,0) ;
  
		scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/output_width, (img_valid_height<<16)/output_height, output_width, output_height);
		scaler_output_addr_set(SCALER_1, output_addr, NULL, NULL);	
    }
	
	// set scatop_1 path
	gp_memset((INT8S *)&scaler1_mas,0,sizeof(SCALER_MAS));
	scaler1_mas.mas_0 = MAS_EN_READ|MAS_EN_WRITE;
	scaler_mas_set(SCALER_1,&scaler1_mas);									
						
	// Start JPEG decode
	if (jpeg_decode_output_set(jpg_temp_out_buf, 0x0, 0x0, fifo_line)) { 
		gp_free((void *) jpg_temp_out_buf);
		return -1;
	}
		

    scaler_status = jpeg_scaler_fifo_mode_flow_by_piece(fd, file_size, file_buff_addr);					
	if (jpg_temp_out_buf)
		gp_free((void *)jpg_temp_out_buf);
	if (file_buff_addr)											
	    gp_free((void *)file_buff_addr);

	// STOP JPEG and Scaler
	jpeg_decode_stop();
	scaler_stop(SCALER_1);
	
	if (scaler_status & C_SCALER_STATUS_DONE) { 
		return 0;
	}
	return -1;		    
}


INT32S video_decode_jpeg_as_yuv422_by_piece(INT16S fd, INT32U file_size, INT32U output_width, INT32U output_height, INT32U output_addr)
{
	INT16U img_valid_width, img_valid_height, img_extend_width, img_extend_height;
	INT32U jpg_temp_out_buf;
	INT32S scaler_status;
	INT8U pad_line_num;
	INT8U auto_config;
	INT16U i;
	INT32U size_to_read;
	INT8U *data_addr;
	INT32U file_buff_addr ;
	INT16U img_src_valid_width, img_src_valid_height ;
	

	INT32U fifo_line;
	SCALER_MAS scaler1_mas;
	//INT8U scaler_done=0 ;

	if (fd<0 || !output_width || !output_height || !output_addr) {
		return -1;
	}
	

	// allocate buffer to decode
	file_buff_addr = (INT32U)gp_malloc_align(JPEG_DEC_PIECE_SIZE, 16);
	if (file_buff_addr==0) {
	    DBG_PRINT("video_decode_jpeg_as_rgb565_by_piece failed to allocate memory\r\n");
	    return -1;
	}
	
	// read file header or entire file
	if (file_size<=JPEG_DEC_PIECE_SIZE)
	    size_to_read = file_size;
	else
	    size_to_read = JPEG_DEC_PIECE_SIZE ;
    
	if (read(fd, file_buff_addr, size_to_read) <= 0) {
		gp_free((void *) file_buff_addr);
		DBG_PRINT("video_decode_jpeg_as_rgb565_by_piece read jpeg file fail.\r\n");
		return STATUS_FAIL;
	}	
	
	data_addr = (INT8U *)file_buff_addr ;
	// Find JPEG header(0xFFD8)
	for (i=0; i<64; i++) {
		if (*data_addr == 0xFF) {
			break;
		} else {
			data_addr++;
		}
	}
	
	// Parse JPEG data header
	jpeg_decode_init();
	if (jpeg_decode_parse_header((INT8U *) data_addr, size_to_read) != JPEG_PARSE_OK) {
		return -1;
	}
	img_valid_width = jpeg_decode_image_width_get();
	img_valid_height = jpeg_decode_image_height_get();
	img_extend_width = jpeg_decode_image_extended_width_get();
	img_extend_height = jpeg_decode_image_extended_height_get();
	img_src_valid_width = img_valid_width ;
	img_src_valid_height = img_valid_height ;
		
	// Setup JPEG and Scaler engine for decoding
	scaler_init(SCALER_1);
   	//scaler_fifo_line_set(SCALER_1, C_SCALER_CTRL_IN_FIFO_DISABLE);		// Default disable 
	scaler_input_format_set(SCALER_1, C_SCALER_CTRL_IN_YUYV);
	scaler_output_format_set(SCALER_1, C_SCALER_CTRL_OUT_YUYV);
	scaler_out_of_boundary_mode_set(SCALER_1, 1);	
	scaler_out_of_boundary_color_set(SCALER_1, 0x008080);			// Black
	
	pad_line_num = 0;
	auto_config = 0;
	
	//---------------------------------------------------------------------------
	// to QVGA (TFT)
	//
	if (output_width==320 && output_height==240) {

	    if (img_valid_width>2000) {
			jpeg_decode_scale_down_set(ENUM_JPG_DIV4);
			img_valid_width >>= 2;
			img_valid_height >>= 2;
			img_extend_width >>= 2;
			img_extend_height >>= 2;
        } else if ((img_valid_width>=(output_width<<2)) && (img_valid_height>=(output_height<<2))) {
			jpeg_decode_scale_down_set(ENUM_JPG_DIV4);
			img_valid_width >>= 2;
			img_valid_height >>= 2;
			img_extend_width >>= 2;
			img_extend_height >>= 2;
			
		} else if ((img_valid_width>=(output_width<<1)) && (img_valid_height>=(output_height<<1))) {
			jpeg_decode_scale_down_set(ENUM_JPG_DIV2);
			img_valid_width >>= 1;
			img_valid_height >>= 1;
			img_extend_width >>= 1;
			img_extend_height >>= 1;
		}

		fifo_line = 0 ;
		if (img_valid_width<=1718) { // FIFO=32 lines
			fifo_line = 32 ;
		} else if (img_valid_width<=3436) { // FIFO=16 lines
			fifo_line = 16 ;
		} 

		if ((jpg_temp_out_buf=(INT32U)gp_malloc_align(img_extend_width*fifo_line*2*2, 32))==0) {
		    DBG_PRINT("video_decode_jpeg_as_rgb565_by_piece failed to allocate memory\r\n");
		    return -1 ;
        }
		scaler_input_fifo_line_set(SCALER_1,fifo_line) ;
		scaler_input_A_addr_set(SCALER_1,jpg_temp_out_buf, 0, 0) ;
		scaler_input_B_addr_set(SCALER_1,jpg_temp_out_buf+img_extend_width*fifo_line*2, 0, 0) ;
		scaler_input_pixels_set(SCALER_1, img_extend_width, img_extend_height);
		scaler_input_visible_pixels_set(SCALER_1, img_valid_width, img_valid_height);	
        scaler_input_offset_set(SCALER_1,0x8000, 0x8000);
        scaler_output_fifo_line_set(SCALER_1,0) ;  
		scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/320, (img_valid_height<<16)/240, 320, 240);
    	scaler_output_addr_set(SCALER_1, output_addr, NULL, NULL);		 
	}	
	
	// set scatop_1 path
	gp_memset((INT8S *)&scaler1_mas,0,sizeof(SCALER_MAS));
	scaler1_mas.mas_0 = MAS_EN_READ|MAS_EN_WRITE;
	scaler_mas_set(SCALER_1,&scaler1_mas);									
						
	// Start JPEG decode
	if (jpeg_decode_output_set(jpg_temp_out_buf, 0x0, 0x0, fifo_line)) { 
		gp_free((void *) jpg_temp_out_buf);
		return -1;
	}
		

    scaler_status = jpeg_scaler_fifo_mode_flow_by_piece(fd, file_size, file_buff_addr);					
	if (jpg_temp_out_buf)
		gp_free((void *)jpg_temp_out_buf);
	if (file_buff_addr)											
	    gp_free((void *)file_buff_addr);

	// STOP JPEG and Scaler
	jpeg_decode_stop();
	scaler_stop(SCALER_1);
	
	if (scaler_status & C_SCALER_STATUS_DONE) { 
		return 0;
	}
	return -1;		    
}


INT32S video_decode_jpeg_as_yuyv(INT8U *data_addr, INT32U data_size, INT32U output_width, INT32U output_height, INT32U output_addr)
{
	return 0;
}

#define DUMMY_ADDR 0xF8500000

void paint_GP420_left_side_of_960x720(INT32U pBufAddr)
{
/*
	INT32U dwCnt, i ;
	INT32U dwAddr_1, dwAddr_2, dwAddr_3;

	for (i=0; i<20; i++) { // clear first line
		*((INT32U *)(pBufAddr)+i) = 0 ;	
	}

	for (dwCnt=0; dwCnt<360; dwCnt++) {
		dwAddr_1 = pBufAddr + dwCnt*3*1280 ;
		dwAddr_2 = dwAddr_1 + 1280;
		dwAddr_3 = dwAddr_2 + 1280;
		//for (i=0; i<40; i++) {   // NN
		//	*((INT32U *)(dwAddr_1)+i) = 0 ;	
		//}
		for (i=0; i<20; i++) {
			*((INT32U *)(dwAddr_2)+i) = 0x80808080 ;	
		}
		for (i=0; i<20; i++) {
			*((INT32U *)(dwAddr_3)+i) = 0 ;	
		}
	}
*/
	INT32U dwCnt, i ;
	INT32U dwAddr_1, dwAddr_2, dwAddr_3;

	for (i=0; i<40; i++) { // clear first line
		*((INT32U *)(pBufAddr)+i) = 0 ;	
	}

	for (dwCnt=0; dwCnt<360; dwCnt++) {
		dwAddr_1 = pBufAddr + dwCnt*3*1280 ;
		dwAddr_2 = dwAddr_1 + 1280;
		dwAddr_3 = dwAddr_2 + 1280;
		//for (i=0; i<40; i++) {   // NN
		//	*((INT32U *)(dwAddr_1)+i) = 0 ;	
		//}
		for (i=0; i<40; i++) {
			*((INT32U *)(dwAddr_2)+i) = 0x80808080 ;	
		}
		for (i=0; i<40; i++) {
			*((INT32U *)(dwAddr_3)+i) = 0 ;	
		}
	}

}

#ifdef HDMI_JPG_DECODE_AS_GP420
INT32S video_decode_jpeg_as_gp420(INT8U *data_addr, INT32U data_size, INT32U output_width, INT32U output_height, INT32U output_addr)
{
	INT16U img_valid_width, img_valid_height, img_extend_width, img_extend_height;
	INT32U jpg_temp_out_buf;
	INT32S scaler_status, jpeg_status;
	INT8U use_scaler, pad_line_num;
	INT8U auto_config;
	INT16U i;
	INT32U y_addr;
	INT32U fifo_line;
	//INT8U scaler_done=0 ;
	SCALER_MAS scaler1_mas;
	INT8U bRatio_4_3_to_16_9 = 0 ;
	INT32U factor_y ;

	if (!data_addr || !output_width || !output_height) {
		return -1;
	}

	// Find JPEG header(0xFFD8)
	for (i=0; i<64; i++) {
		if (*data_addr == 0xFF) {
			break;
		} else {
			data_addr++;
		}
	}		


	// Parse JPEG data header
	jpeg_decode_init();
	if (jpeg_decode_parse_header((INT8U *) data_addr, data_size) != JPEG_PARSE_OK) {
		return -1;
	}	
	img_valid_width = jpeg_decode_image_width_get();
	img_valid_height = jpeg_decode_image_height_get();
	img_extend_width = jpeg_decode_image_extended_width_get();
	img_extend_height = jpeg_decode_image_extended_height_get();
		
	// Setup JPEG and Scaler engine for decoding
	scaler_init(SCALER_1);
   	//scaler_fifo_line_set(SCALER_1, C_SCALER_CTRL_IN_FIFO_DISABLE);		// Default disable 
	scaler_input_format_set(SCALER_1, C_SCALER_CTRL_IN_YUYV);
	scaler_output_format_set(SCALER_1, C_SCALER_CTRL_OUT_YUYV);
	scaler_out_of_boundary_mode_set(SCALER_1, 1);	
	scaler_out_of_boundary_color_set(SCALER_1, 0x008080);			// Black
	
	jpg_temp_out_buf = 0;
	use_scaler = 0;
	pad_line_num = 0;
	auto_config = 0;
	if (output_width==1280 && output_height==720) { // for HDMI.
		if (img_valid_width==1920 && img_valid_height==1080 && jpeg_decode_image_yuv_mode_get()==C_JPG_CTRL_YUV420) {			
			// JPEG decode with 2/3 scale-down to 1280x270, GP420, SDRAM
			jpeg_image_size_set(1920, 1104);		// Feed JPEG more data so that it can scale-down to 720 lines
			jpeg_yuv_sampling_mode_set(C_JPG_CTRL_YUV420 | C_JPG_CTRL_GP420);		// Must be YUV420
			jpeg_decode_scale_down_set(ENUM_JPG_HV_DIV23);
			jpeg_decode_output_set(output_addr, 0x0, 0x0, 0x0);
		} else if (img_valid_width==1280 && img_valid_height==720) {
			// JPEG decode to 1280x270, GP420, SDRAM
			jpeg_yuv_sampling_mode_set(jpeg_decode_image_yuv_mode_get() | C_JPG_CTRL_GP420);
			jpeg_decode_output_set(output_addr, 0x0, 0x0, 0x0);
		} else {
			// Auto 
			auto_config = 1;
		}

		// for Standard image resolution
		if (!auto_config) {  
			if (jpg_temp_out_buf && jpeg_decode_output_set(jpg_temp_out_buf, 0x0, 0x0, 0x0)) {
				gp_free((void *) jpg_temp_out_buf);
					
				return -1;
			}
			
			// Start JPEG
			jpeg_decode_once_start((INT8U *) jpeg_decode_image_vlc_addr_get(), data_size<<1);
			// When JPEG is decoding image, we can fill the black padding lines at the same time
			if (pad_line_num) {
				INT32U *pad_ptr;
				INT32U cnt;
				
				pad_ptr = (INT32U *) output_addr;
				cnt = output_width * pad_line_num >> 1;
				while (cnt--) {
					*pad_ptr++ = 0;
				}
			}
			jpeg_status = jpeg_decode_status_query(1);
			
			// Start Scaler
			if (jpeg_status & C_JPG_STATUS_DECODE_DONE) {  // jpeg decoded done

				if (use_scaler) { // useless here
					scaler_start(SCALER_1);
					scaler_status = scaler_wait_idle(SCALER_1);
				} else {
					scaler_status = C_SCALER_STATUS_DONE;
				}
			} else {
				scaler_status = C_SCALER_STATUS_STOP;
			}
		}
		
	} else {
		// Auto 
		auto_config = 1;
	}

	if (auto_config) {


		if (output_width==1280 && output_height==720) {
			if (img_valid_width==1440 && img_valid_height==1080 ||
				img_valid_width==640 && img_valid_height==480) // ||
				//img_valid_width==848 && img_valid_height==480 )
				bRatio_4_3_to_16_9 = 1 ;
		}

		if (img_valid_width>=2000) {
			jpeg_decode_scale_down_set(ENUM_JPG_DIV4);
			img_valid_width >>= 2;
			img_valid_height >>= 2;
			img_extend_width >>= 2;
			img_extend_height >>= 2;
		} else if (img_valid_width>1280) {

			jpeg_decode_scale_down_set(ENUM_JPG_DIV2);
			img_valid_width >>= 1;
			img_valid_height >>= 1;
			img_extend_width >>= 1;
			img_extend_height >>= 1;
		}

	
		fifo_line = 0 ;
		if (img_valid_width<=1718) { // FIFO=32 lines
			fifo_line = 32 ;
		} else if (img_valid_width<=3436) { // FIFO=16 lines
			fifo_line = 16 ;
		} 

		// TBD: use JPEG output iRAM FIFO mode instead of SDRAM frame mode here
		jpg_temp_out_buf = (INT32U) gp_malloc_align(img_extend_width*fifo_line*4, 32);
		//jpg_temp_out_buf = 0xF8000000 ;
		//jpg_temp_out_buf = 0x007A0000 ;
		//jpg_temp_out_buf = 0xF8019000 ;
		//jpg_temp_out_buf = 0x00700000 ;
		//jpg_temp_out_buf = 0xF8021700 ;

		scaler_input_fifo_line_set(SCALER_1,fifo_line) ;
		scaler_input_A_addr_set(SCALER_1,jpg_temp_out_buf, 0, 0) ;
		scaler_input_B_addr_set(SCALER_1,jpg_temp_out_buf+img_extend_width*fifo_line*2, 0, 0) ;
		scaler_input_pixels_set(SCALER_1, img_extend_width, img_extend_height);
		scaler_input_visible_pixels_set(SCALER_1, img_valid_width, img_valid_height);	
		
		scaler_output_fifo_line_set(SCALER_1,0) ;
		
		if (bRatio_4_3_to_16_9) {
			factor_y = (img_valid_height<<16)/(output_height-(pad_line_num<<1)) ;
			
			scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/960, 
											factor_y, output_width, output_height-pad_line_num);	
		} else {
			scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/output_width, (img_valid_height<<16)/(output_height-(pad_line_num<<1)), 
											output_width, output_height-pad_line_num);	
		}
		scaler_output_addr_set(SCALER_1, DUMMY_ADDR, NULL, NULL);
		
		// set scatop_1 path
		gp_memset((INT8S *)&scaler1_mas,0,sizeof(SCALER_MAS));
		scaler1_mas.mas_0 = MAS_EN_READ;
		scaler1_mas.mas_3 = MAS_EN_WRITE;	
		scaler_mas_set(SCALER_1,&scaler1_mas);

		wrap_addr_set(WRAP_CSI2SCA, DUMMY_ADDR);
		wrap_path_set(WRAP_CSI2SCA,0,1);
		wrap_filter_flush(WRAP_CSI2SCA);
		
		// set Conv422: Setup 422_to_420 so that we can output GP420 to HDMI
		y_addr = output_addr+(output_width*pad_line_num<<1) ;
		conv422_init(); 
		conv422_input_pixels_set(output_width,output_height);
		conv422_fifo_line_set(output_height);
		if (bRatio_4_3_to_16_9)
			conv422_output_A_addr_set(y_addr+160);
			//conv422_output_A_addr_set(y_addr+80);
		else
			conv422_output_A_addr_set(y_addr);
		//conv422_output_B_addr_set(y_addr);
		conv422_output_format_set(FORMAT_420);
		conv422_mode_set(FIFO_MODE);
		conv422_clear_set();			

		// Start JPEG
		if (jpeg_decode_output_set(jpg_temp_out_buf, 0x0, 0x0, fifo_line)) {
			gp_free((void *) jpg_temp_out_buf);
			return -1;
		}			
		scaler_status = jpeg_scaler_fifo_mode_flow((INT8U *)jpeg_decode_image_vlc_addr_get(), data_size<<1);					
	}

	if (jpg_temp_out_buf) {
		gp_free((void *) jpg_temp_out_buf);
	}
	
	// STOP JPEG and Scaler
	jpeg_decode_stop();
	scaler_stop(SCALER_1);
	
	// paint black for left side
	if (bRatio_4_3_to_16_9) {
		paint_GP420_left_side_of_960x720(output_addr);
	}
	
	if (scaler_status & C_SCALER_STATUS_DONE) { 
		return 0;
	}
	return -1;
}


// only decode JPEG, not include AVI
INT32S video_decode_jpeg_as_gp420_by_piece(INT16S fd, INT32U file_size, INT32U output_width, INT32U output_height, INT32U output_addr)
{
	INT16U img_valid_width, img_valid_height, img_extend_width, img_extend_height;
	INT32U jpg_temp_out_buf;
	INT32S scaler_status;
	INT8U use_scaler, pad_line_num;
	
	INT16U i;
	INT32U y_addr;
	INT32U fifo_line;
	//INT8U scaler_done=0 ;
	SCALER_MAS scaler1_mas;
	INT8U bRatio_4_3_to_16_9 = 0 ;
	INT32U factor_y ;

	INT32U size_to_read;
	INT8U *data_addr;
	INT32U file_buff_addr ;
	//INT16U img_src_extend_width, img_src_extend_height ;


DBG_PRINT("video_decode_jpeg_as_gp420_by_piece()=============\r\n");

	if (!output_width || !output_height) {
		return -1;
	}

	// Find JPEG header(0xFFD8)
	for (i=0; i<64; i++) {
		if (*data_addr == 0xFF) {
			break;
		} else {
			data_addr++;
		}
	}		


	// allocate buffer to decode
	file_buff_addr = (INT32U)gp_malloc_align(JPEG_DEC_PIECE_SIZE, 16);
	if (file_buff_addr==0) {
	    DBG_PRINT("video_decode_jpeg_as_gp420_by_piece failed to allocate memory\r\n");
	    return -1;
	}

	// read file header or entire file
	if (file_size<=JPEG_DEC_PIECE_SIZE)
	    size_to_read = file_size;
	else
	    size_to_read = JPEG_DEC_PIECE_SIZE ;
    
	if (read(fd, file_buff_addr, size_to_read) <= 0) {
		gp_free((void *) file_buff_addr);
		DBG_PRINT("video_decode_jpeg_as_rgb565_by_piece read jpeg file fail.\r\n");
		return STATUS_FAIL;
	}
	
	data_addr = (INT8U *)file_buff_addr ;


	// Parse JPEG data header
	jpeg_decode_init();
	if (jpeg_decode_parse_header((INT8U *) data_addr, size_to_read) != JPEG_PARSE_OK) {
		return -1;
	}	
	img_valid_width = jpeg_decode_image_width_get();
	img_valid_height = jpeg_decode_image_height_get();
	img_extend_width = jpeg_decode_image_extended_width_get();
	img_extend_height = jpeg_decode_image_extended_height_get();
		
	// Setup JPEG and Scaler engine for decoding
	scaler_init(SCALER_1);
   	//scaler_fifo_line_set(SCALER_1, C_SCALER_CTRL_IN_FIFO_DISABLE);		// Default disable 
	scaler_input_format_set(SCALER_1, C_SCALER_CTRL_IN_YUYV);
	scaler_output_format_set(SCALER_1, C_SCALER_CTRL_OUT_YUYV);
	scaler_out_of_boundary_mode_set(SCALER_1, 1);	
	scaler_out_of_boundary_color_set(SCALER_1, 0x008080);			// Black
	
	jpg_temp_out_buf = 0;
	use_scaler = 0;
	pad_line_num = 0;
	
	
	if (!(img_valid_width==1920 && img_valid_height==1080))
		bRatio_4_3_to_16_9 = 1 ;



	if (img_valid_width>=2000) {
		jpeg_decode_scale_down_set(ENUM_JPG_DIV4);
		img_valid_width >>= 2;
		img_valid_height >>= 2;
		img_extend_width >>= 2;
		img_extend_height >>= 2;
	} else if (img_valid_width>1280) {

		jpeg_decode_scale_down_set(ENUM_JPG_DIV2);
		img_valid_width >>= 1;
		img_valid_height >>= 1;
		img_extend_width >>= 1;
		img_extend_height >>= 1;
	}


	fifo_line = 0 ;
	if (img_valid_width<=1718) { // FIFO=32 lines
		fifo_line = 32 ;
	} else if (img_valid_width<=3436) { // FIFO=16 lines
		fifo_line = 16 ;
	} 

	// TBD: use JPEG output iRAM FIFO mode instead of SDRAM frame mode here
	jpg_temp_out_buf = (INT32U) gp_malloc_align(img_extend_width*fifo_line*4, 32);


	scaler_input_fifo_line_set(SCALER_1,fifo_line) ;
	scaler_input_A_addr_set(SCALER_1,jpg_temp_out_buf, 0, 0) ;
	scaler_input_B_addr_set(SCALER_1,jpg_temp_out_buf+img_extend_width*fifo_line*2, 0, 0) ;
	scaler_input_pixels_set(SCALER_1, img_extend_width, img_extend_height);
	scaler_input_visible_pixels_set(SCALER_1, img_valid_width, img_valid_height);	
	
	scaler_output_fifo_line_set(SCALER_1,0) ;
	
	if (bRatio_4_3_to_16_9) {
		factor_y = (img_valid_height<<16)/(output_height-(pad_line_num<<1)) ;
		scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/960, 
										factor_y, output_width, output_height-pad_line_num);	
	} else {
		scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/output_width, (img_valid_height<<16)/(output_height-(pad_line_num<<1)), 
										output_width, output_height-pad_line_num);	
	}
	scaler_output_addr_set(SCALER_1, DUMMY_ADDR, NULL, NULL);
	
	// set scatop_1 path
	gp_memset((INT8S *)&scaler1_mas,0,sizeof(SCALER_MAS));
	scaler1_mas.mas_0 = MAS_EN_READ;
	scaler1_mas.mas_3 = MAS_EN_WRITE;	
	scaler_mas_set(SCALER_1,&scaler1_mas);

	wrap_addr_set(WRAP_CSI2SCA, DUMMY_ADDR);
	wrap_path_set(WRAP_CSI2SCA,0,1);
	wrap_filter_flush(WRAP_CSI2SCA);
	
	// set Conv422: Setup 422_to_420 so that we can output GP420 to HDMI
	y_addr = output_addr+(output_width*pad_line_num<<1) ;
	conv422_init(); 
	conv422_input_pixels_set(output_width,output_height);
	conv422_fifo_line_set(output_height);
	if (bRatio_4_3_to_16_9)
		conv422_output_A_addr_set(y_addr+160);
		//conv422_output_A_addr_set(y_addr+80);
	else
		conv422_output_A_addr_set(y_addr);
	//conv422_output_B_addr_set(y_addr);
	conv422_output_format_set(FORMAT_420);
	conv422_mode_set(FIFO_MODE);
	conv422_clear_set();			

	// Start JPEG
	if (jpeg_decode_output_set(jpg_temp_out_buf, 0x0, 0x0, fifo_line)) {
		gp_free((void *) jpg_temp_out_buf);
		return -1;
	}			
	scaler_status = jpeg_scaler_fifo_mode_flow_by_piece(fd, file_size, file_buff_addr);					


	if (jpg_temp_out_buf) 
		gp_free((void *) jpg_temp_out_buf);
	if (file_buff_addr)
	    gp_free((void *) file_buff_addr);
	
	// STOP JPEG and Scaler
	jpeg_decode_stop();
	scaler_stop(SCALER_1);
	
	// paint black for left side
	if (bRatio_4_3_to_16_9) {
		paint_GP420_left_side_of_960x720(output_addr);
	}
	
	if (scaler_status & C_SCALER_STATUS_DONE) { 
		return 0;
	}
	return -1;
}
#endif // #ifdef HDMI_JPG_DECODE_AS_GP420


#ifdef HDMI_JPG_DECODE_AS_YUV422
// only decode AVI, not include JPG
INT32S video_decode_jpeg_as_gp422(INT8U *data_addr, INT32U data_size, INT32U output_width, INT32U output_height, INT32U output_addr)
{
	INT16U img_valid_width, img_valid_height, img_extend_width, img_extend_height;
	INT32U jpg_temp_out_buf;
	INT32S scaler_status;
	INT8U use_scaler, pad_line_num;
	INT16U i;
	INT32U y_addr;
	INT32U fifo_line;
	SCALER_MAS scaler1_mas;
	INT8U bRatio_4_3_to_16_9 = 0 ;
	INT32U factor_y ;

	if (!data_addr || !output_width || !output_height) {
		return -1;
	}

	// Find JPEG header(0xFFD8)
	for (i=0; i<64; i++) {
		if (*data_addr == 0xFF) {
			break;
		} else {
			data_addr++;
		}
	}		

	// Parse JPEG data header
	jpeg_decode_init();
	if (jpeg_decode_parse_header((INT8U *) data_addr, data_size) != JPEG_PARSE_OK) {
		return -1;
	}	
	img_valid_width = jpeg_decode_image_width_get();
	img_valid_height = jpeg_decode_image_height_get();
	img_extend_width = jpeg_decode_image_extended_width_get();
	img_extend_height = jpeg_decode_image_extended_height_get();
	
/*	
if (gpio_read_io(IO_A14)==1)
	gpio_write_io(IO_A14, 0);	//pull Low
else
	gpio_write_io(IO_A14, 1);	//pull high
*/	
	
		
	// Setup JPEG and Scaler engine for decoding
	scaler_init(SCALER_1);
   	//scaler_fifo_line_set(SCALER_1, C_SCALER_CTRL_IN_FIFO_DISABLE);		// Default disable 
	scaler_input_format_set(SCALER_1, C_SCALER_CTRL_IN_YUYV);
	scaler_output_format_set(SCALER_1, C_SCALER_CTRL_OUT_YUYV);
	scaler_out_of_boundary_mode_set(SCALER_1, 1);	
	scaler_out_of_boundary_color_set(SCALER_1, 0x008080);			// Black
	
	jpg_temp_out_buf = 0;
	use_scaler = 0;
	pad_line_num = 0;

	if (output_width==1280 && output_height==720) {
		if (img_valid_width==1440 && img_valid_height==1080 ||
			img_valid_width==640 && img_valid_height==480) 
			bRatio_4_3_to_16_9 = 1 ;
	}

	if (img_valid_width>=2000) {
		jpeg_decode_scale_down_set(ENUM_JPG_DIV4);
		img_valid_width >>= 2;
		img_valid_height >>= 2;
		img_extend_width >>= 2;
		img_extend_height >>= 2;
	} else if (img_valid_width>1280) {

		jpeg_decode_scale_down_set(ENUM_JPG_DIV2);
		img_valid_width >>= 1;
		img_valid_height >>= 1;
		img_extend_width >>= 1;
		img_extend_height >>= 1;
	}


	fifo_line = 0 ;
	if (img_valid_width<=1718) { // FIFO=32 lines
		fifo_line = 32 ;
	} else if (img_valid_width<=3436) { // FIFO=16 lines
		fifo_line = 16 ;
	} 

	jpg_temp_out_buf = 0xF8008700 ;  // buffer using IRAM

	scaler_input_fifo_line_set(SCALER_1,fifo_line) ;
	scaler_input_A_addr_set(SCALER_1,jpg_temp_out_buf, 0, 0) ;
	scaler_input_B_addr_set(SCALER_1,jpg_temp_out_buf+img_extend_width*fifo_line*2, 0, 0) ;
	//scaler_input_B_addr_set(SCALER_1,0xF8021700, 0, 0) ;
	scaler_input_pixels_set(SCALER_1, img_extend_width, img_extend_height);
	scaler_input_visible_pixels_set(SCALER_1, img_valid_width, img_valid_height);	
	
	scaler_output_fifo_line_set(SCALER_1,0) ;
	
	if (bRatio_4_3_to_16_9) {
		factor_y = (img_valid_height<<16)/(output_height-(pad_line_num<<1)) ;
		scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/960, 
										factor_y, output_width, output_height-pad_line_num);	
        y_addr = output_addr+(output_width*pad_line_num<<1) + 320 ;	
        for (i=0;i<80;i++) {
            *((INT32U*)output_addr+i)=0x00800080;
        }
	} else {
		scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/output_width, (img_valid_height<<16)/(output_height-(pad_line_num<<1)), 
										output_width, output_height-pad_line_num);	
        y_addr = output_addr+(output_width*pad_line_num<<1) ;											
	}
	scaler_output_addr_set(SCALER_1, y_addr, NULL, NULL);
	
	// set scatop_1 path
	gp_memset((INT8S *)&scaler1_mas,0,sizeof(SCALER_MAS));
	scaler1_mas.mas_0 = MAS_EN_READ;
	scaler1_mas.mas_3 = MAS_EN_WRITE;	
	scaler_mas_set(SCALER_1,&scaler1_mas);

	wrap_addr_set(WRAP_CSI2SCA, y_addr);
	wrap_path_set(WRAP_CSI2SCA,0,1);
	wrap_filter_flush(WRAP_CSI2SCA);

	
    (*((volatile INT32U *) 0xC0190018)) = 0x300 ; // Conv422 reset & bypass
    (*((volatile INT32U *) 0xC0180000)) &= ~(0x800) ; // wrapper set to 16-bit burst
    

	// Start JPEG
	if (jpeg_decode_output_set(jpg_temp_out_buf, 0x0, 0x0, fifo_line)) {
		gp_free((void *) jpg_temp_out_buf);
		return -1;
	}			
	scaler_status = jpeg_scaler_fifo_mode_flow((INT8U *)jpeg_decode_image_vlc_addr_get(), data_size<<2);					

	// STOP JPEG and Scaler
	jpeg_decode_stop();
	scaler_stop(SCALER_1);
	
	if (scaler_status & C_SCALER_STATUS_DONE) { 
		return 0;
	}
	return -1;
}


// only decode JPEG, not include AVI
INT32S video_decode_jpeg_as_gp422_by_piece(INT16S fd, INT32U file_size, INT32U output_width, INT32U output_height, INT32U output_addr)
{
	INT16U img_valid_width, img_valid_height, img_extend_width, img_extend_height, i;
	INT32U jpg_temp_out_buf;
	INT32S scaler_status;
	INT8U  use_scaler, pad_line_num;
	
	INT32U y_addr;
	INT32U fifo_line;
	SCALER_MAS scaler1_mas;
	INT8U  bRatio_4_3_to_16_9 = 0 ;
	INT32U factor_y ;
	INT32U size_to_read;
	INT8U  *data_addr;
	INT32U file_buff_addr ;

DBG_PRINT("video_decode_jpeg_as_gp422_by_piece()=============\r\n");

	if (!output_width || !output_height) {
		return -1;
	}

	// allocate buffer to decode
	file_buff_addr = (INT32U)gp_malloc_align(JPEG_DEC_PIECE_SIZE, 16);
	if (file_buff_addr==0) {
	    DBG_PRINT("video_decode_jpeg_as_gp422_by_piece() failed to allocate memory\r\n");
	    return -1;
	}

	// read file header or entire file
	if (file_size<=JPEG_DEC_PIECE_SIZE)
	    size_to_read = file_size;
	else
	    size_to_read = JPEG_DEC_PIECE_SIZE ;
    
	if (read(fd, file_buff_addr, size_to_read) <= 0) {
		gp_free((void *) file_buff_addr);
		DBG_PRINT("video_decode_jpeg_as_gp422_by_piece() read jpeg file fail.\r\n");
		return STATUS_FAIL;
	}
	data_addr = (INT8U *)file_buff_addr ;

	// Parse JPEG data header
	jpeg_decode_init();
	if (jpeg_decode_parse_header((INT8U *) data_addr, size_to_read) != JPEG_PARSE_OK) {
		return -1;
	}	
	img_valid_width = jpeg_decode_image_width_get();
	img_valid_height = jpeg_decode_image_height_get();
	img_extend_width = jpeg_decode_image_extended_width_get();
	img_extend_height = jpeg_decode_image_extended_height_get();
		
	// Setup JPEG and Scaler engine for decoding
	scaler_init(SCALER_1);
   	//scaler_fifo_line_set(SCALER_1, C_SCALER_CTRL_IN_FIFO_DISABLE);		// Default disable 
	scaler_input_format_set(SCALER_1, C_SCALER_CTRL_IN_YUYV);
	scaler_output_format_set(SCALER_1, C_SCALER_CTRL_OUT_YUYV);
	scaler_out_of_boundary_mode_set(SCALER_1, 1);	
	scaler_out_of_boundary_color_set(SCALER_1, 0x008080);			// Black
	
	jpg_temp_out_buf = 0;
	use_scaler = 0;
	pad_line_num = 0;
	
	if (!(img_valid_width==1920 && img_valid_height==1080))
		bRatio_4_3_to_16_9 = 1 ;

	if (img_valid_width>=2000) {
		jpeg_decode_scale_down_set(ENUM_JPG_DIV4);
		img_valid_width >>= 2;
		img_valid_height >>= 2;
		img_extend_width >>= 2;
		img_extend_height >>= 2;
	} else if (img_valid_width>1280) {
		jpeg_decode_scale_down_set(ENUM_JPG_DIV2);
		img_valid_width >>= 1;
		img_valid_height >>= 1;
		img_extend_width >>= 1;
		img_extend_height >>= 1;
	}

	fifo_line = 0 ;
	if (img_valid_width<=1718) { // FIFO=32 lines
		fifo_line = 32 ;
	} else if (img_valid_width<=3436) { // FIFO=16 lines
		fifo_line = 16 ;
	} 

    jpg_temp_out_buf = 0xF8008700 ;

	scaler_input_fifo_line_set(SCALER_1,fifo_line) ;
	scaler_input_A_addr_set(SCALER_1,jpg_temp_out_buf, 0, 0) ;
	scaler_input_B_addr_set(SCALER_1,jpg_temp_out_buf+img_extend_width*fifo_line*2, 0, 0) ;
	scaler_input_pixels_set(SCALER_1, img_extend_width, img_extend_height);
	scaler_input_visible_pixels_set(SCALER_1, img_valid_width, img_valid_height);	
	scaler_output_fifo_line_set(SCALER_1,0) ;
	
	if (bRatio_4_3_to_16_9) {
		factor_y = (img_valid_height<<16)/(output_height-(pad_line_num<<1)) ;
		scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/960, 
										factor_y, output_width, output_height-pad_line_num);	
	    y_addr = output_addr+(output_width*pad_line_num<<1)+320 ;
        for (i=0;i<80;i++) 
            *((INT32U*)output_addr+i)=0x00800080;
	} else {
		scaler_output_pixels_set(SCALER_1, (img_valid_width<<16)/output_width, (img_valid_height<<16)/(output_height-(pad_line_num<<1)), 
										output_width, output_height-pad_line_num);	
		y_addr = output_addr+(output_width*pad_line_num<<1); 
	}
	
	scaler_output_addr_set(SCALER_1, y_addr, NULL, NULL);
	
	// set scatop_1 path
	gp_memset((INT8S *)&scaler1_mas,0,sizeof(SCALER_MAS));
	scaler1_mas.mas_0 = MAS_EN_READ|MAS_EN_WRITE;
	scaler_mas_set(SCALER_1,&scaler1_mas);

	wrap_addr_set(WRAP_CSI2SCA, y_addr);
	wrap_path_set(WRAP_CSI2SCA,0,1);
	wrap_filter_flush(WRAP_CSI2SCA);

    (*((volatile INT32U *) 0xC0190018)) = 0x300 ; // Conv422 reset & set to bypass
    (*((volatile INT32U *) 0xC0180000)) &= ~(0x800) ; // wrapper set to 16-bit burst
    
	// Start JPEG
	if (jpeg_decode_output_set(jpg_temp_out_buf, 0x0, 0x0, fifo_line)) {
		gp_free((void *) jpg_temp_out_buf);
		return -1;
	}			
	scaler_status = jpeg_scaler_fifo_mode_flow_by_piece(fd, file_size, file_buff_addr);					

	if (file_buff_addr)
	    gp_free((void *) file_buff_addr);
	
	// STOP JPEG and Scaler
	jpeg_decode_stop();
	scaler_stop(SCALER_1);
	
	if (scaler_status & C_SCALER_STATUS_DONE) { 
		return 0;
	}
	return -1;
}

#endif  // HDMI_JPG_DECODE_AS_YUV422

#if DUAL_STREAM_FUNC_ENABLE
void Disp_Mjpeg_Init(void)
{
	INT32U buffer_addrs;
	INT32U i;

	buffer_addrs = (INT32U)gp_malloc_align((BROWSE_DISP_JPEG_CNT*BROWSE_DISP_JPEG_MAX_SIZE), 32);
	if(buffer_addrs == NULL)
	{
		DBG_PRINT("Browse_Disp_Jpeg_Buf is Null!\r\n");
	}

	for(i=0; i<BROWSE_DISP_JPEG_CNT; i++)
	{
		Browse_Disp_Jpeg_Buf[i].Jpeg_Buf = buffer_addrs+(i*BROWSE_DISP_JPEG_MAX_SIZE);
		Browse_Disp_Jpeg_Buf[i].Jpeg_Buf_Is_Used = 0;
	}	
}

void Disp_Mjpeg_Exit(void)
{
	if(Browse_Disp_Jpeg_Buf[0].Jpeg_Buf != 0)
	{
		gp_free((void *)Browse_Disp_Jpeg_Buf[0].Jpeg_Buf);
		Browse_Disp_Jpeg_Buf[0].Jpeg_Buf = 0;
	}
}

void Disp_Mjpeg_Buf_Return(INT32U bufIdx)
{
	sw_wifi_jpeg_lock();
	Browse_Disp_Jpeg_Buf[bufIdx].Jpeg_Buf_Is_Used = 0;
	sw_wifi_jpeg_unlock();
}

void Disp_Mjpeg_Buf_Clear(void)
{
	INT8U i;
	for(i=0; i<BROWSE_DISP_JPEG_CNT; i++)
	{
		Browse_Disp_Jpeg_Buf[i].Jpeg_Buf_Is_Used = 0;
	}
}

void Disp_MJpeg_To_Wifi(INT32U dispAddr)
{
	INT8U i;
	INT32U RetVlcSize;
	INT32S  nRet;

	for(i=0; i<BROWSE_DISP_JPEG_CNT; i++)
	{
		if(Browse_Disp_Jpeg_Buf[i].Jpeg_Buf_Is_Used == 0)
		{
			break;
		}
	}
	
	if(i == BROWSE_DISP_JPEG_CNT)
	{
		return;
	}

	if(Encode_Disp_Buf_To_Jpeg(dispAddr,Browse_Disp_Jpeg_Buf[i].Jpeg_Buf,WIFI_JPEG_WIDTH,WIFI_JPEG_HEIGHT,BROWSE_DISP_JPEG_MAX_SIZE,&RetVlcSize) == STATUS_OK)
	{
		Browse_Disp_Jpeg_Buf[i].Jpeg_Buf_Is_Used = 1;
		Browse_Disp_Jpeg_Buf[i].mjpegWriteData.msg_id = MJPEG_SEND_EVENT;
		Browse_Disp_Jpeg_Buf[i].mjpegWriteData.mjpeg_addr = Browse_Disp_Jpeg_Buf[i].Jpeg_Buf;
		Browse_Disp_Jpeg_Buf[i].mjpegWriteData.mjpeg_addr_idx = i;
		Browse_Disp_Jpeg_Buf[i].mjpegWriteData.mjpeg_size = RetVlcSize;
		Browse_Disp_Jpeg_Buf[i].mjpegWriteData.running_app = STATE_BROWSE;
		
		nRet = mjpeg_send_picture(&(Browse_Disp_Jpeg_Buf[i].mjpegWriteData));
		if(nRet) // WIFI Busy
		{
			Disp_Mjpeg_Buf_Return(i);
		}
	}
}
#endif


