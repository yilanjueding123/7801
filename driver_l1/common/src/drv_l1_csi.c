/*
* Purpose: CMOS sensor interface driver
*
* Author: Tristan Yang
*
* Date: 2013/10/25
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 1.00
* History :
*/

#include "drv_l1_csi.h"
#include "drv_l1_interrupt.h"


#define C_CSI_DRV_MAX_FRAME_NUM				32
#define C_CSI_DRV_DUMMY_BUFFER				0xF8500000			// Dummy address

static INT32U free_queue_buffer[C_CSI_DRV_MAX_FRAME_NUM];
static INT8U buf_read, buf_write;

static void (*csi_isr_callback)(CSI_EVENT_ENUM event, INT32U buffer);
static INT8U csi_stop_flag;
static INT8U csi_fifo_enable;
static INT8U csi_fifo_index;				// TBD: move this variable to internal RAM to save SDRAM bandwidth
static INT8U csi_under_run_flag;

static INT32S csi_device_protect(void);
static void csi_device_unprotect(INT32S mask);
static void csi_isr(void);

void drvl1_csi_init(void)
{
	R_CSI_TG_CTRL0 = C_CSI_CTRL1_DEDICATE_IRQ | C_CSI_CTRL1_OUT_YUV | C_CSI_CTRL1_IN_YUV | C_CSI_CTRL1_CAP;
	R_CSI_TG_CTRL1 = C_CSI_CTRL2_HIGH_PRIORITY | C_CSI_CTRL2_NON_STOP;
	R_CSI_SEN_CTRL	= 0x0;
	R_CSI_TG_HLSTART = 0x0;   			// Horizontal latch start
	R_CSI_TG_VL0START = 0x0;  	 	// Field 0 vertical latch start
	R_CSI_TG_VL1START = 0x0;	   	// Field 1 vertical latch start
	R_CSI_TG_HWIDTH = 640;		    			// Sensor width
	R_CSI_TG_VHEIGHT = 480; 	   				// Sensor height
	R_CSI_TG_HRATIO = 0x0;    					// Horizontal compress ratio control
	R_CSI_TG_VRATIO = 0x0;	    				// Vertical compress ratio control

	*P_CSI_TG_FBSADDR = C_CSI_DRV_DUMMY_BUFFER;	// Frame buffer start address(FIFO frame buffer A when FIFO mode is used)
	*P_CSI_TG_FBSADDR_B = C_CSI_DRV_DUMMY_BUFFER;	// FIFO frame buffer B when FIFO mode is used

	R_TGR_IRQ_EN = 0x0;       	    		// Disable IRQ
	R_TGR_IRQ_STATUS = R_TGR_IRQ_STATUS;   	// Clear IRQ status

	// Set black screen region
	R_CSI_TG_HSTART = 0x0;		// Black screen horizontal start
	R_CSI_TG_HEND = 0xFFF;		// Black screen horizontal end
	R_CSI_TG_VSTART = 0x0;		// Black screen vertical start
	R_CSI_TG_VEND = 0xFFF;		// Black screen vertical end

	*P_CSI_MD_FBADDR = C_CSI_DRV_DUMMY_BUFFER;
	R_CSI_MD_CTRL = 0x0;

	for (buf_write=0; buf_write<C_CSI_DRV_MAX_FRAME_NUM; buf_write++) {
		free_queue_buffer[buf_write] = C_CSI_DRV_DUMMY_BUFFER;
	}
	buf_read = buf_write = 0;

	vic_irq_register(VIC_CSI, csi_isr);
	vic_irq_enable(VIC_CSI);
}

