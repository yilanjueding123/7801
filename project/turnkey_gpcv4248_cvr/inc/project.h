#ifndef __PROJECT_H__
#define __PROJECT_H__

/* Pseudo header files for customer level plateform define */
#include "customer.h"


// Operating System definitions and config
#define _OS_NONE                0
#define _OS_UCOS2               1
#define	_OPERATING_SYSTEM       _OS_UCOS2

// Global definitions
#define TRUE                    1
#define FALSE                   0

#ifndef NULL
#define NULL                    0
#endif

#define STATUS_OK               0
#define STATUS_FAIL             -1
#define SUCCESS					0
//#define FAIL					-1
#define ENABLE 					1
#define DISABLE					0

typedef char                    CHAR;   // By default, char is unsigned. It can be changed to signed by compiler option
typedef unsigned char           BOOLEAN;
typedef unsigned char           INT8U;
typedef signed   char           INT8S;
typedef unsigned short          INT16U;
typedef signed   short          INT16S;
typedef unsigned int            INT32U;
typedef signed   int            INT32S;
typedef float                   FP32;
typedef	long long		INT64S;	
typedef	unsigned long long	INT64U;	
typedef double                  FP64;

#define WRITE8(reg)  (*(INT8U *)(reg))
#define WRITE16(reg) (*(INT16U *)(reg))
#define WRITE32(reg) (*(INT32U *)(reg))

#ifndef __ALIGN_DEFINE__
#define __ALIGN_DEFINE__
    #ifdef __ADS_COMPILER__
        #define ALIGN32 __align(32)
        #define ALIGN16 __align(16)
        #define ALIGN8 __align(8)
        #define ALIGN4 __align(4)
    #else
        #ifdef __CS_COMPILER__
        #define ALIGN32 __attribute__ ((aligned(32)))
        #define ALIGN16 __attribute__ ((aligned(16)))
        #define ALIGN8 __attribute__ ((aligned(8)))
        #define ALIGN4 __attribute__ ((aligned(4)))
        #else
        #define ALIGN32 __align(32)
        #define ALIGN16 __align(16)
        #define ALIGN8 __align(8)
        #define ALIGN4 __align(4)
        #endif
    #endif
#endif


#ifndef __CS_COMPILER__
    #define ASM(asm_code)  __asm {asm_code}  /*ADS embeded asm*/
    #define IRQ            __irq
    #define PACKED 			__packed
#else
    #define ASM(asm_code)  asm(#asm_code);  /*ADS embeded asm*/
    #define IRQ            __attribute__ ((interrupt))
    #define PACKED	   __attribute__((__packed__))
#endif

#endif      // __PROJECT_H__
