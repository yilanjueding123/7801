#ifndef __GPLIB_H__
#define __GPLIB_H__

#include "driver_l2.h"
#include "gplib_cfg.h"
#include "project.h"


// Component layer functions
extern void gplib_init(INT32U free_memory_start, INT32U free_memory_end);

//Time and data reminder
typedef struct
{
    INT32S tm_sec;  /* 0-59 */
    INT32S tm_min;  /* 0-59 */
    INT32S tm_hour; /* 0-23 */
    INT32S tm_mday; /* 1-31 */
    INT32S tm_mon;  /* 1-12 */
    INT32S tm_year;
    INT32S tm_wday; /* 0-6 Sunday,Monday,Tuesday,Wednesday,Thursday,Friday,Saturday */
}TIME_T;

#define ALARM_DAY       0
//#define ALARM_ANNUAL    1

extern INT32S calendar_init(void);
extern INT32S cal_time_get(TIME_T  *tm);
extern INT32S cal_time_set(TIME_T tm);
extern void cal_factory_date_time_set(TIME_T  *tm);


// Memory management
extern void gp_mm_init(INT32U free_memory_start, INT32U free_memory_end);
extern void * gp_malloc_align(INT32U size, INT32U align);		// SDRAM allocation. Align value must be power of 2, eg: 2, 4, 8, 16, 32...
extern void * gp_malloc(INT32U size);							// SDRAM allocation
extern void gp_free(void *ptr);									// Both SDRAM and IRAM can be freed by this function


// File System
#define FAT16_Type	        		0x01
#define FAT32_Type	        		0x02
#define FAT12_Type	        		0x03
#define FORCE_FAT32_Type	        0x12
#define FORCE_FAT16_Type	        0x11

/*-----------------  seek flags  ------------------*/
#define SEEK_SET     	0 		/* offset from beginning of the file*/
#define SEEK_CUR     	1 		/* offset from current location     */
#define SEEK_END     	2 		/* offset from eof  */

/***************** open flags (the 2nd parameter)**********************/
#define O_RDONLY        0x0000
#define O_WRONLY        0x0001
#define O_RDWR          0x0002
#define O_ACCMODE       0x0003

#define O_TRUNC         0x0200 	/*    both */
#define O_CREAT         0x0400
#define O_EXCL		    0x4000	/* not fcntl */

/* File attribute constants for _findfirst() */
#define _A_NORMAL       0x00    /* Normal file - No read/write restrictions */
#define _A_RDONLY       0x01    /* Read only file */
#define _A_HIDDEN       0x02    /* Hidden file */
#define _A_SYSTEM       0x04    /* System file */
#define _A_SUBDIR       0x10    /* Subdirectory */
#define _A_ARCH         0x20    /* Archive file */

/* FAT file system attribute bits                               */
#define D_NORMAL        0       /* normal                       */
#define D_RDONLY        0x01    /* read-only file               */
#define D_HIDDEN        0x02    /* hidden                       */
#define D_SYSTEM        0x04    /* system                       */
#define D_VOLID         0x08    /* volume id                    */
#define D_DIR           0x10    /* subdir                       */
#define D_ARCHIVE       0x20    /* archive bit                  */

#define D_FILE			(0x40)	/* all attribute but D_DIR		*/
#define D_FILE_1		(0x80)	/* contain D_NORMAL,D_RDONLY,D_ARCHIVE */
#define D_ALL (D_FILE | D_RDONLY | D_HIDDEN | D_SYSTEM | D_DIR | D_ARCHIVE)

#define UNI_GBK			0
#define UNI_BIG5		1
#define UNI_SJIS		2

#define UNI_ENGLISH		0x8003
#define UNI_ARABIC		0x8004
#define UNI_UNICODE		0x8100

// file system error code
/* Internal system error returns                                */
#define SUCCESS         0       /* Function was successful      */
#define DE_INVLDFUNC    -1      /* Invalid function number      */
#define DE_FILENOTFND   -2      /* File not found               */
#define DE_PATHNOTFND   -3      /* Path not found               */
#define DE_TOOMANY      -4      /* Too many open files          */
#define DE_ACCESS       -5      /* Access denied                */
#define DE_INVLDHNDL    -6      /* Invalid handle               */
#define DE_MCBDESTRY    -7      /* Memory control blocks shot   */
#define DE_NOMEM        -8      /* Insufficient memory          */
#define DE_INVLDMCB     -9      /* Invalid memory control block */
#define DE_INVLDENV     -10     /* Invalid enviornement         */
#define DE_INVLDFMT     -11     /* Invalid format               */
#define DE_INVLDACC     -12     /* Invalid access               */
#define DE_INVLDDATA    -13     /* Invalid data                 */
#define DE_INVLDDRV     -15     /* Invalid drive                */
#define DE_RMVCUDIR     -16     /* Attempt remove current dir   */
#define DE_DEVICE       -17     /* Not same device              */
#define DE_MAX_FILE_NUM       -18     /* No more files                */
#define DE_WRTPRTCT     -19     /* No more files                */
#define DE_BLKINVLD     -20     /* invalid block                */
#define DE_INVLDBUF     -24     /* invalid buffer size, ext fnc */
#define DE_SEEK         -25     /* error on file seek           */
#define DE_HNDLDSKFULL  -28     /* handle disk full (?)         */
#define DE_INVLDPARM    -87     /* invalid parameter			*/
#define DE_UNITABERR    -88     /* unitab error					*/
#define DE_TOOMANYFILES	-89		/* to many files				*/

#define DE_DEADLOCK	-36
#define DE_LOCK		-39

#define DE_INVLDCDFILE	-48		/* invalid cd file name */
#define DE_NOTEMPTY		-49		/* DIR NOT EMPTY */
#define DE_ISDIR		-50		/* Is a directory name          */
#define DE_FILEEXISTS   -80		/* File exists                  */
#define DE_DEVICEBUSY	-90
#define DE_NAMETOOLONG	-100	/* specified path name too long */
#define DE_FILENAMEINVALID -110	/* Invalid */

struct stat_t
{
	INT16U	st_mode;
	INT32S	st_size;
	INT32U	st_mtime;
};

struct _diskfree_t {
	INT32U total_clusters;
	INT32U avail_clusters;
	INT32U sectors_per_cluster;
	INT32U bytes_per_sector;
};

struct deviceinfo 	 {
	INT8S device_name[16]; 		 // device name
	INT8S device_enable; 			 // device enable status
	INT8S device_typeFAT; 		 // device FAT type
	INT32U device_availspace; 	 // device available space
	INT32U device_capacity; 		 // device capacity
};

// data structure for _setftime()
struct timesbuf 	 {
	INT16U modtime;
	INT16U moddate;
	INT16U accdate;
};

struct f_info {
	INT8U	f_attrib;
	INT16U	f_time;
	INT16U	f_date;
	INT32U	f_size;
	INT16U	entry;
	INT8S	f_name[256];
	INT8S	f_short_name[8 + 3 + 1];
};

typedef struct {
	INT32U  f_entry;
	INT16U  f_offset;
	INT8S   f_dsk;
	INT8S	f_is_root;		// to differentiate the root folder and the first folder in root folder, in disk with FAT16/FAT12 format
} f_pos, *f_ppos;

struct sfn_info {
    INT8U   f_attrib;
	INT16U  f_time;
	INT16U  f_date;
	INT32U  f_size;
    INT8S    f_name[9];
    INT8S    f_extname[4];
    f_pos	f_pos;
};

struct nls_table {
	CHAR			*charset;
	INT16U			Status;
	INT16S			(*init_nls)(void);
	INT16S			(*exit_nls)(void);
	INT16U			(*uni2char)(INT16U uni);
	INT16U			(*char2uni)(INT8U **rawstring);
};

typedef struct
{
	f_pos		cur_pos;
	f_pos		cur_dir_pos;
	INT16U		level;
	INT16S		find_begin_file;
	INT16S		find_begin_dir;
	INT16U		index;

	INT32U		sfn_cluster;
	INT32U		sfn_cluster_offset;

	INT8U		dir_change_flag;
	INT8U		root_dir_flag;			// if the root folder have found the file, this flag is setted
	INT16U		dir_offset[16];
	INT32U		cluster[16];
	INT32U	 	cluster_offset[16];
} STDiskFindControl;

struct STFileNodeInfo
{
	INT8S		disk;					//disk
	INT16U		attr;					//set the attribute to be found
	INT8S		extname[4];				//extension
	INT8S		*pExtName;				//for extend disk find funtion, support find multi extend name
	INT8S		*path;
	INT16U		*FileBuffer;			//buffer point for file nodes
	INT16U		FileBufferSize;			//buffer size, every 20 words contain a file node, so must be multiple of 20
	INT16U		*FolderBuffer;			//buffer point for folder nodes
	INT16U		FolderBufferSize;		//buffer size, every 20 words contain a file node, so must be multiple of 20

	// the following parameter user do not care
	INT8S		flag;
	INT8U		root_dir_flag;			// if the root folder have found the file, this flag is setted
	// 08.02.27 add for search more then one kinds extern name of file
	INT16U		MarkDistance;
	INT32S		MaxFileNum;
	INT32S		MaxFolderNum;
	// 08.02.27 add end
};

typedef struct
{
	INT8U	name[11 + 1];
	INT16U	f_time;
	INT16U	f_date;
} STVolume;

// file system global variable
extern OS_EVENT *gFS_sem;
extern INT16U			gUnicodePage;
extern const struct nls_table	nls_ascii_table;
//extern const struct nls_table	nls_arabic_table;
extern const struct nls_table	nls_cp936_table;
extern const struct nls_table	nls_cp950_table;
extern const struct nls_table	nls_cp932_table;
//extern const struct nls_table	nls_cp1252_table;
/***************************************************************************/
/*        F U N C T I O N    D E C L A R A T I O N S	     			   */
/***************************************************************************/
//========================================================
//Function Name:	fs_get_version
//Syntax:		const char *fs_get_version(void);
//Purpose:		get file system library version
//Note:			the return version string like "GP$xyzz" means ver x.y.zz
//Parameters:   void
//Return:		the library version
//=======================================================
const char *fs_get_version(void);

