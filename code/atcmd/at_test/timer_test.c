#include "s907x.h"
#include "timer_test.h"

#if M_AT_TEST

timer_hdl_t timx_hdl;
static TIM_PWM_CTR tim_pwm_shadow = {0};
static int PWM_CHANGE_MARK = 0;


u8  newstarttest  = 0;
u16      time_cnt = 0;
u32 	 runtime_cnt = 0;
u8 master_tim_sel  = 0;
u32 totalDatas = 0;
u32 hundred_million = 0;
u32 pwm_dma_buffer1;
u32 pwm_dma_buffer2;
u32 pwm_dma_default;
u32 pwm_dma_buffer;
//dma use 32bits
//static  u32 pwm_dma_buffer[] = {
//	0x0000001,
//	0x0000002,
//	0x0000003,
//	0x0000004,
//	0x0000005,
//	0x0000006,
//	0x0000007,	
//	0x0000008,
//	0x0000009,
//	0x0000009,
//	 
//	0x0000009,
//	0x0000009, 
//	0x0000008,
//	0x0000007,
//	0x0000006,
//	0x0000005,
//	0x0000004,
//	0x0000003, 
//	0x0000002,
//	0x0000001,
//};   
     
u32 capture_dma_buffer[20];
 
void s907x_hal_time_basic_msp_init(void *tim) 
{



}

void s907x_hal_time_basic_msp_deinit(void *tim) 
{



}
void s907x_hal_time_pwm_msp_init(void *tim, u8 channel) 
{
	switch(channel)
	{
		case 0:
			//PWM0_SEL0(HAL_ON);
			PWM0_SEL1(HAL_ON); 
			//PWM0_SEL2(HAL_ON); 
			// PWM0_SEL4(HAL_ON); 
		break;
		case 1:
			//PWM1_SEL0(HAL_ON);
			//PWM1_SEL1(HAL_ON); 
			PWM1_SEL2(HAL_ON); 
			//PWM1_SEL3(HAL_ON);  
			//PWM1_SEL4(HAL_ON); 
		break;
		case 2:
			PWM2_SEL0(HAL_ON);
			//PWM2_SEL1(HAL_ON); 
			// PWM2_SEL2(HAL_ON);
		break;
		case 3:
			PWM3_SEL0(HAL_ON);
			//PWM3_SEL2(HAL_ON);
		break;
		case 4:
			//PWM4_SEL1(HAL_ON); 
			PWM4_SEL2(HAL_ON);
			//PWM4_SEL3(HAL_ON); 
		break;
		case 5:
			PWM5_SEL2(HAL_ON);
			//PWM5_SEL3(HAL_ON);   
		break;
		#if 0
		case 6:
			PWM6_SEL1(HAL_ON);
			//PWM6_SEL2(HAL_ON);  
		break;
		case 7:
			PWM7_SEL1(HAL_ON);
			//PWM7_SEL2(HAL_ON); 
		break;
		#endif
	}
}

void s907x_hal_time_pwm_msp_deinit(void *tim, u8 channel) 
{

	switch(channel)
	{
		case 0:
			//PWM0_SEL0(HAL_ON);
			PWM0_SEL1(HAL_ON); 
			//PWM0_SEL2(HAL_ON); 
			// PWM0_SEL4(HAL_ON); 
		break;
		case 1:
			//PWM1_SEL0(HAL_ON);
			//PWM1_SEL1(HAL_ON); 
			PWM1_SEL2(HAL_ON); 
			//PWM1_SEL3(HAL_ON);  
			//PWM1_SEL4(HAL_ON); 
		break;
		case 2:
			PWM2_SEL0(HAL_ON);
			//PWM2_SEL1(HAL_ON); 
			// PWM2_SEL2(HAL_ON);
		break;
		case 3:
			PWM3_SEL0(HAL_ON);
			//PWM3_SEL2(HAL_ON);
		break;
		case 4:
			//PWM4_SEL1(HAL_ON); 
			PWM4_SEL2(HAL_ON);
			//PWM4_SEL3(HAL_ON); 
		break;
		case 5:
			PWM5_SEL2(HAL_ON);
			//PWM5_SEL3(HAL_ON);   
		break;
		#if 0
		case 6:
			PWM6_SEL1(HAL_ON);
			//PWM6_SEL2(HAL_ON);  
		break;
		case 7:
			PWM7_SEL1(HAL_ON);
			//PWM7_SEL2(HAL_ON); 
		break;
		#endif
	}
}


