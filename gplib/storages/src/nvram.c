/*
* Purpose: NV RAM access
*
* Author: wschung
*
* Date: 2008/06/03
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 2.01
* History :
*         1 2008/06/06 Created by wschung.  v1.00
*         2 2009/01/13 Using 19 bytes as resoure file name and 4 bytes as file sector count   V2.00
*         3 2009/01/21 Adding NVRAM_V3 for Nand flash APP area supporting
*/
#include "storages.h"

//=== This is for code configuration DON'T REMOVE or MODIFY it ===========//
#if (defined GPLIB_NVRAM_SPI_FLASH_EN) && (GPLIB_NVRAM_SPI_FLASH_EN == 1) //
//========================================================================//

#ifndef NVRAM_MANAGER_FORMAT
#define NVRAM_V1    1
#define NVRAM_V2    2
#define NVRAM_V3    3
#define NVRAM_MANAGER_FORMAT NVRAM_V3
#endif



//Identification of each area
#define SPI_FLASH_TAG   "GPNV"
#define NAND_FLASH_TAG  "PGandnandn"
#define APP_AREA_TAG    "GPAP"
#define RESOURCE_TAG    "GP"

// SPI flash controller
#include "drv_l2_spifc.h"
#define SPIFC_RES 1		// using SPI controller
static spifc_apis_ops* pSpifc_apis_ops;

#define BootToAPP_HEADER_OFFSET_SEC_CNT       136
#define APPToResource_HEADER_OFFSET_SEC_CNT       20
#define HEADER_BCH_BLK_CNT         16
#define HEADER_BOOT_BACKUP_CNT     19

//#define RS_TABLE_SIZE               200//83

#define BOOT_AREA_START			0
#define BOOT_AREA_SIZE			10
#define PHYSIC_BLOCK_START		(BOOT_AREA_START + BOOT_AREA_SIZE)


#if NVRAM_MANAGER_FORMAT == NVRAM_V3
extern INT32U NandBootWriteSector(INT32U wWriteLBA, INT16U wLen, INT32U DataBufAddr); 
extern INT32U NandBootReadSector(INT32U wReadLBA, INT16U wLen, INT32U DataBufAddr);
extern INT32U NandBootFlush(void);
#endif

#if (defined GPLIB_FILE_SYSTEM_EN) && (GPLIB_FILE_SYSTEM_EN == 1) && _OPERATING_SYSTEM == _OS_UCOS2 // jandy add 
    extern OS_EVENT *gFS_sem;
    #define _NVNAND_OS_     1
#elif _OPERATING_SYSTEM == _OS_UCOS2
    OS_EVENT *gFS_sem;
    #define _NVNAND_OS_     1
#else
    #define _NVNAND_OS_     0
#endif

INT32U combin_reg_data(INT8U *data, INT32S len);
//NV_FS_HANDLE NVFILEHANDE[5];
//resource

INT32U APP_area_position;
INT32U Resource_area_position;
INT32U Resource_area_size;
INT32U User_area_position;
INT16U total_rs;
INT16U RS_TABLE_SIZE;

FLASHRSINDEX *RS_TABLE =NULL; //Index table of RES

//user setting

INT32U tbuf[200]; //128

#ifdef C_EMBEDED_STORAGE_NAND
#define NAND_TABLE_SIZE  20
INT16U ND_MAPTABLE[NAND_TABLE_SIZE];
#endif

//L3
INT16U nv_open(INT8U *path);
INT32U nv_read(INT16S fd, INT32U buf, INT32U size);
INT32U nv_mcu_read(INT16S fd, INT32U buf, INT32U size);
//INT32U nv_fast_read(INT16S fd, INT32U buf, INT32U size);
INT32U nv_lseek(INT16S fd, INT32S foffset, INT16S origin);
INT32U nv_rs_size_get(INT16S fd); // jandy add to querry fd size
//L2
INT32U nvmemory_rs_byte_load(INT8U *name ,INT32U offset_byte, INT32U *pbuf , INT32U byte_count);
INT32U nvmemory_rs_sector_load(INT8U *name ,INT32U offset_secter, INT32U *pbuf , INT16U secter_count);
//L1
INT32U nvspiflash_retrieval(void);
INT32U nvspiflash_rs_get(INT16U itag ,INT32U offset_secter, INT32U *pbuf , INT16U secter_count);
INT32U nvspiflash_user_set (INT16U itag ,INT32U *pbuf , INT16U secter_count);
INT32U nvspiflash_user_get (INT16U itag ,INT32U *pbuf , INT16U secter_count);
//NV NAND
//L1
INT32U nvnandflash_retrieval(void);
INT32U nvnandflash_rs_get(INT16U itag ,INT32U offset_secter, INT32U *pbuf , INT16U secter_count);

#if NVRAM_MANAGER_FORMAT == NVRAM_V3
INT32U nvnandflash_user_get(INT16U itag ,INT32U *pbuf_align_16 , INT16U secter_count);
INT32U nvnandflash_user_set(INT16U itag ,INT32U *pbuf_align_16 , INT16U secter_count);
#else
INT32U nvnandflash_user_get(INT32U *pbuf_align_16 , INT16U secter_count);
INT32U nvnandflash_user_set(INT32U *pbuf_align_16 , INT16U secter_count);
#endif
//sunxw added NVRAM for jpeg or gif decode use
//L1
INT32U nvram_rs_get(INT16U itag ,INT32U offset_secter, INT32U *pbuf , INT16U secter_count);

void NVNAND_OS_LOCK(void);
void NVNAND_OS_UNLOCK(void);

#if SPIFC_RES
static OS_EVENT		*sw_spifc_sem = NULL;
#endif



void NVNAND_OS_LOCK(void)
{
  #if _NVNAND_OS_ == 1
	INT8U err = NULL;
	
	OSSemPend(gFS_sem, 0, &err);
  
  #endif
}

void NVNAND_OS_UNLOCK(void)
{
  #if _NVNAND_OS_ == 1
	OSSemPost(gFS_sem);
  #endif
}

INT32U nvram_rs_get(INT16U itag ,INT32U offset_secter, INT32U *pbuf , INT16U secter_count)
{
	if (C_NV_FLASH_TYPE == C_SPI_FLASH)
		return nvspiflash_rs_get(itag, offset_secter, pbuf, secter_count);
	else
#if NAND1_EN == 1  	
		return nvnandflash_rs_get(itag, offset_secter, pbuf, secter_count);
#endif
	return 1;
}


INT32U combin_reg_data(INT8U *data, INT32S len)
{
	INT32S i;
	INT32U value = 0;
	for (i=0;i<len;i++) {
		value |= data[i] << i*8;
	}
	return value;
}


INT16U name2index(INT8U *name)
{
#if NVRAM_MANAGER_FORMAT == NVRAM_V1 
  INT16U iffind,i,j ;
  iffind = 0 ;
  for (i =0 ; i<total_rs; i++) {
    for (j =0 ; j<8; j++) {
          if (RS_TABLE[i].name[j] == *(name+j))
          {
           if(j ==7)
            return  i ;
          }
          else
           break;
    }
  }
#else
  INT16U i,j ;
  for (i =0 ; i<total_rs; i++) {
    for (j =0 ; j<20; j++) {
          if (*(name+j) == '.') name++; //by pass [.]
          if ( (RS_TABLE[i].name[j] == 0x00) && (j >3) )  return  i ;
          if (RS_TABLE[i].name[j] != *(name+j))
                    break;
    }
  }
#endif
  return 0xffff;
}
//////////////////////////////////////////////////////////
//NV FILE SYSTEM L3
/////////////////////////////////////////////////////////
INT16U nv_open(INT8U *path)
{
  INT16U index;
  index = name2index(path);
  if (index == 0xffff) {  
//      DBG_PRINT ("nv_open retry!\r\n");	
//	  nvmemory_init();
//	  index = name2index(path);
//	  if (index == 0xffff) {  
       	 	//DBG_PRINT ("nv_open retry fail~~~~~~!\r\n");	
    		return 0xFFFF;
//      }
  }
  RS_TABLE[index].offset = 0;
  return index;
}

