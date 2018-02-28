/******************************************************
* drv_l2_usbd_uvc.h
*
* Purpose: USB l2 controller driver/interface
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
#ifndef DRV_L2_USBD_UVC_H
#define DRV_L2_USBD_UVC_H
#include "project.h"
#include "drv_l1_usbd.h"
#include "Customer.h"
/*********************************************************************
        Structure, enumeration and definition
**********************************************************************/
#define UVC_VERSION 0x0100

#if ( (SENSOR_WIDTH==1280)&&(SENSOR_HEIGHT==720) )
	#define JPG_WIDTH	1280
	#define JPG_HEIGHT	720
#else
	#define JPG_WIDTH   640
	#define JPG_HEIGHT  480
#endif

#define USB_UVC_VCIF_NUM    0 /* Video control interface */
#define USB_UVC_VSIF_NUM    1 /* Video streamming interface for EP5 */
#define FRAME_RATE_SETTING	30			// 15 or 30

#define AUDIO_SUPPORT		1 /* Support AUDIO */

#define USB_UVC_VCIF_NUM    0 /* Video control interface */
#define USB_UVC_VSIF_NUM    1 /* Video streamming interface for EP5 */
#define USB_UAC_VCIF_NUM    2 /* Audio streamming interface for EP7 */
#define USB_UAC_VSIF_NUM    3 /* Audio streamming interface for EP7 */

#if (AUDIO_SUPPORT == 0)
#define USB_TOTAL_IF_NUM	2
#else
#define USB_TOTAL_IF_NUM	4
#endif

#define WBVAL(x) ((x) & 0xFF),(((x) >> 8) & 0xFF)
#define B3VAL(x) (x & 0xFF),((x >> 8) & 0xFF),((x >> 16) & 0xFF)
#define DBVAL(x) ((x) & 0xFF),(((x) >> 8) & 0xFF),(((x) >> 16) & 0xFF),(((x) >> 24) & 0xFF)

#define EP0_BUF_SIZE	64

typedef void (*ISO_FRAME_TRANSFER_DONE)(void);

/* Standard descriptor declarations */
__packed typedef struct _USB_DEVICE_DESCRIPTOR
{
  INT8U  bLength;
  INT8U  bDescriptorType;
  INT16U bcdUSB;
  INT8U  bDeviceClass;
  INT8U  bDeviceSubClass;
  INT8U  bDeviceProtocol;
  INT8U  bMaxPacketSize0;
  INT16U idVendor;
  INT16U idProduct;
  INT16U bcdDevice;
  INT8U  iManufacturer;
  INT8U  iProduct;
  INT8U  iSerialNumber;
  INT8U  bNumConfigurations;
} USB_DEVICE_DESCRIPTOR;

/* USB 2.0 Device Qualifier Descriptor */
__packed typedef struct _USB_DEVICE_QUALIFIER_DESCRIPTOR
{
  INT8U  bLength;
  INT8U  bDescriptorType;
  INT16U bcdUSB;
  INT8U  bDeviceClass;
  INT8U  bDeviceSubClass;
  INT8U  bDeviceProtocol;
  INT8U  bMaxPacketSize0;
  INT8U  bNumConfigurations;
  INT8U  bReserved;
} USB_DEVICE_QUALIFIER_DESCRIPTOR;

/* USB Standard Configuration Descriptor */
__packed typedef struct _USB_CONFIGURATION_DESCRIPTOR
{
  INT8U  bLength;
  INT8U  bDescriptorType;
  INT16U wTotalLength;
  INT8U  bNumInterfaces;
  INT8U  bConfigurationValue;
  INT8U  iConfiguration;
  INT8U  bmAttributes;
  INT8U  bMaxPower;
} USB_CONFIGURATION_DESCRIPTOR;

/* USB Standard Interface Descriptor */
__packed typedef struct _USB_INTERFACE_DESCRIPTOR
{
  INT8U  bLength;
  INT8U  bDescriptorType;
  INT8U  bInterfaceNumber;
  INT8U  bAlternateSetting;
  INT8U  bNumEndpoints;
  INT8U  bInterfaceClass;
  INT8U  bInterfaceSubClass;
  INT8U  bInterfaceProtocol;
  INT8U  iInterface;
} USB_INTERFACE_DESCRIPTOR;

/* USB Standard Endpoint Descriptor */
__packed typedef struct _USB_ENDPOINT_DESCRIPTOR
{
  INT8U  bLength;
  INT8U  bDescriptorType;
  INT8U  bEndpointAddress;
  INT8U  bmAttributes;
  INT16U wMaxPacketSize;
  INT8U  bInterval;
} USB_ENDPOINT_DESCRIPTOR;

