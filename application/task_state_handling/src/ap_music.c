#include "ap_music.h"
#include "ap_state_config.h"
#include "ap_video_record.h"
#include "LDWs.h"
#define 	LDW_SND_FROM_SDRAM 	Enable_Lane_Departure_Warning_System

STAudioTaskPara music_para;

INT32S music_file_idx;
INT32U fm_ch_freq;
INT32U next_prev_start;
INT8U  pre_next_state = 0;
STORAGE_FINFO audio_finfo;
audio_status_st aud_status;

INT16U camera_fd;
INT16U click_fd;
INT16U on_fd;
INT16U off_fd;
INT16U lock_fd;
INT16U beep_fd;
INT16U ldw_off;
INT16U ldw_on;
INT16U ldw_alarm;
LDW_Fmt LDW[LDW_SIZE];

static void audio_send_play(void);
static void audio_next(void);
static void audio_prev(void);
INT32S ap_music_index_get(void);
INT32S storage_file_nums_get(void);
INT8U audio_fg_vol_get(void);
INT32U audio_fm_freq_ch_get(void);
void ap_music_update_icon_status(void);
void ap_music_reset(void);

void ap_music_init(void)
{
	music_file_idx = ap_state_music_get();
	if(music_file_idx >= storage_file_nums_get()){
		music_file_idx = 0;
	}
	aud_status.aud_state = STATE_IDLE;
	aud_status.volume = ap_state_config_volume_get();
	aud_status.play_style = ap_state_music_play_mode_get();
	audio_vol_set(aud_status.volume);
	
//	SetFrequency(ap_state_music_fm_ch_get());
//	fm_ch_freq = ap_state_music_fm_ch_get();
#if 0
	if(ap_state_music_play_onoff_get()) {
		audio_mute_ctrl_set(FALSE);
	}
	else {
		audio_mute_ctrl_set(TRUE);
	}
#endif
}

void ap_music_effect_resource_init(void)
{
    camera_fd = nv_open((INT8U *) "CAMERA.WAV");
    click_fd = nv_open((INT8U *) "CLICK.WAV");
    on_fd = nv_open((INT8U *) "POWERON_AUDIO.WAV");
    off_fd = nv_open((INT8U *) "POWEROFF_AUDIO.WAV");
//    lock_fd = nv_open((INT8U *) "LOCK.WAV");
    beep_fd = nv_open((INT8U *) "BEEP.WAV");
    ldw_on = nv_open((INT8U *) "LDW_TURNON.WAV");
    ldw_off = nv_open((INT8U *) "LDW_TURNOFF.WAV");
    ldw_alarm = nv_open((INT8U *) "LDW_ALARM.WAV");


    #if LDW_SND_FROM_SDRAM
    {
	INT32U ldw_sdram_size;
	INT8U *ptr;
	LDW[0].length = nv_rs_size_get(ldw_on);
	LDW[1].length = nv_rs_size_get(ldw_off);
	LDW[2].length = nv_rs_size_get(ldw_alarm);

	ldw_sdram_size = LDW[0].length + LDW[1].length + LDW[2].length;
	ptr = (INT8U*)gp_malloc(ldw_sdram_size);
	if (ptr!=NULL)
	{
		LDW[0].start_addr = (INT32U)ptr;
		LDW[1].start_addr = LDW[0].start_addr+LDW[0].length;
		LDW[2].start_addr = LDW[1].start_addr+LDW[1].length;
	}
	else
	{
		LDW[0].start_addr = NULL;
		LDW[1].start_addr = NULL;
		LDW[2].start_addr = NULL;
		DBG_PRINT("LDW_SDRAM alloc fail \r\n");
	}

	nv_read(ldw_on, LDW[0].start_addr,LDW[0].length);	
	nv_read(ldw_off, LDW[1].start_addr,LDW[1].length);	
	nv_read	(ldw_alarm, LDW[2].start_addr,LDW[2].length);
    }
    #endif	
}

void ap_music_reset(void)
{
	music_file_idx = ap_state_music_get();
	if(music_file_idx >= storage_file_nums_get()){
		music_file_idx = 0;
	}	
	aud_status.aud_state = STATE_IDLE;
	msgQSend(AudioTaskQ, MSG_AUD_STOP, NULL, 0, MSG_PRI_NORMAL);
	aud_status.volume = ap_state_config_volume_get();
	aud_status.play_style = ap_state_music_play_mode_get();
	audio_vol_set(aud_status.volume);
//	SetFrequency(ap_state_music_fm_ch_get());
//	fm_ch_freq = ap_state_music_fm_ch_get();
	ap_music_update_icon_status();	
}

