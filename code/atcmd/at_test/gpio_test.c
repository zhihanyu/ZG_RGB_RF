#include "s907x.h"
#include "gpio_test.h"

#if M_AT_TEST
sema_t clock_syn_sema;
sema_t	clock_syn_b_sema;

u8 SYN = 0;
//example
hal_test_name_map_t gpio_test_map[] = 
{
	{0, "GPIO_INIT"},
	{1, "GPIO_RW"},
	{2, "GPIO_PULL"},
	{3, "GPIO_INT"},
	{4, "GPIO_TOG"},
	{5, "GPIO_DINIT"},
	
}; 
   
  

void gpio_init_test(hal_test_t *test)
{
 gpio_init_t init;
 u32 gpio_pin;

 ASSERT(test);
 
 gpio_pin = BIT(test->arg[0]);
 if(test->arg[2]==TEST_GPIO_PULLUP){
	 init.pull = GPIO_PULLUP;
	 HAL_TEST_DBG("TEST GPIO PULLUP ");
 }if(test->arg[2]==TEST_GPIO_PULLDOWN){
	 init.pull = GPIO_PULLDOWN;
	 HAL_TEST_DBG("TEST GPIO PULLDOWN ");
 }if(test->arg[2]==TEST_GPIO_NOPULL){
	 init.pull = GPIO_NOPULL;
	 HAL_TEST_DBG("TEST GPIO NOPULL ");
 }
 
 if(test->arg[1] == TEST_GPIO_READ) {
	  init.mode = GPIO_MODE_INPUT;
 } else if(test->arg[1] == TEST_GPIO_WRITE) {
	 init.mode = GPIO_MODE_OUTPUT;
 }
 s907x_hal_gpio_init(gpio_pin, &init);
 HAL_TEST_DBG("GPIO %d init sucess\n",test->arg[0]);

}
void gpio_rw_test(hal_test_t *test)
{
    gpio_init_t init;
	u32 gpio_pin;
	
	ASSERT(test);
	
	gpio_pin = BIT(test->arg[0]);

    if(test->arg[1] == TEST_GPIO_READ) {
		s907x_hal_gpio_set_io(gpio_pin,GPIO_MODE_INPUT);
        HAL_TEST_DBG("GPIO %d rd status = %d\n", test->arg[0], s907x_hal_gpio_read(gpio_pin));
    } else if(test->arg[1] == TEST_GPIO_WRITE) {
    	s907x_hal_gpio_set_io(gpio_pin,GPIO_MODE_OUTPUT);
        s907x_hal_gpio_write(gpio_pin, test->arg[2] > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_TEST_DBG("GPIO %d  wr status = %d\n", test->arg[0], test->arg[2] > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

void gpio_user_isr(void *context)
{
    u32 *gpio_pin = (u32 *)context;
	u8 i;
    u32 pin_num;

    for(i = 0; i < 32; i++) {
        if(*gpio_pin & BIT(i)) {
            pin_num = i;
			break;
        }
    }
	
    HAL_TEST_DBG("GPIO %x rd status = %d\n", pin_num, s907x_hal_gpio_read(*gpio_pin));
	
	s907x_hal_gpio_it_stop(*gpio_pin);
}

void gpio_int_test(hal_test_t *test)
{
    ASSERT(test);
    gpio_init_t init;
    u32 gpio_pin;

    ASSERT(test);
    
    gpio_pin  = BIT(test->arg[0]);    

	if (test->arg[1] ==TEST_GPIO_TRIGGER_RISING){
		init.mode = GPIO_MODE_INT_RISING;
		if(test->arg[2]==TEST_GPIO_PULLUP){
		    init.pull = GPIO_PULLUP;
			HAL_TEST_DBG("TEST GPIO PULLUP ");
		}if(test->arg[2]==TEST_GPIO_PULLDOWN){
			init.pull = GPIO_PULLDOWN;
			HAL_TEST_DBG("TEST GPIO PULLDOWN ");
		}if(test->arg[2]==TEST_GPIO_NOPULL){
			init.pull = GPIO_NOPULL;
			HAL_TEST_DBG("TEST GPIO NOPULL ");
		}
		HAL_TEST_DBG("TEST_GPIO_TRIGGER_RISING INT\n");
	}else if(test->arg[1] ==TEST_GPIO_TRIGGER_FALLING){
		init.mode = GPIO_MODE_INT_FALLING;
		if(test->arg[2]==TEST_GPIO_PULLUP){
		    init.pull = GPIO_PULLUP;
			HAL_TEST_DBG("TEST GPIO PULLUP ");
		}if(test->arg[2]==TEST_GPIO_PULLDOWN){
			init.pull = GPIO_PULLDOWN;
			HAL_TEST_DBG("TEST GPIO PULLDOWN ");
		}if(test->arg[2]==TEST_GPIO_NOPULL){
			init.pull = GPIO_NOPULL;
			HAL_TEST_DBG("TEST GPIO NOPULL ");
		}
		HAL_TEST_DBG("TEST_GPIO_TRIGGER_FALLING INT\n");
	}else if(test->arg[1] ==TEST_GPIO_LEVEL_LOW){
		init.mode = GPIO_MODE_INT_LEVEL_L;
		if(test->arg[2]==TEST_GPIO_PULLUP){
		    init.pull = GPIO_PULLUP;
			HAL_TEST_DBG("TEST GPIO PULLUP ");
		}if(test->arg[2]==TEST_GPIO_PULLDOWN){
			init.pull = GPIO_PULLDOWN;
			HAL_TEST_DBG("TEST GPIO PULLDOWN ");
		}if(test->arg[2]==TEST_GPIO_NOPULL){
			init.pull = GPIO_NOPULL;
			HAL_TEST_DBG("TEST GPIO NOPULL ");
		}
		HAL_TEST_DBG("TEST_GPIO_LEVEL_LOW INT\n");
	}else if(test->arg[1] ==TEST_GPIO_LEVEL_HIGH){
		init.mode = GPIO_MODE_INT_LEVEL_H;
		if(test->arg[2]==TEST_GPIO_PULLUP){
		    init.pull = GPIO_PULLUP;
			HAL_TEST_DBG("TEST GPIO PULLUP ");
		}if(test->arg[2]==TEST_GPIO_PULLDOWN){
			init.pull = GPIO_PULLDOWN;
			HAL_TEST_DBG("TEST GPIO PULLDOWN ");
		}if(test->arg[2]==TEST_GPIO_NOPULL){
			init.pull = GPIO_NOPULL;
			HAL_TEST_DBG("TEST GPIO NOPULL ");
		}
		HAL_TEST_DBG("TEST_GPIO_LEVEL_HIGH INT\n");
	}
	
    s907x_hal_gpio_init(gpio_pin, &init);  
    
    s907x_hal_gpio_it_start(gpio_pin, gpio_user_isr, &gpio_pin);     
}

void gpio_pull_test(hal_test_t *test)
{
	u32 gpio_pin;
	u8 pull;
	ASSERT(test);
	
	gpio_pin = BIT(test->arg[0]);
	if(test->arg[1]==TEST_GPIO_PULLUP){
		pull = GPIO_PULLUP;
		HAL_TEST_DBG("TEST GPIO PULLUP ");
	}if(test->arg[1]==TEST_GPIO_PULLDOWN){
		pull = GPIO_PULLDOWN;
		HAL_TEST_DBG("TEST GPIO PULLDOWN ");
	}if(test->arg[1]==TEST_GPIO_NOPULL){
		pull = GPIO_NOPULL;
		HAL_TEST_DBG("TEST GPIO NOPULL ");
	}

	s907x_hal_gpio_set_pull(gpio_pin,pull);
}

void gpio_togglepin_test(hal_test_t *test)
{
	u32 gpio_pin;
	
	ASSERT(test);
	
	gpio_pin = BIT(test->arg[0]);
	s907x_hal_gpio_togglepin(gpio_pin);

}

void gpio_dinit_test(hal_test_t *test)
{
	u32 gpio_pin;
	
	ASSERT(test);
	
	gpio_pin = BIT(test->arg[0]);

	s907x_hal_gpio_deinit(gpio_pin);

}


void gpio_test(hal_test_t *test)
{
	ASSERT(test);
	ASSERT(test->no < (sizeof(gpio_test_map)/sizeof(hal_test_name_map_t)));

	HAL_TEST_DBG("test no %d = %s\n", gpio_test_map[test->no].type, gpio_test_map[test->no].name);
    
	switch(test->no) 
	{
		case GPIO_INIT:
			gpio_init_test(test);
			break;
		case GPIO_RW:
			gpio_rw_test(test);
			break;
		case GPIO_PULL:
			gpio_pull_test(test);
			break;
        case GPIO_INT:
            gpio_int_test(test); 
            break;
		case GPIO_TOGPIN:
			gpio_togglepin_test(test);
			break;
		case GPIO_DINIT:
			gpio_dinit_test(test);
			break;
		default:

		break;
	}
    
    
}

#endif

void at_test_led_init(u8 id)
{
		u8 led_sel = id;
		gpio_init_t init;
		u32 gpio_pin;

		init.pull = GPIO_PULLUP;
		init.mode = GPIO_MODE_OUTPUT;

		if(led_sel == 1)
		{
			//LED6
			gpio_pin = BIT(5);
		}
		else if(led_sel == 0)
		{
			//LED7
			gpio_pin = BIT(6);	
		}
			
		s907x_hal_gpio_init(gpio_pin, &init);
		HAL_TEST_DBG("led init sucess...\n");
}

void at_test_led_deint(u8 id)
{
		u8 led_sel = id;
		//gpio_init_t init;
		u32 gpio_pin;
		
		if(led_sel == 1)
		{
			//LED6
			gpio_pin = BIT(5);
		}
		else if(led_sel == 0)
		{
			//LED7
			gpio_pin = BIT(6);	
		}
		
		s907x_hal_gpio_deinit(gpio_pin);
}

void syn_gpio_isr(void *context)
{
	u32 *gpio_pin = (u32 *)context;
	#if M_AT_TEST
	//post sema
	if(!SYN)
		wl_send_sema_fromisr(&clock_syn_sema);
	else
		wl_send_sema_fromisr(&clock_syn_b_sema);
	#endif
	s907x_hal_gpio_it_stop(*gpio_pin);	
}


void at_clock_syn_init(u8 mode, u8 pin)
{
	u32 gpio_pin;
	gpio_init_t init;
	
	u8 trap = mode;
	
	gpio_pin = BIT(pin);
	if(!trap){//output	
		
		init.mode = GPIO_MODE_OUTPUT;
		init.pull = GPIO_PULLUP;
	}else{
	
		init.mode = GPIO_MODE_INT_FALLING;
		init.pull = GPIO_PULLUP;
	}
	
	s907x_hal_gpio_init(gpio_pin, &init);  
    
	if(trap)
		s907x_hal_gpio_it_stop(gpio_pin);
    	//hal_gpio_it_start(gpio_pin, syn_gpio_isr, &gpio_pin); 
}

void at_clock_i2c_syn_init(u8 mode, u8 pin)
{
	u32 gpio_pin;
	gpio_init_t init;
	
	u8 trap = mode;
	
	gpio_pin = BIT(pin);
	if(!trap){//output	
		
		init.mode = GPIO_MODE_OUTPUT;
		init.pull = GPIO_PULLUP;
	}else{
	
		init.mode = GPIO_MODE_INPUT;//GPIO_MODE_INT_FALLING
		init.pull = GPIO_PULLUP;
	}
	
	s907x_hal_gpio_init(gpio_pin, &init);  
    
	if(trap)
		s907x_hal_gpio_it_stop(gpio_pin);
    	//hal_gpio_it_start(gpio_pin, syn_gpio_isr, &gpio_pin); 
}

void at_gpio_togglepin(u8 pin)
{
	u32 gpio_pin;
	
	gpio_pin = BIT(pin);
	s907x_hal_gpio_togglepin(gpio_pin);

}

void at_gpio_write(u8 pin, gpio_status_e sta)
{
	u32 gpio_pin;
	
	gpio_pin = BIT(pin);
	
	s907x_hal_gpio_write(gpio_pin, sta);
}


void at_gpio_it_start(u8 pin)
{
	u32 gpio_pin;
	
	gpio_pin = BIT(pin);
	
	s907x_hal_gpio_it_start(gpio_pin, syn_gpio_isr, &gpio_pin);
}



