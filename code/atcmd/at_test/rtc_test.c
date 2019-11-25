#include "s907x.h"
#include "rtc_test.h"

#if M_AT_TEST

rtc_hdl_t rtc_hdl;


hal_test_name_map_t rtc_test_map[] = 
{
	{0, "rtc basic"},
	{1, "rtc alarm"},

};

static const char *week[7] = {
"sunday", "monday", "tuesday","wednesday","thursday","friday", "saturday"
}; 
       
//A   
void rtc_test_basic(hal_test_t *test)
{
	ASSERT(test);
    
    //get init time form SNTP
    //s907x hw only support 24hour mode 
    if(test->arg[0] == 1) {
        ASSERT(test->arg_cnt == 7);
		memset((void*)&rtc_hdl, 0, sizeof(rtc_hdl));
        //init clk_sel & zone
        rtc_hdl.config.clk_sel = RTC_CLOCKSEL_I32K;
        rtc_hdl.config.zone = 8;
        s907x_hal_rtc_init(&rtc_hdl);
        //set init_time use basic or unix time
        rtc_hdl.config.init_time.year = test->arg[1];
        rtc_hdl.config.init_time.month = test->arg[2];
        rtc_hdl.config.init_time.day = test->arg[3];
        rtc_hdl.config.init_time.hour = test->arg[4];
        rtc_hdl.config.init_time.min = test->arg[5];
        rtc_hdl.config.init_time.sec = test->arg[6];
        s907x_hal_rtc_set_basictime(&rtc_hdl);
        s907x_hal_rtc_get_time(&rtc_hdl);
    } else if(test->arg[0] == 0){
         s907x_hal_rtc_get_time(&rtc_hdl);
    } else if(test->arg[0] == 2) {//AT+RTC=0,2,8  sntp_time_get & set system_time
        ASSERT(test->arg_cnt == 2);
        memset((void*)&rtc_hdl, 0, sizeof(rtc_hdl));

        rtc_hdl.config.clk_sel = RTC_CLOCKSEL_NORMAL;
        rtc_hdl.config.zone = test->arg[1];
        if(sntp_setup(test->arg[1], "ntp1.aliyun.com", "ntp2.aliyun.com", "ntp3.aliyun.com")) {
            wl_os_mdelay(100);
            s907x_hal_rtc_get_time(&rtc_hdl);
        }
        else {
            HAL_TEST_DBG("sntp_setup fail\n");
        }
    }

   
    HAL_TEST_DBG("year %d month %d days = %d  week = %s hour = %d min %d sec %d\n", rtc_hdl.real_time.year, rtc_hdl.real_time.month, rtc_hdl.real_time.day, week[rtc_hdl.real_time.week], rtc_hdl.real_time.hour, rtc_hdl.real_time.min, rtc_hdl.real_time.sec);
}


void alarm_timeout_isr(void *context)
{
    rtc_alarm_cb_t *alarm_cb = context;
    rtc_hdl_t *rtc = &rtc_hdl;

    ASSERT(rtc);
    
    if(alarm_cb->alarm_mode & ALARM_BY_ABSOLUTE_TIME) {
        HAL_TEST_DBG("alarm group %d alram time %d:%d\n", alarm_cb->id, alarm_cb->hour, alarm_cb->min);
    }
    if(alarm_cb->alarm_mode & ALARM_BY_PASS_TIME) {
        HAL_TEST_DBG("%d mintus down!\n", rtc->alarm.pass_time_min);
    }
	if(alarm_cb->alarm_mode & ALARM_BY_FIXEDTIME) {
        HAL_TEST_DBG("fixed time down!\n", rtc->alarm.pass_time_min);
    }
    //can stop alram here
    //hal_rtc_alarm_cmd(rtc, DISABLE);
    s907x_hal_rtc_get_time(rtc);

    HAL_TEST_DBG("alrm end year %d month %d days = %d hour = %d min %d sec %d\n", rtc_hdl.real_time.year, rtc_hdl.real_time.month, rtc_hdl.real_time.day, rtc_hdl.real_time.hour, rtc_hdl.real_time.min, rtc_hdl.real_time.sec);  
}

//init  45s
//alarm + 5s -> every 50s alarm

void rtc_test_alarm(hal_test_t *test)
{
    rtc_hdl_t *rtc = &rtc_hdl;
    u8 day,hour, min,sec;
    u8 id;

	ASSERT(test);
    //alarm by day,hour,min,sec
    rtc->alarm.alarm_mode = BIT(test->arg[0]);
    
    if(test->arg_cnt < 2) {
        return;
    }
    if (rtc->alarm.alarm_mode & ALARM_BY_ABSOLUTE_TIME) {
        if(test->arg_cnt < 4) {
            return;
        }
        id   = test->arg[1];
        hour = test->arg[2];
        min  = test->arg[3];

        if(id >= ALRAM_MAX_NUMS) {
            id = ALRAM_MAX_NUMS - 1;
        }   
        if(hour > 23) {
            hour = 23;
        }
        if(min > 59) {
            min = 59;
        }  
        rtc_hdl.alarm.abs_time[id].enable = TRUE;
        rtc_hdl.alarm.abs_time[id].hour = hour;
        rtc_hdl.alarm.abs_time[id].min  = min;         
    }
    if (rtc->alarm.alarm_mode & ALARM_BY_PASS_TIME) {
        rtc_hdl.alarm.pass_time_min = test->arg[1];
    }
	if(rtc->alarm.alarm_mode & ALARM_BY_FIXEDTIME)
	{
		if(test->arg_cnt < 5) {
            return;
        }
        day   = test->arg[1];
        hour = test->arg[2];
        min  = test->arg[3];
		sec = test->arg[4];
		
        if(day > 200) {
            day = 200;
        } 
        if(hour > 23) {
            hour = 23;
        }
        if(min > 59) {
            min = 59;
        }  
		if(sec > 59) {
            sec = 59;
        } 

        rtc_hdl.alarm.fixed_time.day = day;
        rtc_hdl.alarm.fixed_time.hour = hour;
        rtc_hdl.alarm.fixed_time.min  = min;  
		rtc_hdl.alarm.fixed_time.sec  = sec; 
		
	}


    rtc->alarm.event.func = alarm_timeout_isr;
    rtc->alarm.event.context = rtc;

    s907x_hal_rtc_alarm_init(rtc);
    s907x_hal_rtc_alarm_cmd(rtc, ENABLE);
    s907x_hal_rtc_get_time(rtc);
    HAL_TEST_DBG("alrm start year %d month %d days = %d hour = %d min %d sec %d\n", rtc_hdl.real_time.year, rtc_hdl.real_time.month, rtc_hdl.real_time.day, rtc_hdl.real_time.hour, rtc_hdl.real_time.min, rtc_hdl.real_time.sec);
}

void rtc_test(hal_test_t *test)
{
	ASSERT(test);
	ASSERT(test->no < (sizeof(rtc_test_map)/sizeof(hal_test_name_map_t)));

	HAL_TEST_DBG("test no %d = %s\n",test->no, rtc_test_map[test->no].name);
       
	switch(test->no) 
	{
		case RTC_TEST_BASIC:    
            rtc_test_basic(test);
			break;
		case RTC_TEST_ALARM:
            rtc_test_alarm(test);
			break;
		default:

			break;
	}
}

#endif