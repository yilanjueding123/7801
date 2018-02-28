#ifndef __AVIPACKER_H__
#define __AVIPACKER_H__

#include "application.h"

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
	GP_AVI_RECT	rcFrame;
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
	// unsigned int	Unknown[7];
} GP_AVI_BITMAPINFO;	// strf

typedef struct
{
	unsigned short	wFormatTag;
	unsigned short	nChannels;
	unsigned int	nSamplesPerSec;
	unsigned int	nAvgBytesPerSec;
	unsigned short	nBlockAlign;
	unsigned short	wBitsPerSample;
	//unsigned short	cbSize;				// useless when wFormatTag is WAVE_FORMAT_PCM
} GP_AVI_PCMWAVEFORMAT;	// strf

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


/////////////////////////////////////////////////////////////////////////////
INT32S AviPackerV3_TaskCreate(	INT8U	prio,
								void	*_AviPacker,
								void	*pIdxRingBuf,
								int		IdxRingBufSize );
/*---------------------------------------------------------------------------
Description:
	create avi packer task and os event

Return Value:
	AVIPACKER_RESULT_OK
	AVIPACKER_RESULT_FILE_OPEN_ERROR
	AVIPACKER_RESULT_IDX_FILE_OPEN_ERROR
	AVIPACKER_RESULT_MEM_ALIGN_ERR
	AVIPACKER_RESULT_OS_ERR
	AVIPACKER_RESULT_FILE_WRITE_ERR
////////////////////////////////////////////////////////////////////////// */

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
const char *AviPackerV3_GetVersion(void);
/*---------------------------------------------------------------------------
Description
	AviPackerV3_GetVersion will return version string

Return Value:
	The version string of AviPacker 
////////////////////////////////////////////////////////////////////////// */	


/////////////////////////////////////////////////////////////////////////////
void AviPackerV3_SetErrHandler(void *WorkMem, int (*ErrHandler)(int ErrCode));
/*---------------------------------------------------------------------------
Description
1. If error occured while running AviPacker, AviPacker will call ErrHandler
   back with current error code (ErrCode)
2. ErrHandler should return non-zero value if user wants to continue porcess
   AviPacker; or zero to stop.
////////////////////////////////////////////////////////////////////////// */	


/////////////////////////////////////////////////////////////////////////////
int AviPackerV3_AddInfoStr(void *WorkMem, const char *fourcc, const char *info_string);
/*---------------------------------------------------------------------------
Description
	Add info string to INFO LIST

	fourcc : the type of information string
	info_string : information string

Return Value:
	Return 1 if info_string has been wrote successfully.
////////////////////////////////////////////////////////////////////////// */	

int AviPackerV3_GetWorkMemSize(void);
/*---------------------------------------------------------------------------
Description
	Working memory size of AVI-Packer V3

Return Value:
	Working memory size of AVI-Packer V3 , count in byte
////////////////////////////////////////////////////////////////////////// */	

void VdoFramNumsLowBoundReg(void *WorkMem, unsigned int fix_frame_cnts);
/*---------------------------------------------------------------------------
Description
	fixed total frame number before avi video close to guarantee total display time
    in wondows
Unit: frames
Return Value:
	void
////////////////////////////////////////////////////////////////////////// */	

unsigned long current_movie_Byte_size_get(void *WorkMem);    
void AviPacker_Break_Set(void *WorkMem, INT8U Break1_Work0);

#endif // __AVIPACKER_H__
