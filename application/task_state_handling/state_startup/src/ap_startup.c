#include "ap_music.h"
#include "ap_state_config.h"
#include "ap_startup.h"
#include "drv_l1_tft.h"
#include "drv_l2_spi_flash.h"
#include "avi_encoder_app.h"

void ap_startup_init(void)
{
	video_encode_entrance();
}