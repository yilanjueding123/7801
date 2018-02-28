#define		CREAT_DRIVERLAYER_STRUCT

#include "fsystem.h"

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#if (defined RAMDISK_EN) && (RAMDISK_EN == 1)                     //
//================================================================//

INT8U	*gRamDisk = NULL;

INT32S RAMDISK_Initial(void)
{
	if(gRamDisk == NULL) {
		gRamDisk = gp_malloc_align(C_RAMDISK_SIZE, 4);
	}

	if(gRamDisk != NULL)	return 0;
	else					return -1;
}

INT32S RAMDISK_Uninitial(void)
{
	if(gRamDisk != NULL) {
		gp_free(gRamDisk);
		gRamDisk = NULL;
	}
	return 0;
}

void RAMDISK_GetDrvInfo(struct DrvInfo *info)
{
	info->nSectors = C_RAMDISK_SIZE/512;
	info->nBytesPerSector = 512;
}

INT32S RAMDISK_ReadSector(INT32U blkno, INT32U blkcnt, INT32U buf)
{
	memcpy((void *) buf, gRamDisk + (blkno << 9), 512*blkcnt);
	return 0;
}

INT32S RAMDISK_WriteSector(INT32U blkno, INT32U blkcnt, INT32U buf)
{
	memcpy(gRamDisk + (blkno << 9), (void *) buf, 512*blkcnt);
	return 0;
}

INT32S RAMDISK_Flush(void)
{
	return 0;
}

struct Drv_FileSystem FS_RAMDISK_driver = {
	"RAMDISK",
	DEVICE_READ_ALLOW|DEVICE_WRITE_ALLOW,
	RAMDISK_Initial,
	RAMDISK_Uninitial,
	RAMDISK_GetDrvInfo,
	RAMDISK_ReadSector,
	RAMDISK_WriteSector,
	RAMDISK_Flush,
};

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#endif //(defined RAMDISK_EN) && (RAMDISK_EN == 1)                //
//================================================================//