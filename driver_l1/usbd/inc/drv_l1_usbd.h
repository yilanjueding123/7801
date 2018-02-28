/******************************************************
* Drv_l1_usbd.h
*
* Purpose: usb controller L1 driver
*
* Author: Eugene Hsu
*
* Date: 2012/10/09
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 
* History :
*
*******************************************************/
#ifndef DRV_L1_USBD_H
#define DRV_L1_USBD_H
#include "udc_s220.h" 
#include "project.h"
#include "application.h"

#define  USBD_POLLING_METHOD    1
#define  OLD_BULK_CONTROL_FLOW	0

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef	void (*USBDISRHANDLER)(void);


#define		BIT0		0x00000001
#define		BIT1		0x00000002
#define		BIT2		0x00000004
#define		BIT3		0x00000008
#define		BIT4		0x00000010
#define		BIT5		0x00000020
#define		BIT6		0x00000040
#define		BIT7		0x00000080
#define		BIT8		0x00000100
#define		BIT9		0x00000200
#define		BIT10		0x00000400
#define		BIT11		0x00000800
#define		BIT12		0x00001000
#define		BIT13		0x00002000
#define		BIT14		0x00004000
#define		BIT15		0x00008000
#define		BIT16		0x00010000
#define		BIT17		0x00020000
#define		BIT18		0x00040000
#define		BIT19		0x00080000
#define		BIT20		0x00100000
#define		BIT21		0x00200000
#define		BIT22		0x00400000
#define		BIT23		0x00800000
#define		BIT24		0x01000000
#define		BIT25		0x02000000
#define		BIT26		0x04000000
#define		BIT27		0x08000000
#define		BIT28		0x10000000
#define		BIT29		0x20000000
#define		BIT30		0x40000000
#define		BIT31		0x80000000

/********************* Define rUDC_EP12DMA bit mask (Offset + 0x000, USB device EP12 DMA control status) *****************/
#define MASK_USBD_EP12_DMA_EN				BIT31
#define MASK_USBD_EP12_DMA_FLUSH			BIT30
#define MASK_USBD_EP12_DMA_FIFO_FLUSH		BIT29
#define MASK_USBD_EP12_DMA_WRITE			BIT28
#define MASK_USBD_EP12_DMA_MODIFY_EN		BIT27
#define MASK_USBD_EP12_DMA_COUNT_ALIGN		BIT26

/********************* Define rUDC_EP7DMA bit mask (Offset + 0x008, USB device EP7 DMA control) *****************/
#define MASK_USBD_EP7_DMA_EN				BIT31
#define MASK_USBD_EP7_DMA_FLUSH				BIT30


/********************* Define rUDCCS_UDC bit mask (Offset + 0x080, USB device controller status) *****************/
#define MASK_USBD_UDC_CS_SYS_DISCONNECT		BIT31
#define MASK_USBD_UDC_CS_USBPHY_CLK_EN		BIT30
#define MASK_USBD_UDC_CS_SYS_PARTIALM		BIT29
#define MASK_USBD_UDC_CS_SYS_SUSPENDM		BIT28
#define MASK_USBD_UDC_CS_FORCE_DISCONNECT	BIT27
#define MASK_USBD_UDC_CS_FORCE_CONNECT		BIT26
#define MASK_USBD_UDC_CS_SOFTRST			BIT18	/* Software reset */
#define MASK_USBD_UDC_CS_VBUS				BIT17	/* VBUS STATUS */
#define MASK_USBD_UDC_CS_VBUS_D				BIT16	/* VBUS STATUS AFTER DEBOUNSE */

/********************* Define rUDC_IRQ_ENABLE bit mask (Offset + 0x084, USB device controller interrupt enable) **/
#define MASK_USBD_UDC_IE_EPAB_DMA			BIT30	/* EPAB DMA finish interrupt enable */
#define MASK_USBD_UDC_IE_EP89_DMA			BIT29	/* EP89 DMA finish interrupt enable */
#define MASK_USBD_UDC_IE_AUDIO_DMA			BIT28	/* AUDIO DMA finish interrupt enable */
#define MASK_USBD_UDC_IE_FORCE_DISC			BIT27	/* Force disconnect finish interrupt enable */
#define MASK_USBD_UDC_IE_FORCE_CONN			BIT26	/* Force connect finish interrupt enable */
#define MASK_USBD_UDC_IE_DMA				BIT25	/* DMA interrupt enable */
#define MASK_USBD_UDC_IE_VBUS				BIT24	/* VBUS interrupt enable */
#define MASK_USBD_UDC_IE_RESETN				BIT19	/* UDC USB_RESET END interrupt enable */
#define MASK_USBD_UDC_IE_SCONF				BIT18	/* UDC HOST set configuration interrupt enable */
#define MASK_USBD_UDC_IE_RESUME				BIT17	/* UDC BUS RESUME interrupt enable */
#define MASK_USBD_UDC_IE_SUSPEND			BIT16	/* UDC BUS SUSPEND interrupt enable */
#define MASK_USBD_UDC_IE_EP1INN				BIT15	/* UDC DMA & EP1_IN END interrupt enable */
#define MASK_USBD_UDC_IE_EP3I				BIT14	/* UDC EP3_IN transaction interrupt enable */
#define MASK_USBD_UDC_IE_PIPO				BIT13	/* UDC PING-PONG_FIFO_SWAP interrupt enable */
#define MASK_USBD_UDC_IE_HCS				BIT12	/* UDC HOST clear stall interrupt enable */
#define MASK_USBD_UDC_IE_EP2N				BIT11	/* UDC EP2_NAK interrupt enable */
#define MASK_USBD_UDC_IE_EP1N				BIT10	/* UDC EP1_NAK interrupt enable */
#define MASK_USBD_UDC_IE_EP0N				BIT9	/* UDC EP0_NAK interrupt enable */
#define MASK_USBD_UDC_IE_HSS				BIT8	/* UDC HOST_SET_STALL interrupt enable */
#define MASK_USBD_UDC_IE_EP2O				BIT7	/* UDC EP2_OUT transaction interrupt enable */
#define MASK_USBD_UDC_IE_EP1I				BIT6	/* UDC EP1_IN transaction interrupt enable */
#define MASK_USBD_UDC_IE_EP1SI				BIT5	/* UDC EP1S_IN transaction interrupt enable */
#define MASK_USBD_UDC_IE_EP0I				BIT4	/* UDC EP0_OUT transaction interrupt enable */
#define MASK_USBD_UDC_IE_EP0O				BIT3	/* UDC EP0_IN transaction interrupt enable */
#define MASK_USBD_UDC_IE_EP0S				BIT2	/* UDC EP0_SETUP transaction interrupt enable */
#define MASK_USBD_UDC_IE_SUSP				BIT1	/* UDC USB_SUSPEND_DIFF interrupt enable */
#define MASK_USBD_UDC_IE_RESET				BIT0	/* UDC USB_RESET interrupt enable */

