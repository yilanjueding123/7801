/**************************************************************************************************
audio record to storage:
The encode format can be
1. A1800 (GP)
2. WAVE  (PCM)
3. WAVE  (microsoft ADPCM)
4. WAVE  (IMA ADPCM)
5. MP3	 sample rate, 48k, 44.1K, 32k, 24K, 22.05k, 16K, 12K, 11.025K, 8k  
		 bit rate, 32,40,48,56,64,80,96,112,128,160,192,224,256,320 kbps
		  			 8,16,24,32,40,48,56, 64, 80, 96,112,128,144,160 kbps
***************************************************************************************************/
#include "audio_record.h"

/*  define */
#define ADC_RECORD_STACK_SIZE			1024
#define AUD_ENC_QACCEPT_MAX				5

/* os stack */
INT32U  AdcRecordStack[ADC_RECORD_STACK_SIZE];

/* os global varaible */
void	 	*aud_enc_reqQ_area[AUD_ENC_QACCEPT_MAX];
OS_EVENT 	*aud_enc_reqQ;

/* global varaible */
static AUD_ENC_WAVE_HEADER	WaveHeadPara;
static Audio_Encode_Para	AudioEncodePara;
static Audio_Encode_Para	*pAudio_Encode_Para;
static INT8U g_ring_buffer;
static INT8U g_buffer_index;
static INT8U g_switch_flag;
static INT32U g_aud_pcm_buffer[C_AUD_PCM_BUFFER_NO];

static DMA_STRUCT g_aud_adc_dma_dbf; 

INT32S (*pfn_audio_encode_stop)(void);
int    (*pfn_audio_encode_once)(void *workmem, const short* buffer_addr, int cwlen);
void   *hProc;

/* function */
//dma
static INT32S aud_adc_double_buffer_put(INT16U *data,INT32U cwlen, OS_EVENT *os_q);
static INT32U aud_adc_double_buffer_set(INT16U *data, INT32U cwlen);
static INT32S aud_adc_dma_status_get(void);
static void aud_adc_double_buffer_free(void);
static void adc_hardware_start(INT32U SampleRate);

//other
static void aud_enc_RIFF_init(INT32U samplerate);
static INT32S write_audio_data(INT32U buffer_addr, INT32U cbLen);
static INT32S adc_memory_allocate(INT32U cbLen);
static INT32S adc_memory_free(void);
static void adc_work_mem_free(void);

//wave
static INT32S wave_encode_start(void);
static INT32S wave_encode_stop(void);
static int wave_encode_once(void *workmem, const short* buffer_addr, int cwlen);

#if APP_WAV_CODEC_EN
static INT32S wave_encode_lib_start(INT32U AudioFormat);
static INT32S wave_encode_lib_stop(void);
static int wave_encode_lib_once(void *workmem, const short* buffer_addr, int cwlen);
#endif
#if APP_MP3_ENCODE_EN
static INT32S mp3_encode_start(void);
static INT32S mp3_encode_stop(void);
static int mp3_encode_once(void *workmem, const short* buffer_addr, int cwlen);
#endif

extern INT32S mic_timer_stop(INT8U timer_id);
extern void mic_fifo_clear(void);
extern void mic_fifo_level_set(INT8U level);
extern INT32S mic_auto_sample_start(void);
extern INT32S mic_sample_rate_set(INT8U timer_id, INT32U hz);
//===============================================================================================
// 	audio task create
//	parameter:	priority.
//	return:		none.
//===============================================================================================
INT32S adc_record_task_create(INT8U priority)
{
	INT8U 	err;
	aud_enc_reqQ = OSQCreate(aud_enc_reqQ_area, AUD_ENC_QACCEPT_MAX);
	if (!aud_enc_reqQ)		
		return STATUS_FAIL;
	
	pAudio_Encode_Para = &AudioEncodePara;
	gp_memset((INT8S *)pAudio_Encode_Para, 0, sizeof(AudioEncodePara));
	
	err = OSTaskCreate(adc_record_entry, 0, (void *)&AdcRecordStack[ADC_RECORD_STACK_SIZE - 1], priority);
	DEBUG_MSG(DBG_PRINT("AudioEncodeTaskCreate = %d\r\n", err));
    if(err != OS_NO_ERR)
    	return STATUS_FAIL;
    else
    	return STATUS_OK;
}

//===============================================================================================
// 	audio task detete
//	parameter:	priority.
//	return:		none.
//===============================================================================================
INT32S adc_record_task_del(INT8U priority)
{
	INT8U err;
	
	OSQFlush(aud_enc_reqQ);
	err = OSTaskDel(priority);
	DEBUG_MSG(DBG_PRINT("AudioEncodeTaskDel = %d\r\n", err));
	OSQDel(aud_enc_reqQ, OS_DEL_ALWAYS, &err);
	return err;
}

