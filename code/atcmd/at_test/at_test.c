#include "s907x.h"
#include "at_test.h"
#include "gpio_test.h"
#include "uart_test.h"
#include "adc_test.h"
#include "spi_test.h"
#include "i2c_test.h"
#include "i2s_test.h"
#include "rtc_test.h"
#include "timer_test.h"
#include "ping_test.h"
#include "iperf_test.h"
#include "wlan_test.h"
#include "ota_test.h"
#include "tuya_test.h"
#include "mqtt_test.h"
#include "rf_test.h"

#if M_AT_TEST   
//AT+UART=xxx,xxx,xxx
static int at_test_hal(void *context, hal_test_t *test)
{	
	int i;
    char  *argv[AT_SET_MAX_ARGC];
    int    argc;

	ASSERT(test);

	at_cmd_t *cmd = (at_cmd_t *)context;

    argc   = at_parse_set(cmd->set, argv, AT_TEST_SETTAG); 

	test->no = atoi(argv[0]);
    test->arg_cnt = argc - 1;
	for(i = 0; i < argc - 1; i ++) {
		test->arg[i] = atoi(argv[1 + i]);
	}
	return TRUE;
}


static void at_test_gpio(void *context)
{	
	hal_test_t test;
	
	if(at_test_hal(context, &test)) {
		gpio_test(&test);
	} else {
		HAL_TEST_DBG("%s failed\n", __func__);
	}
}

static void at_test_i2c(void *context)
{	
	hal_test_t test;
	
	if(at_test_hal(context, &test)) {
		i2c_test(&test);
	} else {
		HAL_TEST_DBG("%s failed\n", __func__);
	}
}

static void at_test_uart(void *context)
{
	hal_test_t test;
	
	if(at_test_hal(context, &test)) {
		uart_test(&test);
	} else {
		HAL_TEST_DBG("%s failed\n", __func__);
	}
}

static void at_test_spi(void *context)
{
	hal_test_t test;
	
	if(at_test_hal(context, &test)) {
		spi_test(&test);
	} else {
		HAL_TEST_DBG("%s failed\n", __func__);
	}
}


static void at_test_adc(void *context)
{
	hal_test_t test;
	
	if(at_test_hal(context, &test)) {
		adc_test(&test);
	} else {
		HAL_TEST_DBG("%s failed\n", __func__);
	}
}


static void at_test_i2s(void *context)
{
	hal_test_t test;
	
	if(at_test_hal(context, &test)) {
		i2s_test(&test);
	} else {
		HAL_TEST_DBG("%s failed\n", __func__);
	}
}

static void at_test_timer(void *context)
{
	hal_test_t test;
	
	if(at_test_hal(context, &test)) {
		timer_test(&test);
	} else {
		HAL_TEST_DBG("%s failed\n", __func__);
	}
} 

static void at_test_rtc(void *context)
{
	hal_test_t test;
	
	if(at_test_hal(context, &test)) {
		rtc_test(&test);
	} else {
		HAL_TEST_DBG("%s failed\n", __func__);
	}
}



static void at_test_wdg(void *context)
{
	hal_test_t test;
	
	if(at_test_hal(context, &test)) {
		wdg_test(&test);
	} else {
		HAL_TEST_DBG("%s failed\n", __func__);
	}
}

//flash use hex data
static void at_test_flash(void *context)
{
	hal_test_t test;
	int i;
    char  *argv[AT_SET_MAX_ARGC];
    int    argc;
	at_cmd_t *cmd = (at_cmd_t *)context;

    argc   = at_parse_set(cmd->set, argv, AT_TEST_SETTAG); 

	test.no = atoi(argv[0]);
    test.arg_cnt = argc - 1;
	for(i = 0; i < argc - 1; i ++) {
        test.arg[i] = strtoul((const u8*)argv[1 + i], (u8 **)NULL, 16);
	}

	if(at_test_hal(context, &test)) {
		flash_test(&test);
	} else {
		HAL_TEST_DBG("%s failed\n", __func__);
	}
}

static void at_test_ps(void *context)
{
	hal_test_t test;
	
	if(at_test_hal(context, &test)) {
		//ps_test(&test);
	} else {
		HAL_TEST_DBG("%s failed\n", __func__);
	}
}
static void at_test_ping(void *context)
{
	ping_test(context);
}
 
static void at_test_iperf(void *context)
{
	iperf_test(context);
}

static void at_mqtt_test(void *context)
{
  hal_test_t test;
	
  if(at_test_hal(context, &test)) {
      mqtt_test(&test);
  } else {
      HAL_TEST_DBG("%s failed\n", __func__);
  }
}
  

static void at_wlan_test(void *context)
{
    wlan_test(context);
} 

static void at_rf_test(void *context)
{
    rf_test(context);
}

#if TUYA_BUILD
static void at_tuya_test(void *context)
{
    tuya_test(context);
} 
#endif

static void at_test_ota(void *context)
{
    ota_test(context);
}

 
static at_item_t at_test_tbl[] =  
{
	//hal test
	{{"GPIO"},      at_test_gpio},	
	{{"I2C"},       at_test_i2c},
	{{"UART"},      at_test_uart},
	{{"SPI"},       at_test_spi},
	{{"ADC"},       at_test_adc},
	{{"I2S"},       at_test_i2s},
    {{"RTC"},       at_test_rtc},
	{{"TIMER"},     at_test_timer},	
	{{"WDG"},       at_test_wdg},
 	{{"FLASH"},     at_test_flash},   
	
    {{"PINGTEST"},  at_test_ping},
    {{"IPERF"},     at_test_iperf},
    {{"PS"},		at_test_ps},  
    {{"WLAN"},      at_wlan_test},
    {{"RF"},      at_rf_test},
    {{"MQTT"},      at_mqtt_test},
	{{"OTA"},       at_test_ota},
	#if TUYA_BUILD
	{{"TUYA"}, at_tuya_test}, 
	#endif
};
  

void at_test_init(void)
{
	at_add_cmd(&at_test_tbl[0], (sizeof(at_test_tbl)/sizeof(at_item_t)));	
}


void at_test_deinit(void)
{
	at_remove_cmd(&at_test_tbl[0], (sizeof(at_test_tbl)/sizeof(at_item_t)));	
}
#endif

