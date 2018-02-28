#include "ap_state_handling.h"
#include "ap_state_resource.h"
#include "ap_state_config.h"
#include "ap_thumbnail.h"
#include "ap_music.h"
#include "task_video_decoder.h"
#include "ap_display.h"

static INT32U thumbnail_output_buff;
static INT32U thumbnail_temp_buff;
static INT8S thumbnail_sts;
static INT8U icon_showed_num_in_one_page = 0;
static INT8U thumbnail_entrance_first_time = 0;
static INT8U showed_number_before_after_play_index_updated = 0;
static INT8S icon_showed_number_before_play_index = 0;
static INT8S icon_showed_number_after_play_index = 0;
static INT8S thumbnail_next_key_hold_number = -1;
static INT8S thumbnail_prev_key_hold_number = -1;
static INT16S first_entrance_play_index = -1;
static INT16U cursor_play_index = 0;
static INT32S page_showed_order = -1;
static INT16U total_file_num;
static INT16U deleted_file_num = 0;
static INT8U thumbnail_cursor_drawed = 0;
static INT8U clean_showed_buffer = 0;
INT8U g_thumbnail_reply_action_flag = 0;

//	prototypes
INT32S ap_thumbnail_jpeg_decode(STOR_SERV_PLAYINFO *info_ptr);
void ap_thumbnail_icon_draw(INT16U *frame_buff, INT16U *icon_stream, DISPLAY_ICONSHOW *icon_info, INT8U type);
void ap_thumbnail_cursor_draw(INT16U *frame_buff, INT16U *icon_stream, DISPLAY_ICONSHOW *icon_info, INT8U type);
void ap_thumbnail_clean_frame_buffer(void);

extern INT16U icon_locked[];

void ap_thumbnail_init(INT8U flag)
{
	INT32U search_type;

	if(flag) search_type = STOR_SERV_SEARCH_INIT;
	else search_type = STOR_SERV_SEARCH_ORIGIN;

	OSQPost(DisplayTaskQ, (void *) MSG_DISPLAY_TASK_EFFECT_INIT);

	thumbnail_output_buff = (INT32U) gp_malloc_align(getDispDevBufSize(), 64);
	if (!thumbnail_output_buff) {
		DBG_PRINT("State thumbnail allocate jpeg output buffer fail.\r\n");
	}		

	gp_memset((INT8S *)thumbnail_output_buff, 0, getDispDevBufSize());
	
	if(ap_display_get_device() == DISP_DEV_TFT) { //TFT
		thumbnail_temp_buff = (INT32U) gp_malloc_align(THUMBNAIL_ICON_WIDTH * THUMBNAIL_ICON_HEIGHT * 2, 64);
	} else {
		thumbnail_temp_buff = (INT32U) gp_malloc_align(THUMBNAIL_ICON_WIDTH_TV * THUMBNAIL_ICON_HEIGHT_TV * 2, 64);
	}
	if (!thumbnail_temp_buff) {
		DBG_PRINT("State thumbnail allocate jpeg temp buffer fail.\r\n");
	}
	thumbnail_sts = 0;
	if (ap_state_handling_storage_id_get() == NO_STORAGE) {
		g_thumbnail_reply_action_flag = 1;
		ap_thumbnail_sts_set(THUMBNAIL_UNMOUNT);
		ap_thumbnail_no_media_show(STR_NO_SD);
	} else {
		g_thumbnail_reply_action_flag = 0;
		msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_THUMBNAIL_REQ, (void *) &search_type, sizeof(INT32U), MSG_PRI_NORMAL);
	}
}

void ap_thumbnail_exit(void)
{
	icon_showed_num_in_one_page = 0;
	thumbnail_entrance_first_time = 0;
	showed_number_before_after_play_index_updated = 0;
	icon_showed_number_before_play_index = 0;
	icon_showed_number_after_play_index = 0;
	thumbnail_next_key_hold_number = -1;
	thumbnail_prev_key_hold_number = -1;
	first_entrance_play_index = -1;
	cursor_play_index = 0;
	page_showed_order = -1;
	total_file_num = 0;
	deleted_file_num = 0;
	thumbnail_cursor_drawed = 0;
	clean_showed_buffer = 0;
	gp_free((void *) thumbnail_output_buff);
	gp_free((void *) thumbnail_temp_buff);
	thumbnail_output_buff = 0;
	thumbnail_temp_buff = 0;
	thumbnail_sts = 0;
	
	msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
}

void ap_thumbnail_sts_set(INT8S sts)
{
	if (sts > 0) {
		thumbnail_sts |= sts;
	} else {
		thumbnail_sts &= sts;
	}
}

INT8S ap_thumbnail_sts_get(void)
{
	return thumbnail_sts;
}

INT16U ap_thumbnail_func_key_active(void)
{
	if (!thumbnail_sts) {
		return (cursor_play_index + deleted_file_num);
	}
	return 0;
}

void ap_thumbnail_next_key_active(INT8U err_flag)
{
	INT32U search_type = STOR_SERV_SEARCH_NEXT;
	
	if (err_flag) {
		search_type |= (err_flag << 8);
	}
	if (thumbnail_sts == 0) {
		if( (icon_showed_num_in_one_page != 0) || (total_file_num == 1) || (total_file_num == 0) ){
			//thumbnail_next_key_hold_number++;
			return;
		}else{
			g_thumbnail_reply_action_flag = 0;
			msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_THUMBNAIL_REQ, (void *) &search_type, sizeof(INT32U), MSG_PRI_NORMAL);
		}
	}
}

void ap_thumbnail_prev_key_active(INT8U err_flag)
{
	INT32U search_type = STOR_SERV_SEARCH_PREV;
	
	if (err_flag) {
		search_type |= (err_flag << 8);
	}
	if (thumbnail_sts == 0) {
		if( (icon_showed_num_in_one_page != 0) || (total_file_num == 1) || (total_file_num == 0) ){
			//thumbnail_prev_key_hold_number++;
			return;
		}else{
			g_thumbnail_reply_action_flag = 0;
			msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_THUMBNAIL_REQ, (void *) &search_type, sizeof(INT32U), MSG_PRI_NORMAL);
		}
	}
}


