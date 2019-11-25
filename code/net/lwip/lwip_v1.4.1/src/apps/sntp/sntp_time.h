#ifndef SNTP_TIME_H_
#define SNTP_TIME_H_


#include "sntp.h"
#include <stdbool.h>

void  sntp_set_server(int id, const char* name_or_ip);
void  sntp_set_system_time(time_t GMT_Time);
char* sntp_get_realtime(rtc_hdl_t *rtc);
void  sntp_set_zone(rtc_hdl_t *rtc, int zone);
int   sntp_get_zone(void);







#endif /* SNTP_TIME_H_ */
