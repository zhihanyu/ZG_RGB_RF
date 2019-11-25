#ifndef HAL_ADC_H
#define HAL_ADC_H

#define ADC_CHN_0    0     
#define ADC_CHN_1    1
#define ADC_CHN_TEMP 2




#define ADC_IT_ONESHOT			(BIT0)
#define ADC_IT_CONTINOUS		(BIT1)	
#define ADC_IT_DMA              (BIT2)
#define IS_ADC_IT(x)			((x) == ADC_IT_ONESHOT || (x) == ADC_IT_CONTINOUS || (x) == ADC_IT_DMA)

#define ADC_FIFO                16


#define ADC_GAIN0			0.078172001
#define ADC_GAIN1			0.078085709
#define ADC_GAIN2			0.078264276


#define ADC_OFFSET0  		-1261.644661
#define ADC_OFFSET1  		-1259.026661
#define ADC_OFFSET2  		-1267.556995

#define ADC_BOUNDARY0       0x7310
#define ADC_BOUNDARY1       0xbe50

typedef struct adc_oneshot_
{
	u8  enable;
	u8  read_nums;  //max 16
	u16 rsvd;
	u32 delay;
}adc_oneshot_t;

typedef struct adc_config_
{	
	u32 mode;
	adc_oneshot_t oneshot;
}adc_config_t;

typedef struct adc_data_
{	
	u16 chn[2];
    u16 temperature;
	double voltage[2];
    u16 rsvd;
}adc_data_t;


typedef struct adc_hdl_
{
    adc_config_t config; 
    adc_data_t   data[ADC_FIFO];
	hal_cb_t      it;
}adc_hdl_t;






hal_status_e s907x_hal_adc_init(adc_hdl_t *adc);
hal_status_e s907x_hal_adc_deinit(adc_hdl_t *adc);
u32 		 s907x_hal_adc_read(adc_hdl_t *adc);
hal_status_e s907x_hal_adc_get_mv(adc_hdl_t *adc);
hal_status_e s907x_hal_adc_poll_oneshot(adc_hdl_t *adc, u32 timeout);
hal_status_e s907x_hal_adc_poll_continous(adc_hdl_t *adc);
hal_status_e s907x_hal_adc_start_it(adc_hdl_t *adc, u32 mode);
hal_status_e s907x_hal_adc_stop_it(adc_hdl_t *adc, u32 mode);
hal_status_e s907x_hal_adc_interrupt_oneshot(adc_hdl_t *adc, hal_int_cb cb, void *arg);
hal_status_e s907x_hal_adc_interrupt_continous(adc_hdl_t *adc, hal_int_cb cb, void *arg);





#endif