/********************* Define rUDC_IRQ_FLAG bit mask (Offset + 0x088, USB device controller interrupt flag) ******/
#define MASK_USBD_UDC_IF_EPAB_DMA			BIT30	/* EPAB DMA finish interrupt flag */
#define MASK_USBD_UDC_IF_EP89_DMA			BIT29	/* EP89 DMA finish interrupt flag */
#define MASK_USBD_UDC_IF_AUDIO_DMA			BIT28	/* AUDIO DMA finish interrupt flag */
#define MASK_USBD_UDC_IF_FORCE_DISC			BIT27	/* Force Disconnect finish flag, write 1 clear */
#define MASK_USBD_UDC_IF_FORCE_CONN			BIT26	/* Force connect finish flag, write 1 clear */
#define MASK_USBD_UDC_IF_DMA				BIT25	/* DMA interrupt flag, write 1 clear */
#define MASK_USBD_UDC_IF_VBUS				BIT24	/* USB VBUS toggle interrupt flag, write 1 clear */
#define MASK_USBD_UDC_IF_RESETN				BIT19	/* UDC USB_RESET END interrupt flag */
#define MASK_USBD_UDC_IF_SCONF				BIT18	/* UDC HOST Set Configuration interrupt flag */
#define MASK_USBD_UDC_IF_RESUME				BIT17	/* UDC USB BUS RESUME interrupt flag */
#define MASK_USBD_UDC_IF_SUSPEND			BIT16	/* UDC USB BUS SUSPEND interrupt flag */
#define MASK_USBD_UDC_IF_EP1INN				BIT15	/* UDC DMA & EP1_IN END interrupt flag */
#define MASK_USBD_UDC_IF_EP3I				BIT14	/* UDC EP3_IN transaction interrupt flag */
#define MASK_USBD_UDC_IF_PIPO				BIT13	/* UDC PING-PONG_FIFO_SWAP interrupt flag */
#define MASK_USBD_UDC_IF_HCS				BIT12	/* UDC HOST_CLEAR_STALL interrupt flag */
#define MASK_USBD_UDC_IF_EP2N				BIT11	/* UDC EP2_NAK interrupt flag */
#define MASK_USBD_UDC_IF_EP1N				BIT10	/* UDC EP1_NAK interrupt flag */
#define MASK_USBD_UDC_IF_EP0N				BIT9	/* UDC EP0_NAK interrupt flag */
#define MASK_USBD_UDC_IF_HSS				BIT8	/* UDC HOST_SET_STALL interrupt flag */
#define MASK_USBD_UDC_IF_EP2O				BIT7	/* UDC EP2_OUT transaction interrupt flag */
#define MASK_USBD_UDC_IF_EP1I				BIT6	/* UDC EP1_IN transaction interrupt flag */
#define MASK_USBD_UDC_IF_EP1SI				BIT5	/* UDC EP1S_IN transaction interrupt flag */
#define MASK_USBD_UDC_IF_EP0I				BIT4	/* UDC EP0_OUT transaction interrupt flag */
#define MASK_USBD_UDC_IF_EP0O				BIT3	/* UDC EP0_IN transaction interrupt flag */
#define MASK_USBD_UDC_IF_EP0S				BIT2	/* UDC EP0_SETUP transaction interrupt flag */
#define MASK_USBD_UDC_IF_SUSP				BIT1	/* UDC USB_SUSPEND_DIFF interrupt flag */
#define MASK_USBD_UDC_IF_RESET				BIT0	/* UDC USB_RESET interrupt flag */

/********************* Define rEP5CTL bit mask (Offset + 0x140, USB device EP5 control) *****************/
#define MASK_USBD_EP5_FLUSH					BIT1
#define MASK_USBD_EP5_CTL_EN				BIT0

/********************* Define rEP5FRAMCTL bit mask (Offset + 0x148, USB device EP5 control) *****************/
#define MASK_USBD_EP5_CTL_EMPTY				BIT7

/********************* Define rEP5HDCTRL bit mask (Offset + 0x14C, USB device EP5 video class header data control) *****************/
#define MASK_USBD_EP5_HDCTL_ERROR_FLAG		BIT5
#define MASK_USBD_EP5_HDCTL_ERROR			BIT4
#define MASK_USBD_EP5_HDCTL_FRAMEND			BIT3
#define MASK_USBD_EP5_HDCTL_FRAMESTILL		BIT2
#define MASK_USBD_EP5_HDCTL_FRAMESRC		BIT1
#define MASK_USBD_EP5_HDCTL_FRAMEPT			BIT0

/********************* Define rEP5EN bit mask (Offset + 0x150, USB device EP5 enable control) *****************/
#define MASK_USBD_EP5_EN					BIT0

/********************* Define rEP5DMAEN bit mask (Offset + 0x160, USB device EP5 DMA control) *****************/
#define MASK_USBD_EP5_DMA_EN				BIT0

/********************* Define rEP7CTL bit mask (Offset + 0x1C0, USB device EP7 control) *****************/
#define MASK_USBD_EP7_BUF_FLUSH				BIT7	/* write 1 and write 0 manually, not auto */
#define MASK_USBD_EP7_CTL_VLD				BIT3
#define MASK_USBD_EP7_CTL_EN				BIT0

/********************* Define rEP12_CTRL bit mask (Offset + 0x330, USB device endpoint1/2 control) **********/
#define MASK_USBD_EP12_C_EP2_OUT_DIR		BIT11	//New add
#define MASK_USBD_EP12_C_EP1_IN_DIR			BIT10	//New add
#define MASK_USBD_EP12_C_EP2_OUT_ENA		BIT9	//New add
#define MASK_USBD_EP12_C_EP1_IN_ENA			BIT8	//New add
#define MASK_USBD_EP12_C_MSDC_CMD_VLD		BIT7
#define MASK_USBD_EP12_C_EP2_OVLD			BIT6
#define MASK_USBD_EP12_C_EP1_IVLD			BIT5
#define MASK_USBD_EP12_C_SET_EP1_IVLD		BIT4
#define MASK_USBD_EP12_C_CLR_EP2_OVLD		BIT3
#define MASK_USBD_EP12_C_RESET_PIPO_FIFO	BIT2
#define MASK_USBD_EP12_C_EP12_ENA			BIT1
#if (OLD_BULK_CONTROL_FLOW == 1)
#define MASK_USBD_EP12_C_EP12_DIR			BIT0
#else
#define MASK_USBD_EP12_C_EP12_ENA_AUTO_CLR  BIT0	/* 0: auto clear enable, 1: manual clear */
#endif

