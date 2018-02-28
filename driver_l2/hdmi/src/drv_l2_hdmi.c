/*
* Purpose: Driver layer2 for controlling HDMI
*
* Author: Tristan
*
* Date: 2008/12/11
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 1.06
*/

#include "driver_l1.h"
#include "Customer.h"
#include "drv_l1_i2c.h"
#include "drv_l1_tft.h"
#include "drv_l2_hdmi.h"

//#if (defined _DRV_L2_HDMI) && (_DRV_L2_HDMI == 1)

#define  HDMI_640X480P60_BIT      0x00000001
#define  HDMI_720X480P60_BIT      0x00000002
#define  HDMI_1280X720P60_BIT     0x00000004

//HDMI EDID check ----------------------------------------------------------------------------
enum {
	HDMI_640X480P60 = 0,
	HDMI_720X480P60,
	HDMI_1280X720P60,
	HDMI_VIDEO_TIMING_NUM
};

typedef struct
{
	/*	Progressive: 0, Interlaced: 1	*/
	/*	Negative: 0, Positive: 1	*/
	short	PixelClock,	HFreq,	VFreq;
} HDMI_TIMMING_TAB;

static const HDMI_TIMMING_TAB HdmiTiming[HDMI_VIDEO_TIMING_NUM] = 
{
	//	PxlClk	HFreq	VFreq	HRes	VRes	Intrlac	HTotal	HBlank	HFPorch	HSWidth	HBPorch	HPol	VTotal	VBlank	VFPorch	VSWidth	VBPorch	VPol	Oversmp	ID	IsPC	PHY1		PHY2
	{	2475,	315,	60	}, 	// 640 x 480 x60
	{	2700,	315,	60	},	// 480  p60
	{	7425,	450,	60	},	// 720  p60	
};

typedef struct
{
    unsigned short PixelClock;                                     // in 10khz
    unsigned char HorizontalAddressableVideo;                      // in pixels
    unsigned char HorizontalBlanking;                              // in pixels
    unsigned char HorizontalExtraInfo;
    unsigned char VerticalAddressableVideo;
    unsigned char VerticalBlanking;
    unsigned char VerticalExtraInfo;
	
    unsigned char HorizontalFrontPorch;                            //  in pixels
    unsigned char HorizontalSyncPulseWidth;
    unsigned char VerticalSync;
    unsigned char HV_Info;

    unsigned char HorizontalAddressableVideoImageSize;              // in mm
    unsigned char VerticalAddressableVideoImageSize;                // in mm
    unsigned char HV_AddressableVideoImageSize;						// in mm
    unsigned char RightHorizontalBorderOrLeftHorizontalBorder;      // in pixels
    unsigned char TopVerticalBorderOrBottomVerticalBorder;          // in Lines
    unsigned char Info;
} VESA_E_EDID_TIMING_MODE;


typedef struct
{
    unsigned long Indicator;
    unsigned char DisplayRangeLimitsOffsets;
    unsigned char MinimumVerticalRate;
    unsigned char MaximumVerticalRate;
    unsigned char MinimumHorizontalRate;
    unsigned char MaximumHorizontalRate;
    unsigned char MaximumPixelClock;
    unsigned char VideoTimingSupportFlags;
    unsigned char VideoTimingData[7];
} VESA_E_EDID_DISPLAY_RANGE_LIMITS;

typedef struct
{
    int Size, Blanking, FrontPorch, PluseWidth;
} TIMING;

typedef struct
{
    int PixelClock;
    int hSize, hBlanking, hFrontPorch, hPluseWidth;
    int vSize, vBlanking, vFrontPorch, vPluseWidth;
} FRAME_TIMING;

typedef struct
{
    int maxPixelClock;          // MHz
    int hMaxRate, hMinRate;     // kHz
    int vMaxRate, vMinRate;     // Hz
} TIMING_LIMITS;

