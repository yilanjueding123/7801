#include "gplib_print_string.h"
#include "drv_l1_uart.h"

#if GPLIB_PRINT_STRING_EN == 1

#include <stdarg.h>
#include <stdio.h>

#if _DRV_L1_UART == 1
	#if GPDV_BOARD_VERSION == GPCV1248_MINI
		#define SEND_DATA(x)	drv_l1_uart1_data_send(x, 1)
		#define GET_DATA(x)		drv_l1_uart1_data_get(x, 1)
	#else
		#define SEND_DATA(x)	uart_data_send(UART_USED_NUM, x, 1)
		#define GET_DATA(x)		uart_data_get(UART_USED_NUM, x, 1)
	#endif
#else
	#define SEND_DATA(x)
	#define GET_DATA(x)		(*(x) = '\r')
#endif

static CHAR print_buf[PRINT_BUF_SIZE];

void print_string(CHAR *fmt, ...)
{
    va_list v_list;
    CHAR *pt;

    va_start(v_list, fmt);
    vsprintf(print_buf, fmt, v_list);
    va_end(v_list);

    print_buf[PRINT_BUF_SIZE-1] = 0;
    pt = print_buf;
    while (*pt) {
		SEND_DATA(*pt);
		pt++;
	}
}

void get_string(CHAR *s)
{
    INT8U temp;

    while (1) {
        GET_DATA(&temp);
        SEND_DATA(temp);
        if (temp == '\r') {
            *s = 0;
            return;
        }
        *s++ = (CHAR) temp;
    }
}

#else

void print_string(CHAR *fmt, ...)
{
}

void get_string(CHAR *s)
{
}

#endif		// GPLIB_PRINT_STRING_EN
