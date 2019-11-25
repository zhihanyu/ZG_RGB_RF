#include "s907x.h"
#include "adc_test.h"
   
#if M_AT_TEST

static  int adc_int_finish = FALSE;
static  adc_hdl_t adc_hdl;

void adc_read_poll(hal_test_t *test)
{
    hal_status_e status;
	adc_hdl_t*adc = &adc_hdl;
	int i = 0;
	u32 loop = test->arg[1];

    adc->config.oneshot.enable = FALSE;
	s907x_hal_adc_init(adc);
 
	for (i = 0; i < loop; i++) {
        status = s907x_hal_adc_poll_continous(adc);
        if(status == HAL_OK) {
            HAL_TEST_DBG("adc0  = %x adc1  =%x\n", adc->data[0].chn[0], adc->data[0].chn[1]);
        }
		wl_os_mdelay(1000);
	}
}   
 


void voltage_print(u8 id,float value)
{
	u32 tmp,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6;
	u8 flag = 0;
	if(value<0)
	{
		value = 0-value;
		flag = 1;
	}else
		flag = 0;
		
	
	tmp = (int)value;
	tmp1=(int)((value-tmp)*10)%10;
	tmp2=(int)((value-tmp)*100)%10;
	tmp3=(int)((value-tmp)*1000)%10;
	tmp4=(int)((value-tmp)*10000)%10;
	tmp5=(int)((value-tmp)*100000)%10;
	tmp6=(int)((value-tmp)*1000000)%10;
	if(flag == 1)
		printf("ADC%d = -%d.%d%d%d%d%d%d mv\r\n",id,tmp,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6);
	else
		printf("ADC%d = %d.%d%d%d%d%d%d mv\r\n",id,tmp,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6);
}

void adc_get_voltage(hal_test_t *test)
{
    hal_status_e status;
	adc_hdl_t*adc = &adc_hdl;
	int i = 0;
	int channel = test->arg[0];
	u32 loop = test->arg[1];

    adc->config.oneshot.enable = FALSE;
	s907x_hal_adc_init(adc);
	
	for (i = 0; i < loop; i++) {
        status = s907x_hal_adc_get_mv(adc);
        if(status == HAL_OK) {
			if(channel == 0)
            	voltage_print(0,adc->data[0].voltage[0]);
			else if(channel == 1)
				voltage_print(1,adc->data[0].voltage[1]);
			else if(channel == 2)
			{
				voltage_print(0,adc->data[0].voltage[0]);
				voltage_print(1,adc->data[0].voltage[1]);
			}
        }
		wl_os_mdelay(1000);
	}
}   
 
static void adc_read_isr(void *context)
{
    adc_hdl_t*adc = &adc_hdl;

    HAL_TEST_DBG("adc0  = %x adc1  =%x\n", adc->data[0].chn[0], adc->data[0].chn[1]);
}

void adc_read_poll_int(hal_test_t *test)
{
	adc_hdl_t*adc = &adc_hdl;
    
    adc->config.oneshot.enable = FALSE;

    s907x_hal_adc_init(adc);

    s907x_hal_adc_interrupt_continous(adc, adc_read_isr, adc);
}


void adc_oneshot_pool_test(hal_test_t *test)
{   
    int i;
    u32 loop = 1;
    adc_hdl_t *adc = &adc_hdl;

    ASSERT(test);
    ASSERT(test->arg_cnt == 2);

    loop = test->arg[1];
    adc->config.oneshot.enable = TRUE;
    adc->config.oneshot.delay = test->arg[0];
    adc->config.oneshot.read_nums = ADC_FIFO;
    s907x_hal_adc_init(adc);
    while(loop--) {
        s907x_hal_adc_poll_oneshot(adc, HAL_MAX_DELAY);
        for(i = 0; i < 8; i++) {
            HAL_TEST_DBG("cnt = %d adc0  = %x adc1  =%x\n", i, adc->data[i].chn[0], adc->data[i].chn[1]);
        }
        s907x_hal_adc_stop(adc);
    }
}

static void adc_oneshot_poll_isr(void *context)
{   
    int i;
    adc_hdl_t *adc = (adc_hdl_t*)context;
#if 1//user can stop oneshot interrupt
    adc_ll_mask_it(adc, ADC_IT_ONESHOT, DISABLE);  
    adc_ll_clr_flag(adc);	 
    s907x_hal_adc_stop(context);
#endif
    adc_int_finish = TRUE;
    for(i = 0; i < 8; i++) {
        HAL_TEST_DBG("cnt = %d adc0  = %x adc1  =%x\n", i, adc->data[i].chn[0], adc->data[i].chn[1]);
    }
}


void adc_oneshot_int(hal_test_t *test)
{
	adc_hdl_t *adc = &adc_hdl;
    
    s907x_hal_adc_init(adc);
    
    adc->config.oneshot.enable = TRUE;
    adc->config.oneshot.delay = test->arg[0];
    adc->config.oneshot.read_nums = ADC_FIFO;
    adc_int_finish = FALSE;

    s907x_hal_adc_interrupt_oneshot(adc, adc_oneshot_poll_isr, adc);
    while(!adc_int_finish) {
        wl_os_mdelay(100);
    }

   
}   






//example
hal_test_name_map_t adc_test_map[] = 
{
	{0, "ADC_READ_POLL"},
	{1, "ADC_READ_INT"},
	{2, "ADC_ONESHOT_POOL"},
	{3, "ADC_ONESHOT_INT"},
    {4, "ADC_DEINIT"},
    {5, "ADC_GET_VOLTAGE"},
};




void adc_test(hal_test_t *test)
{
	adc_hdl_t *adc = &adc_hdl;

	ASSERT(test);
	ASSERT(test->no < (sizeof(adc_test_map)/sizeof(hal_test_name_map_t)));

	HAL_TEST_DBG("test no %d = %s\n", adc_test_map[test->no].type, adc_test_map[test->no].name);
    
    if(test->no != ADC_DEINIT) {
	   memset((void*)(adc), 0, sizeof(adc_hdl_t));
    }
	switch(test->no) 
	{
		case ADC_READ_POLL:
			adc_read_poll(test);
			break;
        case ADC_READ_INT:
            adc_read_poll_int(test); 
            break;
		case ADC_ONESHOT_POOL:
			adc_oneshot_pool_test(test);
			break;
		case ADC_ONESHOT_INT:
			adc_oneshot_int(test);
			break;
        case ADC_DEINIT:
            s907x_hal_adc_deinit(adc);
            break;
		case ADC_GET_VOLTAGE:
			adc_get_voltage(test);
			break;

		default:

		break;
	}
    
    
}

#endif
