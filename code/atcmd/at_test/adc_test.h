#ifndef ADC_TEST_H
#define ADC_TEST_H





typedef enum ADC_TEST{
	ADC_READ_POLL = 0,
	ADC_READ_INT = 1,
	ADC_ONESHOT_POOL = 2,
	ADC_ONESHOT_INT = 3,
    ADC_DEINIT = 4,
    ADC_GET_VOLTAGE = 5
}ADC_TEST_ITEM;







void adc_test(hal_test_t *test);





















#endif