//===============================================================================================
// 	audio task main entry
//===============================================================================================
void adc_record_entry(void *param)
{
	INT8U   err;
	INT16U  mask, t;
	INT16U  *ptr;
	INT32U  msg_id, pcm_addr;
	INT32S  nRet, i;
	INT32U  ready_addr, free_addr;

	while(1) 
	{
		msg_id = (INT32U) OSQPend(aud_enc_reqQ, 0, &err);
		if(err != OS_NO_ERR)
			continue;
		
		switch(msg_id)
		{
		case MSG_ADC_DMA_DONE:
			pcm_addr = ready_addr;
			ready_addr = free_addr;
			g_buffer_index++;
			if(g_buffer_index >= C_AUD_PCM_BUFFER_NO) g_buffer_index = 0;
			free_addr = g_aud_pcm_buffer[g_buffer_index];
			
			if(pAudio_Encode_Para->Status == C_STOP_RECORDING)
			{	//check dma is done and stop
				if(aud_adc_dma_status_get() == 0) {
					OSQPost(aud_enc_reqQ, (void *) MSG_AUDIO_ENCODE_STOP);
				}
			}
			else
			{	//set dma buffer
				aud_adc_double_buffer_set((INT16U *)free_addr, pAudio_Encode_Para->PCMInFrameSize);
			}
			
			DEBUG_MSG(DBG_PRINT(".1"));
			//unsigned to signed
			cache_invalid_range(pcm_addr, pAudio_Encode_Para->PCMInFrameSize << 1);
			ptr = (INT16U*)pcm_addr;
		#if	1	//20111019
		for(i=0; i<pAudio_Encode_Para->PCMInFrameSize; i++)
			{
				//t = *ptr;
			//	t ^= mask;
		#if LPF_ENABLE == 1
				t = (INT16U)LPF_process(*ptr);
		#endif
				*ptr++ = t;	
			}
	   #endif	
			//encode pcm wave 
			nRet = pfn_audio_encode_once(hProc, (const short*)pcm_addr, pAudio_Encode_Para->PCMInFrameSize); 	
			if(nRet < 0)
			{
				OSQPost(aud_enc_reqQ, (void *) MSG_AUDIO_ENCODE_ERR);	
				continue;
			}
			
			//check storage is full
			if(pAudio_Encode_Para->SourceType == C_GP_FS)
			{
				if(pAudio_Encode_Para->FileLenth >= pAudio_Encode_Para->disk_free_size)
					OSQPost(aud_enc_reqQ, (void *) MSG_AUDIO_ENCODE_STOPING);
			}
			break;
		
		case MSG_AUDIO_ENCODE_START:
			hProc = 0;
			g_ring_buffer = 0;
			mask = 0x8000;
			
			if(pAudio_Encode_Para->AudioFormat == WAVE_FORMAT_PCM)
			{
				nRet = wave_encode_start();
				pfn_audio_encode_once = wave_encode_once;
				pfn_audio_encode_stop = wave_encode_stop;
			}
			
		#if	APP_WAV_CODEC_EN		
			else if((pAudio_Encode_Para->AudioFormat == WAVE_FORMAT_IMA_ADPCM) ||
					(pAudio_Encode_Para->AudioFormat == WAVE_FORMAT_ADPCM))
			{
				nRet = wave_encode_lib_start(pAudio_Encode_Para->AudioFormat);
				pfn_audio_encode_once = wave_encode_lib_once;
				pfn_audio_encode_stop = wave_encode_lib_stop;
			}
		#endif
		#if APP_MP3_ENCODE_EN
			else if(pAudio_Encode_Para->AudioFormat == WAVE_FORMAT_MP3)
			{
				nRet = mp3_encode_start();	
				pfn_audio_encode_once = mp3_encode_once;
				pfn_audio_encode_stop = mp3_encode_stop;
			}
		#endif
			if(nRet < 0)
			{
				DEBUG_MSG(DBG_PRINT("AudioEncodeStartFail!!!\r\n"));
				adc_work_mem_free();
				pAudio_Encode_Para->Status = C_START_FAIL;
				continue;
			}
						
		#if	APP_DOWN_SAMPLE_EN
			if(pAudio_Encode_Para->bEnableDownSample)
			{	
				if(pAudio_Encode_Para->DownsampleFactor * pAudio_Encode_Para->SampleRate > 48000)
					pAudio_Encode_Para->DownsampleFactor = 48000 / pAudio_Encode_Para->SampleRate;
					
				if(pAudio_Encode_Para->DownsampleFactor >= 2 )
				{	
					pAudio_Encode_Para->DownSampleWorkMem = DownSample_Create(pAudio_Encode_Para->PCMInFrameSize, pAudio_Encode_Para->DownsampleFactor);	
					
					DownSample_Link(pAudio_Encode_Para->DownSampleWorkMem,	NULL, pfn_audio_encode_once, pAudio_Encode_Para->SampleRate,
										pAudio_Encode_Para->Channel, pAudio_Encode_Para->DownsampleFactor);
														
					hProc = pAudio_Encode_Para->DownSampleWorkMem;
					pfn_audio_encode_once = DownSample_PutData;
					
					pAudio_Encode_Para->SampleRate = DownSample_GetSampleRate(hProc);
					pAudio_Encode_Para->Channel = DownSample_GetChannel(hProc);
					pAudio_Encode_Para->PCMInFrameSize *=  pAudio_Encode_Para->DownsampleFactor; 	
				}
			}
		#endif
		#if	LPF_ENABLE == 1			
			//LPF_init(pAudio_Encode_Para->SampleRate, 3);
			//20111019
			//22050-4K-41760
			LPF_init(32768, 3);//49152//32768
		#endif				
			nRet = adc_memory_allocate(pAudio_Encode_Para->PCMInFrameSize<<1);
			if(nRet < 0)
			{
				DEBUG_MSG(DBG_PRINT("AudioEncodeMemoryFail!!!\r\n"));
				adc_work_mem_free();
				pAudio_Encode_Para->Status = C_START_FAIL;
				continue;
			}
			ready_addr = g_aud_pcm_buffer[0];
			free_addr = g_aud_pcm_buffer[1];
			g_buffer_index = 1;
			aud_adc_double_buffer_put((INT16U*)ready_addr, pAudio_Encode_Para->PCMInFrameSize, aud_enc_reqQ);
		    aud_adc_double_buffer_set((INT16U*)free_addr, pAudio_Encode_Para->PCMInFrameSize);
			adc_hardware_start(pAudio_Encode_Para->SampleRate);
			pAudio_Encode_Para->Status = C_START_RECORD;
			break;
		
		case MSG_AUDIO_ENCODE_STOPING:	
			pAudio_Encode_Para->Status = C_STOP_RECORDING;
			break;
			
		case MSG_AUDIO_ENCODE_STOP:
			DEBUG_MSG(DBG_PRINT("AudioEncodeStop\r\n"));
			mic_timer_stop(C_ADC_USE_TIMER);
			pfn_audio_encode_stop();
			OSQFlush(aud_enc_reqQ);
			adc_memory_free();
			aud_adc_double_buffer_free();
			
		#if APP_DOWN_SAMPLE_EN
			if(pAudio_Encode_Para->bEnableDownSample)
			{
				DownSample_Del(pAudio_Encode_Para->DownSampleWorkMem);
				pAudio_Encode_Para->bEnableDownSample = FALSE;
				pAudio_Encode_Para->DownsampleFactor = 0;
			}
		#endif			
			pAudio_Encode_Para->Status = C_STOP_RECORD;		
			break;
		
		case MSG_AUDIO_ENCODE_ERR:
			DEBUG_MSG(DBG_PRINT("AudioEncodeERROR!!!\r\n"));
//			OSQPost(aud_enc_reqQ, (void *) MSG_AUDIO_ENCODE_STOPING);			//wwj comment: must not send this message, let user use audio_encode_stop() to stop audio
//			msgQSend(ApQ, MSG_APQ_AVI_PACKER_ERROR, NULL, NULL, MSG_PRI_NORMAL);
			msgQSend(ApQ, MSG_APQ_AUDIO_ENCODE_ERR, NULL, NULL, MSG_PRI_NORMAL);
			break;
		}
	}
}

