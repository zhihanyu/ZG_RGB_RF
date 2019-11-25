#include "s907x.h"
#include "uart_test.h"
#include "gpio_test.h"
#include "hal_timer.h"
#include "timer_test.h"


#if M_AT_TEST

#define DATA_SIZE  		256
#define POLL_OFFSET	    0x00
#define IT_OFFSET		0x10
#define DMA_OFFSET 		0x20
#define TRANSMIT		0x10
#define		ONEMINUTE		60000
static uart_hdl_t uart_hdl;
//static timer_hdl_t timx_hdl;
static u8 txbuffer[DATA_SIZE];
static u8 rxbuffer[DATA_SIZE];
static u8 uart_tx_done = 0;
static u8 uart_rx_done = 0;
static u8 uart_trx_error = 0;

static sema_t uart_thread_destory_sema;
static sema_t sla_rx_sema;
static sema_t sla_tx_sema;

static sema_t mst_rx_sema;
static sema_t mst_tx_sema;

static u8 sla_txbuffer[DATA_SIZE];
static u8 sla_rxbuffer[DATA_SIZE];
static u8 mst_txbuffer[DATA_SIZE];
static u8 mst_rxbuffer[DATA_SIZE];

//extern
extern u8  newstarttest;
extern u16      time_cnt;
extern u32 	 runtime_cnt;
extern u8 master_tim_sel;
extern u32 totalDatas;
extern	timer_hdl_t timx_hdl;
    
void s907x_hal_uart_msp_init(uart_hdl_t *uart) 
{
	//set pinmux func
    //UART0_RX_SEL0(HAL_ON);
    //UART0_TX_SEL0(HAL_ON);

    UART0_RX_SEL1(HAL_ON);
    UART0_TX_SEL1(HAL_ON);
}

void s907x_hal_uart_msp_deinit(uart_hdl_t *uart) 
{
	//clear pinmux func
    //UART0_RX_SEL0(HAL_OFF);
    //UART0_TX_SEL0(HAL_OFF);

    UART0_RX_SEL1(HAL_OFF);
    UART0_TX_SEL1(HAL_OFF);
}



static void txdone_cb(void *context)
{
	uart_hdl_t *uart = (uart_hdl_t *)context;

	uart_tx_done = 1;
	HAL_TEST_DBG("uart tx it remain len = %d\n", uart->it.txlen);
}

static void rxdone_cb(void *context)
{
	uart_hdl_t *uart = (uart_hdl_t *)context;

	uart_rx_done = 1;
	//HAL_TEST_DBG("uart it rx len = %d\n", uart->it.rxlen);
 	printf("r1 ");
 	HAL_DBG_ARRARY(rxbuffer, uart->it.rxlen, ARY_U8, 8);
	uart->it.rxbuf = rxbuffer;
    uart->it.rxlen = 0;
}

static void trx_error_cb(void *context)
{
	
	uart_trx_error = uart_tx_done = uart_rx_done = 1;
	HAL_TEST_DBG("uart tx/rx error\n");
}

static u8 fifo_buffer[DATA_SIZE*2];
static void rxtimeout_cb(void *context)
{
	uart_hdl_t *uart = (uart_hdl_t *)context;
    //call user code
	//HAL_TEST_DBG("rx timeout one frame:%d\n",uart->it.rxlen);
	printf("r2 ");
    HAL_DBG_ARRARY(rxbuffer, uart->it.rxlen, ARY_U8, 8);
	memset((void*)rxbuffer, 0, sizeof(rxbuffer));
	uart->it.rxbuf = rxbuffer;
    //clear & recv again
    uart->it.rxlen = 0;
}



static void txdone_dma_cb(void *context)
{
	uart_hdl_t *uart = (uart_hdl_t *)context;

	uart_tx_done = 1;
	HAL_TEST_DBG("uart tx dma remain len = %d\n", uart->dma.txlen);
	
}

static void rxdone_dma_cb(void *context)
{
	uart_hdl_t *uart = (uart_hdl_t *)context;

	uart_rx_done = 1;
	HAL_TEST_DBG("uart rx dma remain len = %d\n", uart->dma.rxlen);
}

static void perf_txdone_cb(void *context)
{
	uart_hdl_t *uart = (uart_hdl_t *)context;
        
        if(master_tim_sel)
        {
          wl_send_sema_fromisr(&mst_tx_sema);
          //HAL_TEST_DBG("mst_tx_sema post done \n");
        }
        else    
        {
          wl_send_sema_fromisr(&sla_tx_sema);
          //HAL_TEST_DBG("sla_tx_sema post done \n");
        }

}

