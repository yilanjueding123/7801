#include "stdio.h"
#include "string.h"
#include "drv_l1_uart.h"
#include "drv_l1_gsensor.h"
#include "avi_audio_record.h"
#include "ap_state_config.h"
#include "lpf.h"

#define GPS_DEBUG 0
 
#define C_AVI_AUDIO_RECORD_STACK_SIZE	512
#define C_AVI_AUD_ACCEPT_MAX			(AVI_ENCODE_PCM_BUFFER_NO+8)

#define		BIT8		0x00000100		// for audio record mute
#define		RAMP_DOWN_STEP	20

/*os task stack */
static INT32U	AviAudioRecordStack[C_AVI_AUDIO_RECORD_STACK_SIZE];

/* os task queue */
static OS_EVENT *avi_aud_q;
static OS_EVENT *avi_aud_ack_m;
static void *avi_aud_q_stack[C_AVI_AUD_ACCEPT_MAX];

/* static function */ 
static AVIPACKER_FRAME_INFO audio_frame[AVI_ENCODE_PCM_BUFFER_NO] = {0};
static char rx_buf[MAX_UART_SIZE] = "$GPRMC,121252.000,A,3958.3032,N,11629.6046,E,15.15,359.95,070306,,,A*54";
//static char rx_buf[] = "$GPRMC,213647.886,V,,,,,,,010207,,,N*4A";
//static char rx_buf[] = "$GPRMC,121252.000,A,24.77509,N,121.008513,E,15.15,359.95,141106,,,A*54";
__align(4) GPSDATA GPS = {0};

extern INT8U gsensor_data[2][32];