void ap_thumbnail_daw_page_num(INT16U real_play_index,INT16U total_file_number)
{

	STRING_ASCII_INFO ascii_str;
	CHAR thumbnail_file_path[5];

		char page_num[9]= {0};
		// char temp;
		INT8U i,j;
		
		if(real_play_index < 10){
			thumbnail_file_path[1] = 0;
			thumbnail_file_path[0] = real_play_index + 0x30;
		}else if(real_play_index < 100){
			thumbnail_file_path[2] = 0;
			thumbnail_file_path[0] = (real_play_index/10) + 0x30;
			thumbnail_file_path[1] = (real_play_index - 10*(thumbnail_file_path[0]-0x30)) + 0x30;
		}else if(real_play_index < 1000){				
			thumbnail_file_path[3] = 0;
			thumbnail_file_path[0] = (real_play_index/100) + 0x30;
			thumbnail_file_path[1] = ((real_play_index - 100*(thumbnail_file_path[0]-0x30))/10) + 0x30;
			thumbnail_file_path[2] = ((real_play_index - 100*(thumbnail_file_path[0]-0x30)) - 10*(thumbnail_file_path[1]-0x30)) + 0x30;
		}else if(real_play_index < 10000){
			thumbnail_file_path[4] = 0;
			thumbnail_file_path[0] = (real_play_index/1000) + 0x30;
			thumbnail_file_path[1] = ((real_play_index - 1000*(thumbnail_file_path[0]-0x30))/100) + 0x30;
			thumbnail_file_path[2] = ((real_play_index - 1000*(thumbnail_file_path[0]-0x30) - 100*(thumbnail_file_path[1]-0x30))/10) + 0x30;
			thumbnail_file_path[3] = ((real_play_index - 1000*(thumbnail_file_path[0]-0x30) - 100*(thumbnail_file_path[1]-0x30)) - 10*(thumbnail_file_path[2]-0x30)) + 0x30;
		}
		
		
		for(i=0;i<4;i++){
			if(thumbnail_file_path[i]){
				page_num[i] = thumbnail_file_path[i];
			}else{
				break;
			}
		}
		page_num[i] = '/';
		i++;
		
		if(total_file_number < 10){
			thumbnail_file_path[1] = 0;
			thumbnail_file_path[0] = total_file_number + 0x30;
		}else if(total_file_number < 100){
			thumbnail_file_path[2] = 0;
			thumbnail_file_path[0] = (total_file_number/10) + 0x30;
			thumbnail_file_path[1] = (total_file_number - 10*(thumbnail_file_path[0]-0x30)) + 0x30;
		}else if(total_file_number < 1000){				
			thumbnail_file_path[3] = 0;
			thumbnail_file_path[0] = (total_file_number/100) + 0x30;
			thumbnail_file_path[1] = ((total_file_number - 100*(thumbnail_file_path[0]-0x30))/10) + 0x30;
			thumbnail_file_path[2] = ((total_file_number - 100*(thumbnail_file_path[0]-0x30)) - 10*(thumbnail_file_path[1]-0x30)) + 0x30;
		}else if(total_file_number < 10000){
			thumbnail_file_path[4] = 0;
			thumbnail_file_path[0] = (total_file_number/1000) + 0x30;
			thumbnail_file_path[1] = ((total_file_number - 1000*(thumbnail_file_path[0]-0x30))/100) + 0x30;
			thumbnail_file_path[2] = ((total_file_number - 1000*(thumbnail_file_path[0]-0x30) - 100*(thumbnail_file_path[1]-0x30))/10) + 0x30;
			thumbnail_file_path[3] = ((total_file_number - 1000*(thumbnail_file_path[0]-0x30) - 100*(thumbnail_file_path[1]-0x30)) - 10*(thumbnail_file_path[2]-0x30)) + 0x30;
		}

		j=0;
		for(;i<9;i++){
			if(thumbnail_file_path[j]){
				page_num[i] = thumbnail_file_path[j];
			}else{
				break;
			}
			j++;
		}
		
		ascii_str.font_color = 0xffff;
		ascii_str.font_type = 0;
		if(ap_display_get_device() == DISP_DEV_TFT) { //TFT
			ascii_str.pos_x = TFT_WIDTH-ASCII_draw_char_width*i-10;
			ascii_str.pos_y = THUMBNAIL_FILE_NAME_START_Y;
		} else { //TV
			ascii_str.pos_x = TV_WIDTH-ASCII_draw_char_width*i-40;
			ascii_str.pos_y = THUMBNAIL_FILE_NAME_START_Y_TV;
		}
		ascii_str.str_ptr = (char *) page_num;
		if(ap_display_get_device() == DISP_DEV_TFT) { //TFT
			ascii_str.buff_w = TFT_WIDTH;
			ascii_str.buff_h = TFT_HEIGHT;
		} else { //TV
			ascii_str.buff_w = TV_WIDTH;
			ascii_str.buff_h = TV_HEIGHT;
		}
		ap_state_resource_string_ascii_draw((INT16U *) thumbnail_output_buff, &ascii_str, RGB565_DRAW);
}



