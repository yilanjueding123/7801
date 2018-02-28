/*
* Purpose: Interrupt Rquest Controller driver/interface
*
* Author: Tristan Yang
*
* Date: 2007/09/21
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 1.00
*/

#include "driver_l1.h"
#include "drv_l1_sfr.h"
#include "drv_l1_interrupt.h"
#include "Arm.h"

// Interrupt IRQ mask register
#define C_VIC_IRQ_MASK_SET_ALL		0xFFFFFFFF

// Interrupt FIQ mask register
#define C_VIC_FIQ_MASK_SET_ALL		0x0000000F

// Interrupt Global mask register
#define C_VIC_GLOBAL_MASK_CLEAR		0x00000000
#define C_VIC_GLOBAL_MASK_SET		0x00000001


void (*irq_isr[VIC_MAX_IRQ])(void);
void (*fiq_isr[VIC_MAX_FIQ])(void);

INT32U vic_global_mask_set(void)
{
	INT32S old_mask;
  #if _OPERATING_SYSTEM != _OS_NONE
	OS_CPU_SR cpu_sr;
  #endif

  #if _OPERATING_SYSTEM != _OS_NONE				// Soft Protect for critical section
	OS_ENTER_CRITICAL();
  #endif
	old_mask = R_INT_GMASK & C_VIC_GLOBAL_MASK_SET;
	R_INT_GMASK = C_VIC_GLOBAL_MASK_SET;
  #if _OPERATING_SYSTEM != _OS_NONE
	OS_EXIT_CRITICAL();
  #endif

	return old_mask;
}

void vic_global_mask_restore(INT32U old_mask)
{
  #if _OPERATING_SYSTEM != _OS_NONE
	OS_CPU_SR cpu_sr;
  #endif

  #if _OPERATING_SYSTEM != _OS_NONE				// Soft Protect for critical section
	OS_ENTER_CRITICAL();
  #endif
	if (old_mask & C_VIC_GLOBAL_MASK_SET) {
		R_INT_GMASK = C_VIC_GLOBAL_MASK_SET;
	} else {
		R_INT_GMASK = C_VIC_GLOBAL_MASK_CLEAR;
	}
  #if _OPERATING_SYSTEM != _OS_NONE
	OS_EXIT_CRITICAL();
  #endif
}

INT32S vic_irq_register(INT32U irc_num, void (*isr)(void))
{
  #if _OPERATING_SYSTEM != _OS_NONE
	OS_CPU_SR cpu_sr;
  #endif

	if (irc_num && (irc_num<VIC_MAX_IRQ)) {
		irc_num--;

	  #if _OPERATING_SYSTEM != _OS_NONE			// Soft Protect for critical section
		OS_ENTER_CRITICAL();
	  #endif
		irq_isr[irc_num] = isr;
	  #if _OPERATING_SYSTEM != _OS_NONE
		OS_EXIT_CRITICAL();
	  #endif

		return STATUS_OK;
	}

	return STATUS_FAIL;
}

INT32S vic_irq_unregister(INT32U irc_num)
{
  #if _OPERATING_SYSTEM != _OS_NONE
	OS_CPU_SR cpu_sr;
  #endif

	if (irc_num && (irc_num<VIC_MAX_IRQ)) {
		irc_num--;

	  #if _OPERATING_SYSTEM != _OS_NONE			// Soft Protect for critical section
		OS_ENTER_CRITICAL();
	  #endif
		irq_isr[irc_num] = NULL;
	  #if _OPERATING_SYSTEM != _OS_NONE
		OS_EXIT_CRITICAL();
	  #endif

		return STATUS_OK;
	}

	return STATUS_FAIL;
}

INT32S vic_irq_enable(INT32U irc_num)
{
	INT32S old_mask;
  #if _OPERATING_SYSTEM != _OS_NONE
	OS_CPU_SR cpu_sr;
  #endif

	if (irc_num && (irc_num<VIC_MAX_IRQ)) {
		irc_num = VIC_MAX_IRQ - irc_num - 1;

	  #if _OPERATING_SYSTEM != _OS_NONE			// Soft Protect for critical section
		OS_ENTER_CRITICAL();
	  #endif
		old_mask = (R_INT_IRQMASK >> irc_num) & 0x1;
		R_INT_IRQMASK &= ~(1 << irc_num);
	  #if _OPERATING_SYSTEM != _OS_NONE
		OS_EXIT_CRITICAL();
	  #endif

		return old_mask;
	}

	return STATUS_FAIL;
}

INT32S vic_irq_disable(INT32U irc_num)
{
	INT32S old_mask;
  #if _OPERATING_SYSTEM != _OS_NONE
	OS_CPU_SR cpu_sr;
  #endif

	if (irc_num && (irc_num<VIC_MAX_IRQ)) {
		irc_num = VIC_MAX_IRQ - irc_num - 1;

	  #if _OPERATING_SYSTEM != _OS_NONE			// Soft Protect for critical section
		OS_ENTER_CRITICAL();
	  #endif
		old_mask = (R_INT_IRQMASK >> irc_num) & 0x1;
		R_INT_IRQMASK |= (1 << irc_num);
	  #if _OPERATING_SYSTEM != _OS_NONE
		OS_EXIT_CRITICAL();
	  #endif

		return old_mask;
	}

	return STATUS_FAIL;
}

INT32S vic_fiq_register(INT32U irc_num, void (*isr)(void))
{
  #if _OPERATING_SYSTEM != _OS_NONE
	OS_CPU_SR cpu_sr;
  #endif

	if (irc_num && (irc_num<VIC_MAX_FIQ)) {
		irc_num--;

	  #if _OPERATING_SYSTEM != _OS_NONE			// Soft Protect for critical section
		OS_ENTER_CRITICAL();
	  #endif
		fiq_isr[irc_num] = isr;
	  #if _OPERATING_SYSTEM != _OS_NONE
		OS_EXIT_CRITICAL();
	  #endif

		return STATUS_OK;
	}

	return STATUS_FAIL;
}

INT32S vic_fiq_unregister(INT32U irc_num)
{
  #if _OPERATING_SYSTEM != _OS_NONE
	OS_CPU_SR cpu_sr;
  #endif

	if (irc_num && (irc_num<VIC_MAX_FIQ)) {
		irc_num--;

	  #if _OPERATING_SYSTEM != _OS_NONE			// Soft Protect for critical section
		OS_ENTER_CRITICAL();
	  #endif
		fiq_isr[irc_num] = NULL;
	  #if _OPERATING_SYSTEM != _OS_NONE
		OS_EXIT_CRITICAL();
	  #endif

		return STATUS_OK;
	}

	return STATUS_FAIL;
}

INT32S vic_fiq_enable(INT32U irc_num)
{
	INT32S old_mask;
  #if _OPERATING_SYSTEM != _OS_NONE
	OS_CPU_SR cpu_sr;
  #endif

	if (irc_num && (irc_num<VIC_MAX_FIQ)) {
		irc_num = VIC_MAX_FIQ - irc_num - 1;

	  #if _OPERATING_SYSTEM != _OS_NONE			// Soft Protect for critical section
		OS_ENTER_CRITICAL();
	  #endif
		old_mask = (R_INT_FIQMASK >> irc_num) & 0x1;
		R_INT_FIQMASK &= ~(1 << irc_num);
	  #if _OPERATING_SYSTEM != _OS_NONE
		OS_EXIT_CRITICAL();
	  #endif

		return old_mask;
	}

	return STATUS_FAIL;
}

INT32S vic_fiq_disable(INT32U irc_num)
{
	INT32S old_mask;
  #if _OPERATING_SYSTEM != _OS_NONE
	OS_CPU_SR cpu_sr;
  #endif

	if (irc_num && (irc_num<VIC_MAX_FIQ)) {
		irc_num = VIC_MAX_FIQ - irc_num - 1;

	  #if _OPERATING_SYSTEM != _OS_NONE			// Soft Protect for critical section
		OS_ENTER_CRITICAL();
	  #endif
		old_mask = (R_INT_FIQMASK >> irc_num) & 0x1;
		R_INT_FIQMASK |= (1 << irc_num);
	  #if _OPERATING_SYSTEM != _OS_NONE
		OS_EXIT_CRITICAL();
	  #endif

		return old_mask;
	}

	return STATUS_FAIL;
}

#if _OPERATING_SYSTEM != _OS_NONE
void irq_dispatcher(void)
#else
void IRQ irq_dispatcher(void)
#endif
{
	INT32U vector;

	if (R_INT_GMASK & C_VIC_GLOBAL_MASK_SET) {		// Spurious interrupt. Do nothing.
		return;
	}

	vector = R_INT_IRQNUM;
	while (vector) {

	//if (vector!=25) // timer for system tick
    //    DBG_PRINT("IRQ=%d\r\n", vector);	
	//if (vector==27)
	//DBG_PRINT("IRQ=27!!!!!!!!!!!!!!!!!!!!\r\n");


		if (vector < VIC_MAX_IRQ) {
			vector--;
			if (irq_isr[vector]) {				// Check IRQ status register to see which interrupt source is set
				// TBD: Enable Device Protect and then disable Hard Protect here for nesting interrupt
				(*irq_isr[vector])();
				// TBD: Enable Hard Protect and then disable Device Protect here for nesting interrupt
			}
		}

		vector = R_INT_IRQNUM;				// Get next pending interrupt source
	}
}

#if _OPERATING_SYSTEM != _OS_NONE
void fiq_dispatcher(void)
#else
void IRQ fiq_dispatcher(void)
#endif
{
	INT32U vector;

	if (R_INT_GMASK & C_VIC_GLOBAL_MASK_SET) {		// Spurious interrupt. Do nothing.
		return;
	}

	vector = R_INT_FIQNUM;

	while (vector) {
	
//DBG_PRINT("FIQ=%d\r\n", vector);	
	
		if (vector < VIC_MAX_FIQ) {
			vector--;
			if (fiq_isr[vector]) {				// Check FIQ status register to see which interrupt source is set
				// TBD: Enable Device Protect and then disable Hard Protect here for nesting interrupt
				(*fiq_isr[vector])();
				// TBD: Enable Hard Protect and then disable Device Protect here for nesting interrupt
			}
		}

		vector = R_INT_FIQNUM;
	}

}

void vic_init(void)
{
	INT32U i;

	R_INT_IRQMASK = C_VIC_IRQ_MASK_SET_ALL;	// Mask all interrupt
	R_INT_FIQMASK = C_VIC_FIQ_MASK_SET_ALL;	// Mask all fast interrupt
	R_INT_GMASK = C_VIC_GLOBAL_MASK_CLEAR;	// Clear global mask

	for (i=0; i<VIC_MAX_IRQ; i++) {
		irq_isr[i] = NULL;
	}
	for (i=0; i<VIC_MAX_FIQ; i++) {
		fiq_isr[i] = NULL;
	}

  #if _OPERATING_SYSTEM != _OS_NONE
  	register_exception_table(EXCEPTION_IRQ, (INT32U) OS_CPU_IRQ_ISR);
	register_exception_table(EXCEPTION_FIQ, (INT32U) OS_CPU_FIQ_ISR);
  #else
	register_exception_table(EXCEPTION_IRQ, (INT32U) irq_dispatcher);
	register_exception_table(EXCEPTION_FIQ, (INT32U) fiq_dispatcher);
  #endif
}

