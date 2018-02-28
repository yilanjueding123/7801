#ifndef __drv_l1_SCALER_H__
#define __drv_l1_SCALER_H__

#include "driver_l1.h"
#include "drv_l1_sfr.h"

// Control register
#define	C_SCALER_CTRL_IN_RGB565				0x00000001
#define	C_SCALER_CTRL_IN_YUYV				0x00000004
#define	C_SCALER_CTRL_IN_UYVY				0x00000005
#define	C_SCALER_CTRL_IN_MASK				0x0030000F

#define	C_SCALER_CTRL_OUT_RGB565			0x00000010
#define	C_SCALER_CTRL_OUT_YUYV				0x00000040
#define	C_SCALER_CTRL_OUT_UYVY				0x00000050
#define	C_SCALER_CTRL_OUT_MASK				0x000C00F0

#define	C_SCALER_CTRL_START					0x00000100
#define	C_SCALER_CTRL_BUSY					0x00000100
#define	C_SCALER_CTRL_RESET					0x00000200
#define	C_SCALER_CTRL_TYPE_YUV				0x00000400
#define	C_SCALER_CTRL_TYPE_YCBCR			0x00000000
#define	C_SCALER_CTRL_INT_ENABLE			0x00000800
#define C_SCALER_CTRL_IN_FIFO_DISABLE		0x00000000
#define C_SCALER_CTRL_IN_FIFO_16LINE		0x00001000
#define C_SCALER_CTRL_IN_FIFO_32LINE		0x00002000
#define C_SCALER_CTRL_IN_FIFO_64LINE		0x00003000
#define C_SCALER_CTRL_IN_FIFO_128LINE		0x00004000
#define C_SCALER_CTRL_IN_FIFO_256LINE		0x00005000
#define C_SCALER_CTRL_IN_FIFO_LINE_MASK		0x00007000
#define C_SCALER_CTRL_OUT_OF_BOUNDRY    	0x00008000
#define C_SCALER_CTRL_VISIBLE_RANGE	    	0x00010000
#define C_SCALER_CTRL_CONTINUOUS_MODE   	0x00020000
#define C_SCALER_CTRL_OUT_FIFO_DISABLE		0x00000000
#define C_SCALER_CTRL_OUT_FIFO_16LINE		0x00400000
#define C_SCALER_CTRL_OUT_FIFO_32LINE		0x00800000
#define C_SCALER_CTRL_OUT_FIFO_64LINE		0x00C00000
#define C_SCALER_CTRL_OUT_FIFO_LINE_MASK	0x00C00000
#define	C_SCALER_CTRL_OUT_FIFO_INT			0x01000000
#define	C_SCALER_CTRL_RGB1555_TRANSPARENT	0x02000000
// Out-of-boundry color register
#define C_SCALER_OUT_BOUNDRY_COLOR_MAX		0x00FFFFFF

// Output width and height registers
#define C_SCALER_OUT_WIDTH_MAX				0x00001FFF		// Maximum 8191 pixels
#define C_SCALER_OUT_HEIGHT_MAX				0x00001FFF		// Maximum 8191 pixels

// Scaler factor registers
#define C_SCALER_X_FACTOR_MAX				0x00FFFFFF
#define C_SCALER_Y_FACTOR_MAX				0x00FFFFFF

// Scaler input x/y start offset registers
#define C_SCALER_X_START_MAX				0x3FFFFFFF
#define C_SCALER_Y_START_MAX				0x3FFFFFFF

/* Scaler out x offset registers */
#define C_SCALER_OUT_X_OFFSET_MAX			0x00001FFF

// Input width and height registers
#define C_SCALER_IN_WIDTH_MAX				0x00001FFF		// Maximum 8191 pixels
#define C_SCALER_IN_HEIGHT_MAX				0x00001FFF		// Maximum 8191 pixels

// Input width and height registers
#define C_SCALER_IN_VISIBLE_WIDTH_MAX		0x00001FFF		// Maximum 8191 pixels
#define C_SCALER_IN_VISIBLE_HEIGHT_MAX		0x00001FFF		// Maximum 8191 pixels

// Interrupt flag register
#define	C_SCALER_INT_PEND					0x00000001
#define	C_SCALER_INT_DONE					0x00000002
#define	C_SCALER_INT_OUT_FULL				0x00000004

