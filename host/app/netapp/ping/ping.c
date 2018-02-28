/**
 * @file
 * Ping sender module
 *
 */

/*
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 */

/**
 * This is an example of a "ping" sender (with raw API and socket API).
 * It can be used as a start point to maintain opened a network connection, or
 * like a network "watchdog" for your device.
 *
 */

#include <ssv_lib.h>
#include <log.h>
#include "lwip/opt.h"

static unsigned int ping_count = 0;
static unsigned int recv_count = 0;
static unsigned int miss_count = 0;

static unsigned int min_timeout = 0;
static unsigned int max_timeout = 0;
static unsigned int sum_timeout = 0;
static unsigned int avg_timeout = 0;

#if LWIP_RAW /* don't build if not configured for use in lwipopts.h */

#include "ping.h"

#include "lwip/mem.h"
#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/netif.h"
#include "lwip/sys.h"
#include "lwip/timers.h"
#include "lwip/inet_chksum.h"
#include "lwip/ip_addr.h"

#if PING_USE_SOCKETS
#include "lwip/sockets.h"
#include "lwip/inet.h"
#endif /* PING_USE_SOCKETS */


/**
 * PING_DEBUG: Enable debugging for PING.
 */
#ifndef PING_DEBUG
#define PING_DEBUG     LWIP_DBG_ON
#endif

/** ping target - should be a "ip_addr_t" */
#ifndef PING_TARGET
#define PING_TARGET   (netif_default?netif_default->gw:ip_addr_any)
#endif

/** ping receive timeout - in milliseconds */
#ifndef PING_RCV_TIMEO
#define PING_RCV_TIMEO 1000
#endif

/** ping delay - in milliseconds */
#ifndef PING_DELAY
#define PING_DELAY     1000
#endif
static unsigned int     _ping_delay = PING_DELAY;

/** ping identifier - must fit on a u16_t */
#ifndef PING_ID
#define PING_ID        0xAFAF
#endif

/** ping additional data size to include in the packet */
#ifndef PING_DATA_SIZE
#define PING_DATA_SIZE 32
#endif

#if LWIP_NETIF_TX_SINGLE_PBUF 
#define MAX_PING_DATA_SIZE 1464		//-8 byte for icmp header
#else
#define MAX_PING_DATA_SIZE 1472
#endif

/** ping result action - no default action */
#ifndef PING_RESULT
#define PING_RESULT(ping_ok)
#endif

/* ping variables */
static u16_t ping_seq_num;
static u32_t ping_time;
#if !PING_USE_SOCKETS
static struct raw_pcb *ping_pcb;
#endif /* PING_USE_SOCKETS */

// felix:
static size_t sg_ping_size;


/** Prepare a echo ICMP echo request */
static void
ping_prepare_echo( struct icmp_echo_hdr *iecho, u16_t len)
{
  size_t i;
  size_t data_len = len - sizeof(struct icmp_echo_hdr);

  ICMPH_TYPE_SET(iecho, ICMP_ECHO);
  ICMPH_CODE_SET(iecho, 0);
  iecho->chksum = 0;
  iecho->id     = PING_ID;
  iecho->seqno  = htons(++ping_seq_num);

  /* fill the additional data buffer with some data */
  for(i = 0; i < data_len; i++) {
    ((char*)iecho)[sizeof(struct icmp_echo_hdr) + i] = (char)i;
  }

  iecho->chksum = inet_chksum((void*)iecho, len);
}

#if PING_USE_SOCKETS

/* Ping using the socket ip */
static err_t
ping_send(int s, ip_addr_t *addr)
{
  int err;
  struct icmp_echo_hdr *iecho;
  struct sockaddr_in to;
// felix: for icmp length specified
//  size_t ping_size = sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE;
  size_t ping_size = sizeof(struct icmp_echo_hdr) + sg_ping_size;
  LWIP_ASSERT("ping_size is too big", ping_size <= 0xffff);

  iecho = (struct icmp_echo_hdr *)mem_malloc((mem_size_t)ping_size);
  if (!iecho) {
    return ERR_MEM;
  }
  ping_count++;
  ping_time = sys_now();

  ping_prepare_echo(iecho, (u16_t)ping_size);

  to.sin_len = sizeof(to);
  to.sin_family = AF_INET;
  inet_addr_from_ipaddr(&to.sin_addr, addr);

  err = lwip_sendto(s, (void*)iecho, ping_size, 0, (struct sockaddr*)&to, sizeof(to));

  mem_free((void*)iecho);

  return (err ? ERR_OK : ERR_VAL);
}