/********************* Define rUDC_AS_CTRL bit mask (Offset + 0x334, USB device endpoint1/2 Ping-Pong FIFO control) **********/
#define MASK_USBD_EP12_FIFO_C_A_EP2_OVLD	BIT7
#define MASK_USBD_EP12_FIFO_C_A_EP1_IVLD	BIT6
#define MASK_USBD_EP12_FIFO_C_EP2_OVLD		BIT5
#define MASK_USBD_EP12_FIFO_C_EP1_IVLD		BIT4
#define MASK_USBD_EP12_FIFO_C_EP12_DIR		BIT3
#define MASK_USBD_EP12_FIFO_C_CUR_BUF		BIT2
#define MASK_USBD_EP12_FIFO_C_SWITCH_BUF	BIT1
#define MASK_USBD_EP12_FIFO_C_ASE   		BIT0

/********************* Define rEP0_CTRL bit mask (Offset + 0x34C, USB device endpoint0 control status) ******/
#define MASK_USBD_EP0_CS_EP0_OUT_EMPTY		BIT7
#define MASK_USBD_EP0_CS_EP0_OVLD			BIT6
#define MASK_USBD_EP0_CS_CLE_EP0_OUT_VLD	BIT5
#define MASK_USBD_EP0_CS_EP0_IVLD			BIT4
#define MASK_USBD_EP0_CS_SET_EP0_IN_VLD		BIT3
#define MASK_USBD_EP0_CS_EP0_SFIFO_UPDATE	BIT2
#define MASK_USBD_EP0_CS_EP0_SFIFO_VALID	BIT1
#define MASK_USBD_EP0_CS_EP0_DIR			BIT0

/********************* Define rEP1S_CTRL bit mask (Offset + 0x358, USB device endpoint1 special control status) *****/
#define MASK_USBD_EP1_SCS_CLR_EP1S_IN_VALID			BIT3
#define MASK_USBD_EP1_SCS_RESET_EP1S_FIFO			BIT2
#define MASK_USBD_EP1_SCS_EP1S_IN_VALID				BIT1
#define MASK_USBD_EP1_SCS_SET_EP1S_IN_VALID			BIT0

/********************* Define rEP12_STATUS bit mask (Offset + 0x364, USB device endpoint1/2 FIFO status) *****/
#define MSAK_USBD_EP12_FS_N_MSDC_CMD		BIT7
#define MSAK_USBD_EP12_FS_A_FIFO_EMPTY		BIT6
#define MSAK_USBD_EP12_FS_N_EP2_OVLD		BIT5
#define MSAK_USBD_EP12_FS_P_EP1_IVLD		BIT4
#define MSAK_USBD_EP12_FS_MSDC_CMD_VLD		BIT3
#define MSAK_USBD_EP12_FS_FIFO_EMPTY		BIT2
#define MSAK_USBD_EP12_FS_EP2_OVLD			BIT1
#define MSAK_USBD_EP12_FS_EP1_IVLD			BIT0

/********************* Define rUDLC_SET0 bit mask (Offset + 0x3B0, USB deivce linker layer controller) ******/
#define MASK_USBD_UDLC_SET0_CLEAR_SUSPEND_CNT		BIT7
#define MASK_USBD_UDLC_SET0_SIM_MODE				BIT6
#define MASK_USBD_UDLC_SET0_DISCONNECT_SUSPEND_EN	BIT5
#define MASK_USBD_UDLC_SET0_CPU_WAKE_UP_EN			BIT4
#define MASK_USBD_UDLC_SET0_PWR_PARTIAL_N			BIT3
#define MASK_USBD_UDLC_SET0_PWR_SUSPEND_N			BIT2
#define MASK_USBD_UDLC_SET0_ISSUE_RESUME			BIT1
#define MASK_USBD_UDLC_SET0_SOFT_DISCONNECT			BIT0

/********************* Define rUDLC_SET1 bit mask (Offset + 0x3B4, USB deivce linker layer controller setting 1) **********/
#define MASK_USBD_UDLC_SET1_NO_SE1_SUSPEN_OPTION	BIT7
#define MASK_USBD_UDLC_SET1_NO_STOP_CHIRP			BIT6
#define MASK_USBD_UDLC_SET1_INTER_PACKET_DELAY		BIT5
#define MASK_USBD_UDLC_SET1_FORCE_FULLSP			BIT4
#define MASK_USBD_UDLC_SET1_VBUS_LOW_AUTO_DISC		BIT3
#define MASK_USBD_UDLC_SET1_DISC_AUTO_DPDMPD		BIT2
#define MASK_USBD_UDLC_SET1_SUPP_RWAKE				BIT1
#define MASK_USBD_UDLC_SET1_SELF_POWER				BIT0

/********************* Define rUDC_STATUS bit mask (Offset + 0x3B8, USB deivce linker layer controller status) **********/
#define MASK_USBD_UDLC_CS_DISC_CONNECT_STATUS		BIT7
#define MASK_USBD_UDLC_CS_H_HOST_CONFIGED			BIT6
#define MASK_USBD_UDLC_CS_H_LNK_SUSPENDM			BIT5
#define MASK_USBD_UDLC_CS_H_ALLOW_RWAKE				BIT4
#define MASK_USBD_UDLC_CS_CURR_SPEED				BIT1
#define MASK_USBD_UDLC_CS_VBUS_HIGH					BIT0

/********************* Define rUDC_STALL_CTRL bit mask (Offset + 0x3BC, USB deivce linker layer stall control) ************/
#define MASK_USBD_UDLC_STL_CLREPBSTL				BIT15	/* Clear EPB stall */
#define MASK_USBD_UDLC_STL_CLREPASTL				BIT14	/* Clear EPA stall */
#define MASK_USBD_UDLC_STL_CLREP9STL				BIT13	/* Clear EP9 stall */
#define MASK_USBD_UDLC_STL_CLREP8STL				BIT12	/* Clear EP8 stall */
#define MASK_USBD_UDLC_STL_CLREP3STL				BIT11	/* Clear EP3 stall */
#define MASK_USBD_UDLC_STL_CLREP2STL				BIT10	/* Clear EP2 stall */
#define MASK_USBD_UDLC_STL_CLREP1STL				BIT9	/* Clear EP1 stall */
#define MASK_USBD_UDLC_STL_CLREP0STL				BIT8	/* Clear EP0 stall */
#define MASK_USBD_UDLC_STL_SETEPBSTL				BIT7	/* Set EPB stall */
#define MASK_USBD_UDLC_STL_SETEPASTL				BIT6	/* Set EPA stall */
#define MASK_USBD_UDLC_STL_SETEP9STL				BIT5	/* Set EP9 stall */
#define MASK_USBD_UDLC_STL_SETEP8STL				BIT4	/* Set EP8 stall */
#define MASK_USBD_UDLC_STL_SETEP3STL				BIT3	/* Set EP3 stall */
#define MASK_USBD_UDLC_STL_SETEP2STL				BIT2	/* Set EP2 stall */
#define MASK_USBD_UDLC_STL_SETEP1STL				BIT1	/* Set EP1 stall */
#define MASK_USBD_UDLC_STL_SETEP0STL				BIT0	/* Set EP0 stall */

