#ifndef HAL_I2S_H
#define HAL_I2S_H





//data len
#define I2S_DATALEN_16		0
#define I2S_DATALEN_24		1

//format	
#define I2S_FORMAT_LEFT		0
#define I2S_FORMAT_RIGHT	1
#define I2S_FORMAT_I2S		2


//simpling rate
#define I2S_SMP_RATE_8KHZ							0
#define I2S_SMP_RATE_16KHZ							1
#define I2S_SMP_RATE_24KHZ							2	
#define I2S_SMP_RATE_32KHZ							3	
#define I2S_SMP_RATE_48KHZ							4
#define I2S_SMP_RATE_96KHZ							5

//direction
#define I2S_TX_ONLY						0	
#define I2S_RX_ONLY						1
#define I2S_TRX							2					

//internal
#define I2S_LOOPBACK_EXTERNAL			0
#define I2S_LOOPBACK_INTERNAL			1

//channel
#define I2S_CHN_STEREO	   				0
#define I2S_CHN_RSVD	   				1
#define I2S_CHN_MONO	   				2

#define IS_I2S_CHN_NUM(NUM) (((NUM) == I2S_CHN_STEREO) || \
								((NUM) == I2S_CHN_MONO))

#define I2S_ISR_ALL			0
#define I2S_ISR_BITS		1

#define I2S_RX_PAGE			0
#define I2S_TX_PAGE			1

//mode
#define I2S_MASTER_IDX 		0
#define I2S_SLAVE_IDX		1

typedef void (*i2s_int_cb)(void *);

typedef struct i2s_config_
{
	u8  mode;
	u8  sampling_rate;
	u8  datalen;
	u8  direction;	
	u8 	internal;
	u8 format;
	u16	page_num;
	u16 page_size;
	u16 channel;
}i2s_config_t;



typedef struct i2s_it_
{
	hal_cb_t tx_complete;
	hal_cb_t rx_complete;
	hal_cb_t error_cb;
}i2s_it_t;


typedef struct i2s_stereo_
{
	u32 left;	//when i2s playtone, set stereo left channel init value
	u32 right;	//when i2s playtone, set stereo right channel init value
}i2s_stereo_t;


typedef struct i2s_page_
{
	u32 tx_mask;
	u32 tx_addr;
	u32 tx_now;
	u32 rx_mask;
	u32 rx_addr;
	u32 rx_now;
}i2s_page_t;

typedef struct i2s_hdl_
{
	i2s_config_t  config;	
	i2s_it_t      it;
	i2s_page_t    page;
	i2s_stereo_t  stereo;
	u32           mono;
}i2s_hdl_t;




hal_status_e s907x_hal_i2s_init(i2s_hdl_t *i2s,u8 *tx_buf, u8 *rx_buf);
hal_status_e s907x_hal_i2s_deinit(i2s_hdl_t *i2s);
hal_status_e s907x_hal_i2s_enable(void);
hal_status_e s907x_hal_i2s_disable(void);
u16 s907x_hal_i2s_get_pg_sz(void);
hal_status_e s907x_hal_i2s_send_page(i2s_hdl_t *i2s);
hal_status_e s907x_hal_i2s_init_trxpage_buf(i2s_hdl_t *i2s, u8 *tx_buf,u8 *rx_buf,u32 page_num,u32 page_size);
hal_status_e s907x_hal_i2s_recv_page(i2s_hdl_t *i2s);
hal_status_e s907x_hal_i2s_set_param(i2s_hdl_t *i2s , int channel_num,int rate,int word_len);
hal_status_e s907x_hal_i2s_set_direction(i2s_hdl_t *i2s, int type);
void s907x_hal_i2s_set_trx_buf(u8 *tx_buf,u8 *rx_buf);
void s907x_hal_i2s_get_pg_info(i2s_hdl_t *i2s,u32 mode);
void s907x_hal_i2s_int_enable(void);
void s907x_hal_i2s_int_clr(i2s_hdl_t *i2s);
void s907x_hal_i2s_tx_dma_enable(void);















#endif