static void
ping_recv(int s)
{
  char buf[64];
  int fromlen, len;
  struct sockaddr_in from;
  struct ip_hdr *iphdr;
  struct icmp_echo_hdr *iecho;
  u32_t cur_timeout = 0;

  fromlen = sizeof(struct sockaddr_in);
  while((len = lwip_recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*)&from, (socklen_t*)&fromlen)) > 0) {
    if (len >= (int)(sizeof(struct ip_hdr)+sizeof(struct icmp_echo_hdr))) {
		ip_addr_t fromaddr;

        cur_timeout = (sys_now() - ping_time);
		inet_addr_to_ipaddr(&fromaddr, &from.sin_addr);
		LWIP_DEBUGF( PING_DEBUG, ("ping: recv "));
		ip_addr_debug_print(PING_DEBUG, &fromaddr);
		LWIP_DEBUGF( PING_DEBUG, (" %"U32_F" ms\r\n", cur_timeout));
		//LOG_PRINTF("ping recv : %"U32_F" ms\n", cur_timeout);
		iphdr = (struct ip_hdr *)buf;
		iecho = (struct icmp_echo_hdr *)(buf + (IPH_HL(iphdr) * 4));


		if ((iecho->id == PING_ID) && (iecho->seqno == htons(ping_seq_num))) {
            recv_count++;

            if (cur_timeout > max_timeout)
            {
                max_timeout = cur_timeout;
            }
            if (cur_timeout < min_timeout)
            {
                min_timeout = cur_timeout;
            }

            sum_timeout += cur_timeout;

	  		LOG_PRINTF("\r\n(%d, %d, %d) %d bytes from %d.%d.%d.%d: icmp_seq=%d ttl=%d time=%d ms\r\n",
	  		    recv_count, miss_count, ping_count, ntohs(IPH_LEN(iphdr))-(IPH_HL(iphdr) * 4),
			    (from.sin_addr.s_addr>>0)&0xFF,	(from.sin_addr.s_addr>>8)&0xFF,
			    (from.sin_addr.s_addr>>16)&0xFF, (from.sin_addr.s_addr>>24)&0xFF,
			    ntohs(iecho->seqno), IPH_TTL(iphdr), cur_timeout);

    		/* do some ping result processing */
    		PING_RESULT((ICMPH_TYPE(iecho) == ICMP_ER));
    		return;
		}else {
    		LWIP_DEBUGF( PING_DEBUG, ("ping: drop\r\n"));
		}
    }
  }

  if (len == 0 || len < 0) {
	 miss_count++;
     LWIP_DEBUGF( PING_DEBUG, ("ping: recv - %"U32_F" ms - timeout\r\n", cur_timeout));
     LOG_PRINTF("(%d, %d, %d) icmp request (icmp_seq=%d) timeout!\r\n", recv_count, miss_count, ping_count, ping_seq_num);
  }

  /* do some ping result processing */
  PING_RESULT(0);
}

static void
ping_thread(void *arg)
{
  const char *ipaddr = (const char *)arg;
  int s;
  int timeout = PING_RCV_TIMEO;
  ip_addr_t ping_target;

  //LWIP_UNUSED_ARG(arg);

  if ((s = lwip_socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP)) < 0) {
    return;
  }

  lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

  while (1) {

	ping_target.addr=inet_addr(ipaddr);

    if (ping_send(s, &ping_target) == ERR_OK) {
      LWIP_DEBUGF( PING_DEBUG, ("ping: send "));
      ip_addr_debug_print(PING_DEBUG, &ping_target);
      LWIP_DEBUGF( PING_DEBUG, ("\n"));

      ping_recv(s);
    } else {
      LWIP_DEBUGF( PING_DEBUG, ("ping: send "));
      ip_addr_debug_print(PING_DEBUG, &ping_target);
      LWIP_DEBUGF( PING_DEBUG, (" - error\n"));
    }
    sys_msleep(_ping_delay);
  }
}

#else /* PING_USE_SOCKETS */



/* Ping using the raw ip */
static u8_t
ping_recv(void *arg, struct raw_pcb *pcb, struct pbuf *p, ip_addr_t *addr)
{
  struct icmp_echo_hdr *iecho;
  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(pcb);
  LWIP_UNUSED_ARG(addr);
  LWIP_ASSERT("p != NULL", p != NULL);

  if (pbuf_header( p, -PBUF_IP_HLEN)==0) {
    iecho = (struct icmp_echo_hdr *)p->payload;

    if ((iecho->id == PING_ID) && (iecho->seqno == htons(ping_seq_num))) {
      LWIP_DEBUGF( PING_DEBUG, ("(%d, %d, %d) ping: recv "), recv_count, miss_count, ping_count);
      ip_addr_debug_print(PING_DEBUG, addr);
      LWIP_DEBUGF( PING_DEBUG, (" %"U32_F" ms\n", (sys_now()-ping_time)));

      /* do some ping result processing */
      PING_RESULT(1);
      pbuf_free(p);
      return 1; /* eat the packet */
    }
  }

  return 0; /* don't eat the packet */
}

static void
ping_send(struct raw_pcb *raw, ip_addr_t *addr)
{
  struct pbuf *p;
  struct icmp_echo_hdr *iecho;
  size_t ping_size = sizeof(struct icmp_echo_hdr) + PING_DATA_SIZE;

  LWIP_DEBUGF( PING_DEBUG, ("ping: send "));
  ip_addr_debug_print(PING_DEBUG, addr);
  LWIP_DEBUGF( PING_DEBUG, ("\n"));
  LWIP_ASSERT("ping_size <= 0xffff", ping_size <= 0xffff);

  p = pbuf_alloc(PBUF_IP, (u16_t)ping_size, PBUF_RAM);
  if (!p) {
    return;
  }
  if ((p->len == p->tot_len) && (p->next == NULL)) {
    iecho = (struct icmp_echo_hdr *)p->payload;

    ping_prepare_echo(iecho, (u16_t)ping_size);

    raw_sendto(raw, p, addr);
    ping_time = sys_now();
  }
  pbuf_free(p);
}

