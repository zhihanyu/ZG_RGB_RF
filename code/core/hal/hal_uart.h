#ifndef HAL_UART_H
#define HAL_UART_H


//S9070 uarts
#define UART_0				  				(0U)
#define UART_LOG                			(1U)  //loguart
#define UART_MAX_ID			  				(UART_LOG)
#define IS_UART_ID_VALID(x)   				(((x) <= UART_MAX_ID) ? TRUE : FALSE)	

//data length
#define UART_DATALENGTH_7B                  ((uint32_t)0U)
#define UART_DATALENGTH_8B                  ((uint32_t)1U)

#define IS_UART_DATA_LENGTH(LENGTH) 		(((LENGTH) == UART_DATALENGTH_7B) || \
                                     		((LENGTH) == UART_DATALENGTH_8B))
//stop bits
#define UART_STOPBITS_1                     ((uint32_t)0U)
#define UART_STOPBITS_2                     ((uint32_t)1U)

#define IS_UART_STOPBITS(STOPBITS) 			(((STOPBITS) == UART_STOPBITS_1) || \
                                    		((STOPBITS) == UART_STOPBITS_2) )
//parity
#define UART_PARITY_NONE                    ((uint32_t)0U)
#define UART_PARITY_EVEN                    ((uint32_t)1U)
#define UART_PARITY_ODD                     ((uint32_t)2U) 

#define IS_UART_PARITY(PARITY) 				(((PARITY) == UART_PARITY_NONE) || \
                               				((PARITY) == UART_PARITY_EVEN) || \
                               				((PARITY) == UART_PARITY_ODD))

#define UART_FIFO_SIZE                      (16)

//uart interrupt 
#define UART_IT_ERROR                       (BIT0)
#define UART_IT_TXE                         (BIT1)
#define UART_IT_RXNE                        (BIT2)
#define UART_IT_RXTO 	                    (BIT3)
#if UART_MONITOR
#define UART_IT_MD                          (BIT4)
#define UART_IT_MS							(BIT5)
#endif
#define UART_IT_ALL							(BIT_ALL)

											//power level
#define UART_LP_DISABLE 0             		//disable
#define UART_LP_LV0		1					//higher power 
#define UART_LP_LV1		2					//lower power
#define IS_UART_LPLV(LV) 					(((LV) == UART_LP_DISABLE) || \
                                			((LV) == UART_LP_LV0) || \
                                			((LV) == UART_LP_LV1))
                                			
//rx thd
#define UART_RX_THD_1BYTES					(0)
#define UART_RX_THD_4BYTES					(1)
#define UART_RX_THD_8BYTES					(2)
#define UART_RX_THD_14BYTES					(3)

typedef struct uart_dma_
{
	u8 rx_burst_size;
	u8 tx_burst_size;
	hal_cb_t rx_complete;
	hal_cb_t tx_complete;
	u8 *rxbuf;
	u32 rxlen;
	u32 rx_complete_len;
	u8 *last_rx_addr;
	u8 *txbuf;
	u32 txlen;
	u32 tx_complete_len;
    u32 rx_timeout_cnt;
}uart_dma_t;

typedef struct uart_int_
{
	hal_cb_t trx_error;
	hal_cb_t tx_complete;
	hal_cb_t rx_complete;
	hal_cb_t rx_timeout;
	u8 *txbuf;
	u32 txlen;
	u8 *rxbuf;
	u32 rxlen;
}uart_int_t;

typedef struct uart_config_
{
	u32 idx;  //rsvd for s90xx
	u32 parity;	
	u32 datalen;
	u32 stopbits;
	u32 baud;
	u32 lpmode;
	u32 tx_thd;
	u32 rx_thd;
}uart_config_t;


typedef struct uart_hdl_
{
	u32 state;
	uart_config_t  config;	
	uart_int_t it;
	uart_dma_t dma;
}uart_hdl_t;

//send recv always wait
#define uart_tx(a, b, c) s907x_hal_uart_tx((a), (b), (c), HAL_MAX_DELAY)
#define uart_rx(a, b, c) s907x_hal_uart_rx((a), (b), (c), HAL_MAX_DELAY)

//low level api
u8   uart_ll_rx_allow(void *uart);
u8   uart_ll_tx_allow(void *uart);
void uart_ll_send_byte(void *uart, u8 byte);
u8   uart_ll_recv_byte(void *uart);

//hal api
hal_status_e s907x_hal_uart_init(uart_hdl_t *uart);
hal_status_e s907x_hal_uart_deinit(uart_hdl_t *uart);
int			 s907x_hal_uart_tx(uart_hdl_t *uart, u8 *pbuf, uint16_t size, uint32_t timeout);
int 		 s907x_hal_uart_rx(uart_hdl_t *uart, u8 *pbuf, uint16_t size, uint32_t timeout);
hal_status_e s907x_hal_uart_tx_it(uart_hdl_t *uart, u8 *pbuf, uint16_t size);
hal_status_e s907x_hal_uart_rx_it_to(uart_hdl_t *uart, u8*pbuf);
hal_status_e s907x_hal_uart_rx_it(uart_hdl_t *uart, u8 *pbuf, uint16_t size);
hal_status_e s907x_hal_uart_tx_dma(uart_hdl_t *uart, u8 *pbuf, uint16_t size);
hal_status_e s907x_hal_uart_rx_dma(uart_hdl_t *uart, u8 *pbuf, uint16_t size);
u32 		 s907x_hal_uart_rx_dma_adddress(uart_hdl_t *uart); 
hal_status_e s907x_hal_uart_dma_txstop(uart_hdl_t *uart);
hal_status_e s907x_hal_uart_dma_rxstop(uart_hdl_t *uart);


#endif
