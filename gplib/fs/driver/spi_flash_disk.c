/*
* Purpose: SPI flash interface of File system 
*
* Author:
*
* Date: 2009/6/15
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 1.00
*
* History:
        0  V1.00 :
*/

#define CREAT_DRIVERLAYER_STRUCT

#include "fsystem.h"

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#if NOR_EN == 1                                                   //
//================================================================//

#if 0
INT16S g_nvram_fd;

INT32S spi_flash_disk_initial(void)
{	
	g_nvram_fd = nv_open((INT8U*)"PHOTO.BIN");
	if(g_nvram_fd >= 0)	return 0;
	return -1;
}

INT32S spi_flash_disk_uninitial(void)
{
	return 0;
}

void spi_flash_disk_get_drv_info(struct DrvInfo* info)
{
	info->nSectors = nv_rs_size_get(g_nvram_fd);
	info->nBytesPerSector = 512;
}

INT32S spi_flash_disk_read_sector(INT32U blkno , INT32U blkcnt ,  INT32U buf)
{
    INT16U  ret;

	nv_lseek(g_nvram_fd, blkno << 9, SEEK_SET);
	ret = nv_read(g_nvram_fd, buf, blkcnt << 9);
    return ret;
}

INT32S spi_flash_disk_write_sector(INT32U blkno , INT32U blkcnt ,  INT32U buf)
{
	return -1;
}

INT32S spi_flash_disk_flush(void)
{
	return 0;
}

#else


//#define SPIF_PART0_OFFSET			(0x200000 / 512)
//#define	C_SPIFDISK_SIZE				(0x400000-(SPIF_PART0_OFFSET*512))
#define BLOCK_SIZE					4096
#define BLOCK_ADDR_SHIFT			12

#define C_EMPTY						0
#define C_VALID						1
#define C_MODIFIED				    2

#define C_BUFFERSIZE				0x400		//4k
	
typedef struct 
{
	INT32U Flag;	
	INT32U BlkNo;			//the block number
	INT32U Buffer[C_BUFFERSIZE];
}SPI_FLASH_BUFFER_STRUCT;

SPI_FLASH_BUFFER_STRUCT  spi_flash_buffer_struct;
INT32U spi_flash_part0_offset;
INT32U spi_flash_size;

#if 0//_OPERATING_SYSTEM != _OS_NONE
OS_EVENT *gSPIF_sem = 0;
#endif

static void SPIF_SEM_INIT(void)
{
#if 0//_OPERATING_SYSTEM != _OS_NONE
	if(gSPIF_sem == 0)
	{
		gSPIF_sem = OSSemCreate(1);
	}
#endif
}

static void SPIF_LOCK(void)
{
#if 0//_OPERATING_SYSTEM != _OS_NONE
	INT8U err = NULL;
	
	OSSemPend(gSPIF_sem, 0, &err);
#endif
}

static void SPIF_UNLOCK(void)
{
#if 0//_OPERATING_SYSTEM != _OS_NONE
	OSSemPost(gSPIF_sem);
#endif
}

// read 4kB
INT32S driver_l2_SPIF_read(INT32U blkno, INT8U *buf)
{
	return SPI_Flash_read(blkno << BLOCK_ADDR_SHIFT,(INT8U *)buf, BLOCK_SIZE);
}

//=======================================================================
// WRITE 4kB
//=======================================================================
INT32S driver_l2_SPIF_write(INT32U blkno, INT8U *buf)
{
	INT32U addr;
	INT32U i;
	INT32S ret;

	addr = blkno << BLOCK_ADDR_SHIFT;
	ret = SPI_Flash_erase_sector(addr);
	if(ret)
	{
		return -1;
	}
	for(i = 0; i < BLOCK_SIZE / 256; i++)
	{
		ret = SPI_Flash_write_page(addr, buf);
		if(ret)
		{
			return -2;
		}
		addr += 256;
		buf += 256;
	}
	return 0;
}

