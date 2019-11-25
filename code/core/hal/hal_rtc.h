#ifndef HAL_RTC_H
#define HAL_RTC_H



#include "time.h"



#define IS_RTC_ZONE(zone)                   ((zone) >= -11 && (zone) <= 13)

#define RTC_CLOCKSEL_I32K                      0
#define RTC_CLOCKSEL_NORMAL                      1
#define RTC_CLOCKSEL_EXT32K                      2   



#define ALARM_BY_ABSOLUTE_TIME                  BIT(0)
#define ALARM_BY_PASS_TIME                      BIT(1)
#define ALARM_BY_FIXEDTIME                      BIT(2)

#define ALRAM_MAX_NUMS                          12

#define BASE_YEAR                            (1900)
/*
    u16 year;    
    u8 month;    1-12
    u8 day;      1-31
    u8 hour;     0-23    
    u8 min;      0-59
    u8 sec;      0-59
    u8 week;     xx   
*/


typedef struct rtc_config_
{
    u8 clk_sel;
    int zone;
    system_time_t init_time;
}rtc_config_t;

typedef struct rtc_alarm_cb_
{
    u8  alarm_mode;
    u8  id;
    u8  hour;
    u8  min;
}rtc_alarm_cb_t;

typedef struct alarm_config_
{
    u8  alarm_mode;
    u8  rsvd;
    struct aram_time
    {   
        u8 enable;
        u8 rsvd;
        u8 hour;
        u8 min;
    }abs_time[ALRAM_MAX_NUMS];
	struct aram_fixed_time
    {   
        u8 day;
        u8 hour;
        u8 min;
        u8 sec;
    }fixed_time;
    u32 pass_time_min; 
    hal_cb_t      event;
}alarm_config_t;


typedef struct rtc_hdl_
{
    rtc_config_t   config;
    system_time_t  real_time;  
	alarm_config_t alarm;
}rtc_hdl_t;


hal_status_e s907x_hal_rtc_init(rtc_hdl_t *rtc);
hal_status_e s907x_hal_rtc_deinit(rtc_hdl_t *rtc);
void         s907x_hal_rtc_get_time(rtc_hdl_t *rtc);
void         s907x_hal_rtc_set_unixtime(rtc_hdl_t *rtc);
void         s907x_hal_rtc_set_basictime(rtc_hdl_t *rtc);
void         s907x_hal_rtc_alarm_init(rtc_hdl_t *rtc);
void         s907x_hal_rtc_alarm_deinit(rtc_hdl_t *rtc);
void         s907x_hal_rtc_alarm_cmd(rtc_hdl_t *rtc, u8 enable);
#endif
