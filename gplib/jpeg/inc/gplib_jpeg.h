#ifndef __GPLIB_JPEG_H__
#define __GPLIB_JPEG_H__


#include "gplib.h"

INT32U gplib_jpeg_version_get(void);
void default_huffman_dc_lum_table_load(void);

// JPEG load default huffman table.
extern void gplib_jpeg_default_huffman_table_load(void);
extern const INT8U zigzag_scan[64] ;

#endif 		// __GPLIB_JPEG_H__