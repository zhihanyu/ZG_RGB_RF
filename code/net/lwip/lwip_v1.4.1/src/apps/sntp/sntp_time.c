#include "s907x.h"
#include "sntp.h"
#include "sntp_time.h"



static char rtc_asic[64];
static rtc_hdl_t *p_current_rtc = NULL;

static const char week_item[7][4] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
static const char month_item[12][4] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
}; 

 
//sntp set rtc tick
void sntp_set_system_time(time_t GMT_Time)
{
    //must set zone before
    if(!p_current_rtc) {
        return;
    }
    s907x_hal_rtc_init(p_current_rtc);
 
    p_current_rtc->config.init_time.hw_time = GMT_Time + p_current_rtc->config.zone*60*60;
    s907x_hal_rtc_set_unixtime(p_current_rtc);
} 

//sntp change zone
void sntp_set_zone(rtc_hdl_t *rtc, int zone)
{
    ASSERT(IS_RTC_ZONE(zone));
    
    rtc->config.zone = zone;
    p_current_rtc = rtc;
}

int sntp_get_zone(void)
{
	return p_current_rtc->config.zone;
}



//esp at format respone
char* sntp_get_realtime(rtc_hdl_t *rtc)
{
    s907x_hal_rtc_get_time(rtc);
    sprintf(rtc_asic, "%s %s %02d %02d:%02d:%02d %02d\n",
                       week_item[rtc->real_time.week],
                       month_item[rtc->real_time.month],
                       rtc->real_time.day, rtc->real_time.hour, rtc->real_time.min, rtc->real_time.sec, rtc->real_time.year);  

    return rtc_asic;
}


void sntp_set_server(int id, const char* name_or_ip)
{
	if (name_or_ip){
		sntp_setservername(id, (char*)name_or_ip);
	}
}