static void perf_rxdone_cb(void *context)
{
	uart_hdl_t *uart = (uart_hdl_t *)context;
        
        if(master_tim_sel)
        {
          wl_send_sema_fromisr(&mst_rx_sema);
          //HAL_TEST_DBG("mst_rx_sema post done \n");
        }
        else
        {
          wl_send_sema_fromisr(&sla_rx_sema);
          //HAL_TEST_DBG("sla_rx_sema post done \n");
        }    
}

unsigned char WeekDay20(unsigned char y, unsigned char m, unsigned char d) 
{ 
  unsigned char A;
  
  if (m==1||m==2)
  {
	  m+=12;
	  y--; 
  }
  A= (d+2*m+3* (m+1) /5+y+y/4-y/100+y/400+1) %7;
  return A;
}

void uart_config(uart_hdl_t *uart, hal_test_t *test)
{
	u32 datalen   = test->arg[0];
	u32 baud	  = test->arg[1];
	u8	parity	  = test->arg[2];
	u8	stopbits  = test->arg[3];
	u8  lp_mode   = test->arg[4];

	// uart init
	memset((void*)uart, 0, sizeof(uart_hdl_t));
	// uart config
	uart->config.idx = UART_0;
	uart->config.baud = baud;
	uart->config.parity = parity;
	uart->config.stopbits = stopbits;
	//data len
	if(datalen == UART_DATALENGTH_7B) {
		uart->config.datalen = UART_DATALENGTH_7B;
		HAL_TEST_DBG("uart test use 7bit mode\n");
	} else {
		uart->config.datalen = UART_DATALENGTH_8B;
		HAL_TEST_DBG("uart test use 8bit mode\n");	
	}
	//lp mode
	uart->config.lpmode = lp_mode;
	
	//fifo threashold
	uart->config.rx_thd = 0;
	uart->config.tx_thd = 0;
}

 
//test uart tx/rx poll mode 
//tx<-->rx loopback test
void uart_polling_mode_test(hal_test_t *test)
{
	ASSERT(test->arg_cnt == 7);
	uart_hdl_t *uart = &uart_hdl;
	int i,j;
	int tx_cnt, rx_cnt;
	u32 loop	  = test->arg[5];
	u8	nums	  = test->arg[6];
    int m,n;
    u8 *tx_head, *rx_head;

	ASSERT((nums != 0) && (loop != 0));
	
	uart_config(uart, test);
		
	s907x_hal_uart_init(uart);
	
	for ( i = 0; i < nums; i++) {
		txbuffer[i] = i + POLL_OFFSET;
	}
	test->result = TRUE;
    m = nums / UART_FIFO_SIZE;
    n = nums % UART_FIFO_SIZE;
    tx_head = txbuffer;
    rx_head = rxbuffer;

	for (i = 0; i < loop; i++) {

        //fifosize = UART_FIFO_SIZE
        for(j = 0; j < m; j++) {
            //reset buffer
            tx_head = txbuffer;
            rx_head = rxbuffer;
            tx_cnt = s907x_hal_uart_tx(uart, tx_head, UART_FIFO_SIZE, UART_POLL_MAX_WAIT);
            if(tx_cnt != UART_FIFO_SIZE) {
                HAL_TEST_DBG("uart polling test tx err cnt = %d, id = %d\n", tx_cnt, i);
                test->result = FALSE;
                return;
            }
            tx_head += tx_cnt;
            rx_cnt = s907x_hal_uart_rx(uart, rx_head, UART_FIFO_SIZE, UART_POLL_MAX_WAIT);
            if(rx_cnt != UART_FIFO_SIZE) {
                HAL_TEST_DBG("uart polling test rx err cnt = %d id = %d\n", rx_cnt, i);
                test->result = FALSE;
                return;
            }
            rx_head += rx_cnt;
            if((rx_cnt != tx_cnt) || memcmp(rxbuffer, txbuffer, rx_cnt)) {
               HAL_TEST_DBG("uart polling test err id = %d\n", i);
               test->result = FALSE;
               return ;
            }	
        }
        //n < UART_FIFO_SIZE
        if( n > 0 ) {
            tx_head = txbuffer;
            rx_head = rxbuffer;
            tx_cnt = s907x_hal_uart_tx(uart, tx_head, n, UART_POLL_MAX_WAIT);
            if(tx_cnt != n) {
                HAL_TEST_DBG("uart polling test tx err cnt = %d, id = %d\n", tx_cnt, i);
                test->result = FALSE;
                return;
            }
            tx_head += tx_cnt;
            rx_cnt = s907x_hal_uart_rx(uart, rx_head, n, UART_POLL_MAX_WAIT);
            if(rx_cnt != n) {
                HAL_TEST_DBG("uart polling test rx err cnt = %d id = %d\n", rx_cnt, i);
                test->result = FALSE;
                return;
            }
            rx_head += rx_cnt;
            if((rx_cnt != tx_cnt) || memcmp(rxbuffer, txbuffer, rx_cnt)) {
               HAL_TEST_DBG("uart polling test err id = %d\n", i);
               test->result = FALSE;
               return ;
            }
        }
        HAL_TEST_DBG("uart polling test %d success\n", i);  
	}
	HAL_TEST_DBG("uart polling test down\n");
}


