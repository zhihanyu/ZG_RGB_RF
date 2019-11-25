#include "s907x.h"
#include "wdg_test.h"

#if M_AT_TEST

wdg_hdl_t wdg_hdl;


hal_test_name_map_t wdg_test_map[] = 
{
	{0, "wdg basic"},
	{1, "wdg alarm"},

}; 

 

void wdg_test_basic(hal_test_t *test)
{
	ASSERT(test);
    
	if(test->arg[0]) {
		wdg_hdl.time_ms = test->arg[1];
		s907x_hal_wdg_init(&wdg_hdl);
		s907x_hal_wdg_start(&wdg_hdl);
	} else {
		s907x_hal_wdg_refresh(&wdg_hdl);
	}
}

 
static void wdg_isr_cb(void *context)
{
	HAL_TEST_DBG("%s\n", __func__);
    NVIC_SystemReset();	
} 


void wdg_test_int(hal_test_t *test)
{
	ASSERT(test);
    
	if(test->arg[0]) {
		wdg_hdl.time_ms = test->arg[1];
		wdg_hdl.it.func = wdg_isr_cb;
		wdg_hdl.it.context = &wdg_hdl;
		s907x_hal_wdg_init(&wdg_hdl);
		s907x_hal_wdg_start_it(&wdg_hdl);
	}
}
 
void wdg_test(hal_test_t *test)
{
	ASSERT(test);
	ASSERT(test->no < (sizeof(wdg_test_map)/sizeof(hal_test_name_map_t)));

	HAL_TEST_DBG("test no %d = %s\n",test->no, wdg_test_map[test->no].name);
       
	switch(test->no) 
	{
		case WDG_TEST_BASIC:    
            wdg_test_basic(test);
			break;
		case WDG_TEST_INT:
            wdg_test_int(test);
			break;
		default:

			break;
	}
}
#endif