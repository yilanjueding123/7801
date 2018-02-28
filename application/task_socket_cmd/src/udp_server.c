//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
#include "lwip/api.h"
#include "stdio.h"
#include "string.h"
#include "sockets.h"
//#include "rtsp.h"
//#include "rtp.h"

#include "lwip/tcpip.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
extern void uart_data_send(INT8U uart_num,INT8U data, INT8U wait);
//---------------------------------------------------------------------------------------
#define WIFI_APP_VERIFY_LEN			9
unsigned char wifi_app_verify[WIFI_APP_VERIFY_LEN+1] = "JOYHONEST";
//---------------------------------------------------------------------------------------

int new_udp_res_cmd_send(struct udp_pcb *pcb, INT32U *data, INT32U len){
	
	//char buffer[16];	
	//char *ptr = data;
	struct pbuf *p;

	p = pbuf_alloc(PBUF_TRANSPORT,len,PBUF_RAM);
	memcpy (p->payload, data, len);
	udp_send(pcb, p);
	pbuf_free(p); //De-allocate packet buffer
		
	return 0;
}

extern void wifi_menu_language_set(INT8U sta);
extern void wifi_app_system_type_set(INT8U sta);
void udp_new_cmd_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
	INT16U	byte_num; //i;
	INT8U	tx_buf[40];
    struct pbuf *tmp_p=NULL;

	if (p != NULL)
	{
    	for(tmp_p=p;tmp_p;tmp_p=tmp_p->next)
		{	
			byte_num = p->len;
			if(byte_num>=40) byte_num = 40;
			memcpy(tx_buf, (void *)((u8*)p->payload), byte_num);
			
			if ((tx_buf[0] != 'U') || (tx_buf[1] != 'D') || (tx_buf[2] != 'P') ||
				(tx_buf[3] != 'S') || (tx_buf[4] != 'O') || (tx_buf[5] != 'C') || 
				(tx_buf[6] != 'K') || (tx_buf[7] != 'E') || (tx_buf[8] != 'T'))
			{
				DBG_PRINT("UDP SOCKET CMD ERROR!!!\r\n", tx_buf[9]);
			}
			else
			{
				if (tx_buf[9]&0x01) wifi_menu_language_set(1);
				else wifi_menu_language_set(0);
				if (tx_buf[9]&0x80)	wifi_app_system_type_set(1);
				else wifi_app_system_type_set(0);
			}
		}
		//new_udp_res_cmd_send(pcb,(INT32U*)wifi_app_verify, WIFI_APP_VERIFY_LEN);
		pbuf_free(p);
	}
	else
	{
		pbuf_free(p);
	} 
}

#define NEW_CMD_UDP_PORT				25010//UDPSRV_SERVER_PORT//
struct udp_pcb  *new_cmd_pcb;

void create_new_udp_for_cmd(void)
{
	new_cmd_pcb = udp_new();
	udp_bind(new_cmd_pcb, IP_ADDR_ANY, NEW_CMD_UDP_PORT);
	udp_recv(new_cmd_pcb, udp_new_cmd_recv, NULL); 
}



