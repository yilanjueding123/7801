#include "avi_encoder_app.h"
#include "video_codec_callback.h"
#include "ap_state_config.h"

#include "my_avi_encoder_state.h"
#include "drv_l1_adc.h"
#include "state_wifi.h"

/* global varaible */
AviEncPara_t AviEncPara;
AviEncPara_t *pAviEncPara = &AviEncPara;
AviEncAudPara_t AviEncAudPara;
AviEncAudPara_t *pAviEncAudPara = &AviEncAudPara;
AviEncPacker_t AviEncPacker0;
AviEncPacker_t *pAviEncPacker0 = &AviEncPacker0;

GP_AVI_AVISTREAMHEADER	avi_aud_stream_header;
GP_AVI_AVISTREAMHEADER	avi_vid_stream_header;
GP_AVI_BITMAPINFO		avi_bitmap_info;
GP_AVI_PCMWAVEFORMAT	avi_wave_info;

static DMA_STRUCT g_avi_adc_dma_dbf;
static INT8U g_pcm_index;

void  (*pfn_avi_encode_put_data)(void *WorkMem, AVIPACKER_FRAME_INFO* pFrameInfo);

/****************************************************************************/
my_AviEncVidPara_t my_AviEncVidPara;
my_AviEncVidPara_t *my_pAviEncVidPara = &my_AviEncVidPara;

/****************************************************************************/
/*
 *	AviPacker_mem_alloc
 */
static INT32S AviPacker_mem_alloc(AviEncPacker_t *pAviEncPacker)
{
	INT32S nRet;

	pAviEncPacker->index_buffer_size = IndexBuffer_Size;
	if(!pAviEncPacker->index_write_buffer)
	{
		pAviEncPacker->index_write_buffer = (INT32U *)gp_malloc_align(IndexBuffer_Size, 32);
	}
	
	if(!pAviEncPacker->index_write_buffer)
	{	
		RETURN(STATUS_FAIL);
	}
	
	pAviEncPacker->avi_workmem = (void *)gp_malloc_align(AviPackerV3_GetWorkMemSize(), 32);
	if(!pAviEncPacker->avi_workmem )
	{
		RETURN(STATUS_FAIL);
	}

	gp_memset((INT8S*)pAviEncPacker->avi_workmem, 0x00, AviPackerV3_GetWorkMemSize());
	gp_memset((INT8S*)pAviEncPacker->index_write_buffer,0,pAviEncPacker->index_buffer_size);

	nRet = STATUS_OK;
Return:
	return nRet;
} 

/****************************************************************************/
/*
 *	my_avi_encode_init
 */
void my_avi_encode_init(void)
{
	// Init video task variables
	my_pAviEncVidPara = &my_AviEncVidPara;	
    gp_memset((INT8S *)my_pAviEncVidPara, 0, sizeof(my_AviEncVidPara_t));

	// Init audio task variabls
	pAviEncAudPara = &AviEncAudPara;
	gp_memset((INT8S *)pAviEncAudPara, 0, sizeof(AviEncAudPara_t));

    pAviEncPara = &AviEncPara;
    gp_memset((INT8S *)pAviEncPara, 0, sizeof(AviEncPara_t));
    pAviEncPacker0 = &AviEncPacker0;
    gp_memset((INT8S *)pAviEncPacker0, 0, sizeof(AviEncPacker_t));   
    
	pAviEncPacker0->file_handle = -1;
	pAviEncPacker0->index_handle = -1;
}

/****************************************************************************/
/*
 *	avi_encode_set_curworkmem
 */
void avi_encode_set_curworkmem(void *pAviEncPacker)
{
	 pAviEncPara->AviPackerCur = pAviEncPacker;
}

/****************************************************************************/
/*
 *	avi_encode_set_file_handle_and_caculate_free_size
 */
