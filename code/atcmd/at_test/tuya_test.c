#include "s907x.h"
#include "string.h"
#include "lwip_conf.h" 
#include "wlan_test.h"
#include "dhcps.h"
         
       
#if TUYA_BUILD     

#include "tuya_uart.h"
#include "wifi_hwl.h"

static u8 current_wlan_mode = 0;
static sema_t scan_sema = NULL;
static int  dhcp_server_init = 0;
static monitor_t test_monitor_data;
static	struct ip_addr auto_reconnect_ipaddr;
static	struct ip_addr auto_reconnect_netmask;
static	struct ip_addr auto_reconnect_gw;
       
#if bhu
/***********************************************************
*  Function: hwl_wf_all_ap_scan
*  Input: none
*  Output: ap_ary num
*  Return: OPERATE_RET
***********************************************************/
_WIFI_HWL_EXT \
OPERATE_RET hwl_wf_all_ap_scan(OUT AP_IF_S **ap_ary,OUT UINT_T *num);

/***********************************************************
*  Function: hwl_wf_assign_ap_scan
*  Input: ssid
*  Output: ap
*  Return: OPERATE_RET
***********************************************************/
_WIFI_HWL_EXT \
OPERATE_RET hwl_wf_assign_ap_scan(IN CONST CHAR_T *ssid,OUT AP_IF_S **ap);

/***********************************************************
*  Function: hwl_wf_release_ap
*  Input: ap
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
_WIFI_HWL_EXT \
OPERATE_RET hwl_wf_release_ap(IN AP_IF_S *ap);

/***********************************************************
*  Function: hwl_wf_set_cur_channel
*  Input: chan
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
_WIFI_HWL_EXT \
OPERATE_RET hwl_wf_set_cur_channel(IN CONST BYTE_T chan);

/***********************************************************
*  Function: hwl_wf_get_cur_channel
*  Input: none
*  Output: chan
*  Return: OPERATE_RET
***********************************************************/
_WIFI_HWL_EXT \
OPERATE_RET hwl_wf_get_cur_channel(OUT BYTE_T *chan);

/***********************************************************
*  Function: hwl_wf_sniffer_set
*  Input: en cb
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
_WIFI_HWL_EXT \
OPERATE_RET hwl_wf_sniffer_set(IN CONST BOOL_T en,IN CONST SNIFFER_CALLBACK cb);

/***********************************************************
*  Function: hwl_wf_get_ip
*  Input: wf
*  Output: ip
*  Return: OPERATE_RET
***********************************************************/
_WIFI_HWL_EXT \
OPERATE_RET hwl_wf_get_ip(IN CONST WF_IF_E wf,OUT NW_IP_S *ip);

/***********************************************************
*  Function: hwl_wf_set_ip
*  Input: wf
*  Output: ip
*  Return: OPERATE_RET
***********************************************************/
_WIFI_HWL_EXT \
OPERATE_RET hwl_wf_set_ip(IN CONST WF_IF_E wf,IN CONST NW_IP_S *ip);

/***********************************************************
*  Function: hwl_wf_get_mac
*  Input: wf
*  Output: mac
*  Return: OPERATE_RET
***********************************************************/
_WIFI_HWL_EXT \
OPERATE_RET hwl_wf_get_mac(IN CONST WF_IF_E wf,OUT NW_MAC_S *mac);

/***********************************************************
*  Function: hwl_wf_set_mac
*  Input: wf mac
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
_WIFI_HWL_EXT \
OPERATE_RET hwl_wf_set_mac(IN CONST WF_IF_E wf,IN CONST NW_MAC_S *mac);

/***********************************************************
*  Function: hwl_wf_wk_mode_set
*  Input: mode
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
_WIFI_HWL_EXT \
OPERATE_RET hwl_wf_wk_mode_set(IN CONST WF_WK_MD_E mode);

/***********************************************************
*  Function: hwl_wf_wk_mode_get
*  Input: none
*  Output: mode
*  Return: OPERATE_RET
***********************************************************/
_WIFI_HWL_EXT \
OPERATE_RET hwl_wf_wk_mode_get(OUT WF_WK_MD_E *mode);

/***********************************************************
*  Function: hwl_wf_station_connect
*  Input: ssid passwd
*  Output: mode
*  Return: OPERATE_RET
***********************************************************/
_WIFI_HWL_EXT \
OPERATE_RET hwl_wf_station_connect(IN CONST CHAR_T *ssid,IN CONST CHAR_T *passwd);