static uint8_t TO_hex(int value)
{

	uint8_t hex = 0;
	uint8_t hex_H = 0;
	uint8_t hex_L = 0;


	hex_L = value%10;
	hex_H = value/10%10;

    hex = (hex_H << 4) | hex_L;
	
	HAL_TEST_DBG("hex = %x \r\n",hex);

	return hex;
}

void s907x_hal_time_capture_msp_init(void *tim, u8 channel) 
{
	if(channel == CAPTURE_CHANNEL_0) {
        TIM_CAP_SEL0(HAL_ON);
    }
}

void s907x_hal_time_capture_msp_deinit(void *tim, u8 channel) 
{
	if(channel == CAPTURE_CHANNEL_0) {
        TIM_CAP_SEL0(HAL_OFF);
    }
}


void timer_basic_interrupt(void *context)
{
	timer_hdl_t* tim = (timer_hdl_t*) context;

	HAL_TEST_DBG("timer_basic_interrupt ISR %d tick = %d\n", tim->config.idx, wl_get_systemtick());
}


void timer_pwm_dma_cb(void *context)
{
	timer_dma_context_t *timer_dma_context = (timer_dma_context_t*)context;
	timer_hdl_t *ptimer = (timer_hdl_t*)timer_dma_context->timer;
	u32 channel = timer_dma_context->channel;

	//printf("channel:%d\n",channel);
        
        if(tim_pwm_shadow.TIM_VALID){
          if(tim_pwm_shadow.CHN_CONFIG.IS_CHN_ENABLE[channel]){
            if(tim_pwm_shadow.CHN_CONFIG.IS_DMA_BUF_CHANGE[channel]){
              if(channel == tim_pwm_shadow.CHN_CONFIG.CHANNEL[channel]){
                  tim_pwm_shadow.CHN_CONFIG.IS_DMA_BUF_CHANGE[channel] = FALSE;
                  //printf("channel:%d\n",channel);
                  tim_pwm_shadow.CHN_CONFIG.DMA_BUF[channel] = (u32)tim_pwm_shadow.CHN_CONFIG.PUSLE[channel] * 1.0 /100 * ptimer->config.period;
                  ptimer->dma.txbuf[channel] = (u8*)&tim_pwm_shadow.CHN_CONFIG.DMA_BUF[channel];
                  //printf("%x\n", tim->dma.txbuf[channel]);
                  ptimer->dma.txlen[channel] = sizeof(ptimer->dma.txbuf[channel]); 
                  s907x_hal_timer_pwm_dma_reload(ptimer,channel);
              }
            }
          }
        }
}

void timer_capture_dma_cb(void *context)
{
	HAL_DBG_ARRARY((u8*)capture_dma_buffer, sizeof(capture_dma_buffer), ARY_U32, 1);
    s907x_hal_timer_stop_capture_dma(context);
}


void timer_basic_test(hal_test_t *test)
{
    timer_hdl_t *tim = &timx_hdl;
    test->result = FALSE;
    tim->config.idx = test->arg[0];
	u16 period = test->arg[2];
    ASSERT(IS_BASIC_TIMER(test->arg[0]));

    if(test->arg[1] == 0) {
        s907x_hal_timer_stop(tim);
		s907x_hal_timer_base_deinit(tim);
        test->result = TRUE;
        return;
    } else if(test->arg[1] == 1){
       
        HAL_TEST_DBG("%s TimerIdx:%d \n", __func__, tim->config.idx);
        s907x_hal_timer_stop(tim);

        tim->config.prescaler = 0x0;
        tim->config.period = 40000; //1khz 
        tim->config.int_enable = 1;
		//set user callback
        tim->it.basic_user_cb.func = timer_basic_interrupt;
        tim->it.basic_user_cb.context = tim;

        s907x_hal_timer_base_init(tim);
        s907x_hal_timer_start_base(tim);
	}else if(test->arg[1] == 2){
		s907x_hal_timer_stop(tim);
		s907x_hal_timer_set_period(tim, period);
		s907x_hal_timer_start_base(tim);
	}
}
 

