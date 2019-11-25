/* Copyright Statement:
 *
 * (C) 2016-2018  SCI Inc. All rights reserved.
 */
#include "s907x.h"
#include "at_wlan.h"
#include "at_network.h"
#include "at_test.h"
       
//support at type
//set  AT+xxx=sss,sss,ss,
//get  AT+xxx? 
   
//HEAD + BODY + VALUE
 
at_hdl_t at_hdl;
static  loguart_t loguart;
 
static void atbase_test(void *context)
{
	at_cmd_t *cmd = (at_cmd_t *)context;
	ASSERT(cmd);
	
	at_rsp(AT_OK);
}

static void atbase_reset(void *context)
{
	at_cmd_t *cmd = (at_cmd_t *)context;
	ASSERT(cmd);
	
	HAL_NVIC_SystemReset();
	//wl_hal_delay(200);
	at_rsp(AT_OK);
	

}

static void atbase_version(void *context)
{
	at_cmd_t *cmd = (at_cmd_t *)context;
	ASSERT(cmd);
	
	
}

static void atbase_gsleep(void *context)
{
	at_cmd_t *cmd = (at_cmd_t *)context;	
	ASSERT(cmd);	
}

static void atbase_sleep(void *context)
{
	at_cmd_t *cmd = (at_cmd_t *)context;
	ASSERT(cmd);
}

static void atbase_display(void *context)
{
	at_cmd_t *cmd = (at_cmd_t *)context;
	ASSERT(cmd);
}

static void atbase_restore(void *context)
{
	at_cmd_t *cmd = (at_cmd_t *)context;
	ASSERT(cmd);
}

static void atbase_uartcur(void *context)
{
	at_cmd_t *cmd = (at_cmd_t *)context;
	ASSERT(cmd);
}

static void atbase_uartdef(void *context)
{
	at_cmd_t *cmd = (at_cmd_t *)context;
	ASSERT(cmd);
}	

static void atbase_wakegpio(void *context)
{
	at_cmd_t *cmd = (at_cmd_t *)context;
	ASSERT(cmd);
}

static void atbase_rfpower(void *context)
{
	at_cmd_t *cmd = (at_cmd_t *)context;
	ASSERT(cmd);
}
static void atbase_sram(void *context)
{
	at_cmd_t *cmd = (at_cmd_t *)context;
	ASSERT(cmd);
} 

static void atbase_adc(void *context)
{
	at_cmd_t *cmd = (at_cmd_t *)context;
	ASSERT(cmd);
} 

static void atbase_setiocfg(void *context)
{
	at_cmd_t *cmd = (at_cmd_t *)context;
	ASSERT(cmd);
} 

static void atbase_getiocfg(void *context)
{
	at_cmd_t *cmd = (at_cmd_t *)context;
	char *set;

	ASSERT(cmd);

	if(cmd->setsize) { //write mode
		set = cmd->set;
		at_rsp(set);
	}
} 

static void atbase_gpiodir(void *context)
{
	at_cmd_t *cmd = (at_cmd_t *)context;
	ASSERT(cmd);
} 

static void atbase_gpiowirte(void *context)
{
	at_cmd_t *cmd = (at_cmd_t *)context;
	ASSERT(cmd);
} 

static void atbase_gpioread(void *context)
{
	at_cmd_t *cmd = (at_cmd_t *)context;
	ASSERT(cmd);
} 

static void atbase_curmsg(void *context)
{
	at_cmd_t *cmd = (at_cmd_t *)context;
	ASSERT(cmd);
} 
 
static void atbase_defmsg(void *context)
{
	at_cmd_t *cmd = (at_cmd_t *)context;
	ASSERT(cmd);
} 
 

static at_item_t at_base_tbl[] = 
{
	{{"TEST"}, atbase_test},   
	{{"RESET"}, atbase_reset},     
	{{"VERSION"}, atbase_version},
	{{"GSLEEP"}, atbase_gsleep},
	{{"SLEEP"}, atbase_sleep},
	{{"ATE"}, atbase_display},
	{{"RESTORE"}, atbase_restore},
	{{"UART_CUR"}, atbase_uartcur},
	{{"UART_DEF"}, atbase_uartdef},
	{{"WAKEUPGPIO"}, atbase_wakegpio},
	{{"RFPOWER"}, atbase_rfpower},
	{{"SYSRAM"}, atbase_sram},
	{{"SYSADC"}, atbase_adc},
	{{"SYSIOSETCFG"}, atbase_setiocfg},
	{{"SYSIOGETCFG"}, atbase_getiocfg},
	{{"SYSGPIODIR"}, atbase_gpiodir},
	{{"SYSGPIOWRITE"}, atbase_gpiowirte},
	{{"SYSGPIOREAD"}, atbase_gpioread},
	{{"SYSMSG_CUR"}, atbase_curmsg},
	{{"SYSMSG_DEF"}, atbase_defmsg},
};