/* USB String Descriptor */
__packed typedef struct _USB_STRING_DESCRIPTOR
{
  INT8U  bLength;
  INT8U  bDescriptorType;
  INT16U bString/*[]*/;
} USB_STRING_DESCRIPTOR;

/* USB Common Descriptor */
__packed typedef struct _USB_COMMON_DESCRIPTOR
{
  INT8U  bLength;
  INT8U  bDescriptorType;
} USB_COMMON_DESCRIPTOR;


/* UVC class descriptor declarations */
__packed typedef struct _UVC_INTERFACE_ASSOCIATION_DESCRIPTOR
{
  INT8U  bLength;                            // size of this descriptor in bytes: 8
  INT8U  bDescriptorType;                    // INTERFACE ASSOCIATION Descriptor Type
  INT8U  bFirstInterface;                    // interface number of first VideoControl interface
  INT8U  bInterfaceCount;                    // number of VideoStreaming interfaces
  INT8U  bFunctionClass;                     // Video Interface Class Code
  INT8U  bFunctionSubclass;                  // Video Interface Subclass Codes
  INT8U  bFunctionProtocol;                  // not used
  INT8U  iFunction;                          // index of a string descriptor describing this interface
} UVC_INTERFACE_ASSOCIATION_DESCRIPTOR;

__packed typedef struct _UVC_VC_INTERFACE_HEADER_DESCRIPTOR
{
  INT8U  bLength;                            // size of this descriptor in bytes: 12+n
  INT8U  bDescriptorType;                    // CS_INTERFACE descriptor type
  INT8U  bDescriptorSubtype;                 // VC_HEADER descriptor subtype
  INT16U bcdUVC;                             // USB CDC specification release version
  INT16U wTotalLength;                       // total number of bytes for VC IF (incl. header, unit, terminal descriptors)
  INT32U dwClockFrequency;                   // use of this has been deprecated
  INT8U  bInCollection;                      // number of VS interfaces for video interface collection
  INT8U  baInterfaceNr[1];                   // interface number of first VideoStreaming interface
} UVC_VC_INTERFACE_HEADER_DESCRIPTOR;

__packed typedef struct _UVC_CAMERA_TERMINAL_DESCRIPTOR
{
  INT8U  bLength;                            // size of this descriptor in bytes: 15+n
  INT8U  bDescriptorType;                    // CS_INTERFACE descriptor type
  INT8U  bDescriptorSubtype;                 // VC_HEADER descriptor subtype
  INT8U  bTerminalID;                        // ID to address this terminal
  INT16U wTerminalType;                      // type of terminal
  INT8U  bAssocTerminal;                     // ID of the associated output terminal
  INT8U  iTerminal;                          // string descriptor index describing the terminal
  INT16U wObjectiveFocalLengthMin;           // see USB_Video_Class_1.1.pdf, 2.4.2.5.1
  INT16U wObjectiveFocalLengthMax;           // see USB_Video_Class_1.1.pdf, 2.4.2.5.1
  INT16U wOcularFocalLengt;                  // see USB_Video_Class_1.1.pdf, 2.4.2.5.1
  INT8U  bControlSize;                       // size in bytes of the bmControls field
  INT8U  bmControls[1];                      // Bitmap for the supported controls
} UVC_CAMERA_TERMINAL_DESCRIPTOR;

__packed typedef struct _UVC_OUTPUT_TERMINAL_DESCRIPTOR
{
  INT8U  bLength;                            // size of this descriptor in bytes: 9(+n)
  INT8U  bDescriptorType;                    // CS_INTERFACE descriptor type
  INT8U  bDescriptorSubtype;                 // VC_HEADER descriptor subtype
  INT8U  bTerminalID;                        // ID to address this terminal
  INT16U wTerminalType;                      // type of terminal
  INT8U  bSourceID;                          // ID of the connected Unit or Terminal
  INT8U  bAssocTerminal;                     // ID of the associated output terminal
  INT8U  iTerminal;                          // string descriptor index describing the terminal                                            // additional fields depending on the terminal type
} UVC_OUTPUT_TERMINAL_DESCRIPTOR;

__packed typedef struct _UVC_SELECTOR_UNIT_DESCRIPTOR
{
  INT8U  bLength;                            // size of this descriptor in bytes: 6+p
  INT8U  bDescriptorType;                    // CS_INTERFACE descriptor type
  INT8U  bDescriptorSubtype;                 // VC_SELECTOR_UNIT descriptor subtype
  INT8U  bUnitID;                            // ID to address this unit
  INT8U  bNrInPins;                          // number of input pins for this unit: p
  INT8U  baSourceID[1];                      // number of Unit or terminal connected to this pin
  INT8U  iSelector;                          // string descriptor index describing the selector unit
} UVC_SELECTOR_UNIT_DESCRIPTOR;