//test uart tx/rx interrupt mode 
void uart_int_mode_test(hal_test_t *test)
{	
	ASSERT(test->arg_cnt >= 5);
	uart_hdl_t *uart = &uart_hdl;
	int i;
	hal_status_e status;
	u32 loop	  = test->arg[5];
	u8	nums	  = test->arg[6];

	ASSERT((nums != 0) && (loop != 0));
	
	uart_config(uart, test);
		
	s907x_hal_uart_init(uart);

	uart->it.tx_complete.func= txdone_cb;
	uart->it.tx_complete.context = uart;
	uart->it.rx_complete.func= rxdone_cb;
	uart->it.rx_complete.context = uart;
	uart->it.trx_error.func = trx_error_cb;
	uart->it.trx_error.context = uart;

	for ( i = 0; i < nums; i++) {
		txbuffer[i] = i + IT_OFFSET;
	}

	test->result = TRUE;
	
	for(i = 0; i < loop; i++) {
		
		uart_tx_done = uart_rx_done = uart_trx_error = 0;
		
		status = s907x_hal_uart_rx_it(uart, (u8 *)rxbuffer, nums);
		if (status != HAL_OK) {
			HAL_TEST_DBG("UART%d rx it error, status=%d\n",uart->config.idx, status);
			test->result = FALSE;
			break;
		}
		status = s907x_hal_uart_tx_it(uart, (u8 *)txbuffer, nums);
		if (status != HAL_OK) {
			HAL_TEST_DBG("UART%d tx it error, status=%d\n",uart->config.idx, status);
			test->result = FALSE;
			break;
		}
		//wait tx rx finish 
		while((uart_tx_done == 0) || (uart_rx_done == 0));
		//check error happed
		if(uart_trx_error) {
			HAL_TEST_DBG("UART%d trx it error, status=%d\n",uart->config.idx, status);
			test->result = FALSE;
			break;
		}
		if(memcmp(rxbuffer, txbuffer, nums)) {
           HAL_TEST_DBG("uart it test err id = %d\n", i);
		   test->result = FALSE;
		   break;
		}		
        HAL_TEST_DBG("uart int test %d success\n", i); 
	}
	
	HAL_TEST_DBG("uart it test down\n");
}



/*test uart rx timeout mode 
  s9070 support uart rx hw timeout
  note: suggest one frame size smaller than rx fifo size(16)
  */
void uart_int_timeout_test(hal_test_t *test)
{

	ASSERT(test->arg_cnt == 5);
	uart_hdl_t *uart = &uart_hdl;

	uart_config(uart, test);
		
	s907x_hal_uart_init(uart);

	uart->it.rx_timeout.func= rxtimeout_cb;
	uart->it.rx_timeout.context = uart;
	
	uart->it.rx_complete.func= rxdone_cb;
	uart->it.rx_complete.context = uart;
	
	//s907x_hal_uart_rx_it(uart, (u8 *)rxbuffer, DATA_SIZE);
	
	s907x_hal_uart_rx_it_to(uart, rxbuffer);
}




