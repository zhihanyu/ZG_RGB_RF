#ifndef AP_NETWORK_H
#define AP_NETWORK_H


#include "RingBuffer.h"
#include "wlan_uart_utils.h"
#include "at_net_utils.h"

#define MAX_RSP_SIZE 256

#define AP_MODE 0
#define STA_MODE 1

// Error message display
#define AT_NET_PROMPT_BYPASS_CMD_CHAR   ">"
#define AT_NET_PROMPT_CMD_NUM_EXCEED    "no tail\r\n"
#define AT_NET_PROMPT_RECV_UART         "\r\nRecv %d bytes\r\n"     // ReceiveDataLen

#define AT_NET_PROMPT_RECV_MSG_PASSTH   "%s"                        // ReceivedData
#define AT_NET_PROMPT_RECV_MSG_AMULT    "\r\n+IPD,%d,%d:%s\r\n"     // LinkID, ReceiveDataLen, ReceivedData
#define AT_NET_PROMPT_RECV_MSG_ASING    "\r\n+IPD,%d:%s\r\n"        // ReceiveDataLen, ReceivedData

#if CONFIG_NET_AT_PASSIVE_RECV
#define AT_NET_PROMPT_RECV_MSG_PMULT    "\r\n+IPD,%d,%d\r\n"        // LinkID, ReceiveDataLen
#define AT_NET_PROMPT_RECV_MSG_PSING    "\r\n+IPD,%d\r\n"           // ReceiveDataLen

#define AT_NET_PROMPT_READ_RECVMODE     "+CIPRECVMODE:%d\r\n"   // Passive receive mode config
#endif

#define AT_NET_PROMPT_RECV_DATA_LEN     "+CIPRECVLEN:%d,%d,%d,%d,%d\r\n"    // ReceiveDataLen
#define AT_NET_PROMPT_RECV_DATA_PRINT   "+CIPRECVDATA,%d:%s\r\n"            // ReceivedDataLenToPrint, ReceivedDataToPrint

#define AT_NET_PROMPT_CONNECT_DUP       "ALREADY CONNECTED\r\n"
#define AT_NET_PROMPT_CONNECT_FAIL      "%d, CONNECT FAIL\r\n"  // LinkID
#define AT_NET_PROMPT_CONNECT_OK_MULT   "%d,CONNECT\r\n"        // LinkID
#define AT_NET_PROMPT_CONNECT_OK_SING   "CONNECT\r\n"        // LinkID
#define AT_NET_PROMPT_LINK_ID_MULT      "MUX=%d\r\n"            // MultipleMode 
#define AT_NET_PROMPT_LINK_ID_RANGE     "ID ERROR\r\n"
#define AT_NET_PROMPT_LINK_TYPE         "Link type ERROR\r\n"
#define AT_NET_PROMPT_LINK_CLOSED_MULT  "%d, CLOSED\r\n"        // LinkID
#define AT_NET_PROMPT_LINK_CLOSED_SING  "CLOSED\r\n"            // LinkID
#define AT_NET_PROMPT_LINK_NOT_EXIST    "UNLINK\r\n"
#define AT_NET_PROMPT_LINK_NOT_VALID    "link is not valid\r\n"
#define AT_NET_PROMPT_LINK_IS_BUILDED   "link is builded\r\n"
#define AT_NET_PROMPT_ENTRY_ERROR       "ENTRY ERROR\r\n"
#define AT_NET_PROMPT_DNS_FAIL          "DNS Fail\r\n"
#define AT_NET_PROMPT_SEND_BUSY         "\r\nbusy s...\r\n"
#define AT_NET_PROMPT_SEND_OK           "\r\nSEND OK\r\n"
#define AT_NET_PROMPT_SEND_FAIL         "SEND FAIL\r\n"
#define AT_NET_PROMPT_SEND_CANCEL       "SEND Canceled\r\n"
#define AT_NET_PROMPT_SEND_MODE_ERR     "send mode error\r\n"
#define AT_NET_PROMPT_SENDBUF_SEGID     "%d,%d\r\n"             // SegmentIDCurrentBuffer, SegmentIDLastSendSuccessfully. 
#define AT_NET_PROMPT_SENDBUF_SING_OK   "\r\n%d,SEND OK\r\n"    // SegmentID
#define AT_NET_PROMPT_SENDBUF_SING_FAIL "%d,SEND FAIL\r\n"      // SegmentID
#define AT_NET_PROMPT_SENDBUF_MULT_OK   "\r\n%d,%d,SEND OK\r\n" // LinkID, SegmentID
#define AT_NET_PROMPT_SENDBUF_MULT_FAIL "%d,%d,SEND FAIL\r\n"   // LinkID, SegmentID
#define AT_NET_PROMPT_SENDBUF_STATUS    "%d,%d,%d,%d,%d\r\n"    // NextSegID, LastSendSegID, LastSuccSendSegID, RemainBuffSize, RemainQueueNum
#define AT_NET_PROMPT_SENDBUF_TOO_LONG  "too long\r\n"
#define AT_NET_PROMPT_SEGMENT_SING      "%d,%s\r\n"             // SegmentID, Status
#define AT_NET_PROMPT_SEGMENT_MULT      "%d,%d,%s\r\n"          // LinkID, SegmentID, Status