static void at_task(void *context);




static void at_base_init(void)
{
	at_add_cmd(&at_base_tbl[0], (sizeof(at_base_tbl)/sizeof(at_item_t)));	
}


static void at_base_deinit(void)
{
	at_remove_cmd(&at_base_tbl[0], (sizeof(at_base_tbl)/sizeof(at_item_t)));	

}
  

void at_add_module(void)
{
	at_base_init();
#if M_AT_ESP 
    at_wlan_init();
	at_network_init();	
#endif
#if M_AT_TEST
	at_test_init();
#endif
}

void at_remove_module(void)
{
	at_base_deinit();
#if M_AT_ESP
	at_wlan_deinit();
	at_network_deinit();
#endif
#if M_AT_TEST
	at_test_deinit();
#endif
}


static void loguart_hdl_recv(void *context)
{
   loguart_t *ploguart = (loguart_t*)context;
    
   ASSERT( ploguart );
   
   if(loguart.user_define) {
      //coustom define, default use sci consol program
	  wlan_uart_rx_ll_cb(ploguart);	
   } else {
   	  wl_send_sema_fromisr(loguart.finsh_sema);
   }
}
 
 
//register at to loguart
static void at_reg_loguart(at_hdl_t *at)
{
    ASSERT(at);
    
    loguart.finsh_sema = &at_hdl.sema;
    loguart.recv_hdl.func = loguart_hdl_recv;
    loguart.recv_hdl.context = (void*)&loguart;
    loguart.rxbuf = at->rxbuf;
    loguart.rxlen = 0;
	loguart.object = NULL;
    loguart.busy = FALSE;
    loguart.user_define = FALSE;
    loguart_init(&loguart);
    change_irq_polarity(M_RCC_PERIPH_LOGUART, LOGUART_TIMER_PRIO);
  
}

//unregister at to loguart
static void at_unreg_loguart(at_hdl_t *at)
{  
    loguart_deinit(&loguart);  
}


int at_hdl_int(void)
{
	//sema only from uart isr
	wl_init_sema(&at_hdl.sema, 0, sema_counter);
	at_reg_loguart(&at_hdl);
	wl_init_list(&at_hdl.tbl);
	at_hdl.op_mode = AT_TASK_OP_NORMAL_CMD;
	at_hdl.exit = FALSE;
	at_add_module();
	wl_create_thread("at thread", AT_TASK_STACK_SZ, AT_TASK_PRIO, (thread_func_t)at_task, &at_hdl);
	return 0;	
}

void at_hdl_deinit(void)
{
	at_hdl.exit = TRUE; 
	wl_send_sema(&at_hdl.sema);
}




int at_get_param(char **value, char *val)
{
    char delim[]= ",";
    char *token;
    int cnt = 0;

    for(token = strsep(&val,","); token != NULL; token = strsep(&val, delim)) {
        if(*token)
			*value++ = token;
		else
			*value++;
         cnt++;
         if(cnt >= AT_SET_MAX_ARGC) {
            break;
         }
    } 
    return cnt;
}

int at_parse_param_str(char **argv)
{
    int ret = HAL_ERROR;
    char *param = *argv;

    if (AT_STR_TAG != *param) {
        return ret;
    }

    *param++ = AT_TAIL_TAG;
    *argv = param;

    while((AT_STR_TAG != *param) && (AT_TAIL_TAG != *param)) {
        param++;
    }

    if (AT_STR_TAG != *param)
    {
        return ret;
    }

    *param = AT_TAIL_TAG;
	param++;
	if(AT_TAIL_TAG != *param)
		return ret;
	
    ret = HAL_OK;
    return ret;
}