void timer_tick_test(hal_test_t *test)
{
    u32 delay = test->arg[0];/* ms */
	u32 CTRL[2];
	u32 LOAD[2];
	u32 VAL[2];
	u32 CALIB[2];
	u32 TICK[2];
	
	CTRL[0] = SysTick->CTRL;
	LOAD[0] = SysTick->LOAD;
	VAL[0] = SysTick->VAL;
	CALIB[0] = SysTick->CALIB;
	TICK[0] = wl_get_systemtick();
	
	wl_hal_udelay(delay * 1000);

	CTRL[1] = SysTick->CTRL;
	LOAD[1] = SysTick->LOAD;
	VAL[1] = SysTick->VAL;
	CALIB[1] = SysTick->CALIB;
	TICK[1] = wl_get_systemtick();
	
	HAL_TEST_DBG("systick CTRL: %x, LOAD:%x VAL:%x CALIB:%x tick: %x\n", CTRL[0], LOAD[0], VAL[0], CALIB[0], TICK[0]);
	HAL_TEST_DBG("systick CTRL: %x, LOAD:%x VAL:%x CALIB:%x tick: %x\n", CTRL[1], LOAD[1], VAL[1], CALIB[1], TICK[1]);

	if((TICK[1]- TICK[0]- delay )<100) {
        test->result = TRUE;
    } else {
        test->result = FALSE;
    }
    HAL_TEST_DBG("result = %d\n", test->result);

}
void timer_systimer_test(hal_test_t *test)
{

	
	//system timer TIM0
}


void timer_delay_test(hal_test_t *test)
{
    u32 start, pass;

    int loop = test->arg[0];

    while(loop--) {
   
        start = wl_get_systemtick();
        //set gpio high
        wl_hal_udelay(5000);
        
        pass = wl_get_systemtick() - start;
        
        HAL_TEST_DBG("pass time :%d \n", wl_systemtick_to_ms(pass));

        wl_hal_udelay(5000);
        //set gpio low
    }

}


static int pwm_normal_init(timer_hdl_t *ptimer, TIM_PWM_CTR *shadow)
{
	ASSERT(ptimer || shadow->TIM_VALID)
	int ret = HAL_ERROR;
	int i;
	u32 ccr = ptimer->config.period/2+1;//defaulr ccr
	for(i = 0; i < PWM_CHANNEL_MAX; i++){
		//HAL_TEST_DBG("%d, %d, %d\n", shadow->CHN_CONFIG.IS_CHN_ENABLE[i], shadow->CHN_CONFIG.CHANNEL[i], shadow->CHN_CONFIG.PUSLE[i]);
		if(shadow->CHN_CONFIG.IS_CHN_ENABLE[i]){
			ret = s907x_hal_timer_pwm_init(ptimer, shadow->CHN_CONFIG.POL[i], ccr, shadow->CHN_CONFIG.CHANNEL[i]);
			if(HAL_OK != ret){
				goto exit;
			}
			s907x_hal_timer_start_pwm(ptimer, shadow->CHN_CONFIG.CHANNEL[i]);
		}
	}
	
exit:
	return ret;
}


//AT+TIMER=4,0,frek,pol,chn_nums,[...] 
//AT+TIMER=4,1,frek,pol,chn,pusle
//AT+TIMER=4,2,frek,pol,chn,on/off


//AT+TIMER=4,0,6,0,2,2,7  			(channel 2,7 set pol = 0, default pusle: 50)
//AT+TIMER=4,1,6,0,2,70   			(set channel 2 pusle: 70)
//AT+TIMER=4,1,6,0,7,70   			(set channel 2 pusle: 70)
//AT+TIMER=4,2,6,0,2,0	  			(stop channel 2)
//AT+TIMER=4,2,6,0,7,0	  			(stop channel 7)
//AT+TIMER=4,2,6,0,2,1	  			(start channel 2)
//AT+TIMER=4,2,6,0,7,1	  			(start channel 7)
//AT+TIMER=4,3,x,x,0	  			(pwm disable)
//AT+TIMER=4,3,x,x,1	  			(pwm enable)