// Post effect control register
#define	C_SCALER_HISTOGRAM_EN				0x00000001
#define	C_SCALER_Y_GAMMA_EN					0x00000002
#define	C_SCALER_COLOR_MATRIX_EN			0x00000004
#define	C_SCALER_INTERNAL_LINE_BUFFER		0x00000000
#define	C_SCALER_HYBRID_LINE_BUFFER			0x00000010
#define	C_SCALER_EXTERNAL_LINE_BUFFER		0x00000020
#define	C_SCALER_LINE_BUFFER_MODE_MASK		0x00000030

#define C_SCALER_STATUS_INPUT_EMPTY			0x00000001
#define C_SCALER_STATUS_BUSY				0x00000002
#define C_SCALER_STATUS_DONE				0x00000004
#define C_SCALER_STATUS_STOP				0x00000008
#define C_SCALER_STATUS_TIMEOUT				0x00000010
#define C_SCALER_STATUS_INIT_ERR			0x00000020
#define C_SCALER_STATUS_OUTPUT_FULL			0x00000040

// Backward compatible to old scaler driver
#define C_SCALER_CTRL_FIFO_DISABLE			C_SCALER_CTRL_IN_FIFO_DISABLE
#define C_SCALER_CTRL_FIFO_16LINE			C_SCALER_CTRL_IN_FIFO_16LINE
#define C_SCALER_CTRL_FIFO_32LINE			C_SCALER_CTRL_IN_FIFO_32LINE
#define C_SCALER_CTRL_FIFO_64LINE			C_SCALER_CTRL_IN_FIFO_64LINE
#define C_SCALER_CTRL_FIFO_128LINE			C_SCALER_CTRL_IN_FIFO_128LINE
#define C_SCALER_CTRL_FIFO_256LINE			C_SCALER_CTRL_IN_FIFO_256LINE


typedef enum
{
    SCALER_0,                 
    SCALER_1,
    SCALER_MAX
} SCALER_NUM;

// scaler backen process
typedef struct {
	INT32U SCM_A11;  /*SCM: Scalar Color Matrix*/
    INT32U SCM_A12;
    INT32U SCM_A13;
    INT32U SCM_A21;
    INT32U SCM_A22;
    INT32U SCM_A23;
    INT32U SCM_A31;
    INT32U SCM_A32;
    INT32U SCM_A33;
} COLOR_MATRIX_CFG;

typedef struct 
{										// Offset
	volatile INT32U	CTRL;           	// 0x0000
	volatile INT32U	OUT_BOND_COLOR;		// 0x0004
	volatile INT32U	OUT_WIDTH;			// 0x0008
	volatile INT32U	OUT_HEIGHT;			// 0x000C
	volatile INT32U	X_FACTOR;			// 0x0010
	volatile INT32U	Y_FACTOR;			// 0x0014
	volatile INT32U	X_START;			// 0x0018
	volatile INT32U	Y_START;			// 0x001C
	volatile INT32U	IN_WIDTH;			// 0x0020
	volatile INT32U	IN_HEIGHT;			// 0x0024
	volatile INT32U	IN_Y_ADDR;			// 0x0028
	volatile INT32U	RESERVE1[2];		
	volatile INT32U	OUT_Y_ADDR;			// 0x0034
	volatile INT32U	RESERVE2[2];		
	volatile INT32U	CURRENT_LINE;		// 0x0040
	volatile INT32U	RESERVE3[9];		
	volatile INT32U	IN_VISIBLE_WIDTH;	// 0x0068
	volatile INT32U	IN_VISIBLE_HEIGHT ;	// 0x006C
	volatile INT32U	OUT_X_OFFSET;		// 0x0070	
	volatile INT32U	LINE_BUFFER_ADDR;	// 0x0074
	volatile INT32U	RESERVE4[1];			
	volatile INT32U	INT_FLAG;			// 0x007C
	volatile INT32U	POST;				// 0x0080
	volatile INT32U	MAS_EN;				// 0x0084
	volatile INT32U	AUTO_MODE;			// 0x0088
	volatile INT32U	RESERVE5[29];		
	volatile INT32U	STATUS_WIRTE1;		// 0x0100
	volatile INT32U	STATUS_READ1;		// 0x0104
	volatile INT32U	STATUS_READ2;		// 0x0108
	volatile INT32U	RESERVE6[1];		
	volatile INT32U	IN_Y_ADDR_B;		// 0x0110
}SCALER_SFR;