static void
ping_timeout(void *arg)
{
  struct raw_pcb *pcb = (struct raw_pcb*)arg;
  ip_addr_t ping_target = PING_TARGET;

  LWIP_ASSERT("ping_timeout: no pcb given!", pcb != NULL);

  ping_send(pcb, &ping_target);

  sys_timeout(PING_DELAY, ping_timeout, pcb);
}

static void
ping_raw_init(void)
{
  ping_pcb = raw_new(IP_PROTO_ICMP);
  LWIP_ASSERT("ping_pcb != NULL", ping_pcb != NULL);

  raw_recv(ping_pcb, ping_recv, NULL);
  raw_bind(ping_pcb, IP_ADDR_ANY);
  sys_timeout(PING_DELAY, ping_timeout, ping_pcb);
}

void
ping_send_now()
{
  ip_addr_t ping_target = PING_TARGET;
  LWIP_ASSERT("ping_pcb != NULL", ping_pcb != NULL);
  ping_send(ping_pcb, &ping_target);
}

#endif /* PING_USE_SOCKETS */

void
ping_init(char *ipaddr)
{
#if PING_USE_SOCKETS
  sys_thread_new("ping_thread", ping_thread, (void*)ipaddr, (PING_THREAD_STACK_SIZE<<4), 2);

#else /* PING_USE_SOCKETS */
  ping_raw_init();
#endif /* PING_USE_SOCKETS */
}


/* Added by Felix: */
static void ping_usage(void)
{
    LOG_PRINTF("Usage: ping [-c count] [-s size] [-d delay(ms) min:%d ms] destination\r\n\n", TICK_RATE_MS);
    
}

/* Added by Felix: */
void net_app_ping(s32 argc, char *argv[])
{
    //s32 ping_size=PING_DATA_SIZE;
    s32 ping_cnt=4;
    s32 k, i, s, timeout = PING_RCV_TIMEO;
    ip_addr_t ping_target;
    char *target = NULL;

    sg_ping_size = PING_DATA_SIZE;
    ping_count = 0;
    recv_count = 0;
    miss_count = 0;

    min_timeout = 1000000;
    max_timeout = 0;
    sum_timeout = 0;
    avg_timeout = 0;
    _ping_delay = PING_DELAY;
    
	if(argc==1)
	{
		ping_usage();
		return;
	}


    for(i=1; i<argc; i++) {
        if (strcmp(argv[i], "-c") == 0) {
            if (++i >= argc) {
                ping_usage();
                return;
            }
            ping_cnt = ssv6xxx_atoi(argv[i]);
            continue;
        }
        if (strcmp(argv[i], "-s") == 0) {
            if (++i >= argc) {
                ping_usage();
                return;
            }
            sg_ping_size = ssv6xxx_atoi(argv[i]);
            if (sg_ping_size > MAX_PING_DATA_SIZE){
                LOG_PRINTF("ERROR: -s <size>: maximum size is %d \r\n", MAX_PING_DATA_SIZE);
                return;
            }
            continue;
        }
        if (strcmp(argv[i], "-d") == 0) {
            if (++i >= argc) {
                ping_usage();
                return;
            }
            _ping_delay = ssv6xxx_atoi(argv[i]);
                       
            if (_ping_delay < TICK_RATE_MS){
                LOG_PRINTF("ERROR: -d <delay>: minimum size is %d \r\n", TICK_RATE_MS);
                return;
            }
                
            continue;
        }

        ping_target.addr=inet_addr(argv[i]);
        if (NULL != target || ping_target.addr == IPADDR_NONE){
            ping_usage();
            return;
        }
        target = argv[i];
    }

    if ((s = lwip_socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP)) < 0)
        return;
    lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    LOG_PRINTF("PING %s %d(%d) bytes of data.\r\n", target, sg_ping_size, sg_ping_size+28);
    for(k=0; k<ping_cnt; k++) {
        if (ping_send(s, &ping_target) == ERR_OK) {
            ping_time = sys_now();
            ping_recv(s);
        } else {
            LOG_PRINTF("ping: fatal error !!\r\n");
            break;
        }
        sys_msleep(_ping_delay);
    }
    LOG_PRINTF("\r\nrecv_count:%d, miss_count:%d, ping_count:%d\r\n", recv_count, miss_count, ping_count);
    avg_timeout = (recv_count > 0) ? (sum_timeout/recv_count) : 0;
    if (min_timeout > max_timeout)
        min_timeout = max_timeout;  //in case of all packets lost
    LOG_PRINTF("\r\nrtt min/avg/max = %d/%d/%d ms\r\n", min_timeout, avg_timeout, max_timeout);
    lwip_close(s);

}





#endif /* LWIP_RAW */
