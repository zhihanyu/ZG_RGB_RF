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
#include "s907x.h"
#include "lwip/opt.h"

#if LWIP_RAW /* don't build if not configured for use in lwipopts.h */
#include "lwip/ip_addr.h"
#include "ping_test.h"
#include "ping.h"
#include "lwip/mem.h"
#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/netif.h"
#include "lwip/sys.h"

#include "lwip/inet_chksum.h"

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

/** ping delay - in milliseconds */
#ifndef PING_DELAY
#define PING_DELAY     1000
#endif

/** ping identifier - must fit on a u16_t */
#ifndef PING_ID
#define PING_ID        0xAFAF
#endif

/** ping additional data size to include in the packet */
#ifndef PING_DATA_SIZE
#define PING_DATA_SIZE 32
#endif

/** ping result action - no default action */
#ifndef PING_RESULT
#define PING_RESULT(ping_ok)
#endif
 
/* ping variables */
static u32_t ping_done;

static u32_t is_ping_ongoing = 0;
  
#if !PING_USE_SOCKETS 
static struct raw_pcb *ping_pcb;
#endif /* PING_USE_SOCKETS */

typedef struct _ping_arg
{
    u32_t count;
    u32_t size;
    ping_request_result_t callback;
    ip_addr_t ip;
} ping_arg_t;
ping_arg_t g_ping_arg;


sema_t ping_wake_sema;
sema_t ping_stop_sema;
/** Prepare a echo ICMP request */
static void
ping_prepare_echo( struct icmp_echo_hdr *iecho, u16_t len, ping_static_t *p_ping_static)
{
    size_t i;
    size_t data_len = len - sizeof(struct icmp_echo_hdr);

    ICMPH_TYPE_SET(iecho, ICMP_ECHO);
    ICMPH_CODE_SET(iecho, 0);
    iecho->chksum = 0;
    iecho->id     = PING_ID;
    (p_ping_static->ping_seq_num) = (p_ping_static->ping_seq_num) + 1;
    iecho->seqno  = htons(p_ping_static->ping_seq_num);

    /* fill the additional data buffer with some data */
    for(i = 0; i < data_len; i++)
    {
        ((char*)iecho)[sizeof(struct icmp_echo_hdr) + i] = (char)i;
    }

    iecho->chksum = inet_chksum(iecho, len);
}

#if PING_USE_SOCKETS

/* Ping using the socket ip */
err_t
ping_send(int s, ip_addr_t *addr, ping_static_t *p_ping_static)
{
    int err;
    struct icmp_echo_hdr *iecho;
    struct sockaddr_in to;
    size_t ping_size = sizeof(struct icmp_echo_hdr) + (p_ping_static->size);
    LWIP_ASSERT("ping_size is too big", ping_size <= 0xffff);

    iecho = (struct icmp_echo_hdr *)mem_malloc((mem_size_t)ping_size);
    if (!iecho)
    {
        return ERR_MEM;
    }

    ping_prepare_echo(iecho, (u16_t)ping_size, p_ping_static);

    PING_LOGI("ping: send seq(0x%04X) %"U16_F".%"U16_F".%"U16_F".%"U16_F,     \
                        p_ping_static->ping_seq_num,\
                        ip4_addr1_16(addr),         \
                        ip4_addr2_16(addr),         \
                        ip4_addr3_16(addr),         \
                        ip4_addr4_16(addr));

    to.sin_len = sizeof(to);
    to.sin_family = AF_INET;
    inet_addr_from_ipaddr(&to.sin_addr, addr);
 
    ping_done = 0;
    err = lwip_sendto(s, iecho, ping_size, 0, (struct sockaddr*)&to, sizeof(to));

    mem_free(iecho);

    return (err == ping_size ? ERR_OK : ERR_VAL);
} 