INT32S avi_encode_set_file_handle_and_caculate_free_size(AviEncPacker_t *pAviEncPacker, INT16S FileHandle, INT16S txt_FileHandle)
{
	struct stat_t statbuf;
	
	if(FileHandle < 0)
	{
		return STATUS_FAIL;
	}
	
    gp_strcpy((INT8S*)pAviEncPacker->index_path, (INT8S*)"C:\\");

    if(stat("index0.tmp", &statbuf) < 0)
    {
    	gp_strcat((INT8S*)pAviEncPacker->index_path, (INT8S*)"index0.tmp");
    }
    else
    {
    	gp_strcat((INT8S*)pAviEncPacker->index_path, (INT8S*)"index1.tmp");
    }

    pAviEncPacker->txt_handle = txt_FileHandle;
    pAviEncPacker->file_handle = FileHandle;
    pAviEncPacker->index_handle = open((char*)pAviEncPacker->index_path, O_RDWR|O_CREAT|O_TRUNC);
    if(pAviEncPacker->index_handle < 0)
    {
    	return STATUS_FAIL;
    }

   	pAviEncPara->disk_free_size = vfsFreeSpace(MINI_DVR_STORAGE_TYPE) - 3145728;	//3*1024*1024
   	pAviEncPara->record_total_size = 2*32*1024 + 16; //avi header + data is 32k align + index header
   	
   	return STATUS_OK;
}

/****************************************************************************/
/*
 *	avi_enc_packer_init
 */
INT32U avi_enc_packer_init(AviEncPacker_t *pAviEncPacker)
{
	INT32S nRet;

	if(pAviEncPacker == pAviEncPacker0) {
		pAviEncPacker->task_prio = AVI_PACKER0_PRIORITY;
	} else {
		RETURN(STATUS_FAIL);
	}

	nRet = AviPackerV3_TaskCreate(	pAviEncPacker->task_prio,
									pAviEncPacker->avi_workmem,
									pAviEncPacker->index_write_buffer,
									pAviEncPacker->index_buffer_size);
	if(nRet<0) RETURN(STATUS_FAIL);
	AviPackerV3_SetErrHandler(pAviEncPacker->avi_workmem, avi_packer_err_handle);
	nRet = STATUS_OK;
Return:
	return nRet;
}

/****************************************************************************/
/*
 *	avi_enc_packer_start
 */
INT32U avi_enc_packer_start(AviEncPacker_t *pAviEncPacker)
{
	INT32S nRet; 
	INT32U bflag;

	if(pAviEncPacker == pAviEncPacker0) {
		bflag = AVI_ENCODER_STATE_PACKER0;
	} else {
		RETURN(STATUS_FAIL);
	}
	
	if((avi_encoder_state_get() & bflag) == 0)
	{
		switch(pAviEncPara->source_type)
		{
		case SOURCE_TYPE_FS:
			avi_encode_set_avi_header(pAviEncPacker);
			nRet = AviPackerV3_Open(pAviEncPacker->avi_workmem,
									pAviEncPacker->file_handle, 
									pAviEncPacker->index_handle,
									pAviEncPacker->txt_handle,
									pAviEncPacker->p_avi_vid_stream_header,
									pAviEncPacker->bitmap_info_cblen,
									pAviEncPacker->p_avi_bitmap_info,
									pAviEncPacker->p_avi_aud_stream_header,
									pAviEncPacker->wave_info_cblen,
									pAviEncPacker->p_avi_wave_info);
			pfn_avi_encode_put_data = AviPackerV3_WriteData;
			break;
		case SOURCE_TYPE_USER_DEFINE:
			pfn_avi_encode_put_data = video_encode_frame_ready;
			break;
		}
		avi_encoder_state_set(bflag);
		DEBUG_MSG(DBG_PRINT("a.AviPackerOpen[0x%x] = 0x%x\r\n", bflag, nRet));
	}
	else
	{
		RETURN(STATUS_FAIL);
	}
	nRet = STATUS_OK;
Return:	
	return nRet;
}

/****************************************************************************/
/*
 *	avi_enc_start
 */
