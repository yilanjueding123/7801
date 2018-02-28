/*Pseudo Header*/
#define STATE_BASE              0x00000100

typedef enum
{
    STATE_VIDEO_RECORD = STATE_BASE,
    STATE_VIDEO_PREVIEW,
    STATE_BROWSE,
    STATE_SETTING,
    STATE_STARTUP,
    STATE_THUMBNAIL,
    STATE_AUDIO_RECORD,   // dominant add for audio record
    STATE_CONNECT_TO_PC,
    STATE_MAX
} STATE_ENUM;