#define MAS_EN_READ 	1
#define MAS_EN_WRITE 	2

typedef struct 
{	
	INT8U mas_0;
	INT8U mas_1;
	INT8U mas_2;
	INT8U mas_3;
}SCALER_MAS;


/*
* Function Name :  scaler_init
*
* Syntax : void scaler_init(void);
*
* Purpose :  Initiate Scaler module
*
* Parameters : <IN> none
*              <OUT> none
* Return : none
*
* Note :
*
*/
extern void scaler_init(INT8U scaler_num);

/*
* Function Name :  scaler_image_pixels_set
*
* Syntax : INT32S scaler_image_pixels_set(INT32U input_x, INT32U input_y, INT32U output_x, INT32U output_y);
*
* Purpose :  Set input image pixel and output pixel(1~8000)
*
* Parameters : <IN> input_x:	intput image width
*										input_y:	intput image heigth
*										output_x:	output image width
*										output_y:	output image heigth
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note :
*
*/
extern INT32S scaler_image_pixels_set(INT8U scaler_num,INT32U input_x, INT32U input_y, INT32U output_x, INT32U output_y);		// 1~8000 pixels
/*
* Function Name :  scaler_input_pixels_set
*
* Syntax : INT32S scaler_input_pixels_set(INT32U input_x, INT32U input_y);
*
* Purpose :  Set scaler input resolution at the X&Y axis, 1~8000 pixels, including the padding pixels
*
* Parameters : <IN> input_x:	intput image width
*										input_y:	intput image heigth
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note :
*
*/
extern INT32S scaler_input_pixels_set(INT8U scaler_num,INT32U input_x, INT32U input_y);				// 1~8000 pixels, including the padding pixels
/*
* Function Name :  scaler_input_visible_pixels_set
*
* Syntax : INT32S scaler_input_visible_pixels_set(INT32U input_x, INT32U input_y);
*
* Purpose :  Set scaler real input resolution at the X&Y axis, 1~8000 pixels, not including the padding pixels
*
* Parameters : <IN> input_x:	intput image width
*										input_y:	intput image heigth
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note :
*
*/
extern INT32S scaler_input_visible_pixels_set(INT8U scaler_num,INT32U input_x, INT32U input_y);		// 1~8000 pixels, not including the padding pixels
/*
* Function Name :  scaler_input_A_addr_set
*
* Syntax : INT32S scaler_input_A_addr_set(INT32U y_addr, INT32U u_addr, INT32U v_addr);
*
* Purpose :  Set input image address. 
*
* Parameters : <IN> y_addr:	Start address of input Y data
*										u_addr:	Start address of input U data
*										v_addr:	Start address of input V data
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note : Must be 4-byte alignment
*
*/
extern INT32S scaler_input_A_addr_set(INT8U scaler_num,INT32U y_addr, INT32U u_addr, INT32U v_addr);	// Must be 4-byte alignment
/*
* Function Name :  scaler_input_B_addr_set
*
* Syntax : INT32S scaler_input_B_addr_set(INT32U y_addr, INT32U u_addr, INT32U v_addr);
*
* Purpose :  Set input image address. 
*
* Parameters : <IN> y_addr:	Start address of input Y data
*										u_addr:	Start address of input U data
*										v_addr:	Start address of input V data
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note : Must be 4-byte alignment
*
*/
extern INT32S scaler_input_B_addr_set(INT8U scaler_num,INT32U y_addr, INT32U u_addr, INT32U v_addr);	// Must be 4-byte alignment