//AT+TIMER=4,0,6,0,1,2				(channe 2 pol = 0, default pusle:50)		
//AT+TIMER=4,0,6,1,1,7				(channe 7 pol = 1, default pusle:50)	
//AT+TIMER=4,1,6,0,2,70				(channe 2 pol = 0, set pusle: 70)
//AT+TIMER=4,1,6,1,7,70				(channe 7 pol = 1, set pusle: 70)
//AT+TIMER=4,2,6,0,2,0				(stop channel 2)
//AT+TIMER=4,2,6,1,7,0				(stop channel 7)
//AT+TIMER=4,2,6,0,2,1				(start channel 2)
//AT+TIMER=4,2,6,1,7,1				(start channel 7)
//AT+TIMER=4,3,x,x,0	  			(pwm disable)
//AT+TIMER=4,3,x,x,1	  			(pwm enable)


void timer_pwm_test(hal_test_t *test)
{
	ASSERT(test);
	test->result = TRUE;
	timer_hdl_t *ptimer = &timx_hdl;
	int cmd;
	int pol;
	int i;
	int cnt;
	int num;
	int chn;
	u32 pusle;
	float pusle_k;
	int temp;
	u32 freq_k;
	
	freq_k = test->arg[1];
	if(cmd != 3){
	 ASSERT(freq_k > 0 && freq_k <= HZ_40M);
	}
	temp = HZ_40M / freq_k;
	cmd = test->arg[0];
	pol = test->arg[2];
	cnt = test->arg[3];
	s907x_hal_pinmux_swd_off();

	if(!tim_pwm_shadow.TIM_VALID){
		//time base
		ptimer->config.idx = TIM_PWM;
		ptimer->config.prescaler = PRESCALER_SET;
		ptimer->config.period = temp / (PRESCALER_SET + 1);
		
		//basic user callback
		ptimer->it.basic_user_cb.func = NULL;
		ptimer->it.basic_user_cb.context = NULL;
			
		if(HAL_OK == s907x_hal_timer_base_init(ptimer)){
			tim_pwm_shadow.TIM_VALID = TRUE;
			s907x_hal_timer_start(ptimer);
		}
	}else{
		LINE_TAG();
	}
		
	switch(cmd){
		//normal pwm channel enable
		case 0:
			if(!tim_pwm_shadow.TIM_VALID){
				test->result = FALSE;
				goto exit;
			}
			for(i = 0; i < cnt; i++){
				ASSERT(test->arg[i+4] >= 0 && test->arg[i+4] < PWM_CHANNEL_MAX);
				tim_pwm_shadow.CHN_CONFIG.CHANNEL[test->arg[i+4]] = test->arg[i+4];
				tim_pwm_shadow.CHN_CONFIG.PUSLE[test->arg[i+4]] = 50;//default pusle
				//default pol
				tim_pwm_shadow.CHN_CONFIG.POL[test->arg[i+4]] = pol;
				tim_pwm_shadow.CHN_CONFIG.IS_CHN_ENABLE[test->arg[i+4]] = TRUE;
			}
			
			if(HAL_OK != pwm_normal_init(ptimer, &tim_pwm_shadow)){
				test->result = FALSE;
			}
		break;
		//normal pwm channel set pusle			
		case 1:
			num = test->arg[4]; 
			chn = test->arg[3];
			ASSERT(num > 0 && num < 100);
			ASSERT(chn >= 0 && chn < 8);
			pusle_k = num * 1.0 / 100.0;
			pusle = (int)(ptimer->config.period * pusle_k) + 1;
			if(tim_pwm_shadow.TIM_VALID){
				for(i = 0; i < PWM_CHANNEL_MAX; i++){
					if(tim_pwm_shadow.CHN_CONFIG.IS_CHN_ENABLE[i]){
						if(chn == tim_pwm_shadow.CHN_CONFIG.CHANNEL[i]){
							//HAL_TEST_DBG("%d, %d\n", pusle, ptimer->config.period); 
							tim_pwm_shadow.CHN_CONFIG.PUSLE[chn] = num;
							if(HAL_OK != s907x_hal_timer_pwm_set_ccr(ptimer,  pusle, chn)){
								break;
							}else{
								goto exit;
							}
						}
					}
				}
			}
			test->result = FALSE;
			
		break;
		//normal pwm channel disable
		case 2:
			if(test->arg[4]){
				s907x_hal_timer_start_pwm(ptimer, test->arg[3]);
				tim_pwm_shadow.CHN_CONFIG.IS_CHN_ENABLE[test->arg[3]] = TRUE;
				tim_pwm_shadow.CHN_CONFIG.CHANNEL[test->arg[3]] = test->arg[3];
			
			}else{
				for(i = 0; i < PWM_CHANNEL_MAX; i++){
					if(tim_pwm_shadow.CHN_CONFIG.IS_CHN_ENABLE[i]){
						if(tim_pwm_shadow.CHN_CONFIG.CHANNEL[i] == test->arg[3]){
							//printf("%d, %d\n",tim_pwm_shadow.CHN_CONFIG.CHANNEL[i], test->arg[3]);
							s907x_hal_timer_stop_pwm(ptimer, test->arg[3]);
							//s907x_hal_timer_pwm_deinit(ptimer, test->arg[3]);
							tim_pwm_shadow.CHN_CONFIG.IS_CHN_ENABLE[i] = FALSE;
							tim_pwm_shadow.CHN_CONFIG.CHANNEL[i] = 0;
						}
					}
				}
			}
		break;
		//pwm enable / disable
		//note: before pwm channel working, be sure enable
		case 3:
			if(test->arg[3]){//enable
				if(HAL_OK != s907x_hal_timer_start(ptimer)){
					test->result = FALSE;
				}
			}else{//disable
				if(HAL_OK != s907x_hal_timer_stop(ptimer)){
					test->result = FALSE;
				}
				
			}
		break;
		default:
		break;
	}
		
exit:
    if(tim_pwm_shadow.TIM_VALID){
        HAL_TEST_DBG("|CHN.NO|CHN.POL|CHN.SWITCH|CHN.PUSLE|\n");
        for(i = 0; i < PWM_CHANNEL_MAX; i++){
            HAL_TEST_DBG("   %d       %d        %d          %d\n", i ,tim_pwm_shadow.CHN_CONFIG.POL[i],  \
                                                      tim_pwm_shadow.CHN_CONFIG.IS_CHN_ENABLE[i],        \
                                                 tim_pwm_shadow.CHN_CONFIG.PUSLE[i]);                    \
        }
    }
    
    if(test->result){
            HAL_TEST_DBG("timer normal pwm test success\n"); 
    }else{
            HAL_TEST_DBG("timer normal pwm test fail\n"); 
    }
	
}

