#ifndef AT_NET_UTILS_H
#define AT_NET_UTILS_H

#include "at_common.h"
#include "lwip/netif.h"
#include "lwip/stats.h"

#define AT_IDLE_EVENT		  0
#define AT_CLOSE_CONNS_EVENT  1
#define AT_OPEN_CONNS_EVENT   2

#define AT_NET_OPEN(conns)      (g_at_net_conns_task |= BIT(conns->link_id))
#define AT_NET_CLOSE(conns)     (g_at_net_conns_task &= ~BIT(conns->link_id))
#define AT_NET_IS_OPEN(conns)   (g_at_net_conns_task | BIT(conns->link_id))

#define AT_NETWORK_TASK_DELAY    (200)   
// Enumerate connection status.
// 
typedef enum {
    AT_NET_DEV_STATUS_GET_IP        = 2,    // Associate to AP and get ip address
    AT_NET_DEV_STATUS_CONN_EST      = 3,    // One connection or more had been created
    AT_NET_DEV_STATUS_CONN_CLOSE    = 4,    // Connection closed
    AT_NET_DEV_STATUS_NOT_ASSOC     = 5,    // No associate to AP
    AT_NET_DEV_STATUS_BUTT
}at_net_conns_dev_status_e;

// Enumeration
// Enumerate udp transfer mode for connections
typedef enum {
    AT_NET_UDP_MODE_STATIC  = 0,
    AT_NET_UDP_MODE_ONCE    = 1,
    AT_NET_UDP_MODE_DYNC    = 2,
}at_net_udp_trans_mode_e;

// Enumerate protocol type for connections
    // "TCP", "UDP", "SSL"
typedef enum {
    AT_NET_CONNS_TYPE_TCP = 0,      // TCP connections
    AT_NET_CONNS_TYPE_UDP,          // UDP connections
    AT_NET_CONNS_TYPE_SSL,          // SSL connections
    AT_NET_CONNS_TYPE_BUTT
}at_net_conns_type_e;

// Enumerate connection mode
// ACTIVE, PASSIVE
typedef enum {
    AT_NET_CONNS_MODE_ACTIVE = 0,   // Active mode connection
    AT_NET_CONNS_MODE_PASSIVE,
    AT_NET_CONNS_MODE_BUTT
}at_net_conns_mode_e;

// Enumerate ssl config mode
// NONE, AUTH_SERVER, AUTH_CLIENT
typedef enum {
    AT_NET_TLS_AUTH_MODE_NONE   = 0,            // No auth
    AT_NET_TLS_AUTH_MODE_SERVER = BIT0,         // Auth client. Load client crt and private key
    AT_NET_TLS_AUTH_MODE_CLIENT = BIT1,         // Auth server. Load server crt
    AT_NET_TLS_AUTH_MODE_BOTH   = (BIT0|BIT1),  // Auth client and server. Load client crt, private key and server crt
    AT_NET_TLS_AUTH_MODE_BUTT,
}at_net_tls_auth_mode_e;

// Enumerate device mode for connections
// MultipleMode, ServerMode, PassthroughMode
typedef enum {
    AT_NET_CONNS_DEV_MODE_MULT  = 0,        // Multiple Mode.
    AT_NET_CONNS_DEV_MODE_SERVER,           // Server Mode.
    AT_NET_CONNS_DEV_MODE_PASSTH,           // Passthrough Mode.
#if CONFIG_NET_AT_PASSIVE_RECV
    AT_NET_CONNS_DEV_MODE_PASSIVE_RECV,     // Passive Receive Mode.
#endif
    AT_NET_CONNS_DEV_MODE_BUTT
}at_net_conns_dev_mode_e;

// Enuerate send command mode
// SEND, SENDEX, SENDBUF
typedef enum {
    AT_NET_SEND_CMD_MODE_NORMAL = 0,        // AT+CIPSEND command
    AT_NET_SEND_CMD_MODE_EXTERNAL,          // AT+CIPSENDEX command
    AT_NET_SEND_CMD_MODE_BUF,               // AT+CIPSENDBUF command
    AT_NET_SEND_CMD_MODE_BUTT
}at_net_send_cmd_mode_e;

// Enumerate send result
// SENDING, SENDOK, SENDFAIL
typedef enum {
    AT_NET_SEND_RESULT_FAIL     = 0,        // Send fail, connection will to be closed
    AT_NET_SEND_RESULT_RETRY,               // Sending, TCP/IP Stack re-transmission
    AT_NET_SEND_RESULT_SUCC,                // Send successfully
    AT_NET_SEND_RESULT_BUTT
}at_net_send_result_e;

// Enumerate argument type
// INVALID, DEFAULT, ASSIGNED
typedef enum {
    AT_NET_GET_ARGS_FAIL_INVALID    = 0,    // FAIL: Invalid argument, get fail
    AT_NET_GET_ARGS_FAIL_ON_MULT,           // FAIL: Not assign the link id in multiple mode
    AT_NET_GET_ARGS_OK_DEFAULT,             // OK:  Related argument not assigned, using the default value
    AT_NET_GET_ARGS_OK_ASSIGNED,            // OK:  Related argument assigned and valid
    AT_NET_GET_ARGS_BUTT
}at_net_args_result_e;