//===============================================================================================
// 	aud_adc_double_buffer_put 
//	parameter:	data = buffer_addr.
//				len  = size in word
//				os_q = OS _EVENT
//	return:		status.
//===============================================================================================
static INT32S aud_adc_double_buffer_put(INT16U *data,INT32U cwlen, OS_EVENT *os_q)
{
	INT32S status;

	g_aud_adc_dma_dbf.s_addr = (INT32U) P_MIC_ASADC_DATA;
	g_aud_adc_dma_dbf.t_addr = (INT32U) data;
	g_aud_adc_dma_dbf.width = DMA_DATA_WIDTH_2BYTE;		
	g_aud_adc_dma_dbf.count = (INT32U) cwlen;
	g_aud_adc_dma_dbf.notify = NULL;
	g_aud_adc_dma_dbf.timeout = 0;
	
	status = dma_transfer_with_double_buf(&g_aud_adc_dma_dbf, os_q);
	if (status != 0)
		return status;
		
	return STATUS_OK;
}

//===============================================================================================
// 	aud_adc_double_buffer_set 
//	parameter:	data = buffer_addr.
//				len  = size in word
//	return:		status.
//===============================================================================================
static INT32U aud_adc_double_buffer_set(INT16U *data, INT32U cwlen)
{
	INT32S status;
	
	g_aud_adc_dma_dbf.s_addr = (INT32U) P_MIC_ASADC_DATA;
	g_aud_adc_dma_dbf.t_addr = (INT32U) data;
	g_aud_adc_dma_dbf.width = DMA_DATA_WIDTH_2BYTE;		
	g_aud_adc_dma_dbf.count = (INT32U) cwlen;
	g_aud_adc_dma_dbf.notify = NULL;
	g_aud_adc_dma_dbf.timeout = 0;
	status = dma_transfer_double_buf_set(&g_aud_adc_dma_dbf);
	if(status != 0)
		return status;

	return STATUS_OK;
}

//===============================================================================================
// 	aud_adc_dma_status_get 
//	parameter:	none
//	return:		status.
//===============================================================================================
static INT32S aud_adc_dma_status_get(void)
{
	if (g_aud_adc_dma_dbf.channel == 0xff) 
		return -1;
	
	return dma_status_get(g_aud_adc_dma_dbf.channel);	
}

//===============================================================================================
// 	aud_adc_double_buffer_free 
//	parameter:	none
//	return:		free double buffer dma channel.
//===============================================================================================
static void aud_adc_double_buffer_free(void)
{
	dma_transfer_double_buf_free(&g_aud_adc_dma_dbf);
	g_aud_adc_dma_dbf.channel = 0xff;
}

//===============================================================================================
// 	adc_hardware_start 
//	parameter:	SampleRate = adc sample rate set.
//	return:		none.
//===============================================================================================
static void adc_hardware_start(INT32U SampleRate)
{	
//	R_MIC_SETUP = 0;
	R_MIC_SETUP = 0xC000;
	R_MIC_ASADC_CTRL = 0;
	R_MIC_SH_WAIT = 0x0807;
	
	mic_fifo_clear();	
	mic_fifo_level_set(4);
//	R_MIC_SETUP = 0xA0C0; //enable AGC
	R_MIC_SETUP = 0xF080;
	mic_auto_sample_start();
	OSTimeDly(50); //wait bias stable
	mic_sample_rate_set(C_ADC_USE_TIMER, SampleRate);

	DEBUG_MSG(DBG_PRINT("AudioSampleRate = %d\r\n", SampleRate));
}

