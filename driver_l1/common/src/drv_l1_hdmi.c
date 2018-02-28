/* 
* Purpose: HDMI driver/interface
*
* Author: josephhsieh
*
* Date: 2013/11/21
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 1.00
* History :
*/

//Include files
#include "string.h"
#include "driver_l1.h"
#include "drv_l1_sfr.h"
#include "drv_l1_hdmi.h"

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#if (defined _DRV_L1_HDMI) && (_DRV_L1_HDMI == 1)                   //
//================================================================//

/////////////////// Display Setting ///////////////////
#define TFT_EN                  (1<<0)
#define TFT_CLK_SEL             (7<<1)
#define TFT_MODE                (0xF<<4)
#define TFT_VSYNC_INV           (1<<8)
#define TFT_HSYNC_INV           (1<<9)
#define TFT_DCLK_INV            (1<<10)
#define TFT_DE_INV              (1<<11)
#define TFT_H_COMPRESS          (1<<12)
#define TFT_MEM_BYTE_EN         (1<<13)
#define TFT_INTERLACE_MOD       (1<<14)
#define TFT_VSYNC_UNIT          (1<<15)

#define TFT_REG_POL             (1<<4)
#define TFT_REG_REV             (1<<5)
#define TFT_UD_I                (1<<6)
#define TFT_RL_I                (1<<7)
#define TFT_DITH_EN             (1<<8)
#define TFT_DITH_MODE           (1<<9)
#define TFT_DATA_MODE           (3<<10)
#define TFT_SWITCH_EN           (1<<12)
#define TFT_GAMMA_EN            (1<<13)
#define TFT_DCLK_SEL            (3<<14)
#define TFT_DCLK_DELAY          (7<<18)
#define TFT_SLIDE_EN            (1<<21)

#define TFT_MODE_UPS051         0x0
#define TFT_MODE_UPS052         0x10
#define TFT_MODE_CCIR656        0x20
#define TFT_MODE_PARALLEL       0x30
#define TFT_MODE_TCON           0x40

#define TFT_MODE_MEM_CMD_WR     0x80
#define TFT_MODE_MEM_CMD_RD     0x90
#define TFT_MODE_MEM_DATA_WR    0xa0
#define TFT_MODE_MEM_DATA_RD    0xb0
#define TFT_MODE_MEM_CONTI      0xc0
#define TFT_MODE_MEM_ONCE       0xd0

#define TFT_DCLK_SEL_0          0x0000
#define TFT_DCLK_SEL_90         0x4000
#define TFT_DCLK_SEL_180        0x8000
#define TFT_DCLK_SEL_270        0xC000

#define TFT_DCLK_DELAY_0        0x000000
#define TFT_DCLK_DELAY_1        0x040000
#define TFT_DCLK_DELAY_2        0x080000
#define TFT_DCLK_DELAY_3        0x0C0000
#define TFT_DCLK_DELAY_4        0x100000
#define TFT_DCLK_DELAY_5        0x140000
#define TFT_DCLK_DELAY_6        0x180000
#define TFT_DCLK_DELAY_7        0x1C0000

#define TFT_REG_POL_1LINE       0x0
#define TFT_REG_POL_2LINE       0x1
#define TFT_NEW_POL_EN          0x80

#define PWM_VSET                0x70
#define PWM_VSET_BIT            4

#define TFT_DATA_MODE_8         0x0
#define D1_MODE					2
#define TFT_1280x720_MODE		6

extern void print_string(CHAR *fmt, ...);

static void tft2_clk_set(INT32U clk)
{
	R_TFT2_CTRL &= ~TFT_CLK_SEL;
	R_TFT_TS_MISC &= ~0xC0;
	
	if (clk < TFT_CLK_DIVIDE_9) {
		R_TFT2_CTRL |= clk;
	}
	else {
		R_TFT2_CTRL |= clk & 0xF;
		R_TFT_TS_MISC |= (clk & 0x20) << 1;
		R_TFT_TS_MISC |= (clk & 0x10) << 3;
	}	
}	

static void tft2_data_mode_set(INT32U mode)
{
	R_TFT_TS_MISC &= ~TFT_DATA_MODE;
	R_TFT_TS_MISC |= mode;	
}

