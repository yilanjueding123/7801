#ifndef __DRV_L1_CSI_H__
#define __DRV_L1_CSI_H__


#include "driver_l1.h"
#include "drv_l1_sfr.h"

// CSI(CMOS Sensor Interface)
typedef enum {
	ENUM_CSI_HREF = 0,
	ENUM_CSI_HSYNC_CCIR_601,
	ENUM_CSI_HSYNC_CCIR_656
} CSI_INPUT_INTERFACE_ENUM;

typedef enum {
	ENUM_CSI_NON_INTERLACE = 0,
	ENUM_CSI_INTERLACE,
	ENUM_CSI_INTERLACE_INVERT_FIELD
} CSI_INTERLACE_MODE_ENUM;

typedef enum {
	ENUM_CSI_IN_RGB888_BGRG = 0,
	ENUM_CSI_IN_RGB888_GBGR,
	ENUM_CSI_IN_RGB565_BGRG,
	ENUM_CSI_IN_RGB565_GBGR,
	ENUM_CSI_IN_UYVY,
	ENUM_CSI_IN_UYVY_INVERT_UV7,
	ENUM_CSI_IN_YUYV,
	ENUM_CSI_IN_YUYV_INVERT_UV7
} CSI_INPUT_DATA_ENUM;

typedef enum {
	ENUM_CSI_LATCH_DELAY_1_CLOCK = 0,
	ENUM_CSI_LATCH_DELAY_2_CLOCK,
	ENUM_CSI_LATCH_DELAY_3_CLOCK,
	ENUM_CSI_LATCH_DELAY_4_CLOCK
} CSI_LATCH_DELAY_ENUM;

typedef enum {
	ENUM_CSI_FALLING_EDGE = 0,
	ENUM_CSI_RISING_EDGE
} CSI_LATCH_EDGE_ENUM;

typedef enum {
	ENUM_CSI_OUT_RGB565 = 0,
	ENUM_CSI_OUT_RGB1555,
	ENUM_CSI_OUT_VYUY,
	ENUM_CSI_OUT_VYUY_INVERT_UV7,
	ENUM_CSI_OUT_YUYV,
	ENUM_CSI_OUT_YUYV_INVERT_UV7,
	ENUM_CSI_OUT_Y_ONLY
} CSI_OUTPUT_DATA_ENUM;

typedef enum {
	ENUM_CSI_FIFO_DISABLE = 0,
	ENUM_CSI_FIFO_8LINE = 0x00100000,
	ENUM_CSI_FIFO_16LINE = 0x00200000,
	ENUM_CSI_FIFO_32LINE = 0x00300000
} CSI_OUTPUT_FIFO_ENUM;

typedef enum {
	ENUM_CSI_CLOCK_NONE = 0,			// No clock is output to sensor
	ENUM_CSI_CLOCK_24MHZ,				// Fixed 24MHz clock output to sensor
	ENUM_CSI_CLOCK_SYSTEM_DIV2,			// If system clock=96MHz, clock output to sensor=48MHz
	ENUM_CSI_CLOCK_SYSTEM_DIV4			// If system clock=96MHz, clock output to sensor=24MHz
} CSI_CLOCK_FREQUENCY_ENUM;

typedef enum {
	ENUM_CSI_EVENT_FIFO_FULL = 0,
	ENUM_CSI_EVENT_FRAME_END,
	ENUM_CSI_EVENT_UNDER_RUN,				// CSI controller can not write data to SDRAM on time
	ENUM_CSI_EVENT_DRV_BUFFER_UNDER_RUN,	// CSI driver can not get free buffer for output
	ENUM_CSI_EVENT_MD_FRAME_END,
	ENUM_CSI_EVENT_POSITION_HIT,
	ENUM_CSI_EVENT_MD_UNDER_RUN,
	ENUM_CSI_EVENT_STOP						// CSI clock out and interrupt enable register are disabled during V-blank
} CSI_EVENT_ENUM;