//===============================================================================================
// 	aud_enc_RIFF_init
//	parameter:	samplerate:  wave file sample rate.
//	return:		none.
//===============================================================================================
static void aud_enc_RIFF_init(INT32U samplerate)
{
	WaveHeadPara.RIFF_ID[0] ='R';
	WaveHeadPara.RIFF_ID[1] ='I';
	WaveHeadPara.RIFF_ID[2] ='F';
	WaveHeadPara.RIFF_ID[3] ='F';
	WaveHeadPara.RIFF_len = 0;
	
	WaveHeadPara.type_ID[0] = 'W';
	WaveHeadPara.type_ID[1] = 'A';
	WaveHeadPara.type_ID[2] = 'V';
	WaveHeadPara.type_ID[3] = 'E';
	
	WaveHeadPara.fmt_ID[0] = 'f';
	WaveHeadPara.fmt_ID[1] = 'm';
	WaveHeadPara.fmt_ID[2] = 't';
	WaveHeadPara.fmt_ID[3] = ' ';
	
	WaveHeadPara.fmt_len = 16;
	WaveHeadPara.format = 1;
	WaveHeadPara.channel = 1;
	WaveHeadPara.sample_rate = samplerate;
	
	//8, 16, 24 or 32
	WaveHeadPara.Sign_bit_per_sample = 16;
	//BlockAlign = SignificantBitsPerSample / 8 * NumChannels 
	WaveHeadPara.Block_align = 16/8*1;
	//AvgBytesPerSec = SampleRate * BlockAlign 
	WaveHeadPara.avg_byte_per_sec = samplerate*2;
	
	WaveHeadPara.data_ID[0] = 'd';
	WaveHeadPara.data_ID[1] = 'a'; 
	WaveHeadPara.data_ID[2] = 't';
	WaveHeadPara.data_ID[3] = 'a';
	
	WaveHeadPara.data_len = 0;
}

//===============================================================================================
// 	write_audio_data 
//	parameter:	buffer_addr = buffer_addr.
//				cblen = size in byte
//	return:		success write length in byte.
//===============================================================================================
static INT32S write_audio_data(INT32U buffer_addr, INT32U cbLen)
{
	INT32U cb_write_len;
	
	cb_write_len = write(pAudio_Encode_Para->FileHandle, (INT32U)buffer_addr, cbLen);
		
	if(cb_write_len != cbLen)
	{
		DEBUG_MSG(DBG_PRINT("AudioWriteFileFail!!!\r\n"));
		return AUD_RECORD_FILE_WRITE_ERR;	
	}
	return cb_write_len;	//byte size
}

//===============================================================================================
// 	save_buffer_to_storage 
//	parameter:	
//	return:		status
//===============================================================================================
static INT32S save_buffer_to_storage(void)
{
	INT8U  *addr;
	INT32S size, write_cblen;
	
	if(!g_ring_buffer)
	{
		if(pAudio_Encode_Para->read_index > C_BS_BUFFER_SIZE/2)
		{
			g_ring_buffer = ~g_ring_buffer;
			addr = pAudio_Encode_Para->Bit_Stream_Buffer;
			size = C_BS_BUFFER_SIZE/2;
			write_cblen = write_audio_data((INT32U)addr, size);
				
			if(write_cblen != size)
				return 	AUD_RECORD_FILE_WRITE_ERR;	
		}
	}
	else
	{
		if(pAudio_Encode_Para->read_index < C_BS_BUFFER_SIZE/2)
		{
			g_ring_buffer = ~g_ring_buffer;
			addr = pAudio_Encode_Para->Bit_Stream_Buffer + C_BS_BUFFER_SIZE/2;
			size = C_BS_BUFFER_SIZE/2;
			write_cblen = write_audio_data((INT32U)addr, size);
				
			if(write_cblen != size)
				return 	AUD_RECORD_FILE_WRITE_ERR;	
		}
	}
	
	return AUD_RECORD_STATUS_OK;	
}

//===============================================================================================
// 	save_final_data_to_storage 
//	parameter:	
//	return:		status
//===============================================================================================
static INT32S save_final_data_to_storage(void)
{
	INT8U *addr;
	INT32S size, write_cblen;
	
	if(pAudio_Encode_Para->read_index > C_BS_BUFFER_SIZE/2)
	{
		addr = pAudio_Encode_Para->Bit_Stream_Buffer + C_BS_BUFFER_SIZE/2;
		size = pAudio_Encode_Para->read_index - C_BS_BUFFER_SIZE/2;
		write_cblen = write_audio_data((INT32U)addr, size);

		if(write_cblen != size)
			return 	AUD_RECORD_FILE_WRITE_ERR;
	}
	else
	{
		addr = pAudio_Encode_Para->Bit_Stream_Buffer;
		size = pAudio_Encode_Para->read_index;
		write_cblen = write_audio_data((INT32U)addr, size);

		if(write_cblen != size)
			return 	AUD_RECORD_FILE_WRITE_ERR;
	}
	
	return AUD_RECORD_STATUS_OK;
}