__packed typedef struct _UVC_PROCESSING_UNIT_DESCRIPTOR
{
  INT8U  bLength;                            // size of this descriptor in bytes: 10+n
  INT8U  bDescriptorType;                    // CS_INTERFACE descriptor type
  INT8U  bDescriptorSubtype;                 // VC_PROCESSING_UNIT descriptor subtype
  INT8U  bUnitID;                            // ID to address this unit
  INT8U  bSourceID;                          // ID of the connected Unit or Terminal
  INT16U wMaxMultiplier;                     // 
  INT8U  bControlSize;                       // Size of the bmControl fields, in bytes: n
  INT8U  bmControls[2];                      // Bitmap for the supported controls
  INT8U  iProcessing;                        // string descriptor index describing the processing unit
#if (UVC_VERSION == 0x0110)
  INT8U  bmVideoStandards;                   // bitmap of supported analog video standards
#endif
} UVC_PROCESSING_UNIT_DESCRIPTOR;

__packed typedef struct _UVC_VC_ENDPOINT_DESCRIPTOR
{
  INT8U  bLength;                            // size of this descriptor in bytes: 5
  INT8U  bDescriptorType;                    // CS_ENDPOINT descriptor type
  INT8U  bDescriptorSubtype;                 // EP_INTERRUPT descriptor subtype
  INT16U wMaxTransferSize;                   // maximum structure size this EP is capable of sending
} UVC_VC_ENDPOINT_DESCRIPTOR;

__packed typedef struct _UVC_VS_INTERFACE_INPUT_HEADER_DESCRIPTOR
{
  INT8U  bLength;                            // size of this descriptor in bytes: 13+(p*n)
  INT8U  bDescriptorType;                    // CS_INTERFACE descriptor type
  INT8U  bDescriptorSubtype;                 // VC_INPUT_HEADER descriptor subtype
  INT8U  bNumFormats;                        // number of video payload format descriptors: p
  INT16U wTotalLength;                       // total number of bytes for VS IF (incl. header)
  INT8U  bEndpointAddress;                   // EP used for video data
  INT8U  bmInfo;                             // capabilities for this VideoStreaming interface
  INT8U  bTerminalLink;                      // ID of the connected Output Terminal
  INT8U  bStillCaptureMethod;                // method of supported still image capture
  INT8U  bTriggerSupport;                    // specifies if HW triggering is supported
  INT8U  bTriggerUsage;                      // How to repond to a HW trigger Interrupt
  INT8U  bControlSize;                       // size of each bmaConrtols field: n
  INT8U  bmaControls[1];                     // Bitmap
} UVC_VS_INTERFACE_INPUT_HEADER_DESCRIPTOR;

__packed typedef struct __UVC_VIDEO_PROBE_ANDCOMMIT_CONTROL
{
  INT16U  bmHint;                             // bitfield indicating what fields shall be kept fixed
  INT8U  bFormatIndex;                       // Video format index from a format descriptor
  INT8U  bFrameIndex;                        // Video frame index from a frame descriptor
  INT32U dwFrameInterval;                    // Frame intervall in 100 ns units
  INT16U  wKeyFrameRate;                      // Key frame rate in key-frame per video-frame units
  INT16U  wPFrameRate;                        // PFrame rate i PFrame/key frame units
  INT16U  wCompQuality;                       // Compression quality control in abstarct units (0..10000)
  INT16U  wCompWindowSize;                    // Window size for average bit rate control
  INT16U  wDelay;                             // int. VS interface latency in ms from capture to presentation
  INT32U dwMaxVideoFrameSize;                // maximum video frame or codec specific segemet size in bytes
  INT32U dwMaxPayloadTransferSize;           // max. bytes the device can transmit/receive in single payload transfer
#if (UVC_VERSION == 0x0110)
  INT32U dwClockFrequency;                   // device clock frequency in Hz for sepcified format
  INT8U  bmFraminInfo;                       // bitfield control
  INT8U  bPreferedVersion;                   // preferred payload format version for specifide bFormatIndex
  INT8U  bMinVersion;                        // minimum payload format version for specifide bFormatIndex
  INT8U  bMaxVersion;                        // maximum payload format version for specifide bFormatIndex
#endif   
} UVC_VIDEO_PROBE_AND_COMMIT_CONTROL; 