void ap_thumbnail_reply_action(STOR_SERV_PLAYINFO *info_ptr)
{
	INT8U i,j;
	INT8U icon_row_order, icon_column_order, icon_order_in_one_page, icon_showed_number;
	INT16U total_page_number, page_showed_index, real_play_index, real_play_index_in_page;
	INT32U X_shiftment, X_shiftment_temp, X_shiftment_2, search_type;
	INT32S ret;
	STRING_ASCII_INFO ascii_str;
	DISPLAY_ICONSHOW icon;
	CHAR thumbnail_file_path[5];
	struct stat_t buf_tmp;
	INT32U thumbnail_icon_w, thumbnail_icon_h;

	if(ap_display_get_device() == DISP_DEV_TFT) { //TFT
		thumbnail_icon_w = THUMBNAIL_ICON_WIDTH;
		thumbnail_icon_h = THUMBNAIL_ICON_HEIGHT;
	} else { //TV
		thumbnail_icon_w = THUMBNAIL_ICON_WIDTH_TV;
		thumbnail_icon_h = THUMBNAIL_ICON_HEIGHT_TV;
	}

	if (!thumbnail_sts) {
		ap_thumbnail_sts_set(THUMBNAIL_DECODE_BUSY);
		if (info_ptr->err_flag == STOR_SERV_OPEN_OK) 
		{
			total_file_num = info_ptr->total_file_number;
			real_play_index = ((info_ptr->play_index+1) - (info_ptr->deleted_file_number));

			//if page_showed_order < 0 means it's first time
			if(page_showed_order < 0){
				page_showed_order = (real_play_index-1) / (THUMBNAIL_ROW_X_AMOUNT*THUMBNAIL_COLUMN_Y_AMOUNT);
				thumbnail_entrance_first_time = 1;
			}else{
				thumbnail_entrance_first_time = 0;
			}

			total_page_number = (info_ptr->total_file_number) / (THUMBNAIL_ROW_X_AMOUNT*THUMBNAIL_COLUMN_Y_AMOUNT);
			real_play_index_in_page = real_play_index / (THUMBNAIL_ROW_X_AMOUNT*THUMBNAIL_COLUMN_Y_AMOUNT);				
			page_showed_index = (real_play_index-1) / (THUMBNAIL_ROW_X_AMOUNT*THUMBNAIL_COLUMN_Y_AMOUNT);
			
			//icon_order_in_one_page = ((info_ptr->play_index+1) % (THUMBNAIL_ROW_X_AMOUNT*THUMBNAIL_COLUMN_Y_AMOUNT));
			icon_order_in_one_page = real_play_index - (THUMBNAIL_ROW_X_AMOUNT*THUMBNAIL_COLUMN_Y_AMOUNT)*real_play_index_in_page;	//Avoiding use %
			
			if( real_play_index > total_page_number*THUMBNAIL_ROW_X_AMOUNT*THUMBNAIL_COLUMN_Y_AMOUNT){
				icon_showed_number = (info_ptr->total_file_number) - (THUMBNAIL_ROW_X_AMOUNT*THUMBNAIL_COLUMN_Y_AMOUNT)*total_page_number;	//Avoiding use %
			}else{
				icon_showed_number = THUMBNAIL_ROW_X_AMOUNT*THUMBNAIL_COLUMN_Y_AMOUNT;
			}

			if(icon_order_in_one_page == 0){
				icon_order_in_one_page = (THUMBNAIL_ROW_X_AMOUNT*THUMBNAIL_COLUMN_Y_AMOUNT) - 1;
			}else{
				icon_order_in_one_page--;
			}

			icon_row_order = (icon_order_in_one_page)/THUMBNAIL_ROW_X_AMOUNT;
			//icon_column_order = (icon_order_in_one_page-icon_showed_num_in_one_page)%THUMBNAIL_ROW_X_AMOUNT;
			icon_column_order = (icon_order_in_one_page) - THUMBNAIL_ROW_X_AMOUNT*icon_row_order;	//Avoiding use %

			if( ((page_showed_order != page_showed_index) && (thumbnail_cursor_drawed == 0)) || (clean_showed_buffer == 1) ){
				gp_memset((INT8S *)thumbnail_output_buff, 0, getDispDevBufSize());
				clean_showed_buffer = 0;
			}

			//first time show or page order changed	
			if( (page_showed_order != page_showed_index) || (thumbnail_entrance_first_time == 1) )
			{
				//the first decoded icon won't have to decode twice
				if(info_ptr->play_index != first_entrance_play_index){
					ret = ap_thumbnail_jpeg_decode(info_ptr);
					close(info_ptr->file_handle);
					//copy from temp buffer to output buffer
					for(i=0; i<thumbnail_icon_h; i++)
					{
						if(ap_display_get_device() == DISP_DEV_TFT) { //TFT
							X_shiftment = (i+(icon_row_order*thumbnail_icon_h+(icon_row_order+1)*THUMBNAIL_CURSOR_HORIZONTAL_WIDTH)+THUMBNAIL_ICON_START_Y_POSITION)*TFT_WIDTH + \
										  THUMBNAIL_ICON_START_X_POSITION + ((icon_column_order+1)*THUMBNAIL_CURSOR_VERTICAL_WIDTH);
						} else { //TV
							X_shiftment = (i+(icon_row_order*thumbnail_icon_h+(icon_row_order+1)*THUMBNAIL_CURSOR_HORIZONTAL_WIDTH)+THUMBNAIL_ICON_START_Y_POSITION_TV)*TV_WIDTH + \
										  THUMBNAIL_ICON_START_X_POSITION_TV + ((icon_column_order+1)*THUMBNAIL_CURSOR_VERTICAL_WIDTH);
						}

						X_shiftment_2 = icon_column_order*thumbnail_icon_w;
						X_shiftment_temp = i*thumbnail_icon_w;
						for(j=0; j<(thumbnail_icon_w); j++)
						{
							if (ret == STATUS_OK) {
								*((INT16U *)thumbnail_output_buff + X_shiftment + (j + X_shiftment_2) ) = *((INT16U *)thumbnail_temp_buff + (X_shiftment_temp + j) );
							} else {	//JPEG decode fail
								*((INT16U *)thumbnail_output_buff + X_shiftment + (j + X_shiftment_2) ) = 0xD181;	//using dark red
							}
						}
					}

					if(thumbnail_cursor_drawed == 0){
						//erase the last cursor
						//don't need to do this anymore because we clean buffer when page changed.

						//draw cursor
						icon.icon_w = thumbnail_icon_w + THUMBNAIL_CURSOR_VERTICAL_WIDTH*2;
						icon.icon_h = thumbnail_icon_h + THUMBNAIL_CURSOR_HORIZONTAL_WIDTH*2;
						icon.transparent = TRANSPARENT_COLOR;
						icon_row_order = (icon_order_in_one_page)/THUMBNAIL_ROW_X_AMOUNT;
						icon_column_order = (icon_order_in_one_page) - THUMBNAIL_ROW_X_AMOUNT*icon_row_order;	//Avoiding use %
						if(ap_display_get_device() == DISP_DEV_TFT) { //TFT
							icon.pos_x = THUMBNAIL_ICON_START_X_POSITION + icon_column_order*THUMBNAIL_CURSOR_VERTICAL_WIDTH + icon_column_order*thumbnail_icon_w;
							icon.pos_y = THUMBNAIL_ICON_START_Y_POSITION + icon_row_order*THUMBNAIL_CURSOR_HORIZONTAL_WIDTH + icon_row_order*thumbnail_icon_h;
						} else {
							icon.pos_x = THUMBNAIL_ICON_START_X_POSITION_TV + icon_column_order*THUMBNAIL_CURSOR_VERTICAL_WIDTH + icon_column_order*thumbnail_icon_w;
							icon.pos_y = THUMBNAIL_ICON_START_Y_POSITION_TV + icon_row_order*THUMBNAIL_CURSOR_HORIZONTAL_WIDTH + icon_row_order*thumbnail_icon_h;
						}
						//ap_thumbnail_icon_draw((INT16U *)thumbnail_output_buff, thumbnail_cursor_3x3_96x64, &icon, 0x1);
						ap_thumbnail_cursor_draw((INT16U *)thumbnail_output_buff, thumbnail_cursor_3x3_96x64, &icon, 0x1);
						thumbnail_cursor_drawed = 1;
					}

					//draw video icon and index
					if(info_ptr->file_type == TK_IMAGE_TYPE_MOTION_JPEG){
						icon.icon_w = 32;
						icon.icon_h = 32;
						icon.transparent = TRANSPARENT_COLOR;
						if(ap_display_get_device() == DISP_DEV_TFT) { //TFT
							icon.pos_x = THUMBNAIL_ICON_START_X_POSITION + icon_column_order*THUMBNAIL_CURSOR_VERTICAL_WIDTH + icon_column_order*thumbnail_icon_w + THUMBNAIL_VIDEO_ICON_X_OFFSET;
							icon.pos_y = THUMBNAIL_ICON_START_Y_POSITION + icon_row_order*THUMBNAIL_CURSOR_HORIZONTAL_WIDTH + icon_row_order*thumbnail_icon_h + THUMBNAIL_VIDEO_ICON_Y_OFFSET;
						} else { //TV
							icon.pos_x = THUMBNAIL_ICON_START_X_POSITION_TV + icon_column_order*THUMBNAIL_CURSOR_VERTICAL_WIDTH + icon_column_order*thumbnail_icon_w + THUMBNAIL_VIDEO_ICON_X_OFFSET_TV;
							icon.pos_y = THUMBNAIL_ICON_START_Y_POSITION_TV + icon_row_order*THUMBNAIL_CURSOR_HORIZONTAL_WIDTH + icon_row_order*thumbnail_icon_h + THUMBNAIL_VIDEO_ICON_Y_OFFSET_TV;
						}
						ap_thumbnail_icon_draw((INT16U *)thumbnail_output_buff, icon_playback_movie, &icon, 0x1);						
				    }
				    stat((CHAR *)info_ptr->file_path_addr, &buf_tmp);	//check this file is Locked or not
				    if(buf_tmp.st_mode & D_RDONLY){
						icon.icon_w = 32;
						icon.icon_h = 32;
						icon.transparent = TRANSPARENT_COLOR;
						if(ap_display_get_device() == DISP_DEV_TFT) { //TFT
							icon.pos_x = THUMBNAIL_ICON_START_X_POSITION + icon_column_order*THUMBNAIL_CURSOR_VERTICAL_WIDTH + icon_column_order*thumbnail_icon_w + THUMBNAIL_LOCKED_ICON_X_OFFSET;
							icon.pos_y = THUMBNAIL_ICON_START_Y_POSITION + icon_row_order*THUMBNAIL_CURSOR_HORIZONTAL_WIDTH + icon_row_order*thumbnail_icon_h + THUMBNAIL_LOCKED_ICON_Y_OFFSET;
						} else { //TV
							icon.pos_x = THUMBNAIL_ICON_START_X_POSITION_TV + icon_column_order*THUMBNAIL_CURSOR_VERTICAL_WIDTH + icon_column_order*thumbnail_icon_w + THUMBNAIL_LOCKED_ICON_X_OFFSET_TV;
							icon.pos_y = THUMBNAIL_ICON_START_Y_POSITION_TV + icon_row_order*THUMBNAIL_CURSOR_HORIZONTAL_WIDTH + icon_row_order*thumbnail_icon_h + THUMBNAIL_LOCKED_ICON_Y_OFFSET_TV;
						}
						ap_thumbnail_icon_draw((INT16U *)thumbnail_output_buff, icon_locked/*thumbnail_lock_icon*/, &icon, 0x1);
					}

					if(real_play_index < 10){
						thumbnail_file_path[1] = 0;
						thumbnail_file_path[0] = real_play_index + 0x30;
					}else if(real_play_index < 100){
						thumbnail_file_path[2] = 0;
						thumbnail_file_path[0] = (real_play_index/10) + 0x30;
						thumbnail_file_path[1] = (real_play_index - 10*(thumbnail_file_path[0]-0x30)) + 0x30;
					}else if(real_play_index < 1000){				
						thumbnail_file_path[3] = 0;
						thumbnail_file_path[0] = (real_play_index/100) + 0x30;
						thumbnail_file_path[1] = ((real_play_index - 100*(thumbnail_file_path[0]-0x30))/10) + 0x30;
						thumbnail_file_path[2] = ((real_play_index - 100*(thumbnail_file_path[0]-0x30)) - 10*(thumbnail_file_path[1]-0x30)) + 0x30;
					}else if(real_play_index < 10000){
						thumbnail_file_path[4] = 0;
						thumbnail_file_path[0] = (real_play_index/1000) + 0x30;
						thumbnail_file_path[1] = ((real_play_index - 1000*(thumbnail_file_path[0]-0x30))/100) + 0x30;
						thumbnail_file_path[2] = ((real_play_index - 1000*(thumbnail_file_path[0]-0x30) - 100*(thumbnail_file_path[1]-0x30))/10) + 0x30;
						thumbnail_file_path[3] = ((real_play_index - 1000*(thumbnail_file_path[0]-0x30) - 100*(thumbnail_file_path[1]-0x30)) - 10*(thumbnail_file_path[2]-0x30)) + 0x30;
					}

					ascii_str.font_color = THUMBNAIL_ICON_INDEX_COLOR;
					ascii_str.font_type = 0;
					ascii_str.str_ptr = (char *) thumbnail_file_path;
					if(ap_display_get_device() == DISP_DEV_TFT) { //TFT
						ascii_str.pos_x = THUMBNAIL_ICON_START_X_POSITION + icon_column_order*THUMBNAIL_CURSOR_VERTICAL_WIDTH + icon_column_order*thumbnail_icon_w + THUMBNAIL_ICON_INDEX_X_OFFSET;
						ascii_str.pos_y = THUMBNAIL_ICON_START_Y_POSITION + icon_row_order*THUMBNAIL_CURSOR_HORIZONTAL_WIDTH + icon_row_order*thumbnail_icon_h + THUMBNAIL_ICON_INDEX_Y_OFFSET;
						ascii_str.buff_w = TFT_WIDTH;
						ascii_str.buff_h = TFT_HEIGHT;
					} else { //TV
						ascii_str.pos_x = THUMBNAIL_ICON_START_X_POSITION_TV + icon_column_order*THUMBNAIL_CURSOR_VERTICAL_WIDTH + icon_column_order*thumbnail_icon_w + THUMBNAIL_ICON_INDEX_X_OFFSET_TV;
						ascii_str.pos_y = THUMBNAIL_ICON_START_Y_POSITION_TV + icon_row_order*THUMBNAIL_CURSOR_HORIZONTAL_WIDTH + icon_row_order*thumbnail_icon_h + THUMBNAIL_ICON_INDEX_Y_OFFSET_TV;
						ascii_str.buff_w = TV_WIDTH;
						ascii_str.buff_h = TV_HEIGHT;
					}
					ap_state_resource_string_ascii_draw((INT16U *) thumbnail_output_buff, &ascii_str, RGB565_DRAW);
	
					OSQPost(DisplayTaskQ, (void *) (thumbnail_output_buff|MSG_DISPLAY_TASK_JPEG_DRAW));
					icon_showed_num_in_one_page++;
				}else{
					close(info_ptr->file_handle);
					//if ( (audio_playing_state_get() == STATE_IDLE) || (audio_playing_state_get() == STATE_PAUSED) ){
						msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
					//}
				}

				if(showed_number_before_after_play_index_updated == 0){
					first_entrance_play_index = info_ptr->play_index;
					icon_showed_number_before_play_index = icon_order_in_one_page;
					icon_showed_number_after_play_index = icon_showed_number - icon_order_in_one_page - 1;
					showed_number_before_after_play_index_updated = 1;
				}

				if(icon_showed_num_in_one_page == icon_showed_number)
				{
					if(icon_showed_number_after_play_index == 0){
						search_type = STOR_SERV_SEARCH_GIVEN | (first_entrance_play_index << 16);
						icon_showed_number_after_play_index = -1;
						page_showed_order = -1;
						msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_THUMBNAIL_REQ, (void *) &search_type, sizeof(INT32U), MSG_PRI_NORMAL);
					}else{																	
						ascii_str.font_color = THUMBNAIL_FILE_NAME_COLOR;
						ascii_str.font_type = 0;
						ascii_str.str_ptr = (char *) info_ptr->file_path_addr;
						if(ap_display_get_device() == DISP_DEV_TFT) { //TFT
							ascii_str.buff_w = TFT_WIDTH;
							ascii_str.buff_h = TFT_HEIGHT;
							ascii_str.pos_x = THUMBNAIL_FILE_NAME_START_X;
							ascii_str.pos_y = THUMBNAIL_FILE_NAME_START_Y;
						} else { //TV
							ascii_str.buff_w = TV_WIDTH;
							ascii_str.buff_h = TV_HEIGHT;
							ascii_str.pos_x = THUMBNAIL_FILE_NAME_START_X_TV;
							ascii_str.pos_y = THUMBNAIL_FILE_NAME_START_Y_TV;
						}
						ap_state_resource_string_ascii_draw((INT16U *) thumbnail_output_buff, &ascii_str, RGB565_DRAW);
						ap_state_handling_icon_clear_cmd(ICON_INTERNAL_MEMORY, NULL, NULL);
						icon_showed_num_in_one_page = 0;
						thumbnail_cursor_drawed = 0;
						showed_number_before_after_play_index_updated = 0;
						ap_thumbnail_daw_page_num(real_play_index,total_file_num);

						OSQPost(DisplayTaskQ, (void *) (thumbnail_output_buff|MSG_DISPLAY_TASK_JPEG_DRAW));
						g_thumbnail_reply_action_flag = 1;

						//check if there is any key was hold
					}
				}else{

					if(icon_showed_number_before_play_index > 0)
					{
						search_type = STOR_SERV_SEARCH_PREV;
						icon_showed_number_before_play_index--;
					}else if(icon_showed_number_before_play_index == 0){
						search_type = STOR_SERV_SEARCH_GIVEN | (first_entrance_play_index << 16);
						icon_showed_number_before_play_index = -1;
					}else if(icon_showed_number_after_play_index > 0){
						search_type = STOR_SERV_SEARCH_NEXT;
						icon_showed_number_after_play_index--;
					}else if(icon_showed_number_after_play_index == 0){
						search_type = STOR_SERV_SEARCH_GIVEN | (first_entrance_play_index << 16);
						icon_showed_number_after_play_index = -1;
					}															
					page_showed_order = -1;
					msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_THUMBNAIL_REQ, (void *) &search_type, sizeof(INT32U), MSG_PRI_NORMAL);
				}
			} else {
				//page order is the same, only change cursor.

				close(info_ptr->file_handle);

				//erase the last cursor
				icon.icon_w = thumbnail_icon_w + THUMBNAIL_CURSOR_VERTICAL_WIDTH*2;
				icon.icon_h = thumbnail_icon_h + THUMBNAIL_CURSOR_HORIZONTAL_WIDTH*2;
				icon.transparent = TRANSPARENT_COLOR;					
				if( (total_file_num <= 9) && ((cursor_play_index - ((info_ptr->play_index) - (info_ptr->deleted_file_number))) == (total_file_num-1)) && (icon_order_in_one_page == 0) ){
					icon_row_order = (total_file_num-1)/THUMBNAIL_ROW_X_AMOUNT;
					icon_column_order = (total_file_num-1) - THUMBNAIL_ROW_X_AMOUNT*icon_row_order;	//Avoiding use %
				}else if( (total_file_num <= 9)	&& ((((info_ptr->play_index) - (info_ptr->deleted_file_number)) - cursor_play_index) == (total_file_num-1)) && (icon_order_in_one_page == (total_file_num-1)) ){
					icon_row_order = 0;
					icon_column_order = 0;
				}else if(((info_ptr->play_index) - (info_ptr->deleted_file_number)) < cursor_play_index){
					if(icon_order_in_one_page+1 < THUMBNAIL_ROW_X_AMOUNT*THUMBNAIL_COLUMN_Y_AMOUNT){
						icon_row_order = (icon_order_in_one_page+1)/THUMBNAIL_ROW_X_AMOUNT;
						icon_column_order = (icon_order_in_one_page+1) - THUMBNAIL_ROW_X_AMOUNT*icon_row_order;	//Avoiding use %
					}else if (icon_order_in_one_page+1 == THUMBNAIL_ROW_X_AMOUNT*THUMBNAIL_COLUMN_Y_AMOUNT){
						icon_row_order = 0;
						icon_column_order = 0;
					}
				}else if(((info_ptr->play_index) - (info_ptr->deleted_file_number)) > cursor_play_index){
					if(icon_order_in_one_page-1 >= 0){
						icon_row_order = (icon_order_in_one_page-1)/THUMBNAIL_ROW_X_AMOUNT;
						icon_column_order = (icon_order_in_one_page-1) - THUMBNAIL_ROW_X_AMOUNT*icon_row_order;	//Avoiding use %
					}else if(icon_order_in_one_page == 0){
						icon_row_order = 2;
						icon_column_order = 2;
					}
				}
				if(ap_display_get_device() == DISP_DEV_TFT) { //TFT
					icon.pos_x = THUMBNAIL_ICON_START_X_POSITION + icon_column_order*THUMBNAIL_CURSOR_VERTICAL_WIDTH + icon_column_order*thumbnail_icon_w;
					icon.pos_y = THUMBNAIL_ICON_START_Y_POSITION + icon_row_order*THUMBNAIL_CURSOR_HORIZONTAL_WIDTH + icon_row_order*thumbnail_icon_h;
				} else { //TV
					icon.pos_x = THUMBNAIL_ICON_START_X_POSITION_TV + icon_column_order*THUMBNAIL_CURSOR_VERTICAL_WIDTH + icon_column_order*thumbnail_icon_w;
					icon.pos_y = THUMBNAIL_ICON_START_Y_POSITION_TV + icon_row_order*THUMBNAIL_CURSOR_HORIZONTAL_WIDTH + icon_row_order*thumbnail_icon_h;
				}
				//ap_thumbnail_icon_draw((INT16U *)thumbnail_output_buff, thumbnail_cursor_3x3_black_96x64, &icon, 0x1);
				ap_thumbnail_cursor_draw((INT16U *)thumbnail_output_buff, thumbnail_cursor_3x3_black_96x64, &icon, 0x1);

				//draw cursor
				icon_row_order = (icon_order_in_one_page)/THUMBNAIL_ROW_X_AMOUNT;
				icon_column_order = (icon_order_in_one_page) - THUMBNAIL_ROW_X_AMOUNT*icon_row_order;	//Avoiding use %
				if(ap_display_get_device() == DISP_DEV_TFT) { //TFT
					icon.pos_x = THUMBNAIL_ICON_START_X_POSITION + icon_column_order*THUMBNAIL_CURSOR_VERTICAL_WIDTH + icon_column_order*thumbnail_icon_w;
					icon.pos_y = THUMBNAIL_ICON_START_Y_POSITION + icon_row_order*THUMBNAIL_CURSOR_HORIZONTAL_WIDTH + icon_row_order*thumbnail_icon_h;
				} else { //TV
					icon.pos_x = THUMBNAIL_ICON_START_X_POSITION_TV + icon_column_order*THUMBNAIL_CURSOR_VERTICAL_WIDTH + icon_column_order*thumbnail_icon_w;
					icon.pos_y = THUMBNAIL_ICON_START_Y_POSITION_TV + icon_row_order*THUMBNAIL_CURSOR_HORIZONTAL_WIDTH + icon_row_order*thumbnail_icon_h;
				}
				//ap_thumbnail_icon_draw((INT16U *)thumbnail_output_buff, thumbnail_cursor_3x3_96x64, &icon, 0x1);
				ap_thumbnail_cursor_draw((INT16U *)thumbnail_output_buff, thumbnail_cursor_3x3_96x64, &icon, 0x1);

				//clean file name area
				if(ap_display_get_device() == DISP_DEV_TFT) { //TFT
					gp_memset((INT8S *)(thumbnail_output_buff + TFT_WIDTH*THUMBNAIL_FILE_NAME_AREA_Y*2), 0, TFT_WIDTH*(TFT_HEIGHT - THUMBNAIL_FILE_NAME_AREA_Y)*2);
				} else { //TV
					gp_memset((INT8S *)(thumbnail_output_buff + TV_WIDTH*THUMBNAIL_FILE_NAME_AREA_Y_TV*2), 0, TV_WIDTH*(TV_HEIGHT - THUMBNAIL_FILE_NAME_AREA_Y_TV)*2);
				}

				ascii_str.font_color = THUMBNAIL_FILE_NAME_COLOR;
				ascii_str.font_type = 0;
				ascii_str.str_ptr = (char *) info_ptr->file_path_addr;
				if(ap_display_get_device() == DISP_DEV_TFT) { //TFT
					ascii_str.buff_w = TFT_WIDTH;
					ascii_str.buff_h = TFT_HEIGHT;
					ascii_str.pos_x = THUMBNAIL_FILE_NAME_START_X;
					ascii_str.pos_y = THUMBNAIL_FILE_NAME_START_Y;
				} else { //TV
					ascii_str.buff_w = TV_WIDTH;
					ascii_str.buff_h = TV_HEIGHT;
					ascii_str.pos_x = THUMBNAIL_FILE_NAME_START_X_TV;
					ascii_str.pos_y = THUMBNAIL_FILE_NAME_START_Y_TV;
				}
				ap_state_resource_string_ascii_draw((INT16U *) thumbnail_output_buff, &ascii_str, RGB565_DRAW);
				ap_state_handling_icon_clear_cmd(ICON_INTERNAL_MEMORY, NULL, NULL);					
				
				ap_thumbnail_daw_page_num(real_play_index,total_file_num);
				
				OSQPost(DisplayTaskQ, (void *) (thumbnail_output_buff|MSG_DISPLAY_TASK_JPEG_DRAW));
				msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
				g_thumbnail_reply_action_flag = 1;
			}
			cursor_play_index = ((info_ptr->play_index) - (info_ptr->deleted_file_number));
			deleted_file_num = info_ptr->deleted_file_number;
							
		} else if (info_ptr->err_flag == STOR_SERV_NO_MEDIA) {
			g_thumbnail_reply_action_flag = 1;

			total_file_num = 0;
			ap_state_handling_icon_clear_cmd(ICON_INTERNAL_MEMORY, NULL, NULL);
			ap_thumbnail_no_media_show(STR_NO_MEDIA);
			msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
		} else {
			g_thumbnail_reply_action_flag = 1;

			total_file_num = 0;
			if (ap_state_handling_storage_id_get() == NO_STORAGE) {
				ap_thumbnail_no_media_show(STR_NO_SD);
			} else {
				ap_state_handling_icon_clear_cmd(ICON_INTERNAL_MEMORY, NULL, NULL);
				ap_thumbnail_no_media_show(STR_NO_MEDIA);
				if (info_ptr->err_flag == STOR_SERV_DECODE_ALL_FAIL) {
					close(info_ptr->file_handle);
				}
			}
			msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
		}
		ap_thumbnail_sts_set(~THUMBNAIL_DECODE_BUSY);
	}
}

