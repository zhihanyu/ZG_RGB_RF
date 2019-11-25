#ifndef GPIO_TEST_H
#define GPIO_TEST_H



#define TEST_GPIO_READ  			0
#define TEST_GPIO_WRITE 			1
#define TEST_GPIO_TRIGGER_RISING 	2
#define TEST_GPIO_TRIGGER_FALLING 	3
#define TEST_GPIO_LEVEL_LOW		4
#define TEST_GPIO_LEVEL_HIGH 		5

#define TEST_GPIO_PULLUP		0
#define TEST_GPIO_PULLDOWN		1
#define TEST_GPIO_NOPULL		2

#define	GPIO_SYN				7
#define	GPIO_SYN_B				17

typedef enum GPIO_TEST{
	GPIO_INIT = 0,
	GPIO_RW = 1,
	GPIO_PULL = 2,
	GPIO_INT = 3,
	GPIO_TOGPIN = 4,
	GPIO_DINIT = 5,
}gpio_test_e;


typedef enum gpio_syn_
{
	out_sel = 0,
	in_sel = 1,
}gpio_syn_e;

void at_test_led_init(u8 id);
void at_test_led_deint(u8 id);
void at_clock_syn_init(u8 mode, u8 pin);

void at_gpio_togglepin(u8 pin);
void at_gpio_write(u8 pin, gpio_status_e sta);
void syn_gpio_isr(void *context);
void at_gpio_it_start(u8 pin);
void at_clock_i2c_syn_init(u8 mode, u8 pin);










#endif