INT32U nv_mcu_read(INT16S fd, INT32U buf, INT32U byte_size)
{
  INT32U ret;
  if  ( (RS_TABLE[fd].offset + byte_size) >  (RS_TABLE[fd].size * 512) ) return 1;
  ret = nvmemory_rs_byte_load(RS_TABLE[fd].name , RS_TABLE[fd].offset ,(INT32U *)buf ,byte_size);
  if (!ret)
  {
    RS_TABLE[fd].offset+=byte_size;
  }
  return ret;
}


INT32U nv_read(INT16S fd, INT32U buf, INT32U byte_size)
{
  INT32U ret;
  INT32U start_sector,sector_count,render;
  INT32U offset,i;
  INT32S temp_byte;

 // FLASHRSINDEX *TRACE_TABLE =&RS_TABLE[fd]; 
  //user buffer align -4
  if (((4 - (((INT32U) buf) & 0x00000003)) & 0x00000003) != 0)
  {
    return  nv_mcu_read(fd,buf,byte_size);	
  }	
  if (byte_size<512)
  {
    return nv_mcu_read(fd,buf,byte_size);  /* small size, master ARM enough */
  }
  if  ( (RS_TABLE[fd].offset + byte_size) >  (RS_TABLE[fd].size * 512) ) {
    byte_size = byte_size - ((RS_TABLE[fd].offset + byte_size) - (RS_TABLE[fd].size * 512));
    temp_byte = (INT32S) byte_size;
    if  (temp_byte<0) {return 1;} 
  } 
  
  if (byte_size == 0) {
    return 1;
  }

  offset  =   (RS_TABLE[fd].offset & 0x000001ff);  
  //file offset align -4
  if ((512-offset) & 0x00000003 )
  {
      return  nv_mcu_read(fd,buf,byte_size);	
  }      
  start_sector =  (RS_TABLE[fd].offset) / 512 ;
  sector_count = (byte_size) /512; //secters 
  if (offset)
  {
  //Start
    ret = nvmemory_rs_sector_load(RS_TABLE[fd].name,start_sector,(INT32U *)tbuf ,1);       
    for (i =0 ; i<(512-offset) ;i ++)
       *((INT8U *)buf +i) =*((INT8U *)tbuf +i +offset);       
  //Mid
    sector_count = (byte_size -(512-offset)) / 512;
    ret = nvmemory_rs_sector_load(RS_TABLE[fd].name,start_sector+1,(INT32U *) ((INT8U *)buf +(512-offset)),sector_count);       
  //End  
    render = (byte_size -(512-offset)) % 512;
    if (render)
    {
     ret = nvmemory_rs_sector_load(RS_TABLE[fd].name,start_sector+1+sector_count,(INT32U *)tbuf,1);         
     for (i =0 ; i<render ;i ++)
       *((INT8U *)buf +i +(512-offset) +(sector_count*512)) =*((INT8U *)tbuf +i );       
    }    
  }  
  else
  {
     if (sector_count)
 	   ret = nvmemory_rs_sector_load(RS_TABLE[fd].name,start_sector,(INT32U *)buf,sector_count);   
     render = (byte_size) % 512; 
     if (render)
     {
         ret = nvmemory_rs_sector_load(RS_TABLE[fd].name,start_sector+sector_count,(INT32U *)tbuf,1);         
	     for (i =0 ; i<render ;i ++)
    	   *((INT8U *)buf +i +(sector_count*512)) =*((INT8U *)tbuf +i );       
     } 	   
      
  }
  if (!ret)
  {
    RS_TABLE[fd].offset+=byte_size;
  }
  return ret;
}
//#define SEEK_SET 0 /* start of stream (see fseek) */
//#define SEEK_CUR 1 /* current position in stream (see fseek) */
//#define SEEK_END 2 /* end of stream (see fseek) */

INT32U nv_lseek(INT16S fd, INT32S foffset, INT16S origin)
{
  INT32U targetaddr;

  //  FLASHRSINDEX *TRACE = &RS_TABLE[fd]; //Index table of RES

  if (origin == SEEK_SET)
  {
    if (foffset >= (RS_TABLE[fd].size * 512) )
     {return 0xffffffff;}
    else
     {RS_TABLE[fd].offset =  foffset ;}
  }
  else if (origin == SEEK_CUR)
  {
    targetaddr =RS_TABLE[fd].offset+foffset;
    if ( targetaddr >=  (RS_TABLE[fd].size * 512) ) {return 1;}
    RS_TABLE[fd].offset = targetaddr ;
  }
  else
  {
   RS_TABLE[fd].offset =  (RS_TABLE[fd].size * 512) - 1 ; 
  }
  return  RS_TABLE[fd].offset;
}

INT32U nv_rs_size_get(INT16S fd)
{
    INT32U ret;
    
    nv_lseek(fd,0,SEEK_SET);
	ret = nv_lseek(fd,0,SEEK_END)+1;
	nv_lseek(fd,0,SEEK_SET);

    return ret;
}


//////////////////////////////////////////////////////////
//
//  Flash access(SPI/Nand/Nor Flash)
//
//////////////////////////////////////////////////////////
#if SPIFC_RES

INT32U nvmemory_init(void)
{
	if(sw_spifc_sem == NULL)
	{
		sw_spifc_sem = OSSemCreate(1);
	}

	pSpifc_apis_ops = spifc_attach();
	pSpifc_apis_ops->init();
	nvspiflash_retrieval();
	return 0;
}
#else

INT32U nvmemory_init(void)
{
  INT8U  manID;
  INT8U  data_buf[512];

  if (C_NV_FLASH_TYPE == C_SPI_FLASH)
  {
    SPI_Flash_init();
    R_FUNPOS1 |= (1<<3); // Change CS pin to gpio function
    spi_clk_set(0,SYSCLK_128); /* spi clock = 48M/8/128 = 46.8k */
    SPI_Flash_readID(data_buf);
    manID = data_buf[0];
    if (manID == 0xd2) {
    }
	else if (manID == 0xc2) {
//		 spi_clk_set(0,SYSCLK_4);
		 spi_clk_set(0,SYSCLK_8);	//wwj modify
	}
	else {
//		 spi_clk_set(0,SYSCLK_4);
		 spi_clk_set(0,SYSCLK_8);	//wwj modify
	}
	nvspiflash_retrieval();
  }else if (C_NV_FLASH_TYPE == C_NAND_FLASH){
#if NAND1_EN == 1    
     if (Nand_Init())
       return 1;       
    if ( nvnandflash_retrieval())
       return 2;
#endif       
  }else {// C_NOR_FLASHY)


  }
  //by pass boot area and calcular others areas's position

  return 0;
}

#endif




