#ifndef SPI_TEST_H
#define SPI_TEST_H



#define SPI_TX 0
#define SPI_RX 1

#define SPI_ERROR  1
#define SPI_SUCESS 0

#define SPI_RECV_ING    0
#define SPI_RECV_FINISH 1

#define SPI_SEND_ING 	0
#define SPI_SEND_FINISH 1

#define SPI_POLL_LIMIT  (1000*1000)


typedef enum SPI_TEST_{
	SPI_TEST_BASIC_TRX = 0,
	SPI_TEST_INTERRUPT_TRX = 1,
	SPI_TEST_DMA_TRX = 2,
	SPI_PERFORMANCE_TEST = 3,
}SPI_TEST_ITEM;
	


typedef enum SPI_TEST_MODE_{
	SPI_TEST_MASTER = 1,
	SPI_TEST_SLAVER = 2,
	SPI_TEST_LOOPBACK = 3,
}SPI_TEST_MODE;














void spi_test(hal_test_t *test);















#endif