/*init and start*/
//AT+TIMER=5,0,frek,pol,chn_nums,[...]
//AT+TIMER=5,0,6,0,2,2,3                       (channel 2,3 set: pol = 0, default pusle: 50)
//AT+TIMER=5,0,6,1,3,0,1,7                     (channel 0,1,7 set: pol = 1, default pusle: 50)

/*pusle set to the channel*/
//AT+TIMER=5,1,frek,pol,chn,pusle
//AT+TIMER=5,1,6,0,2,70                        (set channel 2 pusle: 70)
//AT+TIMER=5,1,6,1,7,20                        (set channel 7 pusle: 20) 

/*stop the channel*/
//AT+TIMER=5,2,frek,pol,chn,0
//AT+TIMER=5,2,6,0,2,0                         (stop channel 2)
//AT+TIMER=5,2,6,1,7,0                         (stop channel 7)

/*start the channel with pusle set*/
//AT+TIMER=5,2,frek,pol,chn,1, pusle_set
//AT+TIMER=5,2,6,0,2,1,10                      (restart channel 2, set pusle:10)
//AT+TIMER=5,2,6,1,7,1,50                      (restart channel 7, set pusle:50)

/*pwm enable or disable*/
//AT+TIMER=5,3,x,x,0							(disable all channel)
//AT+TIMER=5,3,x,x,1							(enable all channel)