INT32U nvmemory_rs_byte_load(INT8U *name ,INT32U offset_byte, INT32U *pbuf , INT32U byte_count)
{
 INT16U *tempbuffer;
 INT32U i,j;
 INT32U offset_secor,start_byte;
 INT32U ret,index;
 offset_secor = offset_byte / 512;
 //sector_count = (byte_count  >> 9) +1;
 start_byte   = offset_byte % 512;
 tempbuffer = (INT16U *) gp_malloc_align(2112, 16);
 if(!tempbuffer) {
 	DBG_PRINT("nvmemory_rs_byte_load memloc fail\r\n");
 	return 1;
 }
 
 //begin
 ret = nvmemory_rs_sector_load(name,offset_secor,(INT32U *)tempbuffer,1);
 if (ret >0)
 {
  gp_free((void *) tempbuffer);
  return 1;
 }
 index = 512 - start_byte;
 index = (byte_count > index) ? index :  byte_count;
 for(i=0 ; i <index ;i++)
       *((INT8U *)pbuf +i) =*((INT8U *)tempbuffer +start_byte +i );
 //mid
 byte_count -=index;
 while(byte_count >0)
 {
    offset_secor++;
    ret = nvmemory_rs_sector_load(name,offset_secor,(INT32U *)tempbuffer,1);
    if (ret >0)
    {
      gp_free((void *) tempbuffer);
      return 2;
    }
    if (byte_count>=512) {
        for(j=0 ; j < 512 ;j++)
           *((INT8U *)pbuf +j +index) =*((INT8U *)tempbuffer+j );
        index+=512;
        byte_count-=512;
    } else{
        for(j=0 ; j < byte_count ;j++)
           *((INT8U *)pbuf +j +index) =*((INT8U *)tempbuffer+j );
        break;
    }
 }  
 gp_free((void *) tempbuffer);
 return 0;

 //end
}

INT32U nvmemory_rs_sector_load(INT8U *name ,INT32U offset_secter, INT32U *pbuf , INT16U secter_count)
{
  INT16U itag ;
  itag = name2index(name);
  if (itag ==0xFFFF) return 1;
  if (C_NV_FLASH_TYPE == C_SPI_FLASH)
  {
    return (nvspiflash_rs_get(itag,offset_secter,pbuf,secter_count) );

  }
  else // (C_NV_FLASH_TYPE == C_NAND_FLASH){
  {
#if NAND1_EN == 1    
    return (nvnandflash_rs_get(itag,offset_secter,pbuf,secter_count));
#endif    
  }
  return 1;
}

INT32U nvmemory_user_sector_load(INT16U itag, INT32U *pbuf , INT16U secter_count)
{

  if (C_NV_FLASH_TYPE == C_SPI_FLASH)
  {
    return nvspiflash_user_get ( itag , pbuf , secter_count);

  }else ///if (C_NV_FLASH_TYPE == C_NAND_FLASH){
  {
#if NAND1_EN == 1  
    return nvnandflash_user_get(itag,pbuf,secter_count);
#endif    
  }
    return 1;
}

INT32U nvmemory_user_sector_store(INT16U itag, INT32U *pbuf , INT16U secter_count)
{
  if (C_NV_FLASH_TYPE == C_SPI_FLASH)
  {
    return nvspiflash_user_set ( itag , pbuf , secter_count);

  }else //if (C_NV_FLASH_TYPE == C_NAND_FLASH){
  {
#if NAND1_EN == 1  
    return nvnandflash_user_set(itag,pbuf,secter_count);
#endif    
  }
    return 1;
}

///////////////////////////////////////////////////////
// SPI Flash
///////////////////////////////////////////////////////
#define ResBaseAddr 0x170000


void sw_spifc_lock(void)
{
	INT8U err;

	OSSemPend(sw_spifc_sem, 0, &err);
}

void sw_spifc_unlock(void)
{
	OSSemPost(sw_spifc_sem);
}


INT32S SPIFC_Flash_erase_block(INT32U addr, INT8U eraseArgs)
{
	INT32S retErr;
	
	sw_spifc_lock();
	retErr = pSpifc_apis_ops->erase(addr, eraseArgs);
	sw_spifc_unlock();

	return retErr;	
}

INT32S SPIFC_Flash_read_page(INT32U addr, INT8U *buf)
{
	INT32S ret = STATUS_OK;
	TX_RX_ARGS txrxArgs ={0};

	txrxArgs.addrs = addr;
	txrxArgs.buff = buf;
	txrxArgs.buff_len = 256;

	sw_spifc_lock();
	ret = pSpifc_apis_ops->read(&txrxArgs);
	sw_spifc_unlock();
	
	return ret;
}

INT32S SPIFC_Flash_write_page(INT32U addr, INT8U *buf)
{
	INT32S ret = STATUS_OK;
	TX_RX_ARGS txrxArgs ={0};

	txrxArgs.addrs = addr;
	txrxArgs.buff = buf;
	txrxArgs.buff_len = 256;

	sw_spifc_lock();
	ret = pSpifc_apis_ops->program(&txrxArgs);
	sw_spifc_unlock();
	
	return ret;
}


//retrieval from flash
#if C_NV_FLASH_TYPE == C_SPI_FLASH
#if NVRAM_MANAGER_FORMAT == NVRAM_V1

INT32U nvspiflash_retrieval(void)
{
    INT16U  i,j ;
    INT8U  data_buf[2048];

	#if SPIFC_RES
 	SPIFC_Flash_read_page(ResBaseAddr+0, data_buf);
	#else
 	if (SPI_Flash_read_page(0, data_buf) != STATUS_OK) {
		return 1;
	}
	#endif

	
	// Comparing spi flash bootcode tag
	if (gp_strncmp((INT8S *)data_buf, (INT8S *)SPI_FLASH_TAG,10) != 0) {
			return 2;
	}
	// Getting index position
	Resource_area_position = combin_reg_data(&data_buf[HEADER_OFFSET_SEC_CNT<<1],2);
	//Resource_area_position++;
	for ( i= 0 ; i<4 ; i++)
	{
		#if SPIFC_RES
		SPIFC_Flash_read_page(ResBaseAddr + (Resource_area_position<<9) + (i<<8), data_buf + 256*i);	
		#else
		if (SPI_Flash_read_page( (Resource_area_position<<9) + (i<<8), data_buf + 256*i) != STATUS_OK) {
			return 1;
		}
		#endif
	}
	// Comparing spi flash resource area tag
	if (gp_strncmp((INT8S *)data_buf, (INT8S *)RESOURCE_TAG,2) != 0) {
			return 2;
	}
	total_rs = (INT16U) combin_reg_data(&data_buf[2],2);
    RS_TABLE_SIZE = total_rs ;   
    if (RS_TABLE !=NULL) 
    {
      gp_free((void *) RS_TABLE);
      RS_TABLE =NULL;
    }
    RS_TABLE  =  (FLASHRSINDEX *) gp_malloc_align(((total_rs+1)* sizeof(FLASHRSINDEX)), 16);
	if ( total_rs > 101)
	{
      	for ( i= 0 ; i<4 ; i++)
		{
			#if SPIFC_RES
			SPIFC_Flash_read_page(ResBaseAddr + (Resource_area_position<<9) + (i<<8)+1024, data_buf + 256*i +1024);
			#else
			if (SPI_Flash_read_page( (Resource_area_position<<9) + (i<<8)+1024, data_buf + 256*i +1024) != STATUS_OK) {
				return 1;
			}			
			#endif
		}
	}
	for (i =0 ; i<total_rs+1; i++) {
    	for (j =0 ; j<8; j++)
           RS_TABLE[i].name[j] = data_buf[j+4 + i*10];
      RS_TABLE[i].pos = (INT32U) combin_reg_data(&data_buf[12 + i*10],2) ;

	}
	// The least pos is User area position
	User_area_position = RS_TABLE[i-1].pos;
	for (i =0 ; i<total_rs; i++) {
      RS_TABLE[i].size =  RS_TABLE[i+1].pos - RS_TABLE[i].pos ;

	}
	return 0;
}


#else

