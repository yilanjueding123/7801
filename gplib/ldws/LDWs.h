#ifndef _LDWs_H_
#define _LDWs_H_

#define Enable_Lane_Departure_Warning_System	0

/////////////////////////////////////////////////////////////////////////////////

#define ABS(x)		((x) >= 0 ? (x) : -(x))


typedef struct
{
	short x;
	short y;
	short width;
	short height;

} gpRect;


typedef struct
{

	int workingImg_W;
	int workingImg_H;

	int scaleRate;
	int scaleRate_UI;
	int scaleRate_F;
	int SRC_ROI_W;
	int SRC_ROI_H;

	int skipFrameFlg;

	gpRect ROI;
	int ROI_TopY_limit;
	int ROI_BottY_limit;
	int ROI_initY;
	int initROIFlg;
	int shiftROI_X_flg;
	int bestROI_X;
	int bestROI_Y;
	int startROI_X;
	int endROI_X;
	int detectROIShift_X;
	int dynamicROI_X;
	int dynamicROI_loadWidthOffset;   //value smaller, more sensitive

	int recordROI_X;
	int recordROI_Y;


	int ROI_widthCenter;

	int BLThre;
	int BRThre;
	int lastBLThre;
	int lastBRThre;
	int minBLThre;
	int minBRThre;
	int maxBLThre;
	int maxBRThre;

	int LLineEnergyThre;
	int RLineEnergyThre;
	int LLAlarmineEnergyThre;
	int RLAlarmineEnergyThre;


	//working memory ptr
	unsigned char* ROIImg_ptr;
	unsigned char* smoothImg_ptr;
	unsigned char* sobelImg_ptr;
	unsigned short* countTable_ptr;


	//UI parameter
	int laneSlopeAddValue;    //car class
	int laneWidthAddValue;
	int tunnelSlopeAddValue;    //in tunnel & select sensitve high
	int landLine;
	int excuImg_landLine;
	int excuImg_nonlaneLine;
	int loadCenter_X;
	int camSetShift_X;
	int dispalyLaneBott_shiftX;
	int laneCenterl_X;
	int laneCenterMean_X;
	int centerDist_X;
	int centerDistMin_X;
	int carClassMode;  //height : s(low) = 0, c(mid) = 1, (high) = 2
	int countryMode;   //taiwan = 0, china = 1
	int sensitiveMode; // general = 0, sensitivity = 1
	int turnOnSpeedMode;  // high = 0, low = 1
    int turnOnSpeedReduceCnt;  // high = 0, low = 1
	int sensitiveCnt;
	int chinaReduceCnt;
	int sensitiveSlope;   // more large more sensitive
	int speedDetectCntR_60km;
	int speedDetectCntL_60km;
	int speedDetectFlg_60km;
	int speedDetectCntR_80km;
	int speedDetectCntL_80km;
	int speedDetectFlg_80km;
	int highSpeedGStatusCnt;



	int searchOPStep;
	int degreeStep;


	int maxLineLimit_I_T;
	int maxLineLimit_I_B;
	int maxLineLimit_J_T;
	int maxLineLimit_J_B;

	int thirdLineLimit_I_T;
	int thirdLineLimit_I_B;
	int thirdLineLimit_J_T;
	int thirdLineLimit_J_B;


	int secLineLimit_I_T;
	int secLineLimit_I_B;
	int secLineLimit_J_T;
	int secLineLimit_J_B;



	int fourthLineLimit_I_T;
	int fourthLineLimit_I_B;
	int fourthLineLimit_J_T;
	int fourthLineLimit_J_B;





	int maxLineLimit_I_T_fix;
	int maxLineLimit_I_B_fix;
	int maxLineLimit_J_T_fix;
	int maxLineLimit_J_B_fix;

	int thirdLineLimit_I_T_fix;
	int thirdLineLimit_I_B_fix;
	int thirdLineLimit_J_T_fix;
	int thirdLineLimit_J_B_fix;


	int secLineLimit_I_T_fix;
	int secLineLimit_I_B_fix;
	int secLineLimit_J_T_fix;
	int secLineLimit_J_B_fix;

	int fourthLineLimit_I_T_fix;
	int fourthLineLimit_I_B_fix;
	int fourthLineLimit_J_T_fix;
	int fourthLineLimit_J_B_fix;






	int maxResetCnt;
	int secResetCnt;
	int thirdResetCnt;
	int fourthResetCnt;
	int maxResetCntThre;
	int secResetCntThre;
	int thirdResetCntThre;
	int fourthResetCntThre;

	int RLWarningCheckCnt;
	int LLWarningCheckCnt;
	int RLWarningCheckCntThre;
	int LLWarningCheckCntThre;
	int RLNonWarningCnt;
	int LLNonWarningCnt;
	int RLNonWarningCntThre;
	int LLNonWarningCntThre;
	

	int changeLaneCntThre;
	int changeNonDotLaneCntThre_L;
	int changeNonDotLaneCntThre_R;


	int RLDetectCheckCnt;
	int LLDetectCheckCnt;
	int RLDetectCheckCntThre;
	int LLDetectCheckCntThre;
	int RLNonDetectCnt;
	int LLNonDetectCnt;


	int LLcheckFlg;
	int RLcheckFlg;
	int LLAlarmFlg;
	int RLAlarmFlg;


	int RTP_X;
	int RTP_Y;
	int RBP_X;
	int RBP_Y;

	int LTP_X;
	int LTP_Y;
	int LBP_X;
	int LBP_Y;

	int RT_alarmP_X;
	int RT_alarmP_Y;
	int RB_alarmP_X;
	int RB_alarmP_Y;

	int LT_alarmP_X;
	int LT_alarmP_Y;
	int LB_alarmP_X;
	int LB_alarmP_Y;



	int last_RTP_X;
	//int last_RTP_Y;
	int last_RBP_X;
	//int last_RBP_Y;

	int last_LTP_X;
	//int last_LTP_Y;
	int last_LBP_X;
	//int last_LBP_Y;


	int last_LT_alarmP_X;
	int last_LT_alarmP_Y;
	int last_RT_alarmP_X;
	int last_RT_alarmP_Y;

	int last_LB_alarmP_X;
	int last_LB_alarmP_Y;
	int last_RB_alarmP_X;
	int last_RB_alarmP_Y;



	int LTP_resetFlg;
	int LBP_resetFlg;
	int RTP_resetFlg;
	int RBP_resetFlg;

	int LDW_keyFlg;
	int memoryTurnKeyFlg;
	int memoryTurnKeyCnt;

	int LDW_systemTurnOnCnt;
	int LDW_systemTurnOffCnt;
	int LDW_leaveFreewayCnt;
	int speedLineCnt_L;
	int speedMinPointCnt_L;
	int speedMaxPointCnt_L;
	int continusSmallCnt_L;
	int continusBigCnt_L;
	int continusSamCnt_L;
	int maxCnt_L;
	int minCnt_L;
	int getMaxFlg_L;
	int getMinFlg_L;

	int minCntShiftValueThre;

	int speedLineCnt_R;
	int speedMinPointCnt_R;
	int speedMaxPointCnt_R;
	int continusSmallCnt_R;
	int continusBigCnt_R;
	int continusSamCnt_R;
	int maxCnt_R;
	int maxMinThre;
	int minMinThre;
	int minCnt_R;
	int getMaxFlg_R;
	int getMinFlg_R;
	int checkTurnOnCnt_L;
	int checkTurnOnCnt_R;
	int checkTurnOffCnt_L;
	int checkTurnOffCnt_R;
	int checkTurnOnCntThre;
	int badLaneCntThre;
	int delayTurnOffCntThre;
	int delayTurnOnCntThre;
	int refindROIWeight;

	int getMax;
	int getSec;
	int getThird;
	int getFourth;
    int recordMax;
	int recordSec;

	int dotLineFlg_L;
	int dotLineFlg_R;
	int dotLineFlgCnt_L;
	int dotLineFlgCnt_R;
	//int dotMinThre_L;
	//int dotMinThre_R;

	int turnOn_pickRangeThre;
	int turnOff_pickRangeDoubleThre;
	int turnOn_pickDoubleThre;

	

	int touchNonDotLineFlg_R;
	int touchNonDotLineFlg_L;

	int LaneWidthThre;
	int LaneWidth;
	

	int LDWsAlarmFlg;
	int changeLaneFlg;
	unsigned int tunnelCnt;
    int tunnelCntThre;              // check whether is tunnel threshold
	int lastOpCnt_R;
	int lastOpCnt_L;

	int recordAlarmCntFlg;
	int alarmCount;
	int alarmCntTable[4]; //change lane speed count:  30, 40, 50, 60  
	int dynamicAlarmCntThre;
	int dynamicROIFlg;    //finish ROI set flag
	int ROIResetCnt;
	int limitSlopeFlg;

	int GSMode;     // mode = 0 -> init, mode = 1 -> speed 0, mode = 2 -> Go, mode = 3 -> Stop, mode = 4 -> Moving
	int GSInitFlg;
	int GSModeNotSpeed0Cnt;
	int gsMode4Cnt;  //defend learning threshold too low, can't learn high


	int fix_G_x;
	int fix_G_y;
	int fix_G_z;
	int inverseGzFlg;

	int last_G_x;
	int last_G_y;
	int last_G_z;
	int tmpLast_G_x;
	int tmpLast_G_y;
	int tmpLast_G_z;
	int GSensorFrameCnt;
	int GSensorFrameCnt_10;
	int GValue_sumCnt;
	

	int GValue_mean_x;
	int GValue_mean_y;
	int GValue_mean_z;
	int GValue_mean_last_x;
	int GValue_mean_last_y;
	int GValue_mean_last_z;
	int GValue_vibrateCnt;

	int sumGx;
	int sumGy;
	int sumGz;
	int sumCnt;
	int tilGzCheckCnt;
	int bestGSThre;
	int GValueTable[8];
	int totalGValueCnt;
	int lastGValue;


	int GSensorSpeed0Cnt;
	int GSensorStopCnt;
	int GSensorStopEnergy;
	int GSensorGoCnt;
	int GSensorMovingCnt;
	int maxSpeed0CntThre;
	int objFrameCnt;
	int StopAndGoGetObjSegm;
	int minObjFrameValue;
	int tmpMinObjFrameValue;

	//SFCW  
	int SFCW_keyFlg;
	int SFCWMode;
	int SFCWStatus;       // 1 SFCW alarm
	int SFCWAlarmCnt;
	int SFCWNonAlarmCnt;
	int fixSFCWROI_LTP_X;
	int fixSFCWROI_RTP_X;
	int FCW_ROIReg;
	int FCW_ROIOffset;
	int forwardObjCnt;
	int threObjCnt;
	int forwardObjReduceCnt;
	int stopMovingOptCnt_L;
	int stopMovingOptCnt_R;
	int stopMovingOptCnt_T;
	int stopMovingOptCntThre;
	//int stopMovingDetectROIMode;
	int SFCWCheckCntThre;
	int SFCWAlarmDistance;
	int SFCWAlarmDistanceROI_60KM_T;
	int SFCWAlarmDistanceROI_60KM_B;
	int SFCWAlarmDistanceROI_80KM_T;
	int SFCWAlarmDistanceROI_80KM_B;
	int SFCWROI_shiftX_60km;    //more large more close
	int SFCWROI_shiftX_80km;    //more large more close
//	int SFCWDay_NightMode;


	//Stop and Go
	int StopAndGo_keyFlg;
	int StopAndGoMode;
	int StopAndGoStatus;   // 1 Stop alarm, 2 Go alarm
	int viewAngleMode;  //0 is general view, 1 is super view
	int goStatusCnt;  // first check segment1 wait for AE: cnt 0 ~ 30, check car speed 0 segment2: 30 ~ 60, final go check  segment3: 60 ~ 120, suddenly OB increase reset cnt 70
	int goCheckCnt;
	int recordOBLRate;
	int recordOBRRate;
	int recordGoOBRate;
	int goOBContinusLargeCnt;
	int lastRecordGoOBRate;
	int diffOB;
	int SNGsuddenlyThre;
	int changeRecordGoOBFlg;
	int goAlarmObjThre;
	int goAlarmCnt;
	int delayGoAlarmCnt;
	int goAlarmDoOnceFlg;
	int adaptationGoAlarmThre;
	int dynamicROI_X_Thre;
	int fixBThre;



} LDWsParameter;