void timer_pwm_dma_test(hal_test_t *test)
{
    ASSERT(test);
    timer_hdl_t *ptimer = &timx_hdl;
    test->result = TRUE;
    int i;
    int temp;
    u32 freq_k = test->arg[1];
    u8 channel;
    u32 ccr;
    int cmd;
    int pol;
    int cnt;
    int pwm_dma_pusle;
    int pusle_set;
    int chn;
    
    temp = HZ_40M / freq_k;
    cmd = test->arg[0];
    pol = test->arg[2];
    cnt = test->arg[3];
    
	if(cmd != 3){
	 ASSERT(freq_k > 0 && freq_k <= HZ_40M);
	}
    ASSERT(IS_PWM_POLARITY(pol));
	if(cmd != 3){
		ASSERT(cnt > 0 && cnt <= PWM_CHANNEL_MAX);
	}
   

    if(!tim_pwm_shadow.TIM_VALID){
        //time base
        ptimer->config.idx = TIM_PWM;
        ptimer->config.prescaler = PRESCALER_SET;
        ptimer->config.period = temp / (PRESCALER_SET + 1);
        printf("ptimer->config.period:%d\n",ptimer->config.period);
        //basic user callback
        ptimer->it.basic_user_cb.func = NULL;
        ptimer->it.basic_user_cb.context = NULL;
                
        if(HAL_OK != s907x_hal_timer_base_init(ptimer)){
            test->result = FALSE;
            goto exit;
        }
        tim_pwm_shadow.TIM_VALID = TRUE;
        s907x_hal_timer_start(ptimer);
    }else{
        LINE_TAG();
    }

    switch(cmd){
        case 0:
            if(!tim_pwm_shadow.TIM_VALID){
                    test->result = FALSE;
                    goto exit;
            }
            
            pwm_dma_default = ptimer->config.period / 2;
            pwm_dma_pusle = (int)((pwm_dma_default * 1.0 / ptimer->config.period) * 100);
            
            for(i = 0; i < cnt; i++){
                channel = test->arg[i+4];
                //printf("channel = %d\n", channel);
                
                if(tim_pwm_shadow.CHN_CONFIG.IS_CHN_ENABLE[channel]){
                    test->result = FALSE;
                    goto exit;
                }
                
                tim_pwm_shadow.CHN_CONFIG.CHANNEL[channel] = channel;
                //pol
                tim_pwm_shadow.CHN_CONFIG.POL[channel] = pol;
                tim_pwm_shadow.CHN_CONFIG.IS_CHN_ENABLE[channel] = TRUE;
                
                tim_pwm_shadow.CHN_CONFIG.DMA_BUF[channel] = pwm_dma_default;
                tim_pwm_shadow.CHN_CONFIG.PUSLE[channel] = pwm_dma_pusle;
                
                ptimer->dma.polarity[channel] = pol;
                //default pulse
                ccr = ptimer->config.period/2+1;  
                if(HAL_OK != s907x_hal_timer_pwm_init(ptimer,  ptimer->dma.polarity[channel], ccr, channel)){
                    test->result = FALSE;
                    goto exit;
                }
                
                ptimer->dma.txbuf[channel] = (u8*)&tim_pwm_shadow.CHN_CONFIG.DMA_BUF[channel];
                ptimer->dma.txlen[channel] = sizeof(ptimer->dma.txbuf[channel]);
                //printf("%x\n", ptimer->dma.txbuf[channel]);
                ptimer->dma.tx_complete.func = timer_pwm_dma_cb;
                //tim->dma.tx_complete.context = tim;
                if(HAL_OK != s907x_hal_timer_start_pwm_dma(ptimer,channel)){
                    test->result = FALSE;
                    goto exit;
                }
                
            }             
        break;
        
        case 1:
            chn = test->arg[3];
            pusle_set = test->arg[4]; 
            ASSERT(pusle_set > 0 && pusle_set < 100);
            ASSERT(chn >= 0 && chn < PWM_CHANNEL_MAX);
            
            if(tim_pwm_shadow.TIM_VALID){
                if(tim_pwm_shadow.CHN_CONFIG.IS_CHN_ENABLE[chn]){
                    tim_pwm_shadow.CHN_CONFIG.PUSLE[chn] = pusle_set;
                    tim_pwm_shadow.CHN_CONFIG.IS_DMA_BUF_CHANGE[chn] =  TRUE;
                    goto exit;
                }
            }
            test->result = FALSE;
        break;
        
        case 2:
            chn = test->arg[3];
            //cmd:0 - off  1 - on
            cmd = test->arg[4];
            
            ASSERT(chn >= 0 && chn < PWM_CHANNEL_MAX);
            ASSERT(cmd == 0 || cmd == 1);
            
            if(cmd){
                pusle_set = test->arg[5];
                ASSERT(pusle_set > 0 && pusle_set < 100);
            }
            
            if(cmd){
              
              tim_pwm_shadow.CHN_CONFIG.DMA_BUF[chn] = (u32)pusle_set * 1.0 / 100 * ptimer->config.period;
              ptimer->dma.txbuf[chn] = (u8*)&tim_pwm_shadow.CHN_CONFIG.DMA_BUF[chn];
              ptimer->dma.txlen[chn] = sizeof(ptimer->dma.txbuf[chn]);
                
              tim_pwm_shadow.CHN_CONFIG.PUSLE[chn] = pusle_set;
              tim_pwm_shadow.CHN_CONFIG.IS_CHN_ENABLE[chn] = TRUE;;
              
              if(HAL_OK != s907x_hal_timer_start_pwm_dma(ptimer, chn)){
                    test->result = FALSE;
                    goto exit;
              }
            
           
            }else{
            
              if(HAL_OK != s907x_hal_timer_stop_pwm_dma(ptimer, chn)){
                  test->result = FALSE;
                  goto exit;
              }
              tim_pwm_shadow.CHN_CONFIG.IS_CHN_ENABLE[chn] = FALSE;
              tim_pwm_shadow.CHN_CONFIG.PUSLE[chn] = 0;

            }
            
        break;
		
		case 3:
			if(test->arg[3]){//pwm enable 
				if(HAL_OK != s907x_hal_timer_start(ptimer)){
					test->result = FALSE;
				}
			}else{//pwm disable
				if(HAL_OK != s907x_hal_timer_stop(ptimer)){
					test->result = FALSE;
				}
			}
		break;
    }
    
exit:

    if(tim_pwm_shadow.TIM_VALID){
        HAL_TEST_DBG("|CHN.NO|CHN.POL|CHN.SWITCH|CHN.PUSLE|\n");
        for(i = 0; i < PWM_CHANNEL_MAX; i++){
            HAL_TEST_DBG("   %d       %d        %d          %d\n", i ,tim_pwm_shadow.CHN_CONFIG.POL[i],  \
                                                      tim_pwm_shadow.CHN_CONFIG.IS_CHN_ENABLE[i],        \
                                                 tim_pwm_shadow.CHN_CONFIG.PUSLE[i]);                    \
        }
    }
    
    if(test->result){
            HAL_TEST_DBG("timer normal pwm test success\n"); 
    }else{
            HAL_TEST_DBG("timer normal pwm test fail\n"); 
    }
}



 
void timer_capture_test(hal_test_t *test)
{
    timer_hdl_t *tim = &timx_hdl;
 	u32 mode;
	u8 channel;
    // 1 set basic timer
    tim->config.idx = TIM_CAP; 
    tim->config.prescaler = 2;
    tim->config.period = 0xFFFF;
  
    //basic user callback
    tim->it.basic_user_cb.func = NULL;
    tim->it.basic_user_cb.context = tim;
    s907x_hal_timer_base_init(tim);
    //2 set capture timer & channel
    mode = CAPTURE_PULSE;
    channel = CAPTURE_CHANNEL_0;
  
    //s907x_hal_timer_capture_init(tim,channel,mode);
	s907x_hal_timer_capture_init(tim,mode);
    //set dam recv callback 
    if(test->arg[0]) {
        tim->dma.rx_complete.func = timer_capture_dma_cb;
        tim->dma.rx_complete.context = tim;
        tim->dma.rxbuf = (u8*)capture_dma_buffer;
        tim->dma.rxlen = sizeof(capture_dma_buffer);
        s907x_hal_timer_start_capture_dma(tim);
    } else
        s907x_hal_timer_stop_capture_dma(tim);
 


}


