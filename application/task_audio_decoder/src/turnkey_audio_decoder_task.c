#include "turnkey_audio_decoder_task.h"
#include "ap_music.h"

// Constant definitions used in this file only go here
#define AUDIO_QUEUE_MAX  64
#define AUDIO_FS_Q_SIZE  1
#define AUDIO_WRITE_Q_SIZE MAX_DAC_BUFFERS
#define AUDIO_PARA_MAX_LEN  sizeof(STAudioTaskPara)
#define RAMP_DOWN_STEP 4
#define RAMP_DOWN_STEP_HOLD 4
#define RAMP_DOWN_STEP_LOW_SR	4*16
#define USE_RAMP_DOWN_RAPID		0
#define SKIP_ID3_TAG            1

#define READING_BY_FILE_SRV  1
#define PERFORMANCE_MEASURE  0

#if PERFORMANCE_MEASURE == 1
#define R_TIMERC_CTRL				(*((volatile INT32U *) 0xC0020040))
#define R_TIMERC_CCP_CTRL			(*((volatile INT32U *) 0xC0020044))
#define R_TIMERC_PRELOAD			(*((volatile INT32U *) 0xC0020048))
#define R_TIMERC_CCP_REG			(*((volatile INT32U *) 0xC002004C))
#define R_TIMERC_UPCOUNT			(*((volatile INT32U *) 0xC0020050))

INT32S  cnt_max[3] = {0};
INT32U  cnt_max_fn[3] = {0};
INT32U  point[3] = {0};
INT32S  times;
INT32S  times0;
INT32U  timer_cnt1 = 0;
INT32U  timer_cnt2 = 0;
INT32U  total_cnt = 0;
INT32U  total_frame = 0;
INT32U  total_point = 0;

#endif

/* Task Q declare */
MSG_Q_ID AudioTaskQ = NULL;

OS_EVENT	*audio_wq;
void		*write_q[AUDIO_WRITE_Q_SIZE];
OS_EVENT	*audio_fsq;
void		*fs_q[AUDIO_FS_Q_SIZE];
__align(4) INT8U audio_para[AUDIO_PARA_MAX_LEN];

INT16S          *pcm_out[MAX_DAC_BUFFERS] = {NULL};
INT32U          pcm_len[MAX_DAC_BUFFERS];

AUDIO_CONTEXT   audio_context;
AUDIO_CONTEXT_P audio_context_p = &audio_context;
AUDIO_CTRL      audio_ctrl;
STAudioConfirm  aud_con;
INT32U pcm_buf;

static struct sfn_info aud_sfn_file;
INT8U  stopped;
INT32U dac_buf_nums;
INT8U	FB_seek,MP3_Speed;
INT32U FrameSize_ave;

extern ST_AUDIO_PLAY_TIME aud_time;

void	(*decode_end)(INT32U audio_decoder_num);	// added by Bruce, 2008/10/03
INT32S	(*audio_move_data)(INT32U buf_addr, INT32U buf_size);	// added by Bruce, 2008/10/27
INT32U  g_audio_data_length;	// added by Bruce, 2008/10/31
INT8U   channel;
INT32U   g_audio_sample_rate;	//20090209 roy
//static 	INT32S fg_error_cnt;

#if (defined AUDIO_PROGRESS_SUPPORT) && (AUDIO_PROGRESS_SUPPORT == CUSTOM_ON)
INT32U total_play_time;
INT8U  start_time_send;
#endif

/* Proto types */
void audio_task_init(void);
void audio_task_entry(void *p_arg);
void audio_work_ringbuffer_clear(void);

static void    audio_init(void);

#if APP_WAV_CODEC_FG_EN == 1
static INT32S  audio_wav_dec_play_init(void);
static INT32S  audio_wav_dec_process(void);
#endif

#if APP_MP3_DECODE_FG_EN == 1
static INT32S  audio_mp3_play_init(void);
static INT32S  audio_mp3_process(void);
#endif

#if APP_A1800_DECODE_FG_EN == 1
static INT32S  audio_a1800_play_init(void);//080724
static INT32S  audio_a1800_process(void);//080724
#endif

#if APP_WMA_DECODE_FG_EN == 1
static INT32S  audio_wma_play_init(void);
static INT32S  audio_wma_process(void);
#endif

// added by Bruce, 2008/09/23
#if APP_A1600_DECODE_FG_EN == 1
static INT32S  audio_a16_play_init(void);
static INT32S  audio_a16_process(void);
#endif

#if APP_A6400_DECODE_FG_EN == 1
static INT32S  audio_a64_play_init(void);
static INT32S  audio_a64_process(void);
#endif

#if APP_S880_DECODE_FG_EN == 1
static INT32S  audio_s880_play_init(void);
static INT32S  audio_s880_process(void);
#endif
// added by Bruce, 2008/09/23

//static INT32U  audio_write_buffer(INT16S fd, INT8U *ring_buf, INT32U wi, INT32U ri);


#if APP_MP3_DECODE_FG_EN == 1 || APP_WMA_DECODE_FG_EN == 1 || APP_WAV_CODEC_FG_EN == 1 || APP_A1600_DECODE_FG_EN == 1 || APP_A1800_DECODE_FG_EN == 1 || APP_A6400_DECODE_FG_EN == 1 || APP_S880_DECODE_FG_EN == 1
#if READING_BY_FILE_SRV == 1
static INT32S  audio_write_with_file_srv(INT8U *ring_buf, INT32U wi, INT32U ri);
static INT32S  audio_check_wi(INT32S wi_in, INT32U *wi_out, INT8U wait);
#else
static INT32S  audio_write_buffer(INT8U *ring_buf, INT32U wi, INT32U ri);
#endif
static INT32S  audio_send_to_dma(void);
static INT32S  audio_q_check(void);
#endif

static void    audio_queue_clear(void);
static void    audio_stop_unfinished(void);
static void    audio_send_next_frame_q(void);
//static void    audio_ramp_down(void);
static void    audio_start(STAudioTaskPara *pAudioTaskPara);
static void    audio_pause(STAudioTaskPara *pAudioTaskPara);
static void    audio_resume(STAudioTaskPara *pAudioTaskPara);
static void    audio_stop(STAudioTaskPara *pAudioTaskPara);
static void    audio_decode_next_frame(STAudioTaskPara *pAudioTaskPara);
static void    audio_mute_set(STAudioTaskPara *pAudioTaskPara);
static void    audio_volume_set(STAudioTaskPara *pAudioTaskPara);
static void    audio_reverse_set(STAudioTaskPara *pAudioTaskPara);
static void    audio_play_speed_set(STAudioTaskPara *pAudioTaskPara);

#if (defined AUDIO_FORMAT_JUDGE_AUTO) && (AUDIO_FORMAT_JUDGE_AUTO == 1)
static INT32S audio_get_type(INT16S fd,INT8S* file_name);
#else
static INT32S  audio_get_type(INT8S* file_name);
#endif

#if USE_RAMP_DOWN_RAPID == 1
static void audio_ramp_down_rapid(void);
#endif

#if (defined SKIP_ID3_TAG) && (SKIP_ID3_TAG == 1)
static INT8U audio_get_id3_type(INT8U *data, INT32U length);
static void audio_parse_id3_header(INT8U *header, INT32U *version, INT32S *flags, INT32U *size);
static INT32U audio_id3_get_size(INT8U *ptr);
INT32S audio_id3_get_tag_len(INT8U *data, INT32U length);
#endif

static INT8U reverse_play;
static INT8U reverse_play_start;
static INT8U bit_per_samp;
static INT32U reverse_points;
static INT32U dac_volume;

void audio_task_init(void)
{
    /* Create MsgQueue/MsgBox for TASK */
	if (AudioTaskQ==NULL) {
    		AudioTaskQ = msgQCreate(AUDIO_QUEUE_MAX, AUDIO_QUEUE_MAX, AUDIO_PARA_MAX_LEN);
	}
    	audio_wq = OSQCreate(write_q, AUDIO_WRITE_Q_SIZE);
	audio_fsq = OSQCreate(fs_q, AUDIO_FS_Q_SIZE);

}

void audio_task_entry(void *p_arg)
{
    INT32U  msg_id;
	STAudioTaskPara     *pstAudioTaskPara;


	//audio_task_init();
	if (AudioTaskQ==NULL) {
    		AudioTaskQ = msgQCreate(AUDIO_QUEUE_MAX, AUDIO_QUEUE_MAX, AUDIO_PARA_MAX_LEN);
	}
	audio_init();

	while (1)
	{
	    /* Pend task message */
	    msgQReceive(AudioTaskQ, &msg_id, (void*)audio_para, AUDIO_PARA_MAX_LEN);
	    pstAudioTaskPara = (STAudioTaskPara*) audio_para;

		switch(msg_id) {
			case MSG_AUD_PLAY: /* by file handle */
			case MSG_AUD_PLAY_BY_SPI:
				gpio_write_io(SPEAKER_EN, 1);
				OSTimeDly(10);
				audio_start(pstAudioTaskPara);
				break;
			case MSG_AUD_STOP:
				gpio_write_io(SPEAKER_EN, 0);
				audio_stop(pstAudioTaskPara);
				break;
			case MSG_AUD_PAUSE:
				audio_pause(pstAudioTaskPara);
				break;
			case MSG_AUD_RESUME:
				audio_resume(pstAudioTaskPara);
				break;
			case MSG_AUD_SET_MUTE:
				audio_mute_set(pstAudioTaskPara);
				break;
			case MSG_AUD_REVERSE_SET:
			    audio_reverse_set(pstAudioTaskPara);
			    break;
			case MSG_AUD_PLAY_SPEED_SET:
			    audio_play_speed_set(pstAudioTaskPara);
			    break;
			case MSG_AUD_VOLUME_SET:
				audio_volume_set(pstAudioTaskPara);
				break;
			case MSG_AUD_DECODE_NEXT_FRAME:
				audio_decode_next_frame(pstAudioTaskPara);
				break;
		  #if (defined AUDIO_PROGRESS_SUPPORT) && (AUDIO_PROGRESS_SUPPORT == CUSTOM_ON)
			case MSG_AUD_PLAY_TIME_GET_START:
			    start_time_send = 1;
			    break;
			case MSG_AUD_PLAY_TIME_GET_END:
			    start_time_send = 0;
			    break;
		  #endif
			default:
				break;
		}
	}
}

#define GUARD_BUFFER (5*1024)  // 修正從 SDRAM 播聲音 FileSrvLDWAudioAppRead 可能產生的錯誤

INT32U audio_dec_buf[(RING_BUF_SIZE + GUARD_BUFFER + WAV_PCM_BUF_SIZE*MAX_DAC_BUFFERS + WAV_DEC_MEMORY_SIZE)/4 + 2];
static void audio_init(void)
{
	INT32U ptr;
	
	ptr = (INT32U) &audio_dec_buf[0];
	audio_ctrl.ring_buf = (INT8U *) ptr;
	
	ptr += (RING_BUF_SIZE + GUARD_BUFFER + 3) & ~0x3;
	pcm_buf = ptr;
	
	ptr += (WAV_PCM_BUF_SIZE*MAX_DAC_BUFFERS + 3) & ~0x3;
	audio_ctrl.work_mem = (INT8S *) ptr;
/*	
	audio_ctrl.ring_buf = (INT8U*) gp_malloc(RING_BUF_SIZE);
	if (audio_ctrl.ring_buf == NULL) {
		DBG_PRINT("allocate ring_buf fail\r\n");
	}
	
	pcm_buf = (INT32U) gp_malloc(MP3_PCM_BUF_SIZE*2*MAX_DAC_BUFFERS);
	if (pcm_buf == NULL) {
		DBG_PRINT("allocate pcm buffer fail\r\n");
	}
	
	audio_ctrl.work_mem = (INT8S*) gp_malloc(MP3_DEC_MEMORY_SIZE);
	if (audio_ctrl.work_mem == NULL) {
		DBG_PRINT("allocate working buffer fail\r\n");
	}
*/
	DBG_PRINT("audio memory = %d\r\n", sizeof(audio_dec_buf));
	gp_memset(audio_ctrl.work_mem,0,WAV_DEC_MEMORY_SIZE);
	DBG_PRINT("decode memory -- 0x%x\r\n",audio_ctrl.work_mem);
	
	audio_ctrl.ring_size = RING_BUF_SIZE;
	audio_ctrl.wi = 0;
	audio_ctrl.ri = 0;
	audio_context_p->state = AUDIO_PLAY_STOP;
	stopped = 1;

	decode_end = NULL;
}