INT8S ap_thumbnail_stop_handle(void)
{
	icon_showed_num_in_one_page = 0;
	thumbnail_entrance_first_time = 0;
	thumbnail_cursor_drawed = 0;
	showed_number_before_after_play_index_updated = 0;
	icon_showed_number_before_play_index = 0;
	icon_showed_number_after_play_index = 0;
	first_entrance_play_index = -1;
	cursor_play_index = 0;
	deleted_file_num = 0;
	page_showed_order = -1;
	clean_showed_buffer = 1;
	thumbnail_next_key_hold_number = -1;
	thumbnail_prev_key_hold_number = -1;
	
	return 0;
}

void ap_thumbnail_connect_to_pc(void)
{
	if (ap_thumbnail_stop_handle() & (THUMBNAIL_PLAYBACK_BUSY | THUMBNAIL_PLAYBACK_PAUSE)) {
		OSQPost(DisplayTaskQ, (void *) (thumbnail_output_buff|MSG_DISPLAY_TASK_JPEG_DRAW));
	} else {
		OSQPost(DisplayTaskQ, (void *) (thumbnail_output_buff|MSG_DISPLAY_TASK_JPEG_DRAW));
	}
}

void ap_thumbnail_disconnect_to_pc(void)
{
	OSQPost(DisplayTaskQ, (void *) (thumbnail_output_buff|MSG_DISPLAY_TASK_JPEG_DRAW));
}

