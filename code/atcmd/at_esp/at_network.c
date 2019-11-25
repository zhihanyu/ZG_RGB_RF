/* Copyright Statement:
 *
 * (C) 2016-2018  SCI Inc. All rights reserved.
 */
#include "s907x.h"
 
#include "lwip/err.h"
#include "lwip/inet.h"
#include "lwip_conf.h"
#include "lwip/dns.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/tcp.h"
#include "lwip/err.h"

#include "at_network.h"
#include "at_test.h"
#include "sntp_time.h"

#if M_AT_ESP
#if CONFIG_NET_AT_CMD_SSL
#include "mbedtls/ssl.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/certs.h"
#include "mbedtls/platform.h"
#include "mbedtls/debug.h"
#include "mbedtls/ctr_drbg.h"
#endif
#include "FreeRTOS.h"
#include "task.h"

/*
AT+CIPSTATUS 查询网络连接信息
AT+CIPDOMAIN 域名解析功能
AT+CIPSTART 建立 TCP 连接，UDP 传输或者 SSL 连接
AT+CIPSSLSIZE 设置 SSL buffer大小
AT+CIPSSLCCONF 配置 ESP SSL client
AT+CIPSEND 发送数据
AT+CIPSENDEX 发送数据，达到设置长度，或者遇到字符 \0，则发送数据
AT+CIPSENDBUF 数据写入 TCP 发包缓存
AT+CIPBUFRESET 重置计数（TCP 发包缓存）
AT+CIPBUFSTATUS 查询 TCP 发包缓存的状态
AT+CIPCHECKSEQ 查询写入 TCP 发包缓存的某包是否成功发送
AT+CIPCLOSE 关闭 TCP/UDP/SSL 传输
AT+CIFSR 查询本地 IP 地址
AT+CIPMUX 设置多连接模式
AT+CIPSERVER 设置 TCP 服务器?
AT+CIPSERVERMAXCONN 设置服务器?允许建?立的最?大连接数。
AT+CIPMODE 设置透传模式
AT+SAVETRANSLINK 保存透传连接到 Flash
AT+CIPSTO 设置 s907x 作为 TCP 服务器?时的超时时间
AT+PING Ping 功能
AT+CIUPDATE 通过 Wi-Fi 升级软件
AT+CIPDINFO接收网络数据时，+IPD 是否提示对端 IP 和端?口
AT+CIPRECVMODE 设置 TCP 接收模式
AT+CIPRECVDATA TCP 被动接收模式下，读取缓存的 TCP 数据
AT+CIPRECVLEN TCP 被动接收模式下，查询缓存 TCP 数据的长度
AT+CIPSNTPCFG 设置时域和 SNTP 服务器?
AT+CIPSNTPTIME 查询 SNTP 时间
AT+CIPDNS_CUR 自定义 DNS 服务器?，设置不不保存到 flash
AT+CIPDNS_DEF 自定义 DNS 服务器?，设置保存到 flash
*/

/*example 
if(cmd->setsize) { //write mode
    set = cmd->set;
} else {          //read mode

}

*/
// Global Datas Field

// Connections TASK
#if CONFIG_NET_AT_LIST_PKT_CNT
typedef enum {
    AT_NET_DBG_TYPE_TOP     = 0x00,
    AT_NET_LIST_RX_STAT     = 0x01,
    AT_NET_LIST_TX_STAT     = 0x02,
    AT_NET_GET_TASK_INFO    = 0x03,
    AT_NET_DBG_TYPE_BUTT
}at_net_dbg_type_enum;
#endif

// CONNS_TYPE
static char *ga_pc_at_net_conns_type[AT_NET_CONNS_TYPE_BUTT] = {"TCP", "UDP", "SSL"};
static rtc_hdl_t rtc_hdl;
// CIP_CONNS: Array of pointers to CIP_CONNS_STRUCT, total len is AT_NET_MAX_CONNS_NUM


// Segments for AT+CIPSENDBUF command.
// According to the test result of the ESP8266, this segment informations won't be deleted



// Connections TASK


static int at_net_atoi(const char *data)
{
    int digit = -1;
    int idx = 0, len = 0;

    len = strlen(data);
    if (0 == len)
    {
        AT_DBG_ERR("Data len is 0.");
        return digit;
    }

    for (idx = 0; idx < len; idx++)
    {
        if (0 == isdigit(data[idx]))
        {
            AT_DBG_ERR("Not digital data.");
            return digit;
        }
    }

    digit =  atoi(data);
    return digit;
}

static at_net_args_result_e at_net_get_args_linkid(char ** const argv, const int idx, int *out_link_id)
{
    int link_id = -1;
    int mult_mode = 0; 
    at_net_args_result_e ret = AT_NET_GET_ARGS_FAIL_INVALID;

    at_net_get_conn_mode(AT_NET_CONNS_DEV_MODE_MULT, &mult_mode);
    if (1 == mult_mode)
    {
        if (NULL == argv[idx])
        {
            ret = AT_NET_GET_ARGS_FAIL_ON_MULT;
            return ret;
        }

        // Get LinkID if CIPMUX is 1.
        link_id = at_net_atoi(argv[idx]);
        // Check the range, if exceed, AT_NET_PROMPT_LINK_ID_RANGE and return
        if (AT_NET_LINK_ID_OUT_RANGE(link_id))
        {
            at_rsp(AT_NET_PROMPT_LINK_ID_RANGE);
            return ret;
        }
        ret = AT_NET_GET_ARGS_OK_ASSIGNED;
    }
    else if (0 == mult_mode)
    {
        link_id = 0;
        ret = AT_NET_GET_ARGS_OK_DEFAULT;
    }
    else
    {
        AT_DBG_ERR("Get connection mode fail.");
    }

    *out_link_id = link_id;
    return ret;
}


// This routine to print the status information for SENDBUF command
static void at_net_print_sendbuf_result(const at_net_sendbuf_status_t *status)
{
    int cur_seg_id = status->next_seg_id - 1;
    int last_succ_seg = status->last_succ_seg;

    at_rsp(AT_NET_PROMPT_SENDBUF_SEGID, cur_seg_id, last_succ_seg);

    return;
}




// This routine try to get the input arguments for AT+CIPSTATUS command. It will get the arguments
// sequencely and check the value accoring to the command's definition. When any error occurs
// it will prompt the related error message and return fail, or it will fill the send data struct
// for the caller and return success.
static int at_net_get_args_cipstatus(const at_cmd_t *cmd)
{
    // Set the default status to "fail"
    int ret = HAL_ERROR;

    if (AT_MODE_ACT != cmd->mode)
    {
        AT_DBG_ERR("Invalid command mode %d.", cmd->mode);
        return ret;
    }

    ret = HAL_OK;
    return ret;
}


// This routine try to get the arguments for AT+CIPSTART command. It will get the arguments
// sequencely and check the value accoring to the command's definition. When any error occur
// it will prompt the error message and return fail, or it will fill the connection data struct
// and return success.
static int at_net_get_args_cipstart(const at_cmd_t *cmd, at_net_conns_t *conns)
{
    int ret = HAL_ERROR;
    int cnt = 0;
    int mult_mode; 
    int argc = 0, value = 0;
    at_net_conns_type_e type = AT_NET_CONNS_TYPE_TCP;
    char *argv[AT_SET_MAX_ARGC] = {0};

    // Set the default status to "fail"
    // Parse arguments
    argc = at_parse_set(cmd->set, argv, AT_TEST_SETTAG); 
    // Minimum number for AT+CIPSTART: 3--TCP(ConnectionType + RemoteIP + RemotePort)
    // Maximum number for AT+CIPSTART: 6--UDP(LinkID + ConnectionType + RemoteIP + RemotePort + LocalPort + UDPMode)
    if((argc < 3) || (6 < argc)) {
        at_rsp(AT_NET_PROMPT_CMD_NUM_EXCEED);
        AT_DBG_ERR("at_parse_set fail, argc[%d] out range[3, 6].", argc);
        return ret;
    }

    /* Get input args sequencely according to the AT+CIPSTART. */
    /* Step1: LinkID  */
    cnt = 0;
    // If CIPMUX equal 1, Get LinkID. 
    at_net_get_conn_mode(AT_NET_CONNS_DEV_MODE_MULT, &mult_mode);
    if (1 == mult_mode)
    {
        if(argc == 3) {
            return ret;
        }
        /* get the interger LinkID*/
        value = at_net_atoi(argv[cnt]);    

        // if link id exceed the range, prompt "ID ERROR" and return
        if (AT_NET_LINK_ID_OUT_RANGE(value))
        {
            at_rsp(AT_NET_PROMPT_LINK_ID_RANGE);
            return ret;
        }
        cnt++;
    } 
    // note the link id
    conns->link_id = value;

    /* Step2: Connection type*/
    ret = at_parse_param_str(&argv[cnt]);
    if (HAL_OK != ret)
    {
        AT_DBG_ERR("Invalid string parameter[%s]", argv[cnt]);
        return ret;
    }
    ret = HAL_ERROR;
    for (type = AT_NET_CONNS_TYPE_TCP; type < AT_NET_CONNS_TYPE_BUTT; type++)
    {
        /* get the string connection type */
        /* compare type name with CONNS_TYPE{"TCP", "UDP", "SSL"} */
        AT_DBG_MSG("Remove. argv[%d]: %s, ga_pc_at_net_conns_type[%d]: %s.",
                cnt, argv[cnt], type, ga_pc_at_net_conns_type[type]);
        if (strcmp(argv[cnt], ga_pc_at_net_conns_type[type]) == 0)
        {
            conns->type = type;
            break;
        }
    }
    // if invalid connection type, AT_NET_PROMPT_LINK_TYPE and return 
    // note the connection type
    if (AT_NET_CONNS_TYPE_BUTT <= type)
    {
        at_rsp(AT_NET_PROMPT_LINK_TYPE);
        return ret;
    }
    cnt++;

    /* Step3: Remote ip */
    // get the remote ip
    // if invalid ip_addr(no need check domain name), return
    // note the connection type
    if (NULL == argv[cnt])
    {
        AT_DBG_ERR("Remote ip not assigned.");
        return ret;
    }
    ret = at_parse_param_str(&argv[cnt]);
    if (HAL_OK != ret)
    {
        AT_DBG_ERR("Invalid string parameter[%s]", argv[cnt]);
        return ret;
    }
    ret = HAL_ERROR;
    conns->remote_host = argv[cnt];

    cnt++;

    /* Step4: Remote port */
    if (NULL == argv[cnt])
    {
        AT_DBG_ERR("Remote port not assigned.");
        return ret;
    }
    // get remote port
    value = at_net_atoi(argv[cnt]);
    // if port value exceed 65535, return
    if (AT_NET_PORT_OUT_RANGE(value))
    {
        AT_DBG_ERR("Remote Port[%d] out of range[0, %d].", value, AT_NET_MAX_PORT_VALUE);
        return ret;
    }
    // note remote port
    conns->remote_port = value;
    cnt++;

    /* Step5: TCP: KeepAliveTimer. UDP: LocalPort+UDPMode */
    /* Check for connection type */
    /* For TCP */
    // if tcp connection
    if (AT_NET_CONNS_TYPE_TCP == conns->type)
    {
        // get keep alive timer 
        if (NULL == argv[cnt])
        {
            AT_DBG_MSG("Remove. No keep alive timer for TCP connection.");
            ret = HAL_OK;
            return ret;
        }
        // if alive timer exceed the range, disaplay "" and return
        value = at_net_atoi(argv[cnt]);
        if (value < 0 || AT_NET_MAX_KEEP_ALIVE < value)
        {
            AT_DBG_ERR("TCP keep alive timer[%d] out of range[0, %d].", value, AT_NET_MAX_KEEP_ALIVE);
            return ret;
        }
        // note the keep alive timer
        conns->proto.tcp_keepalive = value;
        cnt++;
    }
    /* For UDP */
    // else udp connection
    else if (AT_NET_CONNS_TYPE_UDP == conns->type)
    {
        /* UDP: Local port */
        if (NULL == argv[cnt])
        {
            AT_DBG_MSG("Remove. No local port for UDP connection.");
            ret = HAL_OK;
            return ret;
        }
        // get udp local port.
        value = at_net_atoi(argv[cnt]);
        // if port value exceed 65535, return
        if (AT_NET_PORT_OUT_RANGE(value))
        {
            AT_DBG_ERR("Local Port[%d] out of range[0, %d].", value, AT_NET_MAX_PORT_VALUE);
            return ret;
        }
        // note local port
        conns->local_port = value;
        cnt++;

        /* UDP: transfer mode */
        if (NULL == argv[cnt])
        {
            AT_DBG_MSG("Remove. No transfer mode for UDP connection.");
            ret = HAL_OK;
            return ret;
        }
        // get udp transfer mode
        value = at_net_atoi(argv[cnt]);
        // if mode value exceed [], prompt "" and return
        if (value < AT_NET_UDP_MODE_STATIC || AT_NET_UDP_MODE_DYNC < value)
        {
            AT_DBG_ERR("UDP transfer mode[%d] out of range [%d, %d].", 
                    value, AT_NET_UDP_MODE_STATIC, AT_NET_UDP_MODE_DYNC);
            return ret;
        }
        // note transfer mode    
        conns->proto.udp_mode = value;
        cnt++;
    }

    if (NULL != argv[cnt])
    {
        AT_DBG_ERR("Too many arguments assigned for CIPSTART.");
        AT_DBG_MSG("Remove. Arguments num[%d], cnt[%d], argv[%d]: %s.",
                argc, cnt, cnt, argv[cnt]);
        return ret;
    }

    ret = HAL_OK;
    // return status information success
    return ret;
}