static void audio_start(STAudioTaskPara *pAudioTaskPara)
{
	INT32S ret;

	if (audio_context_p->state != AUDIO_PLAY_STOP) {
		audio_stop_unfinished();
	}

	stopped = 1;
	audio_queue_clear();

	audio_context_p->source_type = pAudioTaskPara->src_type;

	if (audio_context_p->source_type == AUDIO_SRC_TYPE_FS) {
		ret = audio_play_file_set(pAudioTaskPara->fd, audio_context_p, &audio_ctrl);
	}
	else if (audio_context_p->source_type == AUDIO_SRC_TYPE_USER_DEFINE)	// added by Bruce, 2008/10/27
	{
		//ret = audio_play_file_set(pAudioTaskPara->fd, audio_context_p, &audio_ctrl);

		// modified by Bruce, 2008/10/30
		audio_context_p->audio_format = pAudioTaskPara->audio_format;
	   	if (audio_context_p->audio_format == AUDIO_TYPE_NONE)
	   	{
		   DBG_PRINT("audio_play_file_set find not support audio.\r\n");
	   	}
		audio_ctrl.file_len = g_audio_data_length;
		ret = AUDIO_ERR_NONE;
	}
	else {
		ret = AUDIO_ERR_NONE;
		audio_context_p->audio_format = pAudioTaskPara->audio_format;
		audio_ctrl.file_len = pAudioTaskPara->file_len;
		audio_ctrl.file_handle = pAudioTaskPara ->fd;
	}

	if (ret == AUDIO_ERR_NONE) {
		audio_context_p->state = AUDIO_PLAYING;
		audio_ctrl.ring_size = RING_BUF_SIZE;
		audio_ctrl.reading = 0;
		dac_buf_nums = MAX_DAC_BUFFERS;
		
		switch(audio_context_p->audio_format) {
		#if APP_MP3_DECODE_FG_EN == 1
			case AUDIO_TYPE_MP3:
				audio_context_p->fp_deocde_init = audio_mp3_play_init;
				audio_context_p->fp_deocde = audio_mp3_process;
				audio_ctrl.ring_size = MP3_RING_BUF_SIZE; /* change ring size */
				audio_ctrl.frame_size = MP3_DEC_FRAMESIZE;
				break;
		#endif
		#if APP_WAV_CODEC_FG_EN == 1
			case AUDIO_TYPE_WAV:
				audio_context_p->fp_deocde_init = audio_wav_dec_play_init;
				audio_context_p->fp_deocde = audio_wav_dec_process;
				audio_ctrl.ring_size = WAV_DEC_BITSTREAM_BUFFER_SIZE;
				audio_ctrl.frame_size = WAV_DEC_FRAMESIZE;
				dac_buf_nums = 4;
				reverse_play = 0;
                reverse_play_start = 0;
                reverse_points = 0;
                OSQPost(hAudioDacTaskQ,(void *) MSG_AUD_DAC_REVRESE_CLEAR);
				break;
		#endif
		#if APP_WMA_DECODE_FG_EN == 1
			case AUDIO_TYPE_WMA:
				audio_context_p->fp_deocde_init = audio_wma_play_init;
				audio_context_p->fp_deocde = audio_wma_process;
				audio_ctrl.ring_size = RING_BUF_SIZE; /* change ring size */
				audio_ctrl.frame_size = WMA_DEC_FRAMESIZE;
				break;
		#endif
		#if APP_A1800_DECODE_FG_EN == 1
			case AUDIO_TYPE_A1800://080722
				audio_context_p->fp_deocde_init = audio_a1800_play_init;
				audio_context_p->fp_deocde = audio_a1800_process;
				audio_ctrl.frame_size = A18_DEC_FRAMESIZE;
				break;
		#endif
		#if APP_A1600_DECODE_FG_EN == 1
			case AUDIO_TYPE_A1600:			// added by Bruce, 2008/09/23
				audio_context_p->fp_deocde_init = audio_a16_play_init;
				audio_context_p->fp_deocde = audio_a16_process;
				audio_ctrl.frame_size = A16_DEC_FRAMESIZE;
				break;
		#endif
		#if APP_A6400_DECODE_FG_EN == 1
			case AUDIO_TYPE_A6400:			// added by Bruce, 2008/09/25
				audio_context_p->fp_deocde_init = audio_a64_play_init;
				audio_context_p->fp_deocde = audio_a64_process;
				audio_ctrl.frame_size = A6400_DEC_FRAMESIZE;
				break;
		#endif
		#if APP_S880_DECODE_FG_EN == 1
			case AUDIO_TYPE_S880:			// added by Bruce, 2008/09/25
				audio_context_p->fp_deocde_init = audio_s880_play_init;
				audio_context_p->fp_deocde = audio_s880_process;
				audio_ctrl.frame_size = S880_DEC_FRAMESIZE;
				break;
		#endif
			default:
				audio_send_result(MSG_AUD_BG_PLAY_RES,AUDIO_ERR_INVALID_FORMAT);
				return;
		}
		if (audio_mem_alloc(audio_context_p->audio_format,&audio_ctrl,pcm_out) != AUDIO_ERR_NONE) {
			DBG_PRINT("audio memory allocate fail\r\n");
			audio_send_result(MSG_AUD_PLAY_RES,AUDIO_ERR_MEM_ALLOC_FAIL);
			return;
		}
		//dac_enable_set(TRUE); /* enable dac */
		ret = audio_context_p->fp_deocde_init();
		if (ret != AUDIO_ERR_NONE) {
			audio_stop_unfinished();
			DBG_PRINT("audio play init failed\r\n");
        }
        else {
          #if (defined AUDIO_PROGRESS_SUPPORT) && (AUDIO_PROGRESS_SUPPORT == CUSTOM_ON)
          	OSQPost(hAudioDacTaskQ,(void *) MSG_AUD_START_SAMPLES_COUNT);
          #endif
        	audio_send_next_frame_q();
		}
	}
	else {
   		ret = AUDIO_ERR_INVALID_FORMAT;
	}

	audio_send_result(MSG_AUD_PLAY_RES,ret);
}

static void audio_stop(STAudioTaskPara *pAudioTaskPara)
{
	if ((audio_context_p->state == AUDIO_PLAY_STOP)||(audio_context_p->state == AUDIO_IDLE)) {
		audio_send_result(MSG_AUD_STOP_RES,AUDIO_ERR_NONE);
		return;
	}
	
  #if AUDIO_PRIOR_DYNAMIC_SW == 1
	OSQPost(hAudioDacTaskQ,(void *) MSG_AUD_DAC_STOP);
  #endif

	if (audio_context_p->state == AUDIO_PLAYING) {
#if USE_RAMP_DOWN_RAPID == 0
		//audio_ramp_down();		//ramp down speed normal
#else
		if(audio_context_p->audio_format == AUDIO_TYPE_AVI)
			audio_ramp_down();	//ramp down speed normal
		else
			audio_ramp_down_rapid();
#endif
	}

	audio_stop_unfinished();
	audio_send_result(MSG_AUD_STOP_RES,AUDIO_ERR_NONE);
}

#if 0
static void audio_ramp_down(void)
{
	INT8U   wb_idx;
	INT8U   err, ramp_down_step;
	INT16S  *ptr;
	INT16S  last_ldata,last_rdata;
	INT32U  i,j;
	
	if (stopped) {
		return;
	}

	if(g_audio_sample_rate > 16000)
		ramp_down_step = RAMP_DOWN_STEP;
	else
		ramp_down_step = RAMP_DOWN_STEP_LOW_SR;

	OSQFlush(aud_send_q);

	last_ldata = *(pcm_out[last_send_idx] + pcm_len[last_send_idx]-2);
	last_rdata = *(pcm_out[last_send_idx] + pcm_len[last_send_idx]-1);

	last_ldata = last_ldata & ~(ramp_down_step-1);
	if (channel == 2) {
		last_rdata = last_rdata & ~(ramp_down_step-1);
	}
	else {
		last_rdata = 0;
	}

	DBG_PRINT("ldata = 0x%x\r\n",last_ldata);
	DBG_PRINT("rdata = 0x%x\r\n",last_rdata);
	
	while(1) {
		wb_idx = (INT32U) OSQPend(audio_wq, 0, &err);
		ptr = (INT16S*) pcm_out[wb_idx];

		for (i=0;i<audio_ctrl.frame_size/RAMP_DOWN_STEP_HOLD;i++) {
			for (j=0;j<RAMP_DOWN_STEP_HOLD;j++) {
				*ptr++ = last_ldata;
				if (channel == 2) {
					*ptr++ = last_rdata;
		    	}
	    	}

			if (last_ldata > 0x0) {
				last_ldata -= ramp_down_step;
			}
			else if (last_ldata < 0x0) {
				last_ldata += ramp_down_step;
			}

			if (channel == 2) {
				if (last_rdata > 0x0) {
					last_rdata -= ramp_down_step;
		        }
				else if (last_rdata < 0x0) {
					last_rdata += ramp_down_step;
		        }
		    }
	    }

		pcm_len[wb_idx] = audio_ctrl.frame_size*channel;

		OSQPost(aud_send_q, (void *) wb_idx);
		if (dac_dma_status_get() == 0) {
			OSQPost(hAudioDacTaskQ,(void *) MSG_AUD_DMA_DBF_RESTART);
		}

		if ((last_ldata == 0x0) && (last_rdata == 0x0)) {
			break;
		}
	}
}
#endif

#if USE_RAMP_DOWN_RAPID == 1
static void audio_ramp_down_rapid(void)
{
	INT16S  last_ldata,last_rdata;
	INT16S  i, temp;

	OSQFlush(aud_send_q);
	while(dac_dma_status_get() == 1) {
		OSTimeDly(2);
	}
	//free and reset DMA channel
	dac_dbf_free();

	temp = 0 - RAMP_DOWN_STEP;
	last_ldata = R_DAC_CHA_DATA;
	last_rdata = R_DAC_CHB_DATA;
	//unsigned to signed
	last_ldata ^= 0x8000;
	last_rdata ^= 0x8000;
	//change timer to 44100
	dac_sample_rate_set(44100);

	while(1)
	{
		if (last_ldata > 0x0) {
			last_ldata -= RAMP_DOWN_STEP;
		}
		else if (last_ldata < 0x0) {
			last_ldata += RAMP_DOWN_STEP;
		}

		if ((last_ldata < RAMP_DOWN_STEP)&&(last_ldata > temp)) {
			last_ldata = 0;
		}

		if (channel == 2) {
			if (last_rdata > 0x0) {
				last_rdata -= RAMP_DOWN_STEP;
		    }
			else if (last_rdata < 0x0) {
				last_rdata += RAMP_DOWN_STEP;
		    }

		    if ((last_rdata < RAMP_DOWN_STEP)&&(last_rdata > temp)) {
				last_rdata = 0;
			}
		}

		for(i=0;i<RAMP_DOWN_STEP_HOLD;i++) {
			if (channel == 2){
				while(R_DAC_CHA_FIFO & 0x8000);
				R_DAC_CHA_DATA = last_ldata;
				while(R_DAC_CHB_FIFO & 0x8000);
				R_DAC_CHB_DATA = last_rdata;
			} else {
				while(R_DAC_CHA_FIFO & 0x8000);
				R_DAC_CHA_DATA = last_ldata;
			}
		}

		if ((last_ldata == 0x0) && (last_rdata == 0x0)) {
			break;
		}
	}
}
#endif //USE_RAMP_DOWN_RAPID
/*== AVI audio ctrl Setting explain ==*/
// audio_ctrl.f_last => 1:it mean buffer read file over. 0: it mean buffer read file continue
// when audio play over ,the audio_ctrl.ri == audio_ctrl.wi.
/*== ==*/

static void audio_pause(STAudioTaskPara *pAudioTaskPara)
{
	if (audio_context_p->state != AUDIO_PLAYING) {
		return;
	}

	OSQPost(hAudioDacTaskQ, (void *)MSG_AUD_DMA_PAUSE);

	while(dac_dma_status_get() == 1) {
		OSTimeDly(2);
	}

	stopped = 1;
	dac_dbf_free();
	//dac_enable_set(FALSE);
  #if AUDIO_PRIOR_DYNAMIC_SW == 1
	OSQPost(hAudioDacTaskQ,(void *) MSG_AUD_LOW_PRIORITY_SET);
  #endif
	audio_context_p->state = AUDIO_PLAY_PAUSE;
	audio_send_result(MSG_AUD_PAUSE_RES, AUDIO_ERR_NONE);
}

static void audio_resume(STAudioTaskPara *pAudioTaskPara)
{
	if (audio_context_p->state != AUDIO_PLAY_PAUSE) {
		return;
	}
	
	if (channel == 1) {
		dac_mono_set();
	}
	else {
		dac_stereo_set();
	}
	if(pAudioTaskPara->audio_format != AUDIO_TYPE_WAV){
		dac_sample_rate_set(mp3_dec_get_samplerate((CHAR*)audio_ctrl.work_mem));
	}
	//dac_enable_set(TRUE);
	audio_context_p->state = AUDIO_PLAYING;
	#if READING_BY_FILE_SRV == 1
	if (audio_ctrl.reading) {
		audio_check_wi(AUDIO_READ_WAIT, &audio_ctrl.wi, 1);
	}
	#endif
	audio_send_next_frame_q();
	audio_send_result(MSG_AUD_RESUME_RES,AUDIO_ERR_NONE);
}

static void audio_mute_set(STAudioTaskPara *pAudioTaskPara)
{
	if (pAudioTaskPara->mute == TRUE) {
		dac_pga_set(0);
		//dac_vref_set(FALSE);
	}
	else {
		dac_pga_set(pAudioTaskPara->volume);
		//dac_vref_set(TRUE);
	}
	audio_send_result(MSG_AUD_MUTE_SET_RES,AUDIO_ERR_NONE);
}

static void audio_reverse_set(STAudioTaskPara *pAudioTaskPara)
{
	INT32S len;
    if (audio_context_p->state != AUDIO_PLAYING) {
		return;
	}
	
    if (pAudioTaskPara->reverse == 1) {
        reverse_play = 1;
	} else {
	    reverse_play = 0;
	} 
	
	if(pAudioTaskPara->audio_format == AUDIO_TYPE_MP3)
	{
	    if(audio_ctrl.wi!= audio_ctrl.ri)
	    {
	    	len = audio_ctrl.wi - audio_ctrl.ri;
		    if (len < 0) {
		    	len += audio_ctrl.ring_size;
		    }
		    lseek(audio_ctrl.file_handle, -len, SEEK_CUR);
	 	   	audio_work_ringbuffer_clear();
	    }
	}
}

static void audio_play_speed_set(STAudioTaskPara *pAudioTaskPara)
{
    INT32U speed;
    
	speed = pAudioTaskPara->play_speed;
	// only support 1,2,4 times
	if(pAudioTaskPara->audio_format == AUDIO_TYPE_WAV)
	{
		if (speed == 2 || speed == 4) {
		    dac_pga_set(0);
		} else if (speed == 1) {
		    dac_pga_set(dac_volume);
		} else {
		    return;
		}
		dac_sample_rate_set(g_audio_sample_rate*speed);
	}
	else if(pAudioTaskPara->audio_format == AUDIO_TYPE_MP3)
	{
		if(speed == 1){
			MP3_Speed = 0;
		}
		else if(speed == 2){
			MP3_Speed = 1;
		}
		else if(speed == 4){
			MP3_Speed = 2;
		}
		else{
			MP3_Speed = 0;
		}
	}
}

static void audio_volume_set(STAudioTaskPara *pAudioTaskPara)
{
	if (pAudioTaskPara->volume < 64) {
		dac_pga_set(pAudioTaskPara->volume);
		dac_volume = pAudioTaskPara->volume;
	}
	audio_send_result(MSG_AUD_VOLUME_SET_RES,AUDIO_ERR_NONE);
}

static void audio_decode_next_frame(STAudioTaskPara *pAudioTaskPara)
{
	INT32S ret;
	if (audio_context_p->state != AUDIO_PLAYING) {
		return;
	}
	ret = audio_context_p->fp_deocde();
	if (ret != 0) {
		audio_stop_unfinished();
		audio_send_result(MSG_AUD_PLAY_RES,ret);
		
		#if PERFORMANCE_MEASURE == 1 
		DBG_PRINT("\r\n");
		DBG_PRINT("MAX 1 = %d, frame index = %d, point = %d\r\n",cnt_max[0],cnt_max_fn[0],point[0]);
		DBG_PRINT("MAX 2 = %d, frame index = %d, point = %d\r\n",cnt_max[1],cnt_max_fn[1],point[1]);
		DBG_PRINT("MAX 3 = %d, frame index = %d, point = %d\r\n",cnt_max[2],cnt_max_fn[2],point[2]);
		DBG_PRINT("total count = %d\r\n",total_cnt);	
		DBG_PRINT("total frame = %d\r\n",total_frame);
		DBG_PRINT("ave = %d\r\n",total_cnt/total_frame);
		#endif
		
		if (decode_end != NULL) {
			decode_end(1);	// added by Bruce, 2008/10/03
		}
	}
}

static void audio_send_next_frame_q(void)
{
	msgQSend(AudioTaskQ, MSG_AUD_DECODE_NEXT_FRAME, NULL, 0, MSG_PRI_NORMAL);
}

void audio_send_result(INT32S res_type,INT32S result)
{
	aud_con.result_type = res_type;
	aud_con.result = result;
	aud_con.source_type = audio_context_p->source_type;
	//DBG_PRINT("audio_send_result :res_type =  %x,result = %d\r\n",res_type,result);
	msgQSend(ApQ, EVENT_APQ_ERR_MSG, (void *)&aud_con, sizeof(STAudioConfirm), MSG_PRI_NORMAL);
}