static void tft2_mode_set(INT32U mode)
{
	R_TFT2_CTRL &= ~TFT_MODE;
	R_TFT2_CTRL |= mode;		
}

static void tft2_signal_inv_set(INT32U mask, INT32U value)
{
	/*set vsync,hsync,dclk and DE inv */
	R_TFT2_CTRL &= ~mask;
	R_TFT2_CTRL |= (mask & value);		
}

/////////////////// HDMI Setting ///////////////////
#define HAL_HDMI_PKT_HVLD				0x10000000
#define HAL_HDMI_PKT_VVLD				0x20000000
#define HAL_HDMI_PKT_AUTO				0x40000000
#define HAL_HDMI_PKT_SEND				0x80000000

#define HDMI_ACR_PACKET					0x01
#define HDMI_AUDIOSAMPLE_PACKET			0x02
#define HDMI_GENERALCTRL_PACKET			0x03
#define HDMI_ACP_PACKET					0x04
#define HDMI_ISRC1_PACKET				0x05
#define HDMI_ISRC2_PACKET				0x06
#define HDMI_1BITAUDIOSAMPLE_PACKET		0x07
#define HDMI_DSTAUDIO_PACKET			0x08
#define HDMI_HBRAUDIOSTREAM_PACKET		0x09
#define HDMI_GAMUTMETADATA_PACKET		0x0a
#define HDMI_VENDORINFOFRAME_PACKET		0x81
#define HDMI_AVIINFOFRAME_PACKET		0x82
#define HDMI_SPDINFOFRAME_PACKET		0x83
#define HDMI_AUDIOINFOFRAME_PACKET		0x84
#define HDMI_MPEGINFOFRAME_PACKET		0x85

#define HDMI_VS0	0
#define HDMI_VS1	1
#define HDMI_VS2	2
#define HDMI_VS3	3

enum Sampling {
	Sampling_22K = 0x4000000,
	Sampling_44K = 0x0000000,
	Sampling_88K = 0x8000000,
	Sampling_176K = 0xC000000,
	Sampling_24K = 0x6000000,
	Sampling_48K = 0x2000000,
	Sampling_96K = 0xA000000,
	Sampling_192K = 0xE000000,
	Sampling_32K = 0x3000000,
	Sampling_none = 0x1000000,
	Sampling_768K = 0x9000000
};

enum Original_sampling {
	Original_sampling_no_match = 0x00,
	Original_sampling_16K = 0x80,
	Original_sampling_Reserved1 = 0x40,
	Original_sampling_32K = 0xC0,
	Original_sampling_12K = 0x20,
	Original_sampling_11K = 0xA0,
	Original_sampling_8K = 0x60,
	Original_sampling_Reserved2 = 0xE0,
	Original_sampling_192K = 0x10,
	Original_sampling_24K = 0x90,
	Original_sampling_96K = 0x50,
	Original_sampling_48K = 0xD0,
	Original_sampling_176K = 0x30,
	Original_sampling_22K = 0xB0,
	Original_sampling_88K = 0x70,
	Original_sampling_44K = 0xF0
};


typedef struct
{
	volatile unsigned int dispHDMIPacketHD;	    /* 0x0100 ~ 0x0103 */
	volatile unsigned int dispHDMIPacketBD[7];	/* 0x0104 ~ 0x0107 */
} HDMI_PKT;

typedef struct HDMI_Reg_s {
	volatile HDMI_PKT HdmiPkt[4];			/* 0x0100 ~ 0x017f */
	volatile unsigned int dispHDMIAudioN;	        /* 0x0180 ~ 0x0183 */
	volatile unsigned int dispHDMIAudioCTS;	    /* 0x0184 ~ 0x0187 */
	volatile unsigned int dispHDMIAudioSample;	/* 0x0188 ~ 0x018b */
	volatile unsigned int dispHDMIGCtrlPacket;	/* 0x018c ~ 0x018f */
	volatile unsigned int dispHDMITimeCycle;	    /* 0x0190 ~ 0x0193 */
	volatile unsigned int dispHDMIConfig;		    /* 0x0194 ~ 0x0197 */
} HDMI_Reg_t;

typedef struct AUD_CH_Reg_s {
	volatile unsigned int status0;
	volatile unsigned int status1;
	volatile unsigned int status2;
	volatile unsigned int status3;
} AUD_CH_Reg_t;