void drvl1_csi_input_set(CSI_INPUT_INTERFACE_ENUM interface, CSI_INTERLACE_MODE_ENUM interlace, INT32U invert_clk, CSI_INPUT_DATA_ENUM data_format)
{
	if (interface == ENUM_CSI_HREF) {
		R_CSI_TG_CTRL0 |= C_CSI_CTRL1_HREF_MODE;
		R_CSI_TG_CTRL0 &= ~C_CSI_CTRL1_CCIR656;
	} else if (interface == ENUM_CSI_HSYNC_CCIR_656) {
		R_CSI_TG_CTRL0 &= ~C_CSI_CTRL1_HREF_MODE;
		R_CSI_TG_CTRL0 |= C_CSI_CTRL1_CCIR656;
	} else {		// ENUM_CSI_HSYNC_CCIR_601
		R_CSI_TG_CTRL0 &= ~(C_CSI_CTRL1_CCIR656 | C_CSI_CTRL1_HREF_MODE);
	}

	if (interlace == ENUM_CSI_NON_INTERLACE) {
		R_CSI_TG_CTRL0 &= ~(C_CSI_CTRL1_INTERLACE | C_CSI_CTRL1_FIELD_INVERT);
	} else if (interlace == ENUM_CSI_INTERLACE) {
		R_CSI_TG_CTRL0 &= ~C_CSI_CTRL1_FIELD_INVERT;
		R_CSI_TG_CTRL0 |= C_CSI_CTRL1_INTERLACE;
	} else {	// ENUM_CSI_INTERLACE_INVERT_FIELD
		R_CSI_TG_CTRL0 |= C_CSI_CTRL1_INTERLACE | C_CSI_CTRL1_FIELD_INVERT;
	}

	if (invert_clk) {
		R_CSI_TG_CTRL0 |= C_CSI_CTRL1_IN_CLOCK_INVERT;
	} else {
		R_CSI_TG_CTRL0 &= ~C_CSI_CTRL1_IN_CLOCK_INVERT;
	}

	if (data_format < ENUM_CSI_IN_UYVY) {
		// RGB
		R_CSI_TG_CTRL0 &= ~C_CSI_CTRL1_IN_YUV;

		if (data_format==ENUM_CSI_IN_RGB888_BGRG || data_format==ENUM_CSI_IN_RGB888_GBGR) {
			R_CSI_TG_CTRL0 &= ~C_CSI_CTRL1_RGB565;
		} else {
			R_CSI_TG_CTRL0 |= C_CSI_CTRL1_RGB565;
		}

		if (data_format==ENUM_CSI_IN_RGB888_BGRG || data_format==ENUM_CSI_IN_RGB565_BGRG) {
			R_CSI_TG_CTRL0 &= ~C_CSI_CTRL1_IN_YUYV_GBGR;
		} else {
			R_CSI_TG_CTRL0 |= C_CSI_CTRL1_IN_YUYV_GBGR;
		}
	} else {		// YUV
		R_CSI_TG_CTRL0 |= C_CSI_CTRL1_IN_YUV;

		if (data_format==ENUM_CSI_IN_UYVY || data_format==ENUM_CSI_IN_UYVY_INVERT_UV7) {
			R_CSI_TG_CTRL0 &= ~C_CSI_CTRL1_IN_YUYV_GBGR;
		} else {
			R_CSI_TG_CTRL0 |= C_CSI_CTRL1_IN_YUYV_GBGR;
		}

		if (data_format==ENUM_CSI_IN_UYVY || data_format==ENUM_CSI_IN_YUYV) {
			R_CSI_TG_CTRL1 &= ~C_CSI_CTRL2_IN_UV_BIT7_INVERT;
		} else {
			R_CSI_TG_CTRL1 |= C_CSI_CTRL2_IN_UV_BIT7_INVERT;
		}
	}
}

INT32S drvl1_csi_input_resolution_set(INT32U width, INT32U height)
{
	if (width>4095 || height>4095) {
		return -1;
	}

	R_CSI_TG_HWIDTH = width;
	R_CSI_TG_VHEIGHT = height;

	return 0;
}

// Latch control registers are used in HSYNC mode only. It is not used in HREF mode
void drvl1_csi_input_latch_timing1_set(INT32U h_start, INT32U field0_vstart, INT32U field1_vstart, CSI_LATCH_DELAY_ENUM delay)
{
	R_CSI_TG_HLSTART = h_start & 0x0FFF;  				// Horizontal latch start
	R_CSI_TG_VL0START = field0_vstart & 0x0FFF;  	// Field 0 vertical latch start
	R_CSI_TG_VL1START = field1_vstart & 0x0FFF;		// Field 1 vertical latch start when interlace mode is used

	R_CSI_TG_CTRL1 &= ~C_CSI_CTRL2_LATCH_DELAY_MASK;
	R_CSI_TG_CTRL1 |= delay;
}

