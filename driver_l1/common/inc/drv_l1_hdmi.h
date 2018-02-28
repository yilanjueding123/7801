#ifndef __drv_l1_HDMI_H__
#define __drv_l1_HDMI_H__

#ifdef __cplusplus
extern "C" {
#endif

int drvl1_hdmi_init(unsigned int DISPLAY_MODE, unsigned int AUD_FREQ);
int drvl1_hdmi_exit(void);
void drvl1_hdmi_set_time_cycle(void *RegBase, unsigned int VBack, unsigned int HBlank);
void drvl1_hdmi_set_audio_sample_packet(void *RegBase, unsigned int ch);
void drvl1_hdmi_config_phy(unsigned int phy1, unsigned int phy2);
void drvl1_hdmi_set_acr_packet(void *RegBase, unsigned int N, unsigned int CTS);
void drvl1_hdmi_set_general_ctrl_packet(void *RegBase);
void drvl1_hdmi_send_packet(void *RegBase, unsigned int ch, void *data, unsigned int blank, unsigned int sendMode);
int  drvl1_hdmi_audio_ctrl(unsigned int status);
int  drvl1_hdmi_dac_mute(unsigned int status);

#ifdef __cplusplus
	}
#endif

#endif 		/* __drv_l1_HDMI_H__ */

