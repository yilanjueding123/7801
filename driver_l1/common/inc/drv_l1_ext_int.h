#ifndef __drv_l1_EXT_INT_H__
#define __drv_l1_EXT_INT_H__

#include "driver_l1.h"
#include "drv_l1_sfr.h"

#define EXTC_INT 0x0100
#define EXTB_INT 0x0080
#define EXTA_INT 0x0040

#define EXTC_POL 0x0020
#define EXTB_POL 0x0010
#define EXTA_POL 0x0008

#define EXTC_IEN 0x0004
#define EXTB_IEN 0x0002
#define EXTA_IEN 0x0001

extern void ext_int_init(void);
extern void extab_int_clr(INT8U ext_src);
extern void extab_edge_set(INT8U ext_src, INT8U edge_type);
extern void extab_enable_set(INT8U ext_src, BOOLEAN status);
extern void extab_user_isr_set(INT8U ext_src,void (*user_isr)(void));
extern void extab_user_isr_clr(INT8U ext_src);

#if WIFI_FUNC_ENABLE
#define drv_l1_ext_int_init ext_int_init
#define drv_l1_extabc_edge_set extab_edge_set
#define drv_l1_extabc_user_isr_set extab_user_isr_set
#define drv_l1_extabc_enable_set extab_enable_set
#endif

#endif		// __drv_l1_EXT_INT_H__