#define AT_NET_PROMPT_SSL_NOT_SUPPORT   "ssl not supported\r\n"
#define AT_NET_PROMPT_SSL_REACH_MAX     "ssl reach max\r\n"

#define AT_NET_PROMPT_MODE_PASSTH_ENABLED   "IPMODE=1\r\n"
#define AT_NET_PROMPT_MODE_PASSTH_MUST_0    "IPMODE must be 0\r\n"
#define AT_NET_PROMPT_MODE_MULT_MUST_0      "CIPMUX and CIPSERVER must be 0\r\n"
#define AT_NET_PROMPT_MODE_SERVER_NO_CHANGE "no change\r\n"

#define AT_NET_PROMPT_READ_STATUS_DEV   "STATUS:%d\r\n"         // DeviceStatus
#define AT_NET_PROMPT_READ_STATUS_ENTRY "+CIPSTATUS:%d,\"%s\",\"%s\",%d,%d,%d\r\n"  // LinkID,Type,RemoteIP,RemotePort,LocalPort,UdpMode
#define AT_NET_PROMPT_READ_SSLCCONF     "+CIPSSLCCONF:%d\r\n"   // SSL config
#define AT_NET_PROMPT_READ_MUX          "+CIPMUX:%d\r\n"        // Multiple mode config
#define AT_NET_PROMPT_READ_MODE         "+CIPMODE:%d\r\n"       // Passthrough mode config
#define AT_NET_PROMPT_READ_SERVERMAX    "+CIPSERVERMAXCONN:%d\r\n"  // Max server connection number

#define AT_NET_PROMPT_IFS_IP_STA        "+CIFSR:STAIP,\"%s\"\r\n"
#define AT_NET_PROMPT_IFS_IP_AP         "+CIFSR:APIP,\"%s\"\r\n"
#define AT_NET_PROMPT_IFS_MAC_STA       "+CIFSR:STAMAC,\"%02x:%02x:%02x:%02x:%02x:%02x\"\r\n"
#define AT_NET_PROMPT_IFS_MAC_AP        "+CIFSR:APMAC,\"%02x:%02x:%02x:%02x:%02x:%02x\"\r\n"

#define AT_NET_PROMPT_PING_TIMEOUT		"+timeout\r\n"
#define AT_NET_PROMPT_PING_DURATION		"+%d\r\n"

#define AT_NET_PROMPT_SEND_RES_TRUE     "TRUE"
#define AT_NET_PROMPT_SEND_RES_FALSE    "FALSE"