void audio_play_process(void)
{
	if(storage_file_nums_get() != 0)
	{
		if (aud_status.aud_state == STATE_IDLE) {
			msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_STOP, NULL, NULL, MSG_PRI_NORMAL);
			audio_send_play();	
			return;
		}
	}
}

void audio_play_pause_process(void)
{
	if(storage_file_nums_get() != 0)
	{
		if(aud_status.aud_state == STATE_PAUSED) {
			aud_status.aud_state = STATE_PLAY;
			msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_STOP, NULL, NULL, MSG_PRI_NORMAL);
			msgQSend(AudioTaskQ, MSG_AUD_RESUME, NULL, 0, MSG_PRI_NORMAL);
			ap_state_handling_icon_clear_cmd(ICON_MP3_PAUSE, ICON_MP3_STOP, NULL);
			ap_state_handling_icon_show_cmd(ICON_MP3_PLAY, NULL, NULL);
			return;
		}
		if(aud_status.aud_state == STATE_PLAY) {
			aud_status.aud_state = STATE_PAUSED;
			msgQSend(AudioTaskQ, MSG_AUD_PAUSE, NULL, 0, MSG_PRI_NORMAL);
			if ((ap_video_record_sts_get() & 0x2) == 0) {		//VIDEO_RECORD_BUSY
				msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
			}
			ap_state_handling_icon_clear_cmd(ICON_MP3_PLAY, ICON_MP3_STOP, NULL);
			ap_state_handling_icon_show_cmd(ICON_MP3_PAUSE, NULL, NULL);
			return;
		}
		if ((aud_status.aud_state == STATE_IDLE) ||(aud_status.aud_state == STATE_AUD_FILE_ERR) ) {
			msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_STOP, NULL, NULL, MSG_PRI_URGENT);
			audio_send_play();
			ap_state_handling_icon_clear_cmd(ICON_MP3_PAUSE, ICON_MP3_STOP, NULL);
			ap_state_handling_icon_show_cmd(ICON_MP3_PLAY, NULL, NULL);
			return;
		}
	}
}

void audio_play_style_set(INT8U play_style)
{
	aud_status.play_style = play_style;
}

INT8U  audio_play_style_get( void )
{
	return aud_status.play_style ;
}

void audio_next_process(void)
{
	if(storage_file_nums_get() != 0)
	{
		if ((aud_status.aud_state == STATE_PLAY) || (aud_status.aud_state == STATE_PAUSED)) {
			audio_next();
			audio_send_stop();
			next_prev_start = 1;
			ap_state_handling_icon_clear_cmd(ICON_MP3_PAUSE, ICON_MP3_STOP, NULL);
			ap_state_handling_icon_show_cmd(ICON_MP3_PLAY, NULL, NULL);
		}
		else {
			audio_next();
		}
//		ap_state_handling_mp3_index_show_cmd();
	}
}


void audio_prev_process(void)
{
	if(storage_file_nums_get() != 0)
	{	
		if ((aud_status.aud_state == STATE_PLAY) || (aud_status.aud_state == STATE_PAUSED)) {
			audio_prev();
			audio_send_stop();
			next_prev_start = 1;
			ap_state_handling_icon_clear_cmd(ICON_MP3_PAUSE, ICON_MP3_STOP, NULL);
			ap_state_handling_icon_show_cmd(ICON_MP3_PLAY, NULL, NULL);		
		}
		else {
			audio_prev();
		}
//		ap_state_handling_mp3_index_show_cmd();
	}
}

void audio_mute_ctrl_set(BOOLEAN status)
{
	if (status == TRUE) {
		music_para.mute = TRUE;
	}
	else {
		if (aud_status.volume == 0) {
			music_para.volume = 0;
		}
		else {
			music_para.volume = aud_status.volume*VOLUME_STEP-1;
		}
		DBG_PRINT("volume = %d\r\n",music_para.volume);
		music_para.mute = FALSE;
	}
	msgQSend(AudioTaskQ, MSG_AUD_SET_MUTE, (void *)&music_para, sizeof(STAudioTaskPara), MSG_PRI_NORMAL);
}

