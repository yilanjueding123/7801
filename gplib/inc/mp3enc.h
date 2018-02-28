#ifndef __MP3ENC_H__
#define __MP3ENC_H__

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



#endif	// __MP3ENC_H__