//===============================================================================================
// 	adc_memory_allocate 
//	parameter:	cbLen = adc buffer size in byte.
//	return:		status.
//===============================================================================================
static INT32S adc_memory_allocate(INT32U cbLen)
{
	INT32S i;

	for(i=0; i<C_AUD_PCM_BUFFER_NO; i++)
	{
		g_aud_pcm_buffer[i] =(INT32U) gp_malloc_align(cbLen, 32);
		if(!g_aud_pcm_buffer[i])
		{
			adc_memory_free();
			return AUD_RECORD_MEMORY_ALLOC_ERR;
		}
	}
	return AUD_RECORD_STATUS_OK;
}

//===============================================================================================
// 	adc_memory_free 
//	parameter:	none.
//	return:		status.
//===============================================================================================
static INT32S adc_memory_free(void)
{	
	INT32S i;
	
	for(i=0; i<C_AUD_PCM_BUFFER_NO; i++)
	{
		if(g_aud_pcm_buffer[i])
		{
			gp_free((void *)g_aud_pcm_buffer[i]);
			g_aud_pcm_buffer[i] = 0;
		}
	}
	return AUD_RECORD_STATUS_OK;
}

//===============================================================================================
// 	adc_work_mem_free 
//	parameter:	
//	return:		
//===============================================================================================
static void adc_work_mem_free(void)
{
	if(pAudio_Encode_Para->EncodeWorkMem)
		gp_free((void *)pAudio_Encode_Para->EncodeWorkMem); 
	if(pAudio_Encode_Para->Bit_Stream_Buffer)
		gp_free((void *)pAudio_Encode_Para->Bit_Stream_Buffer);
	if(pAudio_Encode_Para->pack_buffer)
		gp_free((void *)pAudio_Encode_Para->pack_buffer); 
}

//===============================================================================================
// 	audio_record_set_status 
//	parameter:	status
//	return:		
//===============================================================================================
void audio_record_set_status(INT32U status)
{
	pAudio_Encode_Para->Status = status;
}

//===============================================================================================
// 	audio_record_get_status 
//	parameter:	
//	return:		status.
//===============================================================================================
INT32U audio_record_get_status(void)
{
	return pAudio_Encode_Para->Status;
}

//===============================================================================================
// 	audio_record_set_source_type 
//	parameter:	type = GP_FS/user_define
//	return:		
//===============================================================================================
void audio_record_set_source_type(INT32U type)
{
	pAudio_Encode_Para->SourceType = type;
}

//===============================================================================================
// 	audio_record_get_source_type 
//	parameter:	
//	return:		type.
//===============================================================================================
INT32U audio_record_get_source_type(void)
{
	return pAudio_Encode_Para->SourceType;
}

//===============================================================================================
// 	audio_record_set_file_handle_free_size 
//	parameter:	file_handle = wave file handle
//	return:		status.
//===============================================================================================
INT64U audio_record_set_file_handle_free_size(INT16S file_handle)
{
	INT8U  fs_disk;
	INT64U disk_free = 0;
	
	if(file_handle >= 0)
	{
		fs_disk = GetDiskOfFile(file_handle);
		disk_free = vfsFreeSpace(fs_disk);
		pAudio_Encode_Para->FileHandle = file_handle;
		pAudio_Encode_Para->disk_free_size = disk_free - 1048576; //reserved 10K
	}
	else
	{
		pAudio_Encode_Para->FileHandle = -1;
		pAudio_Encode_Para->disk_free_size = 0;
	}
	return disk_free;
}

//===============================================================================================
// 	audio_record_set_info 
//	parameter:	audio_format = format
//				sample_rate =
//				bit_rate = 
//				channel =
//	return:		status.
//===============================================================================================
void audio_record_set_info(INT32U audio_format, INT32U sample_rate, INT32U bit_rate, INT16U channel)
{
	pAudio_Encode_Para->AudioFormat = audio_format;
	pAudio_Encode_Para->SampleRate = sample_rate;
	pAudio_Encode_Para->BitRate = bit_rate; 
	pAudio_Encode_Para->Channel = channel; //1, mono, 2, stereo
}

//===============================================================================================
// 	audio_record_set_down_sample 
//	parameter:	b_down_sample = enable/disable
//				ratio = 2~4
//	return:		status.
//===============================================================================================
INT32S audio_record_set_down_sample(BOOLEAN b_down_sample, INT8U ratio)
{
#if	APP_DOWN_SAMPLE_EN
	pAudio_Encode_Para->bEnableDownSample = b_down_sample;
	pAudio_Encode_Para->DownsampleFactor = ratio;
	return AUD_RECORD_STATUS_OK;
#else
	return AUD_RECORD_STATUS_ERR;
#endif
}

//===============================================================================================
// 	wave_encode_start 
//	parameter:	none.
//	return:		status.
//===============================================================================================
static INT32S wave_encode_start(void)
{
	INT32S cbLen;
	AUD_ENC_WAVE_HEADER *pWaveHeadPara;
	
	g_switch_flag = 0;
	pAudio_Encode_Para->EncodeWorkMem = NULL;
	pAudio_Encode_Para->Bit_Stream_Buffer = (INT8U *)gp_malloc_align(C_BS_BUFFER_SIZE, 4);
	if(!pAudio_Encode_Para->Bit_Stream_Buffer)
		return AUD_RECORD_MEMORY_ALLOC_ERR;	
	//copy header 	
	pWaveHeadPara = &WaveHeadPara;
	cbLen = sizeof(AUD_ENC_WAVE_HEADER);
	aud_enc_RIFF_init(pAudio_Encode_Para->SampleRate);
	gp_memcpy((INT8S*)pAudio_Encode_Para->Bit_Stream_Buffer, (INT8S*)pWaveHeadPara, cbLen);
	
	pAudio_Encode_Para->read_index = cbLen;
	pAudio_Encode_Para->FileLenth = cbLen;
	pAudio_Encode_Para->PackSize = C_WAVE_RECORD_BUFFER_SIZE<<1;
	pAudio_Encode_Para->pack_buffer = NULL;
	pAudio_Encode_Para->PCMInFrameSize = C_WAVE_RECORD_BUFFER_SIZE;
	pAudio_Encode_Para->OnePCMFrameSize = C_WAVE_RECORD_BUFFER_SIZE;
	
	return AUD_RECORD_STATUS_OK;
}