typedef enum {
    AT_NET_MSG_PRINT_AMULT = 0,
    AT_NET_MSG_PRINT_ASING,
#if CONFIG_NET_AT_PASSIVE_RECV
    AT_NET_MSG_PRINT_PMULT,
    AT_NET_MSG_PRINT_PSING,
#endif
    AT_NET_MSG_PRINT_PASSTH,
    AT_NET_MSG_PRINT_BUTT
}at_net_recv_msg_print_e;

// Enumerate task update event
//
typedef enum {
    AT_NET_TASK_CONN_CHANGE         = BIT(0),
    AT_NET_TASK_ADD_NEW_SEGMENT     = BIT(1),
    AT_NET_TASK_TCP_SERVER          = BIT(4),
    AT_NET_TASK_EXIT                = BIT(5),
}at_net_conns_task_event_e;

typedef enum {
    AT_NET_INTF_INIT = 0,
    AT_NET_INTF_UP,
    AT_NET_INTF_DOWN,
    AT_NET_INTF_DOWN_UP,
    AT_NET_INTF_BUTT
}at_net_intf_stat_e;

#if CONFIG_NET_AT_CMD_SSL
typedef struct at_net_tls_conf_
{
    char                    auth_mode;
    char                    rsv[3];
    uint16_t                size;
    uint16_t                remain_size;
}at_net_tls_conf_t;

typedef struct at_net_tls_crts_
{
    const char              *server_crt;    // CA certificate
    const char              *client_crt;    // Client certificate
    const char              *client_pk;     // Client private key
    uint32_t                server_crt_len;
    uint32_t                client_crt_len;
    uint32_t                client_pk_len;
}at_net_tls_crts_t;

typedef struct at_net_tls_context_
{
    mbedtls_ssl_context     ssl_ctx;        // mbedtls ssl context
    mbedtls_ssl_config      ssl_conf;       // SSL configuration
    mbedtls_x509_crt        cacert;         // CA certificate
    mbedtls_x509_crt        clicert;        // Client certificate
    mbedtls_pk_context      pkey;           // Client private key
}at_net_tls_context_t;
#endif

// For AT+CIPSENDBUF commands. Note the segment id for send buffer
typedef struct at_net_sendbuf_status_
{
    int     next_seg_id;            // Segment id for next buffer to be send
    int     last_send_seg;          // Segment id for last buffer be send.
    int     last_succ_seg;          // Segment id for last buffer be send successfully
    int     remain_size;            // Remained memory size for buffer
    int     queue_num;              // The queue number through which the buffer to be send
    _list   node;
}at_net_sendbuf_status_t;

// For AT+CIPSENBUF command's segment send result.
typedef struct at_net_seg_send_record_
{
    int     seg_id;             // Segment id
    int     status;             // Send status of segment. TRUE/FALSE: Succ/Fail
}at_net_seg_send_record_t;

// For AT+CIPSENDBUF command's segment
typedef struct at_net_seg_send_
{
    _list       list;
    int         seg_id;
    uint32_t    seq_num;
    uint32_t    ack_num;
    uint32_t    last_ack;
    uint32_t    start_time;
}at_net_seg_send_t;

//segment control 
//pend_nums ++



// For AT+CIPSEND commands.(AT+CIPSENDEX).
#if 1
typedef struct at_net_send_
{
    int  cmd_mode;       // Send command mode: "AT+CIPSEND, AT+CIPSENDEX, AT+CIPSENDBUF" 
    int                     len;            // Len of the data to send
    union {
        char                *host;          // Remote host string from at_cmd_hdl, will convert to remote_ip after connection
        uint32_t            addr;           // Remote ip address for connection
    }remote;                                // Remote address. Just for UDP connection 
#define remote_addr         remote.addr
#define remote_host         remote.host
    int                     remote_port;    // Remote port. Just for UDP connection
    void                    *conns;         // Related connection struct
}at_net_send_t;
#endif



// Local device mode in connections. For AT+CIPxxx command.
typedef struct  at_net_conns_mode_
{
    char        server_mode;        // TCP server mode. AT+CIPSERVER
    char        mult_mode;          // Multiple connections. AT+CIPMUX
    char        passth_mode;        // Pass-through mode. AT+CIPMODE
    char        passive_recv_mode;  // Receive data mode. AT+CIPRECVMODE
}at_net_conns_mode_t;

// For AT+CIPSERVER command.
typedef struct at_net_tcp_server_
{
    char        max_client;
    char        cnt_client;
    char        rsv[2];
    int         socket;
    int         timeout;
    int         port;
}at_net_tcp_server_t;