#define USB_DEVICE_DESC_SIZE        (sizeof(USB_DEVICE_DESCRIPTOR))
#define USB_CONFIGUARTION_DESC_SIZE (sizeof(USB_CONFIGURATION_DESCRIPTOR))
#define USB_INTERFACE_DESC_SIZE     (sizeof(USB_INTERFACE_DESCRIPTOR))
#define USB_ENDPOINT_DESC_SIZE      (sizeof(USB_ENDPOINT_DESCRIPTOR))

#define UVC_INTERFACE_ASSOCIATION_DESC_SIZE (sizeof(UVC_INTERFACE_ASSOCIATION_DESCRIPTOR))
#define UVC_VC_INTERFACE_HEADER_DESC_SIZE(n) (sizeof(UVC_VC_INTERFACE_HEADER_DESCRIPTOR)-1+n)
#define UVC_CAMERA_TERMINAL_DESC_SIZE(n) (sizeof(UVC_CAMERA_TERMINAL_DESCRIPTOR)-1+n)
#define UVC_OUTPUT_TERMINAL_DESC_SIZE(n) (sizeof(UVC_OUTPUT_TERMINAL_DESCRIPTOR)+n)
#define UVC_SELECTOR_UNIT_DESC_SIZE(p) (sizeof(UVC_SELECTOR_UNIT_DESCRIPTOR)-1+p)
#define UVC_PROCESSING_UNIT_DESC_SIZE(n) (sizeof(UVC_PROCESSING_UNIT_DESCRIPTOR)-2+n)
#define UVC_VC_ENDPOINT_DESC_SIZE (sizeof(UVC_VC_ENDPOINT_DESCRIPTOR))
#define UVC_VS_INTERFACE_INPUT_HEADER_DESC_SIZE(p,n) (sizeof(UVC_VS_INTERFACE_INPUT_HEADER_DESCRIPTOR)-1+(p*n))

/* bmAttributes in Configuration Descriptor */
#define USB_CONFIG_POWERED_MASK                0xC0
#define USB_CONFIG_BUS_POWERED                 0x80
#define USB_CONFIG_SELF_POWERED                0x40
#define USB_CONFIG_REMOTE_WAKEUP               0x20

/* USB Device Classes */
#define USB_DEVICE_CLASS_RESERVED              0x00
#define USB_DEVICE_CLASS_AUDIO                 0x01
#define USB_DEVICE_CLASS_COMMUNICATIONS        0x02
#define USB_DEVICE_CLASS_HUMAN_INTERFACE       0x03
#define USB_DEVICE_CLASS_MONITOR               0x04
#define USB_DEVICE_CLASS_PHYSICAL_INTERFACE    0x05
#define USB_DEVICE_CLASS_POWER                 0x06
#define USB_DEVICE_CLASS_PRINTER               0x07
#define USB_DEVICE_CLASS_STORAGE               0x08
#define USB_DEVICE_CLASS_HUB                   0x09
#define USB_DEVICE_CLASS_MISCELLANEOUS         0xEF
#define USB_DEVICE_CLASS_VENDOR_SPECIFIC       0xFF

/* USB Descriptor Types */
#define USB_DEVICE_DESCRIPTOR_TYPE                 1
#define USB_CONFIGURATION_DESCRIPTOR_TYPE          2
#define USB_STRING_DESCRIPTOR_TYPE                 3
#define USB_INTERFACE_DESCRIPTOR_TYPE              4
#define USB_ENDPOINT_DESCRIPTOR_TYPE               5
#define USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE       6
#define USB_OTHER_SPEED_CONFIG_DESCRIPTOR_TYPE     7
#define USB_INTERFACE_POWER_DESCRIPTOR_TYPE        8
#define USB_OTG_DESCRIPTOR_TYPE                    9
#define USB_DEBUG_DESCRIPTOR_TYPE                 10
#define USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE 11 

/* bmAttributes in Endpoint Descriptor */
#define USB_ENDPOINT_TYPE_MASK                 0x03
#define USB_ENDPOINT_TYPE_CONTROL              0x00
#define USB_ENDPOINT_TYPE_ISOCHRONOUS          0x01
#define USB_ENDPOINT_TYPE_BULK                 0x02
#define USB_ENDPOINT_TYPE_INTERRUPT            0x03
#define USB_ENDPOINT_SYNC_MASK                 0x0C
#define USB_ENDPOINT_SYNC_NO_SYNCHRONIZATION   0x00
#define USB_ENDPOINT_SYNC_ASYNCHRONOUS         0x04
#define USB_ENDPOINT_SYNC_ADAPTIVE             0x08
#define USB_ENDPOINT_SYNC_SYNCHRONOUS          0x0C
#define USB_ENDPOINT_USAGE_MASK                0x30
#define USB_ENDPOINT_USAGE_DATA                0x00
#define USB_ENDPOINT_USAGE_FEEDBACK            0x10
#define USB_ENDPOINT_USAGE_IMPLICIT_FEEDBACK   0x20
#define USB_ENDPOINT_USAGE_RESERVED            0x30