static void audio_stop_unfinished(void)
{
	//INT32S i;
  #if AUDIO_PRIOR_DYNAMIC_SW == 1
	OSQPost(hAudioDacTaskQ,(void *) MSG_AUD_DAC_STOP);
  #endif
	/* wait until dma finish */
	while(dac_dma_status_get() == 1) {
		OSTimeDly(1);
	}
  #if AUDIO_PRIOR_DYNAMIC_SW == 1
	OSQPost(hAudioDacTaskQ,(void *) MSG_AUD_LOW_PRIORITY_SET);
  #endif
  	
	if (audio_context_p->source_type == AUDIO_SRC_TYPE_FS && audio_ctrl.file_handle >= 0) {
		close(audio_ctrl.file_handle);
	} else if (audio_context_p->source_type == AUDIO_SRC_TYPE_APP_RS) {
	    nv_lseek(audio_ctrl.file_handle, 0, SEEK_SET);
	}
	dac_timer_stop();
	dac_dbf_free(); /* release dma channel */
	//dac_enable_set(FALSE);

#if 0
	/* free memory */
    DBG_PRINT("free audio work mem = %x\r\n",audio_ctrl.work_mem);
	gp_free(audio_ctrl.work_mem);
	audio_ctrl.work_mem = NULL;
	
	#if 1
	gp_free(*(pcm_out));
	for (i=0;i<dac_buf_nums;i++) {
		*(pcm_out+i) = NULL;
	}
	#else
	for (i=0;i<dac_buf_nums;i++) {
		gp_free(*(pcm_out+i));
		*(pcm_out+i) = NULL;
	}
	#endif
#endif
	audio_context_p->state = AUDIO_PLAY_STOP;
}

INT32S audio_mem_alloc(INT32U audio_format,AUDIO_CTRL *aud_ctrl, INT16S *pcm[])
{
	INT32S i;
	INT32U pcm_size;
	INT32U wm_size;
	
	switch(audio_format) {
	#if APP_MP3_DECODE_FG_EN == 1 || APP_MP3_DECODE_BG_EN == 1
		case AUDIO_TYPE_MP3:
			#if MP3_DECODE == MP3_DECODE_HW
			wm_size = MP3_DEC_MEMORY_SIZE;
			#else
			wm_size = MP3_DEC_MEMORY_SIZE + MP3_DECODE_HW_RAM;
			#endif
			pcm_size = MP3_PCM_BUF_SIZE;
			break;
	#endif
	#if APP_WMA_DECODE_FG_EN == 1 || APP_WMA_DECODE_BG_EN == 1
		case AUDIO_TYPE_WMA:
			wm_size = WMA_DEC_MEMORY_SIZE;
			pcm_size = WMA_PCM_BUF_SIZE;
			break;
	#endif
	#if APP_WAV_CODEC_FG_EN == 1 || APP_WAV_CODEC_BG_EN == 1
		case AUDIO_TYPE_WAV:
			wm_size = WAV_DEC_MEMORY_SIZE;
			pcm_size = WAV_PCM_BUF_SIZE;
			break;
	#endif
		case AUDIO_TYPE_AVI:
			wm_size = WAV_DEC_MEMORY_SIZE;
			pcm_size = WAV_PCM_BUF_SIZE;
			break;
		case AUDIO_TYPE_AVI_MP3:
			wm_size = MP3_DEC_MEMORY_SIZE;
			pcm_size = MP3_PCM_BUF_SIZE;
			break;
	#if APP_A1800_DECODE_FG_EN == 1 || APP_A1800_DECODE_BG_EN == 1
		case AUDIO_TYPE_A1800://080722
			wm_size = A1800DEC_MEMORY_BLOCK_SIZE;
			pcm_size = A1800_PCM_BUF_SIZE;
			break;
	#endif
	#if APP_A1600_DECODE_FG_EN == 1 || APP_A1600_DECODE_BG_EN == 1
 	  	case AUDIO_TYPE_A1600:			// added by Bruce, 2008/09/23
  	  		wm_size = A16_DEC_MEMORY_SIZE;
  	  		pcm_size = A16_DEC_BITSTREAM_BUFFER_SIZE;
  	  		break;
	#endif
	#if APP_A6400_DECODE_FG_EN == 1 || APP_A6400_DECODE_BG_EN == 1
 	  	case AUDIO_TYPE_A6400:			// added by Bruce, 2008/09/25
  	  		wm_size = A6400_DEC_MEMORY_SIZE;
  	  		pcm_size = A6400_DEC_BITSTREAM_BUFFER_SIZE;
  	  		break;
	#endif
	#if APP_S880_DECODE_FG_EN == 1 || APP_S880_DECODE_BG_EN == 1
 	  	case AUDIO_TYPE_S880:			// added by Bruce, 2008/09/25
  	  		wm_size = S880_DEC_MEMORY_SIZE;
  	  		pcm_size = S880_DEC_BITSTREAM_BUFFER_SIZE;
  	  		break;
	#endif
		default:
			//return AUDIO_ERR_FAILED;
			return AUDIO_ERR_NONE;//0809003
	}

#if 0
	aud_ctrl->work_mem = (INT8S*) gp_malloc(wm_size);
	if (aud_ctrl->work_mem == NULL) {
		return AUDIO_ERR_FAILED;
	}

	gp_memset(aud_ctrl->work_mem,0,wm_size);
	DBG_PRINT("decode memory -- 0x%x\r\n",aud_ctrl->work_mem);
#endif

#if AUDIO_BG_DECODE_EN == 1
	pcm_size += 2 /* add for SPU end data */;
#endif
	
	#if 1
	//pcm_buf = (INT32U) gp_malloc(pcm_size*2*dac_buf_nums);
	//if (pcm_buf == NULL) {
	//	return AUDIO_ERR_FAILED;
	//}
	for (i=0;i<dac_buf_nums;i++) {
		pcm[i] = (INT16S *) (pcm_buf + (i*pcm_size));
		//DBG_PRINT("pcm buffer[%d] = 0x%x (%d)\r\n",i,pcm[i],pcm_size*2);
	} 
	#else
	for (i=0;i<dac_buf_nums;i++) {
		pcm[i] = (INT16S*) gp_malloc(pcm_size*2);
		//DBG_PRINT("pcm buffer[%d] = 0x%x (%d)\r\n",i,pcm[i],pcm_size*2);
		if (pcm[i] == NULL) {
			while(i>0) {
				gp_free(pcm[--i]);
				pcm[i] = NULL;
			}
			return AUDIO_ERR_FAILED;
		}
	}
	#endif
	return AUDIO_ERR_NONE;
}

INT32S audio_play_file_set(INT16S fd, AUDIO_CONTEXT *aud_context, AUDIO_CTRL *aud_ctrl)
{
#if (defined SKIP_ID3_TAG) && (SKIP_ID3_TAG == 1)
	INT8U id3[10];
	INT32U len;
#endif

	sfn_stat(fd,&aud_sfn_file);
#if (defined AUDIO_FORMAT_JUDGE_AUTO) && (AUDIO_FORMAT_JUDGE_AUTO == 1)
	aud_context->audio_format = audio_get_type(fd, aud_sfn_file.f_extname);
#else
	aud_context->audio_format = audio_get_type(aud_sfn_file.f_extname);
#endif
   	if (aud_context->audio_format == AUDIO_TYPE_NONE) {
	   DBG_PRINT("audio_play_file_set find not support audio.\r\n");
   	   return AUDIO_ERR_INVALID_FORMAT;
   	}
	aud_ctrl->file_handle = fd;

#if (defined SKIP_ID3_TAG) && (SKIP_ID3_TAG == 1) /* skip id3 */
    if (aud_context->audio_format == AUDIO_TYPE_MP3) {
    	read(fd,(INT32U)id3,10);
    	len = audio_id3_get_tag_len(id3, 10);
    	aud_ctrl->file_len = aud_sfn_file.f_size-(len&~3);
   	   	lseek(fd,0,SEEK_SET);
   	   	lseek(fd,(len&~3),SEEK_SET); /* 4 byte alignment */
   	}
   	else {
    	aud_ctrl->file_len = aud_sfn_file.f_size;
    }
#else
	aud_ctrl->file_len = aud_sfn_file.f_size;
#endif

	return AUDIO_ERR_NONE;
}

#if (defined AUDIO_FORMAT_JUDGE_AUTO) && (AUDIO_FORMAT_JUDGE_AUTO == 1)
#if APP_WAV_CODEC_FG_EN == 1 || APP_WAV_CODEC_BG_EN == 1 || APP_MP3_DECODE_FG_EN == 1 || APP_MP3_DECODE_BG_EN == 1 || APP_WMA_DECODE_FG_EN == 1 || APP_WMA_DECODE_BG_EN == 1
static  INT8S parse_mp3_file_head(INT8S *p_audio_file_head_ptr)
{
	INT8S  mpeg_version, layer,sample_rate;
	INT8S j =0;
	INT32U ID3V2_length = 0;
	INT32U cnt  = AUDIO_PASER_BUFFER_SIZE;
	MP3_FILE_HEAD  mp3_file_head[2];
	INT8S *p_audio_file_head;

	p_audio_file_head = p_audio_file_head_ptr;

	if(*(p_audio_file_head) == (INT8S)'I' && *(p_audio_file_head + 1) == (INT8S)'D' && *(p_audio_file_head + 2) == (INT8S)'3' )
	{
		ID3V2_length = 10 + ((*(p_audio_file_head + 9)& 0x7f)|((*(p_audio_file_head + 8) & 0x7f)<<7)|((*(p_audio_file_head + 7) & 0x7f)<<14)|((*(p_audio_file_head + 6) & 0x7f)<<21));
		//ID3V2_length += 10 + ((*(p_audio_file_head + 6) & 0x7f)<<21);
		//ID3V2_length += ((*(p_audio_file_head + 7) & 0x7f)<<14);
		//ID3V2_length += ((*(p_audio_file_head + 8) & 0x7f)<<7);
		//ID3V2_length +=  (*(p_audio_file_head + 9)& 0x7f);

		/*if(ID3V2_length > 1024)
		{
			DBG_PRINT("audio file ID3V2 too long \r\n");
			return AUDIO_PARSE_SUCCS ;
		}
		else
		{
			p_audio_file_head +=  ID3V2_length;
			cnt -= ID3V2_length;
		}
		*/
		DBG_PRINT("audio file parse succes \r\n");
		return AUDIO_PARSE_SUCCS ;
	}

	while (cnt > 4)
	{
		if((*(p_audio_file_head) == 0xFF ) && ((*(p_audio_file_head + 1)&0xE0) == 0xE0 )  && ((*(p_audio_file_head + 2)&0xF0 )!= 0xF0 ))  // first 11 bits should be 1
		{

			mpeg_version =( *(p_audio_file_head + 1)&0x18)>>3;
			layer = (*(p_audio_file_head + 1)&0x06)>>1;
			sample_rate = (*(p_audio_file_head + 2)&0x0c)>>2;

			if((mpeg_version != 0x01) && (layer != 0x00) && (layer != 0x03) && (sample_rate != 0x03))   // != RESERVERD
			{
				if(j<2)
				{
					mp3_file_head[j].mpeg_version = mpeg_version;
					mp3_file_head[j].layer = layer;
					mp3_file_head[j].sample_rate = sample_rate;

					j++;
				}
				else if ((mp3_file_head[0].mpeg_version == mp3_file_head[1].mpeg_version) && (mp3_file_head[0].layer == mp3_file_head[1].layer) && (mp3_file_head[0].sample_rate == mp3_file_head[1].sample_rate))
				{
					DBG_PRINT("audio file parse succes \r\n");
					return  AUDIO_PARSE_SUCCS ;
				}

			}
			else
			{
				p_audio_file_head +=  4;
				cnt -= 4 ;
			}

		}
		else
		{
			p_audio_file_head +=  4;
			cnt -= 4 ;

		}

	}

	return AUDIO_PARSE_FAIL ;

}

static  INT8S parse_wma_file_head(INT8S *p_audio_file_head)
{


	if( *(p_audio_file_head) != 0x30 ||*(p_audio_file_head + 1) != 0x26 ||*(p_audio_file_head + 2) != 0xB2 ||*(p_audio_file_head + 3) != 0x75  )
	{
		return AUDIO_PARSE_FAIL ;
	}
	else if( *(p_audio_file_head + 4) != 0x8E  ||*(p_audio_file_head + 5) != 0x66 ||*(p_audio_file_head + 6) != 0xCF ||*(p_audio_file_head + 7) != 0x11 )
	{
		return AUDIO_PARSE_FAIL ;
	}
	else if( *(p_audio_file_head + 8) != 0xA6  ||*(p_audio_file_head + 9) != 0xD9 ||*(p_audio_file_head + 10) != 0x00 ||*(p_audio_file_head + 11) != 0xAA )
	{
		return AUDIO_PARSE_FAIL ;
	}
	else if(*(p_audio_file_head + 12) != 0x00 ||*(p_audio_file_head + 13) != 0x62 ||*(p_audio_file_head + 14) != 0xCE ||*(p_audio_file_head + 15) != 0x6C )
	{
		return AUDIO_PARSE_FAIL ;
	}
	else
	{
		return AUDIO_PARSE_SUCCS ;
	}

}

static  INT8S parse_wav_file_head(INT8S *p_audio_file_head)
{
	INT16S wav_format_type;

	if( *(p_audio_file_head) != (INT8S)'R' || *(p_audio_file_head + 1) != (INT8S)'I' || *(p_audio_file_head + 2) != (INT8S)'F' || *(p_audio_file_head + 3) != (INT8S)'F' )
	{
		return AUDIO_PARSE_FAIL ;
	}
	else if( *(p_audio_file_head + 8) != (INT8S)'W' || *(p_audio_file_head + 9) !=(INT8S)'A' || *(p_audio_file_head + 10) != (INT8S)'V'|| *(p_audio_file_head + 11) != (INT8S)'E')
	{
		return AUDIO_PARSE_FAIL ;
	}
	else if(*(p_audio_file_head + 12) != (INT8S)'f' || *(p_audio_file_head + 13) != (INT8S)'m' || *(p_audio_file_head + 14) != (INT8S)'t' || *(p_audio_file_head + 15) != 0x20 )
	{
		return AUDIO_PARSE_FAIL ;
	}
	else
	{		//add for mantis #9116  090612 by Lion
		wav_format_type = (INT16S)((*(p_audio_file_head + 21))<<8);
		wav_format_type += *(p_audio_file_head + 20);

		if( wav_format_type == WAVE_FORMAT_PCM || wav_format_type == WAVE_FORMAT_ADPCM || wav_format_type == WAVE_FORMAT_ALAW ||wav_format_type == WAVE_FORMAT_MULAW ||wav_format_type == WAVE_FORMAT_IMA_ADPCM )
		{

			return AUDIO_PARSE_SUCCS ;
		}
		else        //for example  WAVE_FORMAT_MPEGLAYER3
		{
			return AUDIO_PARSE_FAIL ;
		}

	}

	/*
	INT8S  temp[9] = {0};
	INT16U 	i;
	gp_memcpy(temp,p_audio_file_head,8);

	for (i=0;i<gp_strlen(temp);i++) {
		temp[i] = gp_toupper(temp[i]);
	}

	if((gp_strcmp(temp, (INT8S *)"RIFFWAVE")==0)&&(gp_strcmp((p_audio_file_head + 12), (INT8S *)"fmt ")==0))
	{
		return AUDIO_PARSE_SUCCS ;
	}
	else
	{
		return AUDIO_PARSE_FAIL ;
	}*/
}