/********************* Define rUDC_LCSET2 bit mask (Offset + 0x3C0, USB deivce linker layer controller setting 2) *********/
#define MASK_USBD_UDLC_SET2_SUSPEND_REF				BIT3
#define MASK_USBD_UDLC_SET2_USBC_EP1S_FIRST			BIT2
#define MASK_USBD_UDLC_SET2_USBC_STOP_SPD_ENUM		BIT1
#define MASK_USBD_UDLC_SET2_USBC_SUPPORT_EP3		BIT0

/********************* Define rDLCIF_UDLC bit mask (Offset + 0x400, USB deivce linker layer controller) ******/
#define MASK_USBD_UDLC_IF_EP7IEND			BIT25	/* UDLC EP7 DMA IN transaction end flag */
#define MASK_USBD_UDLC_IF_EP5IEND			BIT24	/* UDLC EP5 DMA IN transaction end flag */
#define MASK_USBD_UDLC_IF_EP7I				BIT23	/* UDLC EP7 IN transaction interrupt flag */
#define MASK_USBD_UDLC_IF_EP6I				BIT22	/* UDLC EP6 IN transaction interrupt flag */
#define MASK_USBD_UDLC_IF_EP5I				BIT21	/* UDLC EP5 IN transaction interrupt flag */
#define MASK_USBD_UDLC_IF_EP4I				BIT20	/* UDLC EP4 IN transaction interrupt flag */
#define MASK_USBD_UDLC_IF_RESETN			BIT19	/* UDLC USB_RESET END interrupt flag */
#define MASK_USBD_UDLC_IF_SCONF				BIT18	/* UDLC HOST Set Configuration interrupt flag */
#define MASK_USBD_UDLC_IF_RESUME			BIT17	/* UDLC USB BUS RESUME interrupt flag */
#define MASK_USBD_UDLC_IF_SUSPEND			BIT16	/* UDLC USB BUS SUSPEND interrupt flag */
#define MASK_USBD_UDLC_IF_EP1INN			BIT15	/* UDLC DMA & EP1_IN END interrupt flag */
#define MASK_USBD_UDLC_IF_EP3I				BIT14	/* UDLC EP3_IN transaction interrupt flag */
#define MASK_USBD_UDLC_IF_PIPO				BIT13	/* UDLC PING-PONG_FIFO_SWAP interrupt flag */
#define MASK_USBD_UDLC_IF_HCS				BIT12	/* UDLC HOST_CLEAR_STALL interrupt flag */
#define MASK_USBD_UDLC_IF_EP2N				BIT11	/* UDLC EP2_NAK interrupt flag */
#define MASK_USBD_UDLC_IF_EP1N				BIT10	/* UDLC EP1_NAK interrupt flag */
#define MASK_USBD_UDLC_IF_EP0N				BIT9	/* UDLC EP0_NAK interrupt flag */
#define MASK_USBD_UDLC_IF_HSS				BIT8	/* UDLC HOST_SET_STALL interrupt flag */
#define MASK_USBD_UDLC_IF_EP2O				BIT7	/* UDLC EP2_OUT transaction interrupt flag */
#define MASK_USBD_UDLC_IF_EP1I				BIT6	/* UDLC EP1_IN transaction interrupt flag */
#define MASK_USBD_UDLC_IF_EP1SI				BIT5	/* UDLC EP1S_IN transaction interrupt flag */
#define MASK_USBD_UDLC_IF_EP0I				BIT4	/* UDLC EP0_IN transaction interrupt flag */
#define MASK_USBD_UDLC_IF_EP0O				BIT3	/* UDLC EP0_OUT transaction interrupt flag */
#define MASK_USBD_UDLC_IF_EP0S				BIT2	/* UDLC EP0_SETUP transaction interrupt flag */
#define MASK_USBD_UDLC_IF_SUSP				BIT1	/* UDLC USB_SUSPEND_DIFF interrupt flag */
#define MASK_USBD_UDLC_IF_RESET				BIT0	/* UDLC USB_RESET interrupt flag */

/********************* Define rDLCIE_UDLC bit mask (Offset + 0x404, USB deivce linker layer controller) ******/
#define MASK_USBD_UDLC_IE_EP7IEND			BIT25	/* UDLC EP7 DMA IN transaction end flag */
#define MASK_USBD_UDLC_IE_EP5IEND			BIT24	/* UDLC EP5 DMA IN transaction interrupt enable */
#define MASK_USBD_UDLC_IE_EP7I				BIT23	/* UDLC EP7 IN transaction interrupt enable */
#define MASK_USBD_UDLC_IE_EP6I				BIT22	/* UDLC EP6 IN transaction interrupt enable */
#define MASK_USBD_UDLC_IE_EP5I				BIT21	/* UDLC EP5 IN transaction interrupt enable */
#define MASK_USBD_UDLC_IE_EP4I				BIT20	/* UDLC EP4 IN transaction interrupt enable */
#define MASK_USBD_UDLC_IE_RESETN			BIT19	/* UDLC USB_RESET END interrupt enable */
#define MASK_USBD_UDLC_IE_SCONF				BIT18	/* UDLC HOST set configuration interrupt enable */
#define MASK_USBD_UDLC_IE_RESUME			BIT17	/* UDLC USB BUS RESUME interrupt enable */
#define MASK_USBD_UDLC_IE_SUSPEND			BIT16	/* UDLC USB BUS SUSPEND interrupt enable */
#define MASK_USBD_UDLC_IE_EP1INN			BIT15	/* UDLC DMA & EP1_IN END interrupt enable */
#define MASK_USBD_UDLC_IE_EP3I				BIT14	/* UDLC EP3_IN transaction interrupt enable */
#define MASK_USBD_UDLC_IE_PIPO				BIT13	/* UDLC PING-PONG_FIFO_SWAP interrupt enable */
#define MASK_USBD_UDLC_IE_HCS				BIT12	/* UDLC HOST_CLEAR_STALL interrupt enable */
#define MASK_USBD_UDLC_IE_EP2N				BIT11	/* UDLC EP2_NAK interrupt enable */
#define MASK_USBD_UDLC_IE_EP1N				BIT10	/* UDLC EP1_NAK interrupt enable */
#define MASK_USBD_UDLC_IE_EP0N				BIT9	/* UDLC EP0_NAK interrupt enable */
#define MASK_USBD_UDLC_IE_HSS				BIT8	/* UDLC HOST_SET_STALL interrupt enable */
#define MASK_USBD_UDLC_IE_EP2O				BIT7	/* UDLC EP2_OUT transaction interrupt enable */
#define MASK_USBD_UDLC_IE_EP1I				BIT6	/* UDLC EP1_IN transaction interrupt enable */
#define MASK_USBD_UDLC_IE_EP1SI				BIT5	/* UDLC EP1S_IN transaction interrupt enable */
#define MASK_USBD_UDLC_IE_EP0I				BIT4	/* UDLC EP0_IN transaction interrupt enable */
#define MASK_USBD_UDLC_IE_EP0O				BIT3	/* UDLC EP0_OUT transaction interrupt enable */
#define MASK_USBD_UDLC_IE_EP0S				BIT2	/* UDLC EP0_SETUP transaction interrupt enable */
#define MASK_USBD_UDLC_IE_SUSP				BIT1	/* UDLC USB_SUSPEND_DIFF interrupt enable */
#define MASK_USBD_UDLC_IE_RESET				BIT0	/* UDLC USB_RESET interrupt enable */