INT32U nvspiflash_retrieval(void)
{
    INT16U  i,j ;
    //INT8U  data_buf[2048];
    INT16U *PageBuffer;
    INT16U Rspages;
    PageBuffer = (INT16U *) gp_malloc_align(2112, 16);
	#if SPIFC_RES
 	SPIFC_Flash_read_page(ResBaseAddr + 0, (INT8U*) PageBuffer);
	#else
 	if (SPI_Flash_read_page(0, (INT8U*) PageBuffer) != STATUS_OK) {
 	     gp_free((void *) PageBuffer);
		return 1;
	}
	#endif
	// Comparing spi flash bootcode tag
	if (gp_strncmp((INT8S *)PageBuffer, (INT8S *)SPI_FLASH_TAG,4) != 0) {
        gp_free((void *) PageBuffer);
		return 2;
	}
	// Getting index position
	#if SPIFC_RES
 	SPIFC_Flash_read_page(ResBaseAddr + 0x100, (INT8U*) (PageBuffer+0x80));
	#else
 	if (SPI_Flash_read_page(0x100, (INT8U*) (PageBuffer+0x80)) != STATUS_OK) {
 	     gp_free((void *) PageBuffer);
		return 1;
	}
	#endif
	
	APP_area_position =combin_reg_data((INT8U *)&PageBuffer[BootToAPP_HEADER_OFFSET_SEC_CNT],4);
	#if SPIFC_RES
	SPIFC_Flash_read_page(ResBaseAddr + (APP_area_position*512),(INT8U*)PageBuffer);
	#else
	if (SPI_Flash_read_page( (APP_area_position*512),(INT8U*)PageBuffer) != STATUS_OK) {
	    gp_free((void *) PageBuffer);
		return 1;
	}
	#endif
	// Comparing spi flash APP area tag
	if (gp_strncmp((INT8S *)PageBuffer, (INT8S *)APP_AREA_TAG,4) != 0) {
            gp_free((void *) PageBuffer);
			return 2;
	}

	Resource_area_position =combin_reg_data((INT8U *)&PageBuffer[APPToResource_HEADER_OFFSET_SEC_CNT],4);
	Resource_area_size =combin_reg_data((INT8U *)&PageBuffer[APPToResource_HEADER_OFFSET_SEC_CNT+2],4);
	Resource_area_position = Resource_area_position+APP_area_position;
	for ( i= 0 ; i<4 ; i++)
	{
		#if SPIFC_RES
		SPIFC_Flash_read_page(ResBaseAddr + (Resource_area_position*512) + (256*i),(INT8U*)(PageBuffer + (128*i)));
		#else
		if (SPI_Flash_read_page( (Resource_area_position*512) + (256*i),(INT8U*)(PageBuffer + (128*i))) != STATUS_OK) {
 		    gp_free((void *) PageBuffer);
			return 1;
		}
		#endif
	}
	
	
	// Comparing spi flash resource area tag
	if (gp_strncmp((INT8S *)PageBuffer, (INT8S *)RESOURCE_TAG,2) != 0) {
            gp_free((void *) PageBuffer);
			return 3;
	}
	total_rs = (INT16U) combin_reg_data((INT8U *)&PageBuffer[1],2);	
    RS_TABLE_SIZE = total_rs ;  
 
    if (RS_TABLE !=NULL) 
    {
      gp_free((void *) RS_TABLE);
      RS_TABLE =NULL;
    }
    RS_TABLE  =  (FLASHRSINDEX *) gp_malloc_align(((total_rs+1)* sizeof(FLASHRSINDEX)), 16);
    Rspages = (((total_rs*23 +4)-1) / 256)+1; //  256 bytes/page
    if (Rspages >4)
    {
      	for ( i= 0 ; i<Rspages-4 ; i++)
		{
			#if SPIFC_RES
			SPIFC_Flash_read_page(ResBaseAddr + (Resource_area_position*512) + (256*i)+1024, (INT8U*)(PageBuffer + 128*i +512));
			#else
			if (SPI_Flash_read_page( (Resource_area_position*512) + (256*i)+1024, (INT8U*)(PageBuffer + 128*i +512)) != STATUS_OK) {
    		    gp_free((void *) PageBuffer);
				return 1;
			}
			#endif
		}
	}
   for (i =0 ; i<total_rs+1; i++) {
     for (j =0 ; j<19; j++)
           RS_TABLE[i].name[j] = *((INT8U *)PageBuffer + j+24 +(24*i) );
      RS_TABLE[i].pos = (INT32U) combin_reg_data((INT8U *)PageBuffer +44+ (24*i),4);
   }
	// The least pos is User area position
	User_area_position =Resource_area_size;
	
	//DBG_PRINT("User_area_position = %d\r\n", User_area_position);
	for (i =0 ; i<total_rs-1; i++) {
      RS_TABLE[i].size =  RS_TABLE[i+1].pos - RS_TABLE[i].pos ;

	}
    RS_TABLE[i].size =  Resource_area_size - RS_TABLE[i].pos  ;
    gp_free((void *) PageBuffer);
	return 0;
}


#endif


INT32U nvspiflash_rs_get(INT16U itag ,INT32U offset_secter, INT32U *pbuf , INT16U secter_count)
{
   INT16U i;
   INT16U max_sec_count,read_sec_count;
   INT32U start_addr_sec;
   // checking rs tage
   if ( itag > total_rs)  return 1;
   max_sec_count  = RS_TABLE[itag].size;// RS_TABLE[itag+1].pos - RS_TABLE[itag].pos;
   start_addr_sec = RS_TABLE[itag].pos + Resource_area_position;
   //Add offset
   start_addr_sec += offset_secter;
   read_sec_count = ( (max_sec_count-offset_secter) < secter_count) ?  (max_sec_count-offset_secter) : secter_count ;
   for (i =0 ; i<read_sec_count << 1; i++)
   {
	#if SPIFC_RES
    SPIFC_Flash_read_page(ResBaseAddr + (start_addr_sec*512) + (256*i),(INT8U *)pbuf + 256*i );
    #else
    if (SPI_Flash_read_page( (start_addr_sec*512) + (256*i),(INT8U *)pbuf + 256*i ) != STATUS_OK)
		return 2;
	#endif
   }
   return 0;
}

#if 1
//Each stored data is fixed as 512 bytes (1 sector) and is stored secter by secter on FLASH


