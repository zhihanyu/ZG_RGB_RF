#ifndef HAL_TIMER_H
#define HAL_TIMER_H


#define TIM_PWM  		0   //pwm timer 0
#define TIM_CAP 	 	1   //capture timer 1
#define TIM0            2   //system timer 2

#define TIM1 	 		3   //user timer 3
#define TIM2 	 		4   //user timer 4
#define TIM3 	 		5   //user timer 5

#define TIM_USER        TIM1
#define TIM_MAX         TIM3

#define IS_BASIC_TIMER(x) ((x >= TIM_USER) && (x <= TIM_MAX))


#define TIM_MSP_BASIC      0 
#define TIM_MSP_PWM        1  
#define TIM_MSP_CAPTURE    2

//interrupt
#define TIM_IT_Update			((u32)0x00000001)
#define TIM_IT_CC0				((u32)0x00000002)
#define TIM_IT_CC1				((u32)0x00000004)
#define TIM_IT_CC2				((u32)0x00000008)
#define TIM_IT_CC3				((u32)0x00000010)
#define TIM_IT_CC4				((u32)0x00000020)
#define TIM_IT_CC5				((u32)0x00000040)
#define IS_TIM_IT(IT) ((((IT) & (u32)0xFF80) == 0x0000) && (((IT) & (u32)0x7F) != 0x0000))

#define IS_TIM_GET_IT(IT) (((IT) == TIM_IT_Update) || \
                           ((IT) == TIM_IT_CC0) || \
                           ((IT) == TIM_IT_CC1) || \
                           ((IT) == TIM_IT_CC2) || \
                           ((IT) == TIM_IT_CC3) || \
                           ((IT) == TIM_IT_CC4) || \
                           ((IT) == TIM_IT_CC5))

#define TIM_IT_MASK            (0xFFFFFFFF)


//pwm config
#define PWM_POLARITY_HIGH	0
#define PWM_POLARITY_LOW	1

#define IS_PWM_POLARITY(POL)    (((POL) == PWM_POLARITY_HIGH)  || \
                                 ((POL) == PWM_POLARITY_LOW))

//capture config only one channel support
#define CAPTURE_PULSE	0
#define CAPTURE_NUMBER	1

#define CAPTURE_CHANNEL_0		0
#define CAPTURE_CHANNEL_NUMS    1

#define PWM_MAX_CHN  8

typedef struct timer_dma_
{
	hal_cb_t rx_complete;
	hal_cb_t tx_complete;
	u8 *rxbuf;
	u32 rxlen;
	u32 rx_complete_len;
	u8 *txbuf[PWM_MAX_CHN];
	u32 txlen[PWM_MAX_CHN];
	u32 reload[PWM_MAX_CHN];
	u32 polarity[PWM_MAX_CHN];
    void *resource;
}timer_dma_t;


typedef struct timer_it_
{
	hal_cb_t basic_user_cb;
	void     *object;
}timer_it_t;



typedef void (*timer_int_cb)(void *);




typedef struct timer_config_
{
	u8  idx;
	u32 prescaler;
	u32 period;
	u32 int_enable;
}timer_config_t;



typedef struct timer_hdl_
{
	timer_config_t  config;	
	timer_it_t      it;
	timer_dma_t     dma;
}timer_hdl_t;

typedef struct timer_dma_context_
{
    void *timer;
	int channel;
}timer_dma_context_t;




u32 s907x_hal_timer_get_counter(timer_hdl_t *tim);
hal_status_e s907x_hal_timer_base_init(timer_hdl_t *tim);
hal_status_e s907x_hal_timer_base_deinit(timer_hdl_t *tim);
hal_status_e s907x_hal_timer_start_base(timer_hdl_t *tim);
hal_status_e s907x_hal_timer_start(timer_hdl_t *tim);
hal_status_e s907x_hal_timer_stop(timer_hdl_t *tim);


hal_status_e s907x_hal_timer_set_period(timer_hdl_t *tim, u32 period);
hal_status_e s907x_hal_timer_set_polarity(timer_hdl_t *tim, u32 polarity);

hal_status_e s907x_hal_timer_pwm_init(timer_hdl_t *tim, u32 polarity, u32 ccr, u8 channel);
hal_status_e s907x_hal_timer_pwm_deinit(timer_hdl_t *tim, u8 channel);
hal_status_e s907x_hal_timer_start_pwm(timer_hdl_t *tim, u8 channel);
hal_status_e s907x_hal_timer_stop_pwm(timer_hdl_t *tim, u8 channel);

hal_status_e s907x_hal_timer_pwm_dma_reload(timer_hdl_t *tim, u8 channel);
hal_status_e s907x_hal_timer_start_pwm_dma(timer_hdl_t *tim, u8 channel);
hal_status_e s907x_hal_timer_stop_pwm_dma(timer_hdl_t *tim, u8 channel);
hal_status_e s907x_hal_timer_pwm_set_ccr(timer_hdl_t *tim, u32 ccr, u8 channel);

hal_status_e s907x_hal_timer_capture_init(timer_hdl_t *tim, u32 mode);
hal_status_e s907x_hal_timer_capture_deinit(timer_hdl_t *tim);
hal_status_e s907x_hal_timer_start_capture(timer_hdl_t *tim);
hal_status_e s907x_hal_timer_start_capture_dma(timer_hdl_t *tim);
hal_status_e s907x_hal_timer_stop_capture_dma(timer_hdl_t *tim);





#endif