/********************* Define rEP89_DMA bit mask (Offset + 0x010, EP89 DMA control status) ******/
#define MASK_USBD_EP89_DMA_EN				BIT31
#define MASK_USBD_EP89_DMA_FLUSH			BIT30
#define MASK_USBD_EP89_DMA_FIFO_FLUSH		BIT29
#define MASK_USBD_EP89_DMA_WRITE			BIT28
#define MASK_USBD_EP89_DMA_MODIFY_EN		BIT27
#define MASK_USBD_EP89_DMA_COUNT_ALIGN		BIT26

/********************* Define rEPAB_DMA bit mask (Offset + 0x018, EPAB DMA control status) ******/
#define MASK_USBD_EPAB_DMA_EN				BIT31
#define MASK_USBD_EPAB_DMA_FLUSH			BIT30
#define MASK_USBD_EPAB_DMA_FIFO_FLUSH		BIT29
#define MASK_USBD_EPAB_DMA_WRITE			BIT28
#define MASK_USBD_EPAB_DMA_MODIFY_EN		BIT27
#define MASK_USBD_EPAB_DMA_COUNT_ALIGN		BIT26

/********************* Define rSTANDARD_REQ_IF bit mask (Offset + 0x410, standard request inerrupt flag) ******/
#define MASK_USBD_STDREQ_IF_SET_DESC		BIT14	/* Set descriptor interrupt flag */
#define MASK_USBD_STDREQ_IF_GET_DESC		BIT13	/* Get descriptor interrupt flag */
#define MASK_USBD_STDREQ_IF_GET_CONF		BIT12	/* Get configuration interrupt flag */
#define MASK_USBD_STDREQ_IF_GET_STS			BIT11	/* Get status interrupt flag */
#define MASK_USBD_STDREQ_IF_GET_INTF		BIT10	/* Get interface interrupt flag */
#define MASK_USBD_STDREQ_IF_SET_INTF		BIT9	/* Set interface interrupt flag */
#define MASK_USBD_STDREQ_IF_SET_ADDR		BIT8	/* Set address interrupt flag */
#define MASK_USBD_STDREQ_IF_SET_CONF		BIT7	/* Set configuration interrupt flag */
#define MASK_USBD_STDREQ_IF_SET_TEST		BIT6	/* Set feature interrupt flag */
#define MASK_USBD_STDREQ_IF_SET_REMOTE		BIT5	/* Set remote interrupt flag */
#define MASK_USBD_STDREQ_IF_SET_EPX_STALL	BIT4	/* Set feature interrupt flag (other EP stall) */
#define MASK_USBD_STDREQ_IF_SET_EP0_STALL	BIT3	/* Set feature interrupt flag (EP0 stall) */
#define MASK_USBD_STDREQ_IF_CLR_REMOTE_IF	BIT2	/* Clear feature interrupt flag (device remote wakeup) */
#define MASK_USBD_STDREQ_IF_CLR_EPX_STALL	BIT1	/* Clear feature interrupt flag (other EP stall) */
#define MASK_USBD_STDREQ_IF_CLR_EP0_STALL	BIT0	/* Clear feature interrupt flag (EP0 stall) */

/********************* Define rSTANDARD_REQ_IE bit mask (Offset + 0x414, standard request inerrupt enable) ******/
#define MASK_USBD_STDREQ_IE_SET_DESC		BIT14	/* Set descriptor interrupt enable */
#define MASK_USBD_STDREQ_IE_GET_DESC		BIT13	/* Get descriptor interrupt enable */
#define MASK_USBD_STDREQ_IE_GET_CONF		BIT12	/* Get configuration interrupt enable */
#define MASK_USBD_STDREQ_IE_GET_STS			BIT11	/* Get status interrupt enable */
#define MASK_USBD_STDREQ_IE_GET_INTF		BIT10	/* Get interface interrupt enable */
#define MASK_USBD_STDREQ_IE_SET_INTF		BIT9	/* Set interface interrupt enable */
#define MASK_USBD_STDREQ_IE_SET_ADDR		BIT8	/* Set address interrupt enable */
#define MASK_USBD_STDREQ_IE_SET_CONF		BIT7	/* Set configuration interrupt enable */
#define MASK_USBD_STDREQ_IE_SET_TEST		BIT6	/* Set feature interrupt enable */
#define MASK_USBD_STDREQ_IE_SET_REMOTE		BIT5	/* Set remote interrupt enable */
#define MASK_USBD_STDREQ_IE_SET_EPX_STALL	BIT4	/* Set feature interrupt enable (other EP stall) */
#define MASK_USBD_STDREQ_IE_SET_EP0_STALL	BIT3	/* Set feature interrupt enable (EP0 stall) */
#define MASK_USBD_STDREQ_IE_CLR_REMOTE_IF	BIT2	/* Clear feature interrupt enable (device remote wakeup) */
#define MASK_USBD_STDREQ_IE_CLR_EPX_STALL	BIT1	/* Clear feature interrupt enable (other EP stall) */
#define MASK_USBD_STDREQ_IE_CLR_EP0_STALL	BIT0	/* Clear feature interrupt enable (EP0 stall) */

