#ifndef HAL_RCC_H
#define HAL_RCC_H


 

//peripheral select 
#define RCC_PERIPH_ADC  	 BIT(0)
#define RCC_PERIPH_UART		 BIT(1)
#define RCC_PERIPH_I2C0      BIT(2)
#define RCC_PERIPH_I2C1      BIT(4)
#define RCC_PERIPH_SPI0      BIT(5)
#define RCC_PERIPH_SPI1      BIT(6)
#define RCC_PERIPH_I2S       BIT(7)
#define RCC_PERIPH_LOGUART   BIT(8)
#define RCC_PERIPH_RTC       BIT(9)
#define RCC_PERIPH_GPIO      BIT(10)
#define RCC_PERIPH_TIMER1    BIT(11)

#define M_RCC_PERIPH_ADC       BIT(0)
#define M_RCC_PERIPH_UART      BIT(1)
#define M_RCC_PERIPH_I2C0      BIT(2)
#define M_RCC_PERIPH_I2C1      BIT(4)
#define M_RCC_PERIPH_SPI0      BIT(5)
#define M_RCC_PERIPH_SPI1      BIT(6)
#define M_RCC_PERIPH_I2S       BIT(7)
#define M_RCC_PERIPH_LOGUART   BIT(8)
#define M_RCC_PERIPH_RTC       BIT(9)
#define M_RCC_PERIPH_GPIO      BIT(10)
#define M_RCC_PERIPH_TIMER1    BIT(11)

#define LOGUART_TIMER_PRIO      10



void rcc_clock_switch(u32 peripheral, hal_switch_e enable);
































#endif

