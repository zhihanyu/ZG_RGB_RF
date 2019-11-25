#include "s907x.h"
#include "i2s_test.h"
 
#if M_AT_TEST

static i2s_hdl_t i2s_type;
static u32 i2s_tx_buf[1000];
static u32 i2s_rx_buf[1000];
static u8 i2s_test_break = 0;
void s907x_hal_i2s_msp_init()
{
	I2S_MCK_SEL0(HAL_ON);
	I2S_SD_TX_SEL0(HAL_ON);
	I2S_SD_RX_SEL0(HAL_ON);
	I2S_SD_WS_SEL0(HAL_ON);
	I2S_CLK_SEL0(HAL_ON);
}

void s907x_hal_i2s_msp_deinit()
{
	I2S_MCK_SEL0(HAL_OFF);
	I2S_SD_TX_SEL0(HAL_OFF);
	I2S_SD_RX_SEL0(HAL_OFF);
	I2S_SD_WS_SEL0(HAL_OFF);
	I2S_CLK_SEL0(HAL_OFF);
}



void i2s_tx_complete(void *data)
{
	i2s_hdl_t *i2s = (i2s_hdl_t *)data;
	
	s907x_hal_i2s_send_page(i2s);
}

void i2s_rx_complete(void *data)
{
	i2s_hdl_t *i2s  = (i2s_hdl_t *)data;
	int i;
	u32 *rx_buf_24bit = (u32*)i2s_rx_buf;
	u16 *rx_buf_16bit = (u16*)i2s_rx_buf;
	u8* rx_buf_temp = (u8*)i2s_rx_buf;
	u32 pagesize_bytes = 0;
	u32 test_fail = 0;
	u32 PageSize = s907x_hal_i2s_get_pg_sz();
	u32 cur_rx_page = s907x_hal_i2s_get_cur_pg();

	pagesize_bytes = PageSize * 4;
	
	rx_buf_temp = rx_buf_temp + pagesize_bytes * cur_rx_page;
	
	if (i2s->config.datalen == I2S_DATALEN_24) {
		rx_buf_24bit = (u32*)rx_buf_temp;
		for (i = 0; i < PageSize; i++) {
			rx_buf_24bit[i] = rx_buf_24bit[i]&0xFFFFFF;
		}
		HAL_DBG_ARRARY((u8*)rx_buf_24bit, PageSize*4, ARY_U32, 8);
	} else if (i2s->config.datalen == I2S_DATALEN_16) {
		rx_buf_16bit = (u16*)rx_buf_temp;
		//HAL_DBG_ARRARY((u8*)rx_buf_16bit, PageSize * 2*2, ARY_U16, 8);
	} else {
		printf("unknown error happen!!!!\n");
	}

	s907x_hal_i2s_recv_page(i2s);
}


/*
*    fill tx buf with raw_pcm, clear rx buf, set enable and interrupt
*/
static void i2s_loopback_test(i2s_hdl_t *i2s)
{
	u32 i = 0;
	u32 *tx_buf_24bit = (u32*)i2s_tx_buf;
	u16 *tx_buf_16bit = (u16*)i2s_tx_buf;
	u32 PageSize = s907x_hal_i2s_get_pg_sz();

	/* fill buffer with raw pcm */
	if (i2s->config.datalen == I2S_DATALEN_24) {
		for (i=0 ; i< (i2s->config.page_num * PageSize); i++) {
			s32 raw_pcm;
			raw_pcm = ((i+101)*1705-67)&0xffffff;
			if (i%2 == 1)
			raw_pcm ^= 0x800000;
			tx_buf_24bit[i] = raw_pcm;
		}
	} else {
		for (i=0 ; i< (i2s->config.page_num * PageSize * 2); i++) {
			s16 raw_pcm;
			raw_pcm = (i+101)*30-67;
			if (i%2 == 1)
			raw_pcm ^= 0x8000;
			tx_buf_16bit[i] = raw_pcm;
		}
	}

	memset(i2s_rx_buf, 0, 1000);
	s907x_hal_i2s_set_trx_buf((u8*)i2s_tx_buf, (u8*)i2s_rx_buf);
	s907x_hal_i2s_tx_dma_enable();
	s907x_hal_i2s_int_enable();
	s907x_hal_i2s_int_clr(i2s);

	s907x_hal_i2s_enable();
}           

void i2s_rx_test(i2s_hdl_t *i2s)
{
	s907x_hal_i2s_set_trx_buf(NULL, (u8*)i2s_rx_buf);
	s907x_hal_i2s_int_enable();
	s907x_hal_i2s_int_clr(i2s);

	s907x_hal_i2s_enable();
}


void i2s_test_interrupt(hal_test_t *test)
{
	i2s_hdl_t *i2s = &i2s_type;
	if(test->arg[0] == 0){
		i2s->it.rx_complete.func = i2s_rx_complete;
		i2s->it.rx_complete.context = i2s;
		i2s_rx_test(i2s);
	}
	else if(test->arg[0] == 1){
		i2s->it.tx_complete.func = i2s_tx_complete;
		i2s->it.tx_complete.context = i2s;
		i2s->it.rx_complete.func = i2s_rx_complete;
		i2s->it.rx_complete.context = i2s;
		
		i2s_loopback_test(i2s);
	}
	else if(test->arg[0] == 2){
		i2s_ll_set_it_enable(i2s,DISABLE);
		s907x_hal_i2s_disable();
	}
}            



static void i2s_test_enter(hal_test_t *test)
{
	int mode,i;
	i2s_hdl_t *i2s = &i2s_type;

	ASSERT(test);
	
	// config i2s_type
	i2s->config.mode =(u8)test->arg[1];//I2S_MASTER_IDX
	i2s->config.datalen = (u8)test->arg[2];//I2S_DATALEN_16
	i2s->config.direction = (u8)test->arg[3];//I2S_TRX
	i2s->config.page_num = (u16)test->arg[4];//2<= <=4
	i2s->config.page_size = (u16)test->arg[5];//2~4096
	i2s->config.channel = (u16)test->arg[6];//I2S_CHN_STEREO
	i2s->config.sampling_rate = (u8)test->arg[7];//I2S_SMP_RATE_16KHZ

	i2s->config.internal = (u8)test->arg[8];//I2S_LOOPBACK_INTERNAL;
	i2s->config.format = (u8)test->arg[9];//I2S_FORMAT_I2S;
	//i2s->config.endian_swap = (u8)test->arg[8];
	s907x_hal_i2s_init(i2s,(u8*)i2s_tx_buf,(u8*)i2s_rx_buf);
}


static void i2s_test_exit(hal_test_t *test)
{
	i2s_hdl_t *i2s = &i2s_type;

	s907x_hal_i2s_deinit(i2s);

} 


//example
hal_test_name_map_t i2s_test_map[] = 
{
	{0, "it mode"},
};

 
//¡ä??¡¥3¨¦¦Ì?DMA
void i2s_test(hal_test_t *test)
{	
	ASSERT(test);
	ASSERT(test->no < (sizeof(i2s_test_map)/sizeof(hal_test_name_map_t)));

	HAL_TEST_DBG("test no %d = %s\n", test->no, i2s_test_map[test->no].name);
	
	i2s_test_enter(test);

	switch(test->no) 
	{
		case I2S_TEST_INTERRUPT:
			i2s_test_interrupt(test);
			break;
		default:
			
			break;
	}
	
    //i2s_test_exit(test);

	HAL_TEST_DBG("i2c test item:%d finish\n",test->no);

}

#else
void i2s_test(hal_test_t *test)
{

}
#endif