/********************* Define rNEWEP_IF bit mask (Offset + 0x420, new bulk EP inerrupt flag) ******/
#define MASK_USBD_NEWEP_IF_EP11O			BIT11	/* EP11_OUT transaction interrupt flag */
#define MASK_USBD_NEWEP_IF_EP10I			BIT10	/* EP10_IN transaction interrupt flag */
#define MASK_USBD_NEWEP_IF_EP11NAK			BIT9	/* EP11 NAK interrupt flag */
#define MASK_USBD_NEWEP_IF_EP10NAK			BIT8	/* EP10 NAK interrupt flag */
#define MASK_USBD_NEWEP_IF_PIPOEPAB			BIT7	/* EP10/11 PING-PONG FIFO SWAP interrupt flag */
#define MASK_USBD_NEWEP_IF_EPADMA			BIT6	/* EP10 DMA IN data finish interrupt flag */
#define MASK_USBD_NEWEP_IF_EP9O				BIT5	/* EP9 OUT transaction interrupt flag */
#define MASK_USBD_NEWEP_IF_EP8I				BIT4	/* EP8 IN transaction interrupt flag */
#define MASK_USBD_NEWEP_IF_EP9N				BIT3	/* EP9 NAK transaction interrupt flag */
#define MASK_USBD_NEWEP_IF_EP8N				BIT2	/* EP8 NAK transaction interrupt flag */
#define MASK_USBD_NEWEP_IF_PIPOEP89			BIT1	/* EP89 PING-PONG FIFO SWAP interrupt flag */
#define MASK_USBD_NEWEP_IF_EP8DMA			BIT0	/* EP8 DMA IN data finish interrupt flag */

/********************* Define rNEWEP_IE bit mask (Offset + 0x424, new bulk EP inerrupt enable) ******/
#define MASK_USBD_NEWEP_IE_EP11O			BIT11	/* EP11_OUT transaction interrupt enable */
#define MASK_USBD_NEWEP_IE_EP10I			BIT10	/* EP10_IN transaction interrupt enable */
#define MASK_USBD_NEWEP_IE_EP11NAK			BIT9	/* EP11 NAK interrupt enable */
#define MASK_USBD_NEWEP_IE_EP10NAK			BIT8	/* EP10 NAK interrupt enable */
#define MASK_USBD_NEWEP_IE_PIPOEPAB			BIT7	/* EP10/11 PING-PONG FIFO SWAP interrupt enable */
#define MASK_USBD_NEWEP_IE_EPADMA			BIT6	/* EP10 DMA IN data finish interrupt enable */
#define MASK_USBD_NEWEP_IE_EP9O				BIT5	/* EP9 OUT transaction interrupt enable */
#define MASK_USBD_NEWEP_IE_EP8I				BIT4	/* EP8 IN transaction interrupt enable */
#define MASK_USBD_NEWEP_IE_EP9N				BIT3	/* EP9 NAK transaction interrupt enable */
#define MASK_USBD_NEWEP_IE_EP8N				BIT2	/* EP8 NAK transaction interrupt enable */
#define MASK_USBD_NEWEP_IE_PIPOEP89			BIT1	/* EP89 PING-PONG FIFO SWAP interrupt enable */
#define MASK_USBD_NEWEP_IE_EP8DMA			BIT0	/* EP8 DMA IN data finish interrupt enable */

/********************* Define rEP89_CTRL bit mask (Offset + 0x500, EP8/9 control) ******/
#define MASK_USBD_EP89_CTL_EP9_OUT_ENA		BIT8
#define MASK_USBD_EP89_CTL_EP8_IN_ENA		BIT7
#define MASK_USBD_EP89_CTL_EP9_OVLD			BIT6	/* Current EP9 OUT buffer valid flag status */
#define MASK_USBD_EP89_CTL_EP8_IVLD			BIT5	/* Current EP8 IN buffer valid flag status */
#define MASK_USBD_EP89_CTL_SET_EP8_IVLD		BIT4	/* Set current EP8 IN buffer valid flag status */
#define MASK_USBD_EP89_CTL_CLR_EP9_OVLD		BIT3	/* Clear current EP9 OUT buffer valid flag status */
#define MASK_USBD_EP89_CTL_RESET_PIPO_FIFO	BIT2	/* Write 1 to reset current and next EP8/9 FIFO */
#define MASK_USBD_EP89_CTL_EP89_ENA			BIT1	/* enable/disable bulk in/out */
#define MASK_USBD_EP89_CTL_EP89_DIR			BIT0	/* 0 = BULK OUT, 1= BULK IN */

/********************* Define rEP89_PPC bit mask (Offset + 0x504, EP8/9 PING-PONG FIFO control) ******/
#define MASK_USBD_EP89_PPC_A_EP9_OVLD		BIT7	/* Another EP9 OUT buffer valid flag status */
#define MASK_USBD_EP89_PPC_A_EP8_IVLD		BIT6	/* Another EP8 IN buffer valid flag status */
#define MASK_USBD_EP89_PPC_EP9_OVLD			BIT5	/* Current EP9 OUT buffer valid flag status */
#define MASK_USBD_EP89_PPC_EP8_IVLD			BIT4	/* Current EP8 IN buffer valid flag status */
#define MASK_USBD_EP89_PPC_EP89DIR			BIT3	/* EP89 DIR, 0 = BULK OUT, 1 = BULK IN */
#define MASK_USBD_EP89_PPC_CURRENT_BUF		BIT2	/* 1 = buffer0(PING), 0 = buffer1(PONG) */
#define MASK_USBD_EP89_PPC_SWITCH_BUF		BIT1	/* Write 1 to switch buffer0/buffer1 */
#define MASK_USBD_EP89_PPC_AUTO_SWITCH_EN	BIT0	/* 0 = ping pong will not auto switch, 1 = ping pong will auto switch */

/********************* Define rEP89_FS bit mask (Offset + 0x508, EP8/9 PING-PONG FIFO status) ******/
#define MASK_USBD_EP89_FS_A_FIFO_EMPTY		BIT6	/* Another buffer status(CPU can't access now) 0 = empty, 1= empty */
#define MASK_USBD_EP89_FS_A_EP9_OVLD		BIT5	/* Another EP9 OUT buffer valid flag status, 0 = invalid, 1= valid */
#define MASK_USBD_EP89_FS_A_EP8_IVLD		BIT4	/* Another EP8 IN buffer valid flag status, 0 = invalid, 1= valid */
#define MASK_USBD_EP89_FS_FIFO_EMPTY		BIT2	/* Data in current buffer, 0 = not empty, 1= empty */
#define MASK_USBD_EP89_FS_EP9_OVLD 			BIT1	/* Same as 0x504[5] */
#define MASK_USBD_EP89_FS_EP8_IVLD 			BIT0	/* Same as 0x504[4] */

