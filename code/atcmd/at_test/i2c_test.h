#ifndef I2C_TEST_H
#define I2C_TEST_H


#define I2C_ERROR  	1
#define I2C_SUCESS 	0

#define I2C_RECV_ING    	0
#define I2C_RECV_FINISH 	1

#define I2C_SEND_ING 		0
#define I2C_SEND_FINISH 	1

#define I2C_TEST_I2C0       1  //for master
#define I2C_TEST_I2C1       2  //for slaver
#define I2C_TEST_LOOPBACK   3
#define	I2C_PERFORMACE_TEST 4

#define I2C_MST_IDX			I2C_TEST_I2C0
#define I2C_SLV_IDX			I2C_TEST_I2C1


#define I2C_MST_POLL_WAIT   (1000)
//pending
#define I2C_SLV_POLL_WAIT	(1000) 

typedef enum TM_TYPE{
	I2C_TEST_POLL = 0,
	I2C_TEST_INTERRUPT = 1,
	I2C_TEST_DMA = 2,
}I2C_TS_TYPE;


void i2c_test_poll( hal_test_t *test);
void i2c_test_interrupt(hal_test_t *test);
void i2c_test_dma( hal_test_t *test);
void i2c_test(hal_test_t *test);

#endif