err_t
ping_recv(int s, ip_addr_t *addr, ping_static_t *p_ping_static)
{
    char buf[64];
    int fromlen, len;
    struct sockaddr_in from;
    struct ip_hdr *iphdr;
    struct icmp_echo_hdr *iecho;

    fromlen = sizeof(struct sockaddr_in);

	while((sys_now() - p_ping_static->ping_time) < PING_RCV_TIMEO){
		if((len = lwip_recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*)&from,
                               (socklen_t*)&fromlen)) > 0){
	        if (len >= (int)(sizeof(struct ip_hdr) + sizeof(struct icmp_echo_hdr)))
	        {
	            ip_addr_t fromaddr;
	            u32_t cur_time = sys_now() - p_ping_static->ping_time;

	            inet_addr_to_ipaddr(&fromaddr, &from.sin_addr);
	            /* LWIP_DEBUGF( PING_DEBUG, ("ping: recv ")); */
	            iphdr = (struct ip_hdr *)buf;
	            iecho = (struct icmp_echo_hdr *)(buf + (IPH_HL(iphdr) * 4));

	            /* ignore packet if it is not ping reply */
	            if ((0 != (iecho->type)) || ((addr->addr) != (fromaddr.addr)))
					continue;

	            if ((iecho->id == PING_ID) && (iecho->seqno == htons(p_ping_static->ping_seq_num)))
	            {
	                PING_LOGI("ping: recv seq(0x%04X) %"U16_F".%"U16_F".%"U16_F".%"U16_F", %"U32_F" ms", \
	                                    htons(iecho->seqno),             \
	                                    ip4_addr1_16(&fromaddr),         \
	                                    ip4_addr2_16(&fromaddr),         \
	                                    ip4_addr3_16(&fromaddr),         \
	                                    ip4_addr4_16(&fromaddr),         \
	                                    cur_time);

	                /* LWIP_DEBUGF( PING_DEBUG, (" %"U32_F" ms\n", (sys_now() - ping_time))); */
	                if(p_ping_static->ping_min_time == 0 || p_ping_static->ping_min_time > cur_time)
	                {
	                    p_ping_static->ping_min_time = cur_time;
	                }
	                if(p_ping_static->ping_max_time == 0 || p_ping_static->ping_max_time < cur_time)
	                {
	                    p_ping_static->ping_max_time = cur_time;
	                }
	                p_ping_static->ping_avg_time = p_ping_static->ping_avg_time + cur_time;

	                ping_done = 1;
	                p_ping_static->ping_recv_num = p_ping_static->ping_recv_num + 1;

	                /* do some ping result processing */
	                PING_RESULT((ICMPH_TYPE(iecho) == ICMP_ER));
	                return ERR_OK;
	            }
	            else
	            {
	                /* Treat ping ack received after timeout as success */
	                PING_LOGI("ping: Get ping ACK seq(0x%04X), expected seq(0x%04X)", htons(iecho->seqno), p_ping_static->ping_seq_num);
	                /* Can not return, due to there could be ping ack which has matched sequence num. */
					if(htons(iecho->seqno) == p_ping_static->ping_seq_num){
						//p_ping_static->ping_recv_num = p_ping_static->ping_recv_num + 1;
						//p_ping_static->ping_lost_num = p_ping_static->ping_lost_num - 1;
					}
	            }
	        }
		}
	}

    PING_LOGI("--- ping: timeout");
    p_ping_static->ping_lost_num = p_ping_static->ping_lost_num + 1;

    /* do some ping result processing */
    PING_RESULT(0);
	return ERR_TIMEOUT;
}