/***********************************************************
*  Function: hwl_wf_station_disconnect
*  Input: none
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
_WIFI_HWL_EXT \
OPERATE_RET hwl_wf_station_disconnect(VOID);

/***********************************************************
*  Function: hwl_wf_station_get_conn_ap_rssi
*  Input: none
*  Output: rssi
*  Return: OPERATE_RET
***********************************************************/
_WIFI_HWL_EXT \
OPERATE_RET hwl_wf_station_get_conn_ap_rssi(OUT SCHAR_T *rssi);

/***********************************************************
*  Function: hwl_wf_station_stat_get
*  Input: none
*  Output: stat
*  Return: OPERATE_RET
***********************************************************/
_WIFI_HWL_EXT \
OPERATE_RET hwl_wf_station_stat_get(OUT WF_STATION_STAT_E *stat);

/***********************************************************
*  Function: hwl_wf_ap_start
*  Input: cfg
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
_WIFI_HWL_EXT \
OPERATE_RET hwl_wf_ap_start(IN CONST WF_AP_CFG_IF_S *cfg);

/***********************************************************
*  Function: hwl_wf_ap_stop
*  Input: none
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
_WIFI_HWL_EXT \
OPERATE_RET hwl_wf_ap_stop(VOID);
#endif

	
static int  at_get_param(char **value, char *val)
{
    char delim[]= ",";
    char *token;
    int cnt = 0;

    for(token = strsep(&val,","); token != NULL; token = strsep(&val, delim)) {
        *value++ = token;
         cnt++;
         if(cnt >= AT_SET_MAX_ARGC) {
            break;
         }
    } 
    return cnt;
} 
 
 


//uart
int tuya_uart_deinit_test(char  **argv, int argc)
{
	int ret = 0;
	ret = ty_uart_free(TY_UART0);

	return ret;
}

u8 s_data[100] = {0,1,2,3,4,5,6,7,8,9,
			     0,1,2,3,4,5,6,7,8,9,
			     0,1,2,3,4,5,6,7,8,9,
			     0,1,2,3,4,5,6,7,8,9,
			     0,1,2,3,4,5,6,7,8,9,
			     0,1,2,3,4,5,6,7,8,9,
			     0,1,2,3,4,5,6,7,8,9,
			     0,1,2,3,4,5,6,7,8,9,
			     0,1,2,3,4,5,6,7,8,9,
			     0,1,2,3,4,5,6,7,8,9,};
u8 g_data[100] = {0};


int tuya_uart_send_test(char  **argv, int argc)
{
	u8 *data = s_data;
	int ret = 0;
	ty_uart_send_data(TY_UART0,data,100);
	
	return ret;
}


int tuya_uart_get_test(char  **argv, int argc)
{
	u8 *data = g_data;
	u32 len = atoi(argv[1]);
	int csize;
	memset(data,0,100);

	csize = ty_uart_read_data(TY_UART0,data,len);
	if(csize>0)
		printf("read data:\n");
		printf_arrary(data,csize,ARY_U8,0);


	return 0;
}



int tuya_uart_init_test(char  **argv, int argc)
{
	u32 baud,bufsz;
	int ret = 0;
	if(argc < 2) {
	    return HAL_ERROR;
	} 
	baud = atoi(argv[1]);
	bufsz = atoi(argv[2]);

	ret = ty_uart_init(TY_UART0,baud,TYWL_8B,TYP_NONE,TYS_STOPBIT1,bufsz);

	return ret;
}

int tuya_uart_loopback_test(char  **argv, int argc)
{
	u8 *pbuf = NULL;
	u8 *prbuf = NULL;
	u32 len = atoi(argv[1]);
	int offset = atoi(argv[2]);
	int csize;
	int ret;
	int cnt;
	
	if(len <= 0){
		ret = HAL_ERROR;
		goto exit;
	}
	
	pbuf = (u8*)wl_malloc(sizeof(u8)*len);
	if(!pbuf){
		ret = HAL_NO_MEMORY;
		goto exit;
	}
	
	prbuf = (u8*)wl_malloc(sizeof(u8)*len);
	if(!prbuf){
		ret = HAL_NO_MEMORY;
		goto exit;
	}
	
	for(cnt = 0; cnt < len; cnt++){
		*(pbuf+cnt) = cnt + offset;
	}
	ty_uart_send_data(TY_UART0,pbuf,len);
	wl_os_mdelay(10);
	csize = ty_uart_read_data(TY_UART0,prbuf,len);
	if(csize){
		if(!wl_memcmp(prbuf, pbuf, len)){
			ret = 0;
		}else{
			printf("uart read:\n");
			printf_arrary(prbuf,len,ARY_U8,0);
			printf("uart write:\n");
			printf_arrary(pbuf,len,ARY_U8,0);
			ret = HAL_ERROR;
		}
	}

exit:
	if(pbuf || prbuf){
		wl_free(pbuf);
		wl_free(prbuf);
	}
	return ret;
}



//gpio 
int tuya_gpio_inout_test(char  **argv, int argc)
{
	u32 port,in;
	int ret = 0;
	if(argc < 2) {
	    return HAL_ERROR;
	} 
	port = atoi(argv[1]);
	in = atoi(argv[2]);

	ret = tuya_gpio_inout_set(port,in);

	return ret;
}

int tuya_gpio_rw_test(char  **argv, int argc)
{
	u32 port,in,rw;
	u32 val;
	int ret = 0;

	rw = atoi(argv[1]);
	port = atoi(argv[2]);
	in = atoi(argv[3]);

	if(rw ==0)
	{
		val = tuya_gpio_read(port);
		printf(" val = %d\n",val);
	}
	else
		ret = tuya_gpio_write(port,in);

	return ret;
}
int gpio_irq(void *context)
{
    u32 *gpio_pin = (u32 *)context;
	u8 i;
    u32 pin_num;

    for(i = 0; i < 32; i++) {
        if(*gpio_pin & BIT(i)) {
            pin_num = i;
			break;
        }
    }
	
    HAL_TEST_DBG("GPIO %x rd status = %d\n", pin_num, s907x_hal_gpio_read(*gpio_pin));
	
	s907x_hal_gpio_it_stop(*gpio_pin);

}

int tuya_gpio_irq_test(char  **argv, int argc)
{
	u32 port;
	int ret = 0;

	port = atoi(argv[1]);

	ret = tuya_gpio_irq_init(port,gpio_irq,0);


	return ret;
}

// wlan
NW_IP_S tuya_ip;

int tuya_smode_hdl(char  **argv, int argc)
{
    int ret;
    u8  mode;
    if(argc < 2) {
        return HAL_ERROR;
    } 
    mode = atoi(argv[1]);
    if(mode > WWM_STATIONAP) {
        return HAL_ERROR;
    } 
	ret = hwl_wf_wk_mode_set(mode);    
 
	HAL_TEST_DBG("set wlan mode = %d\n", mode);
    return ret;
}

int tuya_wlan_ap_get_client(void)
{
	int ret = HAL_OK;
	int nums;
	int i;
	s907x_ap_client_infor_t *ap_clinet, *head;
	
	nums = s907x_wlan_ap_get_client_nums();
	if(nums > 0) {
		head = ap_clinet = wl_zmalloc(sizeof(s907x_ap_client_infor_t)*nums);
		if(!ap_clinet) {
			HAL_TEST_DBG("malloc ap client error\n");
			return HAL_ERROR;
		}
		ret = s907x_wlan_ap_get_client_infor(ap_clinet, nums);
		for( i = 0; i < nums; i++) {
			HAL_TEST_DBG("clinet %d mac "MAC_FMT" rssi = %d", i, ap_clinet->hw_addr[0], ap_clinet->hw_addr[1], ap_clinet->hw_addr[2],
            	ap_clinet->hw_addr[3], ap_clinet->hw_addr[4], ap_clinet->hw_addr[5], ap_clinet->rssi);
			ap_clinet++;
		}
		if(head) {
			wl_free(head);
			head = NULL;
		}
	} else {
		HAL_TEST_DBG("no sta connected now!\n");
	}
	return ret;	
}


int tuya_wlan_get_ip_test(char  **argv, int argc)
{
	NW_IP_S *ip= &tuya_ip;
	int ret;
	u8 wf = atoi(argv[1]);
	if(wf == 1)
		ret = hwl_wf_get_ip(WF_AP,ip);
	else if(wf == 0)
		ret = hwl_wf_get_ip(WF_STATION,ip);
	else
		ret = tuya_wlan_ap_get_client();
	if(2!=wf)
	{
		if(0==ret)
		{
			printf("get_ip_status:\n");
			printf("gate:%s \n",ip->gw);
			printf("ip:%s \n",ip->ip);
			printf("mask:%s \n",ip->mask);
		}else
			printf("get ip error\r\n");
	}
	return ret;

}

int tuya_wlan_set_mac_test(char  **argv, int argc)
{
	NW_MAC_S mac;
	int ret;
        u32 id;
	u8 wf = atoi(argv[1]);
	
	if(argc < 3) {
		return HAL_ERROR;
	}
	sscanf(argv[2], MAC_FMT, &mac.mac[0], &mac.mac[1], &mac.mac[2], &mac.mac[3],&mac.mac[4],&mac.mac[5]);

	ret = hwl_wf_set_mac(wf,&mac);
	
	return ret;
}

int tuya_wlan_get_mac_test(char  **argv, int argc)
{
	NW_MAC_S mac;
	int ret;
	u8 wf = atoi(argv[1]);
	

	if(argc < 2) {
		return HAL_ERROR;
	}

	ret = hwl_wf_get_mac(wf, &mac);
	if(!ret) {
		HAL_TEST_DBG("mac = "MAC_FMT"\n", mac.mac[0], mac.mac[1], mac.mac[2], mac.mac[3],mac.mac[4],mac.mac[5]);
	}

	return ret;

}

int tuya_wlan_channel_test(char  **argv, int argc)
{
	int ret;
	u32 mode = atoi(argv[1]);
	u8 channel=(u8)atoi(argv[2]);

	if(0!=mode)
		ret = hwl_wf_set_cur_channel(channel);
	else{
		ret = hwl_wf_get_cur_channel(&channel);
		if(!ret)
			printf("get channel = %d\r\n",channel);
	}
	return ret;
}

int tuya_wlan_scan_hdl(char  **argv, int argc)
{
	
	AP_IF_S *ap_ary[64];
	int scan_num = 1;
	u8 i;
	int ret;
	
	printf("heap size %d\n", tuya_SysGetHeapSize());

	if(argc == 1) {
	    ret = hwl_wf_all_ap_scan(ap_ary,&scan_num);
	    if(0!=ret) {
	        printf("scan error\r\n");
	    }
	} else if(argc == 2){
	    ret = hwl_wf_assign_ap_scan(argv[1], ap_ary);
	    if(!ret ) {
	        HAL_TEST_DBG("-----------------------scan result------------------\n");
	        HAL_TEST_DBG("ssid %s bssid "MAC_FMT" channel %d rssi  %d \n", ap_ary[0]->ssid, ap_ary[0]->bssid[0], 
	       ap_ary[0]->bssid[1],ap_ary[0]->bssid[2],ap_ary[0]->bssid[3],ap_ary[0]->bssid[4],ap_ary[0]->bssid[5], 
	       ap_ary[0]->channel, ap_ary[0]->rssi);           
	    }
	}	
	if(0== ret)
		hwl_wf_release_ap(*ap_ary);


	printf("heap size %d\n", tuya_SysGetHeapSize());

	return ret;
}

int tuya_wlan_sta_start(char  **argv, int argc)
{
	int ret = 0;
	char *ssid;
	char *password;
	int mode;
	//mode check
	hwl_wf_wk_mode_get((WF_WK_MD_E *)&mode);

	if(WWM_STATION != mode && mode != WWM_STATIONAP)
	{
		HAL_TEST_DBG("wifi mode:%d",mode);
		return -1;
	}

	ssid = argv[2];
	password = argv[3];
	
	
	ret = hwl_wf_station_connect(ssid,password);
	if(0!=ret)
	{
		printf("connected error \r\n");
		return -1;
	}
	
	
	

	return ret;
}

int tuya_wlan_sta_stop(char  **argv, int argc)
{
	int ret;
	
	ret = hwl_wf_station_disconnect();

	return ret;
}

int tuya_wlan_sta_hdl(char  **argv, int argc)
{
	int ret;
	int cmd;
	u8 mode;

	if(argc < 2) {
		return HAL_ERROR;
	}

	cmd = atoi(argv[1]);
	switch(cmd)
	{
		case 0:    // stop sta 
			ret = tuya_wlan_sta_stop(argv, argc);
			break;
		case 1:   // start sta
			ret = tuya_wlan_sta_start(argv, argc);
			break;
		default:  // none
			ret = -1;
			break;
	}


	return ret;

}

int tuya_wlan_ap_start(char  **argv, int argc)
{
	WF_AP_CFG_IF_S cfg;
	int ret = 0;
	
	cfg.s_len = strlen(argv[2]);
	cfg.p_len = strlen(argv[3]);
	memcpy(cfg.ssid,argv[2],cfg.s_len);
	memcpy(cfg.passwd,argv[3],cfg.p_len);
	cfg.chan = atoi(argv[4]);
	cfg.ssid_hidden = atoi(argv[5]);
	cfg.md = atoi(argv[6]);

	ret = hwl_wf_ap_start(&cfg);

	return ret;
}

int tuya_wlan_ap_stop(char  **argv, int argc)
{
	int ret;
	
	ret = hwl_wf_ap_stop();

	return ret;
}

int tuya_wlan_ap_hdl(char  **argv, int argc)
{

	int ret;
	int cmd;
	u8 mode;

	if(argc < 2) {
		return HAL_ERROR;
	}

	cmd = atoi(argv[1]);	
	switch(cmd)
	{
		case 0:    // stop ap 
			ret = tuya_wlan_ap_stop(argv, argc);
			break;
		case 1:   // start ap  
			ret = tuya_wlan_ap_start(argv, argc);
			break;
		default:  // none
			ret = -1;
			break;
	}


	return ret;
}

void sniffer_cb(u8 *data,u16 len )
{
	if((data != NULL)&&(len!=0))
		printf_arrary(data,len,ARY_U8,8);
}

int tuya_wlan_sniffer_hdl(char  **argv, int argc)
{
	int ret =0;
	int en = atoi(argv[1]);
	if(en == 1)
		ret = hwl_wf_sniffer_set(1,(const SNIFFER_CALLBACK)sniffer_cb);
	else
		ret = hwl_wf_sniffer_set(0,NULL);


	return ret;
}


int tolower(int c)  
{  
    if (c >= 'A' && c <= 'Z')   
        return c + 'a' - 'A';   
    else  
        return c;  
}

int htoi(char s[])  
{  
    int i;  
    int n = 0;  
    if (s[0] == '0' && (s[1]=='x' || s[1]=='X')){  
        i = 2;  
    }  
    else  {  
        i = 0;  
    }  
    for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >='A' && s[i] <= 'Z');++i){  
        if (tolower(s[i]) > '9')  {  
            n = 16 * n + (10 + tolower(s[i]) - 'a');  
        }  
        else  {  
            n = 16 * n + (tolower(s[i]) - '0');  
        }  
    }  
    return n;  
}

int tuya_flash_read(u32 ad, u8* buf, u32 length)
{
	int ret;
	ret = tuya_uni_flash_read(ad, buf, length);
	if(!ret){
		printf_arrary(buf,length,ARY_U8,0);
	}
	return ret;
}

int tuya_flash_write(u32 ad, u8* buf, u32 length)
{
	int ret;
	ret = tuya_uni_flash_write(ad, buf, length);
	return ret;
}

int tuya_flash_erase(u32 ad,u32 length)
{
	int ret;
	ret = tuya_uni_flash_erase(ad, length);
	return ret;
}



//AT+TUYA=flash,read,addr,len			
//AT+TUYA=flash,erase,addr,len
//AT+TUYA=flash,write,addr,len,buf_offset
/*for example:
	AT+TUYA=flash,0,18003000,100   (read)
	AT+TUYA=flash,2,18003000,100   (erase)
									
								   (write)
	AT+TUYA=flash,1,18003000,10,0
		write buffer:0,1,2,3,4,5,6,7,8,9
	AT+TUYA=flash,1,18003000,10,1
		write buffer:1,2,3,4,5,6,7,8,9,10
*/