//=xxxTAGxxxTAGxxx  put xxx xxx xxx ->argv[x]
int at_parse_set(char *set, char **argv, char tag)
{
	int argc = 0;

	if(set == NULL)
		return 0;

	while((argc < AT_SET_MAX_ARGC) && (*set != AT_TAIL_TAG)) {
		while(*set == tag){
			if((*set == tag) && (*(set+1) == tag)){
				argv[argc] = NULL;
				argc++;
			}
			*set = AT_TAIL_TAG;
			set++;
		}
		if(*set == AT_TAIL_TAG)
			break;
		else{
			argv[argc] = set;
		}
		argc++;
		set++;
		while( (*set != tag)&&(*set != AT_TAIL_TAG) ) {
			set++;
		}
	}
	return argc;
}


static int at_get_body(char *start, char *body, int *mode)
{
	u8  temp;
	int body_len = 0;

	ASSERT(start);
	ASSERT(body);
	ASSERT(mode);

    //default is at action
    *mode = AT_MODE_ACT;

	while(1) {
		temp = *start++;
		//tail match
		if(temp == AT_TAIL_TAG) {
			break;
		}  
		//not overfolw and match
		if(temp == AT_READ_TAG  || 
		   temp == AT_WRITE_TAG)	{	
		   *mode = (temp == AT_WRITE_TAG ? AT_MODE_W : AT_MODE_R);
			break;
		}
        body[body_len++] = temp;
		//body len overfolw
		if(body_len >= AT_BODY_LEN - 2) {
			break;
		}                
	}
   //ADD TAIL
    if(body_len < AT_BODY_LEN) {
        body[body_len] = 0;
    }
    else {
        body[AT_BODY_LEN - 1] = 0;
    }
	return body_len;
}

static u16 at_body_hash(char *body)
{	
	u32 sum = 0;

	while(*body) {
		sum += (*body++);
	}
	return (u16)(sum & 0xFFFF);
}

static int at_get_value(char *start, char *val)
{
	u8  temp;
	int val_len = 0;

	ASSERT(start);
	ASSERT(val);

	while(1) {
		temp = *start++;
		//tail match
		if(temp == AT_TAIL_TAG) {
			break;
		}
        val[val_len++] = temp;
		//val len overfolw
		if(val_len >= AT_VALUE_LEN - 2) {
			
			break;
		}
	}
     //ADD TAIL
    if(val_len > 0)
      val[val_len] = 0;       
	return val_len;

}


void at_add_cmd(at_item_t *item, int len)
{
	int i;

	ASSERT(item);
	
	wl_enter_critical();

	for(i = 0; i < len; i++) {	
		item->cmd.hash = at_body_hash(item->cmd.body);
		wl_list_add(&item->node, &at_hdl.tbl);
		item ++;
	}

	wl_exit_critical();
}


void at_remove_cmd(at_item_t *item, int len)
{
	at_item_t *node;
	_list *pos;
	int i;

	wl_enter_critical();	
	for( i = 0; i < len; i++) {
		item->cmd.hash = at_body_hash(item->cmd.body);
		list_for_each(pos, &at_hdl.tbl) {
			//find item
			node = list_entry(pos, at_item_t, node);
			if((node->cmd.hash == item->cmd.hash) &&
			   !strcmp(node->cmd.body, item->cmd.body)){
				wl_list_del(pos);
				break;
			}
		}
	}
	wl_exit_critical();
}

void at_cmd_set_passth_mode(u32 enable, void *object, int is_isr)
{
	if(!is_isr)
    	wl_enter_critical();

    if(enable) {
		//stop at command
        at_cmd_set_operate_mode(AT_TASK_OP_RECV_UART_DATA);
		loguart.recv_hdl.func = wlan_uart_rx_ll_cb;
		loguart.recv_hdl.context = (void*)&loguart;
		loguart.object = object;
        
		//passth_mode or SEND_CMD_MODE_BUF -> start timer
		wlan_uart_passth_event(&loguart, TRUE);
		loguart.user_define = TRUE;
    } else {
		//resume at command 
		//at_hdl.normal_cmd = TRUE;
		at_cmd_set_operate_mode(AT_TASK_OP_NORMAL_CMD);
		loguart.recv_hdl.func = loguart_hdl_recv;
		loguart.recv_hdl.context = (void*)&loguart;
		wlan_uart_passth_event(&loguart, FALSE);
		loguart.object = NULL;
		loguart.user_define = FALSE;
    }

	if(!is_isr)
    	wl_exit_critical();
    
}

void *at_cmd_get_passth_conn(void)
{
    return loguart.object;
}