typedef struct
{
	unsigned char type;
	unsigned char version;	
	unsigned char length;
	unsigned char flag;	
	unsigned char CheckSum;
	unsigned char Body[27];
} HDMI_PACKET;

static void hdmi_packet_checksum(void *data)
{
	HDMI_PACKET *Pkt = (HDMI_PACKET *)(data);
	unsigned char *body = (unsigned char *)(Pkt->Body);
	int CheckSum = 0, i;
	
	CheckSum -= Pkt->type + Pkt->version + Pkt->length;
	for(i=0;i<Pkt->length;i++)
		CheckSum -= *body++;
	Pkt->CheckSum = (unsigned char)(CheckSum & 0xFF);
}


static void hdmi_avi_info_pkt(HDMI_PACKET *Pkt, unsigned char VID)
{
#if 0	// default 480P setting
	unsigned int *p = (unsigned int *)Pkt;

	p[0] = 0x600d0282;
	p[1] = 0x81253;	
	p[2] = 0x2;
	p[3] = 0x0;
	p[4] = 0x0;
	p[5] = 0x0;
	p[6] = 0x0;
	p[7] = 0x0;
#else
	memset((void*)Pkt, 0, sizeof(HDMI_PACKET));
	Pkt->type = HDMI_AVIINFOFRAME_PACKET;
	Pkt->version = 2;
	Pkt->length = 13;
	Pkt->Body[0] = 0x12;	//
	Pkt->Body[1] = 0x08;	// Active Portion Aspect Ratio
	Pkt->Body[2] = 0x00;
	Pkt->Body[3] = VID;	// VideoIdCode
	Pkt->Body[4] = 0x00;	// PixelRep
	Pkt->Body[5] = 0x00;
	Pkt->Body[6] = 0x00;
	Pkt->Body[7] = 0x00;
	Pkt->Body[8] = 0x00;
	Pkt->Body[9] = 0x00;
	Pkt->Body[10] = 0x00;
	Pkt->Body[11] = 0x00;
	Pkt->Body[12] = 0x00;	
#endif
}

static void hdmi_vendor_info_frame_pkt(HDMI_PACKET *Pkt)
{
#if 0	// default 480P setting
	unsigned int *p = (unsigned int *)Pkt;

	p[0] = 0x60040181;
	p[1] = 0xc036b;
	p[2] = 0x0;
	p[3] = 0x0;
	p[4] = 0x0;
	p[5] = 0x0;
	p[6] = 0x0;
	p[7] = 0x0;
#else
	memset((void*)Pkt, 0, sizeof(HDMI_PACKET));
	Pkt->type = HDMI_VENDORINFOFRAME_PACKET;
	Pkt->version = 1;
	Pkt->length = 4;
	Pkt->Body[0] = 0x03;
	Pkt->Body[1] = 0x0C;
	Pkt->Body[2] = 0x00;
	Pkt->Body[3] = 0x00;	
#endif
}

static void hdmi_spd_info_frame_pkt(HDMI_PACKET *Pkt)
{
#if 0	// default 480P setting
	unsigned int *p = (unsigned int *)Pkt;
	
	p[0] = 0x60190183;
	p[1] = 0x6c50475f;
	p[2] = 0x7375;
	p[3] = 0x4d504700;
	p[4] = 0x30333850;
	p[5] = 0x4d502030;
	p[6] = 0x50;
	p[7] = 0xd00;
#else
	memset((void*)Pkt, 0, sizeof(HDMI_PACKET));
	Pkt->type = HDMI_SPDINFOFRAME_PACKET;
	Pkt->version = 1;
	Pkt->length = 25;
	Pkt->Body[0] = 0x47;
	Pkt->Body[1] = 0x50;
	Pkt->Body[2] = 0x6C;
	Pkt->Body[3] = 0x75;
	Pkt->Body[4] = 0x73;
	Pkt->Body[5] = 0x00;
	Pkt->Body[6] = 0x00;
	Pkt->Body[7] = 0x00;
	Pkt->Body[8] = 0x47;
	Pkt->Body[9] = 0x50;
	Pkt->Body[10] = 0x4D;
	Pkt->Body[11] = 0x50;
	Pkt->Body[12] = 0x38;
	Pkt->Body[13] = 0x33;
	Pkt->Body[14] = 0x30;
	Pkt->Body[15] = 0x30;
	Pkt->Body[16] = 0x20;
	Pkt->Body[17] = 0x50;
	Pkt->Body[18] = 0x4D;
	Pkt->Body[19] = 0x50;
	Pkt->Body[20] = 0x00;
	Pkt->Body[21] = 0x00;
	Pkt->Body[22] = 0x00;
	Pkt->Body[23] = 0x00;
	Pkt->Body[24] = 0x0D;
#endif
}