/*
* Function Name :  scaler_input_format_set
*
* Syntax : INT32S scaler_input_format_set(INT32U format);
*
* Purpose :  Set input image format 
*
* Parameters : <IN> format:	
*													#define	C_SCALER_CTRL_IN_RGB565			0x00000001
*													#define	C_SCALER_CTRL_IN_YUYV			0x00000004
*													#define	C_SCALER_CTRL_IN_UYVY			0x00000005
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note : 
*
*/
extern INT32S scaler_input_format_set(INT8U scaler_num,INT32U format);								// C_SCALER_CTRL_IN_RGB565/C_SCALER_CTRL_IN_YUYV/C_SCALER_CTRL_IN_UYVY
/*
* Function Name :  scaler_input_offset_set
*
* Syntax : INT32S scaler_input_offset_set(INT32U offset_x, INT32U offset_y);
*
* Purpose :  Set the input start point on X&Y axis
*
* Parameters : <IN> offset_x:	Input X offset
*										offset_y:	Input Y offset
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note :
*
*/
extern INT32S scaler_input_offset_set(INT8U scaler_num,INT32U offset_x, INT32U offset_y);			// Set scaler start x and y position offset
/*
* Function Name :  scaler_output_offset_set
*
* Syntax : INT32S scaler_output_offset_set(INT32U offset_x, INT32U offset_y);
*
* Purpose :  Set the output start point on X&Y axis
*
* Parameters : <IN> offset_x:	Input X offset
*										offset_y:	Input Y offset
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note :
*
*/
extern INT32S scaler_output_offset_set(INT8U scaler_num,INT32U x_out_offset);
/*
* Function Name :  scaler_output_pixels_set
*
* Syntax : INT32S scaler_output_pixels_set(INT32U factor_x, INT32U factor_y, INT32U output_x, INT32U output_y);
*
* Purpose :  Set scaler output resolution and scale factor at the X&Y axis
*
* Parameters : <IN> factor_x:	X scale factor
*														0x10000: Original Size
*														0x08000: Zoom-in two times
*														0x20000: Zoom-out two times
*										factor_y:	Y scale factor
*														0x10000: Original Size
*														0x08000: Zoom-in two times
*														0x20000: Zoom-out two times	
*										output_x: Out buffer width
*										output_y:	Out buffer height
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note :
*
*/
extern INT32S scaler_output_pixels_set(INT8U scaler_num,INT32U factor_x, INT32U factor_y, INT32U output_x, INT32U output_y);		// factor_x:(input_x<<16)/output_x (1~0x00FFFFFF), factor_y:(input_y<<16)/output_y, output_x: must be multiple of 16 when output format is YUV422/YUV420/YUV444, multiple of 32 when output format is YUV411, multiple of 8 for others(Maximum 2040 for YUYV8X32 and YUYV8X64, Maximum 4088 for YUYV32X32 and YUYV32X64, Maximum 8000 pixels for others), output_y: 1~8000 pixels
/*
* Function Name :  scaler_output_addr_set
*
* Syntax : INT32S scaler_output_addr_set(INT32U y_addr, INT32U u_addr, INT32U v_addr);
*
* Purpose :  Set output image address. 
*
* Parameters : <IN> y_addr:	Start address of output Y data
*										u_addr:	Start address of output U data
*										v_addr:	Start address of output V data
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note : Must be 4-byte alignment
*
*/
extern INT32S scaler_output_addr_set(INT8U scaler_num,INT32U y_addr, INT32U u_addr, INT32U v_addr);	// Must be 4-byte alignment
/*
* Function Name :  scaler_output_format_set
*
* Syntax : INT32S scaler_output_format_set(INT32U format);
*
* Purpose :  Set output image format 
*
* Parameters : <IN> format:	
*													#define	C_SCALER_CTRL_OUT_RGB565		0x00000010
*													#define	C_SCALER_CTRL_OUT_YUYV			0x00000040
*													#define	C_SCALER_CTRL_OUT_UYVY			0x00000050
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note : 
*
*/
extern INT32S scaler_output_format_set(INT8U scaler_num,INT32U format);								// C_SCALER_CTRL_OUT_RGB565/C_SCALER_CTRL_OUT_YUYV/C_SCALER_CTRL_OUT_UYVY
/*
* Function Name :  scaler_fifo_line_set
*
* Syntax : INT32S scaler_fifo_line_set(INT32U mode);
*
* Purpose :  Set Scaler FIFO
*
* Parameters : <IN> mode:	
*												#define C_SCALER_CTRL_FIFO_DISABLE	0x00000000
*												#define C_SCALER_CTRL_FIFO_16LINE		0x00001000
*												#define C_SCALER_CTRL_FIFO_32LINE		0x00002000
*												#define C_SCALER_CTRL_FIFO_64LINE		0x00003000
*												#define C_SCALER_CTRL_FIFO_128LINE	0x00004000
*												#define C_SCALER_CTRL_FIFO_256LINE	0x00005000	
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note :
*
*/
extern INT32S scaler_fifo_line_set(INT8U scaler_num,INT32U mode);									// C_SCALER_CTRL_FIFO_DISABLE/C_SCALER_CTRL_FIFO_16LINE/C_SCALER_CTRL_FIFO_32LINE/C_SCALER_CTRL_FIFO_64LINE/C_SCALER_CTRL_FIFO_128LINE/C_SCALER_CTRL_FIFO_256LINE
/*
* Function Name :  scaler_input_fifo_line_set
*
* Syntax : INT32S scaler_input_fifo_line_set(INT32U mode);
*
* Purpose :  Set Scaler input FIFO
*
* Parameters : <IN> mode:	
*												#define C_SCALER_CTRL_IN_FIFO_DISABLE		0x00000000
*												#define C_SCALER_CTRL_IN_FIFO_16LINE		0x00001000
*												#define C_SCALER_CTRL_IN_FIFO_32LINE		0x00002000
*												#define C_SCALER_CTRL_IN_FIFO_64LINE		0x00003000
*												#define C_SCALER_CTRL_IN_FIFO_128LINE		0x00004000
*												#define C_SCALER_CTRL_IN_FIFO_256LINE		0x00005000
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note :
*
*/
extern INT32S scaler_input_fifo_line_set(INT8U scaler_num,INT32U mode);								// C_SCALER_CTRL_IN_FIFO_DISABLE/C_SCALER_CTRL_IN_FIFO_16LINE/C_SCALER_CTRL_IN_FIFO_32LINE/C_SCALER_CTRL_IN_FIFO_64LINE/C_SCALER_CTRL_IN_FIFO_128LINE/C_SCALER_CTRL_IN_FIFO_256LINE
/*
* Function Name :  scaler_output_fifo_line_set
*
* Syntax : INT32S scaler_output_fifo_line_set(INT32U mode);
*
* Purpose :  Set Scaler output FIFO
*
* Parameters : <IN> mode:	
*												#define C_SCALER_CTRL_OUT_FIFO_DISABLE	0x00000000
*												#define C_SCALER_CTRL_OUT_FIFO_16LINE		0x00400000
*												#define C_SCALER_CTRL_OUT_FIFO_32LINE		0x00800000
*												#define C_SCALER_CTRL_OUT_FIFO_64LINE		0x00C00000
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note :
*
*/
extern INT32S scaler_output_fifo_line_set(INT8U scaler_num,INT32U mode);								// C_SCALER_CTRL_OUT_FIFO_DISABLE/C_SCALER_CTRL_OUT_FIFO_16LINE/C_SCALER_CTRL_OUT_FIFO_32LINE/C_SCALER_CTRL_OUT_FIFO_64LINE
/*
* Function Name :  scaler_YUV_type_set
*
* Syntax : INT32S scaler_YUV_type_set(INT32U type);	
*
* Purpose :  Set YUV type when YUYV or UYVY format is selected
*
* Parameters : <IN> type:	
*												#define	C_SCALER_CTRL_TYPE_YUV			0x00000400
*												#define	C_SCALER_CTRL_TYPE_YCBCR		0x00000000
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note :
*
*/
extern INT32S scaler_YUV_type_set(INT8U scaler_num,INT32U type);										// C_SCALER_CTRL_TYPE_YUV/C_SCALER_CTRL_TYPE_YCBCR
/*
* Function Name :  scaler_out_of_boundary_mode_set
*
* Syntax : INT32S scaler_out_of_boundary_mode_set(INT32U mode);	
*
* Purpose :  Set scaler out of boundary mode
*
* Parameters : <IN> mode:	
*											0:	Use the boundary data of the input picture, 
*											1:	Use Use color defined in scaler_out_of_boundary_color_set()
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note :
*
*/
extern INT32S scaler_out_of_boundary_mode_set(INT8U scaler_num,INT32U mode);							// mode: 0=Use the boundary data of the input picture, 1=Use Use color defined in scaler_out_of_boundary_color_set()
/*
* Function Name :  scaler_out_of_boundary_color_set
*
* Syntax : INT32S scaler_out_of_boundary_color_set(INT32U ob_color);	
*
* Purpose :  Set scaler out of boundary color value
*
* Parameters : <IN> ob_color:	The format of ob_color is Y-Cb-Cr
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note :
*
*/
extern INT32S scaler_out_of_boundary_color_set(INT8U scaler_num,INT32U ob_color);					// The format of ob_color is Y-Cb-Cr
/*
* Function Name :  scaler_line_buffer_mode_set
*
* Syntax : INT32S scaler_line_buffer_mode_set(INT32U mode);	
*
* Purpose :  Set scaler line buffer mode
*
* Parameters : <IN> mode:	
*												#define	C_SCALER_INTERNAL_LINE_BUFFER		0x00000000
*												#define	C_SCALER_HYBRID_LINE_BUFFER			0x00000010
*												#define	C_SCALER_EXTERNAL_LINE_BUFFER		0x00000020
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note :
*
*/
extern INT32S scaler_line_buffer_mode_set(INT8U scaler_num,INT32U mode);								// mode: C_SCALER_INTERNAL_LINE_BUFFER/C_SCALER_HYBRID_LINE_BUFFER/C_SCALER_EXTERNAL_LINE_BUFFER
/*
* Function Name :  scaler_external_line_buffer_set
*
* Syntax : INT32S scaler_external_line_buffer_set(INT32U addr);	
*
* Purpose :  Set start addr when C_SCALER_EXTERNAL_LINE_BUFFER mode is selected
*
* Parameters : <IN> addr:	External line buffer start addr
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note : 	Must be 4-byte alignment
*
*/
extern INT32S scaler_external_line_buffer_set(INT8U scaler_num,INT32U addr);							// Must be 4-byte alignment