INT32U nvspiflash_user_set (INT16U itag ,INT32U *pbuf , INT16U secter_count)
{
  INT32U base_adr,i,j,found;
  if (secter_count > 0x80) return 1;
  //Get Base address
  base_adr = (User_area_position + Resource_area_position); //64K per sector
  i= base_adr & (0x80-1);
  base_adr = base_adr & ~(0x80 -1);
  if (i) base_adr +=0x80; //bounary as 64K (ERASE UNIT)
  base_adr += (itag *0x80*USER_SPI_BLK);
  //Get the least empty secter
  found = 0;
  
  for (j =0 ;j <USER_SPI_BLK ;j++)  
  {
#if ( USER_SPI_AREA_SIZE == USER_SPI_512_B)
	   for (i =0 ;i <0x80 ; i++)
	   {
		 #if SPIFC_RES
		 SPIFC_Flash_read_page(ResBaseAddr + (base_adr+i) * 512 , (INT8U *)tbuf );
		 #else
		 if (SPI_Flash_read_page(  (base_adr+i) * 512 , (INT8U *)tbuf ) != STATUS_OK)
			return 1;
		 #endif
	     if ((tbuf[0] ==0xffffffff) &&  (tbuf[1] ==0xffffffff))
	     {
	        found = 1;	
	        break;
	     }
	   }
#else
	   for (i =0 ;i <0x100 ; i++)
	   {
		 #if SPIFC_RES
		 SPIFC_Flash_read_page(ResBaseAddr + (base_adr*512) + (i* 256) , (INT8U *)tbuf );
	     #else
		 if (SPI_Flash_read_page(  (base_adr*512) + (i* 256) , (INT8U *)tbuf ) != STATUS_OK)
			return 1;
		 #endif
	     if ((tbuf[0] ==0xffffffff) &&  (tbuf[1] ==0xffffffff))
	     {
	        found = 1;	
	        break;
	     }
	   }
#endif	   
	   if (found) break;	   
	   base_adr +=0x80;
  }  
	if ( (found) && ( (128-i) > secter_count ) )
  {

   if (itag==0)
   {
#if ( USER_SPI_AREA_SIZE == USER_SPI_512_B)  
	    for (j =0 ;j < 2 ;j++)
    	{
			#if SPIFC_RES
	    	if(SPIFC_Flash_write_page(ResBaseAddr + (((base_adr+i) * 512 ) + 256*j ) , (INT8U *)pbuf +256*j ) != STATUS_OK)
				return 1;
			#else
	    	if(SPI_Flash_write_page(( ((base_adr+i) * 512 ) + 256*j ) , (INT8U *)pbuf +256*j ) != STATUS_OK)
				return 1;
			#endif
	    }			
#else
		#if SPIFC_RES
 		if(SPIFC_Flash_write_page(ResBaseAddr + (base_adr*512) + (i* 256)    , (INT8U *)pbuf  ) != STATUS_OK)
			return 1;
		#else
 		if(SPI_Flash_write_page( (base_adr*512) + (i* 256)    , (INT8U *)pbuf  ) != STATUS_OK)
			return 1;
		#endif
#endif    
   }else
   {
   	    for (j =0 ;j < (secter_count <<1) ;j++)
    	{
			#if SPIFC_RES
	    	if(SPIFC_Flash_write_page(ResBaseAddr + (((base_adr+i) * 512 ) + 256*j ) , (INT8U *)pbuf +256*j ) != STATUS_OK)
				return 1;
    		#else
	    	if(SPI_Flash_write_page(( ((base_adr+i) * 512 ) + 256*j ) , (INT8U *)pbuf +256*j ) != STATUS_OK)
				return 1;
			#endif
	    }	   
   }
   
  }      
  else
  {
 		    if( (found == 1) && ((128-i) <= secter_count))
		    {
		    	base_adr += 0x80;
		    }
    //Erase All of blocks 
    for (i =0 ;i <USER_SPI_BLK ;i++) {
    	#if SPIFC_RES
    	SPIFC_Flash_erase_block(ResBaseAddr + (base_adr - 0x80 - (i* 0x80)) * 512 ,ERASE_BLOCK_64K);
    	#else 
        SPI_Flash_erase_block( (base_adr - 0x80 - (i* 0x80)) * 512 );  
       #endif
   }
    //Write at the first page    
    base_adr= base_adr - (0x80*USER_SPI_BLK) ;  
   if (itag==0)
   {    
#if ( USER_SPI_AREA_SIZE == USER_SPI_512_B)                
    	for (j =0 ;j < 2 ;j++)
	    {   
			#if SPIFC_RES
	    	if(SPIFC_Flash_write_page(ResBaseAddr + ((base_adr * 512 ) + (256*j) ) , (INT8U *)pbuf +256*j ) != STATUS_OK)
				return 1;			
			#else
	    	if(SPI_Flash_write_page( ((base_adr * 512 ) + (256*j) ) , (INT8U *)pbuf +256*j ) != STATUS_OK)
				return 1;
			#endif
	    }			
#else
		#if SPIFC_RES
	    if(SPIFC_Flash_write_page(ResBaseAddr + (base_adr*512) + (i* 256)+ (256*j) , (INT8U *)pbuf +256*j  ) != STATUS_OK)
			return 1;		
		#else
	    if(SPI_Flash_write_page( (base_adr*512) + (i* 256)+ (256*j) , (INT8U *)pbuf +256*j  ) != STATUS_OK)
			return 1;
		#endif
#endif    
   }else
   {
   	    for (j =0 ;j < (secter_count <<1) ;j++)
    	{
			#if SPIFC_RES
	    	if(SPIFC_Flash_write_page((ResBaseAddr + ((base_adr+i) * 512 ) + 256*j ) , (INT8U *)pbuf +256*j ) != STATUS_OK)
				return 1;
    		#else
	    	if(SPI_Flash_write_page(( ((base_adr+i) * 512 ) + 256*j ) , (INT8U *)pbuf +256*j ) != STATUS_OK)
				return 1;
			#endif
	    }
   }
    
  }  
  return 0;
}

INT32U nvspiflash_user_get (INT16U itag ,INT32U *pbuf , INT16U secter_count)
{
  INT32U base_adr,i,j,found;
   if (secter_count > 0x80) return 1;
  base_adr = (User_area_position + Resource_area_position); //64K per sector
  i= base_adr & (0x80-1);
  base_adr = base_adr & ~(0x80 -1);
  if (i) base_adr +=0x80; //bounary as 64K
  base_adr += (itag *0x80*USER_SPI_BLK);
  found = 0;
  for (j =0 ;j <USER_SPI_BLK ;j++)  
  {
#if ( USER_SPI_AREA_SIZE == USER_SPI_512_B)
	   for (i =0 ;i <0x80 ; i++)
	   {
		 #if SPIFC_RES
		 SPIFC_Flash_read_page(ResBaseAddr + (base_adr+i) * 512 , (INT8U *)tbuf );		 
	     #else
		 if (SPI_Flash_read_page(  (base_adr+i) * 512 , (INT8U *)tbuf ) != STATUS_OK)
			return 1;
		 #endif
	     if ((tbuf[0] ==0xffffffff) &&  (tbuf[1] ==0xffffffff))
	     {
	        found = 1;	
	        break;
	     }
	   }
#else
	   for (i =0 ;i <0x100 ; i++)
	   {
		 #if SPIFC_RES
		 SPIFC_Flash_read_page(ResBaseAddr + (base_adr*512) + (i* 256), (INT8U *)tbuf );		 
	     #else
		 if (SPI_Flash_read_page(  (base_adr*512) + (i* 256), (INT8U *)tbuf ) != STATUS_OK)
			return 1;
		 #endif
	     if ((tbuf[0] ==0xffffffff) &&  (tbuf[1] ==0xffffffff))
	     {
	        found = 1;	
	        break;
	     }
	   }
#endif	   
	   if (found) break;	   
	   base_adr +=0x80;
  }        
  if (found)
  {
      if (i==0) //checking boundary
      {
        if (j==0) return 0;
        else
        {
          base_adr= base_adr -0x80;
          i       = 0x80;
        }      
      }   
   if (itag==0)
   {         
#if ( USER_SPI_AREA_SIZE == USER_SPI_512_B)      
	  for (j =0 ;j <2 ;j++)
	  {
		 #if SPIFC_RES
		 SPIFC_Flash_read_page(ResBaseAddr + ((base_adr +i-1) * 512) + 256*j  , (INT8U *)pbuf + 256*j );
		 #else
		 if (SPI_Flash_read_page(( (base_adr +i-1) * 512) + 256*j  , (INT8U *)pbuf + 256*j ) != STATUS_OK)
			return 1;
		 #endif
	  }
#else
	 #if SPIFC_RES
	 SPIFC_Flash_read_page(ResBaseAddr + (base_adr*512) + ((i-1)* 256) , (INT8U *)pbuf );
	 #else
	 if (SPI_Flash_read_page((base_adr*512) + ((i-1)* 256) , (INT8U *)pbuf ) != STATUS_OK)
			return 1;
	 #endif
#endif	  
   }else
   {
      for (j =0 ;j < (secter_count <<1) ;j++)
	  {
		 #if SPIFC_RES
		 SPIFC_Flash_read_page(ResBaseAddr + ((base_adr +i-1) * 512) + 256*j  , (INT8U *)pbuf + 256*j );
	  	 #else
		 if (SPI_Flash_read_page(( (base_adr +i-1) * 512) + 256*j  , (INT8U *)pbuf + 256*j ) != STATUS_OK)
			return 1;
		 #endif
	  }   
   }
   return 0;	  
 }
 return 1;
}