static void hdmi_audio_info_frame_pkt(HDMI_PACKET *Pkt)
{
#if 0	// default 480P setting
		unsigned int *p = (unsigned int *)Pkt;
		
		p[0] = 0x600a0184;
		p[1] = 0x52;
		p[2] = 0x1f;
		p[3] = 0x0;
		p[4] = 0x0;
		p[5] = 0x0;
		p[6] = 0x0;
		p[7] = 0x0;
#else
		memset((void*)Pkt, 0, sizeof(HDMI_PACKET));
		Pkt->type = HDMI_AUDIOINFOFRAME_PACKET;
		Pkt->version = 1;
		Pkt->length = 10;
		Pkt->Body[0] = 0x00;
		Pkt->Body[1] = 0x00;
		Pkt->Body[2] = 0x00;
		Pkt->Body[3] = 0x1F;
		Pkt->Body[4] = 0x00;
		Pkt->Body[5] = 0x00;
		Pkt->Body[6] = 0x00;
		Pkt->Body[7] = 0x00;
		Pkt->Body[8] = 0x00;
		Pkt->Body[9] = 0x00;
#endif
}

static void hdmi_audio_channel_status(void *RegBase, unsigned int sampling_rate, unsigned int org_sampling_frequency)
{
	AUD_CH_Reg_t *AUD_ch = (AUD_CH_Reg_t*)RegBase;

	AUD_ch->status0 = 0x00120014 | sampling_rate;
	AUD_ch->status1 = 0x02 | org_sampling_frequency;
	AUD_ch->status2 = 0x00220014 | sampling_rate;
	AUD_ch->status3 = 0x02 | org_sampling_frequency;
}

//
// DAC 播放須要 timerE 驅動
// HDMI mode 時則 HDMI的REG 取代timerE 驅動
// ex:
//	dac_sample_rate_set(sample_rate);
//	drvl1_hdmi_audio_ctrl(1);
//    此時若為DAC輸出，則HDMI audio 打開無效。（因為HDMI audio 發聲的條件是要有 video）
//    此時若為 HDMI 輸出，則timerE被HDMI REG 取代，HDMI 播聲音，DAC 若 enable 也可聽到聲音
int drvl1_hdmi_audio_ctrl(unsigned int status)
{
	if (status)
		R_HDMI_AUD_CTRL |= 0x1;
	else
		R_HDMI_AUD_CTRL &= (~0x1);
	return 0;
}

int drvl1_hdmi_dac_mute(unsigned int status)
{

	if (status)
		R_HDMI_AUD_CTRL |= 0x40;		// DAC off
	else
		R_HDMI_AUD_CTRL &= (~0x40);		// DAC on
	return 0;
}

void drvl1_hdmi_send_packet(void *RegBase, unsigned int ch, void *data, unsigned int blank, unsigned int sendMode)
{
	HDMI_Reg_t *pHDMI_Reg = (HDMI_Reg_t*)RegBase;
	const unsigned long *tmp = (const unsigned long *)data;

	hdmi_packet_checksum(data);

	pHDMI_Reg->HdmiPkt[ch].dispHDMIPacketBD[0] = tmp[1];
	pHDMI_Reg->HdmiPkt[ch].dispHDMIPacketBD[1] = tmp[2];
	pHDMI_Reg->HdmiPkt[ch].dispHDMIPacketBD[2] = tmp[3];
	pHDMI_Reg->HdmiPkt[ch].dispHDMIPacketBD[3] = tmp[4];
	pHDMI_Reg->HdmiPkt[ch].dispHDMIPacketBD[4] = tmp[5];
	pHDMI_Reg->HdmiPkt[ch].dispHDMIPacketBD[5] = tmp[6];
	pHDMI_Reg->HdmiPkt[ch].dispHDMIPacketBD[6] = tmp[7];
	pHDMI_Reg->HdmiPkt[ch].dispHDMIPacketHD = (tmp[0] & 0xFFFFFF) | blank | sendMode;
}

