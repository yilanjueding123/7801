#ifndef __IR_H__
#define __IR_H__
//===============================================================================================
#include "project.h"

#if C_IR_REMOTE
//this file only for "红外遥控接收功能 with MK6A12P";

extern void F_128Hz_IR_Service(void);
extern void ir_init(void);
extern INT32U g_ir_get(void);
extern void g_ir_set(INT32U val);
//===============================================================================================
#endif 		
#endif 		// __IR_H__
