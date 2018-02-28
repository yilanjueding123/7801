#ifndef __STORAGE_GPLIB_H__
#define __STORAGE_GPLIB_H__

#include "gplib.h"

//#define  C_CARD_DECTECTION_PIN  64

//Embeded memory
#define  C_SPI_FLASH    0
#define  C_NAND_FLASH   1
#define  C_NOR_FLASH    2

#if (defined RESOURCE_DEV_TYPE) && (RESOURCE_DEV_TYPE==DEVICE_SPI)
    #define  C_NV_FLASH_TYPE C_SPI_FLASH
#elif (defined RESOURCE_DEV_TYPE) && (RESOURCE_DEV_TYPE==DEVICE_NAND)
    #define  C_NV_FLASH_TYPE C_NAND_FLASH
#elif (defined RESOURCE_DEV_TYPE) && (RESOURCE_DEV_TYPE==C_NOR_FLASH)
    #define  C_NV_FLASH_TYPE C_NOR_FLASH
#else
    #define  C_NV_FLASH_TYPE C_SPI_FLASH  /* Module Default Device Config */
#endif

//SPI USER AREA
#if (C_NV_FLASH_TYPE == C_SPI_FLASH)
	#define USER_SPI_BLK 1
	#define USER_SPI_256_B   1
	#define USER_SPI_512_B   2
	#define USER_SPI_AREA_SIZE USER_SPI_512_B
#endif	



//#define C_EMBEDED_STORAGE_SPI   1
//#define C_EMBEDED_STORAGE_NAND  1

#if (defined NVRAM_MANAGER_FORMAT) && (NVRAM_MANAGER_FORMAT >= NVRAM_V2)
#ifndef _FLASHRSINDEX_DEF_
#define _FLASHRSINDEX_DEF_
typedef struct 
{
    //INT8U  tag;     // none
    INT8U  name[19]; //FILE NAME
    INT32U pos;     //Phyical address
    INT32U offset;  //ADD
    INT32U size;    //by sector
} FLASHRSINDEX;
#endif

#else
typedef struct 
{
    //INT8U  tag;     // none
    INT8U  name[8]; //FILE NAME
    INT32U pos;     //Phyical address
    INT32U offset;  //ADD
    INT32U size;    //by sector
} FLASHRSINDEX;
#endif

extern INT16U nv_open(INT8U *path);                                 
extern INT32U nv_read(INT16S fd, INT32U buf, INT32U size);          
extern INT32U nv_lseek(INT16S fd, INT32S foffset, INT16S origin);   
//extern INT32U nv_fast_read(INT16S fd, INT32U buf, INT32U byte_size);
extern INT32U nv_rs_size_get(INT16S fd); // jandy add to querry fd size
extern INT32U combin_reg_data(INT8U *data, INT32S len);

#endif