INT32S avi_enc_start(void)
{
	INT8U  err;
	INT32S nRet, msg;
	
	nRet = STATUS_OK;

	if((avi_encode_get_status()&C_AVI_ENCODE_START) == 0)
	{
		POST_MESSAGE(my_AVIEncodeApQ, MSG_AVI_START_ENCODE, my_avi_encode_ack_m, 5000, msg, err);	
		avi_encode_set_status(C_AVI_ENCODE_START);
		DEBUG_MSG(DBG_PRINT("encode start\r\n")); 
	}
	
	// start audio 
	if(pAviEncAudPara->audio_format && ((avi_encode_get_status()&C_AVI_ENCODE_AUDIO) == 0))	
	{
		if(avi_audio_record_start() < 0) RETURN(STATUS_FAIL);
		avi_encode_set_status(C_AVI_ENCODE_AUDIO);
		DEBUG_MSG(DBG_PRINT("audio start\r\n"));
	}
	
Return:	
	return nRet;
}

/****************************************************************************/
/*
 *	vid_enc_preview_start
 */
INT32S vid_enc_preview_start(void)
{
	INT8U  err;
	INT32S nRet, msg;
	
	nRet = STATUS_OK;
	if((avi_encoder_state_get() & AVI_ENCODER_STATE_SENSOR) == 0)
	{	
		POST_MESSAGE(my_AVIEncodeApQ, MSG_AVI_START_SENSOR, my_avi_encode_ack_m, 5000, msg, err);	
		avi_encoder_state_set(AVI_ENCODER_STATE_SENSOR);
	}

Return:
	if(nRet < 0) {
		DBG_PRINT("wwj: start sensor fail...\r\n");
	}
	return nRet;
}

/****************************************************************************/
/*
 *	vid_enc_preview_stop
 */
INT32S vid_enc_preview_stop(void)
{
	INT8U  err;
	INT32S nRet, msg;
	
	nRet = STATUS_OK;

	if(avi_encoder_state_get() & AVI_ENCODER_STATE_SENSOR)
	{
		POST_MESSAGE(my_AVIEncodeApQ, MSG_AVI_STOP_SENSOR, my_avi_encode_ack_m, 5000, msg, err);	
		avi_encoder_state_clear(AVI_ENCODER_STATE_SENSOR);
	}

	// stop audio
	if(avi_encode_get_status()&C_AVI_ENCODE_AUDIO)
	{
		if(avi_audio_record_stop() < 0) RETURN(STATUS_FAIL);
		avi_encode_clear_status(C_AVI_ENCODE_AUDIO);
		avi_encode_audio_timer_stop();
		DEBUG_MSG(DBG_PRINT("audio stop\r\n"));
	}

Return:	
	return nRet;
}

/****************************************************************************/
/*
 *	avi_enc_disable_sensor_clock
 */
INT32S vid_enc_disable_sensor_clock(void)
{
	INT8U  err;
	INT32S nRet, msg;

	nRet = STATUS_OK;
	if(avi_encoder_state_get() & AVI_ENCODER_STATE_SENSOR)
	{
		POST_MESSAGE(my_AVIEncodeApQ, MSG_AVI_DISABLE_SENSOR_CLOCK, my_avi_encode_ack_m, 5000, msg, err);
		avi_encoder_state_clear(AVI_ENCODER_STATE_SENSOR);
	}
Return:	
	return nRet;
}



/****************************************************************************/
/*
 *	avi_enc_stop
 */

extern void jpeg_Q_clear(void);
extern void fifo_Q_clear(void);