#define AT_TEST_SETTAG                  ','
#define AT_NET_STR_SWITCH_UART_MODE     "+++"

 
// Definations
#define AT_NET_MAX_CMD_SEND_SIZE        2048
#define AT_NET_MAX_CMD_RECV_SIZE        AT_NET_MAX_CMD_SEND_SIZE
#define AT_NET_MIN_TLS_SIZE             AT_NET_MAX_CMD_SEND_SIZE
#define AT_NET_MAX_TLS_SIZE             4096
#define AT_NET_MAX_TLS_LINK_NUM         1
#define AT_NET_MAX_CONNS_NUM            5
#define AT_NET_MAX_SEG_NUM              32
#define AT_NET_MAX_SEND_BUFF_QUEUE      8
#define AT_NET_MAX_PORT_VALUE           65535
#define AT_NET_MAX_KEEP_ALIVE           7200
#define AT_NET_MAX_SEGMENT_RECORDS      32
#define AT_NET_MIN_PASSIVE_CONNS_NUM    1
#define AT_NET_MAX_PASSIVE_CONNS_NUM    AT_NET_MAX_CONNS_NUM
#define AT_NET_DEF_PASSIVE_CONNS_NUM    AT_NET_MIN_PASSIVE_CONNS_NUM

#define AT_NET_SEGMENT_SEND_TIMEOUT     120000      // 2 minutes

#define AT_NET_INVALID_SOCKET           (-1)
#define AT_NET_INVALID_LINKID           (-1)
#define AT_NET_DEFAULT_SERVER_PORT      333
#define AT_NET_SEND_BLOCK_INTVL         100         // 100MS
#define AT_NET_SEND_BLOCK_RETRY_CNT     30
#define AT_NET_PASSIVE_CONN_TIMEOUT     (3*60*1000) // 180seconds, according to test

#define AT_NET_PING_PKT_SIZE            32
#define AT_NET_PING_PKT_CNT             1
#define AT_NET_PING_REPLY_RECV_MAX		1000

// Debug informations
#define AT_NET_TLS_DBG_ENABLED          1
#define AT_NET_TLS_DBG_LEVEL            2


#define AT_NET_LINK_ID_OUT_RANGE(id)            (((id) < 0) || (AT_NET_MAX_CONNS_NUM <= (id)))
#define AT_NET_PORT_OUT_RANGE(port)             (((port) <= 0) || (AT_NET_MAX_PORT_VALUE < (port)))
#define AT_NET_SEG_RECORD_IDX(id)               ((id) & 0x001F)
#define AT_NET_LINK_ID_GET_FAIL(val)            ((val) < AT_NET_GET_ARGS_OK_DEFAULT)
#define AT_NET_LINK_ID_ASSIGNED(val)            ((val) == AT_NET_GET_ARGS_OK_ASSIGNED)
#define AT_NET_LINK_ID_NOT_ASSIGNED(val)        ((val) == AT_NET_GET_ARGS_FAIL_ON_MULT)
#define AT_NET_LINK_ID_DEFULT(val)              ((val) == AT_NET_GET_ARGS_OK_DEFAULT)
#define AT_NET_BOOL_ARGUMENT_INVALID(val)       ((0 != (val)) && (1 != (val)))

#define MIN(a, b)               (((a) < (b)) ? (a) : (b))
#define MAX(a, b)               (((a) > (b)) ? (a) : (b))
#define ARG_BIT_IS_ON(arg, val)     ((arg) & (val))
#define ARG_BIT_SET_ON(arg, val)    ((arg) | (val))
#define ARG_BIT_SET_OFF(arg, val)   ((arg) ^ (val))

#define in_range(c, lo, up)     ((uint8_t)c >= lo && (uint8_t)c <= up)
#define isdigit(c)              in_range(c, '0', '9')










void at_network_init(void);
void at_network_deinit(void);






#endif