static INT32S audio_real_type_get(INT16S fd, INT8S type_index)
{
	INT8U err;
	INT16U 	i;
	INT32U  audio_file_head;
	INT32S len;
	AUDIO_TYPE audio_real_type;
	TK_FILE_SERVICE_STRUCT audio_file_head_para;
	AUDIO_FILE_PARSE audio_file_parse_head[AUDIO_MAX_TYPE];

	if ((type_index != AUDIO_TYPE_WAV) && (type_index != AUDIO_TYPE_MP3) && (type_index != AUDIO_TYPE_WMA)) {
		return type_index;
	}

	audio_file_parse_head[AUDIO_TYPE_MP3].audio_file_real_type = AUDIO_TYPE_MP3;
	audio_file_parse_head[AUDIO_TYPE_MP3].parse_audio_file_head = parse_mp3_file_head;

	audio_file_parse_head[AUDIO_TYPE_WMA].audio_file_real_type = AUDIO_TYPE_WMA;
	audio_file_parse_head[AUDIO_TYPE_WMA].parse_audio_file_head = parse_wma_file_head;

	audio_file_parse_head[AUDIO_TYPE_WAV].audio_file_real_type = AUDIO_TYPE_WAV;
	audio_file_parse_head[AUDIO_TYPE_WAV].parse_audio_file_head = parse_wav_file_head;

	audio_file_head_para.fd = fd;
	audio_file_head_para.result_queue = audio_fsq;
	audio_file_head_para.buf_size = AUDIO_PASER_BUFFER_SIZE;

	audio_file_head = (INT32U) gp_malloc((audio_file_head_para.buf_size));
	//audio_file_head = (INT32U) gp_malloc(audio_file_head_para.buf_size);
	if (audio_file_head == NULL)
	{
		DBG_PRINT("audio file parse failed to allocate memory\r\n");

		return type_index;   //what i can do if malloc error?
	}

	audio_file_head_para.buf_addr = (INT32U) audio_file_head;

	lseek((INT16S)audio_file_head_para.fd, 0, SEEK_SET);
	msgQSend(StorageServiceQ, MSG_FILESRV_FS_READ, (void *)&audio_file_head_para, sizeof(TK_FILE_SERVICE_STRUCT), MSG_PRI_URGENT);

        len = (INT32S)OSQPend(audio_fsq, 200, &err);
        if ((err != OS_NO_ERR) || len < 0)
	{
	 	gp_free((void *) audio_file_head);
    	    	return type_index;  // AUDIO_READ_FAIL;     //how to handle it  if read error?
        }
	lseek((INT16S)audio_file_head_para.fd, 0, SEEK_SET);

	// 1. check it is real the type_index,according the file extension to judge the file real type.
	if((audio_file_parse_head[type_index].parse_audio_file_head((INT8S *)audio_file_head )) == AUDIO_PARSE_SUCCS )
	{
		gp_free((void *) audio_file_head);
		return audio_file_parse_head[type_index].audio_file_real_type;
	}

	// else then enum all support file type except current extension type
	for (i=1;i<AUDIO_MAX_TYPE;i++)
	{
		if(( i == type_index) && (i < AUDIO_MAX_TYPE -1))
		{
			i++;
		}
		else if ((i == type_index) && (i >= AUDIO_MAX_TYPE -1))
		{
			audio_real_type = AUDIO_TYPE_NONE;
			break;  // if type_index = AUDIO_MAX_TYPE & i the same as type_index ,so should be break.
		}

		if((audio_file_parse_head[i].parse_audio_file_head((INT8S *)audio_file_head ) )== AUDIO_PARSE_SUCCS )
		{
			audio_real_type = audio_file_parse_head[i].audio_file_real_type;
			break;
		}
		else
		{
			audio_real_type = AUDIO_TYPE_NONE;
		}

	}

	gp_free((void *) audio_file_head);

	return audio_real_type;

}
#endif

static INT32S audio_get_type(INT16S fd,INT8S* file_name)
{
   	INT8S  	temp[5] = {0};
	INT16U 	i;

	gp_strcpy(temp,file_name);

	for (i=0;i<gp_strlen(temp);i++) {
		temp[i] = gp_toupper(temp[i]);
	}

#if (defined GP_FILE_FORMAT_SUPPORT) && (GP_FILE_FORMAT_SUPPORT == GP_FILE_FORMAT_SET_1)
	if(gp_strcmp(temp, (INT8S *)"GA")==0) {
		return AUDIO_TYPE_WMA;
   	}
   	else {
   	    return AUDIO_TYPE_NONE;
   	}
#endif

#if APP_WAV_CODEC_FG_EN == 1 || APP_WAV_CODEC_BG_EN == 1
   	if(gp_strcmp(temp, (INT8S *)"WAV")==0)
	{

		//return AUDIO_TYPE_WAV;
		return audio_real_type_get(fd,AUDIO_TYPE_WAV);
   	}

#endif

#if APP_MP3_DECODE_FG_EN == 1 || APP_MP3_DECODE_BG_EN == 1
   	if(gp_strcmp(temp, (INT8S *)"MP3")==0)
	{
		//return AUDIO_TYPE_MP3;
		return audio_real_type_get(fd,AUDIO_TYPE_MP3);
	}

#endif

#if APP_WMA_DECODE_FG_EN == 1 || APP_WMA_DECODE_BG_EN == 1
	if(gp_strcmp(temp, (INT8S *)"WMA")==0)
	{
		//return AUDIO_TYPE_WMA;
		return audio_real_type_get(fd,AUDIO_TYPE_WMA);
	}
#endif

#if APP_A1800_DECODE_FG_EN == 1 || APP_A1800_DECODE_BG_EN == 1
    if(gp_strcmp(temp, (INT8S *)"A18")==0)//080722
    	return AUDIO_TYPE_A1800;
#endif

    if(gp_strcmp(temp, (INT8S *)"IDI")==0)//080903
    	return AUDIO_TYPE_MIDI;

    if(gp_strcmp(temp, (INT8S *)"GMD")==0)
    	return AUDIO_TYPE_MIDI;

#if APP_A1600_DECODE_FG_EN == 1 || APP_A1600_DECODE_BG_EN == 1
    if(gp_strcmp(temp, (INT8S *)"A16")==0)	// added by Bruce, 2008/09/23
    	return AUDIO_TYPE_A1600;
#endif

#if APP_A6400_DECODE_FG_EN == 1 || APP_A6400_DECODE_BG_EN == 1
    if(gp_strcmp(temp, (INT8S *)"A64")==0)	// added by Bruce, 2008/09/25
    	return AUDIO_TYPE_A6400;
#endif

#if APP_S880_DECODE_FG_EN == 1 || APP_S880_DECODE_BG_EN == 1
    if(gp_strcmp(temp, (INT8S *)"S88")==0)	// added by Bruce, 2008/09/25
    	return AUDIO_TYPE_S880;
#endif

   	return AUDIO_TYPE_NONE;

}
#else
static INT32S audio_get_type(INT8S* file_name)
{
   	INT8S  	temp[5] = {0};
   	INT16U 	i;

	gp_strcpy(temp,file_name);

	for (i=0;i<gp_strlen(temp);i++) {
		temp[i] = gp_toupper(temp[i]);
	}

#if (defined GP_FILE_FORMAT_SUPPORT) && (GP_FILE_FORMAT_SUPPORT == GP_FILE_FORMAT_SET_1)
    if(gp_strcmp(temp, (INT8S *)"GA")==0) {
		return AUDIO_TYPE_WMA;
   	}
   	else {
   	    return AUDIO_TYPE_NONE;
   	}
#endif

#if APP_WAV_CODEC_FG_EN == 1 || APP_WAV_CODEC_BG_EN == 1
   	if(gp_strcmp(temp, (INT8S *)"WAV")==0)
      return AUDIO_TYPE_WAV;
#endif

#if APP_MP3_DECODE_FG_EN == 1 || APP_MP3_DECODE_BG_EN == 1
   	if(gp_strcmp(temp, (INT8S *)"MP3")==0)
    	return AUDIO_TYPE_MP3;
#endif

#if APP_WMA_DECODE_FG_EN == 1 || APP_WMA_DECODE_BG_EN == 1
    if(gp_strcmp(temp, (INT8S *)"WMA")==0)
    	return AUDIO_TYPE_WMA;
#endif

#if APP_A1800_DECODE_FG_EN == 1 || APP_A1800_DECODE_BG_EN == 1
    if(gp_strcmp(temp, (INT8S *)"A18")==0)//080722
    	return AUDIO_TYPE_A1800;
#endif

    if(gp_strcmp(temp, (INT8S *)"IDI")==0)//080903
    	return AUDIO_TYPE_MIDI;

    if(gp_strcmp(temp, (INT8S *)"GMD")==0)
    	return AUDIO_TYPE_MIDI;

#if APP_A1600_DECODE_FG_EN == 1 || APP_A1600_DECODE_BG_EN == 1
    if(gp_strcmp(temp, (INT8S *)"A16")==0)	// added by Bruce, 2008/09/23
    	return AUDIO_TYPE_A1600;
#endif

#if APP_A6400_DECODE_FG_EN == 1 || APP_A6400_DECODE_BG_EN == 1
    if(gp_strcmp(temp, (INT8S *)"A64")==0)	// added by Bruce, 2008/09/25
    	return AUDIO_TYPE_A6400;
#endif

#if APP_S880_DECODE_FG_EN == 1 || APP_S880_DECODE_BG_EN == 1
    if(gp_strcmp(temp, (INT8S *)"S88")==0)	// added by Bruce, 2008/09/25
    	return AUDIO_TYPE_S880;
#endif

   	return AUDIO_TYPE_NONE;

}
#endif

static void audio_queue_clear(void)
{
	//OSQFlush(hAudioDacTaskQ);
	OSQFlush(aud_send_q);
	OSQFlush(audio_wq);
	OSQFlush(audio_fsq);
	OSQPost(hAudioDacTaskQ, (void *)MSG_AUD_DMA_WIDX_CLEAR);
}

#if APP_MP3_DECODE_FG_EN == 1 || APP_WMA_DECODE_FG_EN == 1 || APP_WAV_CODEC_FG_EN == 1 || APP_A1600_DECODE_FG_EN == 1 || APP_A1800_DECODE_FG_EN == 1 || APP_A6400_DECODE_FG_EN == 1 || APP_S880_DECODE_FG_EN == 1
#if READING_BY_FILE_SRV == 0
static INT32S audio_write_buffer(INT8U *ring_buf, INT32U wi, INT32U ri)
{
	INT32S t;
	INT32S len;

	if(wi == 0 && ri == 0) {
		len = read(audio_ctrl.file_handle,(INT32U)ring_buf, audio_ctrl.ring_size/2);
        wi += len;
        return wi;
    }

    len = ri - wi;
    if (len < 0) {
    	len += audio_ctrl.ring_size;
    }
    if(len < audio_ctrl.ring_size/2) {
    //if(len < (audio_ctrl.ring_size - 2048)) {
    	return wi;
    }

	t = wi;
	wi += audio_ctrl.ring_size/2;
	if(wi == audio_ctrl.ring_size) {
		wi = 0;
	}
	if(wi == ri) {
		return t;
	}

	DBG_PRINT("hi read\r\n");
#if 0
	if (OSTaskChangePrio(AUD_DEC_PRIORITY,AUD_DEC_HIGH_PRIORITY) != OS_NO_ERR) {
		DBG_PRINT("change prio failed\r\n");
	}
#endif
	len = read(audio_ctrl.file_handle,(INT32U)ring_buf+t, audio_ctrl.ring_size/2);
	DBG_PRINT("hi read ok\r\n");
#if 0
	if (OSTaskChangePrio(AUD_DEC_HIGH_PRIORITY,AUD_DEC_PRIORITY) != OS_NO_ERR) {
		DBG_PRINT("change prio failed\r\n");
	}
#endif
	if (len<0) {
		DBG_PRINT("error read file \r\n");
	}
	wi = t + len;
	if(wi == audio_ctrl.ring_size) {
		wi = 0;
	}
	return wi;
}
#else