#else

// Tag: 0~4  each slot is  64k (0x10000) ( 0x80  sectors )
INT32U nvspiflash_user_set (INT16U itag ,INT32U *pbuf , INT16U secter_count)
{
  INT32U base_adr,i;
  if (secter_count > 0x80) return 1;
  base_adr = (User_area_position + Resource_area_position); //64K per sector
  i= base_adr & (0x80-1);
  base_adr = base_adr & ~(0x80 -1);
  if (i) base_adr +=0x80;
  base_adr += (itag *0x80);
  SPI_Flash_erase_block(base_adr * 512 );
  for (i =0 ; i < (secter_count <<1) ;i++)
  {
    if(SPI_Flash_write_page( (base_adr * 512 ) + (i <<8) , (INT8U *)pbuf + 256*i ) != STATUS_OK)
		return 1;
  }
  return 0;
}
INT32U nvspiflash_user_get (INT16U itag ,INT32U *pbuf , INT16U secter_count)
{
   INT32U base_adr,i;
   base_adr = (User_area_position + Resource_area_position); //64K per sector
   i= base_adr & (0x80-1);
   base_adr = base_adr & ~(0x80 -1);
   if (i) base_adr +=0x80;
   base_adr += (itag *0x80);
   for (i =0 ;i <secter_count << 1 ;i++)
   {
	 if (SPI_Flash_read_page( (base_adr * 512) +(i <<8) , (INT8U *)pbuf + 256*i ) != STATUS_OK)
		return 1;
   }
   return 0;
}
#endif

#if 1
INT32U nvspiflash_file_size_get(INT16U itag)
{
  	if (C_NV_FLASH_TYPE == C_SPI_FLASH)
  	{
		return RS_TABLE[itag].size;
	}
	return RS_TABLE[itag].size;
}
#endif

#endif//  if (C_NV_FLASH_TYPE == C_SPI_FLASH)

///////////////////////////////////////////////////////
//NAND Flash
///////////////////////////////////////////////////////

//retrieval from nand flash
// BOOT_AREA_START & BOOT_AREA_SIZE	are defined at L2 nand manager
#if NAND1_EN == 1

#if NVRAM_MANAGER_FORMAT == NVRAM_V3
//////////////////////////////////////////////////////////////////////////////////////////
INT32U nvnandflash_rs_get(INT16U itag ,INT32U offset_secter, INT32U *pbuf , INT16U secter_count)
{
   INT32U ret;
   INT16U max_sec_count,read_sec_count;
   INT32U start_addr_sec; 
   //ALIGN16 INT16U    PageBuffer[1056];
   if ( itag > RS_TABLE_SIZE )  return 1;
   max_sec_count  =  RS_TABLE[itag].size;
   start_addr_sec =  RS_TABLE[itag].pos   + Resource_area_position;
      //Add offset
   start_addr_sec += offset_secter;
   read_sec_count = ( (max_sec_count-offset_secter) < secter_count) ?  (max_sec_count-offset_secter) : secter_count ;   
   NVNAND_OS_LOCK();
   ret = NandBootReadSector(start_addr_sec,read_sec_count,(INT32U)pbuf);   
   NVNAND_OS_UNLOCK();
   if (ret) {
	   return 1;
   } else {
	   return 0;
   }
}

//secter_count must be fixed
//pbuf_align_16 must been align 16 bytes
INT32U nvnandflash_user_get(INT16U itag ,INT32U *pbuf_align_16 , INT16U secter_count)
{
   INT32U ret;
   NVNAND_OS_LOCK();
   
   //ret = NandBootReadSector((User_area_position  +Resource_area_position),secter_count,(INT32U)pbuf_align_16);   
   if (itag==0)
     ret= DrvNand_read_sector(0,1,(INT32U)pbuf_align_16);   
   else
     ret= DrvNand_read_sector(itag,secter_count,(INT32U)pbuf_align_16);   
   NVNAND_OS_UNLOCK();
   if (ret) {
	   return 1;
   } else {
	   return 0;
   }

}

