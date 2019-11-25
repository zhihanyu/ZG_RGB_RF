#ifndef RF_TEST_H
#define RF_TEST_H
#include "timers.h"

#define RF_DATA(x)		(x*5ul)
#define RF_CMD_PAYLOAD_BIT_NUM  24

#define RF_STATE_IDLE		0
#define RF_STATE_CMD		1
#define RF_STATE_REPEAT		2

#define RF_BIT_NUM              8
#define RF_IDLE_MAGIN_US        800                     
#define RF_IDLE_PRE_US          13150 	                //13.15ms
#define RF_CMD_TM_US 	        1630                    //1.63ms



typedef struct rf_st_
{
    int capture_stop;  
    u8  buf[3];
    u8  buf_id;
    u16 capture_idx;
    u16 state;
    u32 cmd_bits;
    u8 short_press;
    u32 repeat_cnt;
    u32 repeat_temp;
    sema_t cap_down;
    TimerHandle_t monitor_timer;
    
}rf_t;



void rf_test(void *context);











#endif