//========================================================
//Function Name:	file_search_start
//Syntax:		INT32S file_search_start(struct STFileNodeInfo *stFNodeInfo, STDiskFindControl *pstDiskFindControl)
//Purpose:		search all the files of disk start
//Note:
//Parameters:   stFNodeInfo
//				pstDiskFindControl
//Return:		0 means SUCCESS, -1 means faile
//=======================================================
INT32S file_search_start(struct STFileNodeInfo *stFNodeInfo, STDiskFindControl *pstDiskFindControl);

//========================================================
//Function Name:	file_search_continue
//Syntax:		INT32S file_search_continue(struct STFileNodeInfo *stFNodeInfo, STDiskFindControl *pstDiskFindControl)
//Purpose:		search all the files of disk continue
//Note:
//Parameters:   stFNodeInfo
//				pstDiskFindControl
//Return:		0 means SUCCESS, 1 means search end, -1 means faile
//=======================================================
INT32S file_search_continue(struct STFileNodeInfo *stFNodeInfo, STDiskFindControl *pstDiskFindControl);

//========================================================
//Function Name:	getfirstfile
//Syntax:		f_ppos getfirstfile(INT16S dsk, CHAR *extname, struct sfn_info* f_info, INT16S attr);
//Purpose:		find the first file of the disk(will find into the folder)
//Note:
//Parameters:   dsk, extname, f_info, attr
//Return:		f_ppos
//=======================================================
f_ppos getfirstfile(INT16S dsk, CHAR *extname, struct sfn_info* f_info, INT16S attr);

//========================================================
//Function Name:	getnextfile
//Syntax:		f_ppos getnextfile(INT16S dsk, CHAR *extname, struct sfn_info* f_info, INT16S attr);
//Purpose:		find the next file of the disk(will find into the folder)
//Note:
//Parameters:   dsk, extname, f_info, attr
//Return:		f_ppos
//=======================================================
f_ppos getnextfile(INT16S dsk, CHAR *extname, struct sfn_info* f_info, INT16S attr);

//========================================================
//Function Name:	sfn_open
//Syntax:		INT16S sfn_open(f_ppos ppos);
//Purpose:		open the file that getfirstfile/getnextfile find
//Note:
//Parameters:   ppos
//Return:		file handle
//=======================================================
INT16S sfn_open(f_ppos ppos);

//========================================================
//Function Name:	sfn_stat
//Syntax:		INT16S sfn_stat(INT16S fd, struct sfn_info *sfn_info);
//Purpose:		get file attribute of an opened file
//Note:
//Parameters:   fd, sfn_info
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S sfn_stat(INT16S fd, struct sfn_info *sfn_info);

//========================================================
//Function Name:	GetFileInfo
//Syntax:		INT16S GetFileInfo(f_ppos ppos, struct f_info *f_info);
//Purpose:		get long file name infomation that getfirstfile/getnextfile find
//Note:
//Parameters:   ppos, f_info
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S GetFileInfo(f_ppos ppos, struct f_info *f_info);

//========================================================
//Function Name:	GetFolderInfo
//Syntax:		INT16S GetFolderInfo(f_ppos ppos, struct f_info *f_info);
//Purpose:		get long folder name infomation that getfirstfile/getnextfile find
//Note:
//Parameters:   ppos, f_info
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S GetFolderInfo(f_ppos ppos, struct f_info *f_info);

//========================================================
//Function Name:	sfn_unlink
//Syntax:		INT16S sfn_unlink(f_ppos ppos);
//Purpose:		delete the file that getfirstfile/getnextfile find
//Note:
//Parameters:   ppos
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S sfn_unlink(f_ppos ppos);

//========================================================
//Function Name:	StatFileNumByExtName
//Syntax:		INT16S StatFileNumByExtName(INT16S dsk, CHAR *extname, INT32U *filenum);
//Purpose:		get the file number of the disk that have the same extend name
//Note:
//Parameters:   dsk, extname, filenum
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S StatFileNumByExtName(INT16S dsk, CHAR *extname, INT32U *filenum);

//========================================================
//Function Name:	GetFileNumEx
//Syntax:		INT16S GetFileNumEx(struct STFileNodeInfo *stFNodeInfo, INT32U *nFolderNum, INT32U *nFileNum);
//Purpose:		get the file number and the folder number of the disk that have the same extend name
//Note:
//Parameters:   stFNodeInfo, nFolderNum, nFileNum
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S GetFileNumEx(struct STFileNodeInfo *stFNodeInfo, INT32U *nFolderNum, INT32U *nFileNum);

//========================================================
//Function Name:	GetFileNodeInfo
//Syntax:		f_ppos GetFileNodeInfo(struct STFileNodeInfo *stFNodeInfo, INT32U nIndex, struct sfn_info* f_info);
//Purpose:		get the file node infomation
//Note:			before run this function, ensure you have execute the function "GetFileNumEx()"
//				0 <= nIndex < nMaxFileNum
//Parameters:   stFNodeInfo, nIndex, f_info
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
f_ppos GetFileNodeInfo(struct STFileNodeInfo *stFNodeInfo, INT32U nIndex, struct sfn_info* f_info);

//========================================================
//Function Name:	GetFolderNodeInfo
//Syntax:		f_ppos GetFolderNodeInfo(struct STFileNodeInfo *stFNodeInfo, INT32U nFolderIndex, struct sfn_info* f_info);
//Purpose:		get the folder node infomation
//Note:			before run this function, ensure you have execute the function "GetFileNumEx()"
//				0 <= nIndex < nMaxFileNum
//Parameters:   stFNodeInfo, nFolderIndex, f_info
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
f_ppos GetFolderNodeInfo(struct STFileNodeInfo *stFNodeInfo, INT32U nFolderIndex, struct sfn_info* f_info);

//========================================================
//Function Name:	GetFileNumOfFolder
//Syntax:		INT16S GetFileNumOfFolder(struct STFileNodeInfo *stFNodeInfo, INT32U nFolderIndex, INT16U *nFile);
//Purpose:		get the file number of a folder
//Note:			before run this function, ensure you have execute the function "GetFileNumEx()"
//Parameters:   stFNodeInfo, nFolderIndex, nFile
//Return:		0, SUCCESS
//				-1, FAILE
//================================== =====================
INT16S GetFileNumOfFolder(struct STFileNodeInfo *stFNodeInfo, INT32U nFolderIndex, INT16U *nFile);

//========================================================
//Function Name:	FolderIndexToFileIndex
//Syntax:		INT16S FolderIndexToFileIndex(struct STFileNodeInfo *stFNodeInfo, INT32U nFolderIndex, INT32U *nFileIndex);
//Purpose:		convert folder id to file id(the index number of first file in this folder)
//Note:			before run this function, ensure you have execute the function "GetFileNumEx()"
//Parameters:   stFNodeInfo, nFolderIndex, nFileIndex
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S FolderIndexToFileIndex(struct STFileNodeInfo *stFNodeInfo, INT32U nFolderIndex, INT32U *nFileIndex);

//========================================================
//Function Name:	FileIndexToFolderIndex
//Syntax:		INT16S FileIndexToFolderIndex(struct STFileNodeInfo *stFNodeInfo, INT32U nFileIndex, INT32U *nFolderIndex);
//Purpose:		convert file id to folder id(find what folder id that the file is in)
//				根据file的index得到该file所在的folder的index
//Note:			before run this function, ensure you have execute the function "GetFileNumEx()"
//Parameters:   stFNodeInfo, nFolderIndex, nFileIndex
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S FileIndexToFolderIndex(struct STFileNodeInfo *stFNodeInfo, INT32U nFileIndex, INT32U *nFolderIndex);

//========================================================
//Function Name:	get_fnode_pos
//Syntax:		INT16S get_fnode_pos(f_pos *fpos);
//Purpose:		get the file node position after findfirst/findnext, and then you can open this file by sfn_open
//Note:			before run this function, ensure you have execute the function "_findfirst()/_findnext()"
//Parameters:   fpos
//Return:		0, SUCCESS
//=======================================================
INT16S get_fnode_pos(f_pos *fpos);

//f_ppos getfirstfileEx(INT8S *path, CHAR *extname, struct sfn_info *f_info, INT16S attr);
//f_ppos getnextfileEx(CHAR * extname, struct sfn_info* f_info, INT16S attr);

//========================================================
//Function Name:	dosdate_decode
//Syntax:		void dosdate_decode(INT16U dos_date, INT16U *pyear, INT8U *pmonth, INT8U *pday);
//Purpose:		convert the dos_data to year, month, day
//Note:
//Parameters:   dos_date, pyear, pmonth, pday
//Return:		void
//=======================================================
void dosdate_decode(INT16U dos_date, INT16U *pyear, INT8U *pmonth, INT8U *pday);

//========================================================
//Function Name:	dostime_decode
//Syntax:		void dostime_decode(INT16U dos_time, INT8U *phour, INT8U *pminute, INT8U *psecond);
//Purpose:		convert the dos_time to hour, minute, second
//Note:
//Parameters:   dos_time, phour, pminute, psecond
//Return:		void
//=======================================================
void dostime_decode(INT16U dos_time, INT8U *phour, INT8U *pminute, INT8U *psecond);

//========================================================
//Function Name:	time_decode
//Syntax:		INT8S *time_decode(INT16U *tp, CHAR *timec);
//Purpose:		convert *tp to a string like "hh:mm:ss"
//Note:
//Parameters:   tp, timec
//Return:		the point of string
//=======================================================
INT8S *time_decode(INT16U *tp, CHAR *timec);