void drvl1_hdmi_set_general_ctrl_packet(void *RegBase)
{
	// 雖然我們沒送 GCP，但是 bit10會影響 Display 的顯示方示，所以還是要設成 0x0400;
	HDMI_Reg_t *pHDMI_Reg = (HDMI_Reg_t*)RegBase;
	pHDMI_Reg->dispHDMIGCtrlPacket = 0x0400;
}

void drvl1_hdmi_set_acr_packet(void *RegBase, unsigned int N, unsigned int CTS)
{
	HDMI_Reg_t *pHDMI_Reg = (HDMI_Reg_t*)RegBase;
	int ACR_STEP = 128, ACR_EN = 1;

	// V-Blank, H-Blank 送 ACR Packet (可能會卡到其它的 Packet)
	pHDMI_Reg->dispHDMIAudioN = HAL_HDMI_PKT_AUTO | HAL_HDMI_PKT_VVLD | (ACR_STEP << 20) | N;
	pHDMI_Reg->dispHDMIAudioCTS = (ACR_EN << 31) | CTS;
}

void drvl1_hdmi_set_audio_sample_packet(void *RegBase, unsigned int ch)
{
	HDMI_Reg_t *pHDMI_Reg = (HDMI_Reg_t*)RegBase;

	pHDMI_Reg->dispHDMIAudioSample = 0x001;	// GPCV1248 只有2ch, 只有1個 subpacket，沒什麼好選的
}

void drvl1_hdmi_config_phy(unsigned int phy1, unsigned int phy2)
{	
	R_HDMITXPHYCONFIG1 = phy1;
	R_HDMITXPHYCONFIG2 = phy2;
}

void drvl1_hdmi_set_time_cycle(void *RegBase, unsigned int VBack, unsigned int HBlank )
{
	HDMI_Reg_t *pHDMI_Reg = (HDMI_Reg_t*)RegBase;
	
	pHDMI_Reg->dispHDMITimeCycle = (VBack << 12) | HBlank;
}