static int hdmi_check_supported(unsigned char *edid, unsigned long *flag_supported, unsigned int offset)
{
    int i;
    const unsigned char *ptr;
    TIMING_LIMITS Limits;
    int flag_timing_limits = 0;

    DBG_PRINT("EDID Version = %d.%d\r\n", edid[0x12], edid[0x13]);
    ptr = edid + offset;

    for(i=0;i<4;i++)
    {
    	
        if( ptr[0] != 0 ||
                ptr[1] != 0) // check if timing descriptor
        {
            const VESA_E_EDID_TIMING_MODE *VesaTiming = (const VESA_E_EDID_TIMING_MODE*)ptr;
            FRAME_TIMING Timing;
			unsigned char HorizontalBlanking_upper = VesaTiming->HorizontalExtraInfo & 0x0F;
			unsigned char HorizontalAddressableVideo_upper = (VesaTiming->HorizontalExtraInfo & 0xF0)>>4;
			unsigned char VerticalBlanking_upper = VesaTiming->VerticalExtraInfo & 0x0F;
			unsigned char VerticalAddressableVideo_upper = (VesaTiming->VerticalExtraInfo & 0xF0)>>4;
			unsigned char VerticalSyncPulseWidth = VesaTiming->VerticalSync & 0x0F;
			unsigned char VerticalFrontPorch = (VesaTiming->VerticalSync & 0xF0)>>4;
			unsigned char VerticalSyncPulseWidth_upper = VesaTiming->HV_Info & 0x3;
			unsigned char VerticalFrontPorch_upper = (VesaTiming->HV_Info & 0x0C)>>2;
			unsigned char HorizontalSyncPulseWidth_upper = (VesaTiming->HV_Info & 0x30)>>4;
			unsigned char HorizontalFrontPorch_upper = (VesaTiming->HV_Info & 0xC0)>>6;

            Timing.PixelClock   = VesaTiming->PixelClock;

            Timing.hSize        = VesaTiming->HorizontalAddressableVideo | (HorizontalAddressableVideo_upper << 8);
            Timing.hBlanking    = VesaTiming->HorizontalBlanking | (HorizontalBlanking_upper << 8);
            Timing.hFrontPorch  = VesaTiming->HorizontalFrontPorch | (HorizontalFrontPorch_upper << 8);
            Timing.hPluseWidth  = VesaTiming->HorizontalSyncPulseWidth | (HorizontalSyncPulseWidth_upper << 8);

            Timing.vSize        = VesaTiming->VerticalAddressableVideo | (VerticalAddressableVideo_upper << 8);
            Timing.vBlanking    = VesaTiming->VerticalBlanking | (VerticalBlanking_upper << 8);
            Timing.vFrontPorch  = VerticalFrontPorch | (VerticalFrontPorch_upper << 4);
            Timing.vPluseWidth  = VerticalSyncPulseWidth | (VerticalSyncPulseWidth_upper << 4);

            if(i==0) DBG_PRINT("========= Preffered Timing =========\n");
            else     DBG_PRINT("============ Timing #%d =============\n", i);
            DBG_PRINT("Pixel clock = 0x%x\n",Timing.PixelClock);
            DBG_PRINT("           Addressable Blanking FrontPorch PulseWidth \n");
            DBG_PRINT("Horizontal    %4d        %4d     %4d      %4d    (pixels)\n",Timing.hSize,  Timing.hBlanking,   Timing.hFrontPorch, Timing.hPluseWidth);
            DBG_PRINT("Vertical      %4d        %4d     %4d      %4d    (lines)\n",Timing.vSize,  Timing.vBlanking,   Timing.vFrontPorch,     Timing.vPluseWidth);
        }
        else if(ptr[0] == 0 && ptr[1] == 0 && ptr[2] == 0 && ptr[3] == 0xFD) // Display Range Limits
        {
            const VESA_E_EDID_DISPLAY_RANGE_LIMITS *VesaLimits = (const VESA_E_EDID_DISPLAY_RANGE_LIMITS*)ptr;
            int hMaxOffset = 0, hMinOffset = 0;
            int vMaxOffset = 0, vMinOffset = 0;
            if(flag_timing_limits == 0)
            {
                if(VesaLimits->DisplayRangeLimitsOffsets & 1) vMinOffset = 255;
                if(VesaLimits->DisplayRangeLimitsOffsets & 2) vMaxOffset = 255;
                if(VesaLimits->DisplayRangeLimitsOffsets & 4) hMinOffset = 255;
                if(VesaLimits->DisplayRangeLimitsOffsets & 8) hMaxOffset = 255;


                Limits.maxPixelClock    = VesaLimits->MaximumPixelClock * 10;
                Limits.hMaxRate         = VesaLimits->MaximumHorizontalRate + hMaxOffset;
                Limits.hMinRate         = VesaLimits->MinimumHorizontalRate + hMinOffset;
                Limits.vMaxRate         = VesaLimits->MaximumVerticalRate + vMaxOffset;
                Limits.vMinRate         = VesaLimits->MinimumVerticalRate + vMinOffset;

                DBG_PRINT("========= Timing Limits =========\n");
                DBG_PRINT("Vertical Rate       = %3d ~ %3d Hz\n", Limits.vMinRate, Limits.vMaxRate);
                DBG_PRINT("Horizontal Rate     = %3d ~ %3d kHz\n", Limits.hMinRate, Limits.hMaxRate);
                DBG_PRINT("Maximum Pixel Clock = %3d MHz\n", Limits.maxPixelClock);
                flag_timing_limits = 1;
            }
        }
        ptr += 18;
    }

    if(flag_timing_limits)
    {
        int val = 0;
        if(flag_supported)
        {
            for(i=0;i<sizeof(HdmiTiming)/sizeof(HdmiTiming[0]);i++)
            {
                if( HdmiTiming[i].PixelClock <= Limits.maxPixelClock * 100 &&
                        HdmiTiming[i].HFreq <= Limits.hMaxRate * 10 &&
                        HdmiTiming[i].HFreq >= Limits.hMinRate * 10 &&
                        HdmiTiming[i].VFreq <= Limits.vMaxRate &&
                        HdmiTiming[i].VFreq >= Limits.vMinRate)
                    val |= 1 << i;
            }
            *flag_supported = val;
        }

        return 0;
    }
    else
    {
        return -1;
    }
}