//========================================================
//Function Name:	date_decode
//Syntax:		INT8S *date_decode(INT16U *dp, CHAR *datec);
//Purpose:		convert *dp to a string like "yyyy-mm-dd"
//Note:
//Parameters:   dp, datec
//Return:		the point of string
//=======================================================
INT8S *date_decode(INT16U *dp, CHAR *datec);

//========================================================
//Function Name:	fs_safexit
//Syntax:		void fs_safexit(void);
//Purpose:		close all the opened files except the registed file
//Note:
//Parameters:   NO
//Return:		void
//=======================================================
void fs_safexit(void);

//========================================================
//Function Name:	fs_registerfd
//Syntax:		void fs_registerfd(INT16S fd);
//Purpose:		regist opened file so when you call function fs_safexit() this file will not close
//Note:
//Parameters:   fd
//Return:		void
//=======================================================
void fs_registerfd(INT16S fd);

//========================================================
//Function Name:	disk_safe_exit
//Syntax:		void disk_safe_exit(INT16S dsk);
//Purpose:		close all the opened files of the disk
//Note:
//Parameters:   dsk
//Return:		void
//=======================================================
void disk_safe_exit(INT16S dsk);

//========================================================
//Function Name:	open
//Syntax:		INT16S open(CHAR *path, INT16S open_flag);
//Purpose:		open/creat file
//Note:
//Parameters:   path, open_flag
//Return:		file handle
//=======================================================
INT16S open(CHAR *path, INT16S open_flag);

void F_OS_AdjustCrtTimeEnable(void);
void F_OS_AdjustCrtTimeDisable(void);

//========================================================
//Function Name:	close
//Syntax:		INT16S close(INT16S fd);
//Purpose:		close file
//Note:
//Parameters:   fd
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S close(INT16S fd);

//========================================================
//Function Name:	read
//Syntax:		INT32S read(INT16S fd, INT32U buf, INT32U size);
//Purpose:		read data
//Note:			the buffer is BYTE address, the size is BYTE size
//Parameters:   fd, buf, size
//Return:		really read size
//=======================================================
INT32S read(INT16S fd, INT32U buf, INT32U size);

//========================================================
//Function Name:	write
//Syntax:		INT32S write(INT16S fd, INT32U buf, INT32U size);
//Purpose:		write data
//Note:			the buffer is BYTE address, the size is BYTE size
//Parameters:   fd, buf, size
//Return:		really write size
//=======================================================
INT32S write(INT16S fd, INT32U buf, INT32U size);

//========================================================
//Function Name:	lseek
//Syntax:		INT32S lseek(INT16S fd,INT32S offset,INT16S fromwhere);
//Purpose:		change data point of file
//Note:			use lseek(fd, 0, SEEK_CUR) can get current offset of file.
//Parameters:   fd, offset, fromwhere
//Return:		data point
//=======================================================
INT32S lseek(INT16S fd,INT32S offset,INT16S fromwhere);

//========================================================
//Function Name:	unlink
//Syntax:		INT16S unlink(CHAR *filename);
//Purpose:		delete the file
//Note:
//Parameters:   filename
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S unlink(CHAR *filename);

//========================================================
//Function Name:	rename
//Syntax:		INT16S _rename(CHAR *oldname, CHAR *newname);
//Purpose:		change file name
//Note:
//Parameters:   oldname, newname
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S _rename(CHAR *oldname, CHAR *newname);

//========================================================
//Function Name:	mkdir
//Syntax:		INT16S mkdir(CHAR *pathname);
//Purpose:		cread a folder
//Note:
//Parameters:   pathname
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S mkdir(CHAR *pathname);

//========================================================
//Function Name:	rmdir
//Syntax:		INT16S rmdir(CHAR *pathname);
//Purpose:		delete a folder
//Note:			the folder must be empty
//Parameters:   pathname
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S rmdir(CHAR *pathname);

//========================================================
//Function Name:	chdir
//Syntax:		INT16S chdir(CHAR *path);
//Purpose:		change current path to new path
//Note:
//Parameters:   path
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S chdir(CHAR *path);

//========================================================
//Function Name:	getcwd
//Syntax:		INT32U getcwd(CHAR *buffer, INT16S maxlen );
//Purpose:		get current path
//Note:
//Parameters:   buffer, maxlen
//Return:		the path name string point
//=======================================================
INT32U getcwd(CHAR *buffer, INT16S maxlen );

//========================================================
//Function Name:	fstat
//Syntax:		INT16S fstat(INT16S handle, struct stat_t *statbuf);
//Purpose:		get file infomation
//Note:			the file must be open
//Parameters:   handle, statbuf
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S fstat(INT16S handle, struct stat_t *statbuf);

//========================================================
//Function Name:	stat
//Syntax:		INT16S stat(CHAR *path, struct stat_t *statbuf);
//Purpose:		get file infomation
//Note:
//Parameters:   path, statbuf
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S stat(CHAR *path, struct stat_t *statbuf);

//========================================================
//Function Name:	_findfirst
//Syntax:		INT16S _findfirst(CHAR *name, struct f_info *f_info, INT16U attr);
//Purpose:		find the first file in one folder
//Note:
//Parameters:   name, f_info, attr
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S _findfirst(CHAR *name, struct f_info *f_info, INT16U attr);

//========================================================
//Function Name:	_findnext
//Syntax:		INT16S _findnext(struct f_info *f_info);
//Purpose:		find next file in one folder
//Note:
//Parameters:   f_info
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S _findnext(struct f_info *f_info);

//========================================================
//Function Name:	_getdiskfree
//Syntax:		INT16S _getdiskfree(INT16S dsk, struct _diskfree_t *st_free);
//Purpose:		get disk total space and free space
//Note:
//Parameters:   dsk, st_free
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S _getdiskfree(INT16S dsk, struct _diskfree_t *st_free);

//========================================================
//Function Name:	vfsFreeSpace
//Syntax:		INT32S vfsFreeSpace(INT16S driver);
//Purpose:		get disk free space
//Note:
//Parameters:   dsk, st_free
//Return:		the free space of the disk
//=======================================================
//INT32S vfsFreeSpace(INT16S driver);
INT64U vfsFreeSpace(INT16S driver);

//========================================================
//Function Name:	_changedisk
//Syntax:		INT16S _changedisk(INT8U disk);
//Purpose:		change current disk to another disk
//Note:
//Parameters:   disk
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
#define _changedisk     fs_changedisk
INT16S _changedisk(INT8U disk);

//========================================================
//Function Name:	_copy
//Syntax:		INT16S _copy(CHAR *path1, CHAR *path2);
//Purpose:		copy file
//Note:
//Parameters:   srcfile, destfile
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S _copy(CHAR *path1, CHAR *path2);

//========================================================
//Function Name:	fs_init
//Syntax:		void fs_init(void);
//Purpose:		initial all file system global variable
//Note:
//Parameters:   NO
//Return:		void
//=======================================================
void fs_init(void);

//========================================================
//Function Name:	fs_uninit
//Syntax:		INT16S fs_uninit(void);
//Purpose:		free file system resource, and unmount all disk
//Note:
//Parameters:   NO
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S fs_uninit(void);

//========================================================
//Function Name:	tellcurrentfiledir
//Syntax:		INT16U tellcurrentfiledir(void);
//Purpose:		get current directory entry point
//Note:
//Parameters:   NO
//Return:		directory entry point
//=======================================================
INT16U tellcurrentfiledir(void);

//========================================================
//Function Name:	telldir
//Syntax:		INT16U telldir(void);
//Purpose:		get next directory entry point
//Note:
//Parameters:   NO
//Return:		directory entry point
//=======================================================
INT16U telldir(void);

//========================================================
//Function Name:	seekdir
//Syntax:		void seekdir(INT16U pos);
//Purpose:		set directory entry point, and next time, if you call _findnext(),
//				the function will find file from this point
//Note:
//Parameters:   directory entry point
//Return:		NO
//=======================================================
void seekdir(INT16U pos);     //the parameter "pos" must be the return value of "telldir"

//========================================================
//Function Name:	rewinddir
//Syntax:		void rewinddir(void);
//Purpose:		reset directory entry point to 0
//Note:
//Parameters:   NO
//Return:		NO
//=======================================================
void rewinddir(void);

//========================================================
//Function Name:	_setfattr
//Syntax:		INT16S _setfattr(CHAR *filename, INT16U attr);
//Purpose:		set file attribute
//Note:
//Parameters:   filename, attr
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S _setfattr(CHAR *filename, INT16U attr);

//========================================================
//Function Name:	_setdirattr
//Syntax:		INT16S _setdirattr(CHAR *dirname, INT16U attr);
//Purpose:		set dir attribute
//Note:
//Parameters:   filename, attr
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S _setdirattr(CHAR *dirname, INT16U attr);

//========================================================
//Function Name:	_getdirattr
//Syntax:		INT16S _getdirattr(CHAR *dirname, INT16U *attr);
//Purpose:		get dir attribute
//Note:
//Parameters:   filename, attr
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S _getdirattr(CHAR *dirname, INT16U *attr);

//========================================================
//Function Name:	_devicemount
//Syntax:		INT16S _devicemount(INT16S disked);
//Purpose:		mount disk, then you can use the disk
//Note:
//Parameters:   disk
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S _devicemount(INT16S disked);

//========================================================
//Function Name:	_deviceunmount
//Syntax:		INT16S _deviceunmount(INT16S disked);
//Purpose:		unmount disk
//Note:
//Parameters:   disk
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S _deviceunmount(INT16S disked);

//========================================================
//Function Name:	_getfserrcode
//Syntax:		INT16S _getfserrcode(void);
//Purpose:		get error code(see error.h)
//Note:
//Parameters:   NO
//Return:		error code
//=======================================================
INT16S _getfserrcode(void);

//========================================================
//Function Name:	_clsfserrcode
//Syntax:		void _clsfserrcode(void);
//Purpose:		clear error code to 0
//Note:
//Parameters:   NO
//Return:		void
//=======================================================
void _clsfserrcode(void);