// This routine try to get the input arguments for AT+CIPSSLSIZE command. It will get the arguments
// sequencely and check the value accoring to the command's definition. When any error occurs
// it will prompt the related error message and return fail, or it will fill the send data struct
// for the caller and return success.
static int at_net_get_args_cipsslsize(const at_cmd_t *cmd, int *size)
{
    // Set the default status to "fail"
    int argc = 0;
    int ret = HAL_ERROR;
    int value = 0;
    char *argv[AT_SET_MAX_ARGC] = {0};

    // Parse arguments
    argc = at_parse_set(cmd->set, argv, AT_TEST_SETTAG);
    if (1 != argc)
    {
        AT_DBG_ERR("Arguments number[%d] assigned for cipstatus command.", argc);
        return ret;
    }

    value = at_net_atoi(argv[0]);
    if (value < AT_NET_MIN_TLS_SIZE || AT_NET_MAX_TLS_SIZE < value)
    {
        AT_DBG_ERR("SSL size %d out of range[%d, %d].",
                value, AT_NET_MIN_TLS_SIZE, AT_NET_MAX_TLS_SIZE);
        return ret;
    }

    *size = value;

    ret = HAL_OK;
    return ret;
}


// This routine try to get the input arguments for AT+CIPSSLCCONF command. It will get the arguments
// sequencely and check the value accoring to the command's definition. When any error occurs
// it will prompt the related error message and return fail, or it will fill the send data struct
// for the caller and return success.
static int at_net_get_args_cipsslcconf(const at_cmd_t *cmd, int *auth_mode, int *read_cmd)
{
    // Set the default status to "fail"
    int argc = 0;
    int ret = HAL_ERROR;
    int value = 0;
    char *argv[AT_SET_MAX_ARGC] = {0};

    *read_cmd = HAL_ERROR;
    if (AT_MODE_R == cmd->mode)
    {
        AT_DBG_MSG("Remove. AT+CIPSSLCCONF? command to get current SSL config.");
        *read_cmd = HAL_OK;
        ret = HAL_OK;
        return ret;
    }

    // Parse arguments
    argc = at_parse_set(cmd->set, argv, AT_TEST_SETTAG);
    if (1 != argc)
    {
        AT_DBG_ERR("Arguments assigned for cipstatus command.");
        return ret;
    }

    value = at_net_atoi(argv[0]);
    // Valid auth mode:
    //  0 - No auth
    //  BIT0 - Auth client
    //  BIT1 - Auth server
    //  BIT0|BIT1 - Auth client and server
    if (value < AT_NET_TLS_AUTH_MODE_NONE || AT_NET_TLS_AUTH_MODE_BOTH < value)
    {
        AT_DBG_ERR("SSL config mode %d out of range[%d, %d].",
                value, AT_NET_TLS_AUTH_MODE_NONE, AT_NET_TLS_AUTH_MODE_BOTH);
        return ret;
    }

    *auth_mode = value;

    ret = HAL_OK;
    return ret;
}


static int at_net_get_args_cipsendxx_comm(const int argc, char ** const argv, at_net_send_t *send)
{
    int ret = HAL_ERROR;
    int cnt = 0;
    int value = 0;
    at_net_conns_t *conns = NULL;

    // Minimum number for AT+CIPSEND: 1--TCP(Length)
    // Maximum number for AT+CIPSEND: 4--UDP(LinkID + Length + RemoteIP + RemotePort)
    if((argc < 1) || (4 < argc)) {
        at_rsp(AT_NET_PROMPT_CMD_NUM_EXCEED);
        AT_DBG_ERR("at_parse_set fail, argc[%d] out range[1, 4].", argc);
        return ret;
    }

    // Get the input arguments sequencely and parse
    cnt = 0;
    /* Step1: LinkID */
    ret = at_net_get_args_linkid(argv, cnt, &value);
    if (AT_NET_LINK_ID_GET_FAIL(ret))
    {
        ret = HAL_ERROR;
        return ret;
    }
    else if (AT_NET_LINK_ID_ASSIGNED(ret))
    {
        cnt++;
    }
    // Note the LinkID
    conns = at_net_get_conn(value);
    if (NULL == conns)
    {
        AT_DBG_ERR("No related connection to link id[%d].", value);
        at_rsp(AT_NET_PROMPT_LINK_NOT_VALID);
        ret = HAL_ERROR;
        return ret;
    }
    send->conns = (void *)conns;

    /* Step2: Length */
    if (NULL == argv[cnt])
    {
        at_rsp(AT_NET_PROMPT_CMD_NUM_EXCEED);
        AT_DBG_ERR("Length not assigned.");
        return ret;
    }
    // Get send buffer size.
    value = at_net_atoi(argv[cnt]);
    // Check the range, if exceed AT_NET_MAX_CMD_SEND_BLOCK, AT_NET_PROMPT_SENDBUF_TOO_LONG and return
    if (value <= 0 || AT_NET_MAX_CMD_RECV_SIZE < value)
    {
        at_rsp(AT_NET_PROMPT_SENDBUF_TOO_LONG);
        return ret;
    }
    // Note the send buffer size
    send->len = value;
    cnt++;

    if (NULL == argv[cnt])
    {
        AT_DBG_MSG("Remove. No remote ip assigned.");
        ret = HAL_OK;
        return ret;
    }

    if (AT_NET_CONNS_TYPE_TCP == conns->type)
    {
        at_rsp(AT_NET_PROMPT_CMD_NUM_EXCEED);
        return ret;
    }

    // Get RemoteIP.
    // Check the ip_addr, if invalid ip address, return
    // Note RemoteIP
    ret = at_parse_param_str(&argv[cnt]);
    if (HAL_OK != ret)
    {
        AT_DBG_ERR("Invalid string parameter[%s]", argv[cnt]);
        return ret;
    }
    ret = HAL_ERROR;
    send->remote_host = argv[cnt];
    cnt++;

    if (NULL == argv[cnt])
    {
        AT_DBG_MSG("Remove. No remote port assigned.");
        ret = HAL_OK;
        return ret;
    }
    // Get RemotePort.
    value = at_net_atoi(argv[cnt]);
    // Check the port range, if exceed AT_NET_MAX_PORT_VALUE, return 
    if (AT_NET_PORT_OUT_RANGE(value))
    {
        AT_DBG_ERR("Port[%d] out of range.");
        return ret;
    }
    // Note RemotePort
    send->remote_port = value;
    cnt++;

    if (NULL != argv[cnt])
    {
        AT_DBG_ERR("Too many arguments assigned for CIPSENDxx.");
        AT_DBG_MSG("Remove. Arguments num[%d], cnt[%d], argv[%d]: %s.",
                argc, cnt, cnt, argv[cnt]);
        return ret;
    }

    // Return success
    ret = HAL_OK;
    return ret;

}


// This routine try to get the input arguments for AT+CIPSEND command. It will get the arguments
// sequencely and check the value accoring to the command's definition. When any error occurs
// it will prompt the related error message and return fail, or it will fill the send data struct
// for the caller and return success.
static int at_net_get_args_cipsend(const at_cmd_t *cmd, at_net_send_t *send)
{
    // Set the default status to "fail"
    int argc = 0;
    int ret = HAL_ERROR;
    char *argv[AT_SET_MAX_ARGC] = {0};
    int passth_mode = 0; 
    at_net_conns_t *conns = NULL;

    if(!cmd || !send) {
        return HAL_ERROR;
    }
    // Parse arguments
    argc = at_parse_set(cmd->set, argv, AT_TEST_SETTAG); 
    /* Check for passthrough mode. */
    at_net_get_conn_mode(AT_NET_CONNS_DEV_MODE_PASSTH, &passth_mode);
    if (1 == passth_mode)
    {
        // If CIPMODE equal 1, and any arguments is assigned, AT_NET_PROMPT_MODE_PASSTH_ENABLED and return
        if (0 != argc)
        {
            AT_DBG_ERR("Passthrough mode enabled, no args for AT+CIPSEND command.");
            at_rsp(AT_NET_PROMPT_MODE_PASSTH_ENABLED);
            return ret;
        }
        conns = at_net_get_conn(0);
        if (NULL == conns)
        {
            AT_DBG_ERR("No connection created until now.");
            return ret;
        }
        send->conns = (void *)conns;
        send->len = AT_NET_MAX_RECV_SEND_BUFF_SIZE;
        ret = HAL_OK;
        return ret;
    }
    
    ret = at_net_get_args_cipsendxx_comm(argc, argv, send);
    send->cmd_mode = AT_NET_SEND_CMD_MODE_NORMAL;
    return ret;
}


// This routine try to get the input arguments for AT+CIPSENDEX command. It will get the arguments
// sequencely and check the value accoring to the command's definition. When any error occurs
// it will prompt the related error message and return fail, or it will fill the send data struct
// for the caller and return success.
static int at_net_get_args_cipsendex(const at_cmd_t *cmd, at_net_send_t *send)
{
    // Set the default status to "fail"
    int argc = 0;
    int ret = HAL_ERROR;
    char *argv[AT_SET_MAX_ARGC] = {0};
    
    if(!cmd || !send) {
        return HAL_ERROR;
    }
    // Parse arguments
    argc = at_parse_set(cmd->set, argv, AT_TEST_SETTAG); 
    ret = at_net_get_args_cipsendxx_comm(argc, argv, send);
    send->cmd_mode = AT_NET_SEND_CMD_MODE_EXTERNAL;

    return ret;
}