void audio_vol_set(INT8U vol)
{
	if (vol > MAX_VOLUME_LEVEL) {
		return;
	}

	if (vol == 0) {
		music_para.volume = 0;
		ap_state_handling_icon_clear_cmd(ICON_MP3_VOLUME, NULL, NULL);
		ap_state_handling_icon_show_cmd(ICON_MP3_MUTE, NULL, NULL);		
	}
	else {
		music_para.volume = vol*VOLUME_STEP-1;
		ap_state_handling_icon_clear_cmd(ICON_MP3_MUTE, NULL, NULL);
		ap_state_handling_icon_show_cmd(ICON_MP3_VOLUME, NULL, NULL);			
	}
	aud_status.volume = vol;
	ap_state_config_volume_set(aud_status.volume);
//	ap_state_handling_mp3_volume_show_cmd();
	DBG_PRINT("volume = %d\r\n",music_para.volume);
	msgQSend(AudioTaskQ, MSG_AUD_VOLUME_SET, (void *)&music_para, sizeof(STAudioTaskPara), MSG_PRI_NORMAL);
}

void audio_vol_inc_set(void)
{
	aud_status.volume = ap_state_config_volume_get();
	if (aud_status.volume < MAX_VOLUME_LEVEL) {
		aud_status.volume++;
		music_para.volume = aud_status.volume*VOLUME_STEP-1;
		ap_state_handling_icon_clear_cmd(ICON_MP3_MUTE, NULL, NULL);
		ap_state_handling_icon_show_cmd(ICON_MP3_VOLUME, NULL, NULL);
//		ap_state_handling_mp3_volume_show_cmd();	
		DBG_PRINT("volume = %d\r\n",music_para.volume);
		ap_state_config_volume_set(aud_status.volume);
		msgQSend(AudioTaskQ, MSG_AUD_VOLUME_SET, (void *)&music_para, sizeof(STAudioTaskPara), MSG_PRI_NORMAL);
	}
}

void audio_vol_dec_set(void)
{
	aud_status.volume = ap_state_config_volume_get();
	
	if (aud_status.volume > 0) {
		aud_status.volume--;
		if (aud_status.volume == 0) {
			music_para.volume = 0;
			ap_state_handling_icon_clear_cmd(ICON_MP3_VOLUME, NULL, NULL);
			ap_state_handling_icon_show_cmd(ICON_MP3_MUTE, NULL, NULL);				
		}
		else {
			music_para.volume = aud_status.volume*VOLUME_STEP-1;
			ap_state_handling_icon_clear_cmd(ICON_MP3_MUTE, NULL, NULL);
			ap_state_handling_icon_show_cmd(ICON_MP3_VOLUME, NULL, NULL);			
		}
//		ap_state_handling_mp3_volume_show_cmd();
		DBG_PRINT("volume = %d\r\n",music_para.volume);
		ap_state_config_volume_set(aud_status.volume);
		msgQSend(AudioTaskQ, MSG_AUD_VOLUME_SET, (void *)&music_para, sizeof(STAudioTaskPara), MSG_PRI_NORMAL);
	}
}

INT8U audio_fg_vol_get(void)
{
	return aud_status.volume;
}

void audio_set_mp3_type()
{
	music_para.audio_format = AUDIO_TYPE_MP3;
}

