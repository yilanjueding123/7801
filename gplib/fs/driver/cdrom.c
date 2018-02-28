#define		CREAT_DRIVERLAYER_STRUCT

#include "fsystem.h"

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#if (defined CDROM_EN) && (CDROM_EN == 1)                     //
//================================================================//

INT16S FD_CDROM;

INT32S CDROM_Initial(void)
{
	 
	 FD_CDROM=nv_open((INT8U *) "CD.ISO");
  	 if(FD_CDROM == 0xFFFF)
  	 {
  	 	return 0xffff;
  	 }
  	 else 
  	 {
  	 	return 0;
  	 }
	 
}

INT32S CDROM_Uninitial(void)
{
	 return 0;
}

void CDROM_GetDrvInfo(struct DrvInfo *info)
{
	info->nSectors = 0x8000;
	info->nBytesPerSector = 512;
}

INT32S CDROM_ReadSector(INT32U blkno, INT32U blkcnt, INT32U buf)
{
	//memcpy((void *) buf, gRamDisk + (blkno << 9), 512);
	nv_lseek((INT16S)FD_CDROM,blkno*512,SEEK_SET);
  	nv_read((INT16S)FD_CDROM, buf, 512*blkcnt);
    return 0;    
}

INT32S CDROM_WriteSector(INT32U blkno, INT32U blkcnt, INT32U buf)
{
	//memcpy(gRamDisk + (blkno << 9), (void *) buf, 512);
    return 0;
}

INT32S CDROM_Flush(void)
{
	return 0;
}

struct Drv_FileSystem FS_CDROM_driver = {
	"CDROM",
	DEVICE_READ_ALLOW,
	CDROM_Initial,
	CDROM_Uninitial,
	CDROM_GetDrvInfo,
	CDROM_ReadSector,
	CDROM_WriteSector,
	CDROM_Flush,
};

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#endif //(defined CDROM_EN) && (CDROM_EN == 1)                //
//================================================================//