/********************* Define rEPAB_CTRL bit mask (Offset + 0x550, EPA/B control) ******/
#define MASK_USBD_EPAB_CTL_EP11_OUT_ENA		BIT8
#define MASK_USBD_EPAB_CTL_EP10_IN_ENA		BIT7
#define MASK_USBD_EPAB_CTL_EP11_OVLD		BIT6	/* Current EP11 OUT buffer valid flag status */
#define MASK_USBD_EPAB_CTL_EP10_IVLD		BIT5	/* Current EP10 IN buffer valid flag status */
#define MASK_USBD_EPAB_CTL_SET_EP10_IVLD	BIT4	/* Set current EP10 IN buffer valid flag status */
#define MASK_USBD_EPAB_CTL_CLR_EP11_OVLD	BIT3	/* Clear current EP11 OUT buffer valid flag status */
#define MASK_USBD_EPAB_CTL_RESET_PIPO_FIFO	BIT2	/* Write 1 to reset current and next EPA/B FIFO */
#define MASK_USBD_EPAB_CTL_EPAB_ENA			BIT1	/* enable/disable bulk in/out */
#define MASK_USBD_EPAB_CTL_EPAB_DIR			BIT0	/* 0 = BULK OUT, 1= BULK IN */

/********************* Define rEPAB_PPC bit mask (Offset + 0x554, EPA/B PING-PONG FIFO control) ******/
#define MASK_USBD_EPAB_PPC_A_EP11_OVLD		BIT7	/* Another EP11 OUT buffer valid flag status */
#define MASK_USBD_EPAB_PPC_A_EP10_IVLD		BIT6	/* Another EP10 IN buffer valid flag status */
#define MASK_USBD_EPAB_PPC_EP11_OVLD		BIT5	/* Current EP11 OUT buffer valid flag status */
#define MASK_USBD_EPAB_PPC_EP10_IVLD		BIT4	/* Current EP10 IN buffer valid flag status */
#define MASK_USBD_EPAB_PPC_EPABDIR			BIT3	/* EPAB DIR, 0 = BULK OUT, 1 = BULK IN */
#define MASK_USBD_EPAB_PPC_CURRENT_BUF		BIT2	/* 1 = buffer0(PING), 0 = buffer1(PONG) */
#define MASK_USBD_EPAB_PPC_SWITCH_BUF		BIT1	/* Write 1 to switch buffer0/buffer1 */
#define MASK_USBD_EPAB_PPC_AUTO_SWITCH_EN	BIT0	/* 0 = ping pong will not auto switch, 1 = ping pong will auto switch */

/********************* Define rEPAB_FS bit mask (Offset + 0x558, EPA/B PING-PONG FIFO status) ******/
#define MASK_USBD_EPAB_FS_A_FIFO_EMPTY		BIT6	/* Another buffer status(CPU can't access now) 0 = empty, 1= empty */
#define MASK_USBD_EPAB_FS_A_EP11_OVLD		BIT5	/* Another EP11 OUT buffer valid flag status, 0 = invalid, 1= valid */
#define MASK_USBD_EPAB_FS_A_EP10_IVLD		BIT4	/* Another EP10 IN buffer valid flag status, 0 = invalid, 1= valid */
#define MASK_USBD_EPAB_FS_FIFO_EMPTY		BIT2	/* Data in current buffer, 0 = not empty, 1= empty */
#define MASK_USBD_EPAB_FS_EP11_OVLD			BIT1	/* Same as 0x554[5] */
#define MASK_USBD_EPAB_FS_EP10_IVLD			BIT0	/* Same as 0x554[4] */

/********************* Define rEP3CS bit mask (Offset + 0x374, Endpoint 3 control status) ******/
#define MASK_USBD_EP3_CS_EP3_VLD			BIT3	/* EP3 valid bit */
#define MASK_USBD_EP3_CS_EP3_DIR			BIT0	/* EP3 ebable bit */

/********************* Define rEP4CS bit mask (Offset + 0x100, Endpoint 3 control status) ******/
#define MASK_USBD_EP4_CS_EP4_VLD			BIT3	/* EP4 read valid bit */
#define MASK_USBD_EP4_CS_EP4_DIR			BIT0	/* EP4 ebable bit */

/********************* Define rEP6CS bit mask (Offset + 0x180, Endpoint 3 control status) ******/
#define MASK_USBD_EP6_CS_EP6_VLD			BIT3	/* EP6 read valid bit */
#define MASK_USBD_EP6_CS_EP6_DIR			BIT0	/* EP6 ebable bit */

/****** Define USB device FIFO size *******/
#define     EP0_FIFO_SIZE       64 //usb2.0
#define     EP0_FIFO_SIZE_MASK  0x3f
#define     BULK_FIFO_SIZE      512
#define     INT_EP6_FIFO_SIZE   8
#define		INT_EP3_FIFO_SIZE	64
#define		INT_EP4_FIFO_SIZE	32
#define     ISO_EP5_FIFO_SIZE   4096
#define     ISO_EP7_FIFO_SIZE   256

#define CONTROL_DIR_OUT	0
#define CONTROL_DIR_IN	1

#define BULK_SEND_ERROR_WAIT_COUNT 60000000	/* for BULK busy wait counter */

/* For send error to host when bulk endpoint error */
#define    BULK_SEND_STALL		0
#define	   BULK_SEND_NULL_PKT	1

#define STATUS_OK 0
#define STATUS_FAIL -1

//#define DBG_PRINT	print_string
#define gp_memcpy	memcpy
#define gp_memset	memset
//#define gp_free(...)
//#define gp_malloc_align(...)
extern void * gp_malloc_align(INT32U size, INT32U align);		// SDRAM allocation. Align value must be power of 2, eg: 2, 4, 8, 16, 32...
extern void gp_free(void *ptr);

// #define gp_malloc(...)
#define tiny_counter_get(...)

#define MSDC_SDRAM_BUF_SIZE 128

#define VALID_BIT_4BYTE		0x0F
#define VALID_BIT_3BYTE		0x07
#define VALID_BIT_2BYTE		0x03
#define VALID_BIT_1BYTE		0x01
#define VALID_BIT_0BYTE		0x00


