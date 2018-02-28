/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _IPXE_ARC4_H
#define _IPXE_ARC4_H
#include <ssv_types.h>
#define U8 u8

struct arc4_ctx {
	int i, j;
	U8 state[256];
};

#define ARC4_CTX_SIZE sizeof ( struct arc4_ctx )

void arc4_skip ( const void *key, size_t keylen, size_t skip, const void *src, void *dst, size_t msglen );
int arc4_setkey ( void *ctxv, const void *keyv, size_t keylen );
void arc4_xor ( void *ctxv, const void *srcv, void *dstv, size_t len );

#endif /* _IPXE_ARC4_H */
