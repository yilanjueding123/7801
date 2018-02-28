#ifndef __AP_MUSIC_H__
#define __AP_MUSIC_H__
#include "task_state_handling.h"

#define MAX_VOLUME_LEVEL 	8 /* 0~16 */
#define VOLUME_STEP       	8

#define MAX_FM_FREQ 1080 //unit:100kHz
#define MIN_FM_FREQ 875


extern void ap_music_init(void);
extern void ap_music_effect_resource_init(void); //wwj add
extern void audio_play_process(void);
extern void audio_play_pause_process(void);
extern void audio_next_process(void);
extern void audio_prev_process(void);
extern void audio_send_stop(void);
extern void audio_res_play_process(INT32S result);
extern void audio_res_resume_process(INT32S result);
extern void audio_res_pause_process(INT32S result);
extern void audio_res_stop_process(INT32S result);
extern void audio_mute_ctrl_set(BOOLEAN status);
extern void audio_vol_inc_set(void);
extern void audio_vol_dec_set(void);
extern INT8U audio_fg_vol_get(void);
extern void audio_play_style_set(INT8U play_style);
extern void audio_vol_set(INT8U vol);
extern void audio_playing_mode_set_process(void);
extern void audio_quick_select(INT32U index);
extern void audio_fm_freq_inc_set(void);
extern void audio_fm_freq_dec_set(void);
extern void audio_fm_freq_quick_set(INT32U freq);
extern INT8U audio_playing_state_get(void);
extern void audio_confirm_handler(STAudioConfirm *aud_con);
extern INT32S storage_file_nums_get(void);
extern INT32S storage_scan_status_get(INT8U *status);
extern INT32S storage_fopen(INT32U file_idx, STORAGE_FINFO *storage_finfo);
extern INT32S ap_music_index_get(void);
extern INT32S storage_file_nums_get(void);
extern INT8U audio_fg_vol_get(void);
extern INT32U audio_fm_freq_ch_get(void);
extern void ap_music_update_icon_status(void);
extern void ap_music_reset(void);

extern INT32S music_file_idx;
extern INT32S audio_effect_play(INT32U effect_type);
extern void audio_wav_play(INT16S fd);
extern void audio_wav_pause(void);
extern void audio_wav_resume(void);
extern void audio_wav_stop(void);

extern void audio_vol_set(INT8U vol);
extern void ap_state_audio_play_speed_set(INT8U speed);
extern void ap_state_audio_reverse_set(INT8U reverse);
extern void audio_send_resume(void);
extern void audio_send_pause(void);

#endif /*__AP_MUSIC_H__*/