/* Random Number Generator register definitions for getting random seed */
#define RNG_READ_REG  			   *(volatile unsigned *)(0x90009000)		/* RNG Enable Register */
#define RNG_LONG_RUN_VALUE_REG	   *(volatile unsigned *)(0x90009004)		/* Reserved Register */
#define RNG_MANUAL_SEED_RESET_REG  *(volatile unsigned *)(0x90009008)		/* RNG Seed Reset Register */
#define RNG_OUTPUT_DATA_REG		   *(volatile unsigned *)(0x90009014)		/* RNG Output Data Register */
#define RNG_OUTPUT_VALID_REG	   *(volatile unsigned *)(0x90009018)		/* RNG Output Status Register */
#define RNG_LONG_RUN_COUNT_REG	   *(volatile unsigned *)(0x9000001C)		/* Reserved Register */

typedef union _L1_SETUP_CMD_MSG
{
	INT8U setup_msg[8];
	struct setup_cmd_field
	{
		INT8U   bmRequestType;
    	INT8U   bRequest;
    	INT16U  wValue;
    	INT16U  wIndex;
    	INT16U  wLength;	
	} cmd_pkt;	
} L1_SETUP_CMD_MSG;	

extern void print_string(CHAR *fmt, ...);
/*********************************************************************
        Structure and enumeration
*********************************************************************/
/*********************************************************************
        macro function declaration
**********************************************************************/

/*********************************************************************
        extern functions/variable
**********************************************************************/
/************** USBD API *****************************/
extern INT32S drv_l1_usbd_enable_isr(USBDISRHANDLER isr_handle);
extern void drv_l1_usb_soft_disconnect(void);
extern INT32S drv_l1_usbd_init(void);
extern INT32S drv_l1_usbd_uninit(void);
extern INT32S drv_l1_usbd_disable_isr(void);
extern void drv_l1_usbd_reset(void);
extern void drv_l1_usbd_isr_handle(void);
extern void drv_l1_usbd_set_ep_stall(INT16U mask, INT8U enable);
extern INT32S drv_l1_usbd_suspend(void);
/************** USBD EP0 API *************************/
extern INT32U drv_l1_usbd_send_ep0_in(INT8U * ptr, INT32U len);
extern INT32U drv_l1_usbd_get_ep0_out_cnt(void);
extern INT32S drv_l1_usbd_get_ep0_out_data(void* ptr, INT32U len);
extern void drv_l1_usbd_send_ep0_null_pkt(void);
extern void drv_l1_usbd_send_ep0_stall(void);
extern void drv_l1_usbd_clean_ep0_in(void);
extern void drv_l1_usbd_enable_receive_ep0_out(void);
extern void drv_l1_usbd_disable_receive_ep0_out(void);
extern void drv_l1_usbd_stop_send_ep0_null(void);
extern void drv_l1_usbd_get_setup_pkt(void* msg);
extern void drv_l1_usbd_enble_ep0_state_clear(void);
extern void drv_l1_usbd_disable_ep0_state_clear(void);
/************** USBD BULK API *************************/
extern INT32S drv_l1_usbd_send_bulk_in(void* ptr, INT32U len);
extern INT32S drv_l1_usbd_rec_bulk_out(void* ptr, INT32U len);
extern INT32U drv_l1_usbd_get_bulk_out_cnt(void);
extern INT32S drv_l1_usbd_get_bulk_out_pkt_data(void* ptr, INT32U len);
extern INT32S drv_l1_usbd_get_dma_bulk_out_data_by_len(void* ptr, INT32U len);
extern void drv_l1_usbd_enable_rec_bulk_out_pkt(void);
extern void drv_l1_usbd_disable_bulk_in_out(void);
extern void drv_l1_usbd_enable_bulk_in(void);
extern void drv_l1_usbd_disable_bulk_in(void);
extern void drv_l1_usbd_send_bulk_stall_null(INT8U dir);
extern void drv_l1_usbd_reset_bulk_dma_type(void);

extern INT32S drv_l1_usbd_send_bulk89_in(void* ptr, INT32U len);
extern INT32S drv_l1_usbd_send_bulkAB_in(void* ptr, INT32U len);
extern INT32S drv_l1_usbd_get_dma_bulk89_out_data_by_len(void* ptr, INT32U len);
extern void drv_l1_usbd_enable_rec_bulk89_out_pkt(void);
extern void drv_l1_usbd_enable_rec_bulkAB_out_pkt(void);
extern INT32S drv_l1_usbd_get_dma_bulkAB_out_data_by_len(void* ptr, INT32U len);
extern INT32U drv_l1_usbd_get_bulk89_out_cnt(void);
extern INT32U drv_l1_usbd_get_bulkAB_out_cnt(void);
extern INT32S drv_l1_usbd_get_bulk89_out_pkt_data(void* ptr, INT32U len);
extern INT32S drv_l1_usbd_get_bulkAB_out_pkt_data(void* ptr, INT32U len);
extern void drv_l1_usbd_disable_bulk89(void);
extern void drv_l1_usbd_disable_bulkAB(void);
extern void drv_l1_usbd_switch_fifo_to_valid_data(void);
/************** USBD INT API *************************/
extern INT32S drv_l1_usbd_send_int_ep3_in(void* ptr, INT32U len);
extern INT32S drv_l1_usbd_send_int_ep4_in(void* ptr, INT32U len);
extern INT32S drv_l1_usbd_send_int_ep6_in(void* ptr, INT32U len);
extern void drv_l1_usbd_disable_int_in_packet_clear_interrupt(void);
extern void drv_l1_usbd_enable_int_in(void);

/************** USBD ISO API *************************/
extern void drv_l1_usbd_dma_iso_ep5_in(void*buf, INT32U len);
extern INT32U drv_l1_usbd_iso_ep5_get_alt_intf_num(void);
extern void drv_l1_usbd_frame_iso_ep5_in(void*buf, INT32U len, INT8U toggle);
extern void drv_l1_usbd_iso_ep5_flush(void);
extern void drv_l1_usbd_set_frame_end_ep5(void);
extern void drv_l1_usbd_dma_iso_ep7_in(void*buf, INT32U len);
extern INT32U drv_l1_usbd_iso_ep7_get_alt_intf_num(void);
extern void drv_l1_usbd_iso_ep7_flush(void);
/************** USBD I2C command API *************************/
extern void drv_l1_usbd_phy_i2c_write_cmd(INT8U device, INT8U addr, INT8U data);
extern void drv_l1_usbd_phy_i2c_read_cmd(INT8U device, INT8U addr, INT8U* data);

/************** Random Number Generator API *************************/
extern INT32U drv_l1_get_random_num(void);
#endif  //DRV_L1_USBD_H