void ap_thumbnail_no_media_show(INT16U str_type)
{
	INT32U i, *buff_ptr, color_data, cnt;
	
	buff_ptr = (INT32U *) thumbnail_output_buff;
	color_data = 0x11a4 | (0x11a4<<16);
	cnt = getDispDevBufSize()>>2;
	for (i=0 ; i<cnt ; i++) {
		*buff_ptr++ = color_data;
	}
	if (str_type == STR_NO_SD) {
		ap_state_handling_icon_show_cmd(ICON_INTERNAL_MEMORY, NULL, NULL);
	}
	total_file_num = 0;
	ap_state_handling_str_draw_exit();
	ap_state_handling_str_draw(str_type, WARNING_STR_COLOR);
	OSQPost(DisplayTaskQ, (void *) (thumbnail_output_buff|MSG_DISPLAY_TASK_JPEG_DRAW));
}

void ap_thumbnail_icon_draw(INT16U *frame_buff, INT16U *icon_stream, DISPLAY_ICONSHOW *icon_info, INT8U type)
{
	INT32U x, y, offset_pixel, offset_pixel_tmp, offset_data, offset_data_tmp, pixel_limit;
		
	if(ap_display_get_device() == DISP_DEV_TFT) { //TFT
		pixel_limit = TFT_WIDTH*TFT_HEIGHT;
	} else { //TV
		pixel_limit = TV_WIDTH*TV_HEIGHT;
	}

	offset_data = 0;
	for (y=0 ; y<icon_info->icon_h ; y++) {
		offset_data_tmp = y*icon_info->icon_w;
		if(ap_display_get_device() == DISP_DEV_TFT) { //TFT
			offset_pixel_tmp = (icon_info->pos_y + y)*TFT_WIDTH + icon_info->pos_x;
		} else { //TV
			offset_pixel_tmp = (icon_info->pos_y + y)*TV_WIDTH + icon_info->pos_x;
		}
		for (x=0 ; x<icon_info->icon_w ; x++) {
			if (type) {
				offset_data = offset_data_tmp + x;
			}
			if (*(icon_stream + offset_data) == icon_info->transparent) {
				continue;
			}
			offset_pixel = offset_pixel_tmp + x;
			if (offset_pixel < pixel_limit) {
				*(frame_buff + offset_pixel) = *(icon_stream + offset_data);
			}
		}
	}
}