// Control register 1
#define C_CSI_CTRL1_EN					0x00000001
#define C_CSI_CTRL1_CAP					0x00000002		// Preview mode(0) or Capture mode(1)
#define C_CSI_CTRL1_HREF_MODE			0x00000004		// HSYNC/VSYNC mode(0) or HREF mode(1)
#define C_CSI_CTRL1_RGB565				0x00000008		// RGB888(0) or RGB565(1). Valid only when input format is RGB
#define C_CSI_CTRL1_IN_CLOCK_INVERT		0x00000010
#define C_CSI_CTRL1_IN_YUV				0x00000020		// Input format: RGB(0) or YUV(1)
#define C_CSI_CTRL1_OUT_YUV				0x00000040		// Output format: RGB(0) or YUV(1)
#define C_CSI_CTRL1_BLUE_SCREEN_EN		0x00000080
#define C_CSI_CTRL1_CCIR656				0x00000100		// CCIR601(0) or CCIR656(1)
#define C_CSI_CTRL1_FIELD_LATCH_RISING	0x00000200		// Latch field at falling(0) or rising(1) edge of VSYNC
#define C_CSI_CTRL1_HRST				0x00000400		// Horizontal counter reset at falling(0) or rising(1) edge of HSYNC
#define C_CSI_CTRL1_VADD				0x00000800		// Vertical counter increase at falling(0) or rising(1) edge of HSYNC
#define C_CSI_CTRL1_VRST				0x00001000		// Vertical counter reset at falling(0) or rising(1) edge of VSYNC
#define C_CSI_CTRL1_IN_YUYV_GBGR		0x00002000		// Input data format: UYVY/BGRG(0) or YUYV/GBGR(1)
#define C_CSI_CTRL1_FIELD_INVERT		0x00004000		// Valid only in interlace mode
#define C_CSI_CTRL1_INTERLACE			0x00008000		// Non-interlace(0) or interlace(1)
#define C_CSI_CTRL1_DEDICATE_IRQ		0x00010000		// Share IRQ source with PPU(0) or dedicate IRQ
#define C_CSI_CTRL1_VGA_2_D1_EN			0x00020000		// Enable horizontal scale up from 640 to 720 pixels
#define C_CSI_CTRL1_FIFO_MASK			0x00300000

// Control register 2
#define C_CSI_CTRL2_LATCH_DELAY_MASK	0x00000007
#define C_CSI_CTRL2_OUT_RGB1555_YUYV	0x00000010		// Outpu data format: RGB565/VYUY(0) or RGB1555/YUYV(1)
#define C_CSI_CTRL2_OUT_UV_BIT7_INVERT	0x00000040		// Valid only when output format is YUV
#define C_CSI_CTRL2_CLOCK_OUT_EN		0x00000080
#define C_CSI_CTRL2_SCREEN_CUT_EN		0x00000100
#define C_CSI_CTRL2_IN_UV_BIT7_INVERT	0x00000200
#define C_CSI_CTRL2_OUT_Y_ONLY			0x00000400
#define C_CSI_CTRL2_PLL_CLOCK_OUT		0x00000800		// 24MHz(0) or system_clock/2(1)
#define C_CSI_CTRL2_OUT_CUBIC_EN		0x00001000		// Frame mode(0) or Cubic mode(1)
#define C_CSI_CTRL2_OUT_CUBIC_32X32		0x00002000		// 64x64(0) or 32x32(1)
#define C_CSI_CTRL2_NON_STOP			0x00004000		// Output clock is stopped(0) or not(1) when receive FIFO is full(0)
#define C_CSI_CTRL2_HIGH_PRIORITY		0x00008000		// Sensor has lower(0) or higher(1) priority than PPU

// Preview mode control register
#define C_CSI_MISC_PPU_DEPTH_MASK		0x00000007
#define C_CSI_MISC_BLENDING_EN			0x00000010
#define C_CSI_MISC_BLENDING_LEVEL_MASK	0x0000FC00