// This routine try to get the input arguments for AT+CIPSENDBUF command. It will get the arguments
// sequencely and check the value accoring to the command's definition. When any error occurs
// it will prompt the related error message and return fail, or it will fill the send data struct
// for the caller and return success.
static int at_net_get_args_cipsendbuf(const at_cmd_t *cmd, at_net_send_t *send)
{
    // Set the default status to "fail"
    int argc = 0;
    int ret = HAL_ERROR;
    char *argv[AT_SET_MAX_ARGC] = {0};
    
    if(!cmd || !send) {
        return HAL_ERROR;
    }            
    // Parse arguments
    argc = at_parse_set(cmd->set, argv, AT_TEST_SETTAG); 
    ret = at_net_get_args_cipsendxx_comm(argc, argv, send);
    send->cmd_mode = AT_NET_SEND_CMD_MODE_BUF;
    
    return ret;
}


// This routine try to get the input arguments for AT+CIPBUFRESET command. It will get the arguments
// sequencely and check the value accoring to the command's definition. When any error occurs
// it will prompt the related error message and return fail, or it will fill the send data struct
// for the caller and return success.
static int at_net_get_args_cipbufreset(const at_cmd_t *cmd, int *out_link_id)
{
    // Set the default status to "fail"
    int argc = 0;
    int ret = HAL_ERROR;
    int link_id = 0;
    char *argv[AT_SET_MAX_ARGC] = {0};
    at_net_args_result_e arg_type = AT_NET_GET_ARGS_FAIL_INVALID;

    // Parse arguments
    argc = at_parse_set(cmd->set, argv, AT_TEST_SETTAG);

    arg_type = at_net_get_args_linkid(argv, 0, &link_id);
    if (AT_NET_LINK_ID_GET_FAIL(arg_type))
    {
        return ret;
    }
    // MultipleMode disabled, but assigned the link id
    else if (AT_NET_LINK_ID_DEFULT(arg_type))
    {
        if((NULL != argv[0]) || (0 < argc))
        {
            AT_DBG_ERR("MultipleMode disabled, but the link id is assigned.");
            return ret;
        }
    }
    else if (AT_NET_LINK_ID_ASSIGNED(arg_type))
    {
        if (1 < argc)
        {
            AT_DBG_ERR("Too many arguments assigend.");
            return ret;
        }
    }

    *out_link_id = link_id;

    ret = HAL_OK;
    return ret;
}


// This routine try to get the input arguments for AT+CIPBUFSTATUS command. It will get the arguments
// sequencely and check the value accoring to the command's definition. When any error occurs
// it will prompt the related error message and return fail, or it will fill the send data struct
// for the caller and return success.
static int at_net_get_args_cipbufstatus(const at_cmd_t *cmd, int *out_link_id)
{
    // Set the default status to "fail"
    int argc = 0;
    int ret = HAL_ERROR;
    int link_id = 0;
    char *argv[AT_SET_MAX_ARGC] = {0};
    at_net_args_result_e arg_type = AT_NET_GET_ARGS_FAIL_INVALID;

    // Parse arguments
    argc = at_parse_set(cmd->set, argv, AT_TEST_SETTAG);

    arg_type = at_net_get_args_linkid(argv, 0, &link_id);
    if (AT_NET_LINK_ID_GET_FAIL(arg_type))
    {
        return ret;
    }
    // MultipleMode disabled, but assigned the link id
    else if (AT_NET_LINK_ID_DEFULT(arg_type))
    {
        if ((NULL != argv[0]) || (0 < argc))
        {
            AT_DBG_ERR("MultipleMode disabled, but the link id is assigned.");
            return ret;
        }
    }
    else if (AT_NET_LINK_ID_ASSIGNED(arg_type))
    {
        if (1 < argc)
        {
            AT_DBG_ERR("Too many arguments assigend.");
            return ret;
        }
    }

    *out_link_id = link_id;

    ret = HAL_OK;
    return ret;
}


// This routine try to get the input arguments for AT+CIPCHECKSEQ command. It will get the arguments
// sequencely and check the value accoring to the command's definition. When any error occurs
// it will prompt the related error message and return fail, or it will fill the send data struct
// for the caller and return success.
static int at_net_get_args_cipcheckseq(const at_cmd_t *cmd, int *link_id, int *seg_id)
{
    // Set the default status to "fail"
    int ret = HAL_ERROR;
    int cnt = 0, value = 0;
    char *argv[AT_SET_MAX_ARGC] = {0};
    at_net_args_result_e arg_type = AT_NET_GET_ARGS_FAIL_INVALID;

    // Parse arguments
    at_parse_set(cmd->set, argv, AT_TEST_SETTAG);

    arg_type = at_net_get_args_linkid(argv, 0, &value);
    if (AT_NET_LINK_ID_GET_FAIL(arg_type))
    {
        return ret;
    }
    else if (AT_NET_LINK_ID_ASSIGNED(arg_type))
    {
        cnt++;
    }
    *link_id = value;
    // No care whether the related connection exist or not, just use the segment struct.

    if (NULL == argv[cnt])
    {
        AT_DBG_ERR("No segment ID assigned.");
        return ret;
    }
    *seg_id = at_net_atoi(argv[cnt]);
    cnt++;

    // MultipleMode disabled, but assigned the link id
    if (NULL != argv[cnt])
    {
        AT_DBG_ERR("MultipleMode disabled, but the link id is assigned.");
        AT_DBG_MSG("Remove. Arguments num[], cnt[%d], argv[%d]: %s.",
                cnt, cnt, argv[cnt]);
        return ret;
    }

    ret = HAL_OK;
    return ret;
}


// This routine try to get the input arguments for AT+CIPCLOSE command. It will get the arguments
// sequencely and check the value accoring to the command's definition. When any error occurs
// it will prompt the related error message and return fail, or it will fill the send data struct
// for the caller and return success.
static int at_net_get_args_cipclose(const at_cmd_t *cmd, int *link_id)
{
    // Set the default status to "fail"
    int ret = HAL_ERROR;
    int value = 0;
    at_net_conns_t *conns;
    char *argv[AT_SET_MAX_ARGC] = {0};
    at_net_args_result_e arg_type = AT_NET_GET_ARGS_FAIL_INVALID;

    // Parse arguments
    at_parse_set(cmd->set, argv, AT_TEST_SETTAG);

    arg_type = at_net_get_args_linkid(argv, 0, &value);
    if (AT_NET_LINK_ID_GET_FAIL(arg_type))
    {
        if (AT_NET_LINK_ID_NOT_ASSIGNED(arg_type))
        {
            at_rsp(AT_NET_PROMPT_LINK_ID_MULT, 1);
        }
        return ret;
    }
    else if (AT_NET_LINK_ID_DEFULT(arg_type))
    {
        if (NULL != argv[0])
        {
            AT_DBG_ERR("MultipleMode disabled, but the link id is assigned.");
            at_rsp(AT_NET_PROMPT_LINK_ID_MULT, 0);
            return ret;
        }
    }

    conns = at_net_get_conn(value);
    if (NULL == conns)
    {
        at_rsp(AT_NET_PROMPT_LINK_NOT_EXIST);
        return ret;
    }

    *link_id = value;
    ret = HAL_OK;
    return ret;
}


// This routine try to get the input arguments for AT+CIPMUX command. It will get the arguments
// sequencely and check the value accoring to the command's definition. When any error occurs
// it will prompt the related error message and return fail, or it will fill the send data struct
// for the caller and return success.
static int at_net_get_args_cipmux(const at_cmd_t *cmd, int *mult_mode, int *read_cmd)
{
    // Set the default status to "fail"
    int argc = 0;
    int ret = HAL_ERROR;
    int value = 0;
    char *argv[AT_SET_MAX_ARGC] = {0};

    *read_cmd = HAL_ERROR;
    if (AT_MODE_R == cmd->mode)
    {
        *read_cmd = HAL_OK;
        ret = HAL_OK;
        return ret;
    }

    // Parse arguments
    argc = at_parse_set(cmd->set, argv, AT_TEST_SETTAG);
    if (1 != argc)
    {
        AT_DBG_ERR("Error. arguments num[%d] assigned for cipmux command.", argc);
        return ret;
    }

    value = at_net_atoi(argv[0]);
    if (AT_NET_BOOL_ARGUMENT_INVALID(value))
    {
        AT_DBG_ERR("Invalid multiple mode %d.", value);
        return ret;
    }

    *mult_mode = value;
    ret = HAL_OK;
    return ret;
}


// This routine try to get the input arguments for AT+CIPSERVER command. It will get the arguments
// sequencely and check the value accoring to the command's definition. When any error occurs
// it will prompt the related error message and return fail, or it will fill the send data struct
// for the caller and return success.
static int at_net_get_args_cipserver(const at_cmd_t *cmd, int *server_mode, int *local_port)
{
    // Set the default status to "fail"
    int ret = HAL_ERROR;
    int cnt = 0;
    int argc = 0, value = 0;
    char *argv[AT_SET_MAX_ARGC] = {0};

    // Parse arguments
    argc = at_parse_set(cmd->set, argv, AT_TEST_SETTAG);
    if ((0 == argc) || (NULL == argv[cnt]))
    {
        AT_DBG_ERR("No arguments assigned for cipserver command.");
        return ret;
    }

    value = at_net_atoi(argv[cnt]);
    if (AT_NET_BOOL_ARGUMENT_INVALID(value))
    {
        AT_DBG_ERR("Invalid server mode %d.", value);
        return ret;
    }
    *server_mode = value;
    cnt++;

    value = AT_NET_DEFAULT_SERVER_PORT;
    if (NULL != argv[cnt])
    {
        value = at_net_atoi(argv[cnt]);
        if (AT_NET_PORT_OUT_RANGE(value))
        {
            AT_DBG_ERR("Assigned local server port %d out of range.");
            return ret;
        }
        cnt++;
    }
    *local_port = value;

    if (NULL != argv[cnt])
    {
        AT_DBG_ERR("Too many arguments assigned for CIPSERVER.");
        AT_DBG_MSG("Remove. Arguments num[%d], cnt[%d], argv[%d]: %s.",
                argc, cnt, cnt, argv[cnt]);
        return ret;
    }

    ret = HAL_OK;
    return ret;
}


// This routine try to get the input arguments for AT+CIPSERVERMAXCONN command. It will get the arguments
// sequencely and check the value accoring to the command's definition. When any error occurs
// it will prompt the related error message and return fail, or it will fill the send data struct
// for the caller and return success.
static int at_net_get_args_cipservermaxconn(const at_cmd_t *cmd, int *max_client, int *read_cmd)
{
    // Set the default status to "fail"
    int ret = HAL_ERROR;
    int cnt = 0;
    int argc = 0, value = 0;
    char *argv[AT_SET_MAX_ARGC] = {0};

    *read_cmd = HAL_ERROR;
    if (AT_MODE_R == cmd->mode)
    {
        AT_DBG_MSG("Remove. Read command for CIPSERVERMAXCONN.");
        *read_cmd = HAL_OK;
        ret = HAL_OK;
        return ret;
    }

    AT_DBG_MSG("Remove. CIPSERVERMAXCONN command mode %d, cmd len %d.",
            cmd->mode, cmd->setsize);
    // Parse arguments
    argc = at_parse_set(cmd->set, argv, AT_TEST_SETTAG);
    if ((0 == argc) || (NULL == argv[cnt]))
    {
        AT_DBG_ERR("No arguments assigned for cipservermaxconn command.");
        return ret;
    }

    value = at_net_atoi(argv[cnt]);
    
    if ((value < AT_NET_MIN_PASSIVE_CONNS_NUM) || (AT_NET_MAX_PASSIVE_CONNS_NUM < value))
    {
        AT_DBG_ERR("Invalid server max connection number %d.", value);
        return ret;
    }
    cnt++;

    if (NULL != argv[cnt])
    {
        AT_DBG_ERR("Too many arguments assigned for CIPSERVERMAXCONN.");
        AT_DBG_MSG("Remove. Arguments num[%d], cnt[%d], argv[%d]: %s.",
                argc, cnt, cnt, argv[cnt]);
        return ret;
    }

    *max_client = value;
    ret = HAL_OK;
    return ret;
}