//========================================================
//Function Name:	_format
//Syntax:		INT16S _format(INT8U drv, INT8U fstype);
//Purpose:		format disk to FAT32 or FAT16 type
//Note:
//Parameters:   dsk, fstype
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S _format(INT8U drv, INT8U fstype);

//========================================================
//Function Name:	_deleteall
//Syntax:		INT16S _deleteall(CHAR *filename);
//Purpose:		delete all file and folder in one folder
//Note:
//Parameters:   path
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S _deleteall(CHAR *filename);

//========================================================
//Function Name:	GetSectorsPerCluster
//Syntax:		UINT16 GetSectorsPerCluster(UINT16 dsk)
//Purpose:		get Sector number per cluster
//Note:
//Parameters:   dsk
//Return:		sector number
//=======================================================
INT16U GetSectorsPerCluster(INT16U dsk);

//========================================================
//Function Name:	_GetCluster
//Syntax:		INT32S _GetCluster(INT16S fd);
//Purpose:		get cluster id that data point now locate
//Note:
//Parameters:   fd
//Return:		cluster id
//=======================================================
INT32S _GetCluster(INT16S fd);

//========================================================
//Function Name:	Clus2Phy
//Syntax:		INT32S Clus2Phy(INT16U dsk, INT32U cl_no);
//Purpose:		convert cluster id to sector address
//Note:
//Parameters:   dsk, cl_no
//Return:		sector address
//=======================================================
INT32S Clus2Phy(INT16U dsk, INT32U cl_no);

//========================================================
//Function Name:	DeletePartFile
//Syntax:		INT16S DeletePartFile(INT16S fd, INT32U offset, INT32U length);
//Purpose:		delete part of file, from "offset", delete "length" byte
//Note:			the file system will convert the "offset" and "length" to cluster size
//Parameters:   fd, offset, length
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S DeletePartFile(INT16S fd, INT32U offset, INT32U length);

//========================================================
//Function Name:	InserPartFile
//Syntax:		INT16S InserPartFile(INT16S tagfd, INT16S srcfd, INT32U tagoff, INT32U srclen);
//Purpose:		insert the src file to tag file
//Note:			the file system will convert the "offset" and "length" to cluster size
//Parameters:   tagfd, srcfd, tagoff, srclen
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S InserPartFile(INT16S tagfd, INT16S srcfd, INT32U tagoff, INT32U srclen);

//========================================================
//Function Name:	InserPartFile
//Syntax:		INT16 InserPartFile(INT16 tagfd, INT16 srcfd, UINT32 tagoff, UINT32 srclen)
//Purpose:		split tag file into two file, one is remain in tag file, one is in src file
//Note:			the file system will convert the "offset" and "length" to cluster size
//Parameters:   tagfd, srcfd, splitpoint
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S SplitFile(INT16S tagfd, INT16S srcfd, INT32U splitpoint);

//========================================================
//Function Name:	ChangeCodePage
//Syntax:		INT16U ChangeCodePage(INT16U wCodePage);
//Purpose:		select unicode page
//Note:			if the code page is not exsit, the file system will default change code page to "ascii"
//Parameters:   wCodePage
//Return:		code page
//=======================================================
INT16U ChangeCodePage(INT16U wCodePage);

//========================================================
//Function Name:	GetCodePage
//Syntax:		INT16U GetCodePage(void);
//Purpose:		get unicode page
//Note:
//Parameters:   NO
//Return:		code page
//=======================================================
INT16U GetCodePage(void);

//========================================================
//Function Name:	ChangeUnitab
//Syntax:		INT16S ChangeUnitab(struct nls_table *st_nls_table);
//Purpose:		change unicode convert struct
//Note:
//Parameters:   st_nls_table
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S ChangeUnitab(struct nls_table *st_nls_table);

//========================================================
//Function Name:	checkfattype
//Syntax:		INT16S checkfattype(INT8U disk);
//Purpose:		get the fat type of the disk(FAT16 or FAT32)
//Note:
//Parameters:   disk
//Return:		fat type
//=======================================================
INT16S checkfattype(INT8U disk);

//========================================================
//Function Name:	UpdataDir
//Syntax:		INT16S UpdataDir(INT16S fd);
//Purpose:		updata dir information but not close the file
//Note:
//Parameters:   fd
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S UpdataDir(INT16S fd);

//========================================================
//Function Name:	FileRepair
//Syntax:		INT16S FileRepair(INT16S fd);
//Purpose:		if the file is destroy for some reason, this function will repair the file
//Note:			it can't deal with some complicated condition
//Parameters:   fd
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S FileRepair(INT16S fd);

//========================================================
//Function Name:	sformat
//Syntax:		INT16S sformat(INT8U drv, INT32U totalsectors, INT32U realsectors);
//Purpose:		format some disk that size is less than 16 MB
//Note:
//Parameters:   drv, totalsectors, realsectors
//Return:		0, SUCCESS
//				-1, FAILE
//=======================================================
INT16S sformat(INT8U drv, INT32U totalsectors, INT32U realsectors);

//========================================================
//Function Name:	GetDiskOfFile
//Syntax:		INT16S GetDiskOfFile(INT16S fd);
//Purpose:		get the disk id of an opened file
//Note:
//Parameters:   fd
//Return:		disk id, 0 is disk "A", 1 is disk "B", and itc...
//=======================================================
INT16S GetDiskOfFile(INT16S fd);

//========================================================
//Function Name:	CreatFileBySize
//Syntax:		INT16S CreatFileBySize(CHAR *path, INT32U size);
//Purpose:		creat a file, and allocate "size" byte space
//Note:			size is byte size
//Parameters:   filename, size
//Return:		file handle
//=======================================================
INT16S CreatFileBySize(CHAR *path, INT32U size);
extern INT16S file_cat(INT16S file1_handle, INT16S file2_handle);
extern BOOLEAN flush_fat_buffers(INT16S dsk);
extern INT16S file_fast_cat(INT16S file1_handle, INT16S file2_handle);

INT16S get_volume(INT8U disk_id, STVolume *pstVolume);
INT16S set_volume(INT8U disk_id, INT8U *p_volum);


void unlink_step_flush(void);
void unlink_step_start(void);
INT32S unlink_step_work(void);

//JPEG

typedef enum
{
    ENUM_JPEG_LUMINANCE_QTABLE = 0,
    ENUM_JPEG_CHROMINANCE_QTABLE
} JPEG_QTABLE_ENUM;

// JPEG encoder definitions
#define C_JPEG_FORMAT_YUV_SEPARATE		0x0
#define C_JPEG_FORMAT_YUYV				0x1

#if 1
// JPEG general API
extern INT32U gplib_jpeg_version_get(void);
extern void gplib_jpeg_default_huffman_table_load(void);

// JPEG encoder initiate API
extern void jpeg_encode_init(void);

// JPEG encoder input buffer relative APIs
extern INT32S jpeg_encode_input_size_set(INT16U width, INT16U height);			// Width and height of the image that will be compressed
extern INT32S jpeg_encode_input_format_set(INT32U format);						// format: C_JPEG_FORMAT_YUV_SEPARATE or C_JPEG_FORMAT_YUYV

// JPEG encoder output buffer relative APIs
extern INT32S jpeg_encode_yuv_sampling_mode_set(INT32U encode_mode);			// encode_mode: C_JPG_CTRL_YUV422 or C_JPG_CTRL_YUV420(only valid for C_JPEG_FORMAT_YUV_SEPARATE format)
extern INT32S jpeg_encode_output_addr_set(INT32U addr);							// The address that VLC(variable length coded) data will be output

// JPEG encoder start, restart and stop APIs
extern void jpeg_enable_scale_x2_engine(void);
extern INT32S jpeg_encode_once_start(INT32U y_addr, INT32U u_addr, INT32U v_addr);	// If input format is YUV separate, both y_addr, u_addr and v_addr must be 8-byte alignment. If input format is YUYV, only y_addr(16-byte alignment) is used.
extern INT32S jpeg_encode_on_the_fly_start(INT32U y_addr, INT32U u_addr, INT32U v_addr, INT32U len);	// If input format is YUV separate, both y_addr, u_addr and v_addr must be 8-byte alignment. If input format is YUYV, only y_addr(16-byte alignment) is used.
extern void jpeg_encode_stop(void);

// JPEG encoder status query API
extern INT32S jpeg_encode_status_query(INT32U wait);
extern INT32U jpeg_encode_vlc_cnt_get(void);
#endif
extern void jpeg_header_quantization_table_calculate(JPEG_QTABLE_ENUM table, INT32U quality, INT8U *qtable);

// JPEG decoder definitions
#define JPEG_PARSE_OK				0
#define JPEG_PARSE_NOT_DONE			1
#define JPEG_PARSE_BMP				101
#define JPEG_PARSE_GIF				102
#define JPEG_PARSE_PNG				103
#define JPEG_PARSE_FAIL				-1
#define JPEG_SET_HW_ERROR			-2

#if 1
// JPEG decoder initiate API
extern void jpeg_decode_init(void);

// JPEG decoder header parser APIs
extern INT32S jpeg_decode_parse_header(INT8U *buf_start, INT32U len);	// buf_start points to buffer address of JPEG file
extern INT16U jpeg_decode_image_width_get(void);					// Get image width after header parsing finish
extern INT16U jpeg_decode_image_height_get(void);					// Get image height after header parsing finish
extern INT16U jpeg_decode_image_extended_width_get(void);			// Get image extended width(according to YUV mode)
extern INT16U jpeg_decode_image_extended_height_get(void);			// Get image extended height(according to YUV mode)
extern INT16U jpeg_decode_image_yuv_mode_get(void);					// Get image YUV mode after header parsing finish
extern INT8U jpeg_decode_image_progressive_mode_get(void);			// Get image progressive mode (0=baseline, 1=progressive)
extern INT32S jpeg_decode_date_time_get(INT8U *target);
extern INT8U jpeg_decode_thumbnail_exist_get(void);					// Get information about thumbnail image(0=No thumbnail image, 1=Thumbnail image exists)
extern INT16U jpeg_decode_thumbnail_width_get(void);
extern INT16U jpeg_decode_thumbnail_height_get(void);
extern INT16U jpeg_decode_thumbnail_extended_width_get(void);
extern INT16U jpeg_decode_thumbnail_extended_height_get(void);
extern INT16U jpeg_decode_thumbnail_yuv_mode_get(void);
extern INT8U * jpeg_decode_image_vlc_addr_get(void);				// Get start address of entropy encoded data after header parsing finish

