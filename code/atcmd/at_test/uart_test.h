#ifndef UART_TEST_H
#define UART_TEST_H






#define UART_POLL_MAX_WAIT  2000

#define UART_DMA_RX_TICK    1
#define UART_DMA_RX_TO      50
#define UART_DMA_RX_POLL    200






typedef enum UART_TEST{
	UART_POLLING_MODE = 0,
	UART_INT_MODE = 1,
	UART_INT_TIMEOUT = 2,
	UART_DMA_MODE = 3,
        UART_DMA_TXSTOP_MODE = 4,
		UART_PERFORMANCE_TEST = 5,
}UART_TEST_ITEM;






void uart_test(hal_test_t *test);















#endif