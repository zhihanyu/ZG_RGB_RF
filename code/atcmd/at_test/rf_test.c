#include "s907x.h"
#include "rf_test.h"


#if M_AT_ENABLE

#if M_AT_TEST

static rf_t rf;
static timer_hdl_t tim_40m, tim_32k;


static void ir_recv_data(u8 zero)
{
    int pos  = rf.buf_id / RF_BIT_NUM;

    if(pos >= sizeof(rf.buf)) {
        return;
    }

    rf.buf[pos] = rf.buf[pos] << 1;    
    rf.buf[pos] += zero;

    rf.buf_id ++;
}

static void rf_ob_restart(void)
{
    s907x_hal_timer_stop(&tim_32k);
    rf.state = RF_STATE_IDLE;
    rf.buf_id = 0;
    rf.capture_stop = FALSE;
    wl_memset(rf.buf, 0, sizeof(rf.buf));
    rf.cmd_bits = 0;
}

static void rf_monitor_cb(void *ctx)
{
    if(rf.short_press){
      if(rf.repeat_temp != rf.repeat_cnt){
          rf.repeat_temp = rf.repeat_cnt;
      }else{
          rf.short_press = FALSE;
          rf.repeat_temp = rf.repeat_cnt = 0;
          rf_ob_restart();
      }
    }
}

static void rf_hsm(u16 temp)
{
    timer_hdl_t *tim = &tim_32k;
  
    u32 val = RF_DATA(temp);//tick = 1us
    
    switch(rf.state){
      
      case RF_STATE_IDLE:
        if(val > RF_IDLE_PRE_US - RF_IDLE_MAGIN_US && val < RF_IDLE_PRE_US + RF_IDLE_MAGIN_US){
            rf.state = RF_STATE_CMD;
        }else{
            rf_ob_restart();
        }
      break;
      
      case RF_STATE_CMD:
        s907x_hal_timer_start_base(tim);
        if(val > RF_CMD_TM_US - RF_IDLE_MAGIN_US && val < RF_CMD_TM_US + RF_IDLE_MAGIN_US){
            rf.cmd_bits++;
        }else{
            rf_ob_restart();
        }
        if(rf.cmd_bits >= RF_CMD_PAYLOAD_BIT_NUM){
          if(!rf.short_press){
              rf.short_press = TRUE;
          }else{
              rf.repeat_cnt++;
          }
          
          //rf.state = RF_STATE_REPEAT;  
          wl_send_sema_fromisr(&rf.cap_down);
        }
      break;
      
      case RF_STATE_REPEAT:
         //Z_DEBUG();
      break;
    }
    
    
}


void timer_32k_isr(void *context)
{
    timer_hdl_t *tim = (timer_hdl_t*)context;
    
    if(s907x_hal_gpio_read(GPIO7)) {
        ir_recv_data(1);
    } else {
        ir_recv_data(0);
    }
    s907x_hal_timer_stop(tim);

}

static void timer_capture_gpio7(void *context)
{
    timer_hdl_t *tim = context;
    static u16 cap_0;
    static u16 cap_1;
    u16 temp;
    
    temp = s907x_hal_timer_get_counter(tim);

    if(rf.capture_idx % 2 == 0) {
        cap_0 = s907x_hal_timer_get_counter(tim);
        if(rf.capture_idx > 1) {
                temp = ((cap_0 >= cap_1) ? (cap_0 - cap_1) : (0xFFFF - cap_1 + cap_0));
        }	
    } else {
        cap_1 = s907x_hal_timer_get_counter(tim);
        temp = ((cap_1 >= cap_0) ? (cap_1 - cap_0) : (0xFFFF - cap_0 + cap_1));
    }

    rf_hsm(temp);
    rf.capture_idx++;
	
}

static void rf_ob_init(void)
{
    s907x_hal_timer_stop(&tim_32k);
    rf.state = RF_STATE_IDLE;
    rf.buf_id = 0;
    rf.capture_stop = FALSE;
    wl_memset(rf.buf, 0, sizeof(rf.buf));
    rf.cmd_bits = 0;
    rf.capture_idx = 0;
    rf.short_press = FALSE;
    rf.repeat_cnt = 0;
    rf.repeat_temp = 0;
}

static void rf_bsp_init(void)
{
    wl_memset(&rf, 0, sizeof(rf));
    wl_init_sema(&rf.cap_down, 0, sema_binary);
    
    timer_hdl_t *tim = &tim_40m;
    s907x_hal_timer_stop(tim);
    
    tim->config.idx = TIM_CAP;
    tim->config.prescaler = 199;
    tim->config.period = 65535;
    tim->config.int_enable = 0;
	
    tim->it.basic_user_cb.func = NULL;
    tim->it.basic_user_cb.context = tim;	
	
    s907x_hal_timer_base_init(tim);
    s907x_hal_timer_start_base(tim);
    
    gpio_init_t init;
    init.mode = GPIO_MODE_INT_RISING;
    init.pull = GPIO_PULLDOWN;
	
    s907x_hal_gpio_init(GPIO7, &init);
    s907x_hal_gpio_it_start( GPIO7, timer_capture_gpio7, tim); 

    tim = &tim_32k;
    tim->config.idx = TIM3;
    tim->config.prescaler = 0; 
    tim->config.period = 26;
    tim->config.int_enable = 1;
    
    tim->it.basic_user_cb.func = timer_32k_isr;
    tim->it.basic_user_cb.context = tim;
    s907x_hal_timer_base_init(tim);
    
}


static void rf_test_task(void *context)
{
    int ret;
    rf_ob_init();
    rf_bsp_init();
    rf.monitor_timer = xTimerCreate("rf timer", 150, pdTRUE,(void *)0, rf_monitor_cb);
    xTimerStart(rf.monitor_timer, 0);
    
    while(1){
        ret = wl_wait_sema(&rf.cap_down,portMAX_DELAY);
        if(!ret){
           continue;
        }
        
        printf_arrary((u8*)rf.buf, sizeof(rf.buf), ARY_U8, 0);
        printf("repeat:%d\n\n",rf.repeat_cnt);
        
        rf_ob_restart();
    }
}

void rf_test(void *context)
{
    char  *argv[AT_SET_MAX_ARGC];
    int    argc;
    int    order;

    at_cmd_t *cmd = (at_cmd_t *)context;
    argc = at_get_param(argv, cmd->set);
    if(!argc) {
      return;
    }
    
    order = atoi(argv[0]);
    if(order){
        wl_create_thread("rf test thread", 1024, MAIN_TASK_PRIO, (thread_func_t)rf_test_task, NULL);
    }else{
        
    }
    
    wl_os_mdelay(100);
    if(order){
        USER_DBG("RF TEST START...\n");
    }else{
        USER_DBG("RF TEST STOP...\n");
    }
    
}
#endif





#endif



































