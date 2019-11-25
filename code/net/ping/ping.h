
#ifndef __PING_H__
#define __PING_H__


#include <stdint.h>
#include "lwip/err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PING_IP_ADDR_V4      0
#define PING_IP_ADDR_V6      1

typedef struct _ping_result
{
    uint32_t min_time;
    uint32_t max_time;
    uint32_t avg_time;
    uint32_t total_num;
    uint32_t lost_num;
    uint32_t recv_num;
} ping_result_t;

typedef struct _ping_static
{
    u32_t ping_time;
    u32_t ping_min_time;
    u32_t ping_max_time;
    u32_t ping_avg_time;
    u32_t ping_done;
    u32_t ping_lost_num;
    u32_t ping_recv_num;
    u32_t count;
    u32_t size;
    u16_t ping_seq_num;
    u8_t addr[16];
} ping_static_t;


#define PING_TASK_NAME      "ping_task"
/**
 * PING_USE_SOCKETS: Set to 1 to use sockets, otherwise the raw api is used
 */
#ifndef PING_USE_SOCKETS
#define PING_USE_SOCKETS    LWIP_SOCKET
#endif

/** ping receive timeout - in milliseconds */
#ifndef PING_RCV_TIMEO
#define PING_RCV_TIMEO 3000
#endif

typedef void (* ping_request_result_t)(ping_result_t *result);

//addr_type:PING_IP_ADDR_V4 or PING_IP_ADDR_V6, current only support PING_IP_ADDR_V4

void        ping_init(void *argv);
void        ping_stop(void);
uint32_t    get_ping_done(void);

#if PING_USE_SOCKETS
err_t ping_send(int s, ip_addr_t *addr, ping_static_t *p_ping_static);
err_t ping_recv(int s, ip_addr_t *addr, ping_static_t *p_ping_static);
#endif

#if !PING_USE_SOCKETS
void ping_send_now(void);
#endif /* !PING_USE_SOCKETS */


#ifdef __cplusplus
}
#endif

#endif /* __PING_H__ */