//===============================================================================================
// 	wave_encode_stop 
//	parameter:	none.
//	return:		status.
//===============================================================================================
static INT32S wave_encode_stop(void)
{
	INT32S nRet;
	AUD_ENC_WAVE_HEADER *pWaveHeadPara;
	
	g_switch_flag = 0;
	nRet = save_final_data_to_storage();
	if(nRet < 0)
	{
		adc_work_mem_free();
		return AUD_RECORD_FILE_WRITE_ERR;
	}
	
	//write header
	pWaveHeadPara = &WaveHeadPara;
	pAudio_Encode_Para->FileLenth -= sizeof(AUD_ENC_WAVE_HEADER);
	pWaveHeadPara->RIFF_len = sizeof(AUD_ENC_WAVE_HEADER) + pAudio_Encode_Para->FileLenth - 8;//file size -8
	pWaveHeadPara->data_len = pAudio_Encode_Para->FileLenth;	
	lseek(pAudio_Encode_Para->FileHandle, 0, SEEK_SET);
	write(pAudio_Encode_Para->FileHandle, (INT32U)pWaveHeadPara, sizeof(AUD_ENC_WAVE_HEADER));
	close(pAudio_Encode_Para->FileHandle);
	
	//free memory
	adc_work_mem_free();
	return AUD_RECORD_STATUS_OK;
}

//===============================================================================================
// 	wave_encode_once 
//	parameter:	workmem = work memory
//				buffer_addr = buffer address
//				cwlen = buffer size in word
//	return:		length in word
//===============================================================================================
static int wave_encode_once(void *workmem, const short* buffer_addr, int cwlen)
{
	INT8U  *dest_addr;
	INT32S nRet, temp;
	INT32U cblen, size;
	
	cblen = cwlen<<1;
	temp = pAudio_Encode_Para->read_index + cblen;
	if(temp > C_BS_BUFFER_SIZE)
	{
		size = C_BS_BUFFER_SIZE - pAudio_Encode_Para->read_index;
		dest_addr = pAudio_Encode_Para->Bit_Stream_Buffer + pAudio_Encode_Para->read_index;
		gp_memcpy((INT8S*)dest_addr, (INT8S*)buffer_addr, size);
		
		temp = cblen - size;		//remain size
		dest_addr = pAudio_Encode_Para->Bit_Stream_Buffer;
		buffer_addr += (size>>1); 	//word address
		gp_memcpy((INT8S*)dest_addr, (INT8S*)buffer_addr, temp);
		pAudio_Encode_Para->read_index = temp;
	}
	else
	{
		dest_addr = pAudio_Encode_Para->Bit_Stream_Buffer + pAudio_Encode_Para->read_index;
		gp_memcpy((INT8S*)dest_addr, (INT8S*)buffer_addr, cblen);
		pAudio_Encode_Para->read_index += cblen;
	}
	
	pAudio_Encode_Para->FileLenth += cblen;
	if (pAudio_Encode_Para->FileLenth > AUD_FILE_MAX_RECORD_SIZE && !g_switch_flag) {
		g_switch_flag = 1;
		msgQSend(ApQ, MSG_APQ_RECORD_SWITCH_FILE, NULL, NULL, MSG_PRI_NORMAL);
	}

	/*
	if (pAudio_Encode_Para->FileLenth > pAudio_Encode_Para->disk_free_size) {
		INT32U led_type;
				
		led_type = LED_WAITING_CAPTURE;
		msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_FLASH_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
	} */
	nRet = save_buffer_to_storage();
	if(nRet < 0)
		return AUD_RECORD_FILE_WRITE_ERR;

	return cwlen;  
}


