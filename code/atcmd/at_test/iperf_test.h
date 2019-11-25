#ifndef IPER_TEST_H
#define IPER_TEST_H




#define IPERF_RAW_TCPSERVER      1
#define IPERF_RAW_TCPCLIENT      2  
#define IPERF_SOCKET_UDPSERVER   3
#define IPERF_SOCKET_UDPCLIENT   4


#define IPERF_TEST_NO_STOP    0





typedef struct  iperf_argv_
{
    int test_no;
    ip_addr_t ip;
    u16        port; 
    int        time_s; //sec
}iperf_argv_t;








void iperf_test(void *context);
int  ipref_udp_rx_test(int port);
int  ipref_udp_tx_test(iperf_argv_t *argv);



#endif