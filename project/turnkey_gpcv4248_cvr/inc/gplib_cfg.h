#ifndef __GPLIB_CFG_H__
#define __GPLIB_CFG_H__

    // Memory management module
    #define GPLIB_MEMORY_MANAGEMENT_EN		1
		#define C_MM_IRAM_LINK_NUM			20
		#define C_MM_16BYTE_NUM				40
		#define C_MM_64BYTE_NUM				16
		#define C_MM_256BYTE_NUM			16
		#define C_MM_1024BYTE_NUM			8
		#define C_MM_4096BYTE_NUM			8

    // File system module
    #define	GPLIB_FILE_SYSTEM_EN			1

    // JPEG image encoding/decoding module
    #define GPLIB_JPEG_ENCODE_EN			1
    #define GPLIB_JPEG_DECODE_EN			1

    // SPI flash read/write module
    #define GPLIB_NVRAM_SPI_FLASH_EN        1

    // Print and get string
    #define GPLIB_PRINT_STRING_EN           1
    #define PRINT_BUF_SIZE                  512

    #define GPLIB_CALENDAR_EN               1

    #define SUPPORT_MIDI_LOAD_FROM_NVRAM   0

    #define SUPPORT_MIDI_READ_FROM_SDRAM    0
    
    #define SUPPORT_MIDI_PLAY_WITHOUT_TASK	0

  #ifndef _FS_DEFINE_
  #define _FS_DEFINE_
    #define FS_NAND1    0
    #define FS_NAND2    1
    #define FS_SD       2
    #define FS_RAMDISK  3
    #define FS_MS       4
    #define FS_CF       5
    #define FS_XD       6
    #define FS_USBH     7
    #define FS_NOR      8
    #define FS_CDROM_NF 9
    #define FS_DEV_MAX  3//9

/* DEV MODE */
    #define NAND1_EN          0
    #define NAND2_EN          0
    #define SD_EN             1
    #define RAMDISK_EN        0
	#define MSC_EN            0
	#define CFC_EN            0
    #define SD_DUAL_SUPPORT   0
    #define USB_NUM           0
    #define NOR_EN            0
    #define CDROM_EN          0
    #define MAX_DISK_NUM      (FS_DEV_MAX+1)
  #endif // _FS_DEFINE_

  #define NVRAM_V1    1
  #define NVRAM_V2    2
  #define NVRAM_V3    3
  #define NVRAM_MANAGER_FORMAT    NVRAM_V3

  #define SUPPORT_STG_SUPER_PLUGOUT     1   // Dominant add super plug 


#endif 		// __GPLIB_CFG_H__
