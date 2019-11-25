#include "s907x.h"
#include "lwip/ip_addr.h"
#include "iperf_test.h"
#include "lwip/sockets.h"

#if M_AT_TEST
//#define IPREF_TEST_SRV_ADDR "192.168.1.104"
#define IPREF_TEST_SRV_ADDR "192.168.2.2"
//#define IPREF_TEST_SRV_ADDR "192.168.9.175"
//#define IPREF_TEST_SRV_ADDR "192.168.1.2"

#define UDP_BUF_SIZE (1024)//(44*1472)

 
static int udp_rx_thread_start = 0;
static int udp_tx_thread_start = 0;
static int server_port;

/*udp tx test*/

static int udp_rx_test(int port)
{
    int ret;
    char *rx_buf = wl_zmalloc(UDP_BUF_SIZE);
    struct sockaddr_in dest_addr;
    int socket_hdl;
	struct sockaddr_in client_address;
	socklen_t addr_len = sizeof(client_address);
    
    if(!rx_buf) {
        HAL_TEST_DBG("ipref_udp_rx_test malloc failed\n",__FUNCTION__, __LINE__);
        ret =  HAL_ERROR;
        goto exit;
    }
    socket_hdl = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socket_hdl < 0) {
		HAL_TEST_DBG("%s %d->Error in socket_hdl()\n",__FUNCTION__, __LINE__);
        ret =  HAL_ERROR;
        goto exit;
	}
    dest_addr.sin_len = sizeof(dest_addr);
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	dest_addr.sin_port = htons(port);
	ret = bind(socket_hdl, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
	if (ret == -1) {
		HAL_TEST_DBG("%s %d->Error in bind()\n",__FUNCTION__,__LINE__);
        ret =  HAL_ERROR;
        goto exit;
	}

    while(1) {
        ret = recvfrom(socket_hdl, rx_buf, UDP_BUF_SIZE, 0, (struct sockaddr *) &client_address, &addr_len);
        if(ret <= 0) {
             break;
        }
    }
exit:
    if(socket_hdl >= 0) {
        close(socket_hdl);
    }
    if(rx_buf)
        wl_free(rx_buf);
    return ret;
}
 
static void iperf_udp_rx_thread(void *context)
{

    int port = (*(int *)context);

    udp_rx_thread_start = 1;

    udp_rx_test(port);

    HAL_TEST_DBG("udp rx stopped\n");
    udp_rx_thread_start = 0;
    wl_destory_threadself();

}

int ipref_udp_rx_test(int port)
{
    server_port = port;

    if(udp_rx_thread_start) {
        HAL_TEST_DBG("udp rx is rnnung\n");
        return 0;
    }
    
    wl_create_thread("iperf udp rx task", IPERF_UDP_TASK_STACK_SZ, IPERF_UDP_TASK_PRIO,  iperf_udp_rx_thread, (void *)(&server_port));

    return 0;
}

/*udp tx test*/

static int udp_tx_test(void *context)
{
    int ret;
    char *tx_buf = (char *)wl_zmalloc(UDP_BUF_SIZE);
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int socket_hdl;
    int written;
    int total;
    int fail = FALSE;
    int start = 0;
    int pass = 0;
    
    iperf_argv_t *argv = (iperf_argv_t *)context;
    if(!tx_buf || !argv) {
        HAL_TEST_DBG("ipref_udp_rx_test malloc failed\n",__FUNCTION__, __LINE__);
        ret =  HAL_ERROR;
        goto exit;
    }
    socket_hdl = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socket_hdl < 0) {
		HAL_TEST_DBG("%s %d->Error in socket_hdl()\n",__FUNCTION__, __LINE__);
        ret =  HAL_ERROR;
        goto exit;
	}
    //bind client addr
    memset((uint8_t *)&client_addr, 0, sizeof(client_addr));
    client_addr.sin_len = sizeof(client_addr);
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(2008);

	ret = bind(socket_hdl, (struct sockaddr *)&client_addr, sizeof(client_addr));
	if (ret == -1) {
		HAL_TEST_DBG("%s %d->Error in bind()\n",__FUNCTION__,__LINE__);
        ret =  HAL_ERROR;
        goto exit;
	}   
    //set server addr
    memset((uint8_t *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_len = sizeof(server_addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(argv->port);
    server_addr.sin_addr.s_addr =  argv->ip.addr;


    start = wl_get_systemtick();
    
    while(!fail)
    {

		written = 0;
        total = UDP_BUF_SIZE;
        fail  = FALSE;
 
        pass = wl_systemtick_to_ms(wl_get_systemtick() - start) / 1000;
        if(pass > argv->time_s) {
            ret = HAL_OK;
            break;
        }
        while(1) 
        {
            ret = sendto(socket_hdl, tx_buf + written, total - written, 0, (struct sockaddr const*)&server_addr, sizeof(server_addr));
            if(ret > 0) {
                written += ret;
                if(written >= total) { 
                    break;
                }
            }
            else if(ret == 0){
                fail = TRUE;
                break;
            }
          
     
        }
    }

exit:
    if(socket_hdl >= 0) {
        close(socket_hdl);
    }
    if(tx_buf)
        wl_free(tx_buf);
    return ret;
}


static void iperf_udp_tx_thread(void *context)
{

    udp_tx_thread_start = 1;
    udp_tx_test(context);
    HAL_TEST_DBG("udp tx stopped\n");
    udp_tx_thread_start = 0;
    wl_destory_threadself();

}


int ipref_udp_tx_test(iperf_argv_t *argv)
{
    static iperf_argv_t arg;

    if(!argv) {
        HAL_TEST_DBG("udp tx argv error\n");
        return HAL_ERROR;
    }
    if(udp_tx_thread_start) {
        HAL_TEST_DBG("udp tx is rnnung\n");
        return 0;
    }
    memcpy(&arg, argv, sizeof(iperf_argv_t));

    wl_create_thread("iperf udp tx task", IPERF_UDP_TASK_STACK_SZ, IPERF_UDP_TASK_PRIO,  iperf_udp_tx_thread, (void *)(&arg));

    return 0;

}


#endif
