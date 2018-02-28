
#include "task_state_handling.h"

/****************************************************************************/
#define WIFI_STATE_FLAG_DISCONNECT	0x00	
#define WIFI_STATE_FLAG_CONNECT		0x01

/****************************************************************************/
typedef struct WIFI_STATE_ARGS_s
{
	INT8U Wifi_State_Flag;
}WIFI_STATE_ARGS_t;

/****************************************************************************/
extern INT8U g_LFSRseed[];
extern INT8U g_LFSR_Key[];

extern INT32S Wifi_symbol_show(INT16U new_symbol);

extern INT32U Wifi_State_Get(void);
extern void Wifi_State_Set(INT8U wifiState);
extern INT32U Wifi_Connect(void);
extern INT32U Wifi_Disconnect(void);
extern void Wifi_mode_active(void);
extern void Wifi_mode_shutdown(void);
extern void ap_wifi_signal_setting_page_draw(INT32U buff_addr);
extern void ap_wifi_display_setting_page_draw(INT32U buff_addr);
extern INT8U Gen_LFSR_32Bit_Key(void);
extern void gp_clear_modebak(void);
extern void gp_capture_card_full(void);

extern void sw_wifi_jpeg_sem_init(void);
extern void sw_wifi_jpeg_lock(void);
extern void sw_wifi_jpeg_unlock(void);



