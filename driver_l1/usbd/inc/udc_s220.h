#ifndef __UDC_S220_H__
#define __UDC_S220_H__

/* Basic type defines */
typedef unsigned char			UINT8;
typedef unsigned short			UINT16;
typedef unsigned int			UINT32;
typedef unsigned long long		UINT64;
typedef unsigned char       	BOOL;
typedef unsigned char       	INT8U;
typedef signed   char       	INT8S;
typedef unsigned short      	INT16U;
typedef signed   short      	INT16S;
typedef unsigned int        	INT32U;
typedef signed   int        	INT32S;
typedef	char                	INT8;
typedef	signed short        	SINT16;
typedef	short               	INT16;
typedef	signed int              SINT32;
typedef	int                     INT32;
typedef unsigned long long      UINT64;		
typedef	signed long long        SINT64;
typedef	long long               INT64;

#define	VBUS_SAMPLE_PERIOD_MASK		0xFFFF

/* USB device register definitions */
#define  UDC_BASE					(0xD1100000)
                                																														
#define  rUDC_EP12DMA	 			(*(volatile unsigned *)(UDC_BASE+0x000))																														
#define  rUDC_EP12DA	 			(*(volatile unsigned *)(UDC_BASE+0x004))																														
#define  rUDC_EP7DMA	 			(*(volatile unsigned *)(UDC_BASE+0x008))
#define  rUDC_EP7DA		 			(*(volatile unsigned *)(UDC_BASE+0x00C))

#define  rUDPHYI2CCR	 			(*(volatile unsigned *)(UDC_BASE+0x020))
#define  rUDPHYI2CDR	 			(*(volatile unsigned *)(UDC_BASE+0x024))