// Block motion detection register
#define C_CSI_MD_CTRL_EN				0x00000002
#define C_CSI_MD_CTRL_RATE_MASK			0x0000000C
#define C_CSI_MD_CTRL_RATE_SHIFT		2
#define C_CSI_MD_CTRL_VGA				0x00000010		// Frame size to detect: QVGA(0) or VGA(1)
#define C_CSI_MD_CTRL_BUF_TYPE_MASK		0x00000020		// Latched data type: RGB(0) or YUV(1)
#define C_CSI_MD_CTRL_BUF_TYPE_SHIFT	5
#define C_CSI_MD_CTRL_MODE_MASK			0x000000C0
#define C_CSI_MD_CTRL_MODE_SHIFT		6
#define C_CSI_MD_CTRL_BLOCK_SIZE_8X8	0x00000100		// Block size to detect: 16x16(0) or 8x8(1)
#define C_CSI_MD_CTRL_THRESHOLD_MASK	0x0000007F
#define C_CSI_MD_CTRL_THRESHOLD_SHIFT	9

// Single pixel motion detection register
#define C_CSI_MD_H_POSITION_MASK		0x000003FF
#define C_CSI_MD_V_POSITION_MASK		0x000001FF

// Interrupt enable register
#define C_CSI_INT_FRAME_END_EN			0x00000001
#define C_CSI_INT_MD_FRAME_END_EN		0x00000002
#define C_CSI_INT_POSITION_HIT_EN		0x00000004
#define C_CSI_INT_MD_UNDER_RUN_EN		0x00000008
#define C_CSI_INT_UNDER_RUN_EN			0x00000010
#define C_CSI_INT_FIFO_FULL_EN			0x00000020

// Interrupt status register
#define C_CSI_INT_FLAG_FRAME_END		0x00000001
#define C_CSI_INT_FLAG_MD_FRAME_END		0x00000002
#define C_CSI_INT_FLAG_POSITION_HIT		0x00000004
#define C_CSI_INT_FLAG_MD_UNDER_RUN		0x00000008
#define C_CSI_INT_FLAG_UNDER_RUN		0x00000010		// CSI controller can not write data to SDRAM on time
#define C_CSI_INT_FLAG_FIFO_FULL		0x00000020


extern void drvl1_csi_init(void);

extern void drvl1_csi_input_set(CSI_INPUT_INTERFACE_ENUM interface, CSI_INTERLACE_MODE_ENUM interlace, INT32U invert_clk, CSI_INPUT_DATA_ENUM data_format);
extern INT32S drvl1_csi_input_resolution_set(INT32U width, INT32U height);
extern void drvl1_csi_input_latch_timing1_set(INT32U h_start, INT32U field0_vstart, INT32U field1_vstart, CSI_LATCH_DELAY_ENUM delay);
extern void drvl1_csi_input_latch_timing2_set(CSI_LATCH_EDGE_ENUM field, CSI_LATCH_EDGE_ENUM v_reset, CSI_LATCH_EDGE_ENUM v_increase, CSI_LATCH_EDGE_ENUM h_reset);

extern void drvl1_csi_compress_ratio_set(INT32U h_compress_ratio, INT32U v_compress_ratio);

extern void drvl1_csi_output_format_set(CSI_OUTPUT_DATA_ENUM data_format);
extern void drvl1_csi_output_fifo_set(CSI_OUTPUT_FIFO_ENUM fifo_mode);
extern void drvl1_csi_frame_buffer_init(void);
extern void drvl1_csi_frame_buffer_put(INT32U frame_buf);
extern INT32U drvl1_csi_frame_buffer_get(void);

extern void drvl1_csi_output_clock_set(CSI_CLOCK_FREQUENCY_ENUM freq);		// CSI start/stop output clock to sensor module when this function is called
extern void drvl1_csi_start(void (*callback)(CSI_EVENT_ENUM event, INT32U buffer));
extern void drvl1_csi_stop(void);


#endif		// __DRV_L1_CSI_H__
