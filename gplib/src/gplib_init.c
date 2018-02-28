#include "gplib.h"


void gplib_init(INT32U free_memory_start, INT32U free_memory_end)
{
  #if GPLIB_MEMORY_MANAGEMENT_EN == 1
  	gp_mm_init(free_memory_start, free_memory_end);		// Initiate memory management module to provoide SDRAM/IRAM allocate and free functions
  #endif

  #if GPLIB_FILE_SYSTEM_EN == 1
  	fs_init();								// Initiate file system module
  #endif

  #if GPLIB_JPEG_DECODE_EN == 1
//  	jpeg_decode_init();						// Initiate JPEG decoding module
  #endif
}
