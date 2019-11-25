#include "s907x.h"
#include "sntp_api.h"


bool sntp_setup(rtc_hdl_t *rtc, int zone, char *server1, char *server2, char *server3)
{

    ASSERT(server1 || server2 || server3);
    
    sntp_set_zone(rtc, zone);

	sntp_stop();

	sntp_set_server(0, server1);
	sntp_set_server(1, server2);
	sntp_set_server(2, server3);
    
	sntp_init();

	return true;
}