//===============================================================================================
// 	wave_encode_lib_start 
//	parameter:	none.
//	return:		status.
//===============================================================================================
#if APP_WAV_CODEC_EN
static INT32S wave_encode_lib_start(INT32U AudioFormat)
{
	INT32S nRet, size;
	
	size = wav_enc_get_mem_block_size();
	pAudio_Encode_Para->EncodeWorkMem = (INT8U *)gp_malloc(size);
	if(pAudio_Encode_Para->EncodeWorkMem == NULL)
		return AUD_RECORD_MEMORY_ALLOC_ERR;
	
	gp_memset((INT8S*)pAudio_Encode_Para->EncodeWorkMem, 0, size);
	pAudio_Encode_Para->Bit_Stream_Buffer = (INT8U *)gp_malloc_align(C_BS_BUFFER_SIZE, 4);
	if(!pAudio_Encode_Para->Bit_Stream_Buffer)
		return AUD_RECORD_MEMORY_ALLOC_ERR;
		
	nRet = wav_enc_Set_Parameter( pAudio_Encode_Para->EncodeWorkMem, 
								  pAudio_Encode_Para->Channel, 
								  pAudio_Encode_Para->SampleRate, 
								  AudioFormat );
	if(nRet < 0)	
		return AUD_RECORD_RUN_ERR;
	
	nRet = wav_enc_init(pAudio_Encode_Para->EncodeWorkMem);
	if(nRet < 0)	
		return AUD_RECORD_RUN_ERR;
		
	//copy header
	size = wav_enc_get_HeaderLength(pAudio_Encode_Para->EncodeWorkMem);	
	gp_memset((INT8S*)pAudio_Encode_Para->Bit_Stream_Buffer, 0, size);
	pAudio_Encode_Para->read_index = size;
	pAudio_Encode_Para->FileLenth = size;
	pAudio_Encode_Para->NumSamples = 0;
	pAudio_Encode_Para->PackSize = wav_enc_get_BytePerPackage(pAudio_Encode_Para->EncodeWorkMem);
	pAudio_Encode_Para->pack_buffer = (INT8U *)gp_malloc_align(pAudio_Encode_Para->PackSize, 4);
	if(!pAudio_Encode_Para->pack_buffer)
		return AUD_RECORD_MEMORY_ALLOC_ERR;
		
	size = wav_enc_get_SamplePerFrame(pAudio_Encode_Para->EncodeWorkMem);
	pAudio_Encode_Para->PCMInFrameSize = size * ADPCM_TIMES;
	pAudio_Encode_Para->OnePCMFrameSize = size;
	
	return AUD_RECORD_STATUS_OK;
}

//===============================================================================================
// 	wave_encode_lib_stop 
//	parameter:	none.
//	return:		status.
//===============================================================================================
static INT32S wave_encode_lib_stop(void)
{
	INT8U *pHeader;
	INT32S cbLen, nRet;
	
	nRet = save_final_data_to_storage();
	if(nRet < 0)
	{
		adc_work_mem_free();
		return AUD_RECORD_FILE_WRITE_ERR;
	}
	
	//write header
	cbLen = wav_enc_get_HeaderLength(pAudio_Encode_Para->EncodeWorkMem);
	pHeader = (INT8U *)gp_malloc(cbLen);
	if(!pHeader)
	{
		adc_work_mem_free();
		return AUD_RECORD_MEMORY_ALLOC_ERR;
	}
	
	nRet = wav_enc_get_header(pAudio_Encode_Para->EncodeWorkMem, pHeader, pAudio_Encode_Para->FileLenth);
	lseek(pAudio_Encode_Para->FileHandle, 0, SEEK_SET);
	write(pAudio_Encode_Para->FileHandle, (INT32U)pHeader, cbLen);
	close(pAudio_Encode_Para->FileHandle);

	//free memory
	gp_free((void *)pHeader);
	adc_work_mem_free();
	
	return	AUD_RECORD_STATUS_OK;
}

//===============================================================================================
// 	wave_encode_lib_once 
//	parameter:	workmem = work memory
//				buffer_addr = buffer address
//				cwlen = buffer size in word
//	return:		length in word
//===============================================================================================
static int wave_encode_lib_once(void *workmem, const short* buffer_addr, int cwlen)
{
	INT8U  *dest_addr;
	INT32U N;
	INT32S nRet, cblen, temp, size;
	
	cblen = 0;
	N = cwlen;
	
	while(N >= pAudio_Encode_Para->OnePCMFrameSize)
	{
		nRet = wav_enc_run(pAudio_Encode_Para->EncodeWorkMem, (short *)buffer_addr, pAudio_Encode_Para->pack_buffer);
		if(nRet < 0)		
			return  AUD_RECORD_RUN_ERR;
		
		pAudio_Encode_Para->NumSamples += wav_enc_get_SamplePerFrame(pAudio_Encode_Para->EncodeWorkMem);
		buffer_addr += pAudio_Encode_Para->OnePCMFrameSize;
		cblen += pAudio_Encode_Para->PackSize;
		N -= pAudio_Encode_Para->OnePCMFrameSize;
		
		//copy to bit stream buffer
		temp = pAudio_Encode_Para->read_index + pAudio_Encode_Para->PackSize;
		if(temp > C_BS_BUFFER_SIZE)
		{
			size = C_BS_BUFFER_SIZE - pAudio_Encode_Para->read_index;
			dest_addr = pAudio_Encode_Para->Bit_Stream_Buffer + pAudio_Encode_Para->read_index;
			gp_memcpy((INT8S*)dest_addr, (INT8S*)pAudio_Encode_Para->pack_buffer, size);
		
			temp = pAudio_Encode_Para->PackSize - size;		//remain size
			dest_addr = pAudio_Encode_Para->Bit_Stream_Buffer;
			gp_memcpy((INT8S*)dest_addr, (INT8S*)(pAudio_Encode_Para->pack_buffer + size), temp);
			pAudio_Encode_Para->read_index = temp;
		}
		else
		{
			dest_addr = pAudio_Encode_Para->Bit_Stream_Buffer + pAudio_Encode_Para->read_index;
			gp_memcpy((INT8S*)dest_addr, (INT8S*)pAudio_Encode_Para->pack_buffer, pAudio_Encode_Para->PackSize);
			pAudio_Encode_Para->read_index += pAudio_Encode_Para->PackSize;
		}
	}
	
	pAudio_Encode_Para->FileLenth += cblen;
	nRet = save_buffer_to_storage();
	if(nRet < 0)
		return AUD_RECORD_FILE_WRITE_ERR;
			
	return cwlen;
}
#endif	//APP_WAV_CODEC_EN

