#include "s907x.h"
#include "spi_test.h"
#include "gpio_test.h"
#include "hal_timer.h"
#include "timer_test.h"
 
#if M_AT_TEST

#define SPI_BUF_LEN 64

#define TXD_SLA_OFFS   (0x40)
#define TXD_U16_OFFS   (0x100)
 

#define TXD_MST8_START  (0)
#define TXD_MST16_START (TXD_U16_OFFS + TXD_MST8_START)

#define TXD_SLA8_START	(TXD_SLA_OFFS)
#define TXD_SLA16_START (TXD_U16_OFFS + TXD_SLA8_START)

#define SPI_SEL_BUF(txrx, mode, data_len)  (txrx == SPI_TX ? spi_test_sel_txbuf(mode, data_len) : spi_test_sel_rxbuf(mode, data_len))
#define SPI_SEL_BUF_PERF(txrx, mode, data_len)  (txrx == SPI_TX ? spi_perf_test_sel_txbuf(mode, data_len) : spi_perf_test_sel_rxbuf(mode, data_len))   

#define		ONEMINUTE		60000    
spi_hdl_t spi_master_hdl;
spi_hdl_t spi_slaver_hdl; 

spi_hdl_t perf_spi_slaver_hdl;
spi_hdl_t perf_spi_master_hdl;

static sema_t spi_sla_rx_sema;
static sema_t spi_sla_tx_sema;
static sema_t spi_mst_rx_sema;
static sema_t spi_mst_tx_sema;
static sema_t spi_thread_destory_sema;


//extern
extern timer_hdl_t timx_hdl;
extern u8 	master_tim_sel;
extern u32      totalDatas;
extern u32		runtime_cnt;
extern u8		newstarttest;
extern sema_t 	clock_syn_sema;
extern sema_t	clock_syn_b_sema;
extern u8 SYN;

static u8 master_txbuf_8bit[SPI_BUF_LEN];
static u8 master_rxbuf_8bit[SPI_BUF_LEN];

static u8 slaver_txbuf_8bit[SPI_BUF_LEN];
static u8 slaver_rxbuf_8bit[SPI_BUF_LEN];

static u16 master_txbuf_16bit[SPI_BUF_LEN];
static u16 master_rxbuf_16bit[SPI_BUF_LEN];

static u16 slaver_txbuf_16bit[SPI_BUF_LEN];
static u16 slaver_rxbuf_16bit[SPI_BUF_LEN];

static u8 *mst_txbuf_8bit;
static u8 *sla_txbuf_8bit;
static u16 *mst_txbuf_16bit;
static u16 *sla_txbuf_16bit;

static u8 *mst_rxbuf_8bit = NULL;
static u8 *sla_rxbuf_8bit = NULL;
static u16 *mst_rxbuf_16bit = NULL;
static u16 *sla_rxbuf_16bit = NULL;



static u8 master_tx_it_down = 0;
static u8 master_rx_it_down = 0;
static u8 master_tx_dma_down = 0;
static u8 master_rx_dma_down = 0;
static u8 slaver_tx_it_down = 0;
static u8 slaver_rx_it_down = 0;
static u8 slaver_tx_dma_down = 0;
static u8 slaver_rx_dma_down = 0;


void spi_gpio_set_pull(u32 gpio_pin, u8 pull)
{
	u32 pin = BIT(gpio_pin);
	u8 pull_mode = pull;

	s907x_hal_gpio_set_pull(pin, pull_mode);
}


void s907x_hal_spi_msp_init(spi_hdl_t *spi)
{
    ASSERT(spi);
    
    if(spi->config.idx == SPI_IDX_0) {
        SPI0_CS_SEL1(HAL_ON);	
        SPI0_CLK_SEL1(HAL_ON); 	
        SPI0_MOSI_SEL1(HAL_ON);
        SPI0_MISO_SEL1(HAL_ON);
		//gpio_set_pull(22,1);
    } else {
        SPI1_CLK_SEL1(HAL_ON); 
        SPI1_MOSI_SEL1(HAL_ON);	
        SPI1_MISO_SEL1(HAL_ON);	
        SPI1_CS_SEL1(HAL_ON);	
		//gpio_set_pull(22,1);
    }
}

void s907x_hal_spi_msp_deinit(spi_hdl_t *spi)
{
    if(spi->config.idx == SPI_IDX_0) {
        SPI0_CS_SEL1(HAL_OFF);	
        SPI0_CLK_SEL1(HAL_OFF); 	
        SPI0_MOSI_SEL1(HAL_OFF);
        SPI0_MISO_SEL1(HAL_OFF);
    } else {
        SPI1_CLK_SEL1(HAL_OFF); 
        SPI1_MOSI_SEL1(HAL_OFF);	
        SPI1_MISO_SEL1(HAL_OFF);	
        SPI1_CS_SEL1(HAL_OFF);
    }
}


static int is_spi_test_16bit(int input_data_len)
{
	if(input_data_len <= 8) {
		return FALSE;
	}

	return TRUE;
}


static void *spi_test_sel_txbuf(u32 mode, int input_data_len)
{

	//select 8bits
	if(is_spi_test_16bit(input_data_len)) {
		return mode == HAL_MASTER_SEL ? master_txbuf_16bit : slaver_txbuf_16bit;
	} 
	//select 16bits
	return mode == HAL_MASTER_SEL ? master_txbuf_8bit : slaver_txbuf_8bit;
}

static void *spi_perf_test_sel_txbuf(u32 mode, int input_data_len)
{

	//select 8bits
	if(is_spi_test_16bit(input_data_len)) {
		return mode == HAL_MASTER_SEL ? mst_txbuf_16bit : sla_txbuf_16bit;
	} 
	//select 16bits
	return mode == HAL_MASTER_SEL ? mst_txbuf_8bit : sla_txbuf_8bit;
}

static void *spi_test_sel_rxbuf(u32 mode, int input_data_len)
{

	//select 8bits
	if(is_spi_test_16bit(input_data_len)) {
		return mode == HAL_MASTER_SEL ? master_rxbuf_16bit : slaver_rxbuf_16bit;
	} 
	//select 16bits
	return mode == HAL_MASTER_SEL ? master_rxbuf_8bit : slaver_rxbuf_8bit;
}

static void *spi_perf_test_sel_rxbuf(u32 mode, int input_data_len)
{

	//select 8bits
	if(is_spi_test_16bit(input_data_len)) {
		return mode == HAL_MASTER_SEL ? mst_rxbuf_16bit : sla_rxbuf_16bit;
	} 
	//select 16bits
	return mode == HAL_MASTER_SEL ? mst_rxbuf_8bit : sla_rxbuf_8bit;
}

static void perf_master_tx_int(void *contex) 
{
    spi_hdl_t *spi_m = (spi_hdl_t*)(contex);

    ASSERT(spi_m);

	wl_send_sema_fromisr(&spi_mst_tx_sema);
	//HAL_TEST_DBG("spi mst tx done, sema post\n");
}

static void perf_master_rx_int(void *contex) 
{
    spi_hdl_t *spi_m = (spi_hdl_t*)(contex);

    ASSERT(spi_m);
	
   	wl_send_sema_fromisr(&spi_mst_rx_sema);
	//HAL_TEST_DBG("spi mst rx done, sema post\n");
}

static void perf_slaver_tx_int(void *contex) 
{
    spi_hdl_t *spi_s = (spi_hdl_t*)(contex);

    ASSERT(spi_s);

    wl_send_sema_fromisr(&spi_sla_tx_sema);
    //HAL_TEST_DBG("spi sla tx done, sema post\n");
}

static void perf_slaver_rx_int(void *contex) 
{
    spi_hdl_t *spi_s = (spi_hdl_t*)(contex);

    ASSERT(spi_s);
	
	
	wl_send_sema_fromisr(&spi_sla_rx_sema);
    //HAL_TEST_DBG("spi sla rx done, sema post\n");
}


static void master_tx_int(void *contex) 
{
    spi_hdl_t *spi_m = (spi_hdl_t*)(contex);

    ASSERT(spi_m);

    master_tx_it_down = 1;
}


static void master_rx_int(void *contex) 
{
    spi_hdl_t *spi_m = (spi_hdl_t*)(contex);

    ASSERT(spi_m);
	
    master_rx_it_down = 1;
}

static void slaver_tx_int(void *contex) 
{
    spi_hdl_t *spi_s = (spi_hdl_t*)(contex);

    ASSERT(spi_s);

    slaver_tx_it_down = 1;
}

static void slaver_rx_int(void *contex) 
{
    spi_hdl_t *spi_s = (spi_hdl_t*)(contex);

    ASSERT(spi_s);

    slaver_rx_it_down = 1;
}


static void master_tx_dma(void *contex) 
{
    spi_hdl_t *spi_m = (spi_hdl_t*)(contex);

    ASSERT(spi_m);

    master_tx_dma_down = 1;
}


static void master_rx_dma(void *contex) 
{
    spi_hdl_t *spi_m = (spi_hdl_t*)(contex);

    ASSERT(spi_m);
	
    master_rx_dma_down = 1;
}

static void slaver_tx_dma(void *contex) 
{
    spi_hdl_t *spi_s = (spi_hdl_t*)(contex);

    ASSERT(spi_s);

    slaver_tx_dma_down = 1;
}

static void slaver_rx_dma(void *contex) 
{
    spi_hdl_t *spi_s = (spi_hdl_t*)(contex);

    ASSERT(spi_s);

    slaver_rx_dma_down = 1;
}


void spi_test_basic_trx(hal_test_t *test)
{
	spi_hdl_t *spi_m = &spi_master_hdl;
	spi_hdl_t *spi_s = &spi_slaver_hdl;
    SPI_TEST_MODE mode;

	u16 mst_slen = 0 ,slv_rlen = 0;


	ASSERT(test);

    mode     = (SPI_TEST_MODE)(test->arg[0]);    

    //master uust specific dir
    if(mode & SPI_TEST_MASTER)
        spi_m->config.dir = SPI_MODE_TXRX;

    //slaver prepare tx data
    if(mode & SPI_TEST_SLAVER)
        s907x_hal_spi_slaver_tx(spi_s, SPI_SEL_BUF(SPI_TX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN, SPI_POLL_LIMIT);//×èè?µè'y  SPI_POLL_LIMIT
    
    //use master tx/rx 
    if(mode & SPI_TEST_MASTER)
        mst_slen = s907x_hal_spi_master_txrx(spi_m, SPI_SEL_BUF(SPI_TX, HAL_MASTER_SEL, test->arg[1]), SPI_SEL_BUF(SPI_RX, HAL_MASTER_SEL, test->arg[1]), SPI_BUF_LEN, SPI_POLL_LIMIT);
	
    if(mode & SPI_TEST_SLAVER) {
        //slaver recv master data
        slv_rlen = s907x_hal_spi_slaver_rx(spi_s,  SPI_SEL_BUF(SPI_RX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN, SPI_POLL_LIMIT);//2?×èè?	
    }

    if(mode & SPI_TEST_MASTER) {
        if(mst_slen != SPI_BUF_LEN) {
            goto exit;
        }
    }

    if(mode & SPI_TEST_SLAVER) {
        if(slv_rlen != SPI_BUF_LEN) {
            goto exit;
        }
    }
	if((mode == SPI_TEST_LOOPBACK) && (mst_slen != SPI_BUF_LEN || slv_rlen != SPI_BUF_LEN)) {
        goto exit;
	} 

	//check master rx data == slaver tx data 
    if(mode & SPI_TEST_MASTER) {
        if(memcmp(SPI_SEL_BUF(SPI_RX, HAL_MASTER_SEL, test->arg[1]), SPI_SEL_BUF(SPI_TX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN)) {
            HAL_TEST_DBG("spi poll test fail\n");
            HAL_TEST_DBG("master rx:\n");
            HAL_DBG_ARRARY(SPI_SEL_BUF(SPI_RX, HAL_MASTER_SEL, test->arg[1]), SPI_BUF_LEN, ARY_U8, 0);
            HAL_TEST_DBG("slaver tx:\n");
            HAL_DBG_ARRARY(SPI_SEL_BUF(SPI_TX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN, ARY_U8, 0);
            test->result = FALSE;
            return;
        }
    }
	//check master tx data == slaver rx data
    if(mode & SPI_TEST_SLAVER) {
        if(memcmp(SPI_SEL_BUF(SPI_TX, HAL_MASTER_SEL, test->arg[1]), SPI_SEL_BUF(SPI_RX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN)) {
            HAL_TEST_DBG("spi poll test fail\n");
            HAL_TEST_DBG("master tx:\n");
            HAL_DBG_ARRARY(SPI_SEL_BUF(SPI_TX, HAL_MASTER_SEL, test->arg[1]), SPI_BUF_LEN, ARY_U8, 0);
            HAL_TEST_DBG("slaver rx:\n");
            HAL_DBG_ARRARY(SPI_SEL_BUF(SPI_RX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN, ARY_U8, 0);		
            test->result = FALSE;
            return;
        }	
    }
	
	test->result = TRUE;
    HAL_TEST_DBG("finish spi poll test \n");
    return;
exit:
    HAL_TEST_DBG("spi poll test fail\n");
    test->result = FALSE;
}


void spi_test_interrupt_trx(hal_test_t *test)
{
	spi_hdl_t *spi_m = &spi_master_hdl;
	spi_hdl_t *spi_s = &spi_slaver_hdl;
    SPI_TEST_MODE mode;

	ASSERT(test);
	
    mode  = (SPI_TEST_MODE)(test->arg[0]);

	if(test->arg[4] == 0) { 
        //slaver rx interrupt
        HAL_TEST_DBG("spi test slaver rx interrupt\n");
        master_tx_it_down = slaver_rx_it_down = 0;
        //slaver rx it
        if(mode & SPI_TEST_SLAVER)
            s907x_hal_spi_slaver_recv_interrupt(spi_s, SPI_SEL_BUF(SPI_RX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN);
        //master tx poll/interrupt
        if(mode & SPI_TEST_MASTER)  {
            if(test->arg[5] == 0) {
                    HAL_TEST_DBG("spi test slaver rx interrupt, master tx poll\n");
                    spi_m->config.dir = SPI_MODE_TX;
                    s907x_hal_spi_master_txrx(spi_m, SPI_SEL_BUF(SPI_TX, HAL_MASTER_SEL, test->arg[1]), NULL, SPI_BUF_LEN, SPI_POLL_LIMIT);
                    master_tx_it_down = 1;    
            } else if(test->arg[5] == 1) {
                    HAL_TEST_DBG("spi test slaver rx interrupt, master tx interrupt\n");
                    s907x_hal_spi_master_xfer_interrupt(spi_m, SPI_SEL_BUF(SPI_TX, HAL_MASTER_SEL, test->arg[1]), SPI_BUF_LEN);
            }
        }  
        //for matser wait tx down
        if(mode & SPI_TEST_MASTER) {
            while(!master_tx_it_down){
                //TODO add timeout 
            }
        }
        //for slaver wait rx down
        if(mode & SPI_TEST_SLAVER) {
            while(!slaver_rx_it_down){
                //TODO add timeout 
            }
        }

        //check slaver rx
        if(mode & SPI_TEST_SLAVER) {
            if(memcmp(SPI_SEL_BUF(SPI_TX, HAL_MASTER_SEL, test->arg[1]), SPI_SEL_BUF(SPI_RX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN)) {
                HAL_TEST_DBG("spi it test fail\n");
                HAL_TEST_DBG("master tx:\n");
                HAL_DBG_ARRARY(SPI_SEL_BUF(SPI_TX, HAL_MASTER_SEL, test->arg[1]), SPI_BUF_LEN, ARY_U8, 0);
                HAL_TEST_DBG("slaver rx:\n");
                HAL_DBG_ARRARY(SPI_SEL_BUF(SPI_RX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN, ARY_U8, 0);		
                test->result = FALSE;
                return;
            }	 
        }
        test->result = TRUE;    
    } else if(test->arg[4] == 1) {  
        //master rx interrupt
        HAL_TEST_DBG("spi test master rx interrupt\n");
        master_rx_it_down = slaver_tx_it_down = 0;    
        //master rx it
        if(mode & SPI_TEST_MASTER)
            s907x_hal_spi_master_recv_interrupt(spi_m, SPI_SEL_BUF(SPI_RX, HAL_MASTER_SEL, test->arg[1]), SPI_BUF_LEN);
        
        //slaver tx poll/interrupt
        if(mode & SPI_TEST_SLAVER){
            
            slaver_rx_it_down = 0;
            s907x_hal_spi_slaver_recv_interrupt(spi_s, SPI_SEL_BUF(SPI_RX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN);
            
            
            if(test->arg[6] == 0) {
                HAL_TEST_DBG("spi test master rx interrupt, salver tx poll\n");
                s907x_hal_spi_slaver_tx(spi_s, SPI_SEL_BUF(SPI_TX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN, SPI_POLL_LIMIT);
                slaver_tx_it_down = 1;
            } else if(test->arg[6] == 1) {
                HAL_TEST_DBG("spi test master rx interrupt, salver tx interrupt\n");
                s907x_hal_spi_slaver_xfer_interrupt(spi_s, SPI_SEL_BUF(SPI_TX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN);
            }   
        }

        //master tx poll/interrupt
        if(mode & SPI_TEST_MASTER) {
            if(test->arg[5] == 0) {
                spi_m->config.dir = SPI_MODE_TX;
                HAL_TEST_DBG("spi test master rx interrupt, master tx poll not support\n");
                HAL_TEST_DBG("#run master xfer interrrupt...\n");
                goto interrubt;
            } else if(test->arg[5] == 1) {
            interrubt:
                HAL_TEST_DBG("spi test master rx interrupt, master tx interrupt\n");
                s907x_hal_spi_master_xfer_interrupt(spi_m, SPI_SEL_BUF(SPI_TX, HAL_MASTER_SEL, test->arg[1]), SPI_BUF_LEN);
            }
        }
        //for matser wait rx down
        if(mode & SPI_TEST_MASTER) {
            while(!master_rx_it_down){
                 //TODO add timeout 
            }
            if(memcmp(SPI_SEL_BUF(SPI_RX, HAL_MASTER_SEL, test->arg[1]), SPI_SEL_BUF(SPI_TX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN)) {
                HAL_TEST_DBG("spi it test fail\n");
                HAL_TEST_DBG("master rx:\n");
                HAL_DBG_ARRARY(SPI_SEL_BUF(SPI_RX, HAL_MASTER_SEL, test->arg[1]), SPI_BUF_LEN, ARY_U8, 0);
                HAL_TEST_DBG("slaver tx:\n");
                HAL_DBG_ARRARY(SPI_SEL_BUF(SPI_TX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN, ARY_U8, 0);
                test->result = FALSE;
                return;
            }
        }
        //for slaver wait tx down
        if(mode & SPI_TEST_SLAVER) {
            while(!slaver_tx_it_down){
                 //TODO add timeout 
            }
            while(!slaver_rx_it_down)
            {
              //TODO add timeout
            }
        }
        test->result = TRUE;
    } else if(test->arg[4] == 2)    {
        HAL_TEST_DBG("spi test master trx slaver trx interrupt\n");
        master_tx_it_down = slaver_rx_it_down = 0;
        master_rx_it_down = slaver_tx_it_down = 0; 

        //slaver
        if(mode & SPI_TEST_SLAVER) {
            s907x_hal_spi_slaver_xfer_interrupt(spi_s, SPI_SEL_BUF(SPI_TX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN);
            s907x_hal_spi_slaver_recv_interrupt(spi_s, SPI_SEL_BUF(SPI_RX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN);
        }
        //master
        if(mode & SPI_TEST_MASTER) {
            s907x_hal_spi_master_recv_interrupt(spi_m, SPI_SEL_BUF(SPI_RX, HAL_MASTER_SEL, test->arg[1]), SPI_BUF_LEN);
            s907x_hal_spi_master_xfer_interrupt(spi_m, SPI_SEL_BUF(SPI_TX, HAL_MASTER_SEL, test->arg[1]), SPI_BUF_LEN);
        }
        //slaver
        if(mode & SPI_TEST_SLAVER) {
            while(!slaver_rx_it_down || !slaver_tx_it_down){
                //TODO add timeout 
            }
        }
        //master
        if(mode & SPI_TEST_MASTER) {
            while(!master_tx_it_down || !master_rx_it_down){
                //TODO add timeout 
            }
        }

        //check master rx data == slaver tx data
        if(mode & SPI_TEST_MASTER) {
            if(memcmp(SPI_SEL_BUF(SPI_RX, HAL_MASTER_SEL, test->arg[1]), SPI_SEL_BUF(SPI_TX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN)) {
                HAL_TEST_DBG("spi it test fail\n");
                HAL_TEST_DBG("master rx:\n");
                HAL_DBG_ARRARY(SPI_SEL_BUF(SPI_RX, HAL_MASTER_SEL, test->arg[1]), SPI_BUF_LEN, ARY_U8, 0);
                HAL_TEST_DBG("slaver tx:\n");
                HAL_DBG_ARRARY(SPI_SEL_BUF(SPI_TX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN, ARY_U8, 0);
                test->result = FALSE;
                return;
            }
        }
        //check master tx data == slaver rx data    
        if(mode & SPI_TEST_SLAVER) {
            if(memcmp(SPI_SEL_BUF(SPI_TX, HAL_MASTER_SEL, test->arg[1]), SPI_SEL_BUF(SPI_RX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN)) {
                HAL_TEST_DBG("spi it test fail\n");
                HAL_TEST_DBG("master tx:\n");
                HAL_DBG_ARRARY(SPI_SEL_BUF(SPI_TX, HAL_MASTER_SEL, test->arg[1]), SPI_BUF_LEN, ARY_U8, 0);
                HAL_TEST_DBG("slaver rx:\n");
                HAL_DBG_ARRARY(SPI_SEL_BUF(SPI_RX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN, ARY_U8, 0);		
                test->result = FALSE;
                return;
            }
        }
        test->result = TRUE;
    }
    
    HAL_TEST_DBG("finish spi it test \n");	
}


void spi_test_int_trx_dma(hal_test_t *test)
{
	spi_hdl_t *spi_m = &spi_master_hdl;
	spi_hdl_t *spi_s = &spi_slaver_hdl;
    SPI_TEST_MODE mode;

	ASSERT(test);	

    mode  = (SPI_TEST_MODE)(test->arg[0]);
    if(test->arg[4] == 0) {
        HAL_TEST_DBG("spi test slaver rx dma\n");
        master_tx_dma_down = slaver_rx_dma_down = 0;
        //slaver recv dma
        if(mode & SPI_TEST_SLAVER)
            s907x_hal_spi_slaver_recv_dma(spi_s, SPI_SEL_BUF(SPI_RX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN);
        
        if(mode & SPI_TEST_MASTER) {
            if(test->arg[5] == 0) {
                spi_m->config.dir = SPI_MODE_TX;
                HAL_TEST_DBG("spi test slaver rx dma, master tx poll\n");
                s907x_hal_spi_master_txrx(spi_m, SPI_SEL_BUF(SPI_TX, HAL_MASTER_SEL, test->arg[1]), NULL, SPI_BUF_LEN, SPI_POLL_LIMIT);
                master_tx_dma_down = 1;
            } else if(test->arg[5] == 1) {
                HAL_TEST_DBG("spi test slaver rx dma, master tx interrupt\n");
                master_tx_it_down = 0;
                s907x_hal_spi_master_xfer_interrupt(spi_m, SPI_SEL_BUF(SPI_TX, HAL_MASTER_SEL, test->arg[1]), SPI_BUF_LEN);
            } else if(test->arg[5] == 2) {
                HAL_TEST_DBG("spi test slaver rx dma, master tx dma\n");
                s907x_hal_spi_master_xfer_dma(spi_m, SPI_SEL_BUF(SPI_TX, HAL_MASTER_SEL, test->arg[1]), SPI_BUF_LEN);
            }
        }

        if(mode & SPI_TEST_MASTER) {
            while(!master_tx_dma_down && !master_tx_it_down){
                //TODO add timeout 
            }
        }

        if(mode & SPI_TEST_SLAVER) {
            while(!slaver_rx_dma_down){
                //TODO add timeout 
            }
            if(memcmp(SPI_SEL_BUF(SPI_TX, HAL_MASTER_SEL, test->arg[1]), SPI_SEL_BUF(SPI_RX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN)) {
                HAL_TEST_DBG("spi dma test fail\n");
                HAL_TEST_DBG("master tx:\n");
                HAL_DBG_ARRARY(SPI_SEL_BUF(SPI_TX, HAL_MASTER_SEL, test->arg[1]), SPI_BUF_LEN, ARY_U8, 0);
                HAL_TEST_DBG("slaver rx:\n");
                HAL_DBG_ARRARY(SPI_SEL_BUF(SPI_RX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN, ARY_U8, 0);		
                test->result = FALSE;
                return;
            }	 
        }
        test->result = TRUE;    
    } else if(test->arg[4] == 1) {
        HAL_TEST_DBG("spi test master rx dma \n");
        master_rx_dma_down = slaver_tx_dma_down = 0;    

        //master rx dma
        if(mode & SPI_TEST_MASTER)
            s907x_hal_spi_master_recv_dma(spi_m, SPI_SEL_BUF(SPI_RX, HAL_MASTER_SEL, test->arg[1]), SPI_BUF_LEN);

        //slaver tx poll/it/dma
        if(mode & SPI_TEST_SLAVER) {
          
          slaver_rx_dma_down = 0;
          s907x_hal_spi_slaver_recv_dma(spi_s, SPI_SEL_BUF(SPI_TX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN);
          
            if(test->arg[6] == 0) {
                HAL_TEST_DBG("spi test master rx dma, slaver tx poll\n");
                s907x_hal_spi_slaver_tx(spi_s, SPI_SEL_BUF(SPI_TX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN, SPI_POLL_LIMIT);

            } else if(test->arg[6] == 1) {
                HAL_TEST_DBG("spi test master rx dma, slaver tx interrupt\n");
                slaver_tx_it_down = 0;
                s907x_hal_spi_slaver_xfer_interrupt(spi_s, SPI_SEL_BUF(SPI_TX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN);
            }  else if(test->arg[6] == 2) {
                HAL_TEST_DBG("spi test master rx dma slaver tx dma\n");
                slaver_tx_dma_down = 0;
                s907x_hal_spi_slaver_xfer_dma(spi_s, SPI_SEL_BUF(SPI_TX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN);
            } 
        }

        //master tx dummy poll/it/dma
       if(mode & SPI_TEST_MASTER) {
            if(test->arg[5] == 0) {
                spi_m->config.dir = SPI_MODE_TX;
                HAL_TEST_DBG("spi test master rx dma, master tx poll not support\n");
                HAL_TEST_DBG("#run master tx interrupt...\n");
                goto interrupt;
                
            } else if(test->arg[5] == 1) {
            interrupt:
                HAL_TEST_DBG("spi test master rx dma, master tx interrupt\n");
                master_tx_it_down = 0;
                s907x_hal_spi_master_xfer_interrupt(spi_m, SPI_SEL_BUF(SPI_TX, HAL_MASTER_SEL, test->arg[1]), SPI_BUF_LEN);
            } else if(test->arg[5] == 2) {
                HAL_TEST_DBG("spi test master rx dma, master tx dma\n");
                master_tx_dma_down = 0;
                s907x_hal_spi_master_xfer_dma(spi_m, SPI_SEL_BUF(SPI_TX, HAL_MASTER_SEL, test->arg[1]), SPI_BUF_LEN);
            }
        } 

        if(mode & SPI_TEST_MASTER) {
            
            if(test->arg[5] == 2)
            {
              while(!master_tx_dma_down)
              {   
                //TODO add timeout 
              }
            }
            else
            {
            
              while(!master_tx_it_down)
              {   
                //TODO add timeout 
              }
            
            }
            
            while(!master_rx_dma_down){
                 //TODO add timeout 
            }
            
            if(memcmp(SPI_SEL_BUF(SPI_RX, HAL_MASTER_SEL, test->arg[1]), SPI_SEL_BUF(SPI_TX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN)) {
                HAL_TEST_DBG("spi dma test fail\n");
                HAL_TEST_DBG("master rx:\n");
                HAL_DBG_ARRARY(SPI_SEL_BUF(SPI_RX, HAL_MASTER_SEL, test->arg[1]), SPI_BUF_LEN, ARY_U8, 0);
                HAL_TEST_DBG("slaver tx:\n");
                HAL_DBG_ARRARY(SPI_SEL_BUF(SPI_TX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN, ARY_U8, 0);
                test->result = FALSE;
                return;
            }
        }
        if(mode & SPI_TEST_SLAVER) {
            
            if(test->arg[6] == 1)
            {
              while(!slaver_tx_it_down)
              {
                //TODO add timeout 
              }
            }
            else if(test->arg[6] == 2)
            {
              while(!slaver_tx_dma_down)
              {
                //TODO add timeout
              }
            }
            
            while(!slaver_rx_dma_down)
            {
              //TODO add timeout
            }
        }
        test->result = TRUE;
    } else if(test->arg[4] == 2)    {
        HAL_TEST_DBG("spi test dma master trx slaver trx\n");
        master_tx_dma_down = slaver_rx_dma_down = 0;
        master_rx_dma_down = slaver_tx_dma_down = 0;
 
        //slaver
        if(mode & SPI_TEST_SLAVER) {
            s907x_hal_spi_slaver_xfer_dma(spi_s, SPI_SEL_BUF(SPI_TX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN);
            s907x_hal_spi_slaver_recv_dma(spi_s, SPI_SEL_BUF(SPI_RX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN);
			while(!slaver_rx_dma_down ||
				  !slaver_tx_dma_down ){
				 //TODO add timeout 
			}
        }
        //master
        if(mode & SPI_TEST_MASTER) {
            s907x_hal_spi_master_recv_dma(spi_m, SPI_SEL_BUF(SPI_RX, HAL_MASTER_SEL, test->arg[1]), SPI_BUF_LEN);
            s907x_hal_spi_master_xfer_dma(spi_m, SPI_SEL_BUF(SPI_TX, HAL_MASTER_SEL, test->arg[1]), SPI_BUF_LEN);

            while(!master_tx_dma_down || 
                  !master_rx_dma_down){
                 //TODO add timeout 
            }

        }


        //check master rx data == slaver tx data
        if(mode & SPI_TEST_MASTER) {
            if(memcmp(SPI_SEL_BUF(SPI_RX, HAL_MASTER_SEL, test->arg[1]), SPI_SEL_BUF(SPI_TX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN)) {
                HAL_TEST_DBG("spi dma test fail\n");
                HAL_TEST_DBG("master rx:\n");
                HAL_DBG_ARRARY(SPI_SEL_BUF(SPI_RX, HAL_MASTER_SEL, test->arg[1]), SPI_BUF_LEN, ARY_U8, 0);
                HAL_TEST_DBG("slaver tx:\n");
                HAL_DBG_ARRARY(SPI_SEL_BUF(SPI_TX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN, ARY_U8, 0);
                test->result = FALSE;
                return;
            }
        }

        //check master tx data == slaver rx data
        if(mode & SPI_TEST_SLAVER) {
            if(memcmp(SPI_SEL_BUF(SPI_TX, HAL_MASTER_SEL, test->arg[1]), SPI_SEL_BUF(SPI_RX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN)) {
                HAL_TEST_DBG("spi dma test fail\n");
                HAL_TEST_DBG("master tx:\n");
                HAL_DBG_ARRARY(SPI_SEL_BUF(SPI_TX, HAL_MASTER_SEL, test->arg[1]), SPI_BUF_LEN, ARY_U8, 0);
                HAL_TEST_DBG("slaver rx:\n");
                HAL_DBG_ARRARY(SPI_SEL_BUF(SPI_RX, HAL_SLAVE_SEL, test->arg[1]), SPI_BUF_LEN, ARY_U8, 0);		
                test->result = FALSE;
                return;
            }
        }
        test->result = TRUE;
    }   
    HAL_TEST_DBG("finish spi dma test \n");	
}




//example
hal_test_name_map_t spi_test_map[] = 
{
	{0, "spi  trx poll test"},
	{1, "spi  trx interrupt test"},
	{2, "spi  trx dma  test"},
	{3, "spi  performance test"},
};



static u32 datelen_mode_array[]={0x0f,0x1f,0x3f,0x7f,0xff,0x1ff,0x3ff,0x7ff,0xfff,0x1fff,0x3fff,0x7fff,0xffff};
static void spi_testdata_init(hal_test_t *test)
{
	int i;
        
        u32 datelen = test->arg[1];
	
	//memset(master_txbuf_8bit, 0, SPI_BUF_LEN);
	memset(master_rxbuf_8bit, 0, SPI_BUF_LEN);
	memset(master_rxbuf_16bit, 0, SPI_BUF_LEN);

	memset(slaver_rxbuf_8bit, 0, SPI_BUF_LEN);
	memset(slaver_rxbuf_16bit, 0, SPI_BUF_LEN);	

	//master tx u8  0~3F
	//       tx u16 0x100~0x13F

	//slaver tx u8  0x40~0x7F
	//       tx u16 0x140~0x13F
        
        for(i = 4; i < 17;i++)
        {
          if(datelen == i)
          {
            datelen = datelen_mode_array[i-4];
            break;
          }
        }
	
	for(i= 0; i < SPI_BUF_LEN; i++)
	{
		master_txbuf_8bit[i] = (i + TXD_MST8_START) & datelen;
	}
	for(i= 0; i < SPI_BUF_LEN; i++)
	{
		master_txbuf_16bit[i] = (i + TXD_MST16_START) & datelen;  
	}
	for(i= 0; i < SPI_BUF_LEN; i++)
	{
		slaver_txbuf_8bit[i] = (i + TXD_SLA8_START) & datelen;
	}
	for(i= 0; i < SPI_BUF_LEN; i++)
	{
		slaver_txbuf_16bit[i] = (i + TXD_SLA16_START) & datelen;
	}	
	
}

static int spi_perf_testdata_init(hal_test_t *test)
{
	int i;
	u32 testdataslen = test->arg[0];
	u32 datelen = test->arg[1];

	mst_rxbuf_8bit = (u8 *)wl_zmalloc(testdataslen * sizeof(u8));
	sla_rxbuf_8bit = (u8 *)wl_zmalloc(testdataslen * sizeof(u8));
	mst_rxbuf_16bit = (u16 *)wl_zmalloc(testdataslen * sizeof(u16));
	sla_rxbuf_16bit = (u16 *)wl_zmalloc(testdataslen * sizeof(u16));
	if(!mst_rxbuf_8bit || !sla_rxbuf_8bit || !mst_rxbuf_16bit || !sla_rxbuf_16bit){
		wl_free(mst_rxbuf_8bit);
		wl_free(sla_rxbuf_8bit);
		wl_free(mst_rxbuf_16bit);
		wl_free(sla_rxbuf_16bit);
		return FALSE;
	}
	
	for(i = 4; i < 17;i++){
	if(datelen == i){
			datelen = datelen_mode_array[i-4];
			break;
		}
	}
	
	if(datelen <= 0xff){
		mst_txbuf_8bit = wl_malloc(testdataslen * sizeof(u8));
		sla_txbuf_8bit = wl_malloc(testdataslen * sizeof(u8));
		if(!mst_txbuf_8bit || !sla_txbuf_8bit){
			wl_free(mst_txbuf_8bit);
			wl_free(sla_txbuf_8bit);
			return FALSE;
		}
	}else{
		mst_txbuf_16bit = wl_malloc(testdataslen * sizeof(u16));
		sla_txbuf_16bit = wl_malloc(testdataslen * sizeof(u16));
		if(!mst_txbuf_16bit || !sla_txbuf_16bit){
			wl_free(mst_txbuf_16bit);
			wl_free(sla_txbuf_16bit);
			return FALSE;
		}
	}
	
	for(i = 0; i < testdataslen; i++){
		mst_txbuf_8bit[i] = (i + TXD_MST8_START) & datelen;
		sla_txbuf_8bit[i] = (i + TXD_SLA8_START) & datelen;
		
		mst_txbuf_16bit[i] = (i + TXD_MST16_START) & datelen; 
		sla_txbuf_16bit[i] = (i + TXD_SLA16_START) & datelen;
	}
	
	
	return TRUE;
	
}


static void spi_test_enter(hal_test_t *test)
{
    u32 data_len, buad, clk_pha, clk_pol;
	spi_hdl_t *spi_m = &spi_master_hdl;
	spi_hdl_t *spi_s = &spi_slaver_hdl;
    SPI_TEST_MODE mode;

    mode     = (SPI_TEST_MODE)test->arg[0];             
    data_len = test->arg[1];
	buad     = test->arg[2];
	clk_pha  = HI_UINT8(test->arg[3]);
 	clk_pol  = LO_UINT8(test->arg[3]);
 

	
	spi_testdata_init(test);
    // config spi_m 
    if(mode & SPI_TEST_MASTER) {

        ASSERT(data_len >= 4 && data_len <= 16);

        spi_m->config.idx = SPI_MASTER_IDX;
        spi_m->config.spi_master = HAL_MASTER_SEL;
        spi_m->config.datalen = BIT(data_len - 4);
        spi_m->config.clk_phase = clk_pha;
        spi_m->config.clk_polarity = clk_pol;
        spi_m->config.clk_speed = buad;
        spi_m->config.mode = SPI_MODE_TXRX;

        spi_m->it.tx_complete.func= master_tx_int;
        spi_m->it.tx_complete.context = spi_m;

        spi_m->it.rx_complete.func = master_rx_int;
        spi_m->it.rx_complete.context = spi_m;

        spi_m->dma.rx_complete.func = master_rx_dma;
        spi_m->dma.rx_complete.context = spi_m;
        
        spi_m->dma.tx_complete.func = master_tx_dma;
        spi_m->dma.tx_complete.context = spi_m;

        s907x_hal_spi_init(spi_m);
    }

	// config spi_s
    if(mode & SPI_TEST_SLAVER) {

        spi_s->config.idx = SPI_SLAVE_IDX;
        spi_s->config.spi_master = HAL_SLAVE_SEL;
        spi_s->config.datalen = BIT(data_len - 4);
        spi_s->config.clk_phase = clk_pha;
        spi_s->config.clk_polarity = clk_pol; 
        spi_s->config.mode = SPI_MODE_TXRX;

        spi_s->it.tx_complete.func= slaver_tx_int;
        spi_s->it.tx_complete.context = spi_s;
        
        spi_s->it.rx_complete.func = slaver_rx_int;
        spi_s->it.rx_complete.context = spi_s;
        
        spi_s->dma.rx_complete.func = slaver_rx_dma;
        spi_s->dma.rx_complete.context = spi_s;
                                        
        spi_s->dma.tx_complete.func = slaver_tx_dma;
        spi_s->dma.tx_complete.context = spi_s;

        // init spi_slaver
        s907x_hal_spi_init(spi_s);

    }

	switch(test->no) 
	{
		case SPI_TEST_BASIC_TRX:
			spi_test_basic_trx(test);
			break;
		case SPI_TEST_INTERRUPT_TRX:
			spi_test_interrupt_trx(test);
			break;
		case SPI_TEST_DMA_TRX:
			spi_test_int_trx_dma(test);
			break;
		default:

		break;
	}

}
 
static void spi_test_exit(hal_test_t *test)
{
	spi_hdl_t *spi_m = &spi_master_hdl;
	spi_hdl_t *spi_s = &spi_slaver_hdl;

    ASSERT(test);

	s907x_hal_spi_deinit(spi_m);
	s907x_hal_spi_deinit(spi_s);

    HAL_TEST_DBG("spi test item:%d finish\n",test->no);
}

/*
*	no: 	test item     	0-4
*	dfs:	dataframesize 	4~16
*	buad:   buadrate		(115200)
*	pha:    clk_phase       0-1
*	pol:    clk_polarity    0-1
*	trx:	tx/rx/trx       (0:tx 1:rx 2:trx)
*/

/*		test   	 no	 dlen    buad   pha pol trx  stx  mtx
*       @SPI     0    4-16  2500000	 0	 0	 2	 0-2  0-2
*       @SPI     1    4-16 	2500000  0 	 0	 2	 0-2  0-2  
*       @SPI     2    4-16  2500000  0 	 0	 2	 0-2  0-2 
*       @SPI     3    4-16 	2500000  0	 0 	 2   0-2  0-2 
*/



static void spi_sla_task(hal_test_t *test)
{
	
			hal_status_e s_sta;
			u32 ret;
			int s_slen = 0;
			int s_rlen = 0;
			
			u16 testdataslen;
			spi_hdl_t *spi_s = &perf_spi_slaver_hdl;

			u32 data_len, buad, clk_pha, clk_pol;

			//SPI_TEST_MODE mode;

			testdataslen = (u16)test->arg[0];             
			data_len = test->arg[1];
			buad     = test->arg[2];
			clk_pha  = HI_UINT8(test->arg[3]);
			clk_pol  = LO_UINT8(test->arg[3]); 
	
			//spi_perf_testdata_init(test);
			spi_perf_testdata_init(test);
			
			
			spi_s->config.idx = SPI_SLAVE_IDX;
			spi_s->config.spi_master = HAL_SLAVE_SEL;
			spi_s->config.datalen = BIT(data_len - 4);
			spi_s->config.clk_phase = clk_pha;
			spi_s->config.clk_polarity = clk_pol; 
			spi_s->config.mode = SPI_MODE_TXRX;

			spi_s->it.tx_complete.func= perf_slaver_tx_int;
			spi_s->it.tx_complete.context = spi_s;

			spi_s->it.rx_complete.func = perf_slaver_rx_int;
			spi_s->it.rx_complete.context = spi_s;

			// init spi_slaver
			s907x_hal_spi_init(spi_s);
			at_clock_syn_init(out_sel, GPIO_SYN);//pull up   output
			at_clock_syn_init(in_sel, GPIO_SYN_B);//pull up  input
			SYN = TRUE;//clock_syn_b_sema
			
			wl_init_sema(&spi_sla_rx_sema, 0, sema_binary);
			wl_init_sema(&spi_sla_tx_sema, 0, sema_binary);
			wl_init_sema(&clock_syn_b_sema, 0, sema_binary);
			
			at_gpio_write(GPIO_SYN, GPIO_PIN_SET);
			at_gpio_write(GPIO_SYN_B, GPIO_PIN_SET);	
			while(1)
			{
#if 0 //sla poll		
					memset(SPI_SEL_BUF_PERF(SPI_RX, HAL_SLAVE_SEL, data_len), 0, testdataslen);
					s_slen = hal_spi_slaver_tx(spi_s, SPI_SEL_BUF_PERF(SPI_TX, HAL_SLAVE_SEL, data_len), testdataslen, ONEMINUTE);
					if(s_slen != testdataslen){
						HAL_TEST_DBG("sla txlen error\n");
						break;
					}
					at_gpio_write(GPIO_SYN, GPIO_PIN_SET);
					//wl_os_mdelay(5);
					wl_os_mdelay(1);
					at_gpio_write(GPIO_SYN, GPIO_PIN_RESET);//pull down  //mst rec signal must be waiting 5ms for sla be into rx statue.
					
					
					
					s_rlen = hal_spi_slaver_rx(spi_s, SPI_SEL_BUF_PERF(SPI_RX, HAL_SLAVE_SEL, data_len), testdataslen, ONEMINUTE);//stop wait mst clk
					if(s_rlen != testdataslen){
						HAL_TEST_DBG("sla rxlen error\n");
						break;
					}
					
									
					if(memcmp(SPI_SEL_BUF_PERF(SPI_TX, HAL_MASTER_SEL, data_len), SPI_SEL_BUF_PERF(SPI_RX, HAL_SLAVE_SEL, data_len), testdataslen)){
						HAL_TEST_DBG("sla rx datas error\n");
						
						HAL_TEST_DBG("master tx:\n");
						HAL_DBG_ARRARY(SPI_SEL_BUF_PERF(SPI_TX, HAL_MASTER_SEL, data_len), testdataslen, ARY_U8, 0);
						HAL_TEST_DBG("slaver rx:\n");
						HAL_DBG_ARRARY(SPI_SEL_BUF_PERF(SPI_RX, HAL_SLAVE_SEL, data_len), testdataslen, ARY_U8, 0);		
						test->result = FALSE;
						break;
					}
					
					//wait for mst say ok
					at_gpio_it_start(GPIO_SYN_B);
					ret = wl_wait_sema(&clock_syn_b_sema, ONEMINUTE);
					if(!ret){
						HAL_TEST_DBG("sla wait mst rx done sema time out\n");
						break;
					}
		
	
#endif					

#if 1 //sla int 
					
					
					s_sta = s907x_hal_spi_slaver_xfer_interrupt(spi_s, SPI_SEL_BUF_PERF(SPI_TX, HAL_SLAVE_SEL, data_len), testdataslen);
					if(s_sta != HAL_OK){
						HAL_TEST_DBG("sla tx error\n");
						break;
					}
					
					ret = wl_wait_sema(&spi_sla_tx_sema, ONEMINUTE);
					if(!ret){
						HAL_TEST_DBG("sla tx timeout\n");
						break;
					}
					
					memset(SPI_SEL_BUF_PERF(SPI_RX, HAL_SLAVE_SEL, data_len), 0, testdataslen);
					s_sta = s907x_hal_spi_slaver_recv_interrupt(spi_s, SPI_SEL_BUF_PERF(SPI_RX, HAL_SLAVE_SEL, data_len), testdataslen);
					if(s_sta != HAL_OK){
						HAL_TEST_DBG("sla rx error\n");
						break;
					}
					
					at_gpio_write(GPIO_SYN, GPIO_PIN_SET);
					//wl_os_mdelay(5);//1ms
					at_gpio_write(GPIO_SYN, GPIO_PIN_RESET);
					
					
					ret = wl_wait_sema(&spi_sla_rx_sema, ONEMINUTE);//wait
					if(!ret){
						HAL_TEST_DBG("sla rx timeout\n");
						break;
					}
					
					if(memcmp(SPI_SEL_BUF_PERF(SPI_TX, HAL_MASTER_SEL, data_len), SPI_SEL_BUF_PERF(SPI_RX, HAL_SLAVE_SEL, data_len), testdataslen)){
						HAL_TEST_DBG("sla rx datas error\n");
						
						HAL_TEST_DBG("master tx:\n");
						HAL_DBG_ARRARY(SPI_SEL_BUF_PERF(SPI_TX, HAL_MASTER_SEL, data_len), testdataslen, ARY_U8, 0);
						HAL_TEST_DBG("slaver rx:\n");
						HAL_DBG_ARRARY(SPI_SEL_BUF_PERF(SPI_RX, HAL_SLAVE_SEL, data_len), testdataslen, ARY_U8, 0);		
						test->result = FALSE;
						break;
					}
					
					at_gpio_it_start(GPIO_SYN_B);//open int
					ret = wl_wait_sema(&clock_syn_b_sema, ONEMINUTE);//wait
					if(!ret){
						HAL_TEST_DBG("sla wait mst rx done sema time out\n");
						break;
					}
					
					

				
					
#endif
					ret = wl_wait_sema(&spi_thread_destory_sema, 1);
					if(ret){
						break;
					}
					
					newstarttest = TRUE;
					totalDatas += testdataslen;
				
			}
			
exit:
			wl_free_sema(&spi_thread_destory_sema);
			wl_free_sema(&spi_sla_rx_sema);
			wl_free_sema(&spi_sla_tx_sema);
			wl_free_sema(&clock_syn_b_sema);
			wl_free(mst_txbuf_8bit);
			wl_free(sla_txbuf_8bit);
			wl_free(mst_txbuf_16bit);
			wl_free(sla_txbuf_16bit);
			wl_free(mst_rxbuf_8bit);
			wl_free(sla_rxbuf_8bit);
			wl_free(mst_rxbuf_16bit);
			wl_free(sla_rxbuf_16bit);
			at_test_led_deint(1);
			s907x_hal_timer_base_deinit(&timx_hdl);
			newstarttest = FALSE;
			totalDatas = 0;
			runtime_cnt = 0;
			HAL_TEST_DBG("exit spi task\n");
			wl_destory_threadself();
}




void spi_master_init(u32 datelen, u32 buad, u8 pha, u8 pol)
{
			perf_spi_master_hdl.config.idx = SPI_MASTER_IDX;
			perf_spi_master_hdl.config.spi_master = HAL_MASTER_SEL;
			perf_spi_master_hdl.config.datalen = BIT(datelen - 4);
			perf_spi_master_hdl.config.clk_phase = pha;
			perf_spi_master_hdl.config.clk_polarity = pol;
			perf_spi_master_hdl.config.clk_speed = buad;
			perf_spi_master_hdl.config.mode = SPI_MODE_TXRX;

			perf_spi_master_hdl.it.tx_complete.func= perf_master_tx_int;
			perf_spi_master_hdl.it.tx_complete.context = &perf_spi_master_hdl;

			perf_spi_master_hdl.it.rx_complete.func = perf_master_rx_int;
			perf_spi_master_hdl.it.rx_complete.context = &perf_spi_master_hdl;

			s907x_hal_spi_init(&perf_spi_master_hdl);
}

static void spi_mst_task(hal_test_t *test)
{
			
			hal_status_e m_sta;
			u32 ret;
			int mst_slen = 0;
			u16 testdataslen;
			spi_hdl_t *spi_m = &perf_spi_master_hdl;

			u32 data_len, buad, clk_pha, clk_pol;

			//SPI_TEST_MODE mode;

			testdataslen = (u16)test->arg[0];             
			data_len = test->arg[1];
			buad     = test->arg[2];
			clk_pha  = HI_UINT8(test->arg[3]);
			clk_pol  = LO_UINT8(test->arg[3]); 

			//pi_testdata_init(test);//data init
			spi_perf_testdata_init(test);
			
			ASSERT(data_len >= 4 && data_len <= 16);
			
			spi_master_init(data_len, buad, clk_pha, clk_pol);
			at_clock_syn_init(in_sel, GPIO_SYN);//pull up  in
			at_clock_syn_init(out_sel, GPIO_SYN_B);//pull up out
			
			wl_init_sema(&spi_mst_tx_sema, 0, sema_binary);
			wl_init_sema(&spi_mst_rx_sema, 0, sema_binary);
			wl_init_sema(&clock_syn_sema, 0, sema_binary);
			
			
			master_tim_sel = TRUE;
			at_gpio_write(GPIO_SYN, GPIO_PIN_SET);
			at_gpio_write(GPIO_SYN_B, GPIO_PIN_SET);	
			while(1)
			{
									
#if 0//mst poll 

				mst_slen = hal_spi_master_txrx(spi_m, SPI_SEL_BUF_PERF(SPI_TX, HAL_MASTER_SEL, data_len), SPI_SEL_BUF_PERF(SPI_RX, HAL_MASTER_SEL, data_len), testdataslen, SPI_POLL_LIMIT);
				if(mst_slen != testdataslen){
					HAL_TEST_DBG("mst_slen error\n");
					break;
				}
				
				if(memcmp(SPI_SEL_BUF_PERF(SPI_RX, HAL_MASTER_SEL, data_len), SPI_SEL_BUF_PERF(SPI_TX, HAL_SLAVE_SEL, data_len), testdataslen)) {
					HAL_TEST_DBG("mst rx datas error\n");
					
					
					HAL_TEST_DBG("spi it test fail\n");
					HAL_TEST_DBG("master rx:\n");
					HAL_DBG_ARRARY(SPI_SEL_BUF_PERF(SPI_RX, HAL_MASTER_SEL, data_len), testdataslen, ARY_U8, 0);
					HAL_TEST_DBG("slaver tx:\n");
					HAL_DBG_ARRARY(SPI_SEL_BUF_PERF(SPI_TX, HAL_SLAVE_SEL, data_len), testdataslen, ARY_U8, 0);
					test->result = FALSE;
					break;
				}
				
				at_gpio_write(GPIO_SYN_B, GPIO_PIN_SET);//pull up
				//wl_os_mdelay(5);
				wl_os_mdelay(1);
				at_gpio_write(GPIO_SYN_B, GPIO_PIN_RESET);//pull down

				at_gpio_it_start(GPIO_SYN);
				ret = wl_wait_sema(&clock_syn_sema, ONEMINUTE);
				if(!ret){
					HAL_TEST_DBG("mst wait sla tx done sema time out\n");
					break;
				}
					
#endif

#if 1//mst int
				
				memset(SPI_SEL_BUF_PERF(SPI_RX, HAL_MASTER_SEL, data_len), 0, testdataslen);
				m_sta = s907x_hal_spi_master_recv_interrupt(spi_m, SPI_SEL_BUF_PERF(SPI_RX, HAL_MASTER_SEL, data_len), testdataslen);
				if(m_sta != HAL_OK){
					HAL_TEST_DBG("mst rx error\n");
					break;
				}
					
				m_sta = s907x_hal_spi_master_xfer_interrupt(spi_m, SPI_SEL_BUF_PERF(SPI_TX, HAL_MASTER_SEL, data_len), testdataslen);
				if(m_sta != HAL_OK){
					HAL_TEST_DBG("mst tx error\n");
					break;
				}
					
				ret = wl_wait_sema(&spi_mst_tx_sema, ONEMINUTE);
				if(!ret){
					HAL_TEST_DBG("mst tx timeout\n");
					break;
				}
					
				ret = wl_wait_sema(&spi_mst_rx_sema, ONEMINUTE);
				if(!ret){
					HAL_TEST_DBG("mst rx timeout\n");
					break;
				}
	
				if(memcmp(SPI_SEL_BUF_PERF(SPI_RX, HAL_MASTER_SEL, data_len), SPI_SEL_BUF_PERF(SPI_TX, HAL_SLAVE_SEL, data_len), testdataslen)){
					HAL_TEST_DBG("mst rx datas error\n");
					
					
					HAL_TEST_DBG("spi it test fail\n");
					HAL_TEST_DBG("master rx:\n");
					HAL_DBG_ARRARY(SPI_SEL_BUF_PERF(SPI_RX, HAL_MASTER_SEL, data_len), testdataslen, ARY_U8, 0);
					HAL_TEST_DBG("slaver tx:\n");
					HAL_DBG_ARRARY(SPI_SEL_BUF_PERF(SPI_TX, HAL_SLAVE_SEL, data_len), testdataslen, ARY_U8, 0);
					test->result = FALSE;
					break;
				}
		
				//1ms
				at_gpio_write(GPIO_SYN_B, GPIO_PIN_SET);
				wl_os_mdelay(1);
				at_gpio_write(GPIO_SYN_B, GPIO_PIN_RESET);
		
				//mst wait sla say ready
				at_gpio_it_start(GPIO_SYN);
				ret = wl_wait_sema(&clock_syn_sema, ONEMINUTE);
				if(!ret){
					HAL_TEST_DBG("mst wait sla tx done sema time out\n");
					break;
				}

				
#endif	
				ret = wl_wait_sema(&spi_thread_destory_sema, 1);
				if(ret){
					break;
				}
				newstarttest = TRUE;
				totalDatas += testdataslen;
			}
			
exit:
			wl_free_sema(&spi_thread_destory_sema);
			wl_free_sema(&spi_mst_rx_sema);
			wl_free_sema(&spi_mst_tx_sema);
			wl_free_sema(&clock_syn_sema);
			wl_free(mst_txbuf_8bit);
			wl_free(sla_txbuf_8bit);
			wl_free(mst_txbuf_16bit);
			wl_free(sla_txbuf_16bit);
			wl_free(mst_rxbuf_8bit);
			wl_free(sla_rxbuf_8bit);
			wl_free(mst_rxbuf_16bit);
			wl_free(sla_rxbuf_16bit);
			at_test_led_deint(0);
			s907x_hal_timer_base_deinit(&timx_hdl);
			newstarttest = FALSE;
			totalDatas = 0;
			runtime_cnt = 0;
			HAL_TEST_DBG("exit spi task\n");
			wl_destory_threadself();
			
			
}

static void spi_performance_test(hal_test_t *test)
{
	
	u8 mst_sel = test->arg[5];
	u8 thread_switch = test->arg[6];



	if(mst_sel == 1)//sla
	{
		if(thread_switch == 1){	//start
		
			wl_init_sema(&spi_thread_destory_sema, 0, sema_binary);
				
			at_test_led_init(1);
			
			at_test_timer_init(test);
			
			wl_create_thread("spi sla thread", 512, MAIN_UART_RX_PRIO , (thread_func_t)spi_sla_task, test);
			
			HAL_TEST_DBG("sla ready, waiting mst ...");
		} else{
			wl_send_sema(&spi_thread_destory_sema);
			
			wl_os_mdelay(500);
		}
	}
	else//mst
	{
		if(thread_switch == 1) {//start
		
			wl_init_sema(&spi_thread_destory_sema, 0, sema_binary);
				
			at_test_led_init(0);
			
			at_test_timer_init(test);
			
			wl_create_thread("spi mst thread", 512, MAIN_UART_RX_PRIO, (thread_func_t)spi_mst_task, test);
			
			HAL_TEST_DBG("mst runing...\n");
		} else {

			wl_send_sema(&spi_thread_destory_sema);
			
			wl_os_mdelay(500);
		}

	}
			
}

/*
*	no: 	test item     	0-4
*	dfs:	dataframesize 	4~16
*	buad:   buadrate		(115200)
*	pha:    clk_phase       0-1
*	pol:    clk_polarity    0-1
*	trx:	tx/rx/trx       (0:tx 1:rx 2:trx)
*/

/*		test   	 no	 dlen    buad   pha pol trx  stx  mtx
*       @SPI     0    4-16  2500000	 0	 0	 2	 0-2  0-2
*       @SPI     1    4-16 	2500000  0 	 0	 2	 0-2  0-2  
*       @SPI     2    4-16  2500000  0 	 0	 2	 0-2  0-2 
*       @SPI     3    4-16 	2500000  0	 0 	 2   0-2  0-2 
*/


void spi_test(hal_test_t *test)
{
	ASSERT(test);
	ASSERT(test->no < (sizeof(spi_test_map)/sizeof(hal_test_name_map_t)));
	
	HAL_TEST_DBG("test no %d = %s\n", test->no, spi_test_map[test->no].name);
	
	if(test->no == SPI_PERFORMANCE_TEST)
	{
		spi_performance_test(test);
		return;
	}
    spi_test_enter(test);
    
    spi_test_exit(test);
}

#endif