INT32S avi_enc_stop(void)
{
	INT8U  err;
	INT32S nRet, msg;
	
	nRet = STATUS_OK;

	// stop avi encode
	if(avi_encode_get_status()&C_AVI_ENCODE_START)
	{
		avi_encode_clear_status(C_AVI_ENCODE_START);
		POST_MESSAGE(my_AVIEncodeApQ, MSG_AVI_STOP_ENCODE, my_avi_encode_ack_m, 5000, msg, err);
		DEBUG_MSG(DBG_PRINT("b.encode stop\r\n")); 
	}

Return:

	#if DUAL_STREAM_FUNC_ENABLE
	if(Wifi_State_Get() == WIFI_STATE_FLAG_DISCONNECT)
	#endif
	{
		jpeg_stop();
		jpeg_Q_clear();
	}

	avi_enc_stop_flush();
	fifo_Q_clear();

	// stop audio
	if(avi_encode_get_status()&C_AVI_ENCODE_AUDIO)
	{
		avi_encode_clear_status(C_AVI_ENCODE_AUDIO);
		if(avi_audio_record_stop() < 0) return (STATUS_FAIL);
		DEBUG_MSG(DBG_PRINT("a.audio stop\r\n"));
	}
	return nRet;
}

/****************************************************************************/
/*
 *	avi_enc_packer_stop
 */
INT32U avi_enc_packer_stop(AviEncPacker_t *pAviEncPacker)
{
	INT32S nRet;
	INT32U bflag;
	
	if(pAviEncPacker == pAviEncPacker0)
	{		
		bflag = AVI_ENCODER_STATE_PACKER0;
	}
	else
	{
		RETURN(STATUS_FAIL);
	}
	
	if(avi_encoder_state_get() & bflag)
	{
		switch(pAviEncPara->source_type)
        {
		case SOURCE_TYPE_FS:
			video_encode_end(pAviEncPacker->avi_workmem);
           	nRet = AviPackerV3_Close(pAviEncPacker->avi_workmem); 
           	avi_encode_close_file(pAviEncPacker);
        	break;		
        case SOURCE_TYPE_USER_DEFINE:
        	nRet = STATUS_OK;
        	break;
        } 
        
        if(nRet < 0) RETURN(STATUS_FAIL);
		avi_encoder_state_clear(bflag);
		DEBUG_MSG(DBG_PRINT("c.AviPackerClose[0x%x] = 0x%x\r\n", bflag, nRet)); 
	}
	nRet = STATUS_OK;
Return:
	return nRet;
}


/****************************************************************************/
/*
 *	avi_enc_packer_fast_stop_and_start
 */
INT32U avi_enc_packer_switch_file(AviEncPacker_t *pAviEncPacker, INT16S fd_new, INT16S fd_txt_new)
{
	INT32S nRet;
	INT32U bflag;

	if(pAviEncPacker == pAviEncPacker0)
	{		
		bflag = AVI_ENCODER_STATE_PACKER0;
	}
	else
	{
		RETURN(STATUS_FAIL);
	}
	
	if(avi_encoder_state_get() & bflag)
	{
		switch(pAviEncPara->source_type)
        {
		case SOURCE_TYPE_FS:
			video_encode_end(pAviEncPacker->avi_workmem);
           	nRet = AviPackerV3_SwitchFile(pAviEncPacker->avi_workmem,
									&pAviEncPacker->file_handle, 
									&pAviEncPacker->index_handle,
									&pAviEncPacker->txt_handle,
									(CHAR *)pAviEncPacker->index_path,
									pAviEncPacker->p_avi_vid_stream_header,
									pAviEncPacker->bitmap_info_cblen,
									pAviEncPacker->p_avi_bitmap_info,
									pAviEncPacker->p_avi_aud_stream_header,
									pAviEncPacker->wave_info_cblen,
									pAviEncPacker->p_avi_wave_info,
									fd_new,
									fd_txt_new);			
        	break;
        case SOURCE_TYPE_USER_DEFINE:
        	nRet = STATUS_OK;
        	break;
        } 
        
        if(nRet < 0) RETURN(STATUS_FAIL);
		DEBUG_MSG(DBG_PRINT("c.AviPackerSwitch[0x%x] = 0x%x\r\n", bflag, nRet));
	} else {
		RETURN(STATUS_FAIL);
	}
	nRet = STATUS_OK;
Return:	
	return nRet;
}