INT32S audio_effect_play(INT32U effect_type)
{
	if(ap_state_config_beep_sound_get()==0){
		return -1;
	}
    if (effect_type == EFFECT_CLICK) {
        music_para.fd = click_fd;
    } else if (effect_type == EFFECT_CAMERA) {
        music_para.fd = camera_fd;
    } else if (effect_type == EFFECT_POWER_ON) {
        music_para.fd = on_fd;
    } else if (effect_type == EFFECT_POWER_OFF) {
        music_para.fd = on_fd;
//        music_para.fd = off_fd;
    } else if (effect_type == EFFECT_FILE_LOCK) {
        music_para.fd = lock_fd;
    } else if (effect_type == EFFECT_BEEP) {
        music_para.fd = beep_fd;
    } else if (effect_type == EFFECT_LDW_TurnOn) {
        music_para.fd = ldw_on;
    } else if (effect_type == EFFECT_LDW_TurnOff) {
        music_para.fd = ldw_off;
    } else if (effect_type == EFFECT_LDW_Alarm) {
        music_para.fd = ldw_alarm;		
    } else {
        return -1;
    }
    
    if (music_para.fd == 0xFFFF) {
        //DBG_PRINT("invalid effect file handle\r\n");
        return -1;
    }      

	music_para.src_type = AUDIO_SRC_TYPE_APP_RS;
	music_para.audio_format = AUDIO_TYPE_WAV;
	music_para.file_len = nv_rs_size_get(music_para.fd);
	DBG_PRINT("effect length = %d\r\n",music_para.file_len);
	aud_status.aud_state = STATE_PLAY;


	#if  LDW_SND_FROM_SDRAM
	// redirection
	if (LDW[0].start_addr != NULL)
	{
		switch(effect_type)
		{
			case EFFECT_LDW_TurnOn:
				music_para.fd = 0;
				LDW[0].cur_pos = 0;
				music_para.src_type = AUDIO_SRC_TYPE_SDRAM_LDW;			
				break;
			case EFFECT_LDW_TurnOff:
				music_para.fd = 1;
				LDW[1].cur_pos = 0;				
				music_para.src_type = AUDIO_SRC_TYPE_SDRAM_LDW;			
				break;
			case EFFECT_LDW_Alarm:
				music_para.fd = 2;
				LDW[2].cur_pos = 0;				
				music_para.src_type = AUDIO_SRC_TYPE_SDRAM_LDW;
				break;
		}
	}
	#endif
	
	msgQSend(AudioTaskQ, MSG_AUD_PLAY, (void *)&music_para, sizeof(STAudioTaskPara), MSG_PRI_NORMAL);
	
	return 0;
}

static void audio_send_play(void)
{
	INT32S ret;
	if(storage_file_nums_get() != 0)
	{	
		ret = storage_fopen(music_file_idx, &audio_finfo);
		if (ret == -2) {
			music_file_idx = 0;
			ret = storage_fopen(music_file_idx, &audio_finfo);
			if (ret != STATUS_OK) {
				audio_send_stop();
				msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
				return;
			}
		}	
		else if (ret == STATUS_FAIL) {
			audio_send_stop();
			msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
			return;
		}
		
		ap_state_music_set(music_file_idx);
		DBG_PRINT("play file : %s\r\n",audio_finfo.f_name);
		DBG_PRINT("file size : %d\r\n",audio_finfo.f_size);
		
		music_para.fd = audio_finfo.fd;
		music_para.src_type = AUDIO_SRC_TYPE_FS;
		aud_status.aud_state = STATE_PLAY;
		msgQSend(AudioTaskQ, MSG_AUD_PLAY, (void *)&music_para, sizeof(STAudioTaskPara), MSG_PRI_NORMAL);
	}
}

void audio_send_pause(void)
{
	if(storage_file_nums_get() != 0)
	{
		if(aud_status.aud_state != STATE_PLAY) {
			return;
		}
		aud_status.aud_state = STATE_PAUSED;
		msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
		msgQSend(AudioTaskQ, MSG_AUD_PAUSE, NULL, 0, MSG_PRI_NORMAL);
	}
}

void audio_send_resume(void)
{
	if(storage_file_nums_get() != 0)
	{
		if(aud_status.aud_state != STATE_PAUSED) {
			return;
		}
		aud_status.aud_state = STATE_PLAY;
		msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_STOP, NULL, NULL, MSG_PRI_NORMAL);
		msgQSend(AudioTaskQ, MSG_AUD_RESUME, NULL, 0, MSG_PRI_NORMAL);
	}
}

void audio_send_stop(void)
{
	if(storage_file_nums_get() != 0)
	{
		aud_status.aud_state = STATE_IDLE;
		msgQSend(AudioTaskQ, MSG_AUD_STOP, NULL, 0, MSG_PRI_NORMAL);
		ap_state_handling_icon_clear_cmd(ICON_MP3_PLAY, ICON_MP3_PAUSE, NULL);
		ap_state_handling_icon_show_cmd(ICON_MP3_STOP, NULL, NULL);
	}	
}