void dmarx_timeout_isr(void *context)
{
	u32 addr;

	timer_hdl_t* tim = (timer_hdl_t*) context;

	uart_hdl_t *uart;

	ASSERT(tim);
	
	uart = (uart_hdl_t*)tim->it.object;

	addr = s907x_hal_uart_rx_dma_adddress(uart);
	
	if ((uart->dma.last_rx_addr != uart->dma.rxbuf)) {  //address change
		 uart->dma.last_rx_addr = uart->dma.rxbuf;
	} else  {
		if(++uart->dma.rx_timeout_cnt >= UART_DMA_RX_TO/UART_DMA_RX_TICK) {
			uart->dma.rx_timeout_cnt = 0;
                        
			//stop rx dma
			s907x_hal_uart_dma_rxstop(uart);
			//stop timer
			s907x_hal_timer_stop(tim);
			//calc remain size
			uart->dma.rxlen =  (addr - (u32)uart->dma.rxbuf);
            
            HAL_TEST_DBG("uart dma recv timeout remain len = %d\n", uart->dma.rxlen);

			if(uart->dma.rx_complete.func) {
				uart->dma.rx_complete.func(uart->dma.rx_complete.context);
			}	

		}
	}
}



static void uart_dma_rxto_init(uart_hdl_t *uart)
{
	static timer_hdl_t *tim = &timx_hdl;
	
	memset(&timx_hdl, 0, sizeof(timx_hdl));

    //set timer1 UART_DMA_RX_TICK / ms
	tim->config.idx = TIM1;
	tim->config.prescaler = 0x0;
	tim->config.period = 32000;
    tim->config.int_enable = TRUE;

	//set user callback
	tim->it.basic_user_cb.func = dmarx_timeout_isr;
	tim->it.basic_user_cb.context = tim;
	tim->it.object = uart;
	

	s907x_hal_timer_base_init(tim);
	s907x_hal_timer_start_base(tim);
}




void uart_dma_mode_test(hal_test_t *test)
{
	ASSERT(test->arg_cnt == 7);
	uart_hdl_t *uart = &uart_hdl;
	int i;
	hal_status_e status;
	u32 loop	  = test->arg[5];
	u8	nums	  = test->arg[6];

	ASSERT((nums != 0) && (loop != 0));
	
	uart_config(uart, test);
		
	s907x_hal_uart_init(uart);

	for ( i = 0; i < nums; i++) {
		txbuffer[i] = i + DMA_OFFSET;
	}
    uart->dma.rx_burst_size = 4;
    uart->dma.tx_burst_size = 4;


	uart->dma.rx_complete.func= rxdone_dma_cb;
	uart->dma.rx_complete.context = uart;

	uart->dma.tx_complete.func= txdone_dma_cb;
	uart->dma.tx_complete.context = uart;

    test->result = TRUE;

	for (i = 0; i < loop; i++) {
		
		uart_tx_done = uart_rx_done = uart_trx_error = 0;
#if 1		
		memset((void*)rxbuffer, 0, sizeof(rxbuffer));
		//dma timerout use gtimer check!
		uart_dma_rxto_init(uart);
		//hal_uart_rx_dma will set uart->dma.last_rx_addr = uart->dma.rxbuf = rxbuffer;
		status = s907x_hal_uart_rx_dma(uart, (u8 *)rxbuffer, nums);
		if (status != HAL_OK) {
			HAL_TEST_DBG("UART%d rx dma error, status=%d\n",uart->config.idx, status);
			break;
		}
#endif	
		status = s907x_hal_uart_tx_dma(uart, (u8 *)txbuffer, nums);
		//status = hal_uart_tx_it(uart, (u8 *)txbuffer, nums);
        if (status != HAL_OK) {
			HAL_TEST_DBG("UART%d tx dma error, status=%d\n",uart->config.idx, status);
			break;
		}

		//wait tx rx finish 
		while((uart_tx_done == 0) || (uart_rx_done == 0));
		
		//check error happed
		if(uart_trx_error) {
			HAL_TEST_DBG("UART%d trx dma error, status=%d\n",uart->config.idx, status);
			test->result = FALSE;
			break;
		}
		if(memcmp(rxbuffer, txbuffer, nums)) {
           HAL_TEST_DBG("uart dma test err id = %d\n", i);
		   test->result = FALSE;
		   break;
		}
        
        HAL_TEST_DBG("uart polling test %d success\n", i); 
	}
    HAL_TEST_DBG("uart dma test down\n");
}


void uart_dma_mode_txstop_test(hal_test_t *test)
{
    ASSERT(test->arg_cnt == 7);
    uart_hdl_t *uart = &uart_hdl;
    int i;
    int cnt = 0;
    hal_status_e status;
    
    u32 loop_dmatxstop = test->arg[5];
    int	nums = test->arg[6];

    ASSERT((nums != 0) && (loop_dmatxstop != 0));

    uart_config(uart, test);
          
    s907x_hal_uart_init(uart);
    
    uart->dma.rx_burst_size = 4;
    uart->dma.tx_burst_size = 4;


    uart->dma.rx_complete.func= rxdone_dma_cb;
    uart->dma.rx_complete.context = uart;

    uart->dma.tx_complete.func= txdone_dma_cb;
    uart->dma.tx_complete.context = uart;
    
    for(;;)
    {
      uart_tx_done = uart_rx_done = uart_trx_error = 0;
      for ( i = 0; i < nums; i++) 
      {
          txbuffer[i] = (u8)wl_get_random32();
      }
      
      memset((void*)rxbuffer, 0, sizeof(rxbuffer));
 
      status = s907x_hal_uart_rx_dma(uart, (u8 *)rxbuffer, nums);
      if (status != HAL_OK) {
              HAL_TEST_DBG("UART%d rx dma error, status=%d\n",uart->config.idx, status);
              break;
      }
      
      status = s907x_hal_uart_tx_dma(uart, (u8 *)txbuffer, nums);
   
      if (status != HAL_OK) {
        HAL_TEST_DBG("UART%d tx dma error, status=%d\n",uart->config.idx, status);
        break;
      }
      
      while((uart_tx_done == 0) || (uart_rx_done == 0));
		
      if(uart_trx_error) {
        HAL_TEST_DBG("UART%d trx dma error, status=%d\n",uart->config.idx, status);
        test->result = FALSE;
        break;
      }
        if(memcmp(rxbuffer, txbuffer, nums)) {
        HAL_TEST_DBG("uart dma test err id = %d\n", cnt);
        test->result = FALSE;
        break;
      }
        
      HAL_TEST_DBG("uart dma test %d success\n", ++cnt); 

      HAL_DBG_ARRARY(rxbuffer, nums, ARY_U8, 8);

      if(cnt >= loop_dmatxstop )
      {
        status = s907x_hal_uart_dma_txstop(uart);
        if(status == HAL_OK) 
        {
          HAL_TEST_DBG("uart dma txstop!\n");
          break;
        }
      }

      wl_os_mdelay(1000);
         
    }
}

void uart_txrx_test_buff_init(void)
{
        int i;
        
        //mst
        //tx 00 - 0f
        //rx 0f - 00
        
        //sla
        //tx 0f - 00
        //rx 00 - 0f
        

          for(i = 0; i < TRANSMIT; i++)
          {
            sla_txbuffer[i] = (0x0f - i);
            sla_rxbuffer[i] = i;
          }
  
          
          for(i = 0; i < TRANSMIT; i++)
          {
            mst_txbuffer[i] = i;
            mst_rxbuffer[i] = (0x0f - i);
          }
        
}


