#ifndef _ETHERNETIF_H_
#define _ETHERNETIF_H_
#include "lwip/netif.h"

err_t ethernetif_input(void *dat, u32 len);//创建该进程时，要将某个网络接口结构的 netif 结构指
												//针作为参数传入
err_t ethernetif_init(struct netif *netif);
#endif //#ifndef _ETHERNETIF_H_