void audio_playing_mode_set_process(void)
{
	if(ap_state_music_play_mode_get() == PLAY_REPEAT) {
		DBG_PRINT("sequence mode\r\n"); 
		ap_state_music_play_mode_set(PLAY_SEQUENCE);
		audio_play_style_set(PLAY_SEQUENCE);
		ap_state_handling_icon_clear_cmd(ICON_MP3_PLAY_ONE, NULL, NULL);
		ap_state_handling_icon_show_cmd(ICON_MP3_PLAY_ALL, NULL, NULL);		
	}
	else {
		DBG_PRINT("repeat mode\r\n");
		audio_play_style_set(PLAY_REPEAT);
		ap_state_music_play_mode_set(PLAY_REPEAT);
		ap_state_handling_icon_clear_cmd(ICON_MP3_PLAY_ALL, NULL, NULL);
		ap_state_handling_icon_show_cmd(ICON_MP3_PLAY_ONE, NULL, NULL);			
	}	
}

void audio_quick_select(INT32U index)
{
	if(storage_file_nums_get() != 0)
	{	
		if (index == 0 || index > 9999) {
			return;
		}
		index--;

		if (ScanFileWait(&(FNodeInfo[SD_SLOT_ID].audio), (INT32S)index) == -2 ) {
			DBG_PRINT("wrong index\r\n");
			return;
		}
		music_file_idx = index;
//		ap_state_handling_mp3_index_show_cmd();
		ap_state_handling_icon_clear_cmd(ICON_MP3_PAUSE, ICON_MP3_STOP, NULL);
		ap_state_handling_icon_show_cmd(ICON_MP3_PLAY, NULL, NULL);	
		audio_send_play();
	}	
}

static void audio_next(void)
{
	pre_next_state = 0;
	if (++music_file_idx == storage_file_nums_get()) {
		music_file_idx = 0;
	}
}

static void audio_prev(void)
{
	pre_next_state = 1;
	if (--music_file_idx < 0) {
		music_file_idx = storage_file_nums_get() - 1;
	}
}
/*
void audio_fm_freq_inc_set(void)
{
	INT32U freq;
	
	freq = ap_state_music_fm_ch_get();
	freq++;
	if (freq > MAX_FM_FREQ) {
		freq = MIN_FM_FREQ;
	}
	fm_ch_freq = freq;
	ap_state_music_fm_ch_set(freq);
	ap_state_handling_mp3_FM_channel_show_cmd();
	DBG_PRINT("fm ch inc = %d\r\n",freq);
	SetFrequency(freq);
}

void audio_fm_freq_dec_set(void)
{
	INT32S freq;
	
	freq = ap_state_music_fm_ch_get();
	freq--;
	if (freq < MIN_FM_FREQ) {
		freq = MAX_FM_FREQ;
	}
	fm_ch_freq = freq;
	ap_state_music_fm_ch_set(freq);
	ap_state_handling_mp3_FM_channel_show_cmd();
	DBG_PRINT("fm ch dec = %d\r\n",freq);
	SetFrequency(freq);
}

void audio_fm_freq_quick_set(INT32U freq)
{
	if (freq > MAX_FM_FREQ || freq < MIN_FM_FREQ) {
		return;
	}
	fm_ch_freq = freq;
	ap_state_music_fm_ch_set(freq);
	ap_state_handling_mp3_FM_channel_show_cmd();
	DBG_PRINT("fm ch = %d\r\n",freq);
	SetFrequency(freq);
}

INT32U audio_fm_freq_ch_get(void)
{
	return fm_ch_freq;
}
*/
INT8U audio_playing_state_get(void)
{
	return aud_status.aud_state;	
}

void audio_res_play_process(INT32S result)
{
	switch(result) {
		case AUDIO_ERR_NONE:
			DBG_PRINT("state task - start play\r\n");
			return;
		case AUDIO_ERR_INVALID_FORMAT:
			DBG_PRINT("state task - invalid file format\r\n");
			break;
		case AUDIO_ERR_DEC_FINISH:
			DBG_PRINT("state task - decode finsish\r\n");
			break;
		case AUDIO_ERR_DEC_FAIL:
			DBG_PRINT("state task - decode fail\r\n");
			break;
		default:
			break;
	}

	if (result == AUDIO_ERR_DEC_FINISH) {
		switch (aud_status.play_style) {
			case PLAY_REPEAT:
				audio_send_play();
				ap_state_config_store();
				break;
//			case PLAY_ONCE:
//				msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
//				break;
			case PLAY_SEQUENCE:
				pre_next_state = 0;
				audio_next_process();
				ap_state_config_store();
				break;
			default:
				break;
		}
	}
	else if (result == AUDIO_ERR_READ_FAIL) {
		audio_send_stop();
		msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
	}
	else {
		if (pre_next_state == 0) {
			audio_next_process();
		}
		else {
			audio_prev_process();
		}
	}
}