/* bMaxPower in Configuration Descriptor */
#define USB_CONFIG_POWER_MA(mA)                ((mA)/2)

/* bEndpointAddress in Endpoint Descriptor */
#define USB_ENDPOINT_OUT(addr)                 ((addr) | 0x00)
#define USB_ENDPOINT_IN(addr)                  ((addr) | 0x80)
 
/* Video Class-Specific Request Codes, (USB_Video_Class_1.1.pdf, A.8 Video Class-Specific Request Codes) */
#define RC_UNDEFINED                               0x00
#define SET_CUR                                    0x01
#define GET_CUR                                    0x81
#define GET_MIN                                    0x82
#define GET_MAX                                    0x83
#define GET_RES                                    0x84
#define GET_LEN                                    0x85
#define GET_INFO                                   0x86
#define GET_DEF                                    0x87
 
/* Request Error Code Control, (USB_Video_Class_1.1.pdf, 4.2.1.2 Request Error Code Control) */
#define NO_ERROR_ERR                               0x00
#define NOT_READY_ERR                              0x01
#define WRONG_STATE_ERR                            0x02
#define POWER_ERR                                  0x03
#define OUT_OF_RANGE_ERR                           0x04
#define INVALID_UNIT_ERR                           0x05
#define INVALID_CONTROL_ERR                        0x06
#define INVALID_REQUEST_ERR                        0x07
#define UNKNOWN_ERR                                0xFF
 
/* Defined Bits Containing Capabilities of the Control, (USB_Video_Class_1.1.pdf, 4.1.2 Table 4-3 Defined Bits Containing Capabilities of the Control) */
#define SUPPORTS_GET                 	             0x01
#define SUPPORTS_SET                               0x02
#define STATE_DISABLED                             0x04
#define AUTOUPDATE_CONTROL                         0x08
#define ASYNCHRONOUS_CONTROL                       0x10
 
/* Selector Unit Control Selectors, (USB_Video_Class_1.1.pdf, A.9.3 Selector Unit Control Selectors) */
#define SU_CONTROL_UNDEFINED                       0x00
#define SU_INPUT_SELECT_CONTROL                    0x01
 
/* VideoStreaming Interface Control Selectors, (USB_Video_Class_1.1.pdf, A.9.7 VideoStreaming Interface Control Selectors) */
#define VS_CONTROL_UNDEFINED             	         0x00
#define VS_PROBE_CONTROL                 	         0x01
#define VS_COMMIT_CONTROL                     	   0x02
#define VS_STILL_PROBE_CONTROL               	     0x03
#define VS_STILL_COMMIT_CONTROL                    0x04
#define VS_STILL_IMAGE_TRIGGER_CONTROL      	     0x05
#define VS_STREAM_ERROR_CODE_CONTROL       	       0x06
#define VS_GENERATE_KEY_FRAME_CONTROL     	       0x07
#define VS_UPDATE_FRAME_SEGMENT_CONTROL    	       0x08
#define VS_SYNC_DELAY_CONTROL  
 
/* Processing Unit Control Selectors, (USB_Video_Class_1.1.pdf, A.9.5 Processing Unit Control Selectors) */
#define PU_CONTROL_UNDEFINED            	   	     0x00
#define PU_BACKLIGHT_COMPENSATION_CONTROL          0x01
#define PU_BRIGHTNESS_CONTROL               	     0x02
#define PU_CONTRAST_CONTROL                 	     0x03
#define PU_GAIN_CONTROL                 	   	     0x04
#define PU_POWER_LINE_FREQUENCY_CONTROL 	   	     0x05
#define PU_HUE_CONTROL                  	   	     0x06
#define PU_SATURATION_CONTROL           	   	     0x07
#define PU_SHARPNESS_CONTROL            	   	     0x08
#define PU_GAMMA_CONTROL                	   	     0x09
#define PU_WHITE_BALANCE_TEMPERATURE_CONTROL       0x0A
#define PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL  0x0B
#define PU_WHITE_BALANCE_COMPONENT_CONTROL         0x0C
#define PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL    0x0D
#define PU_DIGITAL_MULTIPLIER_CONTROL   	         0x0E
#define PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL        0x0F
#define PU_HUE_AUTO_CONTROL             	         0x10
#define PU_ANALOG_VIDEO_STANDARD_CONTROL           0x11
#define PU_ANALOG_LOCK_STATUS_CONTROL   	         0x12
 