#if GPS_EN

		#if GPS_DEBUG
		static int gps_cnt = 0;
		#endif
		/////////////////////////////////////////////////////////////////////////////////////////////////////////

		#if GPS_TXT

		static AVIPACKER_FRAME_INFO gps_frame = {0};
		static char gps_txt_buf[MAX_UART_SIZE];

		static void GPS_TXT_SEND(char *buf, int len)
		{
			memcpy(gps_txt_buf, buf, len);
			gps_frame.buffer_addrs = (INT32U)gps_txt_buf;
			gps_frame.buffer_len = len;
			gps_frame.is_used = 1;
			gps_frame.msg_id = AVIPACKER_MSG_GPS_WRITE;
			if(pfn_avi_encode_put_data) {
				pfn_avi_encode_put_data(pAviEncPara->AviPackerCur->avi_workmem, &gps_frame);
			}
		}						

		#endif


		void GPS_Info_add(const char *dst)
		{
			void *ptr = (void *)dst;
			void *pGPS = (void *)(&GPS);	
			memcpy(ptr,pGPS,sizeof(GPSDATA) );
		}

		#if 0
		static void GPRMC_Parse(char *rx_buf, RMCINFO *rmc_info)
		{
			char time[16];
			char data[24];
			
			if (strstr(rx_buf,"$GPRMC")!=NULL)
			{
				rmc_info->Status = 'V';
				sscanf(rx_buf,"$GPRMC,%[^,],%c,%f,%c,%f,%c,%f,%f,%2d%2d%2d,%s", \
				time, \
				&(rmc_info->Status),	\
				&(rmc_info->Latitude),
				&(rmc_info->NSInd),\
				&(rmc_info->Longitude),
				&(rmc_info->EWInd),
				&(rmc_info->Speed),\
				&(rmc_info->Angle),\
				&(rmc_info->Year),&(rmc_info->Month),&(rmc_info->Day),\
				data );
				
				sscanf(time,"%2d%2d%2d%s", \
				&(rmc_info->Hour),&(rmc_info->Minute),&(rmc_info->Second), \
				data );
		         
				DBG_PRINT("H:%d, M:%d, S:%d\r\n",rmc_info->Hour,rmc_info->Minute,rmc_info->Second);
				DBG_PRINT("Status: %c\r\n",rmc_info->Status);
				DBG_PRINT("Latitude: %f\r\n",rmc_info->Latitude);
				DBG_PRINT("NSInd: %cr\r\n",rmc_info->NSInd);
				DBG_PRINT("Longitude: %f\r\n",rmc_info->Longitude);
				DBG_PRINT("EWInd: %c\r\n",rmc_info->EWInd);
				DBG_PRINT("Speed: %f\r\n",rmc_info->Speed);	
				DBG_PRINT("Angle: %f\r\n",rmc_info->Angle);	
				DBG_PRINT("Y:%d, M:%d, D:%d\r\n\n\n",rmc_info->Year,rmc_info->Month,rmc_info->Day);
			}
		}
		#elif GPS_AVI_DEMO

		static float delta = 0;
		static void GPRMC_Parse(char *rx_buf, RMCINFO *rmc_info)
		{
			rmc_info->Hour = 12;
			rmc_info->Minute = 12;
			rmc_info->Second = 52;
			rmc_info->Status = 'A';
			rmc_info->Latitude = 24.77509 + delta;
			rmc_info->NSInd = 'N';
			rmc_info->Longitude = 121.008513 + delta;
			rmc_info->EWInd = 'E';
			rmc_info->Speed = 15.15;
			rmc_info->Angle = 359.95;
			rmc_info->Year = 14;
			rmc_info->Month = 11;
			rmc_info->Day = 14;

			delta += 0.0001;
		}

		#else

		static const INT32U MONTH_TABLE[] = {31, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
		static void utc_conv(RMCINFO *rmc_info)
		{
			float time = rmc_info->Longitude - 7.5;
			INT32U hour = (INT32U)(time/15.0) + 1;

			// DAY
			if (rmc_info->EWInd == 'E')
			{
				rmc_info->Hour += hour;
				if (rmc_info->Hour>=24)
				{
					rmc_info->Hour-=24;
					rmc_info->Day += 1;
				}
			}
			else if (rmc_info->EWInd == 'W')
			{
				INT32S tmp = rmc_info->Hour;
				tmp -= (INT32S)hour;
				if (tmp<0)
				{
					tmp += 24;
					rmc_info->Day -= 1;
				}
				rmc_info->Hour = (INT32U)tmp;
			}

			// MONTH
			if (rmc_info->Day == 0)
			{
				rmc_info->Month -= 1;
				rmc_info->Day = MONTH_TABLE[rmc_info->Month];
				if ( (rmc_info->Month==2)&&((rmc_info->Year)&0x3==0x0) ) {
					rmc_info->Day = 29;		// 潤年
				}

				 if (rmc_info->Month==0)  {
					rmc_info->Month = 12;
					rmc_info->Year -= 1;
				 }	
			}
			else
			{
				switch(rmc_info->Month)
				{
					case 1:
					case 3:
					case 5:
					case 7:
					case 8:
					case 10:
					case 12:
						if (rmc_info->Day>31) {
							rmc_info->Month += 1;
							rmc_info->Day = 1;
						}
						if (rmc_info->Month>12) {
							rmc_info->Month = 1;
							rmc_info->Year += 1;
						}				
						break;
					case 4:
					case 6:
					case 9:
					case 11:
						if (rmc_info->Day>30) {
							rmc_info->Month += 1;
							rmc_info->Day = 1;
						}				
						break;
					case 2:
						{
							INT32U DUR = 28;
							if ( (rmc_info->Month==2)&&((rmc_info->Year)&0x3==0x0) ) 	{
								DUR = 29;	// 潤年
							}

							if (rmc_info->Day>DUR) {
								rmc_info->Month += 1;
								rmc_info->Day = 1;
							}
						}
						break;
				}
			}	
		}

		static float conv(float num)
		{
			int integer = (int)num;
			int f1 = integer/100;
			int f2 = integer%100;
			float ans = (num - (float)integer)/60.0f;

			ans += ((float)f2/60.0f);
			ans += ((float)f1);
			return ans;
		}

		static float stof(const char* s)
		{
			float rez = 0, fact = 1;
			int point_seen;
			int d;
			
			if (*s == '-'){
				s++;
		    		fact = -1;
		  	}
		  	for (point_seen = 0; *s; s++) {
		    		if (*s == '.'){
		      			point_seen = 1; 
		      			continue;
		    		}
		    		d = *s - '0';
		    		if (d >= 0 && d <= 9) {
		      			if (point_seen) fact /= 10.0f;
		      			rez = rez * 10.0f + (float)d;
		    		}
		  	}
		  	return rez * fact;
		}

		static void stot(const char *s, unsigned int *val0, unsigned int *val1, unsigned int *val2) 
		{
			char tmp[16];
			sscanf(s,"%2d%2d%2d%s",val0,val1,val2,tmp);
		}

		static void GPRMC_conv(int flag, int num, const char *s, RMCINFO *rmc_info)
		{
			if (flag == 0)
				num = 0;
			switch(num)
			{
				case 1:
					stot(s,&(rmc_info->Hour),&(rmc_info->Minute),&(rmc_info->Second));
					break;
				case 2:
					rmc_info->Status = *s;
					break;
				case 3:
					{
						float f = stof(s);
						rmc_info->Latitude = conv(f);
					}
					break;
				case 4:
					rmc_info->NSInd = *s;
					break;
				case 5:
					{
						float f = stof(s);
						rmc_info->Longitude = conv(f);
					}
					break;
				case 6:
					rmc_info->EWInd = *s;
					break;
				case 7:
					rmc_info->Speed= stof(s);
					break;
				case 8:
					rmc_info->Angle= stof(s);
					break;
				case 9:
					stot(s,&(rmc_info->Day),&(rmc_info->Month),&(rmc_info->Year));
					break;
				case 0:
				default:
					break;
			}
		}

		static void GPRMC_Parse(const char *rx_buf, RMCINFO *rmc_info)
		{
			char *p = (char *)rx_buf;
			char *p_next;

			if (strstr(rx_buf,"$GPRMC")!=NULL)
			{
				int i;
				rmc_info->Status = 'V';		
				for (i=0;i<10;++i)
				{
					p_next=(char*)strchr(p,',');
					*p_next = '\0';			
					if (p==p_next) {	// no data
						GPRMC_conv(0,i, (const char *)p,rmc_info);
					}	
					else {
						GPRMC_conv(1,i,(const char *)p,rmc_info);
					}	
					p = p_next + 1;			
				}
				if (rmc_info->Status == 'A')
				{
					utc_conv(rmc_info);
				}
		#if GPS_DEBUG
				if (rmc_info->Status == 'V') {
					ap_display_icon_sts_set(ICON_VIDEO_LDW_SART);
				}
				else {
					ap_display_icon_sts_set(ICON_VIDEO_LDW_ACTIVE);			
				}
				gps_cnt = 0;
		#endif
			}
		}
		#endif

#endif   // GPS_EN

INT32S avi_adc_gps_register(void)
{
//	uart_resource_register(avi_aud_q, AVI_GPS_INFO_HANDLE);
	return 0;
}

void avi_adc_gsensor_data_register(void **msgq_id, INT32U *msg_id)
{
	*msgq_id = (void *)avi_aud_q;
	*msg_id = AVI_G_SENSOR_INFO_HANDLE;
}

INT32S avi_adc_record_task_create(INT8U pori)
{	
	INT8U  err;
	INT32S nRet;
	
	avi_aud_q = OSQCreate(avi_aud_q_stack, C_AVI_AUD_ACCEPT_MAX);
	if(!avi_aud_q) RETURN(STATUS_FAIL);
	
	avi_aud_ack_m = OSMboxCreate(NULL);
	if(!avi_aud_ack_m) RETURN(STATUS_FAIL);
	
	err = OSTaskCreate(avi_audio_record_entry, NULL, (void *)&AviAudioRecordStack[C_AVI_AUDIO_RECORD_STACK_SIZE - 1], pori);
	if(err != OS_NO_ERR) RETURN(STATUS_FAIL);

	nRet = STATUS_OK;
Return:
    return nRet;
}

INT32S avi_adc_record_task_del(void)
{
	INT8U  err;
	INT32U nRet, msg;
	
	nRet = STATUS_OK;
	POST_MESSAGE(avi_aud_q, AVI_AUDIO_RECORD_EXIT, avi_aud_ack_m, 5000, msg, err);
Return:	
	OSQFlush(avi_aud_q);
   	OSQDel(avi_aud_q, OS_DEL_ALWAYS, &err);
	OSMboxDel(avi_aud_ack_m, OS_DEL_ALWAYS, &err);
	return nRet;
}

INT32S avi_audio_record_start(void)
{
	INT8U  err;
	INT32S nRet, msg;
	
	nRet = STATUS_OK;
	POST_MESSAGE(avi_aud_q, AVI_AUDIO_RECORD_START, avi_aud_ack_m, 5000, msg, err);
Return:
	return nRet;	
}

INT32S avi_audio_record_restart(void)
{
	INT8U  err;
	INT32S nRet, msg;
	
	nRet = STATUS_OK;
	POST_MESSAGE(avi_aud_q, AVI_AUDIO_RECORD_RESTART, avi_aud_ack_m, 5000, msg, err);
Return:
	return nRet;
}

INT32S avi_audio_record_stop(void)
{
	INT8U  err;
	INT32S nRet, msg;
	
	nRet = STATUS_OK;
	POST_MESSAGE(avi_aud_q, AVI_AUDIO_RECORD_STOPING, avi_aud_ack_m, 5000, msg, err);
Return:	
	if(nRet < 0)
	{
		avi_audio_memory_free();
	}
	return nRet;	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
static unsigned int avi_aud_pkt_idx_get(INT32U addr)
{
	INT32S i;

	for(i=0; i<AVI_ENCODE_PCM_BUFFER_NO; i++) {
		if(audio_frame[i].buffer_addrs == addr) {
			break;
		}
	}

	if((i == AVI_ENCODE_PCM_BUFFER_NO) || (audio_frame[i].buffer_idx != i)) {
		DBG_PRINT("wwj: error\r\n");
		while(1);
	} else {
		return i;
 	}
}

void avi_audio_record_entry(void *parm)
{
	INT32S  nRet, audio_stream, ret, i;
	INT32U  msg_id, pcm_addr, pcm_cwlen;
	INT32U  ready_addr, next_addr;
	INT32S  adc_pga_gain, adc_pga_gain_target;
	INT8U   err, bStop, audio_flag;	
	INT8U 	avi_idx;

	while(1) 
	{
		msg_id = (INT32U) OSQPend(avi_aud_q, 0, &err);
		if(err != OS_NO_ERR)
			continue;

		switch(msg_id&0xFF000000)
		{
			case AVI_AUDIO_DMA_DONE:
				if (msg_id !=C_DMA_STATUS_DONE) {
					DBG_PRINT("AVI_AUDIO_DMA_DONE Error !!\r\n");
					break;
				}

				if(bStop) {
					OSQPost(avi_aud_q, (void *)AVI_AUDIO_RECORD_STOP);  // check dma is done and stop
					avi_idx = avi_audio_get_buffer_idx();					
					if  (audio_frame[avi_idx].is_used!=0) {
						break;   // aud no buf  [exit AVI_AUDIO_DMA_DONE]
					}
				} else {
					// waiting avi packer, if there is no audio buffer
					if ((avi_encode_get_status()&C_AVI_ENCODE_USB_WEBCAM)==0)
					{
						avi_idx = avi_audio_get_buffer_idx();
						if  (audio_frame[avi_idx].is_used!=0)
						{
							/*
							DBG_PRINT("aud no buf  (%d) [ ",avi_idx);
							for (i=0;i<AVI_ENCODE_PCM_BUFFER_NO;++i)
							{
								DBG_PRINT("%d ",audio_frame[i].is_used);
							}
							DBG_PRINT("]\r\n");
							*/
							DBG_PRINT("&");

							avi_adc_double_buffer_set((INT16U*) 0xF8500000, pcm_cwlen);
							break;
						}
					}

					//DBG_PRINT("r=0x%x\r\n",ready_addr);
					pcm_addr = ready_addr;
					ready_addr = next_addr;
					next_addr = avi_audio_get_next_buffer()+16;	// first 16 byte for AVI chunk no and chunk size
				
					avi_adc_double_buffer_set((INT16U*)next_addr, pcm_cwlen);// set dma buffer
				}
			#if C_USB_AUDIO == CUSTOM_ON
				if((avi_encode_get_status()&C_AVI_ENCODE_USB_WEBCAM))
				{
					INT32U FrameSize = USB_AUDIO_PCM_SAMPLES*2;
					INT32U AddrFrame = pcm_addr;
					usb_send_audio((INT8U*)(AddrFrame), FrameSize);					
					break;
				}
			#endif
				{	// Ramp Up/Down
					INT32U REG = R_MIC_READY;
					INT32U boost = 1;
					if (ap_state_config_voice_record_switch_get())
					{	// Voice : ON
						adc_pga_gain -= RAMP_DOWN_STEP;
						boost = 1;
						if (adc_pga_gain<=adc_pga_gain_target) {
							adc_pga_gain = adc_pga_gain_target;
						}
					}
					else
					{	// Voice: OFF
						adc_pga_gain += RAMP_DOWN_STEP;
						if (adc_pga_gain>=0x1F) {  // mute
							adc_pga_gain = 0x1F;
							boost = 0;
						}
					}
					if ( (R_MIC_READY&0x1F)!=adc_pga_gain)
					{
						REG &= (~0x1F);						
						REG |= adc_pga_gain;
						R_MIC_READY = REG;
						DBG_PRINT("G:%d\r\n",adc_pga_gain);
					}
					if (boost==0)  {
						R_MIC_READY &= (~BIT8);    // boost off
					}
					else {
						R_MIC_READY |= BIT8;	// boost on
					}
				}
				
			#if AUDIO_SFX_HANDLE
				pcm_addr = (INT16U *)video_encode_audio_sfx((INT16U *)pcm_addr, pcm_cwlen<<1);
			#endif	
				audio_stream = (pcm_cwlen<<1) + 8 + 2*16;
				
				// when restart, wait pcm frame end
				if(audio_flag == 1) {
					audio_flag = 0;
					OSMboxPost(avi_aud_ack_m, (void*)C_ACK_SUCCESS);
				}
				
				if((avi_encode_get_status()&C_AVI_ENCODE_START) == 0) break;
				
				ret = avi_encode_disk_size_is_enough(audio_stream);
				if (ret == 0) {
					avi_enc_storage_full();
					continue;
				} else if (ret == 2) {
					msgQSend(ApQ, MSG_APQ_RECORD_SWITCH_FILE, NULL, NULL, MSG_PRI_NORMAL);
				}

#if GPS_EN
				{	// Add GPS information
					const char *ptr = (const char *)((pcm_addr-16) +8 + 64*1024); // pcm_addr-16 + 8: buffer start address
															   			// 64*1024: GPS infomation must place here
															   			// audio sampling rate = 32000 Hz
					#if GPS_AVI_DEMO
					   GPRMC_Parse((char *)0x800000, &(GPS.rmcinfo));
					#endif
					GPS_Info_add(ptr);
					#if GPS_DEBUG
					if (gps_cnt>3)
					{
						ap_display_icon_sts_clear(ICON_VIDEO_LDW_SART);
						ap_display_icon_sts_clear(ICON_VIDEO_LDW_ACTIVE);
					}
					gps_cnt++;
					#endif
				}
#endif

				{
					unsigned int idx = avi_aud_pkt_idx_get(pcm_addr-16);
					//audio_frame[idx].buffer_addrs = (pcm_addr-16);	// first 16 byte for AVI chunk no and chunk size
					//audio_frame[idx].buffer_idx = idx;
					audio_frame[idx].ext = (pcm_cwlen<<1)/pAviEncPara->AviPackerCur->p_avi_wave_info->nBlockAlign;
					audio_frame[idx].buffer_len = (INT32U)(pcm_cwlen<<1);
					audio_frame[idx].is_used = 1;
					audio_frame[idx].msg_id = AVIPACKER_MSG_AUDIO_WRITE;
					if(pfn_avi_encode_put_data) {
						pfn_avi_encode_put_data(pAviEncPara->AviPackerCur->avi_workmem, &(audio_frame[idx]));
					}
					//DBG_PRINT("%d",idx);
				}
				break;
			
			case AVI_AUDIO_RECORD_START:
				bStop = audio_flag = 0;
			  #if C_USB_AUDIO == CUSTOM_ON
				if((avi_encode_get_status()&C_AVI_ENCODE_USB_WEBCAM))
				{
					pcm_cwlen = USB_AUDIO_PCM_SAMPLES;
				}
				else 
			  #endif
				{
					pcm_cwlen =  AVI_AUDIO_PCM_SAMPLES;  //////////joseph////////////////////////pAviEncAudPara->pcm_input_size * C_WAVE_ENCODE_TIMES;
				}
			  	DBG_PRINT("audlen=%d\r\n",pcm_cwlen);
				
				nRet = avi_audio_memory_allocate( (pcm_cwlen<<1)+16+16+2000);  // first 16 byte for chunk no and chunk size, last 16 byte for JUNK
																			    // 64KB=>PCM DATA, 4KB前72Byte是GPS資訊
				if(nRet < 0)  {
					DBG_PRINT("Audio Buffer Alloc Fail\r\n");
					goto AUDIO_RECORD_START_FAIL;
				}

				for (i=0; i<AVI_ENCODE_PCM_BUFFER_NO; i++) {
					audio_frame[i].is_used = 0;
					audio_frame[i].buffer_addrs = avi_audio_get_next_buffer();
					audio_frame[i].buffer_idx = i;
				}
				
				for (i=0; i<8; i++) { 
		        	(*((volatile INT32U *) (0xF8300000+i*4))) = 0x00000000; 
				} 		
					
				ready_addr = avi_audio_get_next_buffer()+16;	// first 16 byte for AVI chunk no and chunk size
				next_addr = avi_audio_get_next_buffer()+16;	// first 16 byte for AVI chunk no and chunk size
				nRet = avi_adc_double_buffer_put((INT16U*)ready_addr, pcm_cwlen, avi_aud_q);
				if(nRet < 0) goto AUDIO_RECORD_START_FAIL;
				nRet = avi_adc_double_buffer_set((INT16U*)next_addr, pcm_cwlen);
				if(nRet < 0) goto AUDIO_RECORD_START_FAIL;
				avi_adc_hw_start(pAviEncAudPara->audio_sample_rate);
				adc_pga_gain_target = (INT32S)(R_MIC_READY&0x1F);
				if (ap_state_config_voice_record_switch_get())
				{	// Voice : ON
					adc_pga_gain	= (INT32S)(R_MIC_READY & 0x1F);	// bit[4:0] PGA gain
				}
				else
				{	// Voice OFF
					adc_pga_gain = (INT32S)(0x1F);
					R_MIC_READY |= (INT32U)(adc_pga_gain);
				}
				
			  #if C_USB_AUDIO == CUSTOM_ON
				if((avi_encode_get_status()&C_AVI_ENCODE_USB_WEBCAM))
				{
					OSMboxPost(avi_aud_ack_m, (void*)C_ACK_SUCCESS);
					break;
				}
			  #endif
				//pAviEncPara->delta_ta = (INT64S)my_pAviEncVidPara->dwRate * pcm_cwlen;
				OSMboxPost(avi_aud_ack_m, (void*)C_ACK_SUCCESS);
				break;
AUDIO_RECORD_START_FAIL:
				avi_adc_hw_stop();
				{	// 還原音量
					R_MIC_READY &= (~0x1F);
					R_MIC_READY |= adc_pga_gain_target;
					R_MIC_READY |= BIT8;		// boost on
				}				
				avi_adc_double_buffer_free();
				avi_audio_memory_free();
				DBG_PRINT("AudEncStartFail!!!\r\n");
				OSMboxPost(avi_aud_ack_m, (void*)C_ACK_FAIL);
				break;	
				
			case AVI_AUDIO_RECORD_STOP:
				avi_adc_hw_stop();
				{	// 還原音量
					R_MIC_READY &= (~0x1F);
					R_MIC_READY |= adc_pga_gain_target;
					R_MIC_READY |= BIT8;		// boost on
				}
				{	// 檢查寫卡線程是否把 audio buffer 都還回來！
					INT32U cnt, sum = 0;
					INT32U time_out_cnt = 0;

					while (1) {
						sum = 0;
						for (cnt=0; cnt<AVI_ENCODE_PCM_BUFFER_NO; ++cnt) {
							sum += audio_frame[cnt].is_used;
						}

						if (sum == 0) {
							break;
						}
						if (time_out_cnt>20) { //wwj modify, maybe "5" is too short
							DBG_PRINT("AUDIO STOP Error (abnormal audbuf_status) \r\n");
							break;
						}

						DBG_PRINT("AUDIO STOP WARNING !! audbuf_status [ ");
						for (cnt=0; cnt<AVI_ENCODE_PCM_BUFFER_NO; ++cnt) {
							DBG_PRINT("%d ",audio_frame[cnt].is_used);
						}
						DBG_PRINT("]\r\n");
						OSTimeDly(sum*10);
						time_out_cnt++;
					}
				}
				avi_adc_double_buffer_free();
				avi_audio_memory_free();
				OSMboxPost(avi_aud_ack_m, (void*)C_ACK_SUCCESS);
				break;

			case AVI_AUDIO_RECORD_STOPING:
				bStop = 1;
				break;
		
			case AVI_AUDIO_RECORD_RESTART:
				DBG_PRINT("AVI_AUDIO_RECORD_RESTART\r\n");
				audio_flag = 1;
				break;
			
			case AVI_AUDIO_RECORD_EXIT:
				OSQFlush(avi_aud_q);
				OSMboxPost(avi_aud_ack_m, (void*)C_ACK_SUCCESS);
				OSTaskDel(OS_PRIO_SELF);
				break;
			case AVI_GPS_INFO_HANDLE:
#if GPS_EN				
 				{
					INT8U *p = NULL;//(INT8U*)uart_resource();
					INT32U len = strlen((const char *)p);
					memcpy(rx_buf, p, len);

					if ( (rx_buf[0]=='$')&&(rx_buf[1]=='G')&&(rx_buf[2]=='P')&&(rx_buf[3]=='R')&&(rx_buf[4]=='M')&&(rx_buf[5]=='C') )
					{
						#if GPS_TXT
						GPS_TXT_SEND(rx_buf, len);
						#endif
						GPRMC_Parse((char *)rx_buf, &(GPS.rmcinfo));
					}
					//G_PRINT("%s len=%d\r\n",rx_buf,len);
				}
#endif
 				break;
			case AVI_G_SENSOR_INFO_HANDLE:
				{
					int idx = msg_id & 0x1;
					G_Sensor_gps_data_set((void*)(&(GPS.gs_data)), gsensor_data[idx]);
					// DBG_PRINT("%d, %d, %d\r\n",GPS.gs_data.Axis.Xacc,GPS.gs_data.Axis.Yacc,GPS.gs_data.Axis.Zacc);
				}
				break;
		}	
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
static INT32S avi_wave_encode_start(void)
{
	INT32S nRet, size;

	size = wav_enc_get_mem_block_size();
	pAviEncAudPara->work_mem = (INT8U *)gp_malloc(size);
	if(!pAviEncAudPara->work_mem) RETURN(STATUS_FAIL);
	gp_memset((INT8S*)pAviEncAudPara->work_mem, 0, size);
	nRet = wav_enc_Set_Parameter( pAviEncAudPara->work_mem, 
								  pAviEncAudPara->channel_no, 
								  pAviEncAudPara->audio_sample_rate, 
								  pAviEncAudPara->audio_format);
	if(nRet < 0) RETURN(STATUS_FAIL);
	nRet = wav_enc_init(pAviEncAudPara->work_mem);
	if(nRet < 0) RETURN(STATUS_FAIL);
	pAviEncAudPara->pcm_input_size = wav_enc_get_SamplePerFrame(pAviEncAudPara->work_mem);
	
	switch(pAviEncAudPara->audio_format)
	{
	case WAVE_FORMAT_PCM:
		pAviEncAudPara->pack_size = pAviEncAudPara->pcm_input_size;	
		pAviEncAudPara->pack_size *= 2;
		break;
	
	case WAVE_FORMAT_ALAW:
	case WAVE_FORMAT_MULAW:	
	case WAVE_FORMAT_ADPCM:
	case WAVE_FORMAT_IMA_ADPCM:
		pAviEncAudPara->pack_size = wav_enc_get_BytePerPackage(pAviEncAudPara->work_mem);	
		break;
	}
	
	nRet = STATUS_OK;
Return:	
	return nRet;	
}

static INT32S avi_wave_encode_stop(void)
{
	gp_free((void*)pAviEncAudPara->work_mem);
	pAviEncAudPara->work_mem = 0;
	return STATUS_OK;
}

static INT32S avi_wave_encode_once(INT16S *pcm_input_addr)
{
	INT8U *encode_output_addr;
	INT32S nRet, encode_size, N;
	
	encode_size = 0;
	N = C_WAVE_ENCODE_TIMES;
	encode_output_addr = (INT8U*)pAviEncAudPara->pack_buffer_addr;
	encode_output_addr += 16;		//added by wwj	
	while(N--)
	{
		nRet = wav_enc_run(pAviEncAudPara->work_mem, (short *)pcm_input_addr, encode_output_addr);
		if(nRet < 0)		
			return  STATUS_FAIL;
		
		encode_size += nRet;
		pcm_input_addr += pAviEncAudPara->pcm_input_size;
		encode_output_addr += pAviEncAudPara->pack_size;
	}
	
	return encode_size;
}
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////

