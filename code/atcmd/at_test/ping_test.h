#ifndef PING_TEST_H
#define PING_TEST_H



#include "lwip/ip_addr.h"

#define DEFAULT_PING_SIZE   256
















typedef struct  ping_argv_
{
    int test_no;
    ip_addr_t ip;
    u32 len;
    u32 cnt;
}ping_argv_t;



void ping_test(void *context);





#endif