static void timer_test_enter(hal_test_t *test)
{
    if(test->no == TIM_TEST_BASIC) {
		timer_basic_test(test);
	}else if(test->no == TIM_TEST_TICK) {
		timer_tick_test(test);
	}else if(test->no == TIM_TEST_SYSTIMER) {
		timer_systimer_test(test);//
	}else if(test->no == TIM_TEST_DELAY) {
		timer_delay_test(test);
	}else if(test->no == TIM_TEST_PWM) {
		timer_pwm_test(test);
		//timer_pwm_init(test);
	}else if(test->no == TIM_TEST_PWM_DMA){
		timer_pwm_dma_test(test);
	}
	else if(test->no == TIM_TEST_CAPTURE) {
		timer_capture_test(test);
	}
}

static void timer_test_exit(hal_test_t *test)
{
    ASSERT(test);
    
    //to do
	if(test->no == TIM_TEST_BASIC) {
		
	}else if(test->no == TIM_TEST_TICK) {
		
	}else if(test->no == TIM_TEST_SYSTIMER) {
		
	}else if(test->no == TIM_TEST_DELAY) {
		
	}else if(test->no == TIM_TEST_PWM) {
		
	}else if(test->no == TIM_TEST_CAPTURE) {
		
	}
    

}

//example
hal_test_name_map_t timer_test_map[] = 
{
	{0, "timer basic test"},
	{1, "timer tick test"},
	{2, "systimer test"},
	{3, "timer delay test"},
	{4, "timer pwm test"},
	{5, "timer pwm dma test"},
	{6, "timer capture test"},
};
/*		        test      no   arg0  arg1 
*               @TIMER    0     1-3   0     	    (BASIC )  
*    		    @TIMER    1     0    1000    		(TICK)
*          	    @TIMER    2     x    x	 	        (SYSTIMER)
*             	@TIMER    3     500  x     		    (DELAY)
*              	@TIMER    4     0    199     199  	(PWM - init	  prescaler period channel prioriy)
*             	@TIMER    4     1    xxx     1/0    (PWM - normal start/stop)
*               @TIMER    4     2    xxx     		(PWM - normal pulse)
*               @TIMER    4     3    xxx     		(PWM - normal pulse)
*               @TIMER    4     4    1/0  		(PWM - dma start/stop)
*               @TIMER    5     0    0-1	        (CAPTURE - init) 
*               @TIMER    5     1    x      		(CAPTURE - dma start) 
*               @TIMER    5     2    x      		(CAPTURE - dma stop)
*/