/* Bit Map in Processing Unit control descriptor, maximum size 3 bytes */
#define PU_BM_BRIGHTNESS							BIT0	/* Brightness */
#define PU_BM_CONTRAST								BIT1	/* Contrast */
#define PU_BM_HUE									BIT2	/* Hue */
#define PU_BM_SATURATION							BIT3	/* Saturation */
#define PU_BM_SHARPNESS								BIT4	/* Sharpness */
#define PU_BM_GAMMA									BIT5	/* Gamma */
#define PU_BM_WBTEMP								BIT6	/* White Balance Temperature */
#define PU_BM_WBCOM									BIT7	/* White Balance Component */
#define PU_BM_BLCOM									BIT8	/* Backlight Compensation */
#define PU_BM_GAIN									BIT9	/* Gain */ 
#define PU_BM_PLFREQ								BIT10	/* Power Line Frequency */ 
#define PU_BM_HUEAUTO								BIT11	/* Hue Auto */
#define PU_BM_WBTEMPAUTO							BIT12	/* White Balance Temperature Auto */
#define PU_BM_WBCOMAUTO								BIT13	/* White Balance Component Auto */
#define PU_BM_DMUL									BIT14	/* Digital Multiplier */ 
#define PU_BM_DMULLIM								BIT15	/* Digital Multiplier Limit */ 
#define PU_BM_ANAVSTD								BIT16	/* Analog Video Standard */ 
#define PU_BM_ANAVLOCK								BIT17	/* Analog Video Lock Status */ 
#define PU_BM_CONTRASTAUTO							BIT18	/* Contrast Auto */
 
/* VideoControl Interface Control Selectors, (USB_Video_Class_1.1.pdf, A.9.1 VideoControl Interface Control Selectors) */
#define VC_CONTROL_UNDEFINED                       0x00
#define VC_VIDEO_POWER_MODE_CONTROL                0x01
#define VC_REQUEST_ERROR_CODE_CONTROL              0x02

/* Video Interface Class Codes (USB_Video_Class_1.1.pdf, A.1 Video Interface Class Code) */
#define CC_VIDEO                                   0x0E
 
/* Video Interface Subclass Codes, (USB_Video_Class_1.1.pdf, A.2 Video Interface Subclass Code) */
#define SC_UNDEFINED                               0x00
#define SC_VIDEOCONTROL                            0x01
#define SC_VIDEOSTREAMING                          0x02
#define SC_VIDEO_INTERFACE_COLLECTION              0x03
 
/* Video Interface Protocol Codes, (USB_Video_Class_1.1.pdf, A.3 Video Interface Protocol Codes) */
#define PC_PROTOCOL_UNDEFINED                      0x00

/* Video Class-Specific Descriptor Types, (USB_Video_Class_1.1.pdf, A.4 Video Class-Specific Descriptor Types) */
#define CS_UNDEFINED                               0x20
#define CS_DEVICE                                  0x21
#define CS_CONFIGURATION                           0x22
#define CS_STRING                                  0x23
#define CS_INTERFACE                               0x24
#define CS_ENDPOINT                                0x25
 
/* Video Class-Specific VideoControl Interface Descriptor Subtypes, (USB_Video_Class_1.1.pdf, A.5 Video Class-Specific VC Interface Descriptor Subtypes) */
#define VC_DESCRIPTOR_UNDEFINED                    0x00
#define VC_HEADER                                  0x01
#define VC_INPUT_TERMINAL                          0x02
#define VC_OUTPUT_TERMINAL                         0x03
#define VC_SELECTOR_UNIT                           0x04
#define VC_PROCESSING_UNIT                         0x05
#define VC_EXTENSION_UNIT                          0x06
 
/* Input Terminal Types, (USB_Video_Class_1.1.pdf, B.2 Input Terminal Types) */
#define ITT_VENDOR_SPECIFIC                      0x0200
#define ITT_CAMERA                               0x0201
#define ITT_MEDIA_TRANSPORT_INPUT                0x0202
 
/* USB Terminal Types, (USB_Video_Class_1.1.pdf, B.1 USB Terminal Types) */
#define TT_VENDOR_SPECIFIC         	             0x0100
#define TT_STREAMING               	             0x0101

/* Video Class-Specific Endpoint Descriptor Subtypes, (USB_Video_Class_1.1.pdf, A.7 Video Class-Specific Endpoint Descriptor Subtypes) */
#define EP_UNDEFINED                               0x00
#define EP_GENERAL                                 0x01
#define EP_ENDPOINT                                0x02
#define EP_INTERRUPT                               0x03