// JPEG decoder input relative APIs
extern INT32S jpeg_decode_vlc_maximum_length_set(INT32U len);		// Specify the maximum length that JPEG will read to decode image. Using this API is optional. It is used to prevent JPEG engine from hangging when bad JPEG file is read. The value of length should be >= real VLC length(eg: using file size as its value).

// JPEG decoder output relative APIs
extern INT32S jpeg_decode_output_set(INT32U y_addr, INT32U u_addr, INT32U v_addr, INT32U line);	// Addresses must be 8-byte alignment, line:C_JPG_FIFO_DISABLE/C_JPG_FIFO_ENABLE/C_JPG_FIFO_16LINE/C_JPG_FIFO_32LINE/C_JPG_FIFO_64LINE/C_JPG_FIFO_128LINE/C_JPG_FIFO_256LINE
extern void jpeg_decode_thumbnail_image_enable(void);				// Decode thumbnail image of file instead of original image
extern void jpeg_decode_thumbnail_image_disable(void);				// Decode original image of file
extern INT32S jpeg_decode_clipping_range_set(INT16U start_x, INT16U start_y, INT16U width, INT16U height);	// x, y, width, height must be at lease 8-byte alignment
extern INT32S jpeg_decode_clipping_mode_enable(void);
extern INT32S jpeg_decode_clipping_mode_disable(void);
extern INT32S jpeg_decode_level2_scaledown_enable(void);			// When scale-down level 2 mode is enabled, output size will be 1/2 in width and height
extern INT32S jpeg_decode_level2_scaledown_disable(void);
extern INT32S jpeg_decode_level8_scaledown_enable(void);			// When scale-down level 8 mode is enabled, output size will be 1/8 in width and height
extern INT32S jpeg_decode_level8_scaledown_disable(void);

// JPEG decoder start, restart and stop APIs
#define jpeg_decode_single_vlc_set	jpeg_decode_once_start
#define jpeg_decode_vlc_on_the_fly	jpeg_decode_on_the_fly_start
extern INT32S jpeg_decode_once_start(INT8U *buf, INT32U len);		// When the complete VLC data is available, using this API to decode JPEG image once.
extern INT32S jpeg_decode_on_the_fly_start(INT8U *buf, INT32U len);	// When only partial of the VLC data is available, using this API to decode it. buf must be 16-byte alignment except the first one
extern INT32S jpeg_decode_on_the_fly_start2(INT8U *buf, INT32U len);	// When only partial of the VLC data is available, using this API to decode it. buf must be 16-byte alignment except the first one && Restart decompression when output FIFO buffer is full. This function should be called after Scaler has finished handling the previous FIFO buffer.
extern INT32S jpeg_decode_output_restart(void);						// Restart decompression when output FIFO buffer is full. This function should be called after Scaler has finished handling the previous FIFO buffer.
extern void jpeg_decode_stop(void);

// JPEG decoder status query API
extern INT32S jpeg_decode_status_query(INT32U wait);				// If wait is 0, this function returns immediately. Return value:C_JPG_STATUS_INPUT_EMPTY/C_JPG_STATUS_OUTPUT_FULL/C_JPG_STATUS_DECODE_DONE/C_JPG_STATUS_ENCODE_DONE/C_JPG_STATUS_STOP/C_JPG_STATUS_TIMEOUT/C_JPG_STATUS_INIT_ERR

// Progressive JPEG decoder APIs
#define JPEG_SUSPENDED								0x80000001	// Suspended due to lack of input data

#define JPEG_BS_NOT_ENOUGH							0	// Data in the bitstream buffer are not enough
#define JPEG_REACHED_SOS							1	// Reached start of new scan
#define JPEG_REACHED_EOI							2	// Reached end of image
#define JPEG_ROW_COMPLETED							3	// Completed one iMCU row
#define JPEG_SCAN_COMPLETED							4	// Completed last iMCU row of a scan
#define JPEG_FILE_END								5	// File Read End
#define JPEG_MCU_COMPLETED							6
#define JPEG_PASS_AC_REFINE							7	// To pass ac refine data segment
#define JPEG_HEADER_OK								8	// Found valid image datastream
#endif

#if 0
// Motion-JPEG decoder init API
extern void mjpeg_decode_init(void);

// Motion-JPEG decoder APIs for setting output parameters
extern INT32S mjpeg_decode_output_format_set(INT32U format);
extern INT32S mjpeg_decode_output_addr_set(INT32U y_addr, INT32U u_addr, INT32U v_addr);	// Must be 4-byte alignment

// Motion-JPEG decoder APIs for start, restart and stop function
extern INT32S mjpeg_decode_once_start(INT8U *buf, INT32U len);			// The complete VLC data is available, decode JPEG image once.
extern INT32S mjpeg_decode_on_the_fly_start(INT8U *buf, INT32U len);	// Partial of the VLC data is available. buf must be 16-byte alignment except the first call to this API
extern void mjpeg_decode_stop(void);

// Motion-JPEG decoder API for status query
extern INT32S mjpeg_decode_status_query(INT32U wait);					// If wait is 0, this function returns immediately. Return value: C_JPG_STATUS_SCALER_DONE/C_JPG_STATUS_INPUT_EMPTY/C_JPG_STATUS_STOP/C_JPG_STATUS_TIMEOUT/C_JPG_STATUS_INIT_ERR
#endif

// GPZP
extern INT32S gpzp_decode(INT8U * input_buffer ,INT8U  * output_buffer);

// Print and get string from/to UART
extern void print_string(CHAR *fmt, ...);
extern void get_string(CHAR *s);


#define GPLIB_MP3_HW_EN 	0

/* MP3 */
#define MP3_DEC_FRAMESIZE					1152	// ???
#define MP3_DEC_BITSTREAM_BUFFER_SIZE   	4096    // size in bytes
#define MP3_DEC_MEMORY_SIZE 				14008	//(20632-8)	// 18456
/* MP3 error code */
#define MP3_DEC_ERR_NONE			0x00000000	/* no error */

#define MP3_DEC_ERR_BUFLEN	   	   	0x80000001	/* input buffer too small (or EOF) */
#define MP3_DEC_ERR_BUFPTR	   	   	0x80000002	/* invalid (null) buffer pointer */

#define MP3_DEC_ERR_NOMEM	   	   	0x80000031	/* not enough memory */

#define MP3_DEC_ERR_LOSTSYNC	   	0x80000101	/* lost synchronization */
#define MP3_DEC_ERR_BADLAYER	   	0x80000102	/* reserved header layer value */
#define MP3_DEC_ERR_BADBITRATE	   	0x80000103	/* forbidden bitrate value */
#define MP3_DEC_ERR_BADSAMPLERATE  	0x80000104	/* reserved sample frequency value */
#define MP3_DEC_ERR_BADEMPHASIS	   	0x80000105	/* reserved emphasis value */
#define MP3_DEC_ERR_BADMPEGID		0x80000106	//for error mpegid add by zgq on 20080508

#define MP3_DEC_ERR_BADCRC	   	   	0x80000201	/* CRC check failed */
#define MP3_DEC_ERR_BADBITALLOC	   	0x80000211	/* forbidden bit allocation value */
#define MP3_DEC_ERR_BADSCALEFACTOR  0x80000221	/* bad scalefactor index */
#define MP3_DEC_ERR_BADMODE         0x80000222	/* bad bitrate/mode combination */
#define MP3_DEC_ERR_BADFRAMELEN	    0x80000231	/* bad frame length */
#define MP3_DEC_ERR_BADBIGVALUES    0x80000232	/* bad big_values count */
#define MP3_DEC_ERR_BADBLOCKTYPE    0x80000233	/* reserved block_type */
#define MP3_DEC_ERR_BADSCFSI	    0x80000234	/* bad scalefactor selection info */
#define MP3_DEC_ERR_BADDATAPTR	    0x80000235	/* bad main_data_begin pointer */
#define MP3_DEC_ERR_BADPART3LEN	    0x80000236	/* bad audio data length */
#define MP3_DEC_ERR_BADHUFFTABLE    0x80000237	/* bad Huffman table select */
#define MP3_DEC_ERR_BADHUFFDATA	    0x80000238	/* Huffman data overrun */
#define MP3_DEC_ERR_BADSTEREO	    0x80000239	/* incompatible block_type for JS */

#if GPLIB_MP3_HW_EN == 1

// MP3 Decoder Version
extern const char * mp3_dec_get_version(void);
// MP3 Decoder Initial
extern int mp3_dec_init(void *p_workmem, void *p_bsbuf);
// MP3 set ring size
extern int mp3_dec_set_ring_size(void *p_workmem, int size);
// MP3 header parsing
extern int mp3_dec_parsing(void *p_workmem, int wi);
// MP3 Decoder
extern int mp3_dec_run(void *p_workmem, short *p_pcmbuf, int wi, int granule);
// Get Read Index
extern int mp3_dec_get_ri(void *p_workmem);
// p_workmem:    pointer to working memory
// return value: read index of bitstream ring buffer


// Get mpeg id
extern const char *mp3_dec_get_mpegid(void *p_workmem);
// return value: MP3 Decoder Working memory size
extern int mp3_dec_get_mem_block_size (void);
// return error number.
extern int mp3_dec_get_errno(void *p_workmem);
// return layer.
extern int mp3_dec_get_layer(void *p_workmem);
// return channel.
extern int mp3_dec_get_channel(void *p_workmem);
// return bitrate in kbps.
extern int mp3_dec_get_bitrate(void *p_workmem);
// return sampling rate in Hz.

