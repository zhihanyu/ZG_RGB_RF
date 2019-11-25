#include "s907x.h"
#include "lwip/ip_addr.h"
#include "ping.h"
#include "ping_test.h"
  
#if M_AT_TEST

hal_test_name_map_t ping_test_map[] = 
{
	{1, "ping start"},
    {0, "ping stop"},
}; 
  
 
static int ping_para_argv(char *set, ping_argv_t *ping)
{
    char buf[32];
    char temp;
    int i = 0;
    int argv_cnt = 0;
    int ret = 0; 

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
            ping->test_no = atoi(buf);
            argv_cnt ++;
        } else if(argv_cnt == 1) {
            ipaddr_aton(buf, &ping->ip);
            argv_cnt ++;
        } else if(argv_cnt == 2) {
            ping->len = atoi(buf);
            argv_cnt ++;
        } else if(argv_cnt == 3) {
            ping->cnt = atoi(buf);
            argv_cnt ++;
        } else {
            break;
        }
        if(temp == 0) {
            break;
        }
    }   
    return ret;
}
   
//AT+PING=1,192.168.1.10,100,20,
//AT+PING=0
 
void ping_test(void *context)
{
    at_cmd_t *at = (at_cmd_t *)context;
    ping_argv_t ping;
    
    ASSERT(context);
    //set default
    ping.test_no = -1;
    ping.len = DEFAULT_PING_SIZE; 
    ping.cnt = portMAX_DELAY;
    ping_para_argv(at->set, &ping);

	switch(ping.test_no) 
	{
		case 1:    
						ping_init(&ping);
			break;
				case 0:
						ping_stop();
						break;
		default:

			break;
		}
}

#endif