void audio_res_resume_process(INT32S result)
{
	DBG_PRINT("umi task - resume success\r\n");
}

void audio_res_pause_process(INT32S result)
{
	DBG_PRINT("umi task - pause success\r\n");
}

void audio_res_stop_process(INT32S result)
{
	DBG_PRINT("stop success\r\n");
	if (next_prev_start) {
		next_prev_start = 0;
		audio_send_play();
	}
}

void audio_confirm_handler(STAudioConfirm *aud_con)
{
	switch(aud_con->result_type) {
		case MSG_AUD_PLAY_RES:
			audio_res_play_process(aud_con->result);
			break;
		case MSG_AUD_BG_PLAY_RES:
			break;
		case MSG_AUD_STOP_RES:
			audio_res_stop_process(aud_con->result);
			break;
		case MSG_AUD_PAUSE_RES:
			audio_res_pause_process(aud_con->result);
			break;
		case MSG_AUD_RESUME_RES:
			audio_res_resume_process(aud_con->result);
			break;
		default:
			break;
	}
}

INT32S storage_file_nums_get(void)
{
    return FNodeInfo[SD_SLOT_ID].audio.MaxFileNum;
}

INT32S storage_scan_status_get(INT8U *status)
{
    INT8U state = SCAN_FILE_NOT_COMPLETE;
    
    if (FNodeInfo[SD_SLOT_ID].audio.flag == 1) {
    	state = SCAN_FILE_COMPLETE;
    }
    
    *status = state;

    return STATUS_OK;
}

INT32S storage_fopen(INT32U file_idx, STORAGE_FINFO *storage_finfo)
{
    f_pos *fpos = NULL;
	INT32S ret;
	struct sfn_info sfn_file;
	struct f_info *p_finfo;

   
    ret = ScanFileWait(&(FNodeInfo[SD_SLOT_ID].audio), file_idx);
    if (ret != 0) {
    	return ret;
    }
    fpos = GetFileNodeInfo(&(FNodeInfo[SD_SLOT_ID].audio), file_idx, &sfn_file);
    if (!fpos) {
        DBG_PRINT("Failed to get file info. slot = %d, Index=%d\r\n", SD_SLOT_ID, file_idx);
        return STATUS_FAIL;
    }
    
    if ((storage_finfo->fd = sfn_open(fpos)) < 0) {
        DBG_PRINT("Failed to open file\r\n");
        return STATUS_FAIL;
    }
    
    p_finfo = gp_malloc(sizeof(struct f_info));
    if(p_finfo == NULL)
    {
    	return STATUS_FAIL;
    }

    GetFileInfo(fpos, p_finfo);

    gp_memset((INT8S*)storage_finfo->f_name, 0, sizeof(storage_finfo->f_name));
    gp_memcpy((INT8S*)storage_finfo->f_name, (INT8S*)p_finfo->f_name,sizeof(storage_finfo->f_name));
    gp_memset((INT8S*)storage_finfo->f_extname, 0, sizeof(storage_finfo->f_extname));
    gp_strcpy((INT8S*)storage_finfo->f_extname, (INT8S*)sfn_file.f_extname);
    storage_finfo->f_size = p_finfo->f_size;
    storage_finfo->f_time = p_finfo->f_time;
    storage_finfo->f_date = p_finfo->f_date;
    gp_free(p_finfo);

    return STATUS_OK;
}

INT32S ap_music_index_get(void)
{
	return music_file_idx+1;
}