static void
ping_thread(void *arg)
{
    int s;
    //int timeout = PING_RCV_TIMEO;
    int ret;

    struct timeval timeout;

    ip_addr_t ping_target;
    u32_t residual_count = (((ping_arg_t *)arg)->count);
    ping_request_result_t callback = ((ping_arg_t *)arg)->callback;
    ping_static_t ping_static = {0};
    ping_result_t ping_result = {0};

    timeout.tv_sec  = PING_RCV_TIMEO/1000; //set recvive timeout = 1(sec)
    timeout.tv_usec = (PING_RCV_TIMEO%1000)*1000;

    if ((s = lwip_socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP)) < 0)
    {
        is_ping_ongoing = 0;
        wl_destory_threadself();
    }

    lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    ping_target.addr = (((ping_arg_t *)arg)->ip.addr);
    ping_static.size = (((ping_arg_t *)arg)->size);
    ping_static.ping_seq_num = 0;
    ping_static.count = (((ping_arg_t *)arg)->count);

    ping_static.ping_lost_num = 0;
    ping_static.ping_recv_num = 0;
    do
    {

        if (ping_send(s, &ping_target, &ping_static) == ERR_OK)
        {
#if 0
            LWIP_DEBUGF( PING_DEBUG, ("ping: send "));
            ip_addr_debug_print(PING_DEBUG, &ping_target);
            LWIP_DEBUGF( PING_DEBUG, ("\n"));
#endif
            ping_static.ping_time = sys_now();
            ping_recv(s, &ping_target, &ping_static);
        }
        else
        {
            LWIP_DEBUGF( PING_DEBUG, ("ping: send "));
            ip_addr_debug_print(PING_DEBUG, &ping_target);
            PING_LOGI(" - error");
        }

        ret = wl_wait_sema(&ping_stop_sema, PING_DELAY);    
        if(ret) {
            ping_static.count = ping_static.count - residual_count;
            break;
        }

        if (0 != (ping_static.count))
        {
            residual_count--;
        }
        else
        {
            residual_count = 1;
        }
    }
    while (residual_count > 0);
    ping_static.ping_avg_time = (int)((ping_static.ping_avg_time)/ping_static.count);

    ping_result.min_time = (int)ping_static.ping_min_time;
    ping_result.max_time = (int)ping_static.ping_max_time;
    ping_result.avg_time = (int)ping_static.ping_avg_time;
    ping_result.total_num = (int)ping_static.count;
    ping_result.recv_num = (int)ping_static.ping_recv_num;
    ping_result.lost_num = (int)ping_static.ping_lost_num;

    PING_LOGI("%"U16_F".%"U16_F".%"U16_F".%"U16_F", Packets: Sent = %d, Received =%d, Lost = %d (%d%% loss)",\
                        ip4_addr1_16(&ping_target),         \
                        ip4_addr2_16(&ping_target),         \
                        ip4_addr3_16(&ping_target),         \
                        ip4_addr4_16(&ping_target),         \
                        (int)ping_result.total_num,         \
                        (int)ping_result.recv_num,          \
                        (int)ping_result.lost_num,          \
                        (int)((ping_result.lost_num * 100)/ping_result.total_num));
    PING_LOGI(" Packets: min = %d, max =%d, avg = %d", (int)ping_result.min_time, (int)ping_result.max_time, (int)ping_result.avg_time);
    if(callback != NULL)
    {
        callback(&ping_result);
    }
    lwip_close(s);
    is_ping_ongoing = 0;
    wl_send_sema(&ping_wake_sema);
    wl_destory_threadself();
}

#endif //#if PING_USE_SOCKETS

uint32_t get_ping_done()
{
    return ping_done;
}

void ping_init(void *argv)
{
    ping_argv_t *pargv =  (ping_argv_t *)argv;
    if(!argv) {
        return;
    }
    if(is_ping_ongoing == 1)
    {
        PING_LOGI("Ping is onging, please try it later.");
        return;
    }
    is_ping_ongoing = 1;
    g_ping_arg.count = pargv->cnt;
    g_ping_arg.size = pargv->len;
    g_ping_arg.callback = NULL;
    g_ping_arg.ip.addr = pargv->ip.addr;
    wl_init_sema(&ping_stop_sema, 0, sema_binary);
    wl_init_sema(&ping_wake_sema, 0, sema_binary);

#if PING_USE_SOCKETS
    wl_create_thread(PING_TASK_NAME, PING_TASK_STACK_SZ, PING_TASK_PRIO,  ping_thread, (void *)(&g_ping_arg));
#else /* PING_USE_SOCKETS */
    ping_raw_init();
#endif /* PING_USE_SOCKETS */
}

void ping_stop(void)
{
    if(!is_ping_ongoing) {
        PING_LOGI("Ping is stopped already!");
        return;
    }
    wl_send_sema(&ping_stop_sema);
    wl_wait_sema(&ping_wake_sema, portMAX_DELAY);
    wl_free_sema(&ping_stop_sema);
    wl_free_sema(&ping_wake_sema);
    PING_LOGI("Ping is stopped now");
}



#endif /* LWIP_RAW */