/****************************************************************************/
/*
 *	avi_enc_save_jpeg
 */
INT32S avi_enc_save_jpeg(void)
{
	INT32S nRet;
	
	nRet = STATUS_OK;

	OSQPost(my_AVIEncodeApQ, (void *) MSG_AVI_CAPTURE_PICTURE);

	return nRet;
}

/****************************************************************************/
/*
 *	vid_enc_preview_buf_to_dummy
 */
INT32S vid_enc_preview_buf_to_dummy(void)
{
	INT8U  err;
	INT32S nRet, msg;
	
	nRet = STATUS_OK;

	POST_MESSAGE(my_AVIEncodeApQ, MSG_PREVIEW_BUF_TO_DUMMY, my_avi_encode_ack_m, 5000, msg, err);	
Return:    
	return nRet;

}

/****************************************************************************/
/*
 *	vid_enc_preview_buf_to_display
 */
INT32S vid_enc_preview_buf_to_display(void)
{
	INT8U  err;
	INT32S nRet, msg;
	
	nRet = STATUS_OK;

	POST_MESSAGE(my_AVIEncodeApQ, MSG_PREVIEW_BUF_TO_DISPLAY, my_avi_encode_ack_m, 5000, msg, err);	
Return:    
	return nRet;

}

void avi_encode_audio_timer_start(void)
{
	mic_sample_rate_set(AVI_AUDIO_RECORD_TIMER, pAviEncAudPara->audio_sample_rate);
}

void avi_encode_audio_timer_stop(void)
{
	mic_timer_stop(AVI_AUDIO_RECORD_TIMER);
}

// file handle
INT16S avi_encode_close_file(AviEncPacker_t *pAviEncPacker)
{
	INT32S nRet;
	
	nRet = close(pAviEncPacker->file_handle);
	nRet = close(pAviEncPacker->index_handle);
    nRet = unlink2((CHAR*)pAviEncPacker->index_path);
	pAviEncPacker->file_handle = -1;
	pAviEncPacker->index_handle = -1;
	
	return nRet;
}