/*note:this code test api function,not normal application.*/
int tuya_flash_test_hdl(char  **argv, int argc)
{
	int ret = 0;
	int cmd;
	u8 mode;
	u32 addr;
	u32 len;
	u8 *pbuf = NULL; 

	if(argc < 2) {
		return HAL_ERROR;
	}
	cmd = atoi(argv[1]);	
	addr = strtoul((const u8*)(argv[2]), (u8 **)NULL, 16);
	len = atoi(argv[3]);
	
	pbuf = wl_malloc(sizeof(u8)*len);
	if(!pbuf){
		ret = HAL_NO_MEMORY;
		goto exit;
	}
	if(cmd == 1){
		if(argc < 3)
			return HAL_ERROR;
		u8 offset = atoi(argv[4]);
		for(int i = 0; i < len; i++) {
			*(pbuf+i) = i + offset;
		}

	}
	switch(cmd){
		case 0://read
			ret = tuya_flash_read(addr,pbuf,len);
		break;
		case 1://write  
			ret = tuya_flash_write(addr,pbuf,len);
		break;	
		case 2://erase
			ret = tuya_flash_erase(addr, len);
		break;
	default:
		ret = HAL_ERROR;
		break;
	}	
exit:
	if(pbuf){
		wl_free(pbuf);
	}
	pbuf = NULL;
	return ret;
}