extern int mp3_dec_end(void *p_workmem,int wi);

#endif

extern void mp3_dec_set_ri(CHAR *mp3dec_workmem, INT32S ri); 

extern int mp3_dec_get_samplerate(void *p_workmem);

//MP3 Encoder
#define  MP3_ENC_WORKMEM_SIZE  31704

// layer3.c //
extern int mp3enc_init(
	void *pWorkMem,
	int nChannels,
	int nSamplesPreSec,
	int nKBitsPreSec,
	int Copyright,
	char *Ring,
	int RingBufSize,
	int RingWI);

extern int mp3enc_encframe(void *pWorkMem, const short *PCM);
extern int mp3enc_end(void *pWorkMem);



#define MP3ENC_ERR_INVALID_CHANNEL		0x80000001
#define MP3ENC_ERR_INVALID_SAMPLERATE	0x80000002
#define MP3ENC_ERR_INVALID_BITRATE		0x80000003



const char *mp3enc_GetErrString(int ErrCode);
const char *mp3enc_GetVersion(void);
int mp3enc_GetWorkMemSize(void);




/* WMA */
#define WMA_DEC_FRAMESIZE        2048
#define WMA_DEC_MEMORY_SIZE      45000
#define WMA_DEC_BITSTREAM_BUFFER_SIZE 4096

extern int			wma_dec_run(CHAR *p_workmem, short *p_pcmbuf, int wi);
extern int			wma_dec_init(CHAR *wmadec_workmem, INT8U *bs_buf);
extern int			wma_dec_parsing(CHAR *wmadec_workmem,int wi);
extern int			wma_dec_get_ri(CHAR *wmadec_workmem);
extern int			wma_dec_get_errno(CHAR *wmadec_workmem);
extern const INT8U * wma_dec_get_version(void);
extern int			wma_dec_get_mem_block_size(void);
extern int			wma_dec_get_samplerate(CHAR *wmadec_workmem);
extern int			wma_dec_get_channel(CHAR *wmadec_workmem);
extern int			wma_dec_get_bitspersample(CHAR *wmadec_workmem);
extern int			wma_dec_get_bitrate(CHAR *wmadec_workmem);
extern int			wma_dec_SetRingBufferSize(char *wmadec_workmem, int size);

// SUCCESS codes
#define WMA_OK						0x00000000
#define WMA_S_NEWPACKET				0x00000001
#define WMA_S_NO_MORE_FRAME			0x00000002
#define WMA_S_NO_MORE_SRCDATA		0x00000003
#define WMA_S_LOSTPACKET			0x00000004
#define WMA_S_CORRUPTDATA			0x00000005

// ERROR codes
#define WMA_E_FAIL					0x80004005
#define WMA_E_INVALIDARG			0x80070057
#define WMA_E_NOTSUPPORTED			0x80040000
#define WMA_E_BROKEN_FRAME			0x80040002
#define WMA_E_ONHOLD				0x80040004
#define WMA_E_NO_MORE_SRCDATA		0x80040005
#define WMA_E_WRONGSTATE			0x8004000A

#define WMA_E_BAD_PACKET_HEADER		0x80000011
#define WMA_E_NO_MORE_FRAMES		0x80000012
#define WMA_E_BAD_DRM_TYPE			0x80000013
#define WMA_E_INTERNAL				0x80000014
#define WMA_E_NOMOREDATA_THISTIME	0x80000015
#define	WMA_E_READ_IN_BUFFER		0x80000016
#define WMA_E_INVALIDHEADER			0x80000017
#define WMA_E_BUFFERTOOSMALL		0x80000018


/* WAVE */
#define WAVE_OUT_INTERLEAVED

#define WAV_DEC_FRAMESIZE				2048
#define WAV_DEC_BITSTREAM_BUFFER_SIZE   4096
#define WAV_DEC_MEMORY_SIZE				96

//=======wave format tag====================
#define	WAVE_FORMAT_PCM				(0x0001)
#define	WAVE_FORMAT_ADPCM			(0x0002)
#define	WAVE_FORMAT_ALAW			(0x0006)
#define	WAVE_FORMAT_MULAW			(0x0007)
#define WAVE_FORMAT_IMA_ADPCM		(0x0011)

//========wave decoder error NO.====================
#define WAV_DEC_NO_ERROR    			0
#define WAV_DEC_INBUFFER_NOT_ENOUGH		0x80000001
#define WAV_DEC_RIFF_HEADER_NOT_FOUND	0x80000002
#define WAV_DEC_HEADER_NOT_FOUND		0x80000003
#define WAV_DEC_CHUNK_ALIGN_ERROR		0x80000004
#define WAV_DEC_DATA_CHUNK_NOT_FOUND	0x80000005
#define WAV_DEC_FMT_CHUNK_TOO_SHORT		0x80000006
#define WAV_DEC_CHANNEL_ERROR			0x80000007
#define WAV_DEC_BIT_DEPTH_ERROR			0x80000008
#define WAV_DEC_CHUNK_EXTEND_ERROR		0x80000009
#define WAV_DEC_MSADPCM_COEF_ERROR		0x8000000A
#define WAV_DEC_UNKNOWN_FORMAT			0x8000000B

int wav_dec_init(INT8U *p_workmem, const INT8U *p_bsbuf);
// p_bsbuf:      pointer to bit-stream buffer
// p_workmem:    pointer to working memory
// return value: 0 success

int wav_dec_parsing(INT8U *p_workmem, INT32U wi);//, WAVEFORMATEX *p_wave_format
// p_workmem:    pointer to working memory
// wi:           write index of bitstream ring buffer
// return value: 0 success
int wav_dec_set_param(INT8U *p_workmem, const INT8U *p_WaveFormatEx);
//this function is only used for AVI

int wav_dec_run(INT8U *p_workmem, short *p_pcmbuf, INT32U wi);
// p_ pcmbuf:    pointer to PCM buffer
// p_workmem:    pointer to working memory
// wi:           write index of bitstream ring buffer
// return value:
//	positive : samples of PCM
//  zero:      not enough bitstream data
//  negtive:   error

int wav_dec_get_ri(INT8U *p_workmem);
// p_workmem:    pointer to working memory
// return value: read index of bitstream ring buffer

int wav_dec_get_mem_block_size(void);
// return value: Wave Decoder Working memory size

int wav_dec_get_wFormatTag(INT8U *p_workmem);
// p_workmem:    pointer to working memory
// return value: wave format tag

int wav_dec_get_nChannels(INT8U *p_workmem);
// p_workmem:    pointer to working memory
// return value: number of channels

int wav_dec_get_SampleRate(INT8U *p_workmem);
// p_workmem:    pointer to working memory
// return value: sample rate

int wav_dec_get_nAvgBytesPerSec(INT8U *p_workmem);
// p_workmem:    pointer to working memory
// return value: average bytes per second

int wav_dec_get_nBlockAlign(INT8U *p_workmem);
// p_workmem:    pointer to working memory
// return value: block align

int wav_dec_get_wBitsPerSample(INT8U *p_workmem);
// p_workmem:    pointer to working memory
// return value: bits per sample
const INT8U * wav_dvr_get_version(void);

//---------------------------------------------------------------------------
/* WAVE Encode*/
#ifndef __WAV_ENC_H__
#define __WAV_ENC_H__

//=======Constant Definition================
#define WAV_ENC_MEMORY_BLOCK_SIZE		40
#define DVRWAVE_FRAME_SIZE				512

//=======wave format tag====================
#define WAVE_FORMAT_A1800			(0x1000)
#define WAVE_FORMAT_MP3				(0x2000)
#define	WAVE_FORMAT_PCM				(0x0001)
#define	WAVE_FORMAT_ADPCM			(0x0002)
#define	WAVE_FORMAT_ALAW			(0x0006)
#define	WAVE_FORMAT_MULAW			(0x0007)
#define WAVE_FORMAT_IMA_ADPCM		(0x0011)

//========wave decoder error NO.==================
#define WAV_ENC_NO_ERROR    			0
#define WAV_ENC_UNKNOWN_FORMAT			0x80000100
#define WAV_ENC_CHANNEL_ERROR			0x80000101

int wav_enc_init(unsigned char *p_workmem);

int wav_enc_run(unsigned char *p_workmem,short *p_pcmbuf,unsigned char *p_bsbuf);

int wav_enc_get_header(unsigned char*p_workmem,unsigned char *p_header,int length);

int wav_enc_get_mem_block_size(void);

int wav_enc_get_SamplePerFrame(unsigned char *p_workmem);

int wav_enc_get_BytePerPackage(unsigned char *p_workmem);

int wav_enc_get_HeaderLength(unsigned char *p_workmem);

int wav_enc_Set_Parameter(unsigned char *p_workmem, int Channel,int SampleRate,int FormatTag);

#endif //__WAV_ENC_H__

//---------------------------------------------------------------------------
//A1800 decode
#ifndef __A1800DEC_H__
#define __A1800DEC_H__

#define A1800DEC_MEMORY_BLOCK_SIZE		5824//1676

#define A18_DEC_FRAMESIZE        320
#define A18_DEC_BITSTREAM_BUFFER_SIZE 4096

#define A18_OK						0x00000001
#define A18_E_NO_MORE_SRCDATA		0x80040005

extern int  A18_dec_SetRingBufferSize(void *obj, int size);
extern int  a1800dec_run(void *obj, int write_index, short * pcm_out);
extern int  a1800dec_init(void *obj, const unsigned char* bs_buf);
extern int  a1800dec_parsing(void *obj, int write_index);
extern int  a1800dec_read_index(void *obj);
extern int  a1800dec_GetMemoryBlockSize(void);
extern int  a1800dec_errno(void *obj);

