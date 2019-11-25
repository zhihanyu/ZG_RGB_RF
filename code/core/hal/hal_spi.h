#ifndef HAL_SPI_H
#define HAL_SPI_H



#define SPI_IDX_0				  (0)   //salve only
#define SPI_IDX_1				  (1)	//master only
#define SPI_IDX_MAX               (SPI_IDX_1)
#define IS_SPI_IDX(id)			  ((id) == SPI_IDX_0 || \
								  (id) == SPI_IDX_1)

#define SPI_SLAVE_IDX			   SPI_IDX_0
#define SPI_MASTER_IDX			   SPI_IDX_1

#define SPI_MODE_TX			       (BIT(0))	
#define SPI_MODE_RX			       (BIT(1))	
#define SPI_MODE_TXRX		       (BIT(2))	
#define IS_SPI_MODE(mode)		   ((mode) == SPI_MODE_TX  || \
								   (mode) == SPI_MODE_RX  || \
								   (mode) == SPI_MODE_TXRX)

#define SPI_DATASIZE_4BIT          ((uint32_t)BIT0) /*!< SPI Datasize = 4bits   */
#define SPI_DATASIZE_5BIT          ((uint32_t)BIT1) /*!< SPI Datasize = 5bits   */
#define SPI_DATASIZE_6BIT          ((uint32_t)BIT2) /*!< SPI Datasize = 6bits   */
#define SPI_DATASIZE_7BIT          ((uint32_t)BIT3) /*!< SPI Datasize = 7bits   */
#define SPI_DATASIZE_8BIT          ((uint32_t)BIT4) /*!< SPI Datasize = 8bits   */
#define SPI_DATASIZE_9BIT          ((uint32_t)BIT5) /*!< SPI Datasize = 9bits   */
#define SPI_DATASIZE_10BIT         ((uint32_t)BIT6) /*!< SPI Datasize = 10bits   */
#define SPI_DATASIZE_11BIT         ((uint32_t)BIT7) /*!< SPI Datasize = 11bits   */
#define SPI_DATASIZE_12BIT         ((uint32_t)BIT8) /*!< SPI Datasize = 12bits   */
#define SPI_DATASIZE_13BIT         ((uint32_t)BIT9) /*!< SPI Datasize = 13bits   */
#define SPI_DATASIZE_14BIT         ((uint32_t)BIT10) /*!< SPI Datasize = 14bits   */
#define SPI_DATASIZE_15BIT         ((uint32_t)BIT11) /*!< SPI Datasize = 15bits   */
#define SPI_DATASIZE_16BIT         ((uint32_t)BIT12) /*!< SPI Datasize = 16bits   */



#define IS_SPI_DATASIZE(DATASIZE) (((DATASIZE) == SPI_DATASIZE_16BIT) || \
                                   ((DATASIZE) == SPI_DATASIZE_15BIT) || \
                                   ((DATASIZE) == SPI_DATASIZE_14BIT) || \
                                   ((DATASIZE) == SPI_DATASIZE_13BIT) || \
                                   ((DATASIZE) == SPI_DATASIZE_12BIT) || \
                                   ((DATASIZE) == SPI_DATASIZE_11BIT) || \
                                   ((DATASIZE) == SPI_DATASIZE_10BIT) || \
                                   ((DATASIZE) == SPI_DATASIZE_9BIT)  || \
                                   ((DATASIZE) == SPI_DATASIZE_8BIT)  || \
                                   ((DATASIZE) == SPI_DATASIZE_7BIT)  || \
                                   ((DATASIZE) == SPI_DATASIZE_6BIT)  || \
                                   ((DATASIZE) == SPI_DATASIZE_5BIT)  || \
                                   ((DATASIZE) == SPI_DATASIZE_4BIT))



/** @defgroup SPI_Clock_Phase SPI Clock Phase
  * @{
  */
#define SPI_PHASE_1EDGE            ((uint32_t)0x00000000) /*!< SPI Phase 1EDGE  */
#define SPI_PHASE_2EDGE            ((uint32_t)0x00000001) /*!< SPI Phase 2EDGE  */
#define IS_SPI_CPHA(CPHA)		   (((CPHA) == SPI_PHASE_1EDGE) || \
                           		   ((CPHA) == SPI_PHASE_2EDGE))


/** @defgroup SPI_Clock_Polarity SPI Clock Polarity
  * @{
  */ 
