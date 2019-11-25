#ifndef HAL_SLEEP_H
#define HAL_SLEEP_H

typedef enum
{
	pswake_up_by_wakepin,// 0
	pswake_up_by_gpio,// 1
	pswake_up_by_timer,// 2
	pswake_up_by_uart,// 3
	pswake_up_by_rtc,// 4
	pswake_up_by_wlan// 5
}ps_wake_up_mode_e;

typedef enum
{
	dswake_up_by_wakepin,
	dswake_up_by_timer
}dsleep_wake_up_mode_e;



hal_status_e s907x_hal_sleep_wake_config(int event,u32 val);
hal_status_e s907x_hal_sleep_event_clear(int event);
hal_status_e s907x_hal_sleep_enter(void);
hal_status_e s907x_hal_dsleep_wake_config(int event,u32 val);
hal_status_e s907x_hal_deepsleep_enter(void);
hal_status_e s907x_hal_wake_handle(void);



#endif