//===============================================================================================
// 	mp3_encode_start 
//	parameter:	none.
//	return:		status.
//===============================================================================================
#if APP_MP3_ENCODE_EN
static INT32S mp3_encode_start(void)
{
	INT32S nRet, size;
	
	g_switch_flag = 0;
	size = mp3enc_GetWorkMemSize();
	pAudio_Encode_Para->EncodeWorkMem = (INT8U *)gp_malloc(size);
	if(!pAudio_Encode_Para->EncodeWorkMem)
		return AUD_RECORD_MEMORY_ALLOC_ERR;	
	gp_memset((INT8S*)pAudio_Encode_Para->EncodeWorkMem, 0, size);
	pAudio_Encode_Para->Bit_Stream_Buffer = (INT8U *)gp_malloc_align(C_BS_BUFFER_SIZE, 4);
	if(!pAudio_Encode_Para->Bit_Stream_Buffer)
		return AUD_RECORD_MEMORY_ALLOC_ERR;
		
	nRet = mp3enc_init(	pAudio_Encode_Para->EncodeWorkMem, 
						pAudio_Encode_Para->Channel, 
						pAudio_Encode_Para->SampleRate,
						(pAudio_Encode_Para->BitRate/1000), 
						0, //copyright
						(CHAR *)pAudio_Encode_Para->Bit_Stream_Buffer,
						C_BS_BUFFER_SIZE, 
						0); //RingWI, for ID3 header skip. 
	if(nRet<0)
		return AUD_RECORD_INIT_ERR;
	
	pAudio_Encode_Para->read_index = 0;
	pAudio_Encode_Para->FileLenth = 0;				
	pAudio_Encode_Para->PackSize = 0;
	pAudio_Encode_Para->pack_buffer = NULL;
	size = nRet * MP3_TIME * pAudio_Encode_Para->Channel;
	pAudio_Encode_Para->PCMInFrameSize = size;
	size = nRet*pAudio_Encode_Para->Channel;
	pAudio_Encode_Para->OnePCMFrameSize = size; 
			
	return AUD_RECORD_STATUS_OK;
}

//===============================================================================================
// 	MP3_encode_stop 
//	parameter:	none.
//	return:		status.
//===============================================================================================
static INT32S mp3_encode_stop(void)
{
	INT32S nRet;
	INT32U old_index;
	
	g_switch_flag = 0;
	old_index = pAudio_Encode_Para->read_index;
	nRet = mp3enc_end((void *)pAudio_Encode_Para->EncodeWorkMem);
	if(nRet> 0)
	{
		pAudio_Encode_Para->read_index = nRet;
		nRet = pAudio_Encode_Para->read_index - old_index;
		if(nRet <0)
			nRet += C_BS_BUFFER_SIZE;
		
		pAudio_Encode_Para->FileLenth += nRet;	
		nRet = save_buffer_to_storage();
		if(nRet < 0)
		{
			adc_work_mem_free();
			return AUD_RECORD_FILE_WRITE_ERR;
		}
	}
	
	nRet = save_final_data_to_storage();
	if(nRet < 0)
	{
		adc_work_mem_free();
		return AUD_RECORD_FILE_WRITE_ERR;
	}
	
	close(pAudio_Encode_Para->FileHandle);
	
	//free memory
	adc_work_mem_free();
	return  AUD_RECORD_STATUS_OK;
}

//===============================================================================================
// 	mp3_encode_once 
//	parameter:	workmem = work memory
//				buffer_addr = buffer address
//				cwlen = buffer size in word
//	return:		length in word
//===============================================================================================
static int mp3_encode_once(void *workmem, const short* buffer_addr, int cwlen)
{
	INT32U N, old_index;
	INT32S nRet, cblen;
	
	cblen = 0;
	old_index = pAudio_Encode_Para->read_index;
	N = cwlen;
	
	while(N >= pAudio_Encode_Para->OnePCMFrameSize)
	{
		nRet =  mp3enc_encframe((void *)pAudio_Encode_Para->EncodeWorkMem, (short *)buffer_addr);
		if(nRet<0)	
			return AUD_RECORD_RUN_ERR;
		
		pAudio_Encode_Para->read_index = nRet;
		buffer_addr += pAudio_Encode_Para->OnePCMFrameSize;
		N -= pAudio_Encode_Para->OnePCMFrameSize;
	}
		
	cblen = pAudio_Encode_Para->read_index - old_index;
	if(cblen <0)
		cblen += C_BS_BUFFER_SIZE;
		
	pAudio_Encode_Para->FileLenth += cblen;
	if (pAudio_Encode_Para->FileLenth > AUD_FILE_MAX_RECORD_SIZE && !g_switch_flag) {
		g_switch_flag = 1;
		msgQSend(ApQ, MSG_APQ_RECORD_SWITCH_FILE, NULL, NULL, MSG_PRI_NORMAL);
	}
	/*
	if (pAudio_Encode_Para->FileLenth > pAudio_Encode_Para->disk_free_size) {
		INT32U led_type;
				
		led_type = LED_WAITING_CAPTURE;
		msgQSend(PeripheralTaskQ, MSG_PERIPHERAL_TASK_LED_FLASH_SET, &led_type, sizeof(INT32U), MSG_PRI_NORMAL);
	} */
	nRet = save_buffer_to_storage();
	if(nRet < 0)
		return AUD_RECORD_FILE_WRITE_ERR;
		
	return cwlen;
}
#endif //APP_MP3_ENCODE_EN