// Scaler special mode for Motion-JPEG
/*
* Function Name :  scaler_bypass_zoom_mode_enable
*
* Syntax : INT32S scaler_bypass_zoom_mode_enable(void);
*
* Purpose :  Enable scaler interrupt and clear flag
*
* Parameters : <IN> none
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note : 	
*
*/
extern INT32S scaler_bypass_zoom_mode_enable(INT8U scaler_num);

// Scaler start, restart and stop function APIs
/*
* Function Name :  scaler_start
*
* Syntax : INT32S scaler_start(void);
*
* Purpose :  Start to scale the image data
*
* Parameters : <IN> none
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note : 	
*
*/
extern INT32S scaler_start(INT8U scaler_num);
/*
* Function Name :  scaler_restart
*
* Syntax : INT32S scaler_restart(void);
*
* Purpose :  Restart to scaler the image data
*
* Parameters : <IN> none
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note : 	
*
*/
extern INT32S scaler_restart(INT8U scaler_num);
/*
* Function Name :  scaler_stop
*
* Syntax : void scaler_stop(void);
*
* Purpose :  stop to scaler the image data
*
* Parameters : <IN> none
*              <OUT> none
* Return : none
*
* Note : 	
*
*/
extern void scaler_stop(INT8U scaler_num);

// Scaler device protection APIs, used by Montion-JPEG in JPEG driver
/*
* Function Name :  scaler_device_protect
*
* Syntax : INT32S scaler_device_protect(void);
*
* Purpose :  Disable scaler interrupt
*
* Parameters : <IN> none
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note : 	
*
*/
extern INT32S scaler_device_protect(void);
/*
* Function Name :  scaler_device_unprotect
*
* Syntax : void scaler_device_unprotect(INT32S mask);
*
* Purpose :  Clear scaler interrupt mask
*
* Parameters : <IN> mask: 0: Unmask scaler IRQ 1:	Mask scaler IRQ
*              <OUT> none
* Return : none
*
* Note : 	
*
*/
extern void scaler_device_unprotect(INT32S mask);