void drvl1_csi_input_latch_timing2_set(CSI_LATCH_EDGE_ENUM field, CSI_LATCH_EDGE_ENUM v_reset, CSI_LATCH_EDGE_ENUM v_increase, CSI_LATCH_EDGE_ENUM h_reset)
{
	// Latch field at falling(0) or rising(1) edge of VSYNC
	if (field == ENUM_CSI_FALLING_EDGE) {
		R_CSI_TG_CTRL0 &= ~C_CSI_CTRL1_FIELD_LATCH_RISING;
	} else {
		R_CSI_TG_CTRL0 |= C_CSI_CTRL1_FIELD_LATCH_RISING;
	}

	// Vertical counter increase at falling(0) or rising(1) edge of HSYNC
	if (v_increase == ENUM_CSI_FALLING_EDGE) {
		R_CSI_TG_CTRL0 &= ~C_CSI_CTRL1_VADD;
	} else {
		R_CSI_TG_CTRL0 |= C_CSI_CTRL1_VADD;
	}

	// Vertical counter reset at falling(0) or rising(1) edge of VSYNC
	if (v_reset == ENUM_CSI_FALLING_EDGE) {
		R_CSI_TG_CTRL0 &= ~C_CSI_CTRL1_VRST;;
	} else {
		R_CSI_TG_CTRL0 |= C_CSI_CTRL1_VRST;
	}

	// Horizontal counter reset at falling(0) or rising(1) edge of HSYNC
	if (h_reset == ENUM_CSI_FALLING_EDGE) {
		R_CSI_TG_CTRL0 &= ~C_CSI_CTRL1_HRST;
	} else {
		R_CSI_TG_CTRL0 |= C_CSI_CTRL1_HRST;
	}
}

void drvl1_csi_compress_ratio_set(INT32U h_compress_ratio, INT32U v_compress_ratio)
{
	R_CSI_TG_HRATIO = h_compress_ratio & 0x7F7F;		// Horizontal compress ratio control
	R_CSI_TG_VRATIO = v_compress_ratio & 0x7F7F;		// Vertical compress ratio control
}

void drvl1_csi_output_format_set(CSI_OUTPUT_DATA_ENUM data_format)
{
	if (data_format < ENUM_CSI_OUT_VYUY) {
		// RGB
		R_CSI_TG_CTRL0 &= ~C_CSI_CTRL1_OUT_YUV;

		if (data_format == ENUM_CSI_OUT_RGB565) {
			R_CSI_TG_CTRL1 &= ~C_CSI_CTRL2_OUT_RGB1555_YUYV;
		} else {
			R_CSI_TG_CTRL1 |= C_CSI_CTRL2_OUT_RGB1555_YUYV;
		}
	} else if (data_format == ENUM_CSI_OUT_Y_ONLY) {
		R_CSI_TG_CTRL0 |= C_CSI_CTRL1_OUT_YUV;
		R_CSI_TG_CTRL1 |= C_CSI_CTRL2_OUT_Y_ONLY;
	} else {
		// YUV
		R_CSI_TG_CTRL0 |= C_CSI_CTRL1_OUT_YUV;

		if (data_format==ENUM_CSI_OUT_VYUY || data_format==ENUM_CSI_OUT_VYUY_INVERT_UV7) {
			R_CSI_TG_CTRL1 &= ~C_CSI_CTRL2_OUT_RGB1555_YUYV;
		} else {
			R_CSI_TG_CTRL1 |= C_CSI_CTRL2_OUT_RGB1555_YUYV;
		}

		if (data_format==ENUM_CSI_OUT_VYUY || data_format==ENUM_CSI_OUT_YUYV) {
			R_CSI_TG_CTRL1 &= ~C_CSI_CTRL2_OUT_UV_BIT7_INVERT;
		} else {
			R_CSI_TG_CTRL1 |= C_CSI_CTRL2_OUT_UV_BIT7_INVERT;
		}
	}
}

void drvl1_csi_output_fifo_set(CSI_OUTPUT_FIFO_ENUM fifo_mode)
{
	R_CSI_TG_CTRL0 &= ~C_CSI_CTRL1_FIFO_MASK;
	R_CSI_TG_CTRL0 |= fifo_mode;
}

void drvl1_csi_frame_buffer_init(void)
{
	for (buf_write=0; buf_write<C_CSI_DRV_MAX_FRAME_NUM; buf_write++) {
		free_queue_buffer[buf_write] = C_CSI_DRV_DUMMY_BUFFER;
	}
	buf_read = buf_write = 0;
}

void drvl1_csi_frame_buffer_put(INT32U frame_buf)
{
	INT8U new_write;
	
	if (frame_buf == C_CSI_DRV_DUMMY_BUFFER) {
		return;
	}
	
	new_write = buf_write+1;
	if (new_write == C_CSI_DRV_MAX_FRAME_NUM) {
		new_write = 0;
	}
	if (new_write != buf_read) {
		free_queue_buffer[buf_write] = frame_buf;
		buf_write = new_write;
	}
}

INT32U drvl1_csi_frame_buffer_get(void)
{
	INT32U frame;

	if (buf_write != buf_read) {
		frame = free_queue_buffer[buf_read];
		buf_read++;
		if (buf_read == C_CSI_DRV_MAX_FRAME_NUM) {
			buf_read = 0;
		}
	} else {
		return C_CSI_DRV_DUMMY_BUFFER;
	}

	if (!frame) {
		return C_CSI_DRV_DUMMY_BUFFER;
	}
	return frame;
}