// This routine try to get the input arguments for AT+CIPMODE command. It will get the arguments
// sequencely and check the value accoring to the command's definition. When any error occurs
// it will prompt the related error message and return fail, or it will fill the send data struct
// for the caller and return success.
static int at_net_get_args_cipmode(const at_cmd_t *cmd, int *passth_mode, int *read_cmd)
{
    // Set the default status to "fail"
    int argc = 0;
    int ret = HAL_ERROR;
    int cnt = 0;
    int value = 0;
    char *argv[AT_SET_MAX_ARGC] = {0};

    *read_cmd = HAL_ERROR;
    if (AT_MODE_R == cmd->mode)
    {
        *read_cmd = HAL_OK;
        ret = HAL_OK;
        return ret;
    }

    // Parse arguments
    argc = at_parse_set(cmd->set, argv, AT_TEST_SETTAG);
    if ((0 == argc) || (NULL == argv[cnt]))
    {
        AT_DBG_ERR("No arguments assigned for cipmode command.");
        return ret;
    }

    value = at_net_atoi(argv[cnt]);
    if (AT_NET_BOOL_ARGUMENT_INVALID(value))
    {
        AT_DBG_ERR("Invalid passthrough mode %d.", value);
        return ret;
    }
    cnt++;

    if (NULL != argv[cnt])
    {
        AT_DBG_ERR("Too many arguments assigned for CIPMODE.");
        AT_DBG_MSG("Remove. Arguments num[%d], cnt[%d], argv[%d]: %s.",
                argc, cnt, cnt, argv[cnt]);
        return ret;
    }

    *passth_mode = value;
    ret = HAL_OK;
    return ret;
}

static int at_net_get_args_ping(const at_cmd_t *cmd, char **host)
{
    int argc = 0;
    int ret = HAL_ERROR;
    char *argv[AT_SET_MAX_ARGC] = {0};

    argc = at_parse_set(cmd->set, argv, AT_TEST_SETTAG);
    if (1 != argc)
    {
        AT_DBG_ERR("Invalid argurments for ping.");
        return ret;
    }

    ret = at_parse_param_str(&argv[0]);
    if (HAL_OK != ret)
    {
        AT_DBG_ERR("Invalid string parameter[%s]", argv[0]);
        return ret;
    }
    *host = argv[0];

    ret = HAL_OK;
    return ret;
}

#if CONFIG_NET_AT_PASSIVE_RECV
// This routine try to get the input arguments for AT+CIPRECVMODE command. It will get the arguments
// sequencely and check the value accoring to the command's definition. When any error occurs
// it will prompt the related error message and return fail, or it will fill the send data struct
// for the caller and return success.
static int at_net_get_args_ciprecvmode(const at_cmd_t *cmd, int *passive_recv_mode, int *read_cmd)
{
    // Set the default status to "fail"
    int ret = HAL_ERROR;
    int cnt = 0;
    int value = 0;
    int argc = 0;
    char *argv[AT_SET_MAX_ARGC] = {0};

    *read_cmd = HAL_ERROR;
    if (AT_MODE_R == cmd->mode)
    {
        *read_cmd = HAL_OK;
        ret = HAL_OK;
        return ret;
    }

    // Parse arguments
    argc = at_parse_set(cmd->set, argv, AT_TEST_SETTAG);
    if ((0 == argc) || (NULL == argv[0]))
    {
        AT_DBG_ERR("No arguments assigned for ciprecvmode command.");
        return ret;
    }

    value = at_net_atoi(argv[cnt]);
    if (AT_NET_BOOL_ARGUMENT_INVALID(value))
    {
        AT_DBG_ERR("Invalid passive recv data mode %d.", value);
        return ret;
    }
    cnt++;

    if (NULL != argv[cnt])
    {
        AT_DBG_ERR("Too many arguments assigned for CIPRECVMODE.");
        AT_DBG_MSG("Remove. Arguments num[%d], cnt[%d], argv[%d]: %s.",
                argc, cnt, cnt, argv[cnt]);
        return ret;
    }

    *passive_recv_mode = value;
    ret = HAL_OK;
    return ret;
}


// This routine try to get the input arguments for AT+CIPRECVDATA command. It will get the arguments
// sequencely and check the value accoring to the command's definition. When any error occurs
// it will prompt the related error message and return fail, or it will fill the send data struct
// for the caller and return success.
static int at_net_get_args_ciprecvdata(const at_cmd_t *cmd, int *link_id, int *recv_len)
{
    // Set the default status to "fail"
    int argc = 0;
    int cnt = 0;
    int ret = HAL_ERROR;
    int value = 0;
    char *argv[AT_SET_MAX_ARGC] = {0};

    // Parse arguments
    argc = at_parse_set(cmd->set, argv, AT_TEST_SETTAG);
    if ((0 == argc) || (NULL == argv[cnt]))
    {
        AT_DBG_ERR("No arguments assigned for CIPRECVDATA command.");
        return ret;
    }

    ret = at_net_get_args_linkid(argv, cnt, &value);
    if (AT_NET_LINK_ID_GET_FAIL(ret))
    {
        ret = HAL_ERROR;
        return ret;
    }
    else if (AT_NET_LINK_ID_ASSIGNED(ret))
    {
        cnt++;
    }
    ret = HAL_ERROR;
    *link_id = value;

    if (NULL == argv[cnt])
    {
        AT_DBG_ERR("No receive length assigned.");
        return ret;
    }
    value = at_net_atoi(argv[cnt]);
    if (value < 0 || AT_NET_MAX_CMD_RECV_SIZE < value)
    {
        AT_DBG_ERR("Receive data length[%d] out of range[0, %d]",
                value, AT_NET_MAX_CMD_RECV_SIZE);
        return ret;
    }
    *recv_len = value;
    cnt++;

    if (NULL != argv[cnt])
    {
        AT_DBG_ERR("Too many arguments assigned for CIPRECVDATA.");
        AT_DBG_MSG("Remove. Arguments num[%d], cnt[%d], argv[%d]: %s.",
                argc, cnt, cnt, argv[cnt]);
        return ret;
    }

    ret = HAL_OK;
    return ret;
}



// This routine try to get the input arguments for AT+CIPRECVLEN command. It will get the arguments
// sequencely and check the value accoring to the command's definition. When any error occurs
// it will prompt the related error message and return fail, or it will fill the send data struct
// for the caller and return success.
static int at_net_get_args_ciprecvlen(const at_cmd_t *cmd)
{
    // Set the default status to "fail"
    int ret = HAL_ERROR;

    if (AT_MODE_R != cmd->mode)
    {
        AT_DBG_ERR("Invalid command mode %d.", cmd->mode);
        return ret;
    }

    ret = HAL_OK;
    return ret;
}
#endif


// This routine to check the validation for arguments according to current
// settings: CurrentMode(CIPMUX, CIPSERVER, CIPMODE); 2. ExistConnections
// It return status to indicate HAL_OK or HAL_ERROR.

int at_net_parse_cipstart(at_net_conns_t *conns)
{
    // Set the default status "FALSE"
    int ret = HAL_ERROR;
    int mult_mode = 0, server_mode = 0;
    int idx = 0, cnt = 0;
    at_net_conns_t *saved_conns = NULL;

    // Compare to exist connections.
    for (idx = 0; idx < AT_NET_MAX_CONNS_NUM; idx++)
    {
        // If link id duplicated, prompt "ALREADY CONNECTED" and return
        saved_conns = at_net_get_conn(idx);
        if (NULL == saved_conns)
        {
            continue;
        }
        cnt++;

#if CONFIG_NET_AT_CMD_SSL
        if ((AT_NET_CONNS_TYPE_SSL == conns->type) 
                && (AT_NET_CONNS_TYPE_SSL == saved_conns->type))
        {
            at_rsp(AT_NET_PROMPT_SSL_REACH_MAX);
            return ret;
        }
#endif

        if ((saved_conns->link_id) == (conns->link_id))
        {
            at_rsp(AT_NET_PROMPT_CONNECT_DUP);
            return ret;
        }

        // If UDP mode, RemoteIP, RemotePort, LocalPort duplicated, prompt "<link_id>, CONNECT FAIL" and return
        //      --> Leave the network stack to check this error.
    }

    // Get current mode settings
    // If CIPMUX equal 0, but to create another connection, prompt "ALREADY CONNECTED" and return
    at_net_get_conn_mode(AT_NET_CONNS_DEV_MODE_MULT, &mult_mode);
    if ((0 == mult_mode) && (0 < cnt))
    {
        at_rsp(AT_NET_PROMPT_CONNECT_DUP);
        return ret;
    }

    // If CIPSERVER equal 1, but to create "TCP" connection, prompt "<link_id>, CLOSED" and return
    at_net_get_conn_mode(AT_NET_CONNS_DEV_MODE_SERVER, &server_mode);
    if ((1 == server_mode) && (AT_NET_CONNS_TYPE_TCP == conns->type))
    {
        at_rsp(AT_NET_PROMPT_LINK_CLOSED_MULT, conns->link_id);
        return ret;
    }

    // Return status information "TRUE" 
    ret = HAL_OK;
    return ret;
}




// This routine to parse the arguments according to the current settings: 
// 1. current modes
// 2. current exist connections
// If any error occurs, then prompt the relate error message and return "false" to indicate 
// the invalid arguments, or it will return true to indicate valid arguments.
int at_net_parse_cipsend(at_net_send_t *send)
{
    // Set the default status to "false"
    int ret = HAL_ERROR;

    at_net_conns_t *conns = (at_net_conns_t *)send->conns;
    ret = at_net_segment_send_finished(conns->link_id);
    if (HAL_OK != ret)
    {
        at_rsp(AT_NET_PROMPT_SEND_MODE_ERR);
        return ret;
    }

    ret = at_net_get_addr_from_host(conns, send);
    
    return ret;
}


// This routine to parse the arguments according to the current settings: 
// 1. current modes
// 2. current exist connections
// If any error occurs, then prompt the relate error message and return "false" to indicate 
// the invalid arguments, or it will return true to indicate valid arguments.
int at_net_parse_cipsendex(at_net_send_t *send)
{
    // Set the default status to "false"
    int ret = HAL_ERROR;
    int passth_mode = 0;
    at_net_conns_t *conns = (at_net_conns_t *)send->conns;

    /* Check for passthrough mode. */
    at_net_get_conn_mode(AT_NET_CONNS_DEV_MODE_PASSTH, &passth_mode);
    if (1 == passth_mode)
    {
        at_rsp(AT_NET_PROMPT_MODE_PASSTH_ENABLED);
        return ret;
    }

    ret = at_net_segment_send_finished(conns->link_id);
    if (HAL_OK != ret)
    {
        at_rsp(AT_NET_PROMPT_SEND_MODE_ERR);
        return ret;
    }
    
    ret = at_net_get_addr_from_host(conns, send);
    
    // Keep same to AT+CIPSEND.
    // Nothing to do for parsing arguments for AT+CIPSENDEX command.

    return ret;
}