int sqrtu32(unsigned int a);
int LDWs_get_memorySize(int workingImg_W, int workingImg_H, LDWsParameter *par);
void LDWsInit(int* LDWs_workmem, LDWsParameter *par);
unsigned int udiv32(unsigned int d, unsigned int n);
int divFun(int a, int b);
void mem_set_zero_int32(void *addr, int size);
void getROIImage_1080P_YUYV(unsigned char *inputImg, unsigned char *outputImg, int ImgW, int ImgH, LDWsParameter *par);
void GPLDWSmoothImage(unsigned char *inputImg, unsigned char *outputImg, int ImgW, int ImgH, LDWsParameter *par);
void LDWSmoothSobel_720P_GP420(unsigned char *inputImg, unsigned char *outputImg, int ImgW, int ImgH, LDWsParameter *par);
void LDWSmoothSobel_1080P_YUYV(unsigned char *inputImg, unsigned char *outputImg, int ImgW, int ImgH, LDWsParameter *par);
void LDWSmoothImage_1080P_YUYV(unsigned char *inputImg_ptr, unsigned char *outputImg_ptr, int width, int height, LDWsParameter *par);
void LDWSmoothSobel(unsigned char *inputImg_ptr, unsigned char *outputImg_ptr, int width, int height , LDWsParameter *par);
void sobel(unsigned char* inputImg_ptr, int width, int height, unsigned char* outputImg_ptr, LDWsParameter *par);
void houghTransform(unsigned char* inputImg_ptr, int width, int height, unsigned short* countTable, LDWsParameter *par);
void LDWs(unsigned short* countTable, gpRect ROI, LDWsParameter *par);
void drawLine(unsigned short* drawImg, int startx, int starty, int endx, int endy, int colorR, int colorG, int colorB);


//int lumaSpeed0Detect(int *lumaValue_ptr, int lumaBlockNum, int lumaThre_shiftV, int blockThre, int readLumaFlg, LDWsParameter *par);
void LDWTurnKey( LDWsParameter *LDWPar, unsigned char*sobelDxImg_ptr, int width, int height , gpRect ROI);
void GSensorStatusDetect(int x, int y, int z, int GValueThre_shiftV, LDWsParameter *par);
void dynamicROI(LDWsParameter *LDWPar);


extern int gp_hw_checking_LDWs(void);
const char *LDWs_GetVersion(void);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif