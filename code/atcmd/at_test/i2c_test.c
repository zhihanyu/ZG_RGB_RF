#include "s907x.h"
#include "i2c_test.h"
#include "gpio_test.h"
#include "hal_timer.h"
#include "timer_test.h"

#if M_AT_TEST

static sema_t i2c_sla_rx_done_sema;
static sema_t i2c_sla_tx_done_sema;
static sema_t i2c_mst_rx_done_sema;
static sema_t i2c_mst_tx_done_sema;

static sema_t i2c_thread_destory_sema;
static mutex_t i2c_mutex_sema;

static i2c_hdl_t i2c_master_type;
static i2c_hdl_t i2c_slaver_type;

#define I2C_TEST_LEN  	16
#define TIME_OUT_SYN	1000
#define		ONEMINUTE		60000
static u8 send_buf[I2C_TEST_LEN];
static u8 receive_buf[I2C_TEST_LEN];

static u8  i2c_sla_txbuffer[I2C_TEST_LEN];
static u8  i2c_sla_rxbuffer[I2C_TEST_LEN];
static u8  i2c_mst_txbuffer[I2C_TEST_LEN];
static u8  i2c_mst_rxbuffer[I2C_TEST_LEN];

static int master_tx_it_down = 0;
static int master_rx_it_down = 0;
static int slaver_tx_it_down = 0;
static int slaver_rx_it_down = 0;

static int master_tx_dma_down = 0;
static int master_rx_dma_down = 0;
static int slaver_tx_dma_down = 0;
static int slaver_rx_dma_down = 0;

//extern
extern timer_hdl_t timx_hdl;
extern u8 master_tim_sel;
extern u32      totalDatas;
extern u32		runtime_cnt;
extern u8		newstarttest;
extern sema_t 	clock_syn_sema;
extern sema_t	clock_syn_b_sema;
extern u8 SYN;

void gpio_set_pull(u32 gpio_pin, u8 pull)
{
	u32 pin = BIT(gpio_pin);
	u8 pull_mode = pull;

	s907x_hal_gpio_set_pull(pin, pull_mode);
}
 

void s907x_hal_i2c_msp_init(i2c_hdl_t *i2c)
{
    if(i2c->config.idx == I2C_IDX_0) {
        I2C0_SCL_SEL2(HAL_ON);
        I2C0_SDA_SEL2(HAL_ON);
		//pull up
		gpio_set_pull(22,1);
		gpio_set_pull(19,1);
		
    } else {
        I2C1_SCL_SEL2(HAL_ON);
        I2C1_SDA_SEL2(HAL_ON);
		//pull up
		gpio_set_pull(18,1);
		gpio_set_pull(23,1);
    }
}

void s907x_hal_i2c_msp_deinit(i2c_hdl_t *i2c)
{
    if(i2c->config.idx == I2C_IDX_0) {
        I2C0_SCL_SEL2(HAL_OFF);
        I2C0_SDA_SEL2(HAL_OFF);
		//pull up
		gpio_set_pull(22,1);
		gpio_set_pull(19,1);
    } else {
        I2C1_SCL_SEL2(HAL_OFF);
        I2C1_SDA_SEL2(HAL_OFF);
		//pull up
		gpio_set_pull(18,1);
		gpio_set_pull(23,1);
    }
}


void  performace_tx_interrupt_hdl(void *contex) 
{
    i2c_hdl_t *i2c = (i2c_hdl_t*)contex;

    ASSERT(i2c);

    if(i2c->config.i2c_master == HAL_MASTER_SEL)
    {
		wl_send_sema_fromisr(&i2c_mst_tx_done_sema);
       //wl_send_sema(&i2c_mst_tx_done_sema);
    }
    
    if(i2c->config.i2c_master == HAL_SLAVE_SEL)
    {
		wl_send_sema_fromisr(&i2c_sla_tx_done_sema);
        //wl_send_sema(&i2c_sla_tx_done_sema);
    }
	//HAL_TEST_DBG("finish it tx, sema send\n");
}

void performace_rx_interrupt_hdl(void *contex) 
{
    i2c_hdl_t *i2c = (i2c_hdl_t*)contex;

    ASSERT(i2c);

    if(i2c->config.i2c_master == HAL_MASTER_SEL)
    {
		
		wl_send_sema_fromisr(&i2c_mst_rx_done_sema);
        //wl_send_sema(&i2c_mst_rx_done_sema);

    }
    
    if(i2c->config.i2c_master == HAL_SLAVE_SEL)
    {
		wl_send_sema_fromisr(&i2c_sla_rx_done_sema);
        //wl_send_sema(&i2c_sla_rx_done_sema);
    }

	//HAL_TEST_DBG("finish it rx, sema send\n");
}


void tx_interrupt_hdl(void *contex) 
{
    i2c_hdl_t *i2c = (i2c_hdl_t*)contex;

    ASSERT(i2c);

    if(i2c->config.i2c_master == HAL_MASTER_SEL)
    {
        master_tx_it_down = 1;
    }
    
    if(i2c->config.i2c_master == HAL_SLAVE_SEL)
    {
        slaver_tx_it_down = 1;
    }
	HAL_TEST_DBG("finish it tx\n");
}

void rx_interrupt_hdl(void *contex) 
{
    i2c_hdl_t *i2c = (i2c_hdl_t*)contex;

    ASSERT(i2c);

    if(i2c->config.i2c_master == HAL_MASTER_SEL)
    {
        master_rx_it_down = 1;

    }
    
    if(i2c->config.i2c_master == HAL_SLAVE_SEL)
    {
        slaver_rx_it_down = 1;
    }

	HAL_TEST_DBG("finish it rx\n");
}



void tx_dma_hdl(void *contex) 
{
    i2c_hdl_t *i2c = (i2c_hdl_t*)contex;

    ASSERT(i2c);

    if(i2c->config.i2c_master == HAL_MASTER_SEL)
    {
        master_tx_dma_down = 1;

    }
    
    if(i2c->config.i2c_master == HAL_SLAVE_SEL)
    {
        slaver_tx_dma_down = 1;
    }
	HAL_TEST_DBG("finish dma tx\n");
}

void rx_dma_hdl(void *contex) 
{
    i2c_hdl_t *i2c = (i2c_hdl_t*)contex;

    ASSERT(i2c);

    if(i2c->config.i2c_master == HAL_MASTER_SEL)
    {
        master_rx_dma_down = 1;

    }
    
    if(i2c->config.i2c_master == HAL_SLAVE_SEL)
    {
        slaver_rx_dma_down = 1;
    }

	HAL_TEST_DBG("finish dma rx\n");
}


static void i2c_test_data_init(void)
{
	int i;

	for(i = 0;i < I2C_TEST_LEN; i++) {
		send_buf[i] = i;
	}
	for(i = 0;i < I2C_TEST_LEN; i++) {
		receive_buf[i] = 0;
	}
}

void i2c_test_poll(hal_test_t *test)
{
	i2c_hdl_t *i2c_m = &i2c_master_type;
	i2c_hdl_t *i2c_s = &i2c_slaver_type;
    int test_slaver_rx = test->arg[1];
    
	hal_status_e m_sta ;
	hal_status_e s_sta ;
    
	ASSERT(test);

    if(!test_slaver_rx) {
        i2c_test_data_init();
        //master -> slaver
        HAL_TEST_DBG("i2c slaver rx poll mode\n");
        if(test->arg[0] & I2C_MST_IDX) {
            m_sta = s907x_hal_i2c_master_xfer(i2c_m, send_buf, I2C_TEST_LEN, I2C_MST_POLL_WAIT);
            if(m_sta != HAL_OK) {
                test->result = FALSE;
                HAL_TEST_DBG("%s fail\n", __func__);
                goto exit;
            }
        }	
        if(test->arg[0] & I2C_SLV_IDX) {
            s_sta = s907x_hal_i2c_slavor_recv(i2c_s,receive_buf, I2C_TEST_LEN, I2C_SLV_POLL_WAIT);
            if(s_sta != HAL_OK) {
                test->result = FALSE;
                HAL_TEST_DBG("%s fail\n", __func__);
                goto exit;
            }
            if(memcmp(send_buf, receive_buf, I2C_TEST_LEN)) {
                test->result = FALSE;
                HAL_TEST_DBG("send buf = \n");
                HAL_DBG_ARRARY(send_buf, I2C_TEST_LEN, ARY_U8, 0);
                HAL_TEST_DBG("receive buf = \n");
                HAL_DBG_ARRARY(receive_buf, I2C_TEST_LEN, ARY_U8, 0);
                HAL_TEST_DBG("%s fail\n", __func__);
                goto exit;
            }
        }
    } else {
        //test master rx poll slaver tx poll
        i2c_test_data_init();
        HAL_TEST_DBG("master rx poll mode can not support, recommend by interrupt or dma mode\n");
    }
exit:
	HAL_TEST_DBG("finish i2c bassic test \n");

} 

           

void i2c_test_interrupt(hal_test_t *test)
{
	i2c_hdl_t *i2c_m = &i2c_master_type;
	i2c_hdl_t *i2c_s = &i2c_slaver_type;
    int test_slaver_rx = test->arg[1];

	hal_status_e m_sta ;
	hal_status_e s_sta ;

    if(!test_slaver_rx) {
        //test slaver rx it master tx poll/it
        i2c_test_data_init();
        //step 1 master to slaver
        master_tx_it_down = slaver_rx_it_down = 0;
        HAL_TEST_DBG("i2c slaver it rx\n");
        if(test->arg[0] & I2C_SLV_IDX) {
            s_sta = s907x_hal_i2c_slavor_recv_interrupt(i2c_s, receive_buf, I2C_TEST_LEN);
            if(s_sta != HAL_OK) {
                goto exit;
            }
        }
        //master send data
        if(test->arg[0] & I2C_MST_IDX) {
            if(test->arg[7] == 0) {
                HAL_TEST_DBG("i2c slaver it rx, master tx poll mode\n");    
                m_sta = s907x_hal_i2c_master_xfer(i2c_m, send_buf, I2C_TEST_LEN, 1000);
                if(m_sta != HAL_OK) {
                    goto exit;
                }
                master_tx_it_down = 1;
            } else if(test->arg[7] == 1) {
                HAL_TEST_DBG("i2c slaver it rx, master tx it mode\n");    
                m_sta = s907x_hal_i2c_master_xfer_interrupt(i2c_m, send_buf, I2C_TEST_LEN);
                if(m_sta != HAL_OK) {
                    goto exit;
                }
            }
        }
        //master wait tx finish
        if(test->arg[0] & I2C_MST_IDX) {
            while(!master_tx_it_down);
        }
        //slaver wait rx finish
        if(test->arg[0] & I2C_SLV_IDX) {
            while(!slaver_rx_it_down);
             //check result
            if(memcmp(send_buf, receive_buf, I2C_TEST_LEN)) {
                test->result = FALSE;
                HAL_TEST_DBG("send buf = \n");
                HAL_DBG_ARRARY(send_buf, I2C_TEST_LEN, ARY_U8, 0);
                HAL_TEST_DBG("receive buf = \n");
                HAL_DBG_ARRARY(receive_buf, I2C_TEST_LEN, ARY_U8, 0);
                HAL_TEST_DBG("%s fail\n", __func__);
                goto exit;
            }
        }  
    } else {
        //test master rx it slaver tx poll/it
        i2c_test_data_init();
        //step 2 slaver to master
        master_rx_it_down = slaver_tx_it_down = 0;
        master_tx_it_down = slaver_rx_it_down = 0;
        HAL_TEST_DBG("i2c master it rx\n");
        if(test->arg[0] & I2C_SLV_IDX) { 
            if(test->arg[8] == 0) {
                HAL_TEST_DBG("i2c master it rx, slaver tx poll can not support\n");
                goto exit;
            } else if(test->arg[8] == 1) {
                HAL_TEST_DBG("i2c master it rx, slaver tx it\n");
                s907x_hal_i2c_slavor_xfer_interrupt(i2c_s, send_buf, I2C_TEST_LEN);
            }   
        }
        if(test->arg[0] & I2C_MST_IDX) {
            s907x_hal_i2c_master_recv_interrupt(i2c_m, receive_buf, I2C_TEST_LEN);
        }
        if(test->arg[0] & I2C_MST_IDX) {
            while(!master_rx_it_down);
            //check result
            if(memcmp(send_buf, receive_buf, I2C_TEST_LEN)) {
                test->result = FALSE;
                HAL_TEST_DBG("send buf = \n");
                HAL_DBG_ARRARY(send_buf, I2C_TEST_LEN, ARY_U8, 0);
                HAL_TEST_DBG("receive buf = \n");
                HAL_DBG_ARRARY(receive_buf, I2C_TEST_LEN, ARY_U8, 0);
                HAL_TEST_DBG("%s fail\n", __func__);
                goto exit;
            }
        }
       if(test->arg[0] & I2C_SLV_IDX) {
            while(!slaver_tx_it_down);
        }
    }
exit:
	HAL_TEST_DBG("finish i2c interrupt test \n");

}            

void i2c_test_dma(hal_test_t *test)
{
	ASSERT(test);
	i2c_hdl_t *i2c_m = &i2c_master_type;
	i2c_hdl_t *i2c_s = &i2c_slaver_type;
    int test_slaver_rx = test->arg[1];

    //test slaver rx dma master tx poll/it/dma
    if(!test_slaver_rx) {
        hal_status_e m_sta ;
        hal_status_e s_sta ;

        i2c_test_data_init();
        master_tx_it_down = slaver_rx_it_down = 0;
        master_tx_dma_down = slaver_rx_dma_down = 0;

        HAL_TEST_DBG("i2c slaver dma rx\n");
        if(test->arg[0] & I2C_SLV_IDX) {
            s_sta = s907x_hal_i2c_slavor_recv_dma(i2c_s, receive_buf, I2C_TEST_LEN);
            if(s_sta != HAL_OK) {
                goto exit;
            }
        } 
        //master tx
        if(test->arg[0] & I2C_MST_IDX) {
            if(test->arg[7] == 0) {
                HAL_TEST_DBG("i2c slaver dma rx, master tx poll mode\n");    
                m_sta = s907x_hal_i2c_master_xfer(i2c_m, send_buf, I2C_TEST_LEN, 1000);
                if(m_sta != HAL_OK) {
                    goto exit;
                }
                master_tx_it_down = 1;
            } else if(test->arg[7] == 1) {
                HAL_TEST_DBG("i2c slaver dma rx, master tx it mode\n");    
                m_sta = s907x_hal_i2c_master_xfer_interrupt(i2c_m, send_buf, I2C_TEST_LEN);
                if(m_sta != HAL_OK) {
                    goto exit;
                }
            } else if(test->arg[7] == 2) {
                HAL_TEST_DBG("i2c slaver dma rx, master tx dma mode\n");    
                m_sta = s907x_hal_i2c_master_xfer_dma(i2c_m, send_buf, I2C_TEST_LEN);
                if(m_sta != HAL_OK) {
                    goto exit;
                }
            }
        }

        //master wait tx finish
        if(test->arg[0] & I2C_MST_IDX) {
            //poll/it or dma tx finish
            while(!master_tx_it_down && !master_tx_dma_down);
        }
        //slaver wait rx finish
        if(test->arg[0] & I2C_SLV_IDX) {
            while(!slaver_rx_dma_down);
             //check result
            if(memcmp(send_buf, receive_buf, I2C_TEST_LEN)) {
                test->result = FALSE;
                HAL_TEST_DBG("send buf = \n");
                HAL_DBG_ARRARY(send_buf, I2C_TEST_LEN, ARY_U8, 0);
                HAL_TEST_DBG("receive buf = \n");
                HAL_DBG_ARRARY(receive_buf, I2C_TEST_LEN, ARY_U8, 0);
                HAL_TEST_DBG("%s fail\n", __func__);
                goto exit;
            }
        } 
    } else {
        i2c_test_data_init();
        //test slaver to master
        master_tx_it_down = slaver_rx_it_down = 0;  
        master_rx_dma_down = slaver_tx_dma_down = 0;

        HAL_TEST_DBG("i2c master dma rx\n");
 
        //slaver tx
        if(test->arg[0] & I2C_SLV_IDX) { 
            if(test->arg[8] == 0) {
                HAL_TEST_DBG("i2c master it rx, slaver tx poll can not support\n"); 
                goto exit;
            } else if(test->arg[8] == 1) {
                HAL_TEST_DBG("i2c master dma rx, slaver tx it\n");
                s907x_hal_i2c_slavor_xfer_interrupt(i2c_s, send_buf, I2C_TEST_LEN);
            } else if(test->arg[8] == 2) {
                HAL_TEST_DBG("i2c master dma rx, slaver tx dma\n");
                s907x_hal_i2c_slavor_xfer_dma(i2c_s, send_buf, I2C_TEST_LEN);
            }      
        }

        if(test->arg[0] & I2C_MST_IDX) {
            s907x_hal_i2c_master_recv_dma(i2c_m, receive_buf, I2C_TEST_LEN);
        } 

        if(test->arg[0] & I2C_MST_IDX) {
            while(!master_rx_dma_down);
            //check result
            if(memcmp(send_buf, receive_buf, I2C_TEST_LEN)) {
                test->result = FALSE;
                HAL_TEST_DBG("send buf = \n");
                HAL_DBG_ARRARY(send_buf, I2C_TEST_LEN, ARY_U8, 0);
                HAL_TEST_DBG("receive buf = \n");
                HAL_DBG_ARRARY(receive_buf, I2C_TEST_LEN, ARY_U8, 0);
                HAL_TEST_DBG("%s fail\n", __func__);
                goto exit;
            }
        }
        if(test->arg[0] & I2C_SLV_IDX) {
            //wait slaver tx it or dma finish
            while(!slaver_tx_it_down && !slaver_tx_dma_down);
        }
    }
exit:
    HAL_TEST_DBG("finish i2c dma test \n");
}



//example
hal_test_name_map_t i2c_test_map[] = 
{
	{0, "test slaver/master rx poll mode"},
	{1, "test slaver/master rx it mode"},
	{2, "test slaver/master rx dma mode"},
    {3, "test loopback mode"},
	{4, "i2c performance test"}
};


static u32 debug_buf[128];

static void i2c_test_enter(hal_test_t *test)
{
	u16 i2c_clock,i2c_addr_mode,i2c_own_addr,i2c_target_addr,i2c_general_call;
	int mode;
	i2c_hdl_t *i2c_m = &i2c_master_type;
	i2c_hdl_t *i2c_s = &i2c_slaver_type;

	ASSERT(test);
	
	mode = test->arg[0];
	i2c_addr_mode = test->arg[2]; //7bits or 10bits addrress mode
	i2c_clock = test->arg[3]; //khz
	i2c_own_addr = test->arg[4];
	i2c_target_addr = test->arg[5];
	i2c_general_call = test->arg[6]; //0

	// config i2c_mst_type
	if(mode & I2C_MST_IDX) {
        i2c_m->config.idx = I2C_IDX_0;  
		i2c_m->config.addr_mode = i2c_addr_mode;
		i2c_m->config.clock = i2c_clock;
		i2c_m->config.dir = HAL_DIR_TX;
		i2c_m->config.general_call = i2c_general_call;
		i2c_m->config.i2c_master = HAL_MASTER_SEL;
		i2c_m->config.own_addr = i2c_own_addr;
		i2c_m->config.target_addr = i2c_target_addr;

		i2c_m->it.rx_complete.func = rx_interrupt_hdl;	
		i2c_m->it.tx_complete.func = tx_interrupt_hdl;
		i2c_m->it.rx_complete.context = i2c_m;
		i2c_m->it.tx_complete.context = i2c_m;

		i2c_m->dma.rx_complete.func = rx_dma_hdl;
		i2c_m->dma.rx_complete.context = i2c_m;

		i2c_m->dma.tx_complete.func = tx_dma_hdl;
		i2c_m->dma.tx_complete.context = i2c_m;

        i2c_m->it.txbuf = (u8*)debug_buf;

		s907x_hal_i2c_init(i2c_m);
	}

	// config i2c_slv_type
	if(mode & I2C_SLV_IDX) {
        i2c_s->config.idx = I2C_IDX_1;
		i2c_s->config.addr_mode = i2c_addr_mode;
		i2c_s->config.clock = i2c_clock;
		i2c_s->config.dir = HAL_DIR_RX;
		i2c_s->config.general_call = i2c_general_call;
		i2c_s->config.i2c_master = HAL_SLAVE_SEL;
        if(mode == I2C_TEST_LOOPBACK) {
            i2c_s->config.own_addr = i2c_target_addr;  //loopback set slaver master target address
            i2c_s->config.target_addr = i2c_own_addr;
        } else {
            i2c_s->config.own_addr = i2c_own_addr;
            i2c_s->config.target_addr = i2c_target_addr;
        }
		i2c_s->it.rx_complete.func = rx_interrupt_hdl;
		i2c_s->it.rx_complete.context = i2c_s;

		i2c_s->it.tx_complete.func = tx_interrupt_hdl;
		i2c_s->it.tx_complete.context = i2c_s;

		i2c_s->dma.rx_complete.func = rx_dma_hdl;
		i2c_s->dma.rx_complete.context = i2c_s;

		i2c_s->dma.tx_complete.func = tx_dma_hdl;
		i2c_s->dma.tx_complete.context = i2c_s;

		s907x_hal_i2c_init(i2c_s);
	}

}


//test  loopback / poll int dma / master for i2c0 & i2c1
void i2c_test_loopback(hal_test_t *test)
{
  
  u16 i2c_clock,i2c_addr_mode,i2c_own_addr,i2c_target_addr,i2c_general_call;
  
  i2c_hdl_t *i2c_m = &i2c_master_type;
  i2c_hdl_t *i2c_s = &i2c_slaver_type;
  int i2c0_for_mst;
  u8 delay_cnt = 0;
  hal_status_e m_sta ;
  hal_status_e s_sta ;
  
  s907x_hal_i2c_deinit(i2c_m);
  s907x_hal_i2c_deinit(i2c_s);
  
  ASSERT(test);
  i2c0_for_mst = test->arg[1];//arg[1] = 0  i2c0-mst
  i2c_addr_mode = test->arg[2]; //7bits or 10bits addrress mode
  i2c_clock = test->arg[3]; //khz
  i2c_own_addr = test->arg[4];
  i2c_target_addr = test->arg[5];
  i2c_general_call = test->arg[6]; //0
  
  //config mst
  if(!i2c0_for_mst)
  {
    i2c_m->config.idx = I2C_IDX_0;
  }
  else
  {
    i2c_m->config.idx = I2C_IDX_1;
  }
   
  i2c_m->config.addr_mode = i2c_addr_mode;
  i2c_m->config.clock = i2c_clock;
  i2c_m->config.dir = HAL_DIR_TX;
  i2c_m->config.general_call = i2c_general_call;
  i2c_m->config.i2c_master = HAL_MASTER_SEL;
  i2c_m->config.own_addr = i2c_own_addr;//55
  i2c_m->config.target_addr = i2c_target_addr;//66

  i2c_m->it.rx_complete.func = rx_interrupt_hdl;	
  i2c_m->it.tx_complete.func = tx_interrupt_hdl;
  i2c_m->it.rx_complete.context = i2c_m;
  i2c_m->it.tx_complete.context = i2c_m;

  i2c_m->dma.rx_complete.func = rx_dma_hdl;
  i2c_m->dma.rx_complete.context = i2c_m;

  i2c_m->dma.tx_complete.func = tx_dma_hdl;
  i2c_m->dma.tx_complete.context = i2c_m;

  i2c_m->it.txbuf = (u8*)debug_buf;

  s907x_hal_i2c_init(i2c_m);
  
  //config sla
  if(!i2c0_for_mst)
  {
    i2c_s->config.idx = I2C_IDX_1;
  }
  else
  {
    i2c_s->config.idx = I2C_IDX_0;
  }
  
  i2c_s->config.addr_mode = i2c_addr_mode;
  i2c_s->config.clock = i2c_clock;
  i2c_s->config.dir = HAL_DIR_RX;
  i2c_s->config.general_call = i2c_general_call;
  i2c_s->config.i2c_master = HAL_SLAVE_SEL;
  
  i2c_s->config.own_addr = i2c_target_addr;//66
  i2c_s->config.target_addr = i2c_own_addr;//55
  
  i2c_s->it.rx_complete.func = rx_interrupt_hdl;
  i2c_s->it.rx_complete.context = i2c_s;

  i2c_s->it.tx_complete.func = tx_interrupt_hdl;
  i2c_s->it.tx_complete.context = i2c_s;

  i2c_s->dma.rx_complete.func = rx_dma_hdl;
  i2c_s->dma.rx_complete.context = i2c_s;

  i2c_s->dma.tx_complete.func = tx_dma_hdl;
  i2c_s->dma.tx_complete.context = i2c_s;

  s907x_hal_i2c_init(i2c_s);
  
  if(i2c0_for_mst)
    HAL_TEST_DBG("i2c_1 for master\n");
  else
    HAL_TEST_DBG("i2c_0 for master\n");
  
  
  i2c_test_data_init();
  
  switch(test->arg[0])//test mode int or dma   #Not Support Poll
  {
    case 0://poll
      
      HAL_TEST_DBG("#Can not support poll loopback\n");
      goto interrupt;
      
    break;
    
    case 1://int
      
interrupt:
  
      HAL_TEST_DBG("#Run interrupt loopback...\n");
      
      if(!test->arg[7])
      {
        master_tx_it_down = slaver_rx_it_down = 0;
        HAL_TEST_DBG("i2c master it tx, slaver rx it\n");
        s_sta = s907x_hal_i2c_slavor_recv_interrupt(i2c_s, receive_buf, I2C_TEST_LEN);
        if(s_sta != HAL_OK) 
        {
          goto exit;
        }
        
        m_sta = s907x_hal_i2c_master_xfer_interrupt(i2c_m, send_buf, I2C_TEST_LEN);
        if(m_sta != HAL_OK) 
        {
          goto exit;
        }
      }
      else
      {
        master_rx_it_down = slaver_tx_it_down = 0;
        
        HAL_TEST_DBG("i2c master it rx, slaver tx it\n");
        
        s_sta = s907x_hal_i2c_slavor_xfer_interrupt(i2c_s, send_buf, I2C_TEST_LEN);
        if(s_sta != HAL_OK) 
        {
          goto exit;
        }
        
        m_sta = s907x_hal_i2c_master_recv_interrupt(i2c_m, receive_buf, I2C_TEST_LEN);
        if(m_sta != HAL_OK) 
        {
          goto exit;
        }      
      }
      
      //wl_os_mdelay(50);
      
      if(!test->arg[7])
      {
        while(!master_tx_it_down)
        {
          wl_os_mdelay(500);
          if(++delay_cnt >= 2)
          {
            HAL_TEST_DBG("fail i2c test loopback\n");
            return;
          }   
        }
      }
      else
      {
        while(!master_rx_it_down)
        {
          wl_os_mdelay(500);
          if(++delay_cnt >= 2)
          {
            HAL_TEST_DBG("fail i2c test loopback\n");
            return;
          } 
        }
      }
      
      if(!test->arg[7])
      {
        while(!slaver_rx_it_down)
        {
          wl_os_mdelay(500);
          if(++delay_cnt >= 2)
          {
            HAL_TEST_DBG("fail i2c test loopback\n");
            return;
          } 
        }
      }
      else
      {
        while(!slaver_tx_it_down)
        {
          wl_os_mdelay(500);
          if(++delay_cnt >= 2)
          {
            HAL_TEST_DBG("fail i2c test loopback\n");
            return;
          } 
        }
      }
      

      if(memcmp(send_buf, receive_buf, I2C_TEST_LEN)) 
      {
        test->result = FALSE;
        HAL_TEST_DBG("send buf = \n");
        HAL_DBG_ARRARY(send_buf, I2C_TEST_LEN, ARY_U8, 0);
        HAL_TEST_DBG("receive buf = \n");
        HAL_DBG_ARRARY(receive_buf, I2C_TEST_LEN, ARY_U8, 0);
        HAL_TEST_DBG("%s fail\n", __func__);
        goto exit;
      }  

    break;
    
    case 2://dma
      
      HAL_TEST_DBG("#Run dma loopback...\n");
      
      if(!test->arg[7])
      {
        master_tx_dma_down = slaver_rx_dma_down = 0;
        
        HAL_TEST_DBG("i2c master tx dma, slaver rx dma\n");
        
        s_sta = s907x_hal_i2c_slavor_recv_dma(i2c_s, receive_buf, I2C_TEST_LEN);
        if(s_sta != HAL_OK) 
        {
          goto exit;
        }
        
        m_sta = s907x_hal_i2c_master_xfer_dma(i2c_m, send_buf, I2C_TEST_LEN);
        if(m_sta != HAL_OK) 
        {
          goto exit;
        }
      }
      else
      {
        master_rx_dma_down = slaver_tx_it_down = 0;//slaver_tx_dma_down  #tao ge understand
        
        HAL_TEST_DBG("i2c master rx dma, slaver tx dma\n");
        
        s_sta = s907x_hal_i2c_slavor_xfer_dma(i2c_s,send_buf,I2C_TEST_LEN);
        if(s_sta != HAL_OK)
          goto exit;
        
        m_sta = s907x_hal_i2c_master_recv_dma(i2c_m, receive_buf, I2C_TEST_LEN);
        if(m_sta != HAL_OK)
          goto exit;
      }
      
      if(!test->arg[7])
      {
        while(!master_tx_dma_down)
        {
          wl_os_mdelay(500);
          if(++delay_cnt >= 2)
          {
            HAL_TEST_DBG("fail i2c test loopback\n");
            return;
          }
        }
      }
      else
      {
        while(!slaver_tx_it_down)//slaver_tx_dma_down
        {
          wl_os_mdelay(500);
          if(++delay_cnt >= 2)
          {
            HAL_TEST_DBG("fail i2c test loopback\n");
            return;
          }
        }
      }
      
      if(!test->arg[7])
      {
        while(!slaver_rx_dma_down)
        {
          wl_os_mdelay(500);
          if(++delay_cnt >= 2)
          {
            HAL_TEST_DBG("fail i2c test loopback\n");
            return;
          }
        }
      }
      else
      {
        while(!master_rx_dma_down)
        {
          wl_os_mdelay(500);
          if(++delay_cnt >= 2)
          {
            HAL_TEST_DBG("fail i2c test loopback\n");
            return;
          }
        }
      }
      
      if(memcmp(send_buf, receive_buf, I2C_TEST_LEN)) 
      {
        test->result = FALSE;
        HAL_TEST_DBG("send buf = \n");
        HAL_DBG_ARRARY(send_buf, I2C_TEST_LEN, ARY_U8, 0);
        HAL_TEST_DBG("receive buf = \n");
        HAL_DBG_ARRARY(receive_buf, I2C_TEST_LEN, ARY_U8, 0);
        HAL_TEST_DBG("%s fail\n", __func__);
        goto exit;
      }
      
    break;
  }
  
exit:
  HAL_TEST_DBG("finish i2c test loopback\n"); 
  

  
}

static void i2c_test_exit(hal_test_t *test)
{
	i2c_hdl_t *i2c_m = &i2c_master_type;
	i2c_hdl_t *i2c_s = &i2c_slaver_type;

	s907x_hal_i2c_deinit(i2c_m);
	s907x_hal_i2c_deinit(i2c_s);
}


static void i2c_txrx_test_buff_init(void)
{
        int i;
        
        //mst
        //tx 00 - 0f
        //rx 0f - 00
        
        //sla
        //tx 0f - 00
        //rx 00 - 0f
        

          for(i = 0; i < I2C_TEST_LEN; i++)
          {
            i2c_sla_txbuffer[i] = (0x0f - i);
            i2c_sla_rxbuffer[i] = i;
          }
  
          
          for(i = 0; i < I2C_TEST_LEN; i++)
          {
            i2c_mst_txbuffer[i] = i;
            i2c_mst_rxbuffer[i] = (0x0f - i);
          }
        
}

static void i2c_performace_test_enter(hal_test_t *test)
{
	u16 i2c_clock,i2c_addr_mode,i2c_own_addr,i2c_target_addr,i2c_general_call;
	int mode;
	i2c_hdl_t *i2c_m = &i2c_master_type;
	i2c_hdl_t *i2c_s = &i2c_slaver_type;

	ASSERT(test);
	
	mode = test->arg[0];
	i2c_addr_mode = test->arg[2]; //7bits or 10bits addrress mode
	i2c_clock = test->arg[3]; //khz
	i2c_own_addr = test->arg[4];
	i2c_target_addr = test->arg[5];
	i2c_general_call = test->arg[6]; //0

	// config i2c_mst_type
	if(mode & I2C_MST_IDX) {
        i2c_m->config.idx = I2C_IDX_0;  
		i2c_m->config.addr_mode = i2c_addr_mode;
		i2c_m->config.clock = i2c_clock;
		i2c_m->config.dir = HAL_DIR_TX;
		i2c_m->config.general_call = i2c_general_call;
		i2c_m->config.i2c_master = HAL_MASTER_SEL;
		i2c_m->config.own_addr = i2c_own_addr;
		i2c_m->config.target_addr = i2c_target_addr;

		i2c_m->it.rx_complete.func = performace_rx_interrupt_hdl;	
		i2c_m->it.tx_complete.func = performace_tx_interrupt_hdl;
		i2c_m->it.rx_complete.context = i2c_m;
		i2c_m->it.tx_complete.context = i2c_m;
/*
		i2c_m->dma.rx_complete.func = rx_dma_hdl;
		i2c_m->dma.rx_complete.context = i2c_m;

		i2c_m->dma.tx_complete.func = tx_dma_hdl;
		i2c_m->dma.tx_complete.context = i2c_m;
*/

        i2c_m->it.txbuf = (u8*)debug_buf;

		s907x_hal_i2c_init(i2c_m);
	}

	// config i2c_slv_type
	if(mode & I2C_SLV_IDX) {
        i2c_s->config.idx = I2C_IDX_1;
		i2c_s->config.addr_mode = i2c_addr_mode;
		i2c_s->config.clock = i2c_clock;
		i2c_s->config.dir = HAL_DIR_RX;
		i2c_s->config.general_call = i2c_general_call;
		i2c_s->config.i2c_master = HAL_SLAVE_SEL;
        if(mode == I2C_TEST_LOOPBACK) {
            i2c_s->config.own_addr = i2c_target_addr;  //loopback set slaver master target address
            i2c_s->config.target_addr = i2c_own_addr;
        } else {
            i2c_s->config.own_addr = i2c_own_addr;
            i2c_s->config.target_addr = i2c_target_addr;
        }
		i2c_s->it.rx_complete.func = performace_rx_interrupt_hdl;//			rx_interrupt_hdl
		i2c_s->it.rx_complete.context = i2c_s;

		i2c_s->it.tx_complete.func = performace_tx_interrupt_hdl;//		tx_interrupt_hdl
		i2c_s->it.tx_complete.context = i2c_s;
/*
		i2c_s->dma.rx_complete.func = rx_dma_hdl;
		i2c_s->dma.rx_complete.context = i2c_s;

		i2c_s->dma.tx_complete.func = tx_dma_hdl;
		i2c_s->dma.tx_complete.context = i2c_s;
*/

		s907x_hal_i2c_init(i2c_s);
	}

}


static void i2c_sla_task(void *context)
{
        hal_test_t *test = context;
        hal_status_e s_sta;
        u32 ret;
		u32 to;
		u32 start;
		u32 pass;
        
        i2c_hdl_t *i2c_s = &i2c_slaver_type;//sla
        
        i2c_performace_test_enter(test);//i2c init
        i2c_txrx_test_buff_init();//buf init
		
		//at_clock_i2c_syn_init(out_sel, GPIO_SYN);
		//at_clock_i2c_syn_init(in_sel, GPIO_SYN_B);//pull down  input
		at_clock_syn_init(out_sel, GPIO_SYN);//pull down   output
		at_clock_syn_init(in_sel, GPIO_SYN_B);//pull down  input
		SYN = TRUE;//clock_syn_b_sema
        
        wl_init_sema(&i2c_sla_rx_done_sema, 0, sema_binary);
        wl_init_sema(&i2c_sla_tx_done_sema, 0, sema_binary);
		wl_init_sema(&clock_syn_b_sema, 0, sema_binary);
		
		at_gpio_write(GPIO_SYN, GPIO_PIN_SET);
		at_gpio_write(GPIO_SYN_B, GPIO_PIN_SET);
        
        while(1)
        {
			
#if 0//sla rx int tx int
			
            memset(i2c_sla_rxbuffer, 0 , I2C_TEST_LEN);
            s_sta = hal_i2c_slavor_recv_interrupt(i2c_s, i2c_sla_rxbuffer, I2C_TEST_LEN);
            if(s_sta != HAL_OK) {
                HAL_TEST_DBG("sla rx error\n");
                break;
            }
			
			at_gpio_write(GPIO_SYN, GPIO_PIN_SET);//pull up
			wl_os_mdelay(2);//be sure pull down later
			at_gpio_write(GPIO_SYN, GPIO_PIN_RESET);//pull down
			
			
			ret = wl_wait_sema(&i2c_sla_rx_done_sema, ONEMINUTE);//1s
            if(!ret){
                HAL_TEST_DBG("sla rx timeout\n");
                break;
            }
            
            if(memcmp(i2c_sla_rxbuffer, i2c_mst_txbuffer, I2C_TEST_LEN)){
                HAL_TEST_DBG("sla rx datas error\n");
                break;
            }
			
			//at_gpio_write(GPIO_SYN, GPIO_PIN_SET);//pull up
			
			//wait mst say ok
			at_gpio_it_start(GPIO_SYN_B);
			ret = wl_wait_sema(&clock_syn_b_sema, ONEMINUTE);//wait mst say ok
			if(!ret){
				HAL_TEST_DBG("sla wait mst sema time out\n");
				break;
			}
			
			s_sta = hal_i2c_slavor_xfer_interrupt(i2c_s, i2c_sla_txbuffer, I2C_TEST_LEN);
            if(s_sta != HAL_OK) {
                HAL_TEST_DBG("sla tx error\n");
                break;
            }
			
			at_gpio_write(GPIO_SYN, GPIO_PIN_SET);//pull up
			wl_os_mdelay(2);//be sure pull down later
			at_gpio_write(GPIO_SYN, GPIO_PIN_RESET);//pull down
			
			ret = wl_wait_sema(&i2c_sla_tx_done_sema, ONEMINUTE);
            if(!ret){
                HAL_TEST_DBG("sla tx timeout\n");
                break;
            }
			
			//at_gpio_write(GPIO_SYN, GPIO_PIN_SET);//pull up
			
			//wait mst say ok
			at_gpio_it_start(GPIO_SYN_B);
			ret = wl_wait_sema(&clock_syn_b_sema, ONEMINUTE);//wait mst
			if(!ret){
				HAL_TEST_DBG("sla wait mst sema time out\n");
				break;
			}
			
#endif
			
#if 0//sla rx poll tx int
			
			memset(i2c_sla_rxbuffer, 0 , I2C_TEST_LEN);
			s_sta = hal_i2c_slavor_recv(i2c_s, i2c_sla_rxbuffer, I2C_TEST_LEN, ONEMINUTE);
			if(s_sta != HAL_OK) {
                HAL_TEST_DBG("sla rx error\n");
                break;
            }
			
			if(memcmp(i2c_sla_rxbuffer, i2c_mst_txbuffer, I2C_TEST_LEN)){
                HAL_TEST_DBG("sla rx datas error\n");
                break;
            }
			
			//wait mst say ok
					
#endif
			
#if 1// no syn gpio 
			memset(i2c_sla_rxbuffer, 0 , I2C_TEST_LEN);
            s_sta = s907x_hal_i2c_slavor_recv_interrupt(i2c_s, i2c_sla_rxbuffer, I2C_TEST_LEN);
            if(s_sta != HAL_OK) {
                HAL_TEST_DBG("sla rx error\n");
                break;
            }
			
			at_gpio_write(GPIO_SYN, GPIO_PIN_SET);//pull up  GPIO_SYN 1   // when sla set GPIO_SYN 1,  let mst exit pending
																							
			ret = wl_wait_sema(&i2c_sla_rx_done_sema, ONEMINUTE);//1s
            if(!ret){
                HAL_TEST_DBG("sla rx timeout\n");
                break;
            }
            
            if(memcmp(i2c_sla_rxbuffer, i2c_mst_txbuffer, I2C_TEST_LEN)){
                HAL_TEST_DBG("sla rx datas error\n");
                break;
            }
			
			at_gpio_write(GPIO_SYN, GPIO_PIN_RESET);//pull down 
			
			start = wl_get_systemtick();
			while(s907x_hal_gpio_read(BIT(GPIO_SYN_B))){//wait mst say done
				pass = wl_get_systemtick() - start;
				to = wl_systemtick_to_ms(pass);
				if(to >= TIME_OUT_SYN){
					HAL_TEST_DBG("sla  error 1\n");
					goto exit;
				}
			}
			//at_gpio_write(GPIO_SYN, GPIO_PIN_SET);//pull up  GPIO_SYN 1
			
			s_sta = s907x_hal_i2c_slavor_xfer_interrupt(i2c_s, i2c_sla_txbuffer, I2C_TEST_LEN);
            if(s_sta != HAL_OK) {
                HAL_TEST_DBG("sla tx error\n");
                break;
            }
			
			at_gpio_write(GPIO_SYN, GPIO_PIN_SET);//pull up  GPIO_SYN 1   // when sla set GPIO_SYN 1,  let mst exit pending
			
			ret = wl_wait_sema(&i2c_sla_tx_done_sema, ONEMINUTE);
            if(!ret){
                HAL_TEST_DBG("sla tx timeout\n");
                break;
            }
			
			at_gpio_write(GPIO_SYN, GPIO_PIN_RESET);//pull down //let mst get sla done
			
			start = wl_get_systemtick();
			while(s907x_hal_gpio_read(BIT(GPIO_SYN_B))){//wait to get mst  done
				pass = wl_get_systemtick() - start;
				to = wl_systemtick_to_ms(pass);
				if(to >= TIME_OUT_SYN){
					HAL_TEST_DBG("sla  error 2\n");
					goto exit;
				}
			}
			
			
			
			
#endif
			
	
			ret = wl_wait_sema(&i2c_thread_destory_sema, 1);
			if(ret)
			{
				break;
			}
			
			newstarttest = TRUE;
			totalDatas += I2C_TEST_LEN * 2;
            
        }
        
          
exit: 
	
		wl_free_sema(&i2c_thread_destory_sema);
        wl_free_sema(&i2c_sla_rx_done_sema);
        wl_free_sema(&i2c_sla_tx_done_sema);
		wl_free_sema(&clock_syn_b_sema);
		at_test_led_deint(1);
		s907x_hal_timer_base_deinit(&timx_hdl);
		newstarttest = FALSE;
		totalDatas = 0;
		runtime_cnt = 0;
		HAL_TEST_DBG("exit i2c task\n");
        wl_destory_threadself();
    	
          
}

static void i2c_mst_task(void *context)
{
        hal_test_t *test = context;
        hal_status_e m_sta;
        u32 ret;
		u32 to;
		u32 start;
		u32 pass;
        
        i2c_hdl_t *i2c_m = &i2c_master_type;//mst
        
        i2c_performace_test_enter(test);//i2c init
        i2c_txrx_test_buff_init();//buf init
		
		//at_clock_syn_init(in_sel, GPIO_SYN);//pull down  input
		//at_clock_syn_init(out_sel, GPIO_SYN_B);//pull down output
		at_clock_i2c_syn_init(in_sel, GPIO_SYN);
		at_clock_i2c_syn_init(out_sel, GPIO_SYN_B);//pull down output
        
        wl_init_sema(&i2c_mst_rx_done_sema, 0, sema_binary);
        wl_init_sema(&i2c_mst_tx_done_sema, 0, sema_binary);
		wl_init_sema(&clock_syn_sema, 0, sema_binary);
		
		master_tim_sel = TRUE;
		
		at_gpio_write(GPIO_SYN, GPIO_PIN_SET);
		at_gpio_write(GPIO_SYN_B, GPIO_PIN_SET);
		
        while(1)
		{
#if 0//mst tx int  rx int 
			
			m_sta = hal_i2c_master_xfer_interrupt(i2c_m, i2c_mst_txbuffer, I2C_TEST_LEN);
			if(m_sta != HAL_OK) {
				HAL_TEST_DBG("mst tx error\n");
				break;
			}
			
			ret = wl_wait_sema(&i2c_mst_tx_done_sema, ONEMINUTE);
			if(!ret){
				HAL_TEST_DBG("mst tx timeout\n");
				break;
			}
			
			at_gpio_write(GPIO_SYN_B, GPIO_PIN_SET);//pull up
			wl_os_mdelay(2);//1ms
			//wl_os_mdelay(20);//the time of sending the signal must be  later than sla into waiting,	give 20ms ,sla do more works
			at_gpio_write(GPIO_SYN_B, GPIO_PIN_RESET);//pull down 
			
			at_gpio_it_start(GPIO_SYN);//open int
			ret = wl_wait_sema(&clock_syn_sema, ONEMINUTE);//wait sla say ready
			if(!ret){
				HAL_TEST_DBG("mst wait sla sema time out\n");
				break;
			}
			
			memset(i2c_mst_rxbuffer, 0, I2C_TEST_LEN);
			m_sta = hal_i2c_master_recv_interrupt(i2c_m, i2c_mst_rxbuffer, I2C_TEST_LEN);
			if(m_sta != HAL_OK){
				HAL_TEST_DBG("mst rx error\n");
				break;
			}
			
			ret = wl_wait_sema(&i2c_mst_rx_done_sema, ONEMINUTE);
			if(!ret){
				HAL_TEST_DBG("mst rx timeout\n");
				break;
			}
			
			if(memcmp(i2c_mst_rxbuffer, i2c_sla_txbuffer, I2C_TEST_LEN)){
				HAL_TEST_DBG("mst rx datas error\n");
                break;
			}
		
			at_gpio_write(GPIO_SYN_B, GPIO_PIN_SET);//pull up
			wl_os_mdelay(2);
			//wl_os_mdelay(20);//5ms
			at_gpio_write(GPIO_SYN_B, GPIO_PIN_RESET);//pull down 
			
			at_gpio_it_start(GPIO_SYN);//open int
			ret = wl_wait_sema(&clock_syn_sema, ONEMINUTE);//wait sla say ready
			if(!ret){
				HAL_TEST_DBG("mst wait sla sema time out\n");
				break;
			}
			
#endif
			
#if 0//mst tx poll rx int
			
			m_sta = hal_i2c_master_xfer(i2c_m, , i2c_mst_txbuffer, ONEMINUTE);
			if(m_sta != HAL_OK){
				HAL_TEST_DBG("sla rx error\n");
                break;
			}
					
#endif
			
#if 1//no syn
			m_sta = s907x_hal_i2c_master_xfer_interrupt(i2c_m, i2c_mst_txbuffer, I2C_TEST_LEN);
			if(m_sta != HAL_OK) {
				HAL_TEST_DBG("mst tx error\n");
				break;
			}
			
			ret = wl_wait_sema(&i2c_mst_tx_done_sema, ONEMINUTE);
			if(!ret){
				HAL_TEST_DBG("mst tx timeout\n");
				break;
			}
			
			start = wl_get_systemtick();
			while(s907x_hal_gpio_read(BIT(GPIO_SYN))){//while GPIO_SYN 0 ,mean mst get sla say done,and sla then waiting mst say done
				pass = wl_get_systemtick() - start;
				to = wl_systemtick_to_ms(pass);
				if(to >= TIME_OUT_SYN){
					HAL_TEST_DBG("mst  error 1\n");
					goto exit;
				}
			}
			
			at_gpio_write(GPIO_SYN_B, GPIO_PIN_RESET);//pull down , let sla know mst done
			
			start = wl_get_systemtick();
			while(!s907x_hal_gpio_read(BIT(GPIO_SYN))){//waiting    GPIO_SYN 0 ----- mst pending 
				pass = wl_get_systemtick() - start;
				to = wl_systemtick_to_ms(pass);
				if(to >= TIME_OUT_SYN){
					HAL_TEST_DBG("mst  error 2\n");
					goto exit;
				}
			}        
			at_gpio_write(GPIO_SYN_B, GPIO_PIN_SET);//pull up  reset
			
			wl_os_mdelay(2);//let sla be in ready stably
			
			memset(i2c_mst_rxbuffer, 0, I2C_TEST_LEN);
			m_sta = s907x_hal_i2c_master_recv_interrupt(i2c_m, i2c_mst_rxbuffer, I2C_TEST_LEN);
			if(m_sta != HAL_OK){
				HAL_TEST_DBG("mst rx error\n");
				break;
			}
			
			ret = wl_wait_sema(&i2c_mst_rx_done_sema, ONEMINUTE);
			if(!ret){
				HAL_TEST_DBG("mst rx timeout\n");
				break;
			}
			
			if(memcmp(i2c_mst_rxbuffer, i2c_sla_txbuffer, I2C_TEST_LEN)){
				HAL_TEST_DBG("mst rx datas error\n");
                break;
			}
			
			start = wl_get_systemtick();
			while(s907x_hal_gpio_read(BIT(GPIO_SYN))){//while GPIO_SYN 0 ,mean mst get sla say done,and sla then waiting mst say done
				pass = wl_get_systemtick() - start;
				to = wl_systemtick_to_ms(pass);
				if(to >= TIME_OUT_SYN){
					HAL_TEST_DBG("mst  error 3\n");
					goto exit;
				}
			}
			
			at_gpio_write(GPIO_SYN_B, GPIO_PIN_RESET);//pull down , let sla know mst done
			
			start = wl_get_systemtick();
			while(!s907x_hal_gpio_read(BIT(GPIO_SYN))){//waiting    GPIO_SYN 0 ----- mst pending
				pass = wl_get_systemtick() - start;
				to = wl_systemtick_to_ms(pass);
				if(to >= TIME_OUT_SYN){
					HAL_TEST_DBG("mst  error 4\n");
					goto exit;
				}
			}
			at_gpio_write(GPIO_SYN_B, GPIO_PIN_SET);//pull up  reset
			
			
#endif


			ret = wl_wait_sema(&i2c_thread_destory_sema, 1);
			if(ret)
			{
				break;
			}
			
			newstarttest = TRUE;
			totalDatas += I2C_TEST_LEN * 2;
			
            
        }
        
          
exit: 
	
		wl_free_sema(&i2c_thread_destory_sema);
        wl_free_sema(&i2c_mst_rx_done_sema);
        wl_free_sema(&i2c_mst_tx_done_sema);
		wl_free_sema(&clock_syn_sema);
		at_test_led_deint(0);
		s907x_hal_timer_base_deinit(&timx_hdl);
		newstarttest = FALSE;
		totalDatas = 0;
		runtime_cnt = 0;
		HAL_TEST_DBG("exit i2c task\n");
        wl_destory_threadself();
		
    
          
}

static void i2c_performance_test(hal_test_t *test)
{
    
		u8 mst_sel = test->arg[0];
		u8 i2c_thread_key = test->arg[7];
		  
		if(mst_sel == 2)//sla
		{
				  //i2c sla thread
				  if(!i2c_thread_key)// stop  AT+I2C=4,2,0,0,400,55,66,0,0
				  {
					//deleted thread
					wl_send_sema(&i2c_thread_destory_sema);
					wl_os_mdelay(1000);
				  }
				  else// start AT+I2C=4,2,0,0,400,55,66,0,1
				  {
					//i2c_thread_destory_sema init
					wl_init_sema(&i2c_thread_destory_sema, 0, sema_binary);
					
					//led init
					at_test_led_init(1);
					
					//timer init  for led & print test info
					
					at_test_timer_init(test);
					
					//creat thread
					wl_create_thread("uart_sla_thread", 512, MAIN_UART_RX_PRIO, (thread_func_t)i2c_sla_task, test);	

					HAL_TEST_DBG("sla ready, waiting mst ...\n");
				  }
        
        }
        else if(mst_sel == 1)
        {
         		//i2c mst thread
				if(!i2c_thread_key)// stop AT+I2C=4,1,0,0,400,66,55,0,0
				{
						wl_send_sema(&i2c_thread_destory_sema);
						wl_os_mdelay(1000);
				}
				else// start AT+I2C=4,1,0,0,400,66,55,0,1
				{
						wl_init_sema(&i2c_thread_destory_sema, 0, sema_binary);
						at_test_led_init(0);
						at_test_timer_init(test);
						wl_create_thread("uart_mst_thread", 512, MAIN_UART_RX_PRIO, (thread_func_t)i2c_mst_task, test);	

						HAL_TEST_DBG("i2c mst running...\n");
						
				}
			
        }
              
}


void i2c_test(hal_test_t *test)
{	
	ASSERT(test);
	ASSERT(test->no < (sizeof(i2c_test_map)/sizeof(hal_test_name_map_t)));

	HAL_TEST_DBG("test no %d = %s\n", test->no, i2c_test_map[test->no].name);
	
	if(test->no == I2C_PERFORMACE_TEST)
	{
		i2c_performance_test(test);
		wl_os_mdelay(500);
		return;
	}
	
	i2c_test_enter(test);

	switch(test->no) 
	{
		case I2C_TEST_POLL:
			i2c_test_poll(test);
			break;
		case I2C_TEST_INTERRUPT:
			i2c_test_interrupt(test);
			break;
		case I2C_TEST_DMA:
			i2c_test_dma(test);
			break;
                case I2C_TEST_LOOPBACK:
                        i2c_test_loopback(test);
                        break;
		default:
			
			break;
	}
	
    i2c_test_exit(test);

	HAL_TEST_DBG("i2c test item:%d finish\n",test->no);

}

#endif

