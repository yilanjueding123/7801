#ifndef __AVI_AUDIO_RECORD_H__
#define __AVI_AUDIO_RECORD_H__

#include "avi_encoder_app.h"

#define FLOAT float
typedef struct{
	INT32U Hour;
	INT32U Minute;
	INT32U Second;
	INT32S Status; //Status A=active or V=Void
	FLOAT Latitude;
	INT32S NSInd;
	FLOAT Longitude;
	INT32S EWInd;
	FLOAT Speed; //Speed over the ground in knots
	FLOAT Angle; //Track angle in degrees True
	INT32U Year;
	INT32U Month;
	INT32U Day;
}RMCINFO;

typedef struct {
		RMCINFO rmcinfo; //GPS RMC tag data field, please refer to GPS spec
		Gsensor_Data gs_data;
} GPSDATA,*pGPSDATA;

#endif //__AVI_AUDIO_RECORD_H__