INT32S spi_flash_buffer_flush(void)
{
	INT32S ret;

	if(spi_flash_buffer_struct.Flag == (C_VALID + C_MODIFIED))
	{
		ret = driver_l2_SPIF_write(spi_flash_buffer_struct.BlkNo, (INT8U*)spi_flash_buffer_struct.Buffer);
		if(ret != 0)
		{
			return ret;
		}
	}
	spi_flash_buffer_struct.Flag = C_EMPTY;
	return 0;
} 

INT32S spi_flash_disk_read_sector(INT32U blkno, INT32U blkcnt, INT32U buf)
{
	INT32S	ret = 0;
	INT32U	blk;
	INT16U	cnt;
	DMA_STRUCT str_dma;
	
	if(blkno + blkcnt >= spi_flash_size - spi_flash_part0_offset)
	{
		DBG_PRINT("\t\t++++++++error+++++++++\r\n");
	}
	
	SPIF_LOCK();
	blkno += spi_flash_part0_offset;
	while(blkcnt)
	{
		blk = blkno >> 3;
		if((blkno & 0x7) || (blkcnt < 8))
		{
			if(!((spi_flash_buffer_struct.Flag & C_VALID) && (spi_flash_buffer_struct.BlkNo == blk) ))
			{
				ret = spi_flash_buffer_flush();
				if(ret != 0)
				{
					SPIF_UNLOCK();
					return ret;
				}
				
				ret = driver_l2_SPIF_read(blk, (INT8U *)spi_flash_buffer_struct.Buffer); //这里来调用读取4kbyte data read
				if(ret != 0)
				{
					SPIF_UNLOCK();
					return ret;
				}
				spi_flash_buffer_struct.Flag = C_VALID;
				spi_flash_buffer_struct.BlkNo = blk;
			}
	    
			if((blkno & 7) + blkcnt < 8)
	    		cnt = blkcnt;
	    	else
	    		cnt = 8 - (blkno & 7);
	    	
		
			str_dma.s_addr = (INT32U)(spi_flash_buffer_struct.Buffer + ((blkno & 0x7) << 7));
			str_dma.t_addr = (INT32U)buf;
			str_dma.count =  cnt << 9;
			str_dma.width = 1;
			str_dma.timeout = 0;
			str_dma.notify = NULL;
			ret = dma_transfer_wait_ready(&str_dma);
			if(ret != 0)
			{
				return ret;
			}
		
			//gp_memcpy((INT8S *) buf,(INT8S *)(spi_flash_buffer_struct.Buffer + ((blkno & 7) << 7)), blkcnt << 9);
			
			blkcnt -= cnt;
	    	blkno += cnt;
	    	buf += (cnt << 9);
		}
		else
		{
			cnt = blkcnt >> 3;
			while(cnt)
			{
				ret = driver_l2_SPIF_read(blk, (INT8U*)buf);
				if(ret != 0)
				{
					SPIF_UNLOCK();
					return ret;
				}
				blk += 1;
				cnt -= 1;
				buf += BLOCK_SIZE;
				blkcnt -= BLOCK_SIZE / 512;
				blkno += BLOCK_SIZE / 512;
			}
		}
	}

	SPIF_UNLOCK();
	return ret;
}

