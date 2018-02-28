/******************************************************************************
 * Purpose: General Plus Standard Library
 * Author: Allen Lin
 * Date: 2008/01/21
 * Copyright Generalplus Corp. ALL RIGHTS RESERVED.
 ******************************************************************************/
/******************************************************************************
 * Paresent Header Include
 ******************************************************************************/
#include "gp_stdlib.h"

/******************************************************************************
 * Function Prototypes
 ******************************************************************************/
INT32S gp_strcmp(INT8S *s,INT8S *t);
INT8S *gp_strcpy(INT8S *t,INT8S *s);
INT32S gp_strlen(INT8S *s);
INT32S gp_strncmp(INT8S *dest,INT8S *src,INT32U Len);
INT8S gp_toupper(INT8S c);
INT8S *gp_strcat(INT8S *s,INT8S *t);
INT32S gp_memcpy(INT8S *dest, INT8S *src, INT32U Len);
INT32S gp_memset(INT8S *dest,INT8U byte, INT32U Len);
INT32S gp_memcmp(INT8S* dest, INT8S* src, INT32U Len);
void gp_memcpy_align(void *_dst, const void *_src, int len);
 /******************************************************************************
 * Function Body
 ******************************************************************************/
INT32S gp_strcmp(INT8S *s,INT8S *t)
{
	while ( *s == *t ) {
		if ( *s == 0 )
		return(0);
	    ++s;
	    ++t;
	}
	return ( *s - *t );
}

INT8S *gp_strcpy(INT8S *t,INT8S *s)
{
	INT8S *d;

	d = t;
	while(*s) {
	    *t++ = *s++;
	}
	*t = 0x0;			// Add string terminator

	return(d);
}

INT32S gp_strlen(INT8S *s)
{
	INT32S n = 0;
	while (*s++)
	   n++;
	return(n);
}

INT32S gp_strncmp(INT8S *dest,INT8S *src,INT32U Len)
{
	INT32U i = 0;

	for ( i = 0; i < Len; i++ ) {
		if( dest[i] != src[i] ) {
		    if( dest[i] > src[i] ) {
		        return 1;
		    } else {
		        return -1;
		    }
	    }
	}

	return 0;

}

INT8S gp_toupper(INT8S c)
{
	if(((c>='A')&&(c<='Z'))||((c>='a')&&(c<='z')))
		c &= 0xDF;
	return c;
}

INT8S *gp_strcat(INT8S *s,INT8S *t)
{
	INT8S *d;
	d = s;
	--s;
	while(*++s) ;

	do {
		*s=*t++;
	}
	while(*s++);

	return(d);
}

INT32S gp_memcpy(INT8S *dest, INT8S *src, INT32U Len)
{
	INT32U i = 0;

	for ( i = 0; i < Len; i++ ) {
		dest[i] = src[i];
	}

	return(Len);
}

void gp_memcpy_align(void *_dst, const void *_src, int len)
{
	register long *dst = (long*)_dst;
	const register long *src = (long*)_src;
	register long *end = (long*)((char*)_dst + len);
	len >>= 2;
	switch(len & 15)
	{
	case 15:
		__asm{
			LDMIA src!,{r4-r10}
			STMIA dst!,{r4-r10}
			LDMIA src!,{r4-r11}
			STMIA dst!,{r4-r11}
		}
		break;
	case 14:
		__asm{
			LDMIA src!,{r4-r9}
			STMIA dst!,{r4-r9}
			LDMIA src!,{r4-r11}
			STMIA dst!,{r4-r11}
		}
		break;
	case 13:
		__asm{
			LDMIA src!,{r4-r8}
			STMIA dst!,{r4-r8}
			LDMIA src!,{r4-r11}
			STMIA dst!,{r4-r11}
		}
		break;
	case 12:
		__asm{
			LDMIA src!,{r4-r7}
			STMIA dst!,{r4-r7}
			LDMIA src!,{r4-r11}
			STMIA dst!,{r4-r11}
		}
		break;
	case 11:
		__asm{
			LDMIA src!,{r4-r6}
			STMIA dst!,{r4-r6}
			LDMIA src!,{r4-r11}
			STMIA dst!,{r4-r11}
		}
		break;
	case 10:
		__asm{
			LDMIA src!,{r4-r5}
			STMIA dst!,{r4-r5}
			LDMIA src!,{r4-r11}
			STMIA dst!,{r4-r11}
		}
		break;
	case 9:
		__asm{
			LDMIA src!,{r4-r12}
			STMIA dst!,{r4-r12}
		}
		break;
	case 8:
		__asm{
			LDMIA src!,{r4-r11}
			STMIA dst!,{r4-r11}
		}
		break;
	case 7:
		__asm{
			LDMIA src!,{r4-r10}
			STMIA dst!,{r4-r10}
		}
		break;
	case 6:
		__asm{
			LDMIA src!,{r4-r9}
			STMIA dst!,{r4-r9}
		}
		break;
	case 5:
		__asm{
			LDMIA src!,{r4-r8}
			STMIA dst!,{r4-r8}
		}
		break;
	case 4:
		__asm{
			LDMIA src!,{r4-r7}
			STMIA dst!,{r4-r7}
		}
		break;
	case 3:
		__asm{
			LDMIA src!,{r4-r6}
			STMIA dst!,{r4-r6}
		}
		break;
	case 2:
		__asm{
			LDMIA src!,{r4-r5}
			STMIA dst!,{r4-r5}
		}
		break;
	case 1:
		*dst++ = *src++;
	}

	while(dst < end)
	{
		__asm{
			LDMIA src!,{r4-r11}
			STMIA dst!,{r4-r11}
			LDMIA src!,{r4-r11}
			STMIA dst!,{r4-r11}
		}
	}
}

INT32S gp_memset(INT8S *dest,INT8U byte, INT32U Len)
{
	INT32S i = 0;

	for ( i = 0; i < Len; i++ ) {
		dest[i] = byte;
	}

	return(Len);
}

INT32S gp_memcmp(INT8S* dest, INT8S* src, INT32U Len)
{
	INT32U i;

	for (i=0;i<Len;i++) {
		if( dest[i] != src[i] ) {
		    if(dest[i] > src[i]) {
		        return 1;
		    } else {
		        return -1;
		    }
	    }
	}
	
	return 0;
} 