INT32S avi_encode_set_avi_header(AviEncPacker_t *pAviEncPacker)
{
	INT16U sample_per_block, package_size;
	
	pAviEncPacker->p_avi_aud_stream_header = &avi_aud_stream_header;
	pAviEncPacker->p_avi_vid_stream_header = &avi_vid_stream_header;
	pAviEncPacker->p_avi_bitmap_info = &avi_bitmap_info;
	pAviEncPacker->p_avi_wave_info = &avi_wave_info;
	gp_memset((INT8S*)pAviEncPacker->p_avi_aud_stream_header, 0, sizeof(GP_AVI_AVISTREAMHEADER));
	gp_memset((INT8S*)pAviEncPacker->p_avi_vid_stream_header, 0, sizeof(GP_AVI_AVISTREAMHEADER));
	gp_memset((INT8S*)pAviEncPacker->p_avi_bitmap_info, 0, sizeof(GP_AVI_BITMAPINFO));
	gp_memset((INT8S*)pAviEncPacker->p_avi_wave_info, 0, sizeof(GP_AVI_PCMWAVEFORMAT));
	
	//audio
	avi_aud_stream_header.fccType[0] = 'a';
	avi_aud_stream_header.fccType[1] = 'u';
	avi_aud_stream_header.fccType[2] = 'd';
	avi_aud_stream_header.fccType[3] = 's';
	
	switch(pAviEncAudPara->audio_format) 
	{
	case WAVE_FORMAT_PCM:
		pAviEncPacker->wave_info_cblen = 16;
		avi_aud_stream_header.fccHandler[0] = 1;
		avi_aud_stream_header.fccHandler[1] = 0;
		avi_aud_stream_header.fccHandler[2] = 0;
		avi_aud_stream_header.fccHandler[3] = 0;
			
		avi_wave_info.wFormatTag = WAVE_FORMAT_PCM;
		avi_wave_info.nChannels = pAviEncAudPara->channel_no;	
		avi_wave_info.nSamplesPerSec =  pAviEncAudPara->audio_sample_rate;
		avi_wave_info.nAvgBytesPerSec =  pAviEncAudPara->channel_no * pAviEncAudPara->audio_sample_rate * 2; 
		avi_wave_info.nBlockAlign = pAviEncAudPara->channel_no * 2;
		avi_wave_info.wBitsPerSample = 16; //16bit
			
		avi_aud_stream_header.dwScale = avi_wave_info.nBlockAlign;
		avi_aud_stream_header.dwRate = avi_wave_info.nAvgBytesPerSec;
		avi_aud_stream_header.dwSampleSize = avi_wave_info.nBlockAlign;;	
		break;
		
	case WAVE_FORMAT_ADPCM:
		pAviEncPacker->wave_info_cblen = 50;
		avi_aud_stream_header.fccHandler[0] = 0;
		avi_aud_stream_header.fccHandler[1] = 0;
		avi_aud_stream_header.fccHandler[2] = 0;
		avi_aud_stream_header.fccHandler[3] = 0;

		package_size = 0x100;
		if(pAviEncAudPara->channel_no == 1)
			sample_per_block = 2 * package_size - 12;
		else if(pAviEncAudPara->channel_no == 2)
			sample_per_block = package_size - 12;
		else
			sample_per_block = 1;
		
		avi_wave_info.wFormatTag = WAVE_FORMAT_ADPCM;
		avi_wave_info.nChannels = pAviEncAudPara->channel_no;	
		avi_wave_info.nSamplesPerSec =  pAviEncAudPara->audio_sample_rate;
		avi_wave_info.nAvgBytesPerSec =  package_size * pAviEncAudPara->audio_sample_rate / sample_per_block; // = PackageSize * FrameSize / fs
		avi_wave_info.nBlockAlign = package_size; //PackageSize
		avi_wave_info.wBitsPerSample = 4; //4bit
		avi_wave_info.cbSize = 32;
		// extension ...
		avi_wave_info.ExtInfo[0] = 0x01F4;	avi_wave_info.ExtInfo[1] = 0x0007;	
		avi_wave_info.ExtInfo[2] = 0x0100;	avi_wave_info.ExtInfo[3] = 0x0000;
		avi_wave_info.ExtInfo[4] = 0x0200;	avi_wave_info.ExtInfo[5] = 0xFF00;
		avi_wave_info.ExtInfo[6] = 0x0000;	avi_wave_info.ExtInfo[7] = 0x0000;
		
		avi_wave_info.ExtInfo[8] =  0x00C0;	avi_wave_info.ExtInfo[9] =  0x0040;
		avi_wave_info.ExtInfo[10] = 0x00F0; avi_wave_info.ExtInfo[11] = 0x0000;
		avi_wave_info.ExtInfo[12] = 0x01CC; avi_wave_info.ExtInfo[13] = 0xFF30;
		avi_wave_info.ExtInfo[14] = 0x0188; avi_wave_info.ExtInfo[15] = 0xFF18;
		break;
		
	case WAVE_FORMAT_IMA_ADPCM:
		pAviEncPacker->wave_info_cblen = 20;
		avi_aud_stream_header.fccHandler[0] = 0;
		avi_aud_stream_header.fccHandler[1] = 0;
		avi_aud_stream_header.fccHandler[2] = 0;
		avi_aud_stream_header.fccHandler[3] = 0;
		
		package_size = 0x100;
		if(pAviEncAudPara->channel_no == 1)
			sample_per_block = 2 * package_size - 7;
		else if(pAviEncAudPara->channel_no == 2)
			sample_per_block = package_size - 7;
		else
			sample_per_block = 1;
		
		avi_wave_info.wFormatTag = WAVE_FORMAT_IMA_ADPCM;
		avi_wave_info.nChannels = pAviEncAudPara->channel_no;	
		avi_wave_info.nSamplesPerSec =  pAviEncAudPara->audio_sample_rate;
		avi_wave_info.nAvgBytesPerSec =  package_size * pAviEncAudPara->audio_sample_rate / sample_per_block;
		avi_wave_info.nBlockAlign = package_size;	//PackageSize
		avi_wave_info.wBitsPerSample = 4; //4bit
		avi_wave_info.cbSize = 2;
		// extension ...
		avi_wave_info.ExtInfo[0] = sample_per_block;
		break;
		
	default:
		pAviEncPacker->wave_info_cblen = 0;
		pAviEncPacker->p_avi_aud_stream_header = NULL; 
		pAviEncPacker->p_avi_wave_info = NULL;
	}
	
	avi_aud_stream_header.dwScale = avi_wave_info.nBlockAlign;
	avi_aud_stream_header.dwRate = avi_wave_info.nAvgBytesPerSec;
	avi_aud_stream_header.dwSampleSize = avi_wave_info.nBlockAlign;
	
	//video
	avi_vid_stream_header.fccType[0] = 'v';
	avi_vid_stream_header.fccType[1] = 'i';
	avi_vid_stream_header.fccType[2] = 'd';
	avi_vid_stream_header.fccType[3] = 's';
	avi_vid_stream_header.dwScale = my_pAviEncVidPara->dwScale;
	avi_vid_stream_header.dwRate = my_pAviEncVidPara->dwRate;
	avi_vid_stream_header.rcFrame.right = my_pAviEncVidPara->encode_width;
	avi_vid_stream_header.rcFrame.bottom = my_pAviEncVidPara->encode_height;

	avi_vid_stream_header.fccHandler[0] = 'm';
	avi_vid_stream_header.fccHandler[1] = 'j';
	avi_vid_stream_header.fccHandler[2] = 'p';
	avi_vid_stream_header.fccHandler[3] = 'g';
	
	avi_bitmap_info.biSize = pAviEncPacker->bitmap_info_cblen = 40;
	avi_bitmap_info.biWidth = my_pAviEncVidPara->encode_width;
	avi_bitmap_info.biHeight = my_pAviEncVidPara->encode_height;
	avi_bitmap_info.biBitCount = 24;
	avi_bitmap_info.biCompression[0] = 'M';
	avi_bitmap_info.biCompression[1] = 'J';
	avi_bitmap_info.biCompression[2] = 'P';
	avi_bitmap_info.biCompression[3] = 'G';
	avi_bitmap_info.biSizeImage = my_pAviEncVidPara->encode_width * my_pAviEncVidPara->encode_height * 3;

	return STATUS_OK;
}