int at_cmd_get_operate_mode(void)
{
    return at_hdl.op_mode;
}

void at_cmd_set_operate_mode(int op_mode)
{
    wl_enter_critical();
    at_hdl.op_mode = op_mode;
    wl_exit_critical(); 
}

static void at_cmd_hdl(at_hdl_t *at)
{
	at_item_t *item;
	int   bodylen = 0;
	int   vallen = 0;
    int   argc = 0;
	int   mode;
	u16   hash;
    u8    find = FALSE;
   _list *pos;
	char  *set_pos = NULL;
	char  rxbuf[AT_BUF_MAXLEN] = {0};
	char  body[AT_BODY_LEN] = {0};
	char  value[AT_VALUE_LEN] = {0};	

	ASSERT(at);

	at->lock = _LOCK;
	memcpy(rxbuf, at->rxbuf, AT_BUF_MAXLEN-1);
	at->lock = _UNLOCK;
    //memset(at->rxbuf, 0, sizeof(at->rxbuf));

	if(!mode_switch_hdl(rxbuf)){
		return;
	}

    //for mp mode
    if(s907x_wlan_hanlde_mp(rxbuf) == HAL_OK) {
        return;
    }
	//head
	if(!AT_HEAD_ASSERT(rxbuf)) {
        at_rsp(AT_ERROR);
		return;
	}
	//body
	bodylen = at_get_body(AT_BODY(rxbuf), body, &mode);
	if(!bodylen || (AT_BODY_LEN <= bodylen)) {
        AT_DBG_ERR("Get command body fail, command[%s] len[%d].", body, bodylen);
        at_rsp(AT_ERROR);
		return;
	}
	hash = at_body_hash(body);
	//value
	if(mode == AT_MODE_W) {
		vallen = at_get_value(AT_VALUE(rxbuf, bodylen), value);
		set_pos = value;
	} else if(mode == AT_MODE_R){
		vallen = at_get_value(AT_VALUE(rxbuf, bodylen), value);
		set_pos = value;
	}
	else{
		vallen = 0;
		set_pos = NULL;
	}
    find = FALSE;
	wl_enter_critical();
	list_for_each(pos, &at->tbl) {
		//find item
		item = list_entry(pos, at_item_t, node);
		if((item->cmd.hash == hash) &&
		   !strcmp(item->cmd.body, body)){
            find = TRUE;
			break;
		}
	}		
	wl_exit_critical();
	if(item->func && find) {
        item->cmd.mode    = mode;
		item->cmd.set     = set_pos;
		item->cmd.setsize = vallen;	
        item->cmd.set[vallen] = 0;
        //clear argv
        if(argc < AT_SET_MAX_ARGC) {
            item->func(&item->cmd);
        }
        AT_DBG_MSG("Return from item->func.");
	}
    else {
        AT_DBG_ERR("No related function registered for %s.", body);
        at_rsp(AT_ERROR);
    }
}

static void at_uart_data_hdl(at_hdl_t *at)
{
    switch (at->op_mode)
    {
        case AT_TASK_OP_NORMAL_CMD:
            at_cmd_hdl(at);
            break;
#if M_AT_ESP
        case AT_TASK_OP_RECV_UART_DATA:
            if(wlan_uart_rx_hdl(&loguart) == HAL_NO_MEMORY) {
                AT_DBG_WARN("No more memory.");
            }
            break;
            
        case AT_TASK_OP_SEND_DATA_BLOCK:
            wlan_uart_wait_for_send(&loguart);
            break;
#endif
        default:
            AT_DBG_WARN("Unkown at op mode [%d].", at->op_mode);
    }
    return;
}
static void at_task(void *context)
{
	u32 ret;
	u32 timeout = portMAX_DELAY;
	at_hdl_t *at = (at_hdl_t *)context;
	
	while(1) {
		ret = wl_wait_sema(&at_hdl.sema, timeout);

		if((ret) || (AT_TASK_SEMA_TIMEOUT == timeout)) {
			at_uart_data_hdl(at);
		}
#if M_AT_ESP
		ret = wlan_uart_rx_buffer_empty();
		timeout = (!ret) ? AT_TASK_SEMA_TIMEOUT : portMAX_DELAY;
#endif
		if(at->exit) {
			break;
		}
	}
	//exit 
	at_remove_module();
	at_unreg_loguart(&at_hdl);
	wl_free_sema(&at_hdl.sema);
	wl_destory_threadself();
}


