#ifndef WLAN_UART_UTILS_H
#define WLAN_UART_UTILS_H

#include "at_common.h"
 
//2*6*2k = 24k
#define WLAN_BUF_MALLOC        0
#define UART_TX_MALLOC         0

#define UART_BUF_TX_NR         3
#define UART_BUF_RX_NR         3   
#define UART_MAX_TRANSF_SIZE   AT_NET_MAX_RECV_SEND_BUFF_SIZE
#define UART_PRIO_BUF_TX_NR    2
#define UART_PRIO_TRANSF_SZE   120

typedef struct {
    uint8_t      data[UART_MAX_TRANSF_SIZE];    /**< buffer of data */
    uint16_t     len;                           /**< number of bytes used in buffer */
}uart_buffer_t;

typedef struct uart_pool_{
    uart_buffer_t buffers[UART_BUF_TX_NR];
    uart_buffer_t *ongoing;
    RINGBUFFER(empty_r_buf, UART_BUF_TX_NR, uart_buffer_t *);
    RINGBUFFER(ready_r_buf, UART_BUF_TX_NR, uart_buffer_t *);
}uart_pool_t;

typedef struct {
    uint8_t     data[UART_PRIO_TRANSF_SZE];
    uint16_t    len;
}uart_prio_buffer_t;

typedef struct uart_prio_pool_{
    uart_prio_buffer_t buffers[UART_PRIO_BUF_TX_NR];
    uart_prio_buffer_t *ongoing;
    RINGBUFFER(empty_r_buf, UART_PRIO_BUF_TX_NR, uart_prio_buffer_t *);
    RINGBUFFER(ready_r_buf, UART_PRIO_BUF_TX_NR, uart_prio_buffer_t *);
}uart_prio_pool_t;

typedef uart_pool_t wlan_pool_t;

#if WLAN_BUF_MALLOC
wlan_pool_t *wlan_rx_ptr(void);
#else
uart_buffer_t *wlan_rx_ptr(void);
#endif
void wlan_uart_rx_ll_cb(void *context);
uart_pool_t *wlan_uart_tx_ptr(void);
uart_pool_t *wlan_uart_rx_ptr(void);
int wlan_uart_init_data(void);
int wlan_uart_free_data(void);
int wlan_uart_tx_take_buf(uart_pool_t *pool, uart_buffer_t ** buffer);
int wlan_uart_tx_release_buf(uart_pool_t *pool, uart_buffer_t ** buffer);
int wlan_uart_rx_take_buffer(uart_pool_t *pool, uart_buffer_t ** buffer);
int wlan_uart_rx_transfer_buffer(uart_pool_t *pool, uart_buffer_t ** buffer, int is_isr);
int wlan_uart_rx_release_buffer(uart_pool_t *pool,uart_buffer_t ** buffer);
int wlan_uart_rx_buffer_empty(void);
int wlan_uart_rx_hdl(void *context);
void wlan_uart_wait_for_send(void *context);
int wlan_uart_tx_hdl(uart_pool_t *pool, uart_buffer_t * buffer);
int wlan_uart_rx_pop(uart_pool_t *pool, int is_isr);
void wlan_uart_passth_event(void *context, u32 enable);

#endif