INT32S spi_flash_disk_write_sector(INT32U blkno, INT32U blkcnt, INT32U buf)
{
	INT16S	ret = 0;
	INT32U	blk;
	INT16U	cnt;
	DMA_STRUCT str_dma;

	SPIF_LOCK();
	blkno += spi_flash_part0_offset;
	while(blkcnt)
	{
		blk = blkno >> 3;
		if((blkno & 0x7) || (blkcnt < 8))
		{
			if(!((spi_flash_buffer_struct.Flag & C_VALID) && (spi_flash_buffer_struct.BlkNo == blk)))
			{
				ret = spi_flash_buffer_flush();
				if(ret != 0)
				{
					SPIF_UNLOCK();
					return ret;
				}
				ret = driver_l2_SPIF_read(blk, (INT8U *)spi_flash_buffer_struct.Buffer);
				if(ret != 0)
				{
					SPIF_UNLOCK();
					return ret;
				}
				spi_flash_buffer_struct.Flag = C_VALID;
				spi_flash_buffer_struct.BlkNo = blk;
			}	    	
			
			if((blkno & 7) + blkcnt < 8)
	    		cnt = blkcnt;
	    	else
	    		cnt = 8 - (blkno & 7);

		
			str_dma.s_addr = (INT32U)buf;
			str_dma.t_addr = (INT32U)(spi_flash_buffer_struct.Buffer + ((blkno & 0x7) << 7));
			str_dma.count =  cnt << 9;
			str_dma.width = 1;
			str_dma.timeout = 0;
			str_dma.notify = NULL;
			ret = dma_transfer_wait_ready(&str_dma);
			if(ret != 0)
			{
				return ret;
			}
		
			//gp_memcpy((INT8S *)(spi_flash_buffer_struct.Buffer + ((blkno & 7) << 7)),(INT8S *) buf, cnt << 9);
			spi_flash_buffer_struct.Flag |= C_MODIFIED;

			blkcnt -= cnt;
	    	blkno += cnt;
	    	buf += (cnt << 9);
		}
		else
		{
			cnt = blkcnt >> 3;
			while(cnt)
			{
				ret = driver_l2_SPIF_write(blk, (INT8U*)buf);
				if(ret != 0)
				{
					SPIF_UNLOCK();
					return ret;
				}
				blk += 1;
				cnt -= 1;
				buf += BLOCK_SIZE;
				blkcnt -= BLOCK_SIZE / 512;
				blkno += BLOCK_SIZE / 512;
			}
		}
	}
	SPIF_UNLOCK();
    return ret;
}

void spi_flash_buffer_init(void)
{
	spi_flash_buffer_struct.Flag = C_EMPTY;
	spi_flash_buffer_struct.BlkNo = 0;
}

INT32S spi_flash_disk_initial(void)
{
	INT32S	ret;
	INT8U	manID;
	INT32U	buffer[32 / 4];
	
	ret = SPI_Flash_init();
	if(ret != STATUS_OK)
	{
		return ret;
	}
	
	spi_clk_set(0, SYSCLK_128); /* spi clock = 48M/8/128 = 46.8k */
	SPI_Flash_readID((INT8U*)buffer);
	manID = (INT8U)buffer[0];
	if (manID == 0xd2) {
	}
	else if (manID == 0xc2) {
		spi_clk_set(0,SYSCLK_4);
	}
	else {
		spi_clk_set(0,SYSCLK_4);
	}

	ret = SPI_Flash_read(0, (INT8U*)buffer, 32);
	if(ret != STATUS_OK)
	{
		return ret;
	}
	
	spi_flash_part0_offset = (INT32U)((INT8U*)buffer)[0x17] * 65536 / 512;
	spi_flash_size = (INT32U)((INT8U*)buffer)[0x15] * 65536 / 512;
	if((spi_flash_part0_offset == 0) || (spi_flash_size == 0))
	{
		return STATUS_FAIL;
	}
	spi_flash_buffer_init();
	SPIF_SEM_INIT();
	return STATUS_OK;
}

INT32S spi_flash_disk_uninitial(void)
{
	return 0;
}

void spi_flash_disk_get_drv_info(struct DrvInfo *info)
{
	info->nSectors = spi_flash_size - spi_flash_part0_offset - 128 * 5;//C_SPIFDISK_SIZE / 512;//0x8000;
	info->nBytesPerSector = 512;
}

INT32S spi_flash_disk_flush(void)
{
	INT32S	ret;
	
	SPIF_LOCK();
	ret = spi_flash_buffer_flush();
	if(ret != 0)
	{
		DBG_PRINT("\t\t====spi flush fail!\r\n");
	}
	SPIF_UNLOCK();
	return ret;
}

#endif

struct Drv_FileSystem FS_SPI_FLASH_DISK_driver = {
	"SPINOR" ,
	DEVICE_READ_ALLOW|DEVICE_WRITE_ALLOW ,
	spi_flash_disk_initial ,
	spi_flash_disk_uninitial ,
	spi_flash_disk_get_drv_info ,
	spi_flash_disk_read_sector ,
	spi_flash_disk_write_sector ,
	spi_flash_disk_flush ,
};

//=== This is for code configuration DON'T REMOVE or MODIFY it ===//
#endif //((NOR_EN == 1)                                           //
//================================================================//