void timer_test(hal_test_t *test)
{

	ASSERT(test);
	ASSERT(test->no < (sizeof(timer_test_map)/sizeof(hal_test_name_map_t)));

	HAL_TEST_DBG("test no %d = %s\n", test->no, timer_test_map[test->no].name);
	
	timer_test_enter(test);

    timer_test_exit(test);
}

#endif


void TIM1_timer_basic_interrupt(void *context)
{
		u32 gpio_pin;
 		
#if M_AT_TEST
		if(newstarttest)
			runtime_cnt++;
		
		//print
		if((time_cnt++ > 5))//newstarttest && 
		{
			time_cnt = 0;
			if(totalDatas >= 100000000){
				hundred_million++;
				totalDatas = 0;
			}
				
			HAL_TEST_DBG(">>>连续正确读写总字节数:%d亿 - %d bytes\n",hundred_million, totalDatas);
			HAL_TEST_DBG(">>>连续正确读写总时间: %d\n",runtime_cnt);
		}	
		
		//led
		if(!master_tim_sel)//sla
			gpio_pin = BIT(5);
		else
			gpio_pin = BIT(6);
#endif		
		s907x_hal_gpio_togglepin(gpio_pin);	
}

void at_test_timer_init(hal_test_t *test)
{
#if M_AT_TEST
    	timer_hdl_t *tim = &timx_hdl;
    
    	tim->config.idx = 3;//user tim1//32k

        tim->config.prescaler = 0x0;
        tim->config.period = 32000; //1s/int
        tim->config.int_enable = 1;
		//set user callback
        tim->it.basic_user_cb.func = TIM1_timer_basic_interrupt;
        tim->it.basic_user_cb.context = tim;

        s907x_hal_timer_base_init(tim);
        s907x_hal_timer_start_base(tim);   
#endif    
}