void uart_sla_task(void *context)
{
	hal_test_t *test = context;
	ASSERT(test->arg_cnt >= 5);
	u32 ret = 0;
	u8 mode = test->arg[5];
	uart_txrx_test_buff_init();//data init

	wl_init_sema(&sla_rx_sema,0,sema_binary);//rx sem init
	wl_init_sema(&sla_tx_sema,0,sema_binary);//tx sem init


	uart_hdl_t *uart = &uart_hdl;
	hal_status_e status;

	uart_config(uart, test);
	s907x_hal_uart_init(uart);

	uart->it.tx_complete.func= perf_txdone_cb;
	uart->it.tx_complete.context = uart;
	uart->it.rx_complete.func= perf_rxdone_cb;
	uart->it.rx_complete.context = uart;
	uart->it.trx_error.func = trx_error_cb;
	uart->it.trx_error.context = uart;
	//uart0 init done

	test->result = TRUE;

	while(TRUE)//sla
	{
	
		memset(sla_rxbuffer, 0, DATA_SIZE);
		
		status = s907x_hal_uart_rx_it(uart,(u8 *)sla_rxbuffer, TRANSMIT);
		if (status != HAL_OK) {
			HAL_TEST_DBG("sla rx error\n");
			break;
		}

		//wait rx cpl sem
		ret = wl_wait_sema(&sla_rx_sema, ONEMINUTE);//portMAX_DELAY
		if(!ret)
		{	
			HAL_TEST_DBG("sla rx timeout\n");
			break;
		}
		//check result
		if(memcmp(sla_rxbuffer, mst_txbuffer, TRANSMIT))
		{
			HAL_TEST_DBG("sla rec datas error\n");
			break;
		}
		
		ret = wl_wait_sema(&uart_thread_destory_sema,0);//10ms
		if(ret){
			break;
		}
		
		//sla tx begin
		status = s907x_hal_uart_tx_it(uart, (u8 *)sla_txbuffer, TRANSMIT);
		if (status != HAL_OK) 
		{
			HAL_TEST_DBG("sla tx error\n");
			break;
		}
		
		//wait tx cpl sem
		ret = wl_wait_sema(&sla_tx_sema,ONEMINUTE);
		if(!ret) {	
			HAL_TEST_DBG("sla tx timeout\n");
			break;
		}
		
		//totalDates add here
		newstarttest = TRUE;
		totalDatas += TRANSMIT*2;
			
	}
exit:  
		wl_free_sema(&uart_thread_destory_sema);
		wl_free_sema(&sla_rx_sema);
		wl_free_sema(&sla_tx_sema);
		at_test_led_deint(1);
		s907x_hal_timer_base_deinit(&timx_hdl);
		//hal_uart_deinit(uart);
		memset((void*)&uart_hdl, 0, sizeof(uart_hdl));
		newstarttest = FALSE;
		totalDatas = 0;
		runtime_cnt = 0;
		HAL_TEST_DBG("uart_sla_task exit\r\n");
		wl_destory_threadself();	
}

void uart_mst_task(void *context)
{
      hal_test_t *test = context;
      ASSERT(test->arg_cnt >= 5);
      u32 ret = 0;
      u8 mode = test->arg[5];
      uart_txrx_test_buff_init();//data init

      wl_init_sema(&mst_rx_sema,0,sema_binary);//rx sem init
      wl_init_sema(&mst_tx_sema,0,sema_binary);//tx sem init
      
      
      uart_hdl_t *uart = &uart_hdl;
      hal_status_e status;
      
      uart_config(uart, test);
      s907x_hal_uart_init(uart);

      uart->it.tx_complete.func= perf_txdone_cb;
      uart->it.tx_complete.context = uart;
      uart->it.rx_complete.func= perf_rxdone_cb;
      uart->it.rx_complete.context = uart;
      uart->it.trx_error.func = trx_error_cb;
      uart->it.trx_error.context = uart;
      //uart0 init done
      
      test->result = TRUE;
	  master_tim_sel = TRUE;
      
      while(TRUE)//mst
      {

		//mst tx begin
		status = s907x_hal_uart_tx_it(uart, (u8 *)mst_txbuffer, TRANSMIT);
          
		if (status != HAL_OK) 
		{
			HAL_TEST_DBG("mst tx error\n");
			break;
		}
   
        //wait tx cpl sem
        ret = wl_wait_sema(&mst_tx_sema,ONEMINUTE);//portMAX_DELAY
		
		if(!ret)
		{
			HAL_TEST_DBG("mst tx timeout\n");
			break;
		}
			
        memset(mst_rxbuffer, 0, DATA_SIZE);
		
		
        status = s907x_hal_uart_rx_it(uart,(u8 *)mst_rxbuffer, TRANSMIT);
            
		if (status != HAL_OK) 
		{
			HAL_TEST_DBG("mst rx error\n");
			break;
		}
		
		//wait rx cpl sem
		ret = wl_wait_sema(&mst_rx_sema,ONEMINUTE);//portMAX_DELAY
        //get sem
		if(!ret)
		{
			HAL_TEST_DBG("mst rx timeout\n");
			break;
		}
		
		if(memcmp(mst_rxbuffer, sla_txbuffer, TRANSMIT))
		{
			HAL_TEST_DBG("mst rec datas  error\n");
			break;
		}
		
		ret = wl_wait_sema(&uart_thread_destory_sema,0);//10ms
		if(ret){
			break;
		}
		
		//totalDates add here
		newstarttest = TRUE;
		totalDatas += TRANSMIT*2;

      }
	  
exit:
		wl_free_sema(&uart_thread_destory_sema);
		wl_free_sema(&mst_rx_sema);
		wl_free_sema(&mst_tx_sema);
		at_test_led_deint(0);
		s907x_hal_timer_base_deinit(&timx_hdl);
		//hal_uart_deinit(uart);
		memset((void*)&uart_hdl, 0, sizeof(uart_hdl));
		newstarttest = FALSE;
		totalDatas = 0;
		runtime_cnt = 0;
		HAL_TEST_DBG("uart_mst_task exit\r\n");
		wl_destory_threadself();	 
}