//secter_count must be fixed
//pbuf must been align 16 bytes
INT32U nvnandflash_user_set(INT16U itag ,INT32U *pbuf_align_16 , INT16U secter_count)
{
   INT32U ret;
   NVNAND_OS_LOCK();
   NandBootEnableWrite();
  //ret = NandBootWriteSector((User_area_position  +Resource_area_position),secter_count,(INT32U)pbuf_align_16);   
   if (itag==0)
	   ret= DrvNand_write_sector(0,1,(INT32U)pbuf_align_16);   
   else
       ret= DrvNand_write_sector(itag,secter_count,(INT32U)pbuf_align_16);   
   NandBootFlush();
   NandBootDisableWrite();
   NVNAND_OS_UNLOCK();
   if (ret){
	   return 1;
   } else {
       NVNAND_OS_LOCK();
       NandBootFlush();
       NVNAND_OS_UNLOCK();
	   return 0;
   }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
#else  // !NVRAM_V3

INT32U nvnandflash_rs_get(INT16U itag ,INT32U offset_secter, INT32U *pbuf , INT16U secter_count)
{
#if (defined _DRV_L1_DMA) && (_DRV_L1_DMA == 1)
   DMA_STRUCT dma_struct;
#else
   INT16U j;
#endif
   INT16U i,isector,ipage;
   INT16U max_sec_count,read_sec_count;
   INT32U start_addr_sec;
   INT32U temp,pbufindx;
   INT16U *PageBuffer;
   
   //ALIGN16 INT16U    PageBuffer[1056];
   if ( itag > RS_TABLE_SIZE )  return 1;
   PageBuffer = (INT16U *) gp_malloc_align(2112, 16);
   // checking rs tage
  // for (i =0 ; i<RS_TABLE_SIZE; i++) {
  //    if (RS_TABLE[i].tag == itag )
  //      break;
  // }
   max_sec_count  =  RS_TABLE[itag].size;
   start_addr_sec =  RS_TABLE[itag].pos   + Resource_area_position;
      //Add offset
   start_addr_sec += offset_secter;
   read_sec_count = ( (max_sec_count-offset_secter) < secter_count) ?  (max_sec_count-offset_secter) : secter_count ;

   temp= ND_MAPTABLE[start_addr_sec / (nand_page_nums_per_block_get() * nand_sector_nums_per_page_get()) ];
   if (temp ==0xFFFF ) return 1;
   temp = temp * nand_page_nums_per_block_get(); // block addr by page
   temp = temp | ( (start_addr_sec /nand_sector_nums_per_page_get() ) & (nand_page_nums_per_block_get()   -1)) ;
   isector = start_addr_sec % nand_sector_nums_per_page_get();
   //Start
   pbufindx =0;
   if (isector > 0 )
   {
    NVNAND_OS_LOCK(); 
    Nand_ReadPhyPage(temp,(INT32U)PageBuffer);
    NVNAND_OS_UNLOCK();
#if (defined _DRV_L1_DMA) && (_DRV_L1_DMA == 1)
    if ( secter_count <= (nand_sector_nums_per_page_get()-isector))
        dma_struct.count = secter_count*128;
    else
	    dma_struct.count = (INT32U) (nand_sector_nums_per_page_get()-isector)*128;    
	dma_struct.s_addr = (INT32U) PageBuffer + (isector*512);
	dma_struct.t_addr = (INT32U) pbuf;
	dma_struct.width = DMA_DATA_WIDTH_4BYTE;
	dma_struct.timeout = 64;
    NVNAND_OS_LOCK(); 
	if (dma_transfer_wait_ready(&dma_struct))
    {
        NVNAND_OS_UNLOCK();
	    return -1;
    }
    NVNAND_OS_UNLOCK();
    pbufindx    +=256*(nand_sector_nums_per_page_get()-isector);
    if ( secter_count <= (nand_sector_nums_per_page_get()-isector))
	{
      	 		gp_free((void *) PageBuffer);
      		 	return 0 ;
    }
    secter_count-=(nand_sector_nums_per_page_get()-isector);
#else
    for(i =0; i < (nand_sector_nums_per_page_get()-isector);i++)
    {
     	  for(j=0;j<256;j++) //512 = (128 x 4 ) bytes
      	  {
	      		*((INT16U*)pbuf+j+i*256)= PageBuffer[j+(isector+i)*256];
    	  }
	      pbufindx +=256;
    	  secter_count--;
	      if ( secter_count ==0)
	      {
      	 		gp_free((void *) PageBuffer);
      		 	return 0 ;
    	  }
    }
#endif
    temp++;
   }
   //Mid
   ipage   = secter_count / nand_sector_nums_per_page_get();
   isector = secter_count % nand_sector_nums_per_page_get();
   for(i =0; i < ipage ; i++)
   {
    if ((temp & (nand_page_nums_per_block_get()-1)) ==0) //if new block
    {
        temp= ND_MAPTABLE[temp/ (nand_page_nums_per_block_get())];
        if (temp ==0xFFFF ) {
         gp_free((void *) PageBuffer);
         return 2;
        }
        temp = temp * nand_page_nums_per_block_get(); // block addr by page
    }
    NVNAND_OS_LOCK();
    Nand_ReadPhyPage(temp,(INT32U)( (INT16U *)pbuf+pbufindx));
    NVNAND_OS_UNLOCK();
    pbufindx +=nand_sector_nums_per_page_get()*256 ;//1024; //2048  =(4 x 128)
    temp++;
   }
   //End
   if (isector >0)
   {
      if ((temp & (nand_page_nums_per_block_get()-1)) ==0) //if new block
      {
        temp= ND_MAPTABLE[temp/ (nand_page_nums_per_block_get())];
        if (temp ==0xFFFF ) {
         gp_free((void *) PageBuffer);
         return 2;
         }
        temp = temp * nand_page_nums_per_block_get(); // block addr by page
      }
      NVNAND_OS_LOCK();
      Nand_ReadPhyPage(temp,(INT32U)PageBuffer);

#if (defined _DRV_L1_DMA) && (_DRV_L1_DMA == 1)
 	  dma_struct.s_addr = (INT32U) PageBuffer;
	  dma_struct.t_addr = (INT32U) pbuf + (pbufindx<<1);
	  dma_struct.width = DMA_DATA_WIDTH_4BYTE;
	  dma_struct.count = (INT32U) (isector*128);
	  dma_struct.timeout = 64;
	  if (dma_transfer_wait_ready(&dma_struct))
      {
        NVNAND_OS_UNLOCK();
	    return -1;
      }
      NVNAND_OS_UNLOCK();
#else
      for(i =0; i < isector ;i++)
      {
       for(j=0;j<256;j++) //512 = (128 x 4 ) bytes
         *((INT16U*)pbuf +j+ pbufindx)= PageBuffer[j+i*256];
       pbufindx +=256;
      }
#endif
   }
   gp_free((void *) PageBuffer);
   return 0;
}

INT16U USE_LINK_TBL[10];
INT8U  USE_WRITE_TAG[10];
//secter_count must be fixed
//pbuf_align_16 must been align 16 bytes
INT32U nvnandflash_user_get(INT32U *pbuf_align_16 , INT16U secter_count)
{
   INT16U i,j;
   INT32U user_start_link_blkadr,user_org_blkadr;
   INT16U user_blk_count;
   INT16U lbamark,bchmark,firstblk,firstflag,allblkempty;
   INT16U ipage,isector;
   INT32U pbufindx,current_page_adr,user_current_blk_adr;
   INT32U ret;
   //ALIGN16 INT16U   PageBuffer[1056];
   INT16U *PageBuffer;
   PageBuffer = (INT16U *) gp_malloc_align(2112, 16);

//   if (secter_count <0) return 1;

   //Clear all user mapping blocks
   for(i=0; i<10; i++)
   {
     USE_LINK_TBL[i]  = 0xFFFF;
     USE_WRITE_TAG[i] = 0;
   }
   //1. Find first block addres  & buid mapping table from anothers
   user_start_link_blkadr = (User_area_position  +Resource_area_position)/ (nand_page_nums_per_block_get() * nand_sector_nums_per_page_get());
   user_org_blkadr = user_start_link_blkadr;
   //If location not at starting of the block
   if ((( (User_area_position +Resource_area_position) / nand_sector_nums_per_page_get() ) & (nand_page_nums_per_block_get()  -1)) !=0)
       user_start_link_blkadr++;
   for(i=0; i<10; i++)
   {
     if (ND_MAPTABLE[user_start_link_blkadr] != 0xFFFF)
          USE_LINK_TBL[i] = ND_MAPTABLE[user_start_link_blkadr];
     else
         break;
       user_start_link_blkadr++;
   }
   user_blk_count = i;
   //2. Searching each user blocks to find out current data loaction by first page
   firstflag =1;
   allblkempty  =1;
   NVNAND_OS_LOCK();
   for(i=0; i<user_blk_count; i++)
   {
     ret = Nand_ReadPhyPage( USE_LINK_TBL[i] * nand_page_nums_per_block_get(),(INT32U)PageBuffer);
     if(ret)
     {
        gp_free((void *) PageBuffer);
        NVNAND_OS_UNLOCK();
        return 2;
     }
     lbamark=combin_reg_data((INT8U *)&PageBuffer + 2050,2);
     bchmark=combin_reg_data((INT8U *)&PageBuffer + 2059,2);
     if ((lbamark = 0xFFFF) && (bchmark == 0xFFFF)) //iF a empty block
     {
        USE_WRITE_TAG[i] = 1;
     }else
     {
        if (firstflag) //the first nonempty block
        {
          firstblk  =i;
          firstflag =0;
        }
        allblkempty  =0;
     }
   }
   NVNAND_OS_UNLOCK();
   if(allblkempty ==1) //get orginal blk
      user_current_blk_adr = ND_MAPTABLE[user_org_blkadr] * nand_page_nums_per_block_get();
   else               //get the current block
      user_current_blk_adr = USE_LINK_TBL[firstblk] * nand_page_nums_per_block_get() ;

   //3. Searching each pages within block (back tracing)
   NVNAND_OS_LOCK();  
   for(i=nand_page_nums_per_block_get() -1 ; i>0; i--) //find the last botton nonempty page in the block
   {
     ret = Nand_ReadPhyPage( user_current_blk_adr +i,(INT32U)PageBuffer);
     if(ret)
     if(ret) {
       gp_free((void *) PageBuffer);
       NVNAND_OS_UNLOCK();
       return 3;
      }
     lbamark=combin_reg_data((INT8U *)&PageBuffer + 2050,2);
     bchmark=combin_reg_data((INT8U *)&PageBuffer + 2059,2);
    if ((lbamark != 0xFFFF) || (bchmark != 0xFFFF))
        break;
   }
   NVNAND_OS_UNLOCK();
   current_page_adr = user_current_blk_adr +i; //storage last page address
   //4. Reading User data
   ipage   = secter_count / nand_sector_nums_per_page_get();
   isector = secter_count % nand_sector_nums_per_page_get();
   pbufindx = ipage*nand_sector_nums_per_page_get()*128;
   //end part
   if (isector == 0)
   {
     pbufindx = pbufindx - nand_sector_nums_per_page_get()*128;
     isector =4;
     ipage--; //left page --
   }
   for(i =0; i < isector ;i++)
   {
     for(j=0;j<256;j++) //512 = (256 x 2 ) bytes
          *((INT16U*)pbuf_align_16 +j+ pbufindx)= PageBuffer[j+i*256];
      pbufindx +=256;
   }
    //mid part
   if (ipage > 0) //left page
   {
      current_page_adr = current_page_adr - ipage ;
      pbufindx = 0 ;
     NVNAND_OS_LOCK();
	 for(i =0; i < ipage ;i++)
	 {   
     	 ret = Nand_ReadPhyPage(current_page_adr,(INT32U)((INT16U*)pbuf_align_16+pbufindx));
     	 if(ret) {
     	    gp_free((void *) PageBuffer);
            NVNAND_OS_UNLOCK();
            return 4;
         }
	     pbufindx +=512;
	     current_page_adr++;
   	 }
     NVNAND_OS_UNLOCK();
   }
   gp_free((void *) PageBuffer);
   return 0;
}

//secter_count must be fixed
//pbuf must been align 16 bytes
INT32U nvnandflash_user_set(INT32U *pbuf_align_16 , INT16U secter_count)
{
   INT16U i;
   INT32U user_start_link_blkadr,user_org_blkadr;
   INT16U user_blk_count;
   INT16U lbamark,bchmark,firstblk,firstflag,allblkempty;
   INT32U current_page_adr,user_current_blk_adr;

   INT16U ipage ,left_page;
   INT32U ret;

   //
    INT16U *PageBuffer;
   PageBuffer = (INT16U *) gp_malloc_align(2112, 16);//ALIGN16 INT16U   PageBuffer[1056];
//   if (secter_count <0) return 1;
   //Clear all user mapping blocks
   for(i=0; i<10; i++)
   {
     USE_LINK_TBL[i]  = 0xFFFF;
     USE_WRITE_TAG[i] = 0;
   }
   //1. Find first block addres  & buid mapping table
   user_start_link_blkadr = (User_area_position +Resource_area_position) / (nand_page_nums_per_block_get() * nand_sector_nums_per_page_get());
   user_org_blkadr = user_start_link_blkadr;
   if ((( (User_area_position +  Resource_area_position)  / nand_sector_nums_per_page_get() ) & (nand_page_nums_per_block_get()  -1)) !=0)
       user_start_link_blkadr++;
   for(i=0; i<10; i++)
   {
     if (ND_MAPTABLE[user_start_link_blkadr] != 0xFFFF)
          USE_LINK_TBL[i] = ND_MAPTABLE[user_start_link_blkadr];
     else
         break;
       user_start_link_blkadr++;
   }
   user_blk_count = i;
   //2. Searching each user blocks to find out current data loaction by first page
   firstflag =1;
   allblkempty  =1;
   NVNAND_OS_LOCK();
   for(i=0; i<user_blk_count; i++)
   {
     ret =Nand_ReadPhyPage( USE_LINK_TBL[i] * nand_page_nums_per_block_get(),(INT32U)PageBuffer);

     if(ret) {
            gp_free((void *) PageBuffer);
            NVNAND_OS_UNLOCK();
            return 2;
     }
     lbamark=combin_reg_data((INT8U *)&PageBuffer + 2050,2);
     bchmark=combin_reg_data((INT8U *)&PageBuffer + 2059,2);
     if ((lbamark = 0xFFFF) && (bchmark == 0xFFFF)) //iF a empty block
     {
        USE_WRITE_TAG[i] = 1;
     }else
     {
        if (firstflag) //the first nonempty block
        {
          firstblk  =i;
          firstflag =0;
        }
        allblkempty  =0;
     }
   }
   NVNAND_OS_UNLOCK(); 
   
   if(allblkempty ==1) {//get orginal blk
      user_current_blk_adr = ND_MAPTABLE[user_org_blkadr] * page_nums_per_block;}
   else{
                       //get the current block
      user_current_blk_adr = USE_LINK_TBL[firstblk] * page_nums_per_block;}

   //3. Searching each pages within block
   NVNAND_OS_LOCK();
   for(i=page_nums_per_block -1 ; i>0; i--) //find the last botton nonempty page in the block
   {

     ret = Nand_ReadPhyPage( user_current_blk_adr +i,(INT32U)PageBuffer);

     if(ret) {
       gp_free((void *) PageBuffer);
       NVNAND_OS_UNLOCK();
       return 4;
     }
     lbamark=combin_reg_data((INT8U *)&PageBuffer +2050,2);
     bchmark=combin_reg_data((INT8U *)&PageBuffer +2059,2);
    if ((lbamark != 0xFFFF) || (bchmark != 0xFFFF))
        break;
   }
   NVNAND_OS_UNLOCK();
   current_page_adr = user_current_blk_adr +i; //storage last page address

   //4.checkif enough space to program
   left_page = page_nums_per_block -(current_page_adr - user_current_blk_adr)-1;
   if ( secter_count <= (left_page <<2) ) //write to current block
   {
     user_current_blk_adr = current_page_adr + 1;
   }else //change current block to another block
   {
     //erase current block and checking bad block
     if (!allblkempty)
     {
     	ret = Nand_ErasePhyBlock(user_current_blk_adr);
	    if(ret & 0x1) //check bad block
	    {
			//Erase fail , do not put this block into recycle Fifo.
            *(INT16U*)&(PageBuffer[1024]) = 0xff42;
         	*(INT16U*)&(PageBuffer[1025]) = 0xff42;
          	*(INT16U*)&(PageBuffer[1026]) = 0xff42;
         	*(INT16U*)&(PageBuffer[1027]) = 0xff42;
         	*(INT16U*)&(PageBuffer[1028]) = 0xff42;
            NVNAND_OS_LOCK();
         	ret = Nand_WritePhyPage(user_current_blk_adr + page_nums_per_block -1, (INT32U)PageBuffer); // Last page
            NVNAND_OS_UNLOCK();
            if(ret)
			{
			    NVNAND_OS_LOCK();
				ret = Nand_WritePhyPage(user_current_blk_adr + page_nums_per_block -3,(INT32U)PageBuffer); // Last-2  page
                NVNAND_OS_UNLOCK();
                if(ret) {
				  gp_free((void *) PageBuffer);
                  return 5;
                }
			}
     	}
     }
     //find next empty block
     if (firstblk != user_blk_count-1)
      firstblk++;
     else
      firstblk=0;
     user_current_blk_adr = USE_LINK_TBL[firstblk] * page_nums_per_block ;
   }
   //5. programming data
   *(INT16U*)&(PageBuffer[1024]) = 0xFFFF;
   *(INT16U*)&(PageBuffer[1025]) = 0x55AA;
   *(INT16U*)&(PageBuffer[1026]) = 0xFFFF;
   *(INT16U*)&(PageBuffer[1027]) = 0xFFFF;
   *(INT16U*)&(PageBuffer[1028]) = 0xFFFF;
   ipage   = secter_count / nand_sector_nums_per_page_get();
   if ((secter_count % nand_sector_nums_per_page_get()) > 0) ipage++;

   NVNAND_OS_LOCK();
   for (i = 0; i< ipage; i++)
   {
     ret = Nand_WritePhyPage(user_current_blk_adr + i ,(INT32U) (pbuf_align_16 + i*128));
 	 if(ret) {
 	   gp_free((void *) PageBuffer);
       NVNAND_OS_UNLOCK();
       return 6;
     }
   }
   NVNAND_OS_UNLOCK();
  gp_free((void *) PageBuffer);
  return 0 ;
}



#endif//#ifdef NVRAM_V3

#endif//NAND1_EN == 1
//=== This is for code configuration DON'T REMOVE or MODIFY it ================//
#endif //(defined GPLIB_NVRAM_SPI_FLASH_EN) && (GPLIB_NVRAM_SPI_FLASH_EN == 1) //
//=============================================================================//