// This routine to parse the arguments according to the current settings: 
// 1. current modes
// 2. current exist connections
// If any error occurs, then prompt the relate error message and return "false" to indicate 
// the invalid arguments, or it will return true to indicate valid arguments.
int at_net_parse_cipsendbuf(at_net_send_t *send, at_net_sendbuf_status_t **out_status)
{
    // Set the default status to "false"
    int ret = HAL_ERROR;
    int passth_mode = 0;
    at_net_conns_t *conns = (at_net_conns_t *)send->conns;
    at_net_sendbuf_status_t *status = NULL;

    /* Check for passthrough mode. */
    at_net_get_conn_mode(AT_NET_CONNS_DEV_MODE_PASSTH, &passth_mode);
    if (1 == passth_mode)
    {
        at_rsp(AT_NET_PROMPT_MODE_PASSTH_ENABLED);
        return ret;
    }

    // The connection for SENDBUF should be TCP mode 
    if (AT_NET_CONNS_TYPE_UDP == conns->type)
    {
        AT_DBG_ERR("LinkID[%d] realted connection is UDP mode.\n", conns->link_id);
        return ret;
    }

#if CONFIG_NET_AT_CMD_SSL
    if (AT_NET_CONNS_TYPE_SSL == conns->type)
    {
        AT_DBG_ERR("SSL link connection not support sendbuf command.");
        at_rsp(AT_NET_PROMPT_SSL_NOT_SUPPORT);
        return ret;
    }
#endif
        
    status = at_net_get_sendbuf_status(conns->link_id);
    if (NULL == status)
    {
        AT_DBG_ERR("Get sendbuf status fail.");
        return ret;
    }

    if (status->remain_size < send->len)
    {
        AT_DBG_ERR("Remain size %d less than send length %d.",
            status->remain_size, send->len);
        return ret;
    }
    // Keep same to AT+CIPSEND.
    // Nothing to do for parsing arguments for AT+CIPSENDEX command.

    // Return true.
    *out_status = status;
    ret = HAL_OK;
    return ret;
}

// This routine to parse the arguments according to the current settings: 
// 1. current modes
// 2. current exist connections
// If any error occurs, then prompt the relate error message and return "false" to indicate 
// the invalid arguments, or it will return true to indicate valid arguments.
int at_net_parse_cipbufreset(int link_id)
{
    // Set the default status to "false"
    int ret = HAL_ERROR;
    at_net_conns_t *conns = NULL;

    conns = at_net_get_conn(link_id);
    // The connection for SENDBUF should be TCP mode 
    if (AT_NET_CONNS_TYPE_UDP == conns->type)
    {
        AT_DBG_ERR("LinkID[%d] realted connection is UDP mode.\n", link_id);
        return ret;
    }

#if CONFIG_NET_AT_CMD_SSL
    if (AT_NET_CONNS_TYPE_SSL == conns->type)
    {
        AT_DBG_ERR("SSL link connection not support sendbuf command.");
        at_rsp(AT_NET_PROMPT_SSL_NOT_SUPPORT);
        return ret;
    }
#endif

    // Return true.
    ret = HAL_OK;
    return ret;
}

// This routine to parse the arguments according to the current settings: 
// 1. current modes
// 2. current exist connections
// If any error occurs, then prompt the relate error message and return "false" to indicate 
// the invalid arguments, or it will return true to indicate valid arguments.
int at_net_parse_cipbufstatus(int link_id)
{
    // Set the default status to "false"
    int ret = HAL_ERROR;
    at_net_conns_t *conns = NULL;

    conns = at_net_get_conn(link_id);
    // The connection for SENDBUF should be TCP mode 
    // Note: the AT+CIPBUFSTATUS will show the related status even if the connection isn't exist.
    if ((NULL != conns) && (AT_NET_CONNS_TYPE_UDP == conns->type))
    {
        AT_DBG_ERR("LinkID[%d] realted connection is UDP mode.\n", link_id);
        return ret;
    }

#if CONFIG_NET_AT_CMD_SSL
    if (AT_NET_CONNS_TYPE_SSL == conns->type)
    {
        AT_DBG_ERR("SSL link connection not support sendbuf command.");
        at_rsp(AT_NET_PROMPT_SSL_NOT_SUPPORT);
        return ret;
    }
#endif

    // Return true.
    ret = HAL_OK;
    return ret;
}


// This routine to parse the arguments according to the current settings: 
// 1. current modes
// 2. current exist connections
// If any error occurs, then prompt the relate error message and return "false" to indicate 
// the invalid arguments, or it will return true to indicate valid arguments.
int at_net_parse_cipcheckseq(int link_id)
{
    // Set the default status to "false"
    int ret = HAL_ERROR;
    at_net_conns_t *conns = NULL;

    conns = at_net_get_conn(link_id);
    // The connection for SENDBUF should be TCP mode 
    if (AT_NET_CONNS_TYPE_UDP == conns->type)
    {
        AT_DBG_ERR("LinkID[%d] realted connection is UDP mode.\n", link_id);
        return ret;
    }

#if CONFIG_NET_AT_CMD_SSL
    if (AT_NET_CONNS_TYPE_SSL == conns->type)
    {
        AT_DBG_ERR("SSL link connection not support sendbuf command.");
        at_rsp(AT_NET_PROMPT_SSL_NOT_SUPPORT);
        return ret;
    }
#endif

    // Return true.
    ret = HAL_OK;
    return ret;
}


// This routine to parse the arguments according to the current settings: 
// 1. current modes
// 2. current exist connections
// If any error occurs, then prompt the relate error message and return "false" to indicate 
// the invalid arguments, or it will return true to indicate valid arguments.
int at_net_parse_cipmux(int mult_mode)
{
    // Set the default status to "false"
    int ret = HAL_ERROR;
    int link_id = 0;
    int passth_mode = 0;
    int server_mode = 0;
    at_net_conns_t *conns = NULL;

    // 1. No connections exist.
    for (link_id = 0; link_id < AT_NET_MAX_CONNS_NUM; link_id++)
    {
        conns = at_net_get_conn(link_id);
        if (NULL != conns)
        {
            AT_DBG_WARN("Connection %d exist.", link_id);
            AT_DBG_MSG("Remove. link_id %d, socket %d, mode %d, type %d, remote_port %d, local_port %d.",
                    conns->link_id, conns->socket, conns->mode, conns->type, conns->remote_port, conns->local_port);
            break;
        }
    }
    if (link_id < AT_NET_MAX_CONNS_NUM)
    {
        at_rsp(AT_NET_PROMPT_LINK_IS_BUILDED);
        return ret;
    }

    // 2. PassthroughMode enabled, MultipleMode shouldn't be enabled
    if (1 == mult_mode)
    {
        at_net_get_conn_mode(AT_NET_CONNS_DEV_MODE_PASSTH, &passth_mode);
        if (1 == passth_mode)
        {
            at_rsp(AT_NET_PROMPT_MODE_PASSTH_MUST_0);
            return ret;
        }
    }
    else if (0 == mult_mode)
    {
        at_net_get_conn_mode(AT_NET_CONNS_DEV_MODE_SERVER, &server_mode);
        if (1 == server_mode)
        {
            at_rsp(AT_NET_PROMPT_MODE_MULT_MUST_0);
            return ret;
        }
    }

    // Return true.
    ret = HAL_OK;
    return ret;
}


// This routine to parse the arguments according to the current settings: 
// 1. current modes
// 2. current exist connections
// If any error occurs, then prompt the relate error message and return "false" to indicate 
// the invalid arguments, or it will return true to indicate valid arguments.
int at_net_parse_cipserver(int server_mode)
{
    // Set the default status to "false"
    int ret = HAL_ERROR;
    int mult_mode = 0;

    // 1. PassthroughMode enabled, MultipleMode shouldn't be enabled
    at_net_get_conn_mode(AT_NET_CONNS_DEV_MODE_MULT, &mult_mode);
    if (0 == mult_mode)
    {
        AT_DBG_ERR("MultipleMode disabled.");
        return ret;
    }

    // Return true.
    ret = HAL_OK;
    return ret;
}


// This routine to parse the arguments according to the current settings: 
// 1. current modes
// 2. current exist connections
// If any error occurs, then prompt the relate error message and return "false" to indicate 
// the invalid arguments, or it will return true to indicate valid arguments.
int at_net_parse_cipservermaxconn()
{
    // Set the default status to "false"
    int ret = HAL_ERROR;

    // Nothing to do...
    // TODO.

    // Return true.
    ret = HAL_OK;
    return ret;
}

// This routine to parse the arguments according to the current settings: 
// 1. current modes
// 2. current exist connections
// If any error occurs, then prompt the relate error message and return "false" to indicate 
// the invalid arguments, or it will return true to indicate valid arguments.
int at_net_parse_cipmode(int passth_mode)
{
    // Set the default status to "false"
    int ret = HAL_ERROR;
    int mult_mode = 0;

    // MultipleMode enabled, PassthroughMode can't to be set
    at_net_get_conn_mode(AT_NET_CONNS_DEV_MODE_MULT, &mult_mode);
    if (1 == mult_mode)
    {
        at_rsp(AT_NET_PROMPT_MODE_MULT_MUST_0);
        return ret;
    }

    // Return true.
    ret = HAL_OK;
    return ret;
}


#if CONFIG_NET_AT_PASSIVE_RECV
// This routine to parse the arguments according to the current settings: 
// 1. current modes
// 2. current exist connections
// If any error occurs, then prompt the relate error message and return "false" to indicate 
// the invalid arguments, or it will return true to indicate valid arguments.
int at_net_parse_ciprecvmode(int passive_recv_mode)
{
    // Set the default status to "false"
    int ret = HAL_ERROR;

    // Nothing to do...
    // TODO.

    // Return true.
    ret = HAL_OK;
    return ret;
}


// This routine to parse the arguments according to the current settings: 
// 1. current modes
// 2. current exist connections
// If any error occurs, then prompt the relate error message and return "false" to indicate 
// the invalid arguments, or it will return true to indicate valid arguments.
int at_net_parse_ciprecvdata(int link_id, int recv_len)
{
    // Set the default status to "false"
    int ret = HAL_ERROR;

    // Nothing to do right now.

    // Return true.
    ret = HAL_OK;
    return ret;
}
#endif

static int atwlan_get_arg(void *context, char *argv[])
{
    int    argc;

    ASSERT(context);

    at_cmd_t *cmd = (at_cmd_t *)context;

    argc = at_parse_set(cmd->set, argv, AT_TEST_SETTAG); 

    return argc;
}

