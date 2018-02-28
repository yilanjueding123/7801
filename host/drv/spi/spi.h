/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _SPI_H_
#define _SPI_H_


extern bool spi_host_init(void (*spi_isr)(void));
//extern bool spi_host_write(u8 *buf, u32 sizeToTransfer, u32 *sizeToTransfered, u32 options);
//extern bool spi_host_read(u8 *buf, u32 sizeToTransfer, u32 *sizeToTransfered, u32 options);
extern void spi_irq_enable(bool enable);
extern bool spi_host_readwrite(u8 *buf, u32 sizeToTransfer, u32 *sizeToTransfered, u8 options, bool IsRead);
extern bool is_truly_isr();
#define spi_host_write(a,b,c,d) spi_host_readwrite(a, b, c, d, FALSE)
#define spi_host_read(a,b,c,d) spi_host_readwrite(a, b, c, d, TRUE)

#endif