// status
void avi_encode_set_status(INT32U bit)
{
	pAviEncPara->avi_encode_status |= bit;
}  

void avi_encode_clear_status(INT32U bit)
{
	pAviEncPara->avi_encode_status &= ~bit;
}  

INT32S avi_encode_get_status(void)
{
    return pAviEncPara->avi_encode_status;
}


INT32S avi_encode_packer_memory_alloc(void)
{
	INT32S nRet;
	
	if(AviPacker_mem_alloc(pAviEncPacker0) < 0) RETURN(STATUS_FAIL);
	nRet = STATUS_OK;
Return:	
	return nRet;
}

// check disk free size
INT32S avi_encode_disk_size_is_enough(INT32S cb_write_size)
{
    INT32S nRet;
#if AVI_ENCODE_CAL_DISK_SIZE_EN	
	INT32U temp;
	
	INT64U disk_free_size;
	
	temp = pAviEncPara->record_total_size;
	disk_free_size = pAviEncPara->disk_free_size;
	temp += cb_write_size;
	if(temp >= AVI_FILE_MAX_RECORD_SIZE) RETURN(2);
	if(temp >= disk_free_size) RETURN(FALSE);
	pAviEncPara->record_total_size = temp;
#endif

	nRet = TRUE;
//Return:
	return nRet;
}