void ap_thumbnail_cursor_draw(INT16U *frame_buff, INT16U *icon_stream, DISPLAY_ICONSHOW *icon_info, INT8U type)
{
	INT32U x, y, offset_pixel, offset_pixel_tmp, pixel_limit;//, offset_data, offset_data_tmp;
	INT16U icon_data;

	if(ap_display_get_device() == DISP_DEV_TFT) { //TFT
		pixel_limit = TFT_WIDTH*TFT_HEIGHT;
	} else { //TV
		pixel_limit = TV_WIDTH*TV_HEIGHT;
	}

	//offset_data = 0;
	for (y=0 ; y<icon_info->icon_h ; y++) {
		//offset_data_tmp = y*icon_info->icon_w;
		if(ap_display_get_device() == DISP_DEV_TFT) { //TFT
			offset_pixel_tmp = (icon_info->pos_y + y)*TFT_WIDTH + icon_info->pos_x;
		} else { //TV
			offset_pixel_tmp = (icon_info->pos_y + y)*TV_WIDTH + icon_info->pos_x;
		}
		for (x=0 ; x<icon_info->icon_w ; x++) {
			/*
			if (type) {
				offset_data = offset_data_tmp + x;
			}
			if (*(icon_stream + offset_data) == icon_info->transparent) {
				continue;
			} */
			
			if((y <= 2) || (y >= icon_info->icon_h - 3)) {
				if(icon_stream == thumbnail_cursor_3x3_black_96x64) {
					icon_data = 0;
				} else if(icon_stream == thumbnail_cursor_3x3_96x64) {
					icon_data = 0xff26;
				} else {
					return;
				}
			} else {
				if((x <= 2) || (x >= icon_info->icon_w - 3)) {
					if(icon_stream == thumbnail_cursor_3x3_black_96x64) {
						icon_data = 0;
					} else if(icon_stream == thumbnail_cursor_3x3_96x64) {
						icon_data = 0xff26;
					} else {
						return;
					}
				} else {
					continue;
				}
			}
			
			offset_pixel = offset_pixel_tmp + x;
			if (offset_pixel < pixel_limit) {
				*(frame_buff + offset_pixel) = icon_data;//*(icon_stream + offset_data);
			}
		}
	}
}


