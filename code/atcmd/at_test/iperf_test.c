#include "s907x.h"
#include "lwip/ip_addr.h"
#include "iperf_test.h"
#include "lwip/apps/lwiperf.h"


#if M_AT_TEST

static int iperf_para_argv(char *set, iperf_argv_t *iperf)
{
    char buf[32];
    char temp;
    int i = 0;
    int argv_cnt = 0;


    while(1)
    {
        temp = *set++;
        if(temp != 0 && temp != ',') {
          buf[i++] = temp;
          continue;
        } 
        buf[i] = 0; 
        i = 0;
        //clear index
        if(argv_cnt == 0) {
            iperf->test_no = atoi(buf);
            argv_cnt ++;
        } else if(argv_cnt == 1) {
            if(iperf->test_no == IPERF_RAW_TCPCLIENT || iperf->test_no == IPERF_SOCKET_UDPCLIENT) {
                ipaddr_aton(buf, &iperf->ip);
            } else if(iperf->test_no == IPERF_RAW_TCPSERVER || iperf->test_no == IPERF_SOCKET_UDPSERVER){
                iperf->port = atoi(buf);   
            }
            argv_cnt ++;
        } else if(argv_cnt == 2) {
             if(iperf->test_no == IPERF_RAW_TCPCLIENT || iperf->test_no == IPERF_SOCKET_UDPCLIENT) {
                iperf->port = atoi(buf);   
            }
            argv_cnt ++;
        } else if(argv_cnt == 3) {
             if(iperf->test_no == IPERF_RAW_TCPCLIENT || iperf->test_no == IPERF_SOCKET_UDPCLIENT) {
                iperf->time_s = atoi(buf);   
            }
            argv_cnt ++;
        } else {
            break;
        }
        if(temp == 0) {
            break;
        }
    }   
    return argv_cnt;
}


static void lwiperf_report(void *arg, enum lwiperf_report_type report_type,
  const ip_addr_t* local_addr, u16_t local_port, const ip_addr_t* remote_addr, u16_t remote_port,
  u32_t bytes_transferred, u32_t ms_duration, u32_t bandwidth_kbitpsec)
{
    char *reason;
 
    switch(report_type) 
    {
        /** The server side test is done */
        case LWIPERF_TCP_DONE_SERVER:
        reason = "server down!";    
        break;
        /** The client side test is done */
        case LWIPERF_TCP_DONE_CLIENT:
        reason = "client down!";
        break;
        /** Local error lead to test abort */
        case LWIPERF_TCP_ABORTED_LOCAL:
        reason = "tcp aborted local!";
        break;
        /** Data check error lead to test abort */
        case LWIPERF_TCP_ABORTED_LOCAL_DATAERROR:
        reason = "tcp aborted local data error!";
        break;
        /** Transmit error lead to test abort */
        case LWIPERF_TCP_ABORTED_LOCAL_TXERROR:
        reason = "tcp aborted local txerror error!";
        break;
        /** Remote side aborted the test */
        case LWIPERF_TCP_ABORTED_REMOTE:
		reason = "tcp aborted remote!";
        break;
    }
    printf("iperf test down reason = %s \n", reason);
    printf("local addr = %x local_port = %d\n", local_addr->addr, local_port);
    printf("remote addr = %x remote_port = %d\n", remote_addr->addr, remote_port);
    printf("bytes transferred = %d duration = %d bandwidth_kbitpsec = %d\n", bytes_transferred, ms_duration, bandwidth_kbitpsec);
}

//AT+IPERF=1,5001                //server mode  port
//AT+IPERF=2,192.168.1.2,5001    //client mode  ip,port  
//AT+IPERF=0                     //stop 
void iperf_test(void *context)
{
    at_cmd_t *at = (at_cmd_t *)context;
    iperf_argv_t iperf;
    int cnt;
    void *iperf_section;
    ASSERT(context);
    //set default
    iperf.test_no = -1;
    iperf.time_s = 10;
    iperf.port = 5001;
    cnt = iperf_para_argv(at->set, &iperf);
  
	switch(iperf.test_no) 
	{
		case IPERF_RAW_TCPSERVER:    
            if(cnt == 1) {
                iperf_section = lwiperf_start_tcp_server_default(lwiperf_report, NULL);
            } else if(cnt == 2){
                iperf_section = lwiperf_start_tcp_server(IP_ADDR_ANY, iperf.port, lwiperf_report, NULL);
            }
			break;
		case IPERF_RAW_TCPCLIENT:    
            iperf_section = lwiperf_start_tcp_client(&iperf.ip, iperf.port, LWIPERF_CLIENT, iperf.time_s, lwiperf_report, NULL);
			break;
        case IPERF_SOCKET_UDPSERVER:
            ipref_udp_rx_test(iperf.port);
            break;
        case IPERF_SOCKET_UDPCLIENT:
            ipref_udp_tx_test(&iperf);     
            break;

		default:

			break;
	}
}

#endif