void ap_music_update_icon_status(void)
{
	//play-pause
	if(aud_status.aud_state == STATE_PAUSED) {
		ap_state_handling_icon_clear_cmd(ICON_MP3_PLAY, ICON_MP3_STOP, NULL);
		ap_state_handling_icon_show_cmd(ICON_MP3_PAUSE, NULL, NULL);
	}
	if(aud_status.aud_state == STATE_PLAY) {
		ap_state_handling_icon_clear_cmd(ICON_MP3_PAUSE, ICON_MP3_STOP, NULL);
		ap_state_handling_icon_show_cmd(ICON_MP3_PLAY, NULL, NULL);
	}
	if ((aud_status.aud_state == STATE_IDLE) ||(aud_status.aud_state == STATE_AUD_FILE_ERR) ) {
		ap_state_handling_icon_clear_cmd(ICON_MP3_PLAY, ICON_MP3_PAUSE, NULL);
		ap_state_handling_icon_show_cmd(ICON_MP3_STOP, NULL, NULL);
	}
	//play one/play all
	if(ap_state_music_play_mode_get() == PLAY_REPEAT) {
		ap_state_handling_icon_clear_cmd(ICON_MP3_PLAY_ALL, NULL, NULL);
		ap_state_handling_icon_show_cmd(ICON_MP3_PLAY_ONE, NULL, NULL);		
	} else {
		ap_state_handling_icon_clear_cmd(ICON_MP3_PLAY_ONE, NULL, NULL);
		ap_state_handling_icon_show_cmd(ICON_MP3_PLAY_ALL, NULL, NULL);					
	}
	//volume and index icon
	if (aud_status.volume == 0) {
		ap_state_handling_icon_clear_cmd(ICON_MP3_VOLUME, NULL, NULL);
		ap_state_handling_icon_show_cmd(ICON_MP3_MUTE, ICON_MP3_INDEX, NULL);		
	} else {
		ap_state_handling_icon_clear_cmd(ICON_MP3_MUTE, NULL, NULL);
		ap_state_handling_icon_show_cmd(ICON_MP3_VOLUME, ICON_MP3_INDEX, NULL);			
	}
	if (ap_state_handling_storage_id_get() == NO_STORAGE) {
//		ap_state_handling_mp3_index_show_zero_cmd();
//		ap_state_handling_mp3_total_index_show_zero_cmd();
	}else{
		if(storage_file_nums_get() == 0) {
//			ap_state_handling_mp3_index_show_zero_cmd();
		}else{
			if(music_file_idx >= storage_file_nums_get()){
				music_file_idx = 0;
			}			
//			ap_state_handling_mp3_index_show_cmd();
		}
//		ap_state_handling_mp3_total_index_show_cmd();
	}
//	ap_state_handling_mp3_volume_show_cmd();
//	ap_state_handling_mp3_FM_channel_show_cmd();	
}

void audio_wav_play(INT16S fd)
{
	music_para.fd = fd;
	music_para.src_type = AUDIO_SRC_TYPE_FS;
	aud_status.aud_state = STATE_PLAY;
	msgQSend(AudioTaskQ, MSG_AUD_PLAY, (void *)&music_para, sizeof(STAudioTaskPara), MSG_PRI_NORMAL);
}

void audio_wav_pause(void)
{
	if(aud_status.aud_state != STATE_PLAY) {
		return;
	}	
	aud_status.aud_state = STATE_PAUSED;
	msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
	msgQSend(AudioTaskQ, MSG_AUD_PAUSE, NULL, 0, MSG_PRI_NORMAL);
}

void audio_wav_resume(void)
{
	if(aud_status.aud_state != STATE_PAUSED) {
		return;
	}	
	aud_status.aud_state = STATE_PLAY;
	msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_STOP, NULL, NULL, MSG_PRI_NORMAL);
	msgQSend(AudioTaskQ, MSG_AUD_RESUME, NULL, 0, MSG_PRI_NORMAL);
}

void audio_wav_stop(void)
{
	aud_status.aud_state = STATE_IDLE;
	msgQSend(AudioTaskQ, MSG_AUD_STOP, NULL, 0, MSG_PRI_NORMAL);
}
void ap_state_audio_reverse_set(INT8U reverse)
{
    music_para.reverse = reverse;
    msgQSend(AudioTaskQ, MSG_AUD_REVERSE_SET, (void *)&music_para, sizeof(STAudioTaskPara), MSG_PRI_NORMAL);       
}

void ap_state_audio_play_speed_set(INT8U speed)
{
    music_para.play_speed = speed;
    msgQSend(AudioTaskQ, MSG_AUD_PLAY_SPEED_SET, (void *)&music_para, sizeof(STAudioTaskPara), MSG_PRI_NORMAL);     
}
