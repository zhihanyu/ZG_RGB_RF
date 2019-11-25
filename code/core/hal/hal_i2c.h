#ifndef HAL_I2C_H
#define HAL_I2C_H





typedef enum
{
  HAL_I2C_STATE_RESET           = 0x00,  /*!< I2C not yet initialized or disabled         */
  HAL_I2C_STATE_READY           = 0x01,  /*!< I2C initialized and ready for use           */
  HAL_I2C_STATE_BUSY            = 0x02,  /*!< I2C internal process is ongoing             */
  HAL_I2C_STATE_MASTER_BUSY_TX  = 0x12,  /*!< Master Data Transmission process is ongoing */ 
  HAL_I2C_STATE_MASTER_BUSY_RX  = 0x22,  /*!< Master Data Reception process is ongoing    */
  HAL_I2C_STATE_SLAVE_BUSY_TX   = 0x32,  /*!< Slave Data Transmission process is ongoing  */ 
  HAL_I2C_STATE_SLAVE_BUSY_RX   = 0x42,  /*!< Slave Data Reception process is ongoing     */
  HAL_I2C_STATE_MEM_BUSY_TX     = 0x52,  /*!< Memory Data Transmission process is ongoing */ 
  HAL_I2C_STATE_MEM_BUSY_RX     = 0x62,  /*!< Memory Data Reception process is ongoing    */  
  HAL_I2C_STATE_TIMEOUT         = 0x03,  /*!< Timeout state                               */  
  HAL_I2C_STATE_ERROR           = 0x04   /*!< Reception process is ongoing                */                                                            
}i2c_state_e;

#define I2C_IDX_0				 0
#define I2C_IDX_1				 1
#define I2C_IDX_MAX              I2C_IDX_1
#define IS_I2C_IDX(id)			 ((id) == I2C_IDX_0 || \
								  (id) == I2C_IDX_1)


#define I2C_MASTER_CLK_20K		 (20)		  
#define I2C_MASTER_CLK_100K		 (100)
#define I2C_MASTER_CLK_400K		 (400)
#define I2C_MASTER_CLK_1000K	 (1000)



#define I2C_ADDR_MODE_7BIT		 ((0))
#define I2C_ADDR_MODE_10BIT		 ((1))
#define IS_I2C_ADDR_MODE(mode)	 ((mode) == I2C_ADDR_MODE_7BIT ||	\
								 (mode) == I2C_ADDR_MODE_10BIT)


#define I2C_WAIT_BUS_IDLE         2000


//interrupt mask


#define  I2C_IT_STARTF                      (BIT5)        /*!< START detection flag */
#define  I2C_IT_STOPF                       (BIT5)        /*!< STOP detection flag */
#define  I2C_IT_TXE                         (BIT0)        /*!< Transmit data register empty */
#define  I2C_IT_RC                        	(BIT3)        /*!< Receive data complete */
#define  I2C_IT_RXF							(BIT4)             
#define  I2C_IT_TXF							(BIT5)
#define  I2C_IT_OVR                         (BIT2)        /*!< Overrun/Underrun */
#define  I2C_IT_GC							(BIT4)        /*!< Bus error */
#define  I2C_IT_TXABT						(BIT7)		  /*!< tx abort */
#define  I2C_IT_ALL                         (BIT_ALL)
#define  I2C_IT_BERR                        (BIT8)        /*!< Bus error */
#define  I2C_IT_RXUNDER                     (BIT10)
#define  I2C_IT_RQ                          (BIT11)       /*!< read request */

#define  I2C_IT_PECERR                      (BIT11)        /*!< PEC error in reception */
#define  I2C_IT_TIMEOUT                     (BIT12)        /*!< Timeout or Tlow detection flag */
#define  I2C_IT_ALERT                       (BIT13)        /*!< SMBus alert */
#define  I2C_IT_BUSY                        (BIT14)        /*!< Bus busy */
#define  I2C_IT_DIR                         (BIT15)        /*!< Transfer direction (slave mode) */
#define  I2C_IT_ADDCODE                     (BIT16)        /*!< Address match code (slave mode) */