static INT32S audio_write_with_file_srv(INT8U *ring_buf, INT32U wi, INT32U ri)
{
	INT32S t;
	INT32S len;
	INT8U  err;
	INT32U msg_id;
	TK_FILE_SERVICE_STRUCT audio_fs_para;
	OS_Q_DATA test_storageQ;
	
	if (reverse_play||MP3_Speed) {
	    return wi;
	}
	switch(audio_context_p->source_type) {
		case AUDIO_SRC_TYPE_FS:
			msg_id = MSG_FILESRV_FS_READ;
			break;
		case AUDIO_SRC_TYPE_GPRS:
			msg_id = MSG_FILESRV_NVRAM_AUDIO_GPRS_READ;
			audio_fs_para.spi_para.sec_offset = audio_ctrl.read_secs;
			break;
		case AUDIO_SRC_TYPE_APP_RS:
			msg_id = MSG_FILESRV_NVRAM_AUDIO_APP_READ;
			break;
		case AUDIO_SRC_TYPE_SDRAM_LDW:
			msg_id = MSG_FILESRV_LDW_AUDIO_APP_READ;
			break;
		case AUDIO_SRC_TYPE_APP_PACKED_RS:
			msg_id = MSG_FILESRV_NVRAM_AUDIO_APP_PACKED_READ;
			break;
		case AUDIO_SRC_TYPE_USER_DEFINE:	// added by Bruce, 2008/10/27
			msg_id = MSG_FILESRV_USER_DEFINE_READ;
			break;
		default:
			break;
	}

	audio_fs_para.fd = audio_ctrl.file_handle;
	audio_fs_para.result_queue = audio_fsq;
	
	len = wi - ri;
    if (len < 0) {
    	len += audio_ctrl.ring_size;
    }

	if ((len < 2048) && audio_ctrl.reading) {
		return AUDIO_READ_WAIT;
	}

	//if(wi == 0 && ri == 0) {
	if(ri == wi) {
		audio_fs_para.buf_addr = (INT32U)ring_buf;
		audio_fs_para.buf_size = audio_ctrl.ring_size/2;
		audio_fs_para.spi_para.sec_cnt = audio_ctrl.ring_size/1024;
		if (reverse_play_start) {
		    OSQPost(hAudioDacTaskQ,(void *) MSG_AUD_DAC_REVRESE_CLEAR);
		    OSTimeDly(10);
		    if(audio_context_p->audio_format == AUDIO_TYPE_WAV)
			    audio_fs_para.rev_seek = reverse_points*(bit_per_samp/8)*channel;
			else
			{
			    
			    audio_fs_para.rev_seek = reverse_points*FrameSize_ave;
			}
	       	audio_fs_para.FB_seek = FB_seek;
		    reverse_points = 0;
		    reverse_play_start = 0;
		    audio_mute_ctrl_set(FALSE);
	        mp3_dec_set_ri((void*)audio_ctrl.work_mem, 0);
	        audio_ctrl.wi = 0;
	        wi = 0;
		} else {
		    audio_fs_para.rev_seek = 0;
		} 
		    
		if (audio_context_p->source_type == AUDIO_SRC_TYPE_FS) {
        	OSQQuery(StorageServiceQ->pEvent,&test_storageQ);
        	msgQSend(StorageServiceQ, msg_id, (void *)&audio_fs_para, sizeof(TK_FILE_SERVICE_STRUCT), MSG_PRI_URGENT);
        	OSQQuery(StorageServiceQ->pEvent,&test_storageQ);
        } else {
        	msgQSend(fs_msg_q_id, msg_id, (void *)&audio_fs_para, sizeof(TK_FILE_SERVICE_STRUCT), MSG_PRI_URGENT);
		}
        len = (INT32S) OSQPend(audio_fsq, 0, &err);
        if ((err != OS_NO_ERR) || len < 0) {
    	    return AUDIO_READ_FAIL;
        }
        audio_ctrl.read_secs += (len>>9);
        wi += len;
        if(wi == audio_ctrl.ring_size) {
		    wi = 0;
	    }
        return wi;
    }

    len = ri - wi;
    if (len < 0) {
    	len += audio_ctrl.ring_size;
    }
    
    if(len < audio_ctrl.ring_size/2) {
    	return wi;
    }

	t = wi;
	wi += audio_ctrl.ring_size/2;
	if(wi == audio_ctrl.ring_size) {
		wi = 0;
	}
	if(wi == ri) {
		return t;
	}
	
	if (audio_ctrl.reading) {
		return AUDIO_READ_PEND;
	}

	audio_ctrl.reading = 1;

	audio_fs_para.buf_addr = (INT32U)ring_buf+t;
	audio_fs_para.buf_size = audio_ctrl.ring_size/2;
	audio_fs_para.spi_para.sec_cnt = audio_ctrl.ring_size/1024;
	audio_fs_para.rev_seek = 0;
	if (audio_context_p->source_type == AUDIO_SRC_TYPE_FS) {
        msgQSend(StorageServiceQ, msg_id, (void *)&audio_fs_para, sizeof(TK_FILE_SERVICE_STRUCT), MSG_PRI_URGENT);
    } else {
        msgQSend(fs_msg_q_id, msg_id, (void *)&audio_fs_para, sizeof(TK_FILE_SERVICE_STRUCT), MSG_PRI_URGENT);
	}
	return AUDIO_READ_PEND;
}
void audio_work_ringbuffer_clear()
{
	mp3_dec_set_ri((void*)audio_ctrl.work_mem, 0);
	audio_ctrl.wi = 0;
}

#endif

#if READING_BY_FILE_SRV == 1
static INT32S audio_check_wi(INT32S wi_in, INT32U *wi_out, INT8U wait)
{
	INT32S len;
	INT8U  err;
	INT32S t;

	if (wi_in >= 0) {
		*wi_out = wi_in;
		return AUDIO_ERR_NONE;
	}
	if (wi_in == AUDIO_READ_FAIL) {
		return AUDIO_READ_FAIL;
	}
	/* AUDIO_READ_PEND */
	t = audio_ctrl.wi;
	
	if (wait) {
		//DBG_PRINT("need pend\r\n");
	    len = (INT32S) OSQPend(audio_fsq, 0, &err);
	}
	else {
		len = (INT32U)OSQAccept(audio_fsq, &err);
		if (err == OS_Q_EMPTY) {
			//*wi_out = *wi_out;
			return 	AUDIO_ERR_NONE;
		}
	}

	audio_ctrl.reading = 0;

	if ((err != OS_NO_ERR) || len < 0) {
        return AUDIO_READ_FAIL;
    }
	if (len != (audio_ctrl.ring_size>>1)) {
		audio_ctrl.file_end = 1;
	}
    
    audio_ctrl.read_secs += (len>>9);
	t += len;
	if(t == audio_ctrl.ring_size) {
		t = 0;
	}

	*wi_out = t;
	return 	AUDIO_ERR_NONE;
}
#endif

static INT32S audio_send_to_dma(void)
{
	//INT32U count;

	if (stopped && (audio_q_check()==AUDIO_SEND_FAIL)) { /* queuq full */
		stopped = 0;
		OSQPost(hAudioDacTaskQ,(void *) MSG_AUD_DMA_DBF_START);
	}
	else if (dac_dma_status_get() == 0) {
		if (!stopped) {
			#if 0 /* dac underrun, wait queue full again and start DMA*/
			stopped = 1;
			dac_dbf_free();
			DBG_PRINT("dac underrun !\r\n");

			audio_queue_clear();
			for (count=0;count<dac_buf_nums;count++) {
				OSQPost(audio_wq, (void *) count);
			}
			#else /* if any buffer into queue, start DMA again */
				OSQPost(hAudioDacTaskQ,(void *) MSG_AUD_DMA_DBF_RESTART);
			#endif
		}
	}
	return STATUS_OK;
}

static void audio_short_file_handle(void)
{
    OS_Q      *pq;
    INT32U    wb_idx;
    INT8U     err;
    
	pq = (OS_Q *)aud_send_q->OSEventPtr;
	
	if (pq->OSQEntries >= 2) {
	    OSQPost(hAudioDacTaskQ,(void *) MSG_AUD_DMA_DBF_START);
	    return;
	}
	
	wb_idx = (INT32U) OSQPend(audio_wq, 0, &err);
    gp_memset((INT8S*)pcm_out[wb_idx],0,32*2);
    pcm_len[wb_idx] = 32;
    OSQPost(aud_send_q, (void *) wb_idx);
    
    OSQPost(hAudioDacTaskQ,(void *) MSG_AUD_DMA_DBF_START);
}

static INT32S audio_q_check(void)
{
	OS_Q      *pq;
	pq = (OS_Q *)aud_send_q->OSEventPtr;
	if (pq->OSQEntries >= dac_buf_nums) {
		return AUDIO_SEND_FAIL;
	}
	return AUDIO_SEND_OK;
}
#endif