// CSI start/stop output clock to sensor module when this function is called
void drvl1_csi_output_clock_set(CSI_CLOCK_FREQUENCY_ENUM freq)
{
	if (freq == ENUM_CSI_CLOCK_NONE) {
		R_CSI_TG_CTRL1 &= ~C_CSI_CTRL2_CLOCK_OUT_EN;
		
		return;
	}
	
	if (freq == ENUM_CSI_CLOCK_24MHZ) {
		R_CSI_TG_CTRL1 &= ~C_CSI_CTRL2_PLL_CLOCK_OUT;
	} else if (freq == ENUM_CSI_CLOCK_SYSTEM_DIV2) {
		R_CSI_TG_CTRL1 |= C_CSI_CTRL2_PLL_CLOCK_OUT;
		R_SYSTEM_CTRL &= ~0x00004000;		// C_SYSTEM_CTRL_CSI_CLOCK_DIV
	} else {
		R_CSI_TG_CTRL1 |= C_CSI_CTRL2_PLL_CLOCK_OUT;
		R_SYSTEM_CTRL |= 0x00004000;		// C_SYSTEM_CTRL_CSI_CLOCK_DIV: When set, CMOS sensor clock is divided by 2
	}

	R_CSI_TG_CTRL1 |= C_CSI_CTRL2_CLOCK_OUT_EN;
}

void drvl1_csi_start(void (*callback)(CSI_EVENT_ENUM event, INT32U buffer))
{
	csi_isr_callback = callback;

	csi_stop_flag = 0;
	csi_fifo_index = 0;
	csi_under_run_flag = 0;

	// Clear IRQ status
	R_TGR_IRQ_STATUS = R_TGR_IRQ_STATUS;

	// Set frame buffer start address(FIFO frame buffer A when FIFO mode is used)
	*P_CSI_TG_FBSADDR = drvl1_csi_frame_buffer_get();

	// Check whether FIFO mode is used
	if (R_CSI_TG_CTRL0 & C_CSI_CTRL1_FIFO_MASK) {
		// FIFO frame buffer B when FIFO mode is used
		*P_CSI_TG_FBSADDR_B = drvl1_csi_frame_buffer_get();

		csi_fifo_enable = 1;

		R_TGR_IRQ_EN = C_CSI_INT_FIFO_FULL_EN | C_CSI_INT_FRAME_END_EN;
	} else {
		*P_CSI_TG_FBSADDR_B = C_CSI_DRV_DUMMY_BUFFER;
		csi_fifo_enable = 0;

		R_TGR_IRQ_EN = C_CSI_INT_FRAME_END_EN;
	}

	R_CSI_TG_CTRL0 |= C_CSI_CTRL1_EN;
	R_CSI_TG_CTRL1 |= C_CSI_CTRL2_CLOCK_OUT_EN;
}

static INT32S csi_device_protect(void)
{
	return vic_irq_disable(VIC_CSI);
}

static void csi_device_unprotect(INT32S mask)
{
	if (mask == 0) {						// Clear device interrupt mask
		vic_irq_enable(VIC_CSI);
	} else {
		vic_irq_disable(VIC_CSI);
	}
}