// Video Class-Specific VideoStreaming Interface Descriptor Subtypes, (USB_Video_Class_1.1.pdf, A.6 Video Class-Specific VS Interface Descriptor Subtypes) */
#define VS_UNDEFINED                               0x00
#define VS_INPUT_HEADER                            0x01
#define VS_OUTPUT_HEADER                           0x02
#define VS_STILL_IMAGE_FRAME                       0x03
#define VS_FORMAT_UNCOMPRESSED                     0x04
#define VS_FRAME_UNCOMPRESSED                      0x05
#define VS_FORMAT_MJPEG                            0x06
#define VS_FRAME_MJPEG                             0x07
#define VS_FORMAT_MPEG2TS                          0x0A
#define VS_FORMAT_DV                               0x0C
#define VS_COLORFORMAT                             0x0D
#define VS_FORMAT_FRAME_BASED                      0x10
#define VS_FRAME_FRAME_BASED                       0x11
#define VS_FORMAT_STREAM_BASED                     0x12

/* Audio class */
/* Audio Descriptor Sizes */
#define AUDIO_CONTROL_INTERFACE_DESC_SZ(n)      (0x08+n)
#define AUDIO_STREAMING_INTERFACE_DESC_SIZE     0x07
#define AUDIO_INPUT_TERMINAL_DESC_SIZE          0x0C
#define AUDIO_OUTPUT_TERMINAL_DESC_SIZE         0x09
#define AUDIO_MIXER_UNIT_DESC_SZ(p,n)           (0x0A+p+n)
#define AUDIO_SELECTOR_UNIT_DESC_SZ(p)          (0x06+p)
#define AUDIO_FEATURE_UNIT_DESC_SZ(ch,n)        (0x07+(ch+1)*n)
#define AUDIO_PROCESSING_UNIT_DESC_SZ(p,n,x)    (0x0D+p+n+x)
#define AUDIO_EXTENSION_UNIT_DESC_SZ(p,n)       (0x0D+p+n)
#define AUDIO_STANDARD_ENDPOINT_DESC_SIZE       0x09
#define AUDIO_STREAMING_ENDPOINT_DESC_SIZE      0x07

/* Audio Format Type Descriptor Sizes */
#define AUDIO_FORMAT_TYPE_I_DESC_SZ(n)          (0x08+(n*3))
#define AUDIO_FORMAT_TYPE_II_DESC_SZ(n)         (0x09+(n*3))
#define AUDIO_FORMAT_TYPE_III_DESC_SZ(n)        (0x08+(n*3))
#define AUDIO_FORMAT_MPEG_DESC_SIZE             0x09
#define AUDIO_FORMAT_AC3_DESC_SIZE              0x0A

/* Audio Control Interface Descriptor Subtypes */
#define AUDIO_CONTROL_UNDEFINED                 0x00
#define AUDIO_CONTROL_HEADER                    0x01
#define AUDIO_CONTROL_INPUT_TERMINAL            0x02
#define AUDIO_CONTROL_OUTPUT_TERMINAL           0x03
#define AUDIO_CONTROL_MIXER_UNIT                0x04
#define AUDIO_CONTROL_SELECTOR_UNIT             0x05
#define AUDIO_CONTROL_FEATURE_UNIT              0x06
#define AUDIO_CONTROL_PROCESSING_UNIT           0x07
#define AUDIO_CONTROL_EXTENSION_UNIT            0x08

/*  Input Terminal Types */
#define AUDIO_TERMINAL_INPUT_UNDEFINED          0x0200
#define AUDIO_TERMINAL_MICROPHONE               0x0201
#define AUDIO_TERMINAL_DESKTOP_MICROPHONE       0x0202
#define AUDIO_TERMINAL_PERSONAL_MICROPHONE      0x0203
#define AUDIO_TERMINAL_OMNI_DIR_MICROPHONE      0x0204
#define AUDIO_TERMINAL_MICROPHONE_ARRAY         0x0205
#define AUDIO_TERMINAL_PROCESSING_MIC_ARRAY     0x0206

/* Predefined Audio Channel Configuration Bits */
#define AUDIO_CHANNEL_M                         0x0000  /* Mono */
#define AUDIO_CHANNEL_L                         0x0001  /* Left Front */
#define AUDIO_CHANNEL_R                         0x0002  /* Right Front */
#define AUDIO_CHANNEL_C                         0x0004  /* Center Front */
#define AUDIO_CHANNEL_LFE                       0x0008  /* Low Freq. Enhance. */
#define AUDIO_CHANNEL_LS                        0x0010  /* Left Surround */
#define AUDIO_CHANNEL_RS                        0x0020  /* Right Surround */
#define AUDIO_CHANNEL_LC                        0x0040  /* Left of Center */
#define AUDIO_CHANNEL_RC                        0x0080  /* Right of Center */
#define AUDIO_CHANNEL_S                         0x0100  /* Surround */
#define AUDIO_CHANNEL_SL                        0x0200  /* Side Left */
#define AUDIO_CHANNEL_SR                        0x0400  /* Side Right */
#define AUDIO_CHANNEL_T                         0x0800  /* Top */