extern const char* A18_dec_get_version(void);
extern int  A18_dec_get_bitrate(void *obj);
extern int  A18_dec_get_samplerate(void *obj);
extern int	A18_dec_get_channel(void *obj);
extern int	A18_dec_get_bitspersample(void *obj);
#endif //__A1800DEC_H__

//---------------------------------------------------------------------------
/* a1800 encode */
#ifndef __A1800ENC_H__
#define __A1800ENC_H__

#define	A18_ENC_FRAMESIZE			320		// input pcm size per frame
#define	A18_ENC_MEMORY_SIZE			5784	//

#define A18_ENC_NO_ERROR			0
#define A18_E_MODE_ERR				0x80000004

extern int A18_enc_run(unsigned char *p_workmem, const short *p_pcmbuf, unsigned char *p_bsbuf);
extern int A18_enc_init(unsigned char *p_workmem);
extern int A18_enc_get_BitRate(unsigned char *p_workmem);
extern int A18_enc_get_PackageSize(unsigned char *p_workmem);
extern int A18_enc_get_errno(char *A18enc_workmem);
extern const char* A18_enc_get_version(void);
extern int A18_enc_get_mem_block_size(void);
extern void A18_enc_set_BitRate(unsigned char *p_workmem, int BitRate);
#endif //!__A1800ENC_H__

//---------------------------------------------------------------------------
//A1600
#ifndef __A16_DEC_H__
#define __A16_DEC_H__

#define A16_DEC_FRAMESIZE        128
#define A16_DEC_MEMORY_SIZE      164
#define A16_DEC_BITSTREAM_BUFFER_SIZE 1024

#define     A16_IS_NOT_AT_FILE_END			0		//not arrive at the file end.must be no-zero.
#define     A16_IS_AT_FILE_END				1		//arrive at the file end.must be zero.

#define A16_OK						0
#define A16_E_NO_MORE_SRCDATA			0x80000000
#define A16_E_READ_IN_BUFFER			0x80000001
#define A16_CODE_FILE_FORMAT_ERR		0x80000002
#define A16_E_FILE_END				0x80000003
#define A16_E_MODE_ERR				0x80000004

extern int			A16_dec_run(char *p_workmem, short *p_pcmbuf, int wi);
extern int			A16_dec_init(char* A16dec_workmem, unsigned char* bs_buf);
extern int			A16_dec_parsing(char *A16dec_workmem,int wi);
extern int			A16_dec_get_ri(char *A16dec_workmem);
extern int			A16_dec_get_errno(char *A16dec_workmem);
extern const char * A16_dec_get_version(void);
extern int			A16_dec_get_mem_block_size(void);
extern int			A16_dec_get_bitrate(char *A16dec_workmem);
extern int			A16_dec_get_samplerate(char *A16dec_workmem);
extern int			A16_dec_get_channel(char *A16dec_workmem);
extern int			A16_dec_get_bitspersample(char *A16dec_workmem);
extern void			A16_dec_set_AtFileEnd(char *A16dec_workmem);
extern int			A16_dec_get_FileLen(char *A16dec_workmem);

#endif //!__A16_DEC_H__


//-----------------------------------------------------------------------added by Bruce, 2008/09/26
//A6400
#ifndef __a6400_dec_h__
#define __a6400_dec_h__

/////////////////////////////////////////////////////////////////////////////
//		Constant Definition
/////////////////////////////////////////////////////////////////////////////
#define A6400_DEC_FRAMESIZE					1152	// ???
#define A6400_DEC_BITSTREAM_BUFFER_SIZE   	4096    // size in bytes
#define A6400_DEC_MEMORY_SIZE 				14016	//(20632-8)	// 18456

/////////////////////////////////////////////////////////////////////////////
//		Error Code
/////////////////////////////////////////////////////////////////////////////
#define A6400_DEC_ERR_NONE			0x00000000	/* no error */

#define A6400_DEC_ERR_BUFLEN	   	   	0x80000001	/* input buffer too small (or EOF) */
#define A6400_DEC_ERR_BUFPTR	   	   	0x80000002	/* invalid (null) buffer pointer */

#define A6400_DEC_ERR_NOMEM	   	   	0x80000031	/* not enough memory */

#define A6400_DEC_ERR_LOSTSYNC	   	0x80000101	/* lost synchronization */
#define A6400_DEC_ERR_BADLAYER	   	0x80000102	/* reserved header layer value */
#define A6400_DEC_ERR_BADBITRATE	   	0x80000103	/* forbidden bitrate value */
#define A6400_DEC_ERR_BADSAMPLERATE  	0x80000104	/* reserved sample frequency value */
#define A6400_DEC_ERR_BADEMPHASIS	   	0x80000105	/* reserved emphasis value */
#define A6400_DEC_ERR_BADMPEGID		0x80000106	//for error mpegid add by zgq on 20080508

#define A6400_DEC_ERR_BADCRC	   	   	0x80000201	/* CRC check failed */
#define A6400_DEC_ERR_BADBITALLOC	   	0x80000211	/* forbidden bit allocation value */
#define A6400_DEC_ERR_BADSCALEFACTOR  0x80000221	/* bad scalefactor index */
#define A6400_DEC_ERR_BADMODE         0x80000222	/* bad bitrate/mode combination */
#define A6400_DEC_ERR_BADFRAMELEN	    0x80000231	/* bad frame length */
#define A6400_DEC_ERR_BADBIGVALUES    0x80000232	/* bad big_values count */
#define A6400_DEC_ERR_BADBLOCKTYPE    0x80000233	/* reserved block_type */
#define A6400_DEC_ERR_BADSCFSI	    0x80000234	/* bad scalefactor selection info */
#define A6400_DEC_ERR_BADDATAPTR	    0x80000235	/* bad main_data_begin pointer */
#define A6400_DEC_ERR_BADPART3LEN	    0x80000236	/* bad audio data length */
#define A6400_DEC_ERR_BADHUFFTABLE    0x80000237	/* bad Huffman table select */
#define A6400_DEC_ERR_BADHUFFDATA	    0x80000238	/* Huffman data overrun */
#define A6400_DEC_ERR_BADSTEREO	    0x80000239	/* incompatible block_type for JS */

/////////////////////////////////////////////////////////////////////////////
//		Function Definition
/////////////////////////////////////////////////////////////////////////////

// A6400 Decoder Version
extern const char * a6400_dec_get_version(void);

// A6400 Decoder Initial
extern int a6400_dec_init(void *p_workmem, char *p_bsbuf);

// A6400 header parsing
extern int a6400_dec_parsing(void *p_workmem, int wi);

// A6400 Decoder
extern int a6400_dec_run(void *p_workmem, short *p_pcmbuf, int wi, int granule);

// Get Read Index
extern int a6400_dec_get_ri(void *p_workmem);
// p_workmem:    pointer to working memory
// return value: read index of bitstream ring buffer

// Get mpeg id
//extern const char *A6400_dec_get_mpegid(void *p_workmem);

extern int a6400_dec_get_mem_block_size (void);
// return value: A6400 Decoder Working memory size

// return error number.
extern int a6400_dec_get_errno(void *p_workmem);

// return layer.
//extern int a6400_dec_get_layer(void *p_workmem);

// return channel.
extern int a6400_dec_get_channel(void *p_workmem);

// return bitrate in kbps.
extern int a6400_dec_get_bitrate(void *p_workmem);

// return sampling rate in Hz.
extern int a6400_dec_get_samplerate(void *p_workmem);

extern int a6400_dec_end(void *p_workmem,int wi);

#endif  // __a6400_dec_h__

//-----------------------------------------------------------------------added by Bruce, 2008/09/26
//S880
#ifndef __S880_DEC_H__
#define __S880_DEC_H__

#define L_FRAME16k   320                   /* Frame size at 16kHz                        */

#define S880_DEC_FRAMESIZE        L_FRAME16k
//#define S880_DEC_MEMORY_SIZE      2072
//#define S880_DEC_MEMORY_SIZE      (2072+1296)		//wenli, 2008.9.12
#define S880_DEC_MEMORY_SIZE      (2072+1296+128+184)		//wenli, 2008.9.18

#define S880_DEC_BITSTREAM_BUFFER_SIZE 1024

#define     S880_IS_NOT_AT_FILE_END			0		//not arrive at the file end.must be no-zero.
#define     S880_IS_AT_FILE_END				1		//arrive at the file end.must be zero.

#define S880_OK							0
#define S880_E_NO_MORE_SRCDATA			0x80000000
#define S880_E_READ_IN_BUFFER			0x80000001
#define S880_CODE_FILE_FORMAT_ERR		0x80000002
#define S880_E_FILE_END					0x80000003
#define S880_ERR_INVALID_MODE 			0x80000004

extern int			S880_dec_run(char *p_workmem, short *p_pcmbuf, int wi);
extern int			S880_dec_init(char* S880dec_workmem, unsigned char* bs_buf);
extern int			S880_dec_parsing(char *S880dec_workmem,int wi);
extern int			S880_dec_get_ri(char *S880dec_workmem);
extern int			S880_dec_get_errno(char *S880dec_workmem);
extern const char * S880_dec_get_version(void);
extern int			S880_dec_get_mem_block_size(void);
extern int			S880_dec_get_bitrate(char *S880dec_workmem);
extern int			S880_dec_get_samplerate(char *S880dec_workmem);
extern int			S880_dec_get_channel(char *S880dec_workmem);
extern int			S880_dec_get_bitspersample(char *S880dec_workmem);
extern void			S880_dec_set_AtFileEnd(char *S880dec_workmem);
extern int			S880_dec_get_FileLen(char *S880dec_workmem);

#endif //!__S880_DEC_H__
//==========================================================================
#ifndef __AVIPACKER_H__
#define __AVIPACKER_H__

#define AVIPACKER_RESULT_OK						0
#define AVIPACKER_RESULT_PARAMETER_ERROR		0x80000000