#if CONFIG_NET_AT_PASSIVE_RECV
// For AT+CIPRECVDATA commands
typedef struct at_net_passive_recv_
{
    mutex_t     semaphore;
    int         data_offset;
    int         data_len;
    char        buffer[AT_NET_MAX_RECV_SEND_BUFF_SIZE];
}at_net_passive_recv_t;
#endif


// Data types definitions
typedef struct at_net_conns_
{
    int                 socket;             // socket for connection
    int                 link_id;            // link id for connection
    int                 mode;               // Connection mode: 
                                            //      Active--(Created by CIPSTART command)
                                            //      Passive--(Accepted in TCP server mode)
    at_net_conns_type_e type;               // protocol type -- TCP/UDP/SSL
    union {
        const char      *host;              // Remote host string from at_cmd_hdl, will convert to remote_ip after connection
        uint32_t        addr;               // Remote ip address for connection
    }remote;
#define remote_addr     remote.addr
#define remote_host     remote.host
    int                 remote_port;        // Remote port for connection
    int                 local_port;         // Local port for connection
    union {
        int             udp_mode;           // Union. UDP mode--0/1/2
        int             tcp_keepalive;      // Union. TCP keep alive timer
    }proto;
    time_t              last_active_time;   // Last receive/send packet
#if CONFIG_NET_AT_CMD_SSL
    void                *tls;               // Pointer to TLS struct
    at_net_tls_crts_t   crt;                // Certificate struct
#endif
#if CONFIG_NET_AT_PASSIVE_RECV
    int                 passive_recv_full;
#endif
    struct netif        *intf;              // Connection related local interface
    at_net_intf_stat_e  if_status;
    at_net_send_t       send;
    _list               node;
}at_net_conns_t;

#if CONFIG_NET_AT_LIST_PKT_CNT
typedef struct {
    uint32  enable;
    uint32  at_wlan_rx;
    uint32  at_ether_rx;
    uint32  at_ip_rx;
    uint32  at_udp_rx;
    uint32  at_tcp_rx;
    uint32  at_task_rx;
}at_net_rx_stat_t;

typedef struct {
    uint32  enable;
    uint32  at_uart_rx;
    uint32  at_uart_buff;
    uint32  at_uart_tx;
    uint32  at_uart_overflow;
    uint32  at_task_tx;
    uint32  at_tcp_tx;
    uint32  at_udp_tx;
    uint32  at_ip_tx;
    uint32  at_ether_tx;
    uint32  at_wlan_tx;
}at_net_tx_stat_t;
#endif

//socket event
void                    at_net_sendbuf_status_init(at_net_sendbuf_status_t *status);
int                     at_net_conns_init(void);
int                     at_net_conns_deinit(void);
at_net_conns_t         *at_net_conns_malloc(void);
int                     at_net_conns_get_send_len(void *loguart, int *len);
int                     at_net_conns_get_send_mode(void *loguart, int *mode);
void                    at_net_conns_free(at_net_conns_t *conns);
int                     at_net_add_conn(at_net_conns_t *conns);
int                     at_net_del_conn(const int link_id);
at_net_conns_t         *at_net_get_conn(const int link_id);
int                     at_net_conns_check(at_net_conns_t *check_conns);
void                    at_net_set_conn_mode(const at_net_conns_dev_mode_e mode, const int in_mode_val);
int                     at_net_get_conn_mode(const at_net_conns_dev_mode_e mode, int *out_mode_val);
void                    at_net_set_dev_status(at_net_conns_dev_status_e status);
void                    at_net_get_dev_status(at_net_conns_dev_status_e *status);
at_net_tcp_server_t    *at_net_get_tcp_server(void);
at_net_sendbuf_status_t *at_net_get_sendbuf_status(const int link_id);
int                     at_net_get_segment_send_result(const int link_id, const int seg_id, int *out_result);
int                     at_net_segment_send_finished(const int link_id);
void                    at_net_task_event_send(const at_net_conns_task_event_e event);
int                     at_net_open(at_net_conns_t *conns);
int                     at_net_close(at_net_conns_t *conns);
int                     at_net_bind_server_socket(const int local_port, const int max_client, int *server_sock);
int                     at_net_send_data(const at_net_conns_t *conns, const char *in_data, const int in_len);
int                     at_net_open_connection(at_net_conns_t *conns);
#if CONFIG_NET_AT_PASSIVE_RECV
int                     at_net_get_passive_recv_data_len(const int link_id, int *out_data_len);
int                     at_net_fetch_passive_recv_data(const int link_id, const int recv_len);
#endif
int                     at_net_get_addr_from_host(const at_net_conns_t *conns, at_net_send_t *send);
void                    at_net_change_udp_remote(at_net_conns_t *conns);
void                    at_net_if_status_cb( struct netif *intf);

#if CONFIG_NET_AT_LIST_PKT_CNT
void                    at_net_rx_pkt_stat(uint32 enable);
void                    at_net_tx_pkt_stat(uint32 enable);
#endif

int                     at_net_ping(char *host);
#endif