void serial_performance_test(hal_test_t *test)
{
	ASSERT(test->arg_cnt >= 5);
	u8 mode = test->arg[5];//0-mst 1-sla
	u8 key = test->arg[6];
        
	if(mode == 1)
	{
		if(key == 1)
		{
			//AT+UART=5,1,115200,0,0,0,1,1
            wl_init_sema(&uart_thread_destory_sema,0,sema_binary);//rx sem init		
			
			at_test_led_init(1);
			at_test_timer_init(test);
			
			wl_create_thread("uart_sla_thread", 512, MAIN_UART_RX_PRIO, (thread_func_t)uart_sla_task, test);	
			
			HAL_TEST_DBG("sla ready, waiting mst ...");
                        
		}
		else if(key == 0)
		{
			// AT+UART=5,1,115200,0,0,0,1,0
			
			wl_send_sema(&uart_thread_destory_sema);  
            wl_os_mdelay(1000);
  	
		}
	}
	else if(mode == 0)
	{
		if(key == 1)
		{
			//AT+UART=5,1,115200,0,0,0,0,1
			
			wl_init_sema(&uart_thread_destory_sema,0,sema_binary);//rx sem init
			
			at_test_led_init(0);
			at_test_timer_init(test);
                  
            wl_create_thread("uart_mst_thread", 512, MAIN_UART_RX_PRIO, (thread_func_t)uart_mst_task, test);
			
			HAL_TEST_DBG("mst runing...\n");
		}
		else if(key == 0)
		{
			//AT+UART=5,1,115200,0,0,0,0,0
                  
			wl_send_sema(&uart_thread_destory_sema);  
			wl_os_mdelay(1000);
		}
		
	}
	
	
	
}


//example
hal_test_name_map_t uart_test_map[] = 
{
	{0, "polling mode"},
	{1, "interrupt mode"},
	{2, "interrupt timeout"},
	{3, "DMA mode"},
	{4, "DMA for txstop mode"},
	{5, "Serial Performance Test"},
};

/*		test    no	 datalen  baud  parity  stopbits lpmode  loop  nums 
*       @UART   0     0-1     xxx	0-2     0-1		 0-2     10   0-255 	(polling mode)
*       @UART   1     0-1 	  xxx	0-2 	0-1  	 0-2     10	  0-255		(interrupt mode)
*       @UART   2     0-1     xxx	0-2 	0-1  	 0-2     10	  0-255		(interrupt timeout mode)
*       @UART   3     0-1	  xxx	0-2     0-1      0-2     10	  0-255	 	(DMA mode)
*/



void uart_test(hal_test_t *test)
{
	uart_hdl_t *uart = &uart_hdl;
	ASSERT(test);
	ASSERT(test->no < (sizeof(uart_test_map)/sizeof(hal_test_name_map_t)));

	HAL_TEST_DBG("test no %d = %s\n",test->no, uart_test_map[test->no].name);
        
	if(test->no == UART_PERFORMANCE_TEST)
	{
		serial_performance_test(test);
		wl_os_mdelay(500);
		return;
	}

	memset((void*)&uart_hdl, 0, sizeof(uart_hdl));
    //fix uart
	uart->config.idx = 0;

	//hal_uart_deinit(uart);

	/* reset Test Data */
	memset((void*)rxbuffer, 0, sizeof(rxbuffer));
	memset((void*)txbuffer, 0, sizeof(txbuffer));
		
	switch(test->no) 
	{
		case UART_POLLING_MODE:
			uart_polling_mode_test(test);
			break;
		case UART_INT_MODE:
			uart_int_mode_test(test);
			break;
		case UART_INT_TIMEOUT:
			uart_int_timeout_test(test);
			break;			
		case UART_DMA_MODE:
			uart_dma_mode_test(test);
			break;
        case UART_DMA_TXSTOP_MODE: 
            uart_dma_mode_txstop_test(test);
            break;
		default:

			break;
	}


}

#endif