static void atwlan_cipstatus (void *context)
{
    int ret = HAL_ERROR;
    int link_id = 0;
    at_net_conns_dev_status_e status = AT_NET_DEV_STATUS_NOT_ASSOC;
    at_net_conns_t *conns = NULL;
    at_cmd_t *cmd = (at_cmd_t *)context;
    ASSERT(cmd);

    ret = at_net_get_args_cipstatus(cmd);
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }

    at_net_get_dev_status(&status);
    if (AT_NET_DEV_STATUS_NOT_ASSOC == status)
    {
        at_net_set_dev_status(status);
    }

    at_rsp(AT_NET_PROMPT_READ_STATUS_DEV, status);
    for (link_id = 0; link_id < AT_NET_MAX_CONNS_NUM; link_id++)
    {
        conns = at_net_get_conn(link_id);
        if (NULL == conns)
        {
            continue;
        }
        at_rsp(AT_NET_PROMPT_READ_STATUS_ENTRY, 
                conns->link_id, 
                ((AT_NET_CONNS_TYPE_TCP == conns->type) ? "TCP" : 
                 ((AT_NET_CONNS_TYPE_UDP == conns->type) ? "UDP" : "SSL")), 
                inet_ntoa(conns->remote_addr), 
                conns->remote_port, 
                conns->local_port, 
                0);
        AT_DBG_MSG("Remove. socket[%d], mode[%d], last active time[%08x].",
                conns->socket, conns->mode, conns->last_active_time);
    }

    at_rsp(AT_OK);
    return;
}

static void atwlan_cipdomain (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    ASSERT(cmd);
}


//tcp client connect
static void atwlan_cipstart (void *context)
{
    int ret = HAL_ERROR;
    int mult_mode = 0;
    at_net_conns_t new_conns = {0};
    at_net_conns_t *conns;
    at_cmd_t *cmd = (at_cmd_t *)context;
    ASSERT(cmd);
    

    /* Get input arguments for AT+CIPSTART command */
    ret = at_net_get_args_cipstart(cmd, &new_conns);
    // If get arguments fail, prompt "ERROR" and return
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }
    
    /* parse arguments to check whether it's valid */
    ret = at_net_parse_cipstart(&new_conns);
    // If arguments invalid, prompt "ERROR" and return
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }
    
    //malloc new conns from free
    conns = at_net_conns_malloc();
    if(!conns) {
        AT_DBG_ERR("Conection already full\n");
        at_rsp(AT_ERROR);
        return;
    }
    //conns check ok copy it
    memcpy(conns, &new_conns, sizeof(at_net_conns_t));

#if CONFIG_NET_AT_CMD_SSL
    at_net_get_tls_crts(conns);
#endif
    
    /* Connect to assigned port*/
    ret = at_net_open_connection(conns);
    // If connect fail, prompt "ERROR" and return
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        at_net_conns_free(conns);
        return;
    }
    
    /* Save connections */
    // If connect success, save the connection
    conns->mode = AT_NET_CONNS_MODE_ACTIVE;
    conns->last_active_time = wl_get_systemtick();
#if CONFIG_NET_AT_PASSIVE_RECV
    conns->passive_recv_full = FALSE;
#endif
    at_net_open(conns);
    AT_DBG_MSG("Remove. socket[%d], link_id[%d], mode[%d], last active time[%08x].",
            conns->socket, conns->link_id, conns->mode, conns->last_active_time);

    /* Return OK */
    // Display the connection status
    at_net_get_conn_mode(AT_NET_CONNS_DEV_MODE_MULT, &mult_mode);
    if (1 == mult_mode)
    {
        at_rsp(AT_NET_PROMPT_CONNECT_OK_MULT, conns->link_id);
    }
    else
    {
        at_rsp(AT_NET_PROMPT_CONNECT_OK_SING);
    }
    at_rsp(AT_OK);
    return;
}


static void atwlan_cipsslsize (void *context)
{
#if CONFIG_NET_AT_CMD_SSL
    int ret = HAL_ERROR;
    int size = 0;
    at_cmd_t *cmd = (at_cmd_t *)context;
    ASSERT(cmd);
    at_net_tls_conf_t *tls_conf = NULL;

    ret = at_net_get_args_cipsslsize(cmd, &size);
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }
    
    tls_conf = at_net_get_tls_conf();
    if (NULL == tls_conf)
    {
        at_rsp(AT_ERROR);
        return;
    }

    // Set the ssl size config
    tls_conf->size = size;
    tls_conf->remain_size = size;
#endif

    at_rsp(AT_OK);
    // Return
    return;
}


static void atwlan_cipsslcconf (void *context)
{
#if CONFIG_NET_AT_CMD_SSL
    int ret = HAL_ERROR;
    int read_cmd = HAL_ERROR;
    int auth_mode = AT_NET_TLS_AUTH_MODE_NONE;
    at_cmd_t *cmd = (at_cmd_t *)context;
    ASSERT(cmd);
    at_net_tls_conf_t *tls_conf = NULL;

    ret = at_net_get_args_cipsslcconf(cmd, &auth_mode, &read_cmd);
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }

    // Set the ssl auth mode config
    tls_conf = at_net_get_tls_conf();
    if (NULL == tls_conf)
    {
        at_rsp(AT_ERROR);
        return;
    }

    if (HAL_OK == read_cmd)
    {
        at_rsp(AT_NET_PROMPT_READ_SSLCCONF, tls_conf->auth_mode);
    }
    else
    {
        tls_conf->auth_mode = auth_mode;
    }
#endif

    at_rsp(AT_OK);
    // Return
    return;
}


static void atwlan_cipsend (void *context)
{
    int ret = HAL_ERROR;
    int passth_mode = 0;
    at_net_send_t send;
    at_cmd_t *cmd = (at_cmd_t *)context;
     at_net_conns_t *conns;
    ASSERT(cmd);

    memset((void*)(&send), 0, sizeof(send));
    // Get input arguments for AT+CIPSEND command
    ret = at_net_get_args_cipsend(cmd, &send);
    // If get fails, AT_ERROR and return
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }
  // Parse arguments according current settings
    ret = at_net_parse_cipsend(&send);
    // If arguments invalid, AT_ERROR and return
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }
    
    //copy send struct
    conns = (at_net_conns_t *)send.conns;
    if(NULL == conns) 
    {
        at_rsp(AT_ERROR);
        return;
    }

    wl_enter_critical();
    memcpy(&conns->send, &send, sizeof(at_net_send_t));
    wl_exit_critical();
    
    // Check for PassthroughMode
    at_net_get_conn_mode(AT_NET_CONNS_DEV_MODE_PASSTH, &passth_mode);
    // If CIPMODE equal 1, enter PassthroughMode to transfer data
    if (1 == passth_mode)
    {
        at_cmd_set_passth_mode(ENABLE, NULL, FALSE);
    } 
    else
    {
        at_cmd_set_passth_mode(ENABLE, conns, FALSE);
    }

    at_rsp(AT_OK);
    at_rsp(AT_NET_PROMPT_BYPASS_CMD_CHAR);
    return;
}


static void atwlan_cipsendex (void *context)
{
    int ret = HAL_ERROR;
    at_net_send_t send;
    at_cmd_t *cmd = (at_cmd_t *)context;
    at_net_conns_t *conns;

    ASSERT(cmd);

    // Malloc CIP_CONNS_STRUCT and fill with invalid value. 
       memset((void*)(&send), 0, sizeof(send));

    // Get input arguments for AT+CIPSENDEX command
    ret = at_net_get_args_cipsendex(cmd, &send);
    // If get fails, AT_ERROR and return
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }

    // Parse arguments according current settings
    ret = at_net_parse_cipsendex(&send);
    // If arguments invalid, AT_ERROR and return
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }
    
    conns = (at_net_conns_t *)send.conns;
    if(NULL == conns) 
    {
        at_rsp(AT_ERROR);
        return;
    }
    
    wl_enter_critical();
    memcpy(&conns->send, &send, sizeof(at_net_send_t));
    at_net_change_udp_remote(conns);
    wl_exit_critical();

    at_cmd_set_passth_mode(ENABLE, conns, FALSE);
            
    // Return
    // Print OK.
    at_rsp(AT_OK);
    at_rsp(AT_NET_PROMPT_BYPASS_CMD_CHAR);
    return;
}


// This routine try to get the input arguments for AT+CIPSENDBUF command. It will get the arguments
// sequencely and check the value accoring to the command's definition. When any error occurs
// it will prompt the related error message and return fail, or it will fill the send data struct
// for the caller and return success.

    // Set default status to "fail"
    // 

static void atwlan_cipsendbuf (void *context)
{
    int ret = HAL_ERROR;
    at_net_send_t send;
    at_net_sendbuf_status_t *status = NULL;
    at_cmd_t *cmd = (at_cmd_t *)context;
    at_net_conns_t *conns;
    ASSERT(cmd);

    // Malloc CIP_CONNS_STRUCT and fill with invalid value. 
       memset((void*)(&send), 0, sizeof(send));

    // Get input arguments for AT+CIPSENDBUF command
    ret = at_net_get_args_cipsendbuf(cmd, &send);
    // If get fails, AT_ERROR and return
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }

    // Parse arguments according current settings
    ret = at_net_parse_cipsendbuf(&send, &status);
    // If arguments invalid, AT_ERROR and return
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }
    conns = (at_net_conns_t *)send.conns;
    if (NULL == conns)
    {
        at_rsp(AT_ERROR);
        return;
    }
    
    wl_enter_critical();
    memcpy(&conns->send, &send, sizeof(at_net_send_t));
    at_net_change_udp_remote(conns);
    wl_exit_critical();
    
    status->next_seg_id++;
    at_cmd_set_passth_mode(ENABLE, conns, FALSE);

    // Print status info for SENDBUF command
    at_net_print_sendbuf_result(status);
    // Print OK.
    at_rsp(AT_OK);
    at_rsp(AT_NET_PROMPT_BYPASS_CMD_CHAR);
    return;
}


static void atwlan_cipbufreset (void *context)
{
    int ret = HAL_ERROR;
    int link_id = 0;
    at_net_sendbuf_status_t *status = NULL;
    at_cmd_t *cmd = (at_cmd_t *)context;

    ASSERT(cmd);

    ret = at_net_get_args_cipbufreset(cmd, &link_id);
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }
    
    ret = at_net_parse_cipbufreset(link_id);
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }
    
    status = at_net_get_sendbuf_status(link_id);
    if (NULL == status)
    {
        AT_DBG_ERR("Get sendbuf status fail.");
        at_rsp(AT_ERROR);
        return;
    }

    ret = at_net_segment_send_finished(link_id);
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }

    at_net_sendbuf_status_init(status);

    // Print OK.
    at_rsp(AT_OK);
}


static void atwlan_cipbufstatus (void *context)
{
    int ret = HAL_ERROR;
    int link_id = 0;
    int remain_size = 0, queue_num = 0;
    at_net_sendbuf_status_t *status = NULL;
    at_net_conns_t *conns = NULL;
    at_cmd_t *cmd = (at_cmd_t *)context;

    ASSERT(cmd);

    ret = at_net_get_args_cipbufstatus(cmd, &link_id);
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }

    ret = at_net_parse_cipbufstatus(link_id);
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }
    
    status = at_net_get_sendbuf_status(link_id);
    if (NULL == status)
    {
        AT_DBG_ERR("Get sendbuf status fail.");
        at_rsp(AT_ERROR);
        return;
    }

    conns = at_net_get_conn(link_id);
    if (NULL != conns)
    {
        remain_size = status->remain_size;
        queue_num = status->queue_num;
    }

    at_rsp(AT_NET_PROMPT_SENDBUF_STATUS, 
            status->next_seg_id, 
            status->last_send_seg, 
            status->last_succ_seg, 
            remain_size, 
            queue_num);

    // Print OK.
    at_rsp(AT_OK);
    return;
}