#define  HDMI_SEGMENT_ID	0x60
#define  HDMI_DDC_ID_0		0xA0
#define  HDMI_DDC_ID_1		0xA1

static i2c_bus_handle_t hHdmiDDC;
static i2c_bus_handle_t hHdmiDDCSegment;
static int hdmi_read_EDID(unsigned char *p)
{
	int ret = 0;
	hHdmiDDCSegment.slaveAddr = HDMI_SEGMENT_ID;
	hHdmiDDCSegment.clkRate = 80;
	hHdmiDDC.slaveAddr = HDMI_DDC_ID_0;
	hHdmiDDC.clkRate = 80;

	i2c_init();

	//ret = reg_1byte_data_1byte_write(&hHdmiDDCSegment, 0, 0);
	ret = reg_1byte_data_1byte_write(&hHdmiDDC, 0, 0);
	if (ret < 0) {
		DBG_PRINT("DDC Error!!\r\n");
	}
	hHdmiDDC.slaveAddr = HDMI_DDC_ID_1;
	reg_1byte_data_1byte_write(&hHdmiDDC, 0, 0);
	
	return i2c_bus_read(&hHdmiDDC, p, 128);
}

int drvl2_hdmi_init(unsigned int DISPLAY_MODE, unsigned int AUD_FREQ)
{
	int ret = 0;
	unsigned long flag = 0;
	unsigned char EDID[128];


	ret = hdmi_read_EDID(EDID);
	if (ret >= 0)
	{	// Read DDC data 
		ret = hdmi_check_supported(EDID, &flag, 0x36);
		if (flag == 0) // try again（不知道是電視少送，還是硬體少讀一byte）
			ret = hdmi_check_supported(EDID, &flag, 0x35);
		if ( (DISPLAY_MODE==D1_MODE)&&(flag&HDMI_720X480P60_BIT) ) {
			DBG_PRINT("HDMI_720X480P60_BIT\r\n");
		}
		if ( (DISPLAY_MODE==TFT_1280x720_MODE)&&(flag&HDMI_1280X720P60_BIT) ) {
			DBG_PRINT("HDMI_1280X720P60\r\n");
		}
		if (flag == 0) {
			DBG_PRINT("DDC doesn't contain Monitor Range Limits Information\r\n");			
		}
	}
	else
	{
		DBG_PRINT("DDC read error.\r\n");
		return -1;
	}

	return drvl1_hdmi_init(DISPLAY_MODE, AUD_FREQ);
}

//#endif	// _DRV_L2_HDMI