#define SPI_POLARITY_LOW           ((uint32_t)0x00000000) /*!< SPI polarity Low  */
#define SPI_POLARITY_HIGH          ((uint32_t)0x00000001)           /*!< SPI polarity High */
#define IS_SPI_CPOL(CPOL)		   (((CPOL) == SPI_POLARITY_LOW) || \
                                   ((CPOL) == SPI_POLARITY_HIGH))

#define SPI_IT_RXNE					BIT0
#define SPI_IT_RXOV					BIT1
#define SPI_IT_TXE					BIT2
#define SPI_IT_TXOV					BIT3
#define SPI_IT_OV					BIT4


#define BIT_ISR_TXEIS				((u32)0x00000001)
#define BIT_ISR_TXOIS				((u32)0x00000001 << 1)
#define BIT_ISR_RXUIS				((u32)0x00000001 << 2)
#define BIT_ISR_RXOIS				((u32)0x00000001 << 3)
#define BIT_ISR_RXFIS				((u32)0x00000001 << 4)
#define BIT_ISR_MSTIS				((u32)0x00000001 << 5)

typedef struct spi_config_
{
	u8  idx;
	u8  spi_master;
	u16 dir;
	u32 mode;
	u32 dataframesize;
	u32 clk_speed;
	u32 clk_phase;
	u32 clk_polarity;
	u32 datalen;
}spi_config_t;

typedef struct spi_it_
{
 	void *txbuf;
	u16 txlen;
	void *rxbuf;
	u16 rxlen;
	u16 rxreq;
	hal_cb_t tx_complete;
	hal_cb_t rx_complete;
	hal_cb_t error_cb;
	void *hk;
}spi_it_t;


typedef struct spi_dma_
{
	u8 rx_burst_size;
	u8 tx_burst_size;
	hal_cb_t rx_complete;
	hal_cb_t tx_complete;
	u8 *rxbuf;
	u32 rxlen;
	u32 rx_complete_len;
	u8 *txbuf;
	u32 txlen;
	u32 tx_complete_len;
}spi_dma_t;


typedef struct spi_hdl_
{
	spi_config_t  config;	
	spi_it_t      it;
	spi_dma_t     dma;	
	u32 error_code;
	u32 status;
}spi_hdl_t;













hal_status_e s907x_hal_spi_init(spi_hdl_t *spi);
hal_status_e s907x_hal_spi_deinit(spi_hdl_t *spi);
//poll mode
int			 s907x_hal_spi_master_txrx(spi_hdl_t *spi, void *txbuf, void *rxbuf, u16 xfer_size, uint32_t ms);
int 		 s907x_hal_spi_slaver_tx(spi_hdl_t* spi, void *txbuf, u16 xfer_size, uint32_t timeout);
int 		 s907x_hal_spi_slaver_rx(spi_hdl_t* spi, void *rxbuf, u16 xfer_size, uint32_t timeout);
//interrupt mode
hal_status_e s907x_hal_spi_master_txrx_int(spi_hdl_t* spi, void *txbuf, void *rxbuf, u16 xfer_size);
hal_status_e s907x_hal_spi_master_xfer_interrupt(spi_hdl_t *spi, u8 *pbuf, u16 xfer_size);
hal_status_e s907x_hal_spi_master_recv_interrupt(spi_hdl_t *spi, void *pbuf, u16 xfer_size);
hal_status_e s907x_hal_spi_slaver_xfer_interrupt(spi_hdl_t *spi, u8 *pbuf, u16 xfer_size);
hal_status_e s907x_hal_spi_slaver_recv_interrupt(spi_hdl_t *spi, u8 *pbuf, u16 xfer_size);
//dma mode
hal_status_e s907x_hal_spi_master_txrx_dma(spi_hdl_t* spi, void *txbuf, void *rxbuf, u16 xfer_size);
hal_status_e s907x_hal_spi_master_xfer_dma(spi_hdl_t *spi, void *pbuf, u16 xfer_size);
hal_status_e s907x_hal_spi_master_recv_dma(spi_hdl_t *spi, u8 *pbuf, u16 xfer_size);
hal_status_e s907x_hal_spi_slaver_xfer_dma(spi_hdl_t *spi, u8 *pbuf, u16 xfer_size);
hal_status_e s907x_hal_spi_slaver_recv_dma(spi_hdl_t *spi, u8 *pbuf, u16 xfer_size);





#endif
