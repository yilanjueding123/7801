/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _SDIO_H_
#define _SDIO_H_

//output timing
// 0: cmd  [0]:positive [1]:negative
// 1: data [0]:positive [1]:negative
#define SDIO_DEF_OUTPUT_TIMING 		0 // 3

extern bool sdio_writeb_cmd52(u32 addr, u8 data);
extern bool sdio_write_cmd53(u32 dataPort,u8 *dat, size_t size);
extern bool sdio_read_cmd53(u32 dataPort,u8 *dat, size_t size);
extern bool _sdio_read_reg(u32 addr, u32 *data);
extern bool sdio_set_block_size(unsigned int blksize);
extern u32 sdio_get_block_size();
extern bool sdio_host_init(void (*sdio_isr)(void));
extern bool sdio_host_enable_isr(bool enable);
extern bool is_truly_isr();
#endif /* _SDIO_H_ */