#if APP_MP3_DECODE_FG_EN == 1
INT32S audio_mp3_play_init(void)
{
	INT32U  count;
	INT32S  ret = 0;
	INT32U  in_length;
	#if READING_BY_FILE_SRV == 1
	INT32S  t_wi;
	#endif
	INT32U pos;
    
    #if (defined AUDIO_PROGRESS_SUPPORT) && (AUDIO_PROGRESS_SUPPORT == CUSTOM_ON)
	total_play_time = mp3_get_duration(audio_ctrl.file_handle,audio_ctrl.file_len);
	aud_time.total_play_time = total_play_time;
	aud_time.curr_play_time = 0;
	FrameSize_ave = audio_ctrl.file_len /total_play_time;
    #endif
	
	// mp3 init
	gp_memset((INT8S*)audio_ctrl.ring_buf,0,RING_BUF_SIZE);
	
	#if MP3_DECODE == MP3_DECODE_HW
	ret = mp3_dec_init((void*)audio_ctrl.work_mem, (void*)audio_ctrl.ring_buf);
	mp3_dec_set_ring_size((void*)audio_ctrl.work_mem,audio_ctrl.ring_size);
	#else
	ret = mp3_dec_init((char*)audio_ctrl.work_mem, (unsigned char*)audio_ctrl.ring_buf, (char*)(audio_ctrl.work_mem + MP3_DEC_MEMORY_SIZE));
	mp3_dec_set_bs_buf_size((void*)audio_ctrl.work_mem,audio_ctrl.ring_size);
	#endif
	
	audio_ctrl.file_cnt = 0;
	audio_ctrl.f_last = 0;
	audio_ctrl.try_cnt = 20;
	audio_ctrl.read_secs = 0;
	audio_ctrl.file_end = 0;
	fg_error_cnt = 0;
	
	reverse_play_start = 0;
	reverse_points = 0;

	audio_ctrl.wi = 0;
	audio_ctrl.ri = mp3_dec_get_ri((void*)audio_ctrl.work_mem);
	
	#if READING_BY_FILE_SRV == 1
	t_wi = audio_write_with_file_srv(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	if (audio_check_wi(t_wi, &audio_ctrl.wi, 1) != AUDIO_ERR_NONE) { /* check reading data */
		return AUDIO_ERR_READ_FAIL;
	}
	#else
	audio_ctrl.wi = audio_write_buffer(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	#endif
	
	count = 3;
	while(1)
	{
		in_length = audio_ctrl.ri;
		ret = mp3_dec_parsing((void*)audio_ctrl.work_mem , audio_ctrl.wi);

		audio_ctrl.ri = mp3_dec_get_ri((void*)audio_ctrl.work_mem);

		audio_ctrl.file_cnt += (audio_ctrl.ri - in_length);
		if(audio_ctrl.ri < in_length) {
			audio_ctrl.file_cnt += audio_ctrl.ring_size;
		}

		switch(ret)
		{
			case MP3_DEC_ERR_NONE:
				break;
			case MP3_DEC_ERR_LOSTSYNC:		//not found sync word
			case MP3_DEC_ERR_BADSAMPLERATE:	//reserved sample frequency value
			case MP3_DEC_ERR_BADBITRATE:		//forbidden bitrate value
			case MP3_DEC_ERR_BADLAYER:
			case MP3_DEC_ERR_BADMPEGID:
				//feed in DecodeInBuffer;
				audio_ctrl.ri = mp3_dec_get_ri((void*)audio_ctrl.work_mem);
				#if READING_BY_FILE_SRV == 1
				if (audio_ctrl.file_end == 0) {
					t_wi = audio_write_with_file_srv(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
					OSTimeDly(5);
					if (audio_check_wi(t_wi, &audio_ctrl.wi, 1) != AUDIO_ERR_NONE) { /* check reading data */
						return AUDIO_ERR_READ_FAIL;
					}
				}
				#else
				audio_ctrl.wi = audio_write_buffer(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
				#endif
				if (--count == 0)
					return AUDIO_ERR_DEC_FAIL;
				continue;
			default:
				return AUDIO_ERR_DEC_FAIL;
		}
		if(ret == MP3_DEC_ERR_NONE) {
			break;
		}
	}

	in_length = mp3_dec_get_samplerate((CHAR*)audio_ctrl.work_mem);

	dac_stereo_set();
	dac_sample_rate_set(in_length);
	channel = 2;
	g_audio_sample_rate = in_length;
	
	ret = mp3_dec_get_bitrate((void*)audio_ctrl.work_mem);
	if (ret == 0) {
		return AUDIO_ERR_DEC_FAIL;
	}
	DBG_PRINT("bps: %d\r\n",ret);
	
	ret = mp3_dec_get_channel((void*)audio_ctrl.work_mem);
	if ((ret != 1) && (ret != 2)) {
		return AUDIO_ERR_DEC_FAIL;
	}
	DBG_PRINT("channel: %d\r\n",ret);
	
	DBG_PRINT("sample rate: %d\r\n",mp3_dec_get_samplerate((void*)audio_ctrl.work_mem));
	DBG_PRINT("block size: %d\r\n",mp3_dec_get_mem_block_size());
	
	#if PERFORMANCE_MEASURE == 1
	R_TIMERC_CTRL = 0x8062; /*32768 */
	#endif
	
	MP3_Speed = 0;
	for (count=0;count<dac_buf_nums;count++) {
		OSQPost(audio_wq, (void *) count);
	}
	return AUDIO_ERR_NONE;
}

INT32S  audio_mp3_process(void)
{
	INT32S  pcm_point;
	INT32U  in_length;
	INT32S  tmp_len;
	INT8U   err;
	INT32U  wb_idx;
	INT32U	temp;
	
	#if READING_BY_FILE_SRV == 1
	INT8U   wait;
	INT32S  t_wi;
	#endif

	audio_ctrl.ri = mp3_dec_get_ri((void*)audio_ctrl.work_mem);
	
	#if READING_BY_FILE_SRV == 1
	if (audio_ctrl.file_end == 0) {
	    if ((reverse_play||MP3_Speed)&&(audio_ctrl.wi == audio_ctrl.ri)) {
	        if (reverse_play_start == 0) {
	            OSQPost(hAudioDacTaskQ,(void *) MSG_AUD_DAC_REVRESE);
	            reverse_play_start = 1;
	            if(reverse_play)
	         		FB_seek = 1;
	            else
	          		FB_seek = 0;
	            audio_mute_ctrl_set(TRUE);
	        }
	        if(MP3_Speed==1)
	     		pcm_point = 1;
	     	else if(MP3_Speed==2)
	     		pcm_point = 2;
	     	else
	     		pcm_point = 1;
	     	
	        reverse_points += pcm_point;
	        tmp_len = pcm_point*FrameSize_ave;
	        if(reverse_play)
	        {
				if (audio_ctrl.file_cnt >= tmp_len) {
				    audio_ctrl.file_cnt -= tmp_len;
				} else {
					reverse_play = 0;
					MP3_Speed = 0;
					audio_mute_ctrl_set(FALSE);
					ap_state_handling_icon_clear_cmd(ICON_BACKWARD, ICON_2X, ICON_4X);
					ap_state_handling_icon_clear_cmd(ICON_FORWARD, NULL, NULL);
				    return AUDIO_ERR_DEC_FINISH;
				}
			} 
			else
			{
				temp = audio_ctrl.file_cnt;
				temp += tmp_len;
				if(temp >= (audio_ctrl.file_len)){
					if(audio_ctrl.f_last){
						MP3_Speed = 0;
						audio_mute_ctrl_set(FALSE);
						ap_state_handling_icon_clear_cmd(ICON_BACKWARD, ICON_2X, ICON_4X);
						ap_state_handling_icon_clear_cmd(ICON_FORWARD, NULL, NULL);
						return AUDIO_ERR_DEC_FINISH;
					}
					else
					{
						MP3_Speed = 0;
						audio_ctrl.f_last = 1;
					}
				}else{
					audio_ctrl.file_cnt = temp;
				}
			}
	    	OSTimeDly(20);
			goto mp3OK;
	    }
	    else
			t_wi = audio_write_with_file_srv(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
			
	}
	#else
	audio_ctrl.wi = audio_write_buffer(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	#endif
	
	wb_idx = (INT32U) OSQPend(audio_wq, 0, &err);
	
	#if READING_BY_FILE_SRV == 1
	if (audio_ctrl.file_end == 0) {
		wait = 0;
		if (t_wi == AUDIO_READ_WAIT) {
			wait = 1; /* pend until read finish */
		}
		if (audio_check_wi(t_wi, &audio_ctrl.wi, wait) != AUDIO_ERR_NONE) { /* check reading data */
			return AUDIO_ERR_READ_FAIL;
		}
	}
	#endif
	
	if(audio_ctrl.file_cnt >= audio_ctrl.file_len){
		if(audio_ctrl.f_last){
			return AUDIO_ERR_DEC_FINISH;
		}
		else{
			audio_ctrl.f_last = 1;
		}
	}

	in_length = audio_ctrl.ri;
	
	#if PERFORMANCE_MEASURE == 1
	R_TIMERC_CTRL |= 0x2000; 
	timer_cnt1 = R_TIMERC_UPCOUNT;
	#endif
	
	pcm_point = mp3_dec_run((void*)audio_ctrl.work_mem, pcm_out[wb_idx], audio_ctrl.wi,audio_ctrl.f_last);
	
	#if PERFORMANCE_MEASURE == 1
	timer_cnt2 = R_TIMERC_UPCOUNT;
	R_TIMERC_CTRL &= ~0x2000;
	times = timer_cnt2-timer_cnt1;
	
	total_cnt += times; 
	if (times > cnt_max[0]) {
		cnt_max[0] = times;
		cnt_max_fn[0] = total_frame;
		point[0] = pcm_point;
	}
	else if (times > cnt_max[1]) {
		cnt_max[1] = times;
		cnt_max_fn[1] = total_frame;
		point[1] = pcm_point;
	}
	else if (times > cnt_max[2]) {
		cnt_max[2] = times;
		cnt_max_fn[2] = total_frame;
		point[2] = pcm_point;
	}
	total_frame++;
	#endif
		
	audio_ctrl.ri = mp3_dec_get_ri((void*)audio_ctrl.work_mem);
	audio_ctrl.file_cnt += (audio_ctrl.ri - in_length);

	if(in_length > audio_ctrl.ri) {
		audio_ctrl.file_cnt += audio_ctrl.ring_size;
	}

	if (pcm_point <= 0) {
		if (pcm_point == 0 && ++fg_error_cnt > MP3_FRAME_ERROR_CNT) {
	        return AUDIO_ERR_DEC_FAIL;
	    }
		if (pcm_point < 0) {
			if (--audio_ctrl.try_cnt == 0) {
				return AUDIO_ERR_DEC_FAIL;
			}
		}
		OSQPostFront(audio_wq, (void *)wb_idx);
		audio_send_next_frame_q();
		return 0;
	}
	else {
		fg_error_cnt = 0;
	}


	pcm_len[wb_idx] = pcm_point*2; /* 2 channel */

	OSQPost(aud_send_q, (void *) wb_idx);
	audio_send_to_dma();
mp3OK:
	aud_time.curr_play_time = audio_ctrl.file_cnt/FrameSize_ave;
	audio_send_next_frame_q();
	return 0;
}



#endif // #if APP_MP3_DECODE_FG_EN == 1

#if APP_WMA_DECODE_FG_EN == 1
int audio_wma_play_init()
{
	INT32S count;
	INT32U in_length;
	INT32S ret;
	INT8U  ch;
	INT32S  t_wi;

	audio_ctrl.file_cnt = 0;
	audio_ctrl.try_cnt = 20;
	audio_ctrl.read_secs = 0;
    fg_error_cnt = 0;

	gp_memset((INT8S*)audio_ctrl.ring_buf,0,audio_ctrl.ring_size);

	ret = wma_dec_init((char *)audio_ctrl.work_mem, audio_ctrl.ring_buf,(char *)audio_ctrl.work_mem + 8192,audio_ctrl.ring_size);

	audio_ctrl.wi = wma_dec_get_ri((char *)audio_ctrl.work_mem);
	audio_ctrl.ri = wma_dec_get_ri((char *)audio_ctrl.work_mem);

	#if READING_BY_FILE_SRV == 1
	t_wi = audio_write_with_file_srv(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	if (audio_check_wi(t_wi, &audio_ctrl.wi, 1) != AUDIO_ERR_NONE) { /* check reading data */
		return AUDIO_ERR_READ_FAIL;
	}
	#else
	audio_ctrl.wi = audio_write_buffer(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	#endif
	count = 50;

	while(1)
	{
		in_length = audio_ctrl.ri;
		ret = wma_dec_parsing((char *)audio_ctrl.work_mem , audio_ctrl.wi);
		audio_ctrl.ri = wma_dec_get_ri((char *)audio_ctrl.work_mem);

		audio_ctrl.file_cnt += (audio_ctrl.ri - in_length);
		if(audio_ctrl.ri < in_length) {
			audio_ctrl.file_cnt += WMA_DEC_BITSTREAM_BUFFER_SIZE;
		}

		switch(ret)
		{
			case WMA_OK:
				break;
			case WMA_E_BAD_PACKET_HEADER:
			case WMA_E_INVALIDHEADER:
			case WMA_E_NOTSUPPORTED:
			case WMA_E_NOTSUPPORTED_CODEC:
				return AUDIO_ERR_INVALID_FORMAT;
			case WMA_E_ONHOLD:
			case WMA_E_DATANOTENOUGH:
				//feed in DecodeInBuffer;
				audio_ctrl.ri = wma_dec_get_ri((char *)audio_ctrl.work_mem);
				#if READING_BY_FILE_SRV == 1
				t_wi = audio_write_with_file_srv(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
				if (audio_check_wi(t_wi, &audio_ctrl.wi, 1) != AUDIO_ERR_NONE) { /* check reading data */
					return AUDIO_ERR_READ_FAIL;
				}
				#else
				audio_ctrl.wi = audio_write_buffer(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
				#endif
				if (--count == 0)
					return AUDIO_ERR_DEC_FAIL;
				continue;
			default:
				if (--count == 0)
					return AUDIO_ERR_DEC_FAIL;
				break;
		}

		if(ret == WMA_OK) {
			break;
		}
	}

	dac_sample_rate_set(wma_dec_get_samplerate((char *)audio_ctrl.work_mem));

	ch = wma_dec_get_channel((char *)audio_ctrl.work_mem);
	channel = ch;
	g_audio_sample_rate = wma_dec_get_samplerate((char *)audio_ctrl.work_mem);	//20090209 Roy
	if (ch == 1) {
		dac_mono_set();
	}
	else {
		dac_stereo_set();
	}

	DBG_PRINT("bps: %d\r\n",wma_dec_get_bitrate((char*)audio_ctrl.work_mem));
	DBG_PRINT("channel: %d\r\n",ch);
	DBG_PRINT("sample rate: %d\r\n",wma_dec_get_samplerate((char*)audio_ctrl.work_mem));

	for (count=0;count<dac_buf_nums;count++) {
		OSQPost(audio_wq, (void *) count);
	}

	return AUDIO_ERR_NONE;
}

INT32S  audio_wma_process()
{
	INT32S	pcm_point;
	INT32U  in_length;
	INT8U   err;
	INT32U  wb_idx;
	INT8U   ch;
	INT32S  offset;
	INT32S  len;
	#if READING_BY_FILE_SRV == 1
	INT8U   wait;
	INT32S  t_wi;
	#endif
	
	audio_ctrl.ri = wma_dec_get_ri((char *)audio_ctrl.work_mem);

	offset = wma_dec_get_offset((char *)audio_ctrl.work_mem);

	if(offset > 0) {
		len = audio_ctrl.wi - audio_ctrl.ri;
		if(len < 0) {
			len += WMA_DEC_BITSTREAM_BUFFER_SIZE;
		}

		audio_ctrl.file_cnt += offset;

		if(len > offset)
		{
			audio_ctrl.ri += offset;
			if(audio_ctrl.ri >= WMA_DEC_BITSTREAM_BUFFER_SIZE) {
				audio_ctrl.ri -= WMA_DEC_BITSTREAM_BUFFER_SIZE;
			}
			wma_dec_set_ri((char *)audio_ctrl.work_mem, audio_ctrl.ri);
		}
		else
		{
			offset -= len;
			if(lseek(audio_ctrl.file_handle, offset, SEEK_CUR) < 0) {
				return AUDIO_ERR_DEC_FAIL;
			}
			audio_ctrl.wi = audio_ctrl.ri = 0;
			wma_dec_set_ri((char *)audio_ctrl.work_mem, audio_ctrl.ri);
		}
		wma_dec_reset_offset((char *)audio_ctrl.work_mem);
	}

	#if READING_BY_FILE_SRV == 1
	t_wi = audio_write_with_file_srv(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	#else
	audio_ctrl.wi = audio_write_buffer(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	#endif
	
	wb_idx = (INT32U) OSQPend(audio_wq, 0, &err);
	
	#if READING_BY_FILE_SRV == 1
	wait = 0;
	if (t_wi == AUDIO_READ_WAIT) {
		wait = 1; /* pend until read finish */
	}
	if (audio_check_wi(t_wi, &audio_ctrl.wi, wait) != AUDIO_ERR_NONE) { /* check reading data */
		return AUDIO_ERR_READ_FAIL;
	}
	#endif

	in_length = audio_ctrl.ri;

	pcm_point = wma_dec_run((char *)audio_ctrl.work_mem,pcm_out[wb_idx],audio_ctrl.wi);

	audio_ctrl.ri = wma_dec_get_ri((char *)audio_ctrl.work_mem);
	audio_ctrl.file_cnt += (audio_ctrl.ri - in_length);

	if(in_length > audio_ctrl.ri) {
		audio_ctrl.file_cnt += WMA_DEC_BITSTREAM_BUFFER_SIZE;
	}

	if(audio_ctrl.file_cnt >= audio_ctrl.file_len){
		return AUDIO_ERR_DEC_FINISH;
	}

	if (pcm_point <= 0) {
	    if (pcm_point == 0 && ++fg_error_cnt > 30) {
	        return AUDIO_ERR_DEC_FAIL;
	    }
	    if (pcm_point < 0) {
			if (wma_dec_get_errno((char *)audio_ctrl.work_mem) == WMA_E_NO_MORE_FRAMES) {
				return AUDIO_ERR_DEC_FINISH;
			}
			else {
				return AUDIO_ERR_DEC_FAIL;
			}
		}
		OSQPostFront(audio_wq, (void *)wb_idx);
		audio_send_next_frame_q();
		return 0;
	}
    else {
        fg_error_cnt = 0;
    }

	ch = wma_dec_get_channel((char *)audio_ctrl.work_mem);

	pcm_len[wb_idx] = pcm_point*ch;

	OSQPost(aud_send_q, (void *) wb_idx);
	audio_send_to_dma();
	audio_send_next_frame_q();

	return 0;
}
#endif // #if APP_WMA_DECODE_FG_EN == 1

#if APP_WAV_CODEC_FG_EN == 1

INT32S audio_wav_dec_play_init()
{
	INT32U in_length;
	INT8U  ch;
	INT32S count;
	INT32S t_wi;

	audio_ctrl.file_cnt = 0;
	audio_ctrl.try_cnt = 10;
	audio_ctrl.read_secs = 0;
	audio_ctrl.file_end = 0;

	gp_memset((INT8S*)audio_ctrl.ring_buf,0,RING_BUF_SIZE);
	wav_dec_init((INT8U *)audio_ctrl.work_mem, audio_ctrl.ring_buf);
	audio_ctrl.wi = wav_dec_get_ri((INT8U *)audio_ctrl.work_mem);
	audio_ctrl.ri = wav_dec_get_ri((INT8U *)audio_ctrl.work_mem);
	in_length = audio_ctrl.ri;

	#if READING_BY_FILE_SRV == 1
	t_wi = audio_write_with_file_srv(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	if (audio_check_wi(t_wi, &audio_ctrl.wi, 1) != AUDIO_ERR_NONE) { /* check reading data */
		return AUDIO_ERR_READ_FAIL;
	}
	#else
	audio_ctrl.wi = audio_write_buffer(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	#endif

	audio_ctrl.file_len = wav_dec_parsing((INT8U *)audio_ctrl.work_mem , audio_ctrl.wi);

	if((int)(audio_ctrl.file_len) <= 0) {
		return AUDIO_ERR_DEC_FAIL;
	}

	audio_ctrl.ri = wav_dec_get_ri((INT8U *)audio_ctrl.work_mem);
	audio_ctrl.file_cnt += (audio_ctrl.ri - in_length);

	if(in_length > audio_ctrl.ri) {
		audio_ctrl.file_cnt += WAV_DEC_BITSTREAM_BUFFER_SIZE;
	}

	in_length = wav_dec_get_SampleRate((INT8U *)audio_ctrl.work_mem);
	ch = wav_dec_get_nChannels((INT8U *)audio_ctrl.work_mem);
    bit_per_samp = wav_dec_get_wBitsPerSample((INT8U *)audio_ctrl.work_mem);
    
	dac_sample_rate_set(in_length);
	channel = ch;
	g_audio_sample_rate = in_length;	//20090209 Roy
	if (ch == 1) {
		dac_mono_set();
	} else {
		dac_stereo_set();
	}
    total_play_time = audio_ctrl.file_len*8/bit_per_samp/g_audio_sample_rate/channel;

	//DBG_PRINT("channel: %d\r\n",ch);
	//DBG_PRINT("sample rate: %d\r\n",wav_dec_get_SampleRate((INT8U*)audio_ctrl.work_mem));
	//DBG_PRINT("file len = %d\r\n",audio_ctrl.file_len);
	for (count=0;count<dac_buf_nums;count++) {
		OSQPost(audio_wq, (void *) count);
	}

	return AUDIO_ERR_NONE;
}

INT32S  audio_wav_dec_process()
{
	INT32S	pcm_point;
	INT32U  in_length;
	INT32S  tmp_len;
	INT8U   ch;
	INT32U  wb_idx;
	INT8U   err;
	#if READING_BY_FILE_SRV == 1
	INT8U   wait;
	INT32S  t_wi;
	#endif

	audio_ctrl.ri = wav_dec_get_ri((INT8U *)audio_ctrl.work_mem);

	#if READING_BY_FILE_SRV == 1
	if (audio_ctrl.file_end == 0) {
	    t_wi = audio_write_with_file_srv(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	}
	#else
	audio_ctrl.wi = audio_write_buffer(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	#endif
 
	wb_idx = (INT32U) OSQPend(audio_wq, 0, &err);

	#if READING_BY_FILE_SRV == 1
	if (audio_ctrl.file_end == 0) {
	    wait = 0;
	    if (t_wi == AUDIO_READ_WAIT) {
		    wait = 1; /* pend until read finish */
	    }
	    if (audio_check_wi(t_wi, &audio_ctrl.wi, wait) != AUDIO_ERR_NONE) { /* check reading data */
		    return AUDIO_ERR_READ_FAIL;
	    }
	}
	#endif
    
    if(audio_ctrl.file_cnt >= audio_ctrl.file_len) {
		if (stopped) {
		    OSQPostFront(audio_wq, (void *)wb_idx);
		    audio_short_file_handle();
		}
		return AUDIO_ERR_DEC_FINISH;
	} else {
		in_length = audio_ctrl.wi - audio_ctrl.ri;
		if(audio_ctrl.wi < audio_ctrl.ri) {
			in_length += WAV_DEC_BITSTREAM_BUFFER_SIZE;
		}
		if(in_length > audio_ctrl.file_len-audio_ctrl.file_cnt) {
			tmp_len = audio_ctrl.wi;
			tmp_len -= (in_length - audio_ctrl.file_len + audio_ctrl.file_cnt);
			if(tmp_len < 0) {
				tmp_len += WAV_DEC_BITSTREAM_BUFFER_SIZE;
			}
			audio_ctrl.wi = tmp_len;
		}
	}

    if (reverse_play && (audio_ctrl.wi == audio_ctrl.ri)) {
        pcm_point = 128;
        if (reverse_play_start == 0) {
            OSQPost(hAudioDacTaskQ,(void *) MSG_AUD_DAC_REVRESE);
            reverse_play_start = 1;
        }
        reverse_points += pcm_point;
        tmp_len = pcm_point*(bit_per_samp/8)*channel;
		if (audio_ctrl.file_cnt >= tmp_len) {
		    audio_ctrl.file_cnt -= tmp_len;
		} else {
		    return AUDIO_ERR_DEC_FINISH;
		}    
        goto WAV_OK;
    }
    
	in_length = audio_ctrl.ri;

	pcm_point = wav_dec_run((INT8U *)audio_ctrl.work_mem,pcm_out[wb_idx],audio_ctrl.wi);
   
	if (pcm_point == -1) {
		return AUDIO_ERR_DEC_FAIL;
	}

	audio_ctrl.ri = wav_dec_get_ri((INT8U *)audio_ctrl.work_mem);
	audio_ctrl.file_cnt += (audio_ctrl.ri - in_length);

	if(in_length > audio_ctrl.ri) {
		audio_ctrl.file_cnt += WAV_DEC_BITSTREAM_BUFFER_SIZE;
	}

	if (pcm_point <= 0) {
		if (--audio_ctrl.try_cnt == 0) {
			return AUDIO_ERR_DEC_FAIL;
		}
		OSQPostFront(audio_wq, (void *)wb_idx);
		audio_send_next_frame_q();
		return 0;
	}
	
WAV_OK:
	ch = wav_dec_get_nChannels((INT8U *)audio_ctrl.work_mem);
	pcm_len[wb_idx] = pcm_point*ch;

	OSQPost(aud_send_q, (void *) wb_idx);
	audio_send_to_dma();
	audio_send_next_frame_q();
	return 0;
}
#endif // #if APP_WAV_CODEC_FG_EN == 1

#if APP_A1800_DECODE_FG_EN == 1
INT32S audio_a1800_play_init(void)
{
	INT32U  count;
	INT32S  ret;
	INT32U  in_length;
	INT32S  t_wi;

	audio_ctrl.file_cnt = 0;
	audio_ctrl.try_cnt = 20;
	audio_ctrl.read_secs = 0;//080724

	gp_memset((INT8S*)audio_ctrl.ring_buf,0,RING_BUF_SIZE);
	audio_ctrl.ring_size = A18_DEC_BITSTREAM_BUFFER_SIZE;//must be 256 byte for a1800

	ret = a1800dec_GetMemoryBlockSize();//080723
	if(ret != A1800DEC_MEMORY_BLOCK_SIZE)
		return AUDIO_ERR_DEC_FAIL;

	ret = a1800dec_init((void *)audio_ctrl.work_mem, (const unsigned char *)audio_ctrl.ring_buf);//080723
	audio_ctrl.wi = a1800dec_read_index((void *)audio_ctrl.work_mem);//after initial the value is 0
	audio_ctrl.ri = a1800dec_read_index((void *)audio_ctrl.work_mem);
	in_length = (audio_ctrl.ri);

	#if READING_BY_FILE_SRV == 1
	t_wi = audio_write_with_file_srv(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	if (audio_check_wi(t_wi, &audio_ctrl.wi, 1) != AUDIO_ERR_NONE) { /* check reading data */
		return AUDIO_ERR_READ_FAIL;
	}
	#else
	audio_ctrl.wi = audio_write_buffer(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	#endif

	ret = a1800dec_parsing((void *)audio_ctrl.work_mem, audio_ctrl.wi);
	if(ret != 1)
	{
		return AUDIO_ERR_DEC_FAIL;
	}

	audio_ctrl.ri = a1800dec_read_index((INT8U *)audio_ctrl.work_mem);
	audio_ctrl.file_cnt += (audio_ctrl.ri - in_length);
	if(in_length > audio_ctrl.ri) {
		audio_ctrl.file_cnt += A18_DEC_BITSTREAM_BUFFER_SIZE;
	}

	in_length = A18_dec_get_samplerate((CHAR*)audio_ctrl.work_mem);

	dac_sample_rate_set(in_length);	//a1800 is 16khz sample rate
	dac_mono_set();//a1800 is mono
	channel = 1;
	g_audio_sample_rate = in_length;	//20090209 Roy
	DBG_PRINT("bps: %d\r\n",A18_dec_get_bitrate((void *)audio_ctrl.work_mem));
	DBG_PRINT("sample rate: %d\r\n",A18_dec_get_samplerate((CHAR*)audio_ctrl.work_mem));

	for (count=0;count<dac_buf_nums;count++) {
		OSQPost(audio_wq, (void *) count);
	}

	return AUDIO_ERR_NONE;
}

INT32S  audio_a1800_process(void)
{
	INT32S pcm_point;
	INT32U  in_length;
	INT8U   err;
	INT32U  wb_idx;
	#if READING_BY_FILE_SRV == 1
	INT8U   wait;
	INT32S  t_wi;
	#endif

	audio_ctrl.ri = a1800dec_read_index((INT8U *)audio_ctrl.work_mem);//word to byte

	#if READING_BY_FILE_SRV == 1
	t_wi = audio_write_with_file_srv(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	#else
	audio_ctrl.wi = audio_write_buffer(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	#endif
	
	wb_idx = (INT32U) OSQPend(audio_wq, 0, &err);
	
	#if READING_BY_FILE_SRV == 1
	wait = 0;
	if (t_wi == AUDIO_READ_WAIT) {
		wait = 1; /* pend until read finish */
	}
	if (audio_check_wi(t_wi, &audio_ctrl.wi, wait) != AUDIO_ERR_NONE) { /* check reading data */
		return AUDIO_ERR_READ_FAIL;
	}
	#endif

	if(audio_ctrl.file_cnt >= audio_ctrl.file_len)
	{
			return AUDIO_ERR_DEC_FINISH;
	}

	in_length = audio_ctrl.ri;
	pcm_point = a1800dec_run((CHAR*)audio_ctrl.work_mem, audio_ctrl.wi, pcm_out[wb_idx]);

	audio_ctrl.ri = a1800dec_read_index((CHAR*)audio_ctrl.work_mem);//word to byte
	audio_ctrl.file_cnt += (audio_ctrl.ri - in_length);

	if(in_length > audio_ctrl.ri) {
		audio_ctrl.file_cnt += A18_DEC_BITSTREAM_BUFFER_SIZE;
	}

	if (pcm_point <= 0)
	{
		if (pcm_point < 0)
		{
			if (--audio_ctrl.try_cnt == 0)
			{
				return AUDIO_ERR_DEC_FAIL;
			}
		}
		OSQPostFront(audio_wq, (void *)wb_idx);
		audio_send_next_frame_q();
		return 0;
	}

	pcm_len[wb_idx] = pcm_point; /* 1 channel */
	OSQPost(aud_send_q, (void *) wb_idx);
	audio_send_to_dma();
	audio_send_next_frame_q();
	return 0;
}
#endif // #if APP_A1800_DECODE_FG_EN == 1

//-----------------------------------------------------------------
// added by Bruce, 2008/09/23
//===============================================================================================================
//   A1600 Playback
//===============================================================================================================
#if APP_A1600_DECODE_FG_EN == 1
INT32S audio_a16_play_init(void)
{
	INT32U  count;
	INT32S  ret = 0;
	INT32U  in_length;
	INT32S  t_wi;

	audio_ctrl.file_cnt = 0;
	audio_ctrl.f_last = 0;
	audio_ctrl.try_cnt = 20;
	audio_ctrl.read_secs = 0;//080724

	audio_ctrl.ring_size = A16_DEC_BITSTREAM_BUFFER_SIZE;

	ret = A16_dec_get_mem_block_size();
	if(ret != A16_DEC_MEMORY_SIZE)
		return AUDIO_ERR_DEC_FAIL;

	ret = A16_dec_init((char *)audio_ctrl.work_mem, (unsigned char *)audio_ctrl.ring_buf);//080723
	audio_ctrl.wi = A16_dec_get_ri((void *)audio_ctrl.work_mem);//after initial the value is 0
	audio_ctrl.ri = A16_dec_get_ri((void *)audio_ctrl.work_mem);
	//in_length = (audio_ctrl.ri*2);//word to byte
	in_length = audio_ctrl.ri;

	#if READING_BY_FILE_SRV == 1
	t_wi = audio_write_with_file_srv(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	if (audio_check_wi(t_wi, &audio_ctrl.wi, 1) != AUDIO_ERR_NONE) { /* check reading data */
		return AUDIO_ERR_READ_FAIL;
	}
	#else
	audio_ctrl.wi = audio_write_buffer(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	#endif

	count = 500;
	while(1)
	{
		in_length = audio_ctrl.ri;
		ret = A16_dec_parsing((CHAR*)audio_ctrl.work_mem , audio_ctrl.wi);

		audio_ctrl.ri = A16_dec_get_ri((CHAR*)audio_ctrl.work_mem);

		audio_ctrl.file_cnt += (audio_ctrl.ri - in_length);
		if(audio_ctrl.ri < in_length) {
			audio_ctrl.file_cnt += A16_DEC_BITSTREAM_BUFFER_SIZE;
		}

		switch(ret)
		{
			case A16_OK:
				break;
			case A16_E_NO_MORE_SRCDATA:		//not found sync word
			case A16_E_READ_IN_BUFFER:	//reserved sample frequency value
			case A16_CODE_FILE_FORMAT_ERR:		//forbidden bitrate value
			case A16_E_FILE_END:
			case A16_E_MODE_ERR:
				//feed in DecodeInBuffer;
				audio_ctrl.ri = A16_dec_get_ri((CHAR*)audio_ctrl.work_mem);
				//audio_ctrl.wi = audio_write_buffer(audio_ctrl.file_handle, audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);

				#if READING_BY_FILE_SRV == 1
				t_wi = audio_write_with_file_srv(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
				if (audio_check_wi(t_wi, &audio_ctrl.wi, 1) != AUDIO_ERR_NONE) { /* check reading data */
					return AUDIO_ERR_READ_FAIL;
				}
				#else
				audio_ctrl.wi = audio_write_buffer(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
				#endif

				if (--count == 0)
					return AUDIO_ERR_FAILED;
				continue;
			default:
				return AUDIO_ERR_FAILED;
		}
		if(ret == A16_OK) {
			break;
		}
	}

	in_length = A16_dec_get_samplerate((CHAR*)audio_ctrl.work_mem);

	dac_mono_set();
	dac_sample_rate_set(in_length);
	channel = 1;
	g_audio_sample_rate = in_length;	//20090209 Roy
	DBG_PRINT("bps: %d\r\n",A16_dec_get_bitspersample((CHAR*)audio_ctrl.work_mem));
	DBG_PRINT("channel: %d\r\n",A16_dec_get_channel((CHAR*)audio_ctrl.work_mem));
	DBG_PRINT("sample rate: %d\r\n", A16_dec_get_samplerate((CHAR*)audio_ctrl.work_mem));
	DBG_PRINT("block size: %d\r\n",A16_dec_get_mem_block_size());

	for (count=0;count<dac_buf_nums;count++) {
		OSQPost(audio_wq, (void *) count);
	}

	return AUDIO_ERR_NONE;
}

INT32S  audio_a16_process(void)
{
	INT32S  pcm_point;
	INT32U  in_length;
	INT8U   err;
	INT32U  wb_idx;
	#if READING_BY_FILE_SRV == 1
	INT8U   wait;
	INT32S  t_wi;
	#endif

	audio_ctrl.ri = A16_dec_get_ri((void *)audio_ctrl.work_mem);
	//audio_ctrl.ri = A16_dec_get_ri((INT8U *)audio_ctrl.work_mem);

	#if READING_BY_FILE_SRV == 1
	t_wi = audio_write_with_file_srv(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	#else
	audio_ctrl.wi = audio_write_buffer(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	#endif
	
	wb_idx = (INT32U) OSQPend(audio_wq, 0, &err);
	
	#if READING_BY_FILE_SRV == 1
	wait = 0;
	if (t_wi == AUDIO_READ_WAIT) {
		wait = 1; /* pend until read finish */
	}
	if (audio_check_wi(t_wi, &audio_ctrl.wi, wait) != AUDIO_ERR_NONE) { /* check reading data */
		return AUDIO_ERR_READ_FAIL;
	}
	#endif


	if(audio_ctrl.file_cnt >= audio_ctrl.file_len)
	{
		//if(audio_ctrl.f_last){
			return AUDIO_ERR_DEC_FINISH;
		//}
		//else{
		//	audio_ctrl.f_last = 1;
		//}
	}

	in_length = audio_ctrl.ri;
	//pcm_point = a1800dec_run((CHAR*)audio_ctrl.work_mem, audio_ctrl.wi/2, pcm_out[wb_idx]);
	pcm_point = A16_dec_run((CHAR*)audio_ctrl.work_mem, pcm_out[wb_idx], audio_ctrl.wi);

	audio_ctrl.ri = A16_dec_get_ri((CHAR*)audio_ctrl.work_mem);
	audio_ctrl.file_cnt += (audio_ctrl.ri - in_length);

	if(in_length > audio_ctrl.ri) {
		audio_ctrl.file_cnt += A16_DEC_BITSTREAM_BUFFER_SIZE;
	}

	if (pcm_point <= 0)
	{
		if (pcm_point < 0)
		{
			if (--audio_ctrl.try_cnt == 0)
			{
				return AUDIO_ERR_DEC_FAIL;
			}
		}
		OSQPostFront(audio_wq, (void *)wb_idx);
		audio_send_next_frame_q();
		return 0;
	}

	pcm_len[wb_idx] = pcm_point;
	OSQPost(aud_send_q, (void *) wb_idx);
	audio_send_to_dma();
	audio_send_next_frame_q();
	return 0;
}
#endif	// #if APP_A1600_DECODE_FG_EN == 1

//===============================================================================================================
//   A6400 Playback
//===============================================================================================================
#if APP_A6400_DECODE_FG_EN == 1
INT32S audio_a64_play_init(void)
{
	INT32U  count;
	INT32S  ret = 0;
	INT32U  in_length;
	INT32S  t_wi;

	audio_ctrl.file_cnt = 0;
	audio_ctrl.f_last = 0;
	audio_ctrl.try_cnt = 20;
	audio_ctrl.read_secs = 0;

	audio_ctrl.ring_size = A6400_DEC_BITSTREAM_BUFFER_SIZE;

	ret = a6400_dec_get_mem_block_size();
	if(ret != A6400_DEC_MEMORY_SIZE)
		return AUDIO_ERR_DEC_FAIL;

	ret = a6400_dec_init((void *)audio_ctrl.work_mem, (char *)audio_ctrl.ring_buf);
	audio_ctrl.wi = a6400_dec_get_ri((void *)audio_ctrl.work_mem);//after initial the value is 0
	audio_ctrl.ri = a6400_dec_get_ri((void *)audio_ctrl.work_mem);
	//in_length = (audio_ctrl.ri*2);//word to byte
	in_length = audio_ctrl.ri;

	#if READING_BY_FILE_SRV == 1
	t_wi = audio_write_with_file_srv(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	if (audio_check_wi(t_wi, &audio_ctrl.wi, 1) != AUDIO_ERR_NONE) { /* check reading data */
		return AUDIO_ERR_READ_FAIL;
	}
	#else
	audio_ctrl.wi = audio_write_buffer(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	#endif

	count = 500;
	while(1)
	{
		in_length = audio_ctrl.ri;
		ret = a6400_dec_parsing((CHAR*)audio_ctrl.work_mem , audio_ctrl.wi);

		audio_ctrl.ri = a6400_dec_get_ri((CHAR*)audio_ctrl.work_mem);

		audio_ctrl.file_cnt += (audio_ctrl.ri - in_length);
		if(audio_ctrl.ri < in_length) {
			audio_ctrl.file_cnt += A6400_DEC_BITSTREAM_BUFFER_SIZE;
		}

		switch(ret)
		{
			case A6400_DEC_ERR_NONE:
				break;
			case A6400_DEC_ERR_LOSTSYNC:		//not found sync word
			case A6400_DEC_ERR_BADSAMPLERATE:	//reserved sample frequency value
			case A6400_DEC_ERR_BADBITRATE:		//forbidden bitrate value
			case A6400_DEC_ERR_BADLAYER:
			case A6400_DEC_ERR_BADMPEGID:
				//feed in DecodeInBuffer;
				audio_ctrl.ri = a6400_dec_get_ri((CHAR*)audio_ctrl.work_mem);

				#if READING_BY_FILE_SRV == 1
				t_wi = audio_write_with_file_srv(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
				if (audio_check_wi(t_wi, &audio_ctrl.wi, 1) != AUDIO_ERR_NONE) { /* check reading data */
					return AUDIO_ERR_READ_FAIL;
				}
				#else
				audio_ctrl.wi = audio_write_buffer(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
				#endif

				if (--count == 0)
					return AUDIO_ERR_FAILED;
				continue;
			default:
				return AUDIO_ERR_FAILED;
		}
		if(ret == A6400_DEC_ERR_NONE) {
			break;
		}
	}

	in_length = a6400_dec_get_samplerate((CHAR*)audio_ctrl.work_mem);
	dac_stereo_set();

	dac_sample_rate_set(in_length);
	channel = 2;
	g_audio_sample_rate = in_length;	//20090209 Roy
	DBG_PRINT("bps: %d\r\n",a6400_dec_get_bitrate((CHAR*)audio_ctrl.work_mem));
	DBG_PRINT("channel: %d\r\n",a6400_dec_get_channel((CHAR*)audio_ctrl.work_mem));
	DBG_PRINT("sample rate: %d\r\n",a6400_dec_get_samplerate((CHAR*)audio_ctrl.work_mem));
	DBG_PRINT("block size: %d\r\n",a6400_dec_get_mem_block_size());

	for (count=0;count<dac_buf_nums;count++) {
		OSQPost(audio_wq, (void *) count);
	}

	return AUDIO_ERR_NONE;
}

INT32S  audio_a64_process(void)
{
	INT32S  pcm_point;
	INT32U  in_length;
	INT8U   err;
	INT32U  wb_idx;
	#if READING_BY_FILE_SRV == 1
	INT8U   wait;
	INT32S  t_wi;
	#endif

	audio_ctrl.ri = a6400_dec_get_ri((CHAR*)audio_ctrl.work_mem);

	#if READING_BY_FILE_SRV == 1
	t_wi = audio_write_with_file_srv(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	#else
	audio_ctrl.wi = audio_write_buffer(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	#endif
	
	wb_idx = (INT32U) OSQPend(audio_wq, 0, &err);
	
	#if READING_BY_FILE_SRV == 1
	wait = 0;
	if (t_wi == AUDIO_READ_WAIT) {
		wait = 1; /* pend until read finish */
	}
	if (audio_check_wi(t_wi, &audio_ctrl.wi, wait) != AUDIO_ERR_NONE) { /* check reading data */
		return AUDIO_ERR_READ_FAIL;
	}
	#endif

	if(audio_ctrl.file_cnt >= audio_ctrl.file_len)
	{
		if(audio_ctrl.f_last){
			return AUDIO_ERR_DEC_FINISH;
		}
		else{
			audio_ctrl.f_last = 1;
		}
	}


	in_length = audio_ctrl.ri;

	pcm_point = a6400_dec_run((CHAR*)audio_ctrl.work_mem, pcm_out[wb_idx], audio_ctrl.wi,audio_ctrl.f_last);

	audio_ctrl.ri = a6400_dec_get_ri((CHAR*)audio_ctrl.work_mem);
	audio_ctrl.file_cnt += (audio_ctrl.ri - in_length);

	if(in_length > audio_ctrl.ri) {
		audio_ctrl.file_cnt += A6400_DEC_BITSTREAM_BUFFER_SIZE;
	}

	if (pcm_point <= 0) {
		if (pcm_point < 0) {
			if (--audio_ctrl.try_cnt == 0) {
				return AUDIO_ERR_DEC_FINISH;
			}
		}
		OSQPostFront(audio_wq, (void *)wb_idx);
		audio_send_next_frame_q();
		return 0;
	}

	pcm_len[wb_idx] = pcm_point*2;

	OSQPost(aud_send_q, (void *) wb_idx);
	audio_send_to_dma();
	audio_send_next_frame_q();
	return 0;
}
#endif // #if APP_A6400_DECODE_FG_EN == 1

//===============================================================================================================
//   S880 Playback
//===============================================================================================================
#if APP_S880_DECODE_FG_EN == 1
INT32S audio_s880_play_init(void)
{
	INT32U  count;
	INT32S  ret = 0;
	INT32U  in_length;
	INT32S  t_wi;

	audio_ctrl.file_cnt = 0;
	audio_ctrl.f_last = 0;
	audio_ctrl.try_cnt = 20;
	audio_ctrl.read_secs = 0;//080724

	audio_ctrl.ring_size = S880_DEC_BITSTREAM_BUFFER_SIZE;

	ret = S880_dec_get_mem_block_size();
	if(ret != S880_DEC_MEMORY_SIZE)
		return AUDIO_ERR_DEC_FAIL;

	ret = S880_dec_init((char *)audio_ctrl.work_mem, (unsigned char *)audio_ctrl.ring_buf);//080723
	audio_ctrl.wi = S880_dec_get_ri((void *)audio_ctrl.work_mem);//after initial the value is 0
	audio_ctrl.ri = S880_dec_get_ri((void *)audio_ctrl.work_mem);
	//in_length = (audio_ctrl.ri*2);//word to byte
	in_length = audio_ctrl.ri;

	#if READING_BY_FILE_SRV == 1
	t_wi = audio_write_with_file_srv(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	if (audio_check_wi(t_wi, &audio_ctrl.wi, 1) != AUDIO_ERR_NONE) { /* check reading data */
		return AUDIO_ERR_READ_FAIL;
	}
	#else
	audio_ctrl.wi = audio_write_buffer(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	#endif

	count = 500;
	while(1)
	{
		in_length = audio_ctrl.ri;
		ret = S880_dec_parsing((CHAR*)audio_ctrl.work_mem , audio_ctrl.wi);

		audio_ctrl.ri = S880_dec_get_ri((CHAR*)audio_ctrl.work_mem);

		audio_ctrl.file_cnt += (audio_ctrl.ri - in_length);
		if(audio_ctrl.ri < in_length) {
			audio_ctrl.file_cnt += S880_DEC_BITSTREAM_BUFFER_SIZE;
		}

		switch(ret)
		{
			case S880_OK:
				break;
			case S880_E_NO_MORE_SRCDATA:		//not found sync word
			case S880_E_READ_IN_BUFFER:	//reserved sample frequency value
			case S880_CODE_FILE_FORMAT_ERR:		//forbidden bitrate value
			case S880_E_FILE_END:
		//	case S880_E_MODE_ERR:
				//feed in DecodeInBuffer;
				audio_ctrl.ri = S880_dec_get_ri((CHAR*)audio_ctrl.work_mem);
				//audio_ctrl.wi = audio_write_buffer(audio_ctrl.file_handle, audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);

				#if READING_BY_FILE_SRV == 1
				t_wi = audio_write_with_file_srv(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
				if (audio_check_wi(t_wi, &audio_ctrl.wi, 1) != AUDIO_ERR_NONE) { /* check reading data */
					return AUDIO_ERR_READ_FAIL;
				}
				#else
				audio_ctrl.wi = audio_write_buffer(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
				#endif

				if (--count == 0)
					return AUDIO_ERR_FAILED;
				continue;
			default:
				return AUDIO_ERR_FAILED;
		}
		if(ret == S880_OK) {
			break;
		}
	}

	in_length = S880_dec_get_samplerate((CHAR*)audio_ctrl.work_mem);

	dac_mono_set();
	dac_sample_rate_set(in_length);
	channel = 1;
	g_audio_sample_rate = in_length;	//20090209 Roy
	DBG_PRINT("bps: %d\r\n",S880_dec_get_bitspersample((CHAR*)audio_ctrl.work_mem));
	DBG_PRINT("channel: %d\r\n",S880_dec_get_channel((CHAR*)audio_ctrl.work_mem));
	DBG_PRINT("sample rate: %d\r\n", ret);
	DBG_PRINT("block size: %d\r\n",S880_dec_get_mem_block_size());

	for (count=0;count<dac_buf_nums;count++) {
		OSQPost(audio_wq, (void *) count);
	}

	return AUDIO_ERR_NONE;
}

INT32S  audio_s880_process(void)
{
	INT32S  pcm_point;
	INT32U  in_length;
	INT8U   err;
	INT32U  wb_idx;
	#if READING_BY_FILE_SRV == 1
	INT8U   wait;
	INT32S  t_wi;
	#endif

	audio_ctrl.ri = S880_dec_get_ri((void *)audio_ctrl.work_mem);
	//audio_ctrl.ri = S880_dec_get_ri((INT8U *)audio_ctrl.work_mem);

	#if READING_BY_FILE_SRV == 1
	t_wi = audio_write_with_file_srv(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	#else
	audio_ctrl.wi = audio_write_buffer(audio_ctrl.ring_buf, audio_ctrl.wi, audio_ctrl.ri);
	#endif
	
	wb_idx = (INT32U) OSQPend(audio_wq, 0, &err);
	
	#if READING_BY_FILE_SRV == 1
	wait = 0;
	if (t_wi == AUDIO_READ_WAIT) {
		wait = 1; /* pend until read finish */
	}
	if (audio_check_wi(t_wi, &audio_ctrl.wi, wait) != AUDIO_ERR_NONE) { /* check reading data */
		return AUDIO_ERR_READ_FAIL;
	}
	#endif


	if(audio_ctrl.file_cnt >= audio_ctrl.file_len)
	{
		//if(audio_ctrl.f_last){
			return AUDIO_ERR_DEC_FINISH;
		//}
		//else{
		//	audio_ctrl.f_last = 1;
		//}
	}

	in_length = audio_ctrl.ri;
	//pcm_point = a1800dec_run((CHAR*)audio_ctrl.work_mem, audio_ctrl.wi/2, pcm_out[wb_idx]);
	pcm_point = S880_dec_run((CHAR*)audio_ctrl.work_mem, pcm_out[wb_idx], audio_ctrl.wi);

	audio_ctrl.ri = S880_dec_get_ri((CHAR*)audio_ctrl.work_mem);
	audio_ctrl.file_cnt += (audio_ctrl.ri - in_length);

	if(in_length > audio_ctrl.ri) {
		audio_ctrl.file_cnt += S880_DEC_BITSTREAM_BUFFER_SIZE;
	}

	if (pcm_point <= 0)
	{
		if (pcm_point < 0)
		{
			if (--audio_ctrl.try_cnt == 0)
			{
				return AUDIO_ERR_DEC_FAIL;
			}
		}
		OSQPostFront(audio_wq, (void *)wb_idx);
		audio_send_next_frame_q();
		return 0;
	}

	pcm_len[wb_idx] = pcm_point;
	OSQPost(aud_send_q, (void *) wb_idx);
	audio_send_to_dma();
	audio_send_next_frame_q();
	return 0;
}
#endif // #if APP_S880_DECODE_FG_EN == 1

#if (defined SKIP_ID3_TAG) && (SKIP_ID3_TAG == 1)
static INT8U audio_get_id3_type(INT8U *data, INT32U length)
{
	if (length >= 3 && data[0] == 'T' && data[1] == 'A' && data[2] == 'G') {
    	return ID3_TAG_V1;
	}

  	if (length >= 10 && ((data[0] == 'I' && data[1] == 'D' && data[2] == '3') ||
      	(data[0] == '3' && data[1] == 'D' && data[2] == 'I')) && data[3] < 0xff && data[4] < 0xff)
   	{
   		if (data[0] == 'I')
   			return ID3_TAG_V2;
   		else
   			return ID3_TAG_V2_FOOTER;
	}
  	return ID3_TAG_NONE;
}

static void audio_parse_id3_header(INT8U *header, INT32U *version, INT32S *flags, INT32U *size)
{
	INT8U *ptr;
	INT32U ver = 0;
	ptr = header;
  	ptr += 3;

  	ver = *ptr++ << 8;
  	*version = ver | *ptr++;
  	*flags   = *ptr++;
  	*size    = audio_id3_get_size(ptr);

}

static INT32U audio_id3_get_size(INT8U *ptr)
{
	INT32U value = 0;

	value = (value << 7) | (*ptr++ & 0x7f);
    value = (value << 7) | (*ptr++ & 0x7f);
	value = (value << 7) | (*ptr++ & 0x7f);
	value = (value << 7) | (*ptr++ & 0x7f);

  	return value;
}

INT32S audio_id3_get_tag_len(INT8U *data, INT32U length)
{
  INT32U version;
  INT32S flags;
  INT32U len;

  switch (audio_get_id3_type(data, length)) {
 	case ID3_TAG_V1:
    	return 128;
  	case ID3_TAG_V2:
    	audio_parse_id3_header(data, &version, &flags, &len);
    	if (flags & ID3_TAG_FLAG_FOOTER) {
      		len += 10;
		}
    	return 10 + len;
  	case ID3_TAG_NONE:
    	break;
  }

  return 0;
}
#endif