/* Feature Unit Control Bits */
#define AUDIO_CONTROL_MUTE                      0x0001
#define AUDIO_CONTROL_VOLUME                    0x0002
#define AUDIO_CONTROL_BASS                      0x0004
#define AUDIO_CONTROL_MID                       0x0008
#define AUDIO_CONTROL_TREBLE                    0x0010
#define AUDIO_CONTROL_GRAPHIC_EQUALIZER         0x0020
#define AUDIO_CONTROL_AUTOMATIC_GAIN            0x0040
#define AUDIO_CONTROL_DEALY                     0x0080
#define AUDIO_CONTROL_BASS_BOOST                0x0100
#define AUDIO_CONTROL_LOUDNESS                  0x0200

/*  USB Terminal Types */
#define AUDIO_TERMINAL_USB_UNDEFINED            0x0100
#define AUDIO_TERMINAL_USB_STREAMING            0x0101
#define AUDIO_TERMINAL_USB_VENDOR_SPECIFIC      0x01FF

/* Audio Interface Subclass Codes */
#define AUDIO_SUBCLASS_UNDEFINED                0x00
#define AUDIO_SUBCLASS_AUDIOCONTROL             0x01
#define AUDIO_SUBCLASS_AUDIOSTREAMING           0x02
#define AUDIO_SUBCLASS_MIDISTREAMING            0x03

/* Audio Interface Protocol Codes */
#define AUDIO_PROTOCOL_UNDEFINED                0x00

/* Audio Streaming Interface Descriptor Subtypes */
#define AUDIO_STREAMING_UNDEFINED               0x00
#define AUDIO_STREAMING_GENERAL                 0x01
#define AUDIO_STREAMING_FORMAT_TYPE             0x02
#define AUDIO_STREAMING_FORMAT_SPECIFIC         0x03

/*  Audio Data Format Type I Codes */
#define AUDIO_FORMAT_TYPE_I_UNDEFINED           0x0000
#define AUDIO_FORMAT_PCM                        0x0001
#define AUDIO_FORMAT_PCM8                       0x0002
#define AUDIO_FORMAT_IEEE_FLOAT                 0x0003
#define AUDIO_FORMAT_ALAW                       0x0004
#define AUDIO_FORMAT_MULAW                      0x0005

/* Audio Descriptor Types */
#define AUDIO_UNDEFINED_DESCRIPTOR_TYPE         0x20
#define AUDIO_DEVICE_DESCRIPTOR_TYPE            0x21
#define AUDIO_CONFIGURATION_DESCRIPTOR_TYPE     0x22
#define AUDIO_STRING_DESCRIPTOR_TYPE            0x23
#define AUDIO_INTERFACE_DESCRIPTOR_TYPE         0x24
#define AUDIO_ENDPOINT_DESCRIPTOR_TYPE          0x25

/* Audio Format Types */
#define AUDIO_FORMAT_TYPE_UNDEFINED             0x00
#define AUDIO_FORMAT_TYPE_I                     0x01
#define AUDIO_FORMAT_TYPE_II                    0x02
#define AUDIO_FORMAT_TYPE_III                   0x03

/* Audio Endpoint Descriptor Subtypes */
#define AUDIO_ENDPOINT_UNDEFINED                0x00
#define AUDIO_ENDPOINT_GENERAL                  0x01

/*  Feature Unit Control Selectors */
#define AUDIO_MUTE_CONTROL                      0x01
#define AUDIO_VOLUME_CONTROL                    0x02
#define AUDIO_BASS_CONTROL                      0x03
#define AUDIO_MID_CONTROL                       0x04
#define AUDIO_TREBLE_CONTROL                    0x05
#define AUDIO_GRAPHIC_EQUALIZER_CONTROL         0x06
#define AUDIO_AUTOMATIC_GAIN_CONTROL            0x07
#define AUDIO_DELAY_CONTROL                     0x08
#define AUDIO_BASS_BOOST_CONTROL                0x09
#define AUDIO_LOUDNESS_CONTROL                  0x0A

/************** USBD ISO API *************************/
extern INT32U drv_l2_usbd_uvc_check_ep5_flush(void);
extern INT32U drv_l2_usbd_uvc_check_ep7_flush(void);

#endif  //DRV_L2_USBD_UVC_H
