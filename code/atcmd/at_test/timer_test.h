#ifndef TIMER_TEST_H
#define TIMER_TEST_H



#define PWM_CHANNEL_MAX			8
#define	HZ_40M					40000
#define	PRESCALER_SET			3

#define LINE_TAG()                      printf("\n---------------------------------test---result----------------------------------\n");

typedef enum TIM_TYPE{
	TIM_TEST_BASIC,
	TIM_TEST_TICK,
	TIM_TEST_SYSTIMER,
	TIM_TEST_DELAY,
	TIM_TEST_PWM,
	TIM_TEST_PWM_DMA,
	TIM_TEST_CAPTURE,
}TIM_TS_ITEM;

typedef enum TIM_BASIC_TEST_{
	BASIC_CONTINOUS = 0,
}TIM_BASIC_TEST_ITEM;

typedef enum TIM_PWM_TEST_{
	PWM_NORMAL = 0,
	PWM_NORMAL_CHANGE_PULSE,
	PWM_DMA,	

}TIM_PWM_TEST_ITEM;


typedef enum TIM_CAPTURE_TEST_{
	CAPTURE_INIT = 0,
	CAPTURE_DMA_START,
	CAPTURE_DMA_STOP,
}TIM_CAPTURE_TEST_ITEM;

typedef struct CHANNEL_CFJ_{
	int CHANNEL[PWM_CHANNEL_MAX];
	int POL[PWM_CHANNEL_MAX];
	int IS_CHN_ENABLE[PWM_CHANNEL_MAX];
	int IS_DMA_BUF_CHANGE[PWM_CHANNEL_MAX];
        u32 DMA_BUF[PWM_CHANNEL_MAX];
	u8      PUSLE[PWM_CHANNEL_MAX];
}CHANNEL_CFJ;


typedef struct TIM_PWM_CTR_{
	int TIM_VALID;
	CHANNEL_CFJ CHN_CONFIG;
}TIM_PWM_CTR;






void timer_test(hal_test_t *test);
void at_test_timer_init(hal_test_t *test);


#endif