static void csi_isr(void)
{
	INT32U flag, new_frame, complete_frame;

	flag = R_TGR_IRQ_STATUS;
	flag &= ~(1<<4);
	R_TGR_IRQ_STATUS = flag;		// Clear interrupt pending status

	if ((flag & C_CSI_INT_FLAG_FRAME_END) && (R_TGR_IRQ_EN & C_CSI_INT_FRAME_END_EN)) {
		if (csi_stop_flag) {
			R_TGR_IRQ_EN = 0x0;       	    		// Disable IRQ
			R_CSI_TG_CTRL1 &= ~C_CSI_CTRL2_CLOCK_OUT_EN;
			(*csi_isr_callback)(ENUM_CSI_EVENT_STOP, NULL);
			csi_isr_callback = NULL;
			
			return;
		}
		
		if (csi_isr_callback) {
			new_frame = drvl1_csi_frame_buffer_get();
		} else {
			new_frame = C_CSI_DRV_DUMMY_BUFFER;
		}

		if (csi_fifo_enable) {
			if (csi_fifo_index & 0x1) {
				complete_frame = *P_CSI_TG_FBSADDR_B;
				*P_CSI_TG_FBSADDR_B = new_frame;
			} else {
				complete_frame = *P_CSI_TG_FBSADDR;
				*P_CSI_TG_FBSADDR = new_frame;
			}
			csi_fifo_index ^= 0x1;
		} else {
			complete_frame = *P_CSI_TG_FBSADDR;
			*P_CSI_TG_FBSADDR = new_frame;
		}

		// Callback
		if (csi_isr_callback) {
			// Should not enable under run interrupt, but flag should be check when FIFO end or Frame end
			if ((complete_frame != C_CSI_DRV_DUMMY_BUFFER) && !(flag & C_CSI_INT_FLAG_UNDER_RUN) && !csi_under_run_flag) {
				(*csi_isr_callback)(ENUM_CSI_EVENT_FRAME_END, complete_frame);
			} else {
				// Send frame end without data buffer so that upper layer task will know that under-run has happened
				(*csi_isr_callback)(ENUM_CSI_EVENT_FRAME_END, NULL);

				if (complete_frame != C_CSI_DRV_DUMMY_BUFFER) {
					drvl1_csi_frame_buffer_put(complete_frame);
				}
			}
		} else {
			if (complete_frame != C_CSI_DRV_DUMMY_BUFFER) {
				drvl1_csi_frame_buffer_put(complete_frame);
			}
		}

		// Clear under run flag
		csi_under_run_flag = 0;

	} else if ((flag & C_CSI_INT_FLAG_FIFO_FULL) && (R_TGR_IRQ_EN & C_CSI_INT_FIFO_FULL_EN)) {
		if (csi_isr_callback) {
			new_frame = drvl1_csi_frame_buffer_get();
		} else {
			new_frame = C_CSI_DRV_DUMMY_BUFFER;
		}
		// Change FIFO buffer
		if (csi_fifo_index & 0x1) {
			complete_frame = *P_CSI_TG_FBSADDR_B;
			*P_CSI_TG_FBSADDR_B = new_frame;
		} else {
			complete_frame = *P_CSI_TG_FBSADDR;
			*P_CSI_TG_FBSADDR = new_frame;
		}
		csi_fifo_index ^= 0x1;;

		// Callback
		if (csi_isr_callback) {
			if (complete_frame != C_CSI_DRV_DUMMY_BUFFER) {
				// Under run interrupt should not be enable, but flag should be check when FIFO end or Frame end
				if (!(flag & C_CSI_INT_FLAG_UNDER_RUN)) {
					(*csi_isr_callback)(ENUM_CSI_EVENT_FIFO_FULL, complete_frame);
				} else {
					// Send data under run notification
					if (!csi_under_run_flag) {
						//(*csi_isr_callback)(ENUM_CSI_EVENT_UNDER_RUN, NULL);
						(*csi_isr_callback)(ENUM_CSI_EVENT_FIFO_FULL, NULL);
					}
					drvl1_csi_frame_buffer_put(complete_frame);
					csi_under_run_flag = 1;
				}
			} else {
				// Send buffer under run notification
				if (!csi_under_run_flag) {
					//(*csi_isr_callback)(ENUM_CSI_EVENT_DRV_BUFFER_UNDER_RUN, NULL);
					(*csi_isr_callback)(ENUM_CSI_EVENT_FIFO_FULL, NULL);
				}
				csi_under_run_flag = 1;
			}
		} else {
			if (complete_frame != C_CSI_DRV_DUMMY_BUFFER) {
				drvl1_csi_frame_buffer_put(complete_frame);
			}
		}
	}
}

void drvl1_csi_stop(void)
{
	INT32S mask;

	mask = csi_device_protect();
	
	csi_stop_flag = 1;

	R_CSI_TG_CTRL0 &= ~C_CSI_CTRL1_EN;				// Stop receiving sensor data
	*P_CSI_TG_FBSADDR = C_CSI_DRV_DUMMY_BUFFER;	// Frame buffer start address(FIFO frame buffer A when FIFO mode is used)
	*P_CSI_TG_FBSADDR_B = C_CSI_DRV_DUMMY_BUFFER;	// FIFO frame buffer B when FIFO mode is used
	
	// If we still have interrupt and output clock to sensor, we should stop clock during V-blank
	if (!(R_TGR_IRQ_EN & C_CSI_INT_FRAME_END_EN) || !(R_CSI_TG_CTRL1 & C_CSI_CTRL2_CLOCK_OUT_EN)) {
		R_TGR_IRQ_EN = 0x0;       	    		// Disable IRQ
		R_CSI_TG_CTRL1 &= ~C_CSI_CTRL2_CLOCK_OUT_EN;
		(*csi_isr_callback)(ENUM_CSI_EVENT_STOP, NULL);
		csi_isr_callback = NULL;
	}
  	
	drvl1_csi_frame_buffer_init();

	csi_device_unprotect(mask);
}