// Scaler status polling APIs
/*
* Function Name :  scaler_wait_idle
*
* Syntax : INT32S scaler_wait_idle(void);
*
* Purpose :  Get the current status of scaler engine
*
* Parameters : <IN> none
*              <OUT> none
* Return : INT32S:	Current status of scaler engine
*
* Note : 	
*
*/
extern INT32S scaler_wait_idle(INT8U scaler_num);
/*
* Function Name :  scaler_status_polling
*
* Syntax : INT32S scaler_status_polling(void);
*
* Purpose :  Get scaler status
*
* Parameters : <IN> none
*              <OUT> none
* Return : INT32S:	
*									#define C_SCALER_STATUS_INPUT_EMPTY		0x00000001
*									#define C_SCALER_STATUS_BUSY					0x00000002
*									#define C_SCALER_STATUS_DONE					0x00000004
*									#define C_SCALER_STATUS_STOP					0x00000008		
*
* Note : 	
*
*/
extern INT32S scaler_status_polling(INT8U scaler_num);

// Gamma, color matrix and line buffer mode control
/*
* Function Name :  scaler_gamma_switch
*
* Syntax : INT32S scaler_gamma_switch(INT8U gamma_switch);
*
* Purpose :  Enable/Disable scaler Y gamma function
*
* Parameters : <IN> gamma_switch: 1: Enable 0: Disable
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note : 	
*
*/
extern INT32S scaler_gamma_switch(INT8U scaler_num,INT8U gamma_switch);
/*
* Function Name :  scaler_Y_gamma_config
*
* Syntax : void scaler_Y_gamma_config(INT8U gamma_table_id, INT8U gain_value);
*
* Purpose :  Set the gamma curve of scaler¡¯s output
*
* Parameters : <IN> gamma_table_id: Index of Y gamma table
										gain_value:	 Input gamma value
*              <OUT> none
* Return : none
*
* Note : 	
*
*/
extern void scaler_Y_gamma_config(INT8U scaler_num,INT8U gamma_table_id, INT8U gain_value);
/*
* Function Name :  scaler_color_matrix_switch
*
* Syntax : INT32S scaler_color_matrix_switch(INT8U color_matrix_switch);
*
* Purpose :  Enable/Disable scaler color matrix function
*
* Parameters : <IN> color_matrix_switch: 1: Enable 0: Disable
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note : 	
*
*/
extern INT32S scaler_color_matrix_switch(INT8U scaler_num,INT8U color_matrix_switch);
/*
* Function Name :  scaler_color_matrix_config
*
* Syntax : INT32S scaler_color_matrix_config(COLOR_MATRIX_CFG *color_matrix);
*
* Purpose :  Set scaler color matrix parameter. This is a 10-bit signed value
*
* Parameters : <IN> *color_matrix:
*																typedef struct {
*																		INT32U SCM_A11;  
*																    INT32U SCM_A12;
*																    INT32U SCM_A13;
*																    INT32U SCM_A21;
*																    INT32U SCM_A22;
*																    INT32U SCM_A23;
*																    INT32U SCM_A31;
*																    INT32U SCM_A32;
*																    INT32U SCM_A33;
*																} COLOR_MATRIX_CFG;
*              <OUT> none
* Return : INT32S:	0: Success
*									 -1: Fail
*
* Note : 	
*
*/
extern INT32S scaler_color_matrix_config(INT8U scaler_num,COLOR_MATRIX_CFG *color_matrix);

/*
* Function Name :  scaler_hw_init
*
* Syntax : void scaler_hw_init(void) 
*
* Purpose :  initialize scaler hardware
*
* Return : none
*
* Note : 	
*
*/
extern INT32S scaler_color_matrix_config(INT8U scaler_num,COLOR_MATRIX_CFG *color_matrix);

/*
* Function Name :  scaler_mas_set
*
* Syntax : INT32S scaler_mas_set(INT8U scaler_num,SCALER_MAS* pMas_en)
*
* Purpose : Set scaler  read/write channel
*
* Return : none
*
* Note : 	
*
*/

extern INT32S scaler_mas_set(INT8U scaler_num,SCALER_MAS* pMas_en);
extern INT32S scaler_idle_check(INT8U scaler_num);
extern INT32S scaler_isr_callback_set(void (*callback)(INT32U scaler0_event, INT32U scaler1_event));


#endif		// __drv_l1_SCALER_H__