int drvl1_hdmi_init(unsigned int DISPLAY_MODE, unsigned int AUD_FREQ)
{
	HDMI_PACKET	Pkt;
	unsigned int vback=0, hblank=0;		// video
	unsigned int phy1=0, phy2=0, config=0;
	unsigned int hdmi_ctrl = 0;	
	INT32S ret = -1;

	// fixed pwr on HDMI hardware bug
	{	// reset HDMI module
		R_SYSTEM_HDMI_CTRL &= (~0x8000);	// must
		R_SYSTEM_CLK_EN0  |= 0x8;		// must

		R_SYSTEM_CKGEN_CTRL |= 0x4;
		R_SYSTEM_PLLEN |= 0x80;
		OSTimeDly(1);
		R_HDMITXPHYCONFIG1 &= ~(0x1);
		OSTimeDly(1);
		DBG_PRINT("HDMI RESET2\r\n");
	}


	R_TFT2_CTRL = 0;
	tft2_signal_inv_set(TFT_VSYNC_INV|TFT_HSYNC_INV, TFT_VSYNC_INV|TFT_HSYNC_INV);
	tft2_mode_set(TFT_MODE_PARALLEL);		//  HDMI must  set "PARALLEL" mode
	tft2_data_mode_set(TFT_DATA_MODE_8);
	tft2_clk_set(TFT_CLK_DIVIDE_1); /* FS=66 */
	hdmi_ctrl = 0;

	switch (DISPLAY_MODE)
	{
		case D1_MODE:
			// 720 X 480
			R_SYSTEM_CKGEN_CTRL |= 0x8;	// enable 27M Hz			
			// TFT2 Registers
			R_TFT2_HS_WIDTH 		= 3;				//	1		=HPW
			R_TFT2_H_PERIOD 		= 858-1;	//	1560	=HPW+HBP+HDE+HFP
			R_TFT2_VS_WIDTH 		= 0;				//	1		=VPW				(DCLK)
			R_TFT2_V_PERIOD 		= 524; 	//	262 	=VPW+VBP+VDE+VFP	(LINE)
			R_TFT2_LINE_RGB_ORDER	 = 0x00;
			// HDMI
			R_TFT2_V_START = 0x0009002D; 	// Vsync start of HDMI( < Vynsc end of HDMI)			
			R_TFT2_V_END = 0x000F020D-1;	// Vsync end of HDMI ( < P_TFT2_V_START_END[15:0])
			R_TFT2_H_START = 0x000F0089; 	// Hsync start of HDMI( < Hsync end of HDMI)
			R_TFT2_H_END = 0x004C0358; 		// Hsync end of HDMI ( < P_TFT2_H_START_END[15:0])

			hdmi_ctrl |= 0x8000;
			// HDMI hard marco
			hdmi_avi_info_pkt(&Pkt, 0x02);
			config |= 0x400;
			hblank = 0x8A;
			vback = 0x96d2;
			phy1 = 0x312f0200;
			phy2 = 0x00000030;
			ret = 0;
			break;
		case TFT_1280x720_MODE:
			// R_SYSTEM_CKGEN_CTRL &= (~0x8);	// disable 27M Hz 	// TV need 27M Hz
			// 1280 X 720
			// TFT2 Registers
			R_TFT2_HS_WIDTH = 3;
			R_TFT2_H_PERIOD	= 1650-1;
			R_TFT2_VS_WIDTH = 2;
			R_TFT2_V_PERIOD	= 750-1;
			R_TFT2_LINE_RGB_ORDER	 = 0x00;			
			// HDMI
			R_TFT2_V_START 	= 0x0005001E;//5<<16(HDMI_VSYNC_START) + 30;
			R_TFT2_V_END	= 0x000A02ED;//10<<16(HDMI_VSYNC_END) + 749;
			R_TFT2_H_START 	= 0x006D0171;//109<<16(HDMI_HSYNC_START) + 369;
			R_TFT2_H_END	= 0x00940670;//148<<16(HDMI_HSYNC_END) + 1648;

			hdmi_ctrl |= 0x0000;
			// HDMI hard marco
			hdmi_avi_info_pkt(&Pkt, 0x04);
			config |= 0x3400;
			hblank = 0x172;
			vback = 0x9600;
			phy1 = 0x162C0200;
			phy2 = 0x00000010;
			ret = 0;
			break;
		default:
			ret = -1;
	}

	////////////////////////  HDMI Registers////////////////////////////
	R_SYSTEM_CKGEN_CTRL |= 0x1C;		// for HDMI video and audio clock
	R_SYSTEM_PLLEN		|= 0x80;		// HDMI_CLKEN
	//(*((volatile INT32U *) 0xD05000F0)) |= 0x1; // TV_EN （不能設，會誤進 TV 中斷）
	//(*((volatile INT32U *) 0xC01B0000)) &= 0x01;	// dst_sel （在外面設，因為有 scaler 來的的 path）

	R_TFT2_CTRL |= 0x10000;		// TFT ON  (這時 HDMI 才會獲得 clk，HDMI的 Register才可寫入)
								// default:0x10331

	R_HDMICONFIG = config;								
	drvl1_hdmi_config_phy(phy1, phy2);					
	// hdmi avi info frame
	drvl1_hdmi_send_packet((void*)P_HDMI_PKT_BASE, HDMI_VS0, (void*)(&Pkt), HAL_HDMI_PKT_VVLD, HAL_HDMI_PKT_AUTO);
	drvl1_hdmi_set_time_cycle((void*)P_HDMI_PKT_BASE, vback, hblank);
	// hdmi vendor info frame
	hdmi_vendor_info_frame_pkt(&Pkt);
	drvl1_hdmi_send_packet((void*)P_HDMI_PKT_BASE, HDMI_VS1, (void*)(&Pkt), HAL_HDMI_PKT_VVLD, HAL_HDMI_PKT_AUTO);
	// hdmi spd info frame	
	hdmi_spd_info_frame_pkt(&Pkt);
	drvl1_hdmi_send_packet((void*)P_HDMI_PKT_BASE, HDMI_VS2, (void*)(&Pkt), HAL_HDMI_PKT_VVLD, HAL_HDMI_PKT_AUTO);
	// hdmi audio info frame
	hdmi_audio_info_frame_pkt(&Pkt);
	drvl1_hdmi_send_packet((void*)P_HDMI_PKT_BASE, HDMI_VS3, (void*)(&Pkt), HAL_HDMI_PKT_VVLD, HAL_HDMI_PKT_AUTO);
	// hdmi general ctrl frame
	drvl1_hdmi_set_general_ctrl_packet((void*)P_HDMI_PKT_BASE);
	// hdmi audio sample frame
	drvl1_hdmi_set_audio_sample_packet((void*)P_HDMI_PKT_BASE, 2);

	R_SYSTEM_HDMI_CTRL = hdmi_ctrl;
	switch(AUD_FREQ)
	{
		case 32000:
			// hdmi audio acr frame
			R_SYSTEM_CKGEN_CTRL &= (~0x80);		// using 73.728M Hz
			R_SYSTEM_HDMI_CTRL |= 2303;		   // HDMI_MCLK_DIV 	
			drvl1_hdmi_set_acr_packet((void*)P_HDMI_PKT_BASE, 4096, 74250);
			// "Channel Status Bit"  information
			hdmi_audio_channel_status((void *)P_HDMI_AUD_CH_INFO_BASE, Sampling_32K, Original_sampling_32K);
			break;
		case 44100:
			// hdmi audio acr frame
			R_SYSTEM_CKGEN_CTRL |= 0x80;		// using 67.7376M Hz
			R_SYSTEM_HDMI_CTRL |= 1535;		   // HDMI_MCLK_DIV 				
			drvl1_hdmi_set_acr_packet((void*)P_HDMI_PKT_BASE, 6272, 82500);			
			// "Channel Status Bit"  information
			hdmi_audio_channel_status((void *)P_HDMI_AUD_CH_INFO_BASE, Sampling_44K, Original_sampling_44K);
			break;
		case 48000:
		default:			
			// hdmi audio acr frame
			R_SYSTEM_CKGEN_CTRL &= (~0x80);		// using 73.728M Hz
			R_SYSTEM_HDMI_CTRL |= 1535;		   // HDMI_MCLK_DIV 	
			drvl1_hdmi_set_acr_packet((void*)P_HDMI_PKT_BASE, 6144, 74250);				
			// "Channel Status Bit"  information
			hdmi_audio_channel_status((void *)P_HDMI_AUD_CH_INFO_BASE, Sampling_48K, Original_sampling_48K);
 	}


	//////////////////////////////////////////////////////
	R_HDMI_EN = 0x90000000;		// bit[31:30] must be 2'b10
	R_HDMI_IRQ_STATUS = 0xFFFF;	// clear HDMI interrupt

	(*((volatile INT32U *) 0xD0D00328)) = 0x0;	// HDCP encrypt cipher 的 AN value ,負責 bit[31:0]
	(*((volatile INT32U *) 0xD0D0032c)) = 0x0;	// HDCP encrypt cipher 的 AN value ,負責 bit[63:32]
	(*((volatile INT32U *) 0xD0D00330)) = 0x0;	// HDCP encrypt cipher 的 KM value ,負責 bit[31:0]
	(*((volatile INT32U *) 0xD0D00334)) = 0x0;	// HDCP encrypt cipher 的 KM value ,負責 bit[55:32]
	
	R_TFT2_CTRL |= 0x1;		// TFT ON,  PPU 開始抓資料  

	return ret;
}

int drvl1_hdmi_exit(void)
{
	// disable audio
	R_HDMI_AUD_CTRL &= (~0x41);
	// disable video
	R_TFT2_CTRL &= (~0x1);

       gpio_IOE_switch_config_from_HDMI_to_GPIO();
	
	// disable PHY power
	R_SYSTEM_CLK_EN0 &= (~0x8); // turn off HDMI clock
  #if TV_DET_ENABLE
  #else
	R_SYSTEM_CKGEN_CTRL &= (~0xC);	// turn off 148M Hz
	R_SYSTEM_CKGEN_CTRL &= (~0x8);	// turn off 27M Hz
  #endif
	R_SYSTEM_PLLEN	&= (~0x80);	// HDMI_CLKEN

	return 0;
}

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#endif //(defined _DRV_L1_HDMI) && (_DRV_L1_HDMI == 1)                   //
//================================================================//