//audio
INT32S avi_audio_memory_allocate(INT32U	cblen)
{
	INT32U i, ptr;

	g_pcm_index = 0;
	
	cblen = (cblen + 0xF) & ~0xF;
	ptr = (INT32U) gp_malloc_align(cblen * AVI_ENCODE_PCM_BUFFER_NO, 16);
	if (!ptr) {
		for (i=0; i<AVI_ENCODE_PCM_BUFFER_NO; i++) {
			pAviEncAudPara->pcm_input_addr[i] = NULL;
		}
		
		return STATUS_FAIL;
	}
	
	for(i=0; i<AVI_ENCODE_PCM_BUFFER_NO; i++)
	{
		pAviEncAudPara->pcm_input_addr[i] = ptr;
		ptr += cblen;
	}
	
	return STATUS_OK;
}

void avi_audio_memory_free(void)
{
	INT32U i;
	
	if (pAviEncAudPara->pcm_input_addr[0]) {
		gp_free((void *) pAviEncAudPara->pcm_input_addr[0]);
	}
	for(i=0; i<AVI_ENCODE_PCM_BUFFER_NO; i++)
	{
		pAviEncAudPara->pcm_input_addr[i] = 0;
	}	
}

INT32U avi_audio_get_next_buffer(void)
{
	INT32U addr;

	addr = pAviEncAudPara->pcm_input_addr[g_pcm_index++];
	if(g_pcm_index >= AVI_ENCODE_PCM_BUFFER_NO)
		g_pcm_index = 0;
	
	return addr;
}

INT8U avi_audio_get_buffer_idx(void)
{
	return (INT8U)g_pcm_index;
}

INT32S avi_adc_double_buffer_put(INT16U *data,INT32U cwlen, OS_EVENT *os_q)
{
	INT32S status;

	g_avi_adc_dma_dbf.s_addr = (INT32U) P_I2SRX_DATA;
	g_avi_adc_dma_dbf.t_addr = (INT32U) data;
	g_avi_adc_dma_dbf.width = DMA_DATA_WIDTH_4BYTE;		
	g_avi_adc_dma_dbf.count = (INT32U) cwlen/2;
	g_avi_adc_dma_dbf.notify = NULL;
	g_avi_adc_dma_dbf.timeout = 0;
	status = dma_transfer_with_double_buf(&g_avi_adc_dma_dbf, os_q);
	if(status < 0) return status;
	return STATUS_OK;
}

INT32U avi_adc_double_buffer_set(INT16U *data, INT32U cwlen)
{
	INT32S status;

	g_avi_adc_dma_dbf.s_addr = (INT32U) P_I2SRX_DATA;
	g_avi_adc_dma_dbf.t_addr = (INT32U) data;
	g_avi_adc_dma_dbf.width = DMA_DATA_WIDTH_4BYTE;		
	g_avi_adc_dma_dbf.count = (INT32U) cwlen/2;
	g_avi_adc_dma_dbf.notify = NULL;
	g_avi_adc_dma_dbf.timeout = 0;
	status = dma_transfer_double_buf_set(&g_avi_adc_dma_dbf);
	if(status < 0) return status;
	return STATUS_OK;
}

INT32S avi_adc_dma_status_get(void)
{
	if (g_avi_adc_dma_dbf.channel == 0xff) 
		return -1;
	
	return dma_status_get(g_avi_adc_dma_dbf.channel);	
}

void avi_adc_double_buffer_free(void)
{
	dma_transfer_double_buf_free(&g_avi_adc_dma_dbf);
	g_avi_adc_dma_dbf.channel = 0xff;
}

void avi_adc_hw_start(INT16U sampling_rate)
{
	//drv_l2_audiocodec_wm8988_init();
	//drv_l2_audiocodec_wm8988_rx_init();
	i2s_rx_init(0);
	i2s_rx_sample_rate_set(sampling_rate);
	i2s_rx_mono_ch_set();
	i2s_rx_start();
}

void avi_adc_hw_stop(void)
{
	i2s_rx_stop(0);
}