//status mask
#define I2C_STA_BUSY                    BIT0
#define I2C_STA_ACTIVY                  BIT1
#define I2C_STA_RXNE					BIT2
#define I2C_STA_RXF					 	BIT3
#define I2C_STA_TXE					 	BIT4
#define I2C_STA_TXNF					BIT5
#define I2C_STA_RXS						BIT6
#define I2C_STA_TXABT					BIT7

//I2C ERROR code
#define I2C_ERROR_TX_OVERRUN       		0
#define I2C_ERROR_RX_OVERRUN       		1

#define I2C_DMA_DATA_MAX_LENGTH			4096

typedef struct i2c_config_
{
	u8  idx;
	u8  i2c_master;
	u16 dir;
	u16 clock;
	u16 addr_mode;
	u16 own_addr;
	u16 target_addr;
	u16 general_call;
}i2c_config_t;

typedef struct i2c_it_
{
 	u8 *txbuf;
	u16 txlen;
	u8 *rxbuf;
	u16 rxlen;
	u16 rxreq;
	u32 rst;
	u32 stop;
	hal_cb_t gc;
	hal_cb_t tx_complete;
	hal_cb_t rx_complete;
	hal_cb_t error_cb;
	void *hk;
}i2c_it_t;


typedef struct i2c_dma_
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
}i2c_dma_t;


typedef struct i2c_hdl_
{		
	u32           status;
	u32           error_code;
	i2c_config_t  config;
	i2c_it_t      it;
	i2c_dma_t  	  dma; 
}i2c_hdl_t;






hal_status_e s907x_hal_i2c_init(i2c_hdl_t *i2c);
hal_status_e s907x_hal_i2c_deinit(i2c_hdl_t *i2c);
//poll mode
hal_status_e s907x_hal_i2c_master_xfer(i2c_hdl_t *hi2c, u8 *pbuf, u16 xfer_size, uint32_t ms);
hal_status_e s907x_hal_i2c_master_recv(i2c_hdl_t *hi2c, u8 *pbuf, u16 xfer_size, uint32_t ms);
hal_status_e s907x_hal_i2c_slavor_xfer(i2c_hdl_t *hi2c, u8 *pbuf, u16 xfer_size, uint32_t ms);
hal_status_e s907x_hal_i2c_slavor_recv(i2c_hdl_t *hi2c, u8 *pbuf, u16 xfer_size, uint32_t ms);
//interrupt mode
hal_status_e s907x_hal_i2c_master_xfer_interrupt(i2c_hdl_t *hi2c, u8 *pbuf, u16 xfer_size);
hal_status_e s907x_hal_i2c_master_recv_interrupt(i2c_hdl_t *hi2c, u8 *pbuf, u16 xfer_size);
hal_status_e s907x_hal_i2c_slavor_xfer_interrupt(i2c_hdl_t *hi2c, u8 *pbuf, u16 xfer_size);
hal_status_e s907x_hal_i2c_slavor_recv_interrupt(i2c_hdl_t *hi2c, u8 *pbuf, u16 xfer_size);
//dma mode
hal_status_e s907x_hal_i2c_master_xfer_dma(i2c_hdl_t *hi2c, u8 *pbuf, u16 xfer_size);
hal_status_e s907x_hal_i2c_master_recv_dma(i2c_hdl_t *hi2c, u8 *pbuf, u16 xfer_size);
hal_status_e s907x_hal_i2c_slavor_xfer_dma(i2c_hdl_t *hi2c, u8 *pbuf, u16 xfer_size);
hal_status_e s907x_hal_i2c_slavor_recv_dma(i2c_hdl_t *hi2c, u8 *pbuf, u16 xfer_size);





#endif