static void atwlan_cipcheckseq (void *context)
{
    int ret = HAL_ERROR;
    int link_id = 0, seg_id = 0;
    int mult_mode = 0, send_result= 0;
    at_cmd_t *cmd = (at_cmd_t *)context;
    ASSERT(cmd);

    ret = at_net_get_args_cipcheckseq(cmd, &link_id, &seg_id);
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }

    ret = at_net_get_segment_send_result(link_id, seg_id, &send_result);
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }

    at_net_get_conn_mode(AT_NET_CONNS_DEV_MODE_MULT, &mult_mode);
    if (1 == mult_mode)
    {
        at_rsp(AT_NET_PROMPT_SEGMENT_MULT, link_id, seg_id, 
            ((send_result == HAL_OK) ? AT_NET_PROMPT_SEND_RES_TRUE : AT_NET_PROMPT_SEND_RES_FALSE));
    }
    else if (0 == mult_mode)
    {
        at_rsp(AT_NET_PROMPT_SEGMENT_SING, seg_id, 
            ((send_result == HAL_OK) ? AT_NET_PROMPT_SEND_RES_TRUE : AT_NET_PROMPT_SEND_RES_FALSE));
    }

    // Print OK.
    at_rsp(AT_OK);
    return;
}


static void atwlan_cipclose (void *context)
{
    int ret = HAL_ERROR;
    int link_id = 0;
    at_net_conns_t *conns = NULL;
    at_cmd_t *cmd = (at_cmd_t *)context;
    ASSERT(cmd);

    ret = at_net_get_args_cipclose(cmd, &link_id);
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }

    conns = at_net_get_conn(link_id);
    ret = at_net_close(conns); 
    //TODO pending
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }

    // Print OK.
    at_rsp(AT_OK);
    return;
}             


static void atwlan_cifsr (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context; 
    // wlan_start set mode to STA, wifi_on set mode to wlan driver in wlan_task.
    // Here we need a function wlan_get_mode to get the wlan mode from driver.
    int if_mode = S907X_MODE_NONE;          //need get wifi mode funtion.
    int ap_if_idx = -1, sta_if_idx = -1;
    u8 *mac = NULL;
    ip_addr_t ip = {0};
    struct netif *pnetif = NULL;

    ASSERT(cmd);
    
    if_mode = s907x_wlan_get_mode();
    switch (if_mode) {
        case S907X_MODE_STA_AP:
            sta_if_idx = S907X_DEV0_ID;
            ap_if_idx = S907X_DEV1_ID;
            break;

        case S907X_MODE_STA:
            sta_if_idx = S907X_DEV0_ID;
            break;

        case S907X_MODE_AP:
            ap_if_idx = S907X_DEV0_ID;
            break;

        default:
            AT_DBG_ERR("Unkown wifi interface mode[%d].", if_mode);
    }

    if (-1 != ap_if_idx) {
        pnetif = LwIP_GetNetif(ap_if_idx); 
        if(netif_is_up(pnetif)) {
            mac = LwIP_GetMAC(pnetif);
            memcpy(&ip, LwIP_GetIP(pnetif), sizeof(ip_addr_t));

            at_rsp(AT_NET_PROMPT_IFS_IP_AP, inet_ntoa(ip)); 
            at_rsp(AT_NET_PROMPT_IFS_MAC_AP, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]) ;
        }
    }

    if (-1 != sta_if_idx) {
        pnetif = LwIP_GetNetif(sta_if_idx); 
        if(netif_is_up(pnetif)) {
            mac = LwIP_GetMAC(pnetif);
            ip.addr = 0;
            if (netif_is_link_up(pnetif)) {
                memcpy(&ip, LwIP_GetIP(pnetif), sizeof(ip_addr_t));
            }

            at_rsp(AT_NET_PROMPT_IFS_IP_STA, inet_ntoa(ip));        
            at_rsp(AT_NET_PROMPT_IFS_MAC_STA, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]) ;
        }
    }

    at_rsp(AT_OK);         
    return;
}


static void atwlan_cipmux (void *context)
{
    int ret = HAL_ERROR;
    int mult_mode = 0;
    int read_cmd = HAL_ERROR;
    at_cmd_t *cmd = (at_cmd_t *)context;
    ASSERT(cmd);

    ret = at_net_get_args_cipmux(cmd, &mult_mode, &read_cmd);
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }
    //read cipmux
    if (HAL_OK == read_cmd)
    {
        at_net_get_conn_mode(AT_NET_CONNS_DEV_MODE_MULT, &mult_mode);
        at_rsp(AT_NET_PROMPT_READ_MUX, mult_mode);
    }
    //write cipmux
    else
    {
        ret = at_net_parse_cipmux(mult_mode);
        if (HAL_OK != ret)
        {
            at_rsp(AT_ERROR);
            return;
        }

        at_net_set_conn_mode(AT_NET_CONNS_DEV_MODE_MULT, mult_mode);
    }

    // Print OK.
    at_rsp(AT_OK);
    return;
}


static void atwlan_cipserver (void *context)
{
    int ret = HAL_ERROR;
    int local_port = 0;
    int server_mode = 0;
    int server_sock = -1;
    int orig_server_mode = 0;
    at_cmd_t *cmd = (at_cmd_t *)context;
    at_net_tcp_server_t *tcp_server = NULL;
    ASSERT(cmd);

    ret = at_net_get_args_cipserver(cmd, &server_mode, &local_port);
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }

    ret = at_net_parse_cipserver(server_mode);
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }

    // Compare to the orignal server mode
    at_net_get_conn_mode(AT_NET_CONNS_DEV_MODE_SERVER, &orig_server_mode);
    if (orig_server_mode == server_mode)
    {
        at_rsp(AT_NET_PROMPT_MODE_SERVER_NO_CHANGE);
        at_rsp(AT_OK);
        return;
    }

    tcp_server = at_net_get_tcp_server();
    if (NULL == tcp_server)
    {
        at_rsp(AT_ERROR);
        return;
    }

    if (1 == server_mode)
    {
        ret = at_net_bind_server_socket(local_port, tcp_server->max_client, &server_sock);
        if (HAL_OK != ret)
        {
            at_rsp(AT_ERROR);
            return;
        }
    }
    else 
    {
        AT_DBG_MSG("Remove. Set the tcp server mode to 0.");
        server_sock = tcp_server->socket;
        close(server_sock);

        server_sock = -1;
        local_port = 0;
    }

    tcp_server->socket = server_sock;
    tcp_server->port = local_port;
    AT_DBG_MSG("Remove. server socket[%d-%d], local port[%d-%d], Passive connection cnt[%d], limit[%d].", 
            server_sock, tcp_server->socket, local_port, tcp_server->port,
            tcp_server->cnt_client, tcp_server->max_client);
    at_net_set_conn_mode(AT_NET_CONNS_DEV_MODE_SERVER, server_mode);

    at_net_task_event_send(AT_NET_TASK_TCP_SERVER);

    at_rsp(AT_OK);
    return;
}


static void atwlan_cipservermaxconn (void *context)
{
    int ret = HAL_ERROR;
    int max_client = 0;
    int read_cmd = HAL_ERROR;
    at_net_tcp_server_t *tcp_server = NULL;
    at_cmd_t *cmd = (at_cmd_t *)context;
    ASSERT(cmd);
    
    ret = at_net_get_args_cipservermaxconn(cmd, &max_client, &read_cmd);
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }

    ret = at_net_parse_cipservermaxconn();
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }

    tcp_server = at_net_get_tcp_server();
    if (NULL == tcp_server)
    {
        at_rsp(AT_ERROR);
        return;
    }

    if (HAL_OK == read_cmd)
    {
        at_rsp(AT_NET_PROMPT_READ_SERVERMAX, tcp_server->max_client);
    }
    else
    {
        tcp_server->max_client = max_client;
    }

    at_rsp(AT_OK);
    return;
}


static void atwlan_cipmode (void *context)
{
    int ret = HAL_ERROR;
    int passth_mode = 0;
    int read_cmd = HAL_ERROR;
    at_cmd_t *cmd = (at_cmd_t *)context;
    ASSERT(cmd);

    ret = at_net_get_args_cipmode(cmd, &passth_mode, &read_cmd);
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }

    if (HAL_OK == read_cmd)
    {
        at_net_get_conn_mode(AT_NET_CONNS_DEV_MODE_PASSTH, &passth_mode);
        at_rsp(AT_NET_PROMPT_READ_MODE, passth_mode);
    }
    else
    {
        ret = at_net_parse_cipmode(passth_mode);
        if (HAL_OK != ret)
        {
            at_rsp(AT_ERROR);
            return;
        }

        at_net_set_conn_mode(AT_NET_CONNS_DEV_MODE_PASSTH, passth_mode);
    }

    // Print OK.
    at_rsp(AT_OK);
    return;
}             


static void atwlan_savetranslink (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    ASSERT(cmd);
}             


static void atwlan_cipsto (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    ASSERT(cmd);
}             


static void atwlan_ping (void *context)
{
    int ret = HAL_ERROR;
    at_cmd_t *cmd = (at_cmd_t *)context;
    char *host = NULL;

    ASSERT(cmd);

    ret = at_net_get_args_ping(cmd, &host);
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }

    ret = at_net_ping(host);
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }

    // Print OK.
    at_rsp(AT_OK);
    return;
}  
 

#if CONFIG_NET_AT_PASSIVE_RECV
static void atwlan_ciprecvmode (void *context)
{
    int ret = HAL_ERROR;
    int read_cmd = HAL_ERROR;
    int passive_recv_mode = 0;
    at_cmd_t *cmd = (at_cmd_t *)context;
    ASSERT(cmd);

    ret = at_net_get_args_ciprecvmode(cmd, &passive_recv_mode, &read_cmd);
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }

    if (HAL_OK == read_cmd)
    {
        at_net_get_conn_mode(AT_NET_CONNS_DEV_MODE_PASSIVE_RECV, &passive_recv_mode);
        at_rsp(AT_NET_PROMPT_READ_RECVMODE, passive_recv_mode);
    }
    else 
    {
        ret = at_net_parse_ciprecvmode(passive_recv_mode);
        if (HAL_OK != ret)
        {
            at_rsp(AT_ERROR);
            return;
        }

        at_net_set_conn_mode(AT_NET_CONNS_DEV_MODE_PASSIVE_RECV, passive_recv_mode);
    }

    // Print OK.
    at_rsp(AT_OK);
    return;
}


static void atwlan_ciprecvdata (void *context)
{
    int ret = HAL_ERROR;
    int link_id = 0, recv_len = 0;
    at_cmd_t *cmd = (at_cmd_t *)context;
    ASSERT(cmd);

    ret = at_net_get_args_ciprecvdata(cmd, &link_id, &recv_len);
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }

    ret = at_net_parse_ciprecvdata(link_id, recv_len);
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }

    ret = at_net_fetch_passive_recv_data(link_id, recv_len);
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    }

    // Print OK.
    at_rsp(AT_OK);
    return;
}

static void atwlan_ciprecvlen (void *context)
{
    int ret = HAL_ERROR;
    int link_id = 0;
    int recv_len[AT_NET_MAX_CONNS_NUM] = {0};
    at_cmd_t *cmd = (at_cmd_t *)context;
    ASSERT(cmd);

    ret = at_net_get_args_ciprecvlen(cmd);
    if (HAL_OK != ret)
    {
        at_rsp(AT_ERROR);
        return;
    } 
 
    for (link_id = 0; link_id < AT_NET_MAX_CONNS_NUM; link_id++)
    {
        at_net_get_passive_recv_data_len(link_id, &(recv_len[link_id]));
    }

    at_rsp(AT_NET_PROMPT_RECV_DATA_LEN, 
            recv_len[0], recv_len[1], recv_len[2], recv_len[3], recv_len[4]);
    at_rsp(AT_OK);
    return;
}
#endif  
  