#define  rUDCCS_UDC		 			(*(volatile unsigned *)(UDC_BASE+0x080))																														
#define  rUDC_IRQ_ENABLE			(*(volatile unsigned *)(UDC_BASE+0x084))																														
#define  rUDC_IRQ_FLAG				(*(volatile unsigned *)(UDC_BASE+0x088))																														
#define  rUDC_IRQ_SOURCE			(*(volatile unsigned *)(UDC_BASE+0x08c))
#define  rEP4CS						(*(volatile unsigned *)(UDC_BASE+0x100))																															
#define  rEP4DC						(*(volatile unsigned *)(UDC_BASE+0x104))																														
#define  rEP4DP						(*(volatile unsigned *)(UDC_BASE+0x108))																																
#define  rEP4VB						(*(volatile unsigned *)(UDC_BASE+0x10C))
#define  rEP5CTL					(*(volatile unsigned *)(UDC_BASE+0x140))
#define  rEP5HDLEN					(*(volatile unsigned *)(UDC_BASE+0x144))
#define  rEP5FRAMCTL				(*(volatile unsigned *)(UDC_BASE+0x148))
#define  rEP5HDCTRL					(*(volatile unsigned *)(UDC_BASE+0x14C))
#define  rEP5EN						(*(volatile unsigned *)(UDC_BASE+0x150))
#define  rEP5RPTR					(*(volatile unsigned *)(UDC_BASE+0x154))
#define  rEP5WPTR					(*(volatile unsigned *)(UDC_BASE+0x158))
#define  rEP5FIFO					(*(volatile unsigned *)(UDC_BASE+0x15C))
#define  rEP5DMAEN					(*(volatile unsigned *)(UDC_BASE+0x160))
#define  rEP5VB						(*(volatile unsigned *)(UDC_BASE+0x170))
#define  rEPIFALTIF					(*(volatile unsigned *)(UDC_BASE+0x174))
#define  rEP6CS						(*(volatile unsigned *)(UDC_BASE+0x180))																															
#define  rEP6DC						(*(volatile unsigned *)(UDC_BASE+0x184))																														
#define  rEP6DP						(*(volatile unsigned *)(UDC_BASE+0x188))																																
#define  rEP6VB						(*(volatile unsigned *)(UDC_BASE+0x18C))
#define  rEP7CTL					(*(volatile unsigned *)(UDC_BASE+0x1C0))
#define  rEP7RPTR					(*(volatile unsigned *)(UDC_BASE+0x1C4))
#define  rEP7WPTR					(*(volatile unsigned *)(UDC_BASE+0x1C8))
#define  rEP7FIFO					(*(volatile unsigned *)(UDC_BASE+0x1CC))
#define  rEP7VB						(*(volatile unsigned *)(UDC_BASE+0x1D0))
#define  rDVIDEO_REFCLOCK			(*(volatile unsigned *)(UDC_BASE+0x200))																						
#define  rUDC_BIT_OP0				(*(volatile unsigned *)(UDC_BASE+0x320))																														
#define  rUDC_CP_CBW_TAG			(*(volatile unsigned *)(UDC_BASE+0x328))																														
#define  rEP12_CTRL					(*(volatile unsigned *)(UDC_BASE+0x330))																														
#define  rUDC_AS_CTRL				(*(volatile unsigned *)(UDC_BASE+0x334))
#define  rEP0_VB					(*(volatile unsigned *)(UDC_BASE+0x340))																														
#define  rEP0_SETUP_CTRL			(*(volatile unsigned *)(UDC_BASE+0x344))																														
#define  rEP0_SETUP_FIFO			(*(volatile unsigned *)(UDC_BASE+0x348))																														
#define  rEP0_CTRL					(*(volatile unsigned *)(UDC_BASE+0x34C))																														
#define  rEP0_CNTR					(*(volatile unsigned *)(UDC_BASE+0x350))																														
#define  rEP0_FIFO					(*(volatile unsigned *)(UDC_BASE+0x354))																														
#define  rEP1S_CTRL					(*(volatile unsigned *)(UDC_BASE+0x358))																														
#define  rEP1S_FIFO					(*(volatile unsigned *)(UDC_BASE+0x35C))																														
#define  rEP12_STATUS				(*(volatile unsigned *)(UDC_BASE+0x364))																														
#define  rEP12_CNTRL				(*(volatile unsigned *)(UDC_BASE+0x368))																														
#define  rEP12_CNTRH				(*(volatile unsigned *)(UDC_BASE+0x36C))																														
#define  rEP12_PIPO					(*(volatile unsigned *)(UDC_BASE+0x370))																														
#define  rEP3CS						(*(volatile unsigned *)(UDC_BASE+0x374))																															
#define  rEP3DC						(*(volatile unsigned *)(UDC_BASE+0x378))																														
#define  rEP3DP						(*(volatile unsigned *)(UDC_BASE+0x37C))																																
#define  rEP3VB						(*(volatile unsigned *)(UDC_BASE+0x380))		
#define  rEP0_OUT_NAK				(*(volatile unsigned *)(UDC_BASE+0x384))																														
#define  rEP0_IN_NAK				(*(volatile unsigned *)(UDC_BASE+0x388))																														
#define  rEP1_NAK					(*(volatile unsigned *)(UDC_BASE+0x38C))																														
#define  rEP2_NAK					(*(volatile unsigned *)(UDC_BASE+0x390))
#define  rEP12_VB					(*(volatile unsigned *)(UDC_BASE+0x398))
#define  rEP12_POCNTL				(*(volatile unsigned *)(UDC_BASE+0x39C))																													
#define  rEP12_POCNTH				(*(volatile unsigned *)(UDC_BASE+0x3A0))
#define  rUDLC_SET0					(*(volatile unsigned *)(UDC_BASE+0x3B0))																														
#define  rUDLC_SET1					(*(volatile unsigned *)(UDC_BASE+0x3B4))																														
#define  rUDC_STATUS				(*(volatile unsigned *)(UDC_BASE+0x3B8))																														
#define  rUDC_STALL_CTRL			(*(volatile unsigned *)(UDC_BASE+0x3BC))																														
#define  rUDLC_SET2					(*(volatile unsigned *)(UDC_BASE+0x3C0))																															
#define  rUDC_LCS2					(*(volatile unsigned *)(UDC_BASE+0x3C4))
#define  rUDC_LCS3					(*(volatile unsigned *)(UDC_BASE+0x3C8))
#define  rUDC_ADDR					(*(volatile unsigned *)(UDC_BASE+0x3F4))																															
#define  rDLCIF_UDLC  				(*(volatile unsigned *)(UDC_BASE+0x400))																														
#define  rDLCIE_UDLC  				(*(volatile unsigned *)(UDC_BASE+0x404))																														
#define  rDLCIS_UDC  				(*(volatile unsigned *)(UDC_BASE+0x408))
#define  rSTANDARD_REQ_IF			(*(volatile unsigned *)(UDC_BASE+0x410))
#define  rSTANDARD_REQ_IE			(*(volatile unsigned *)(UDC_BASE+0x414))
/* New register definition for EP89 AB */
#define  rEP89_DMA					(*(volatile unsigned *)(UDC_BASE+0x010))
#define  rEP89_DA					(*(volatile unsigned *)(UDC_BASE+0x014))
#define  rEPAB_DMA					(*(volatile unsigned *)(UDC_BASE+0x018))
#define  rEPAB_DA					(*(volatile unsigned *)(UDC_BASE+0x01C))
#define  rNEWEP_IF  				(*(volatile unsigned *)(UDC_BASE+0x420))
#define  rNEWEP_IE  				(*(volatile unsigned *)(UDC_BASE+0x424))
#define  rEP89_CTRL  				(*(volatile unsigned *)(UDC_BASE+0x500))
#define  rEP89_PPC					(*(volatile unsigned *)(UDC_BASE+0x504))
#define  rEP89_FS					(*(volatile unsigned *)(UDC_BASE+0x508))
#define  rEP89_PICL					(*(volatile unsigned *)(UDC_BASE+0x50C))
#define  rEP89_PICH					(*(volatile unsigned *)(UDC_BASE+0x510))
#define  rEP89_POCL					(*(volatile unsigned *)(UDC_BASE+0x514))
#define  rEP89_POCH					(*(volatile unsigned *)(UDC_BASE+0x518))
#define  rEP89_FIFO					(*(volatile unsigned *)(UDC_BASE+0x51C))
#define  rEP89_VB					(*(volatile unsigned *)(UDC_BASE+0x520))
#define  rEP89_SETTING				(*(volatile unsigned *)(UDC_BASE+0x52C))
#define  rEPAB_CTRL  				(*(volatile unsigned *)(UDC_BASE+0x550))
#define  rEPAB_PPC					(*(volatile unsigned *)(UDC_BASE+0x554))
#define  rEPAB_FS					(*(volatile unsigned *)(UDC_BASE+0x558))
#define  rEPAB_PICL					(*(volatile unsigned *)(UDC_BASE+0x55C))
#define  rEPAB_PICH					(*(volatile unsigned *)(UDC_BASE+0x560))
#define  rEPAB_POCL					(*(volatile unsigned *)(UDC_BASE+0x564))
#define  rEPAB_POCH					(*(volatile unsigned *)(UDC_BASE+0x568))
#define  rEPAB_FIFO					(*(volatile unsigned *)(UDC_BASE+0x56C))
#define  rEPAB_VB					(*(volatile unsigned *)(UDC_BASE+0x570))
#define  rEPAB_SETTING				(*(volatile unsigned *)(UDC_BASE+0x57C))

//==========================================================
#define SYS_IVAN_BASE	0xD0000000
#define	rSYS_CTRL_NEW			 	(*(volatile unsigned *)(SYS_IVAN_BASE+0x08))
#endif  //__UDC_S220_H__