INT32S ap_thumbnail_jpeg_decode(STOR_SERV_PLAYINFO *info_ptr)
{
	INT32U input_buff=0, size, shift_byte, data_tmp;
	// IMAGE_DECODE_STRUCT img_info;
	INT16U logo_fd;
	INT8U temp;
	SCALER_MAS scaler_mas;
	INT32S status;

	ap_state_handling_icon_clear_cmd(ICON_PAUSE, ICON_PLAY, NULL);
	ap_state_handling_icon_clear_cmd(ICON_PLAY1, ICON_PAUSE1, NULL);
	if(info_ptr->file_type == TK_IMAGE_TYPE_WAV) { // WAV
		logo_fd = nv_open((INT8U *) "AUDIO_REC_BG.JPG");
		if(logo_fd != 0xffff) {
			size = nv_rs_size_get(logo_fd);

			input_buff = (INT32U) gp_malloc(size);
			if (!input_buff) {
				DBG_PRINT("State thumbnail allocate jpeg input buffer fail.[%d]\r\n", size);
				return STATUS_FAIL;
			}
			if (nv_read(logo_fd, (INT32U) input_buff, size)) {
				DBG_PRINT("Failed to read resource_header in ap_thumbnail_jpeg_decode()\r\n");
				gp_free((void *) input_buff);
				return STATUS_FAIL;
			}
			temp = 0x80;
		} else {
			return STATUS_FAIL;
		}
	} else { 
		if (info_ptr->file_type == TK_IMAGE_TYPE_JPEG) {  // JPEG
			size = info_ptr->file_size;
			shift_byte = 0;
			temp = 0x80;
		} else { // AVI
			shift_byte = 4096;//272; //modified by wwj
			lseek(info_ptr->file_handle, shift_byte, SEEK_SET);
			if (read(info_ptr->file_handle, (INT32U) &data_tmp, 4) != 4) {
				return STATUS_FAIL;
			}

			read(info_ptr->file_handle, (INT32U) &size, 4);
			if ((data_tmp == AUDIO_STREAM) || (size == 0)) {
				do {
					shift_byte += (size + 8);
					lseek(info_ptr->file_handle, shift_byte, SEEK_SET);
					read(info_ptr->file_handle, (INT32U) &data_tmp, 4);
					read(info_ptr->file_handle, (INT32U) &size, 4);
				} while ((data_tmp == AUDIO_STREAM) || (data_tmp == JUNK_DATA) || (size == 0));
			} else if (data_tmp != VIDEO_STREAM) {
				return STATUS_FAIL;
			}
    		input_buff = (INT32U) gp_malloc(size);
    		if (!input_buff) {
    			DBG_PRINT("State thumbnail allocate jpeg input buffer fail.[%d]\r\n", size);
    			return STATUS_FAIL;
    		}
    		if (read(info_ptr->file_handle, input_buff, size) <= 0) {
    			gp_free((void *) input_buff);
    			DBG_PRINT("State thumbnail read jpeg file fail.\r\n");
    			return STATUS_FAIL;
    		}			
		}
	}

	gp_memset((INT8S *) &scaler_mas, 0, sizeof(SCALER_MAS));
	scaler_mas.mas_0 = MAS_EN_READ|MAS_EN_WRITE;
	scaler_mas_set(SCALER_1, &scaler_mas);

	if (ap_display_get_device()==DISP_DEV_HDMI) {		// GP420
#ifdef HDMI_JPG_DECODE_AS_GP420
	    if (info_ptr->file_type == TK_IMAGE_TYPE_JPEG)
    		status = video_decode_jpeg_as_gp420_by_piece(info_ptr->file_handle, size, THUMBNAIL_ICON_WIDTH, THUMBNAIL_ICON_HEIGHT, thumbnail_temp_buff);
    	else
    	   status = video_decode_jpeg_as_gp420((INT8U *) input_buff, size, THUMBNAIL_ICON_WIDTH, THUMBNAIL_ICON_HEIGHT, thumbnail_temp_buff);
#elif defined(HDMI_JPG_DECODE_AS_YUV422)
	    if (info_ptr->file_type == TK_IMAGE_TYPE_JPEG)
    		status = video_decode_jpeg_as_gp422_by_piece(info_ptr->file_handle, size, THUMBNAIL_ICON_WIDTH, THUMBNAIL_ICON_HEIGHT, thumbnail_temp_buff);
    	else
    	   status = video_decode_jpeg_as_gp422((INT8U *) input_buff, size, THUMBNAIL_ICON_WIDTH, THUMBNAIL_ICON_HEIGHT, thumbnail_temp_buff);
#endif
	} else if (ap_display_get_device()==DISP_DEV_TV) {	// TV, RGB565
	    if (info_ptr->file_type == TK_IMAGE_TYPE_JPEG)
	        status = video_decode_jpeg_as_rgb565_by_piece(info_ptr->file_handle, size, THUMBNAIL_ICON_WIDTH_TV, THUMBNAIL_ICON_HEIGHT_TV, thumbnail_temp_buff);
	    else
		    status = video_decode_jpeg_as_rgb565((INT8U *)input_buff, size, THUMBNAIL_ICON_WIDTH_TV, THUMBNAIL_ICON_HEIGHT_TV, thumbnail_temp_buff);
	} else { // TFT, RGB565
	    if (info_ptr->file_type == TK_IMAGE_TYPE_JPEG)
            status = video_decode_jpeg_as_rgb565_by_piece(info_ptr->file_handle, size, THUMBNAIL_ICON_WIDTH, THUMBNAIL_ICON_HEIGHT, thumbnail_temp_buff);	    
	    else
    		status = video_decode_jpeg_as_rgb565((INT8U *)input_buff, size, THUMBNAIL_ICON_WIDTH, THUMBNAIL_ICON_HEIGHT, thumbnail_temp_buff);
	}
	
	if (input_buff)
    	gp_free((void *) input_buff);
	close(info_ptr->file_handle);
	//if ( (audio_playing_state_get() == STATE_IDLE) || (audio_playing_state_get() == STATE_PAUSED) ){
		msgQSend(StorageServiceQ, MSG_STORAGE_SERVICE_TIMER_START, NULL, NULL, MSG_PRI_NORMAL);
	//}
	if (status) {
		return STATUS_FAIL;
	}
	return STATUS_OK;
}

void ap_thumbnail_clean_frame_buffer(void)
{
	gp_memset((INT8S *)thumbnail_output_buff, 0, getDispDevBufSize());
}