static void atwlan_cipsntpcfg (void *context)
{
    char  rsp[MAX_RSP_SIZE];
    char *default_server[SNTP_MAX_SERVERS] = {{"cn.ntp.org.cn"},{"ntp.sjtu.edu.cn"},{"us.pool.ntp.org"}}; //default sntp server
    int i;
    int server_argv_start;
    int sntp_enable;
    int timezone;
    at_cmd_t *cmd = (at_cmd_t *)context;
    char  *argv[AT_SET_MAX_ARGC];
    int    argc;
    ASSERT(cmd);

    char * sntp_server[SNTP_MAX_SERVERS];

    argc = atwlan_get_arg(context, argv);

    switch(cmd->mode)
    {
        case AT_MODE_R:
            sprintf(rsp, "+CIPSNTPCFG:%s%d,%d", sntp_enabled(), sntp_get_zone())  ;
            for(i = 0; i < SNTP_MAX_SERVERS; i++){
              strcat(rsp, sntp_getservername(i));
              if(i != (SNTP_MAX_SERVERS - 1)) {
                strcat(rsp, ",");
              }
            }
            at_rsp(rsp);
            break;
        case AT_MODE_W:
            sntp_enable = atoi(argv[0]);
            timezone    = atoi(argv[1]);
            
            if(sntp_enable) {
                sntp_set_zone(&rtc_hdl, timezone);
                if(argc > 2) {
                  ASSERT(argv[2] && argv[3] && argv[4]);
                  sntp_setup(&rtc_hdl, timezone, argv[2], argv[3], argv[4]);
                } else  {
                  for(i = 0 ;i < SNTP_MAX_SERVERS; i++){
                     //setSntpServer(i, default_server[i]);
                  }
                }     
            }   
            break;
        case AT_MODE_ACT:
        
            break;
        default:
            return;
    }



    at_rsp(AT_OK);
}


static void atwlan_cipsntptime (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    ASSERT(cmd);

    char * time_result;
    time_result = sntp_get_realtime(&rtc_hdl);//get time from sntp server

    at_rsp("+CIPSNTPTIME:%s",time_result);
    at_rsp(AT_OK);
}             


static void atwlan_cipdns_cur (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    ASSERT(cmd);
    char  *argv[AT_SET_MAX_ARGC];
    int    argc;
    char SERVER_TAG    = '.';

    char *default_server = {"208.67.222.222"}; //default dns server
    int dns_enable = atoi(argv[1]);
    int server_argv_start = 2;
    int count_null_argv = 0;
    bool READ_TAG = false;
    bool WRITE_TAG = false;

    argc = atwlan_get_arg(context, argv); 
    if(argc > AT_SET_MAX_ARGC) {
        return ;
    }   
    
    if(strchr(cmd->set,AT_READ_TAG) != NULL ){
      
      READ_TAG=true;
    }
    if(strchr(cmd->set,AT_WRITE_TAG) != NULL ){
      WRITE_TAG = true; 
      READ_TAG = false;
    }
    
    if(READ_TAG == false && WRITE_TAG == true ) { //this is write command.
 
    if(dns_enable) {   
            
      for(int i=0;i< DNS_MAX_SERVERS;i++){
      char * DnsServer = argv[server_argv_start+i];
      if(DnsServer !=  "" && DnsServer != NULL && strchr(DnsServer,SERVER_TAG) !=  NULL){
         dns_setserver(i,(ip_addr_t *) inet_addr(DnsServer));
      }
      else{
        count_null_argv++;
        continue;
        }
      }
      if(count_null_argv == DNS_MAX_SERVERS){//set default server    
       for(int i=0;i< SNTP_MAX_SERVERS;i++){
       dns_setserver(i,(ip_addr_t *)inet_addr(default_server));
       }
     
      } 
     } 
    }
    //this is read command.
    if(WRITE_TAG == false && READ_TAG == true ){  
      for(int i=0;i< DNS_MAX_SERVERS;i++){
#if LWIP_VERSION == LWIP_SCI		  
		  const ip_addr_t *dnsserver=(ip_addr_t *)dns_getserver(i);      
		  at_rsp("+CIPDNS_DEF::%s",inet_ntoa(dnsserver));
#elif LWIP_VERSION == LWIP_TUYA
		  ip_addr_t dnsserver=(ip_addr_t)dns_getserver(i);    
		  at_rsp("+CIPDNS_DEF::%s",inet_ntoa(&dnsserver));	  
#endif
      }

    }
    at_rsp(AT_OK);
        
        
}

static void atwlan_cipdns_def (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    int argc ; 
    char SERVER_TAG    = '.';
    char *argv[AT_SET_MAX_ARGC];
    char *default_server = {"208.67.222.222"}; //default dns server
    int dns_enable = atoi(argv[1]);
    int server_argv_start = 2;
    int count_null_argv = 0;
    bool READ_TAG = false;
    bool WRITE_TAG = false;
    
    if(argc > AT_SET_MAX_ARGC) {
        return ;
    }   
        
    argc = atwlan_get_arg(context, argv); 
    if(strchr(cmd->set,AT_READ_TAG) != NULL ){
      
      READ_TAG=true;
    }
    if(strchr(cmd->set,AT_WRITE_TAG) != NULL ){
      WRITE_TAG = true; 
      READ_TAG = false;
    }

    if(READ_TAG == false && WRITE_TAG == true ) { //this is write command.

    if(dns_enable) {   
            
      for(int i=0;i< DNS_MAX_SERVERS;i++){
      char * DnsServer = argv[server_argv_start+i];
      if(DnsServer !=  "" && DnsServer != NULL && strchr(DnsServer,SERVER_TAG) !=  NULL){
         dns_setserver(i,(ip_addr_t *) inet_addr(DnsServer));
         //need add save flash function.
         //Save_Flash_Data(DnsServer,"Dns");
      }
      else{
        count_null_argv++;
        continue;
        }
      }
      if(count_null_argv == DNS_MAX_SERVERS){//set default server    
       for(int i=0;i< SNTP_MAX_SERVERS;i++){
       dns_setserver(i,(ip_addr_t *)inet_addr(default_server));
       //need add save flash function.
       //Save_Flash_Data(default_server,"Dns");
       }
     
      } 
     } 
    }
    //this is read command.
    if(WRITE_TAG == false && READ_TAG == true ){  
      for(int i=0;i< DNS_MAX_SERVERS;i++){
#if LWIP_VERSION == LWIP_SCI		  
		  const ip_addr_t *dnsserver=(ip_addr_t *)dns_getserver(i);      
		  at_rsp("+CIPDNS_DEF::%s",inet_ntoa(dnsserver));
#elif LWIP_VERSION == LWIP_TUYA
		  ip_addr_t dnsserver=(ip_addr_t)dns_getserver(i);    
		  at_rsp("+CIPDNS_DEF::%s",inet_ntoa(&dnsserver));	  
#endif
      }

    }
    at_rsp(AT_OK);
}

#if CONFIG_NET_AT_LIST_PKT_CNT
static void atwlan_cippktstat(void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    int argc; 
    char *argv[AT_SET_MAX_ARGC];
    int value = 0;
    int ret = 0;
    
    // Set the default status to "fail"
    // Parse arguments
    argc = at_parse_set(cmd->set, argv, AT_TEST_SETTAG); 
    if((3 < argc)) {
        at_rsp(AT_NET_PROMPT_CMD_NUM_EXCEED);
        AT_DBG_ERR("at_parse_set fail, argc[%d] out range[3, 6].", argc);
        return;
    }

    /* get the interger LinkID*/
    value = at_net_atoi(argv[0]);

    if (value < AT_NET_DBG_TYPE_TOP || AT_NET_DBG_TYPE_BUTT < value)
    {
        at_rsp(AT_ERROR);
        return;
    }

    switch (value)
    {
        case AT_NET_LIST_RX_STAT:
            at_net_rx_pkt_stat(at_net_atoi(argv[1]));
            break;

        case AT_NET_LIST_TX_STAT:
            at_net_tx_pkt_stat(at_net_atoi(argv[1]));
            break;

        case AT_NET_GET_TASK_INFO:
            ret = at_parse_param_str(&argv[1]);

            if (HAL_OK != ret)
            {
                AT_DBG_ERR("Invalid string parameter[%s]", argv[1]);
                return;
            }
            at_net_get_task_info(argv[1]);
            break;

        default:
            at_rsp("Unkown command type[%d].\n", value);
            return;
    }
}
#endif
    
static at_item_t at_network_tbl[] = 
{
    {{"CIPSTATUS"},atwlan_cipstatus},    
    {{"CIPDOMAIN"},atwlan_cipdomain},    
    {{"CIPSTART"},atwlan_cipstart},//网络连接 tcp/udp 连接
    {{"CIPSSLSIZE"},atwlan_cipsslsize},
    {{"CIPSSLCCONF"},atwlan_cipsslcconf},
    {{"CIPSEND"},atwlan_cipsend}, //send normal
    {{"CIPSENDEX"},atwlan_cipsendex},//send ext
    {{"CIPSENDBUF"},atwlan_cipsendbuf},//send buff
    {{"CIPBUFRESET"},atwlan_cipbufreset},//send buff
    {{"CIPBUFSTATUS"},atwlan_cipbufstatus},//send buff
    {{"CIPCHECKSEQ"},atwlan_cipcheckseq},//send buff
    {{"CIPCLOSE"},atwlan_cipclose},//close 
    {{"CIFSR"},atwlan_cifsr},
    {{"CIPMUX"},atwlan_cipmux}, //设置单连接 AT+CIPMUX= 0； 多连接 AT+CIPMUX= 1
    {{"CIPSERVER"},atwlan_cipserver},
    {{"CIPSERVERMAXCONN"},atwlan_cipservermaxconn},
    {{"CIPMODE"},atwlan_cipmode},//设置非透传  AT+CIPMOD=0；设置透传 AT+CIPMOD=1
    {{"SAVETRANSLINK"},atwlan_savetranslink},//slink
    {{"CIPSTO"},atwlan_cipsto},
    {{"PING"},atwlan_ping},
#if CONFIG_NET_AT_PASSIVE_RECV
    {{"CIPRECVMODE"},atwlan_ciprecvmode},
    {{"CIPRECVDATA"},atwlan_ciprecvdata}, //TODO!
    {{"CIPRECVLEN"},atwlan_ciprecvlen},
#endif
    //{{"CIPSNTPCFG"},atwlan_cipsntpcfg},
    {{"CIPSNTPTIME"},atwlan_cipsntptime},
    {{"CIPDNS_CUR"},atwlan_cipdns_cur},
    //{{"CIPDNS_DEF"},atwlan_cipdns_def},
#if CONFIG_NET_AT_LIST_PKT_CNT
    {{"CIPPKTSTAT"},atwlan_cippktstat},
#endif
}; 
  
void at_network_init(void)
{
    at_add_cmd(&at_network_tbl[0], (sizeof(at_network_tbl)/sizeof(at_item_t)));    

    at_net_conns_init();
}
 
 
void at_network_deinit(void)
{
    at_net_conns_deinit();

    at_remove_cmd(&at_network_tbl[0], (sizeof(at_network_tbl)/sizeof(at_item_t)));        
}
#endif