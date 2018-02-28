#ifndef __drv_l1_DMA_H__
#define __drv_l1_DMA_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "project.h"


// DMA Controller
#define C_DMA_STATUS_WAITING        0
#define C_DMA_STATUS_DONE           1
#define C_DMA_STATUS_TIMEOUT        -1


typedef enum {
    DMA_DATA_WIDTH_1BYTE=1,
    DMA_DATA_WIDTH_2BYTE=2,
    DMA_DATA_WIDTH_4BYTE=4
}DMA_DW_ENUM;


typedef struct {
    INT32U s_addr;                          // Source address. Must align to data width
    INT32U t_addr;                          // Target address. Must align to data width
    INT32U count;                           // Transfer count(1~0x00FFFFFF). Total transfer length = width * count
    DMA_DW_ENUM width;
    INT8U timeout;                          // 0: No timeout, 1~255: 1/256 ~ 255/256 second
    INT8U channel;                          // This member is used internally by DMA driver. Don't modify its value.
    INT8S *notify;                          // NULL: notification is not needed. DMA will set C_DMA_STATUS_DONE/C_DMA_STATUS_TIMEOUT when transfer is finish.
} DMA_STRUCT;

extern void dma_init(void);
extern INT32S dma_transfer(DMA_STRUCT *dma_struct);
extern INT32S dma_transfer_wait_ready(DMA_STRUCT *dma_struct);
#if _OPERATING_SYSTEM != _OS_NONE
extern INT32S dma_transfer_with_queue(DMA_STRUCT *dma_struct, OS_EVENT *os_q);
extern INT32S dma_transfer_with_double_buf(DMA_STRUCT *dma_struct, OS_EVENT *os_q);
extern INT32S dma_transfer_double_buf_set(DMA_STRUCT *dma_struct);
extern INT32S dma_transfer_double_buf_free(DMA_STRUCT *dma_struct);
#endif
extern INT32S dma_memory_fill(INT32U t_addr, INT8U value, INT32U byte_count);
extern INT32S dma_buffer_copy(INT32U s_addr, INT32U t_addr, INT32U byte_count, INT32U s_width, INT32U t_width);		// All parameters must be multiple of 2, s_width(source buffer width) must <= t_width(target buffer width)
INT32S dma_status_get(INT8U channel) ;
INT32S dma_dbf_status_get(INT8U channel) ;

#ifdef __cplusplus
}
#endif

#endif		// __drv_l1_DMA_H__