#define AVIPACKER_RESULT_FILE_OPEN_ERROR		0x80000001
#define AVIPACKER_RESULT_BS_BUF_TOO_SMALL		0x80000002
#define AVIPACKER_RESULT_BS_BUF_OVERFLOW		0x80000003
#define AVIPACKER_RESULT_FILE_WRITE_ERR			0x80000004
#define AVIPACKER_RESULT_FILE_SEEK_ERR			0x80000005

#define AVIPACKER_RESULT_MEM_ALIGN_ERR			0x80000006
#define AVIPACKER_RESULT_OS_ERR					0x80000007

#define AVIPACKER_RESULT_IDX_FILE_OPEN_ERROR	0x80000008
#define AVIPACKER_RESULT_IDX_BUF_TOO_SMALL		0x80000009
#define AVIPACKER_RESULT_IDX_BUF_OVERFLOW		0x8000000A
#define AVIPACKER_RESULT_IDX_FILE_WRITE_ERR		0x8000000B
#define AVIPACKER_RESULT_IDX_FILE_SEEK_ERR		0x8000000C
#define AVIPACKER_RESULT_IDX_FILE_READ_ERR		0x8000000D

#define AVIPACKER_RESULT_FILE_READ_ERR			0x8000000E
#define AVIPACKER_RESULT_IGNORE_CHUNK			0x8000000F

#define AVIPACKER_RESULT_FRAME_OVERFLOW			0x80000010
#define AVIPACKER_RESULT_FILE_CAT_ERR			0x80000011  // dominant add

#ifndef AVIIF_KEYFRAME
#define AVIIF_KEYFRAME							0x00000010
#endif // AVIIF_KEYFRAME

/****************************************************************************/

typedef struct
{
	unsigned short	left;
	unsigned short	top;
	unsigned short	right;
	unsigned short	bottom;
} GP_AVI_RECT;

typedef struct
{
	unsigned char	fccType[4];
	unsigned char	fccHandler[4];
	unsigned int	dwFlags;
	unsigned short	wPriority;
	unsigned short	wLanguage;
	unsigned int	dwInitialFrames;
	unsigned int	dwScale;
	unsigned int	dwRate;
	unsigned int	dwStart;
	unsigned int	dwLength;
	unsigned int	dwSuggestedBufferSize;
	unsigned int	dwQuality;
	unsigned int	dwSampleSize;
	GP_AVI_RECT		rcFrame;
} GP_AVI_AVISTREAMHEADER;	// strh

typedef struct
{
	unsigned int	biSize;
	unsigned int	biWidth;
	unsigned int	biHeight;
	unsigned short	biPlanes;
	unsigned short	biBitCount;
	unsigned char	biCompression[4];
	unsigned int	biSizeImage;
	unsigned int	biXPelsPerMeter;
	unsigned int	biYPelsPerMeter;
	unsigned int	biClrUsed;
	unsigned int	biClrImportant;
    unsigned int	Unknown[7];
} GP_AVI_BITMAPINFO;	// strf

typedef struct
{
	unsigned short	wFormatTag;
	unsigned short	nChannels;
	unsigned int	nSamplesPerSec;
	unsigned int	nAvgBytesPerSec;
	unsigned short	nBlockAlign;
	unsigned short	wBitsPerSample;
	unsigned short	cbSize;				// useless when wFormatTag is WAVE_FORMAT_PCM
	unsigned short  ExtInfo[16];		// use when the wFormatTag is MS_ADPCM 
} GP_AVI_PCMWAVEFORMAT;	// strf


/****************************************************************************/
typedef enum
{
	AVIPACKER_MSG_IDX_WRITE = 0x00010000,
	AVIPACKER_MSG_STOP = 0x00020000,
	AVIPACKER_MSG_VIDEO_WRITE = 0x00030000,
	AVIPACKER_MSG_AUDIO_WRITE = 0x00040000,
	AVIPACKER_MSG_HDR_WRITE = 0x00050000,
	AVIPACKER_MSG_GPS_WRITE = 0x00060000
 } AVIPACKER_MSG;

typedef struct
{
	INT32U msg_id;
	INT32U buffer_addrs;
	INT32U buffer_idx;
	INT32U buffer_scope;	
	INT32U buffer_len;
	INT32U buffer_time;
	INT8U  src_from;
	INT8U  is_used;
	INT8U  ext;
	INT32U jpeg_Y_Q_value;
	INT32U jpeg_UV_Q_value;
}AVIPACKER_FRAME_INFO;

// init avi packer
INT32S AviPackerV3_TaskCreate(	INT8U	prio,
								void	*_AviPacker,
								void	*pIdxRingBuf,
								int		IdxRingBufSize );

/////////////////////////////////////////////////////////////////////////////
int AviPackerV3_Open(
	void *WorkMem,
	int								fid,			// Record file ID
	int								fid_idx,		// Temporary file ID for IDX
	int								fid_txt,		// TXT file ID (for GPS)
	const GP_AVI_AVISTREAMHEADER	*VidHdr,		// Video stearm header
	int								VidFmtLen,		// Size of video stream format, count in byte
	const GP_AVI_BITMAPINFO			*VidFmt,		// Video stream format
	const GP_AVI_AVISTREAMHEADER	*AudHdr,		// Audio stearm header = NULL if no audio stream
	int								AudFmtLen,		// Size of audio stream format, count in byte. If zero => no audio stream
	const GP_AVI_PCMWAVEFORMAT		*AudFmt);		// Audio stream format. = NULL if no audio stream
	
/*---------------------------------------------------------------------------
Description:
	open an AVI file for AVI recoder

Return Value:
	AVIPACKER_RESULT_OK
	AVIPACKER_RESULT_FILE_OPEN_ERROR
	AVIPACKER_RESULT_IDX_FILE_OPEN_ERROR
	AVIPACKER_RESULT_MEM_ALIGN_ERR
	AVIPACKER_RESULT_OS_ERR
	AVIPACKER_RESULT_FILE_WRITE_ERR
////////////////////////////////////////////////////////////////////////// */


/////////////////////////////////////////////////////////////////////////////
int AviPackerV3_Close(void *WorkMem);
/*---------------------------------------------------------------------------
Description:
	Close an AVI file that has been opened by AviPackerV3_Open
	
Return Value:
	AVIPACKER_RESULT_OK
	AVIPACKER_RESULT_OS_ERR
////////////////////////////////////////////////////////////////////////// */


/////////////////////////////////////////////////////////////////////////////
int AviPackerV3_SwitchFile(
	void *_AviPacker,
	INT16S							*fid,
	INT16S							*fid_idx,
	INT16S							*fid_txt,
	CHAR							*index_path,
	const GP_AVI_AVISTREAMHEADER	*VidHdr,
	int								VidFmtLen,
	const GP_AVI_BITMAPINFO			*VidFmt,
	const GP_AVI_AVISTREAMHEADER	*AudHdr,
	int								AudFmtLen,
	const GP_AVI_PCMWAVEFORMAT		*AudFmt,
	int				fd_new,
	int				fd_txt_new);	
/*---------------------------------------------------------------------------
Description:
	switch a new file to AviPacker
	
Return Value:
	AVIPACKER_RESULT_OK
	AVIPACKER_RESULT_OS_ERR
////////////////////////////////////////////////////////////////////////// */


// get avi packer version
const char *AviPackerV3_GetVersion(void);

// avi packer error handle
void AviPackerV3_SetErrHandler(void *WorkMem, int (*ErrHandler)(int ErrCode));

// Add info string to INFO LIST
int AviPackerV3_AddInfoStr(void *WorkMem, const char *fourcc, const char *info_string);

int AviPackerV3_GetWorkMemSize(void);

//	AviPackerV3_WriteData
void AviPackerV3_WriteData(void *WorkMem, AVIPACKER_FRAME_INFO* pFrameInfo);

#endif // __AVIPACKER_H__
//============================================================================

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

//NV function
extern INT32U nvmemory_init(void);

extern INT16U nv_open(INT8U *path);
extern INT32U nv_read(INT16S fd, INT32U buf, INT32U size);
//extern INT32U nv_fast_read(INT16S fd, INT32U buf, INT32U byte_size);
extern INT32U nv_lseek(INT16S fd, INT32S foffset, INT16S origin);
extern INT32U nv_rs_size_get(INT16S fd); // jandy add to querry fd size

extern INT32U nvmemory_rs_byte_load(INT8U *name ,INT32U offset_byte, INT32U *pbuf , INT32U byte_count);
extern INT32U nvmemory_rs_sector_load(INT8U *name ,INT32U offset_secter, INT32U *pbuf , INT16U secter_count);
extern INT32U nvmemory_user_sector_load(INT16U itag , INT32U *pbuf , INT16U secter_count);
extern INT32U nvmemory_user_sector_store(INT16U itag , INT32U *pbuf , INT16U secter_count);

//NV SPI
extern INT32U nvspiflash_retrieval(void);
extern INT32U nvspiflash_rs_get(INT16U itag ,INT32U offset_secter, INT32U *pbuf , INT16U secter_count);
extern INT32U nvspiflash_user_set (INT16U itag ,INT32U *pbuf , INT16U secter_count);
extern INT32U nvspiflash_user_get (INT16U itag ,INT32U *pbuf , INT16U secter_count);
#if 1
INT32U nvspiflash_file_size_get(INT16U itag);
#endif
//NV RAM
extern INT32U nvram_rs_get(INT16U itag ,INT32U offset_secter, INT32U *pbuf , INT16U secter_count);
extern INT32U combin_reg_data(INT8U *data, INT32S len);

extern void CRC_cal(INT8U *pucBuf, INT32U len, INT8U *CRC);
extern void cal_day_store_get_register(void (*fp_store)(INT32U),INT32U (*fp_get)(void),void (*fp_config_store)(void));

/**/
extern void mm_dump(void);

#endif 		// __GPLIB_H__