int reg_rw(char  **argv, int argc)
{
	u32 rw = strtoul((const u8*)(argv[1]), (u8 **)NULL, 16);
	u32 addr = strtoul((const u8*)(argv[2]), (u8 **)NULL, 16);
	u32 data = strtoul((const u8*)(argv[3]), (u8 **)NULL, 16);
	pll_clock_switch(RCC_PERIPH_UART, HAL_ON);    

	rcc_clock_switch(RCC_PERIPH_UART, HAL_ON);
	if(rw){
	printf("addr = %x  data = %x\r\n",addr,data);
	HAL_WRITE32(0x400c0000,addr,data);
	}else
	{
		printf("addr = %x  data = %x\r\n",addr,HAL_READ32(0x400c0000,addr));
	}
        return 0;
}



void tuya_test(void *context)
{
    char  *argv[AT_SET_MAX_ARGC];
    int    argc;
	int    ret;
	char  *item;
    
	at_cmd_t *cmd = (at_cmd_t *)context;

	argc = at_get_param(argv, cmd->set);
	if(!argc) {
	    return;
	}
	item = argv[0];

// tuya uart api test
	if(!strcmp(item, "u_init")) {
		ret = tuya_uart_init_test(argv, argc);                                    
	} else if(!strcmp(item, "u_send")) {
	    ret = tuya_uart_send_test(argv, argc);   
	} else if(!strcmp(item, "u_read")) {
	    ret = tuya_uart_get_test(argv, argc);   
	} else if(!strcmp(item, "u_deinit")) {
	    ret = tuya_uart_deinit_test(argv, argc);   
	}else if(!strcmp(item, "u_loopback")){
		ret = tuya_uart_loopback_test(argv, argc); 
	}
// tuya gpio api test

	if(!strcmp(item, "g_init")) {	
		ret = tuya_gpio_inout_test(argv, argc);                               
	} else if(!strcmp(item, "g_rw")) {
	    ret = tuya_gpio_rw_test(argv, argc); 
	} else if(!strcmp(item, "g_irq")) {
	    ret = tuya_gpio_irq_test(argv, argc);   
	} 

//tuya wlan api test
	if(!strcmp(item, "smode")){
		ret = tuya_smode_hdl(argv,argc);
	}else if(!strcmp(item, "sta")) {
		ret = tuya_wlan_sta_hdl(argv,argc);
	}else if(!strcmp(item, "ap")) {
	    ret = tuya_wlan_ap_hdl(argv, argc);   
	} else if(!strcmp(item, "scan")) {
	    ret = tuya_wlan_scan_hdl(argv, argc);   
	} else if(!strcmp(item, "gmac")) {
	    ret = tuya_wlan_get_mac_test(argv, argc);   
	} else if(!strcmp(item, "smac")) {
	    ret = tuya_wlan_set_mac_test(argv, argc);   
	} else if(!strcmp(item, "ip")) {
	    ret = tuya_wlan_get_ip_test(argv, argc);   
	} else if(!strcmp(item, "channel")) {
	    ret = tuya_wlan_channel_test(argv, argc);   
	} 

	if(!strcmp(item, "rw")) {
		ret =reg_rw(argv,argc);
	}


// tuya sniffer api test
	if(!strcmp(item, "sniffer")) {
		ret = tuya_wlan_sniffer_hdl(argv,argc);
	}
	
//tuya flash api test
	if(!strcmp(item, "flash")){
		ret = tuya_flash_test_hdl(argv,argc);
	}

	if(!ret) {
		HAL_TEST_DBG("test tuya item = %s success\n", item); 
	} else {
		HAL_TEST_DBG("test tuya item = %s failed ret = %d\n", item, ret); 
	}
}
#else

    
void tuya_test(void *context)
{

    USER_DBG("user main start..."); 
	printf("heap size %d\n", xPortGetFreeHeapSize());	
 //   user_main();
	printf("heap size %d\n", xPortGetFreeHeapSize());	

}
#endif
