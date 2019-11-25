#include "s907x.h"
#include "string.h"
#include "lwip_conf.h" 
#include "wlan_test.h"
#include "dhcps.h"
 
#if M_AT_TEST
 
static u8 current_wlan_mode = 0;
static sema_t scan_sema = NULL;
static int  dhcp_server_init = 0;
static monitor_t test_monitor_data;
static	struct ip_addr auto_reconnect_ipaddr;
static	struct ip_addr auto_reconnect_netmask;
static	struct ip_addr auto_reconnect_gw;
       
     
static int wlan_smode_hdl(char  **argv, int argc)
{
    int ret;
    u8  mode;
    if(argc < 2) {
        return HAL_ERROR;
    } 
    current_wlan_mode = mode = atoi(argv[1]);
    if(mode >= S907X_MODE_MAX) {
        return HAL_ERROR;
    } 
		s907x_wlan_off(); 
		wl_os_mdelay(50); 
		ret = s907x_wlan_on(mode);    

		HAL_TEST_DBG("set wlan mode = %d\n", current_wlan_mode);
    return ret;
}



static int wlan_gmode_hdl(char  **argv, int argc)
{
	u8 _mode;

    if(argc < 1) {
        return HAL_ERROR;
    }

	HAL_TEST_DBG("get wlan mode = %d\n", current_wlan_mode);
	return HAL_OK;
}

static int wlan_smac_hdl(char  **argv, int argc)
{
	u8 id; 
	u8 mac[6];

	if(argc < 3) {
		return HAL_ERROR;
	}
	id = atoi(argv[1]);
	if(id > S907X_DEV1_ID) {
		id = S907X_DEV1_ID;
	}
	sscanf(argv[2], MAC_FMT, &mac[0], &mac[1], &mac[2], &mac[3],&mac[4],&mac[5]);

	return s907x_wlan_set_mac_address(id, mac);
}

static int wlan_gmac_hdl(char  **argv, int argc)
{
	u8 id; 
	u8 mac[6];
	int ret;

	if(argc < 2) {
		return HAL_ERROR;
	}
	id = atoi(argv[1]);
	ret = s907x_wlan_get_mac_address(id, mac);
	if(!ret) {
		HAL_TEST_DBG("mac = "MAC_FMT"\n", mac[0], mac[1], mac[2], mac[3],mac[4],mac[5]);
	}

	return ret;
}

static int wlan_sphymode_hdl(char  **argv, int argc)
{
	u8 phy_mode; 
	int ret;

	if(argc < 2) {
		return HAL_ERROR;
	}
	phy_mode = atoi(argv[1]);

	ret = s907x_wlan_set_phy_mode(phy_mode);
	if(!ret) {
		HAL_TEST_DBG("set phy mode = %d\n", phy_mode);
	}

	return ret;
}

static int wlan_gphymode_hdl(char  **argv, int argc)
{
	u8 phy_mode; 

	if(argc < 1) {
		return HAL_ERROR;
	}
	phy_mode = s907x_wlan_get_phy_mode();
	HAL_TEST_DBG("get phy mode = %d\n", phy_mode);	
	
	return HAL_OK;
}

static int wlan_scountry_hdl(char  **argv, int argc)
{
	int country;
	int ret;

	if(argc < 2) {
		return HAL_ERROR;
	}
	country = atoi(argv[1]);

	ret = s907x_wlan_set_country(country);
	
	HAL_TEST_DBG("set wlan country = %d\n", country);

	return ret;
}

static int wlan_gcountry_hdl(char  **argv, int argc)
{
	int country;

	if(argc < 1) {
		return HAL_ERROR;
	}

	country = s907x_wlan_get_country();
	
	HAL_TEST_DBG("current wlan country = %d\n", country);

	return HAL_OK;
}



static void s907x_wlan_scan_cb(s907x_scan_result_t *presult)
{
	wlan_target_infor_t *target;
	ASSERT(presult);
	
	target = (wlan_target_infor_t*)presult->context; 
	
	if(presult->max_nums < 1) {
		HAL_TEST_DBG("scan result : no network!\n");	
		if(target) {
			wl_send_sema((sema_t*)&target->sema);
		} else if(scan_sema) {
			wl_send_sema(&scan_sema);
		} 
		return;
	} 
    if(presult->id == 0) {
        HAL_TEST_DBG("-----------------------scan result------------------\n");
    } 
	HAL_TEST_DBG("scan id %02d max ap %02d bssid "MAC_FMT" channel %02d security = %d rssi %02d ssid %s \n", presult->id, presult->max_nums, presult->scan_info.bssid[0], 
	presult->scan_info.bssid[1],presult->scan_info.bssid[2],presult->scan_info.bssid[3],presult->scan_info.bssid[4],presult->scan_info.bssid[5], 
    presult->scan_info.channel, presult->scan_info.security,  presult->scan_info.rssi, presult->scan_info.ssid);
	
    if(target && presult->scan_info.ssid_len && !strncmp(target->ssid, presult->scan_info.ssid, presult->scan_info.ssid_len)) {
		target->match =  TRUE;
        target->channel = presult->scan_info.channel;
        target->security = presult->scan_info.security;
    }
	if(presult->id == presult->max_nums - 1) {
		if(target) {
			wl_send_sema((sema_t*)&target->sema);
		} else if(scan_sema) {
			wl_send_sema(&scan_sema);
		}
	}
}



static int wlan_scan_hdl(char  **argv, int argc)
{
	int ret;
    s907x_scan_result_t result;
    s907x_scan_result_t *presult;

	if(argc < 1) {
		return HAL_ERROR;
	}

	wl_init_sema(&scan_sema, 0, sema_binary);
    if(argc == 1) {
        ret = s907x_wlan_scan(s907x_wlan_scan_cb, S907X_DEFAULT_SCAN_AP_NUMS, NULL);
        if(ret) {
            goto exit;
        }
    } else if(argc == 2){
        ret = s907x_wlan_scan_ssid(argv[1], strlen(argv[1]), &result);
        if(!ret && result.max_nums) {
            presult = &result;
            HAL_TEST_DBG("-----------------------scan result------------------\n");
            HAL_TEST_DBG("scan id %d max ap %d\n", presult->id, presult->max_nums);	
            HAL_TEST_DBG("ssid %s bssid "MAC_FMT" channel %d rssi  %d security = %d\n", presult->scan_info.ssid, presult->scan_info.bssid[0], 
            presult->scan_info.bssid[1],presult->scan_info.bssid[2],presult->scan_info.bssid[3],presult->scan_info.bssid[4],presult->scan_info.bssid[5], 
            presult->scan_info.channel, presult->scan_info.rssi, presult->scan_info.security);           
        }
    }
	wl_wait_sema(&scan_sema, portMAX_DELAY);	
exit:
	wl_free_sema(&scan_sema);	
	scan_sema = NULL;
	return ret;
}


 
static int wlan_connect_with_scan(char *ssid, s907x_sta_init_t *init)
{
	int ret;
	wlan_target_infor_t target;

	ASSERT(init);
	ASSERT(ssid);

	target.match = FALSE;
	strcpy(target.ssid, ssid);
	wl_init_sema(&target.sema, 0, sema_binary);
	ret = s907x_wlan_scan(s907x_wlan_scan_cb, S907X_DEFAULT_SCAN_AP_NUMS, &target);
	if(ret) {
		wl_free_sema(&target.sema);
		return ret;
	}
	wl_wait_sema(&target.sema, portMAX_DELAY);		
	wl_free_sema(&target.sema);
	if(target.match) {
		//set security
		init->security = target.security;
		//set connection mode
		init->conn.mode	=  CONN_MODE_BLOCKING;
		init->conn.blocking_timeout = S907X_DEFAULT_CONN_TO;

		ret = s907x_wlan_start_sta(init);
	} else {
		ret = HAL_ERROR;
	}
	return ret;
} 

static int wlan_connect_without_scan(s907x_security_e security, s907x_sta_init_t *init)
{
	int ret;

	ASSERT(init);

    //set security
	init->security = security;
	//set connection mode
	init->conn.mode	=  CONN_MODE_BLOCKING;
	init->conn.blocking_timeout = S907X_DEFAULT_CONN_TO;

	ret = s907x_wlan_start_sta(init);

	return ret;
}

//autoreconnect set static ip callback
static void autoreconnect_set_staicip(void)
{
    netif_set_addr(LwIP_GetNetif(0),&auto_reconnect_ipaddr, &auto_reconnect_netmask, &auto_reconnect_gw);    
}

//autoreconnect set static ip config
void wlan_config_autoreconnect_staticip(struct ip_addr *ipaddr, struct ip_addr *netmask, struct ip_addr *gw)
{
    auto_reconnect_ipaddr.addr  = ipaddr->addr;
    auto_reconnect_netmask.addr = netmask->addr;
    auto_reconnect_gw.addr      = gw->addr;
    lwip_set_autoreconnect_cb(autoreconnect_set_staicip);
}


static int wlan_sta_connect_with_ssid(char **argv, int argc)
{
	s907x_sta_init_t s907x_sta_init;
	
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw;
	struct ip_addr dns;
	int    ret;
	int    security;
    u8     phymode;
    u8     country_code; 
    u8     mode;
    int    val;
	u8 mac[6];
	u8 host_name[32];

	if(argc < 4) {
		return HAL_ERROR;		
	}
	mode = s907x_wlan_get_mode();

	s907x_wlan_off(); 
	wl_os_mdelay(50);
	s907x_wlan_on(mode);
	
	memset(host_name,0,32);
	s907x_wlan_get_mac_address(S907X_DEV0_ID,mac);
	sprintf(host_name, "sta_%02X%02X%02X",mac[3],mac[4],mac[5]);
	lwip_set_hostname(LwIP_GetNetif(S907X_DEV0_ID),host_name);
	

	memset(&s907x_sta_init, 0, sizeof(s907x_sta_init));
	//network para
	s907x_sta_init.ssid			=  argv[2];
 	s907x_sta_init.ssid_len		=  strlen(argv[2]);
	s907x_sta_init.password		=  argv[3];
	s907x_sta_init.password_len	=  strlen(argv[3]);
	//only have ssid password, try to scan target ssid
	if(argc == 4) {
		ret = wlan_connect_with_scan(argv[2], &s907x_sta_init);
	} else if(argc == 5) {
		security = atoi(argv[4]);
		if(security == -1 || security == S907X_SECURITY_AUTO) {
            ret = wlan_connect_with_scan(argv[2], &s907x_sta_init);    
			return ret;
		}  
		ret = wlan_connect_without_scan((s907x_security_e)security, &s907x_sta_init);
	} else if(argc >= 6) { 
        //set phy mode  
        val = atoi(argv[5]);
        country_code = HI_UINT8((u8)val);
        phymode      = LO_UINT8((u8)val);
        if(country_code >= S907X_COUNTRY_MAX) {
            country_code = S907X_COUNTRY_CN;
        }
        s907x_wlan_set_country(country_code);
        if(phymode > 2) {
            phymode = 2;
        }
        s907x_wlan_set_phy_mode(phymode);
        //security
		security = atoi(argv[4]);
        if(argc >= 9) {
            //set auto conns 
            s907x_sta_init.auto_conn.enable         =  atoi(argv[6]);
            s907x_sta_init.auto_conn.cnt            =  atoi(argv[7]);
            s907x_sta_init.auto_conn.interval_s     =  atoi(argv[8]);
            if(argc >= 12) {
                s907x_sta_init.auto_conn.use_staticip  =  1;
                ipaddr.addr  = inet_addr(argv[9]);
                netmask.addr = inet_addr(argv[10]);
                gw.addr      = inet_addr(argv[11]);
                wlan_config_autoreconnect_staticip(&ipaddr, &netmask, &gw);
            } else {
                s907x_sta_init.auto_conn.use_staticip = 0;
            }
        }
        //connect
        if(security == -1 || security == S907X_SECURITY_AUTO) { 
            ret = wlan_connect_with_scan(argv[2], &s907x_sta_init);
        } else { 			
            ret = wlan_connect_without_scan((s907x_security_e)security, &s907x_sta_init);
        }
	}
	if(!ret) {
        //set static ip
        if(argc >= 12) {

            netif_set_addr(LwIP_GetNetif(0),&ipaddr, &netmask, &gw);
           
            #if LWIP_DNS	
            if(argc == 13) {
                dns.addr = inet_addr(argv[12]);
                LwIP_SetDNS(&dns);
            }
            #endif
            HAL_TEST_DBG("set static ip success: \n");
            HAL_TEST_DBG("gate:%s \n",inet_ntoa(gw.addr));
            HAL_TEST_DBG("ip:%s \n",inet_ntoa(ipaddr.addr));
            HAL_TEST_DBG("mask:%s \n",inet_ntoa(netmask.addr));
        }
		else
			dhcpc_start(0, 0);
    }
	return ret;
}

static int wlan_sta_connect_with_bssid(char **argv, int argc)
{

	return HAL_OK;
}

static int wlan_sta_disconnect(char **argv, int argc)
{
	int ret;

	ret = s907x_wlan_stop_sta();

	return ret;
}
 



static int wlan_sta_get_linkinfo(char **argv, int argc)
{
	int ret;
	s907x_link_info_t info;

	ret = s907x_wlan_get_link_infor(&info);
	if(!info.is_connected) {
		HAL_TEST_DBG("link is not connected\n");
	} else {
		HAL_TEST_DBG("link is connected\n");
		HAL_TEST_DBG("ssid %s bssid "MAC_FMT" channel %d rssi  %d\n", info.ssid, info.bssid[0], 
		info.bssid[1],info.bssid[2],info.bssid[3],info.bssid[4],info.bssid[5], 
		info.channel, info.rssi);
	}
	return ret;
}
    
static int wlan_sta_get_ip(char **argv, int argc)
{
	u8 gw[4],ip[4],mask[4];
    int ret;
	struct netif *pnetif;

	s907x_link_info_t info;

	ret = s907x_wlan_get_link_infor(&info);
    if(ret) {
        return ret;
    }
	if(!info.is_connected) {
		HAL_TEST_DBG("link is not connected\n");
        return HAL_OK;
	}
  
    pnetif = LwIP_GetNetif(0);
    memcpy(gw, LwIP_GetGW(pnetif), sizeof(struct ip_addr));
    memcpy(ip, LwIP_GetIP(pnetif), sizeof(struct ip_addr));
    memcpy(mask, LwIP_GetMASK(pnetif), sizeof(struct ip_addr));

    HAL_TEST_DBG("get_ip_status:\n");
    HAL_TEST_DBG("gate:%s \n",inet_ntoa(gw));
    HAL_TEST_DBG("ip:%s \n",inet_ntoa(ip));
    HAL_TEST_DBG("mask:%s \n",inet_ntoa(mask));

	return HAL_OK;
}
 
static int wlan_sta_hdl(char **argv, int argc)
{
	int ret = HAL_ERROR;
	int cmd;
	u8  mode;
	if(argc < 2) {
		return HAL_ERROR;
	}
	mode = s907x_wlan_get_mode();
	if(mode != S907X_MODE_STA && mode != S907X_MODE_STA_AP){
		HAL_TEST_DBG("run sta in error mode = %d\n", mode);
		return ret;
	}
 
	cmd = atoi(argv[1]);
	switch(cmd)
	{
		case STA_CMD_CONNECT_WITH_SSID:
			ret = wlan_sta_connect_with_ssid(argv, argc);
			break;
		case STA_CMD_CONNECT_WITH_BSSID:
			ret = wlan_sta_connect_with_bssid(argv, argc);
			break;
		case STA_CMD_DISCONNECT:
			ret = wlan_sta_disconnect(argv, argc);
			break;
		case STA_CMD_GET_LINKSTATUS:	
			ret = wlan_sta_get_linkinfo(argv, argc);
			break;
		case STA_CMD_GET_IP:
			ret = wlan_sta_get_ip(argv, argc);
			break;
		default:

			break;
	}	

	return ret;	
}
 
static int wlan_checkip_valid(struct ip_addr *ip1, struct ip_addr *ip2, struct ip_addr *ip)
{
    if(!ip1 || !ip2 || !ip) {
        return 0;
    }
    return (ip4_addr1(ip1) == ip4_addr1(ip)) && (ip4_addr2(ip1) == ip4_addr2(ip)) && (ip4_addr3(ip1) == ip4_addr3(ip)) &&
           (ip4_addr1(ip2) == ip4_addr1(ip)) && (ip4_addr2(ip2) == ip4_addr2(ip)) && (ip4_addr3(ip2) == ip4_addr3(ip)) && 
           (ip4_addr4(ip1) <= ip4_addr4(ip)) && (ip4_addr4(ip) <= ip4_addr4(ip2));
}

//AT+WLAN=ap,0,ssid,password,security,channel,[,hidden ap][,static_ip]
static int wlan_ap_start(char **argv, int argc)
{
	int ret;
	int security;
	int id;
	u8  mode;
    int val;
    int phymode;    
    u8  country_code;
    ip_addr_t  first_ip;
    ip_addr_t  last_ip;
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw;	    
	s907x_ap_init_t ap_init;
	if(argc < 6) {
		return HAL_ERROR;
	}
	 
	mode = s907x_wlan_get_mode();
    if(mode == S907X_MODE_STA_AP) {
        s907x_wlan_on(mode); 
    } else {
        s907x_wlan_off(); 
        wl_os_mdelay(50);
        s907x_wlan_on(mode);   
    }
    memset(&ap_init, 0, sizeof(ap_init));

	ap_init.ssid 			= 	argv[2];
	ap_init.ssid_len		= 	strlen(argv[2]);
	ap_init.password		= 	argv[3];
	ap_init.password_len	= 	strlen(argv[3]);
	security 				= 	atoi(argv[4]);

	if( security != S907X_SECURITY_NONE) { //set default
		ap_init.security = S907X_SECURITY_WPA2_AES;
	} else {
		ap_init.security = S907X_SECURITY_NONE;
	}
    HAL_TEST_DBG("s907x ap only support S907X_SECURITY_NONE or S907X_SECURITY_WPA2_AES encrypt\n");
	ap_init.channel			= 	atoi(argv[5]);
	if(ap_init.channel > 14) {
		ap_init.channel = 1;
	}
	if(argc > 6) {
		ap_init.is_hidded_ssid 	= atoi(argv[6]) > 0 ? 1 : 0;	
	}
	if(mode != S907X_MODE_STA_AP) {
		id = S907X_DEV0_ID;
	} else {
		id = S907X_DEV1_ID;
	}
    if(argc > 7) {
        val = atoi(argv[7]);
        country_code = HI_UINT8((u8)val);
        phymode      = LO_UINT8((u8)val);
        if(country_code >= S907X_COUNTRY_MAX) {
            country_code = S907X_COUNTRY_CN;
        }
		wl_os_mdelay(50);
        s907x_wlan_set_country(country_code);
		wl_os_mdelay(1);
        if(phymode > 2) {
            phymode = 2;
        }
        s907x_wlan_set_phy_mode(phymode);
    }
	ret = s907x_wlan_start_ap(&ap_init);
	if(!ret) {
		if(argc > 8) {
            ret = dhcps_get_iprange(&first_ip, &last_ip);
            if(ret) {
                return ret;
            }
			HAL_TEST_DBG("set ap loacal ipaddress\n");
			ipaddr.addr = inet_addr(argv[8]);
			netmask.addr = inet_addr(argv[9]);
			gw.addr = inet_addr(argv[10]);
            if(!first_ip.addr && !last_ip.addr) {
                //not set ok
            } else {
                //check ip valid
                if(!wlan_checkip_valid(&first_ip, &last_ip, &ipaddr)) {
                    HAL_TEST_DBG("set ipaddr %04x invalid with ip range %04x ~ %04x, set ip range first!\n", ipaddr.addr, first_ip.addr, last_ip.addr );
                }
            }			 
			netif_set_addr(LwIP_GetNetif(id), &ipaddr, &netmask,&gw);
		} else {
            IP4_ADDR(&ipaddr, AP_IP_ADDR0 ,AP_IP_ADDR1 , AP_IP_ADDR2 , AP_IP_ADDR3 );
            IP4_ADDR(&netmask, AP_NETMASK_ADDR0, AP_NETMASK_ADDR1, AP_NETMASK_ADDR2, AP_NETMASK_ADDR3);
            IP4_ADDR(&gw, AP_GW_ADDR0, AP_GW_ADDR1, AP_GW_ADDR2, AP_GW_ADDR3);
			 
			netif_set_addr(LwIP_GetNetif(id), &ipaddr, &netmask,&gw);
		}
        dhcps_init(LwIP_GetNetif(id));
	}
	
	return ret;	
}

static int wlan_ap_stop(char **argv, int argc)
{
	int ret;    

	ret = s907x_wlan_stop_ap();
	if(!ret)
		dhcps_deinit();

	return ret;	
}
 
static int wlan_ap_modify_ssid(char **argv, int argc)
{
	return 0;	
}
 
static int wlan_ap_get_info(char **argv, int argc)
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

static int wlan_ap_get_ipinfo(char **argv, int argc)
{
	u8 gw[4],ip[4],mask[4];
	struct netif *pnetif;
	int id = -1;
	u8  mode;

	mode = s907x_wlan_get_mode();
	if(mode != S907X_MODE_STA_AP) {
		id = S907X_DEV0_ID;
	} else {
		id = S907X_DEV1_ID;
	}

	pnetif = LwIP_GetNetif(id);
	memcpy(gw, LwIP_GetGW(pnetif), sizeof(struct ip_addr));
	memcpy(ip, LwIP_GetIP(pnetif), sizeof(struct ip_addr));
	memcpy(mask, LwIP_GetMASK(pnetif), sizeof(struct ip_addr));

	HAL_TEST_DBG("get_ip_status:\n");
	HAL_TEST_DBG("gate:%s \n",inet_ntoa(gw));
	HAL_TEST_DBG("ip:%s \n",inet_ntoa(ip));
	HAL_TEST_DBG("mask:%s \n",inet_ntoa(mask));
	
	return HAL_OK;
}

static int wlan_ap_dhcps_iprange(char **argv, int argc)
{
    ip_addr_t  first_ip;
    ip_addr_t  last_ip;
    u8  mode;
    int ret = HAL_ERROR;

 	if(argc < 3) {
		return HAL_ERROR;
	}       
    mode = atoi(argv[2]);
    if(mode == 0) {
        ret = dhcps_get_iprange(&first_ip, &last_ip);
        if(!ret) {      
            HAL_TEST_DBG("first_ip %x last_ip %x\n", first_ip.addr, last_ip.addr);
        }   
    } else if(argc >= 5){   
        first_ip.addr = inet_addr(argv[3]);
        last_ip.addr = inet_addr(argv[4]);
        ret = dhcps_set_iprange(&first_ip, &last_ip);
    }
    
    return ret;
}
 
 
static int wlan_ap_hdl(char **argv, int argc)
{	
	int ret;
	int cmd;
	u8 mode;
    
	if(argc < 2) {
		return HAL_ERROR;
	}
	mode = s907x_wlan_get_mode();
	if(mode != S907X_MODE_AP && mode != S907X_MODE_STA_AP){
		HAL_TEST_DBG("run ap in error mode = %d\n", mode);
		return HAL_ERROR;
	}
	cmd = atoi(argv[1]);	
	switch(cmd)
	{
		case AP_CMD_START:
			ret = wlan_ap_start(argv, argc);
			break;
		case AP_CMD_STOP:
			ret = wlan_ap_stop(argv, argc);
			break;
		case AP_CMD_MODIFY_SSID:
			ret = wlan_ap_modify_ssid(argv, argc);
			break;
		case AP_CMD_GET_STA_INFO:
			ret = wlan_ap_get_info(argv, argc);
			break;
		case AP_CMD_GET_IP:	
			ret = wlan_ap_get_ipinfo(argv, argc);
			break;
        case AP_CMD_DHCPS_IPRANGE:
            ret = wlan_ap_dhcps_iprange(argv, argc);
            break;
		default:

			break;
	}	
	return ret;
}

static void s907x_event_cb_scan_down(s907x_event_data *s907x_data, void *context)
{
	HAL_TEST_DBG("s907x event scan down\n");	


}

static void s907x_event_sta_connected(s907x_event_data *s907x_data, void *context)
{
	HAL_TEST_DBG("s907x event sta connected \n");
	HAL_TEST_DBG("ssid %s bssid "MAC_FMT" channel %d\n", s907x_data->esp_data.connected.ssid, 
	s907x_data->esp_data.connected.bssid[0], s907x_data->esp_data.connected.bssid[1], s907x_data->esp_data.connected.bssid[2], 
	s907x_data->esp_data.connected.bssid[3], s907x_data->esp_data.connected.bssid[4], s907x_data->esp_data.connected.bssid[5], s907x_data->esp_data.connected.channel);
}


static void s907x_event_sta_disconnected(s907x_event_data *s907x_data, void *context)
{	
	HAL_TEST_DBG("s907x event sta disconnected \n");
	HAL_TEST_DBG("reason = %d ssid %s bssid "MAC_FMT" \n", s907x_data->esp_data.disconnected.reason, s907x_data->esp_data.connected.ssid, 
	s907x_data->esp_data.connected.bssid[0], s907x_data->esp_data.connected.bssid[1], s907x_data->esp_data.connected.bssid[2], 
	s907x_data->esp_data.connected.bssid[3], s907x_data->esp_data.connected.bssid[4], s907x_data->esp_data.connected.bssid[5]);
}
 
static void s907x_event_sta_authchange(s907x_event_data *s907x_data, void *context)
{
	HAL_TEST_DBG("s907x event sta authchange \n");
}

static void s907x_event_ap_sta_connected(s907x_event_data *s907x_data, void *context)
{
	HAL_TEST_DBG("s907x event ap sta connected \n");
	HAL_TEST_DBG("aid = %d "MAC_FMT "\n", s907x_data->esp_data.sta_connected.aid, s907x_data->esp_data.sta_connected.mac[0], s907x_data->esp_data.sta_connected.mac[1], 
	s907x_data->esp_data.sta_connected.mac[2], s907x_data->esp_data.sta_connected.mac[3],s907x_data->esp_data.sta_connected.mac[4], s907x_data->esp_data.sta_connected.mac[5]);
}

static void s907x_event_ap_sta_disconnected(s907x_event_data *s907x_data, void *context)
{
	HAL_TEST_DBG("s907x event ap sta diconnected \n");
	HAL_TEST_DBG("aid = %d "MAC_FMT "\n", s907x_data->esp_data.sta_disconnected.aid, s907x_data->esp_data.sta_disconnected.mac[0], s907x_data->esp_data.sta_disconnected.mac[1], 
	s907x_data->esp_data.sta_disconnected.mac[2], s907x_data->esp_data.sta_disconnected.mac[3],s907x_data->esp_data.sta_disconnected.mac[4], s907x_data->esp_data.sta_disconnected.mac[5]);
}

static void s907x_event_ap_probereq_received(s907x_event_data *s907x_data, void *context)
{
	HAL_TEST_DBG("s907x event ap probe req received\n");
	HAL_TEST_DBG("rssi = %d "MAC_FMT "\n", s907x_data->esp_data.ap_probereqrecved.rssi, s907x_data->esp_data.ap_probereqrecved.mac[0], s907x_data->esp_data.ap_probereqrecved.mac[1], 
	s907x_data->esp_data.ap_probereqrecved.mac[2], s907x_data->esp_data.ap_probereqrecved.mac[3],s907x_data->esp_data.ap_probereqrecved.mac[4], s907x_data->esp_data.ap_probereqrecved.mac[5]);

} 
 
static int wlan_event_hdl(char **argv, int argc)
{
	int event;
	int opt;
    int all = FALSE;
	s907x_event_callback  event_cb;

	if(argc < 3) {
		return HAL_ERROR;
	}
	
	event = atoi(argv[1]);
	opt   = atoi(argv[2]);
	

 
	switch(event)
	{
		case S907X_EVENT_STAMODE_SCAN_DONE:
			event_cb = s907x_event_cb_scan_down;
			break;
		case S907X_EVENT_STAMODE_CONNECTED:
			event_cb = s907x_event_sta_connected;
			break;
		case S907X_EVENT_STAMODE_DISCONNECTED:
			event_cb = s907x_event_sta_disconnected;
			break;
		case S907X_EVENT_STAMODE_AUTHCHANGE:
			event_cb = s907x_event_sta_authchange;
			break;
		case S907X_EVENT_APMODE_STA_CONNECTED:
			event_cb = s907x_event_ap_sta_connected;
			break;
		case S907X_EVENT_APMODE_STA_DISCONNECTED:
			event_cb = s907x_event_ap_sta_disconnected;
			break;
		case S907X_EVENT_APMODE_PROBE_REG_RECEIVED:
			event_cb = s907x_event_ap_probereq_received;		
			break;
		default:
            if(event == S907X_EVENT_MAX) {
                all = TRUE;
            } else {
                HAL_TEST_DBG("s907x event value error %d\n", event);
                return HAL_ERROR;
            }
            break;
	}
    if(all) {
        if(opt >= 1) {
            s907x_wlan_event_reg(S907X_EVENT_STAMODE_SCAN_DONE, s907x_event_cb_scan_down, NULL);
            s907x_wlan_event_reg(S907X_EVENT_STAMODE_CONNECTED, s907x_event_sta_connected, NULL);
            s907x_wlan_event_reg(S907X_EVENT_STAMODE_DISCONNECTED, s907x_event_sta_disconnected, NULL);
            s907x_wlan_event_reg(S907X_EVENT_STAMODE_AUTHCHANGE, s907x_event_sta_authchange, NULL);
            s907x_wlan_event_reg(S907X_EVENT_APMODE_STA_CONNECTED, s907x_event_ap_sta_connected, NULL);
            s907x_wlan_event_reg(S907X_EVENT_APMODE_STA_CONNECTED, s907x_event_ap_sta_disconnected, NULL);
            s907x_wlan_event_reg(S907X_EVENT_APMODE_PROBE_REG_RECEIVED, s907x_event_ap_probereq_received, NULL);
        } else {
            s907x_wlan_event_unreg(S907X_EVENT_STAMODE_SCAN_DONE, s907x_event_cb_scan_down);
            s907x_wlan_event_unreg(S907X_EVENT_STAMODE_CONNECTED, s907x_event_sta_connected);
            s907x_wlan_event_unreg(S907X_EVENT_STAMODE_DISCONNECTED, s907x_event_sta_disconnected);
            s907x_wlan_event_unreg(S907X_EVENT_STAMODE_AUTHCHANGE, s907x_event_sta_authchange);
            s907x_wlan_event_unreg(S907X_EVENT_APMODE_STA_CONNECTED, s907x_event_ap_sta_connected);
            s907x_wlan_event_unreg(S907X_EVENT_APMODE_STA_CONNECTED, s907x_event_ap_sta_disconnected);
            s907x_wlan_event_unreg(S907X_EVENT_APMODE_PROBE_REG_RECEIVED, s907x_event_ap_probereq_received);  
        }
    } else {
        if(opt >= 1)
            s907x_wlan_event_reg((s907x_event_e)event, event_cb, NULL);
        else
            s907x_wlan_event_unreg((s907x_event_e)event, event_cb);
    }
	return HAL_OK;
}






void monitor_callback(uint8_t*data, int len)
{
    u8 type;
    u8 subtype;

    type    = GET_WLAN_FRAME_TYPE(data);
    subtype = GET_WLAN_FRAME_SUBTYPE(data);

    if((type == S907X_WLAN_TYPE_MGT)&&(subtype == S907X_WLAN_SUBTYPE_BEACON))
        HAL_TEST_DBG("S907X_WLAN_RX_BEACON\n");
    else if((type == S907X_WLAN_TYPE_MGT)&&(subtype == S907X_WLAN_SUBTYPE_PROBEREQ))
        HAL_TEST_DBG("S907X_WLAN_RX_PROBE_REQ\n");
    else if((type == S907X_WLAN_TYPE_MGT)&&(subtype == S907X_WLAN_SUBTYPE_PROBERSP))
        HAL_TEST_DBG("S907X_WLAN_RX_PROBE_RES\n");
    else if((type == S907X_WLAN_TYPE_MGT)&&(subtype == S907X_WLAN_SUBTYPE_ACTION))
        HAL_TEST_DBG("S907X_WLAN_RX_ACTION\n");
    else if(type == S907X_WLAN_TYPE_DATA)
        HAL_TEST_DBG("S907X_WLAN_RX_DATA\n");
    else if(type == S907X_WLAN_TYPE_MGT)
        HAL_TEST_DBG("S907X_WLAN_RX_MANAGEMENT\n");
    
    HAL_DBG_ARRARY((unsigned char *)data, len, ARY_U8, 16);
}



int wlan_test_monitor(void)
{
	s907x_monitor_t monitor;

	if(test_monitor_data.callback != NULL) {	
		//set monitor mode
		monitor.mode = m_all;
		//set coustom context(miro call back)
		monitor.coustom_context = (void*)&test_monitor_data;
		s907x_wlan_start_monitor(&monitor);
	
	} else {
		HAL_TEST_DBG("ERR: no func callback!\n");
		return HAL_ERROR;
	}

	return HAL_OK;
}



static int wlan_monitor_hdl(char  **argv, int argc)
{
	int ret = 0;
	int cmd;
    u8 filter;
    u8 channel;

    if(argc < 2) {
        return HAL_ERROR;
    }
	cmd = atoi(argv[1]);

	
	switch(cmd)
	{
		case TEST_MONITOR_START:
            s907x_wlan_off();
            wl_os_mdelay(50);	
            ret = s907x_wlan_on(S907X_MODE_MONITOR);
            
            if (ret < 0){
                HAL_TEST_DBG("\n\rERROR: Wifi on failed!");
                return HAL_ERROR;
            }
			test_monitor_data.callback = monitor_callback;
            if(argc >= 3) {
                channel = atoi(argv[2]);
                if(channel > 14) {
                    channel = 1;
                }
                ret = s907x_wlan_set_channel(S907X_DEV0_ID, (u8)channel);
                if(ret) {
                    return HAL_ERROR;
                }
                filter = S907X_WLAN_RX_ALL;    
                if(argc >= 4) {
                    filter = atoi(argv[3]);
                    if(filter >= S907X_WLAN_RX_MAX) {
                        filter = S907X_WLAN_RX_ALL;
                    }
                }
                test_monitor_data.m_type = filter;
                
            } else {
                s907x_wlan_set_channel(S907X_DEV0_ID, 1);
                test_monitor_data.m_type = S907X_WLAN_RX_ALL;
            }
			ret = wlan_test_monitor();
			break;
		case TEST_MONITOR_CHANNEL:
            channel = atoi(argv[2]);
            if(channel > 14) {
                channel = 1;
            }
			ret = s907x_wlan_set_channel(S907X_DEV0_ID, (u8)channel);
			break;
		case TEST_MONITOR_STOP:
			memset(&test_monitor_data, S907X_DEV0_ID, sizeof(monitor_t));
			ret = s907x_wlan_stop_monitor();
			break;
	}
		
	
	return ret;
}
     
static int wlan_switch2sta(char  **argv, int argc)
{
	int ret;
	int dhcpc_state;
	u8 mac[6],host_name[32];
	s907x_wlan_off(); 
	wl_os_mdelay(50);
	ret = s907x_wlan_on(S907X_MODE_STA);  
	
	memset(host_name,0,32);
	s907x_wlan_get_mac_address(S907X_DEV0_ID,mac);
	sprintf(host_name, "sta_%02X%02X%02X",mac[3],mac[4],mac[5]);
	lwip_set_hostname(LwIP_GetNetif(S907X_DEV0_ID),host_name);
	

	ret = wlan_sta_connect_with_ssid(argv, argc);
	  
	if(!ret) {	
		dhcpc_state = dhcpc_start(0, 0);
		if(dhcpc_state == 2) {
			ret = HAL_OK;
		} else {
			ret = HAL_ERROR;
		}
	}
	return ret;
}


static int wlan_switch2ap()
{
	int ret;
	int security;	
	s907x_ap_init_t ap_init;
   
	s907x_wlan_off(); 
	wl_os_mdelay(50);
	ret = s907x_wlan_on(S907X_MODE_AP);  

    if(dhcp_server_init) {
        dhcp_server_init = 0;
        dhcps_deinit();
    }
	ap_init.ssid 			= 	"s907x_test_ap";
	ap_init.ssid_len		= 	strlen(ap_init.ssid);
	ap_init.password		= 	"12345678";
	ap_init.password_len	= 	strlen(ap_init.password);
	security 				= 	S907X_SECURITY_WPA2_AES;
	if( security < 0 || security == S907X_SECURITY_WEP || security >= S907X_SECURITY_AUTO) { //set default
		ap_init.security = S907X_SECURITY_WPA2_AES;
	} else {
		ap_init.security = (s907x_security_e)security;
	}
	ap_init.channel			= 	1;
	ret = s907x_wlan_start_ap(&ap_init);
    if(!ret) {
        HAL_TEST_DBG("dhcp server init\n");
        if(!dhcp_server_init) {
            dhcps_init(LwIP_GetNetif(0)); 
            dhcp_server_init = 1;
        }
    }
	return ret;
} 
 
static int wlan_switch2staap(char  **argv, int argc)
{
	int ret;
	s907x_ap_init_t ap_init;
	u8 mac[6],host_name[32];
	s907x_wlan_off(); 
	wl_os_mdelay(50);
	ret = s907x_wlan_on(S907X_MODE_STA);  
	
	memset(host_name,0,32);
	s907x_wlan_get_mac_address(S907X_DEV0_ID,mac);
	sprintf(host_name, "sta_%02X%02X%02X",mac[3],mac[4],mac[5]);
	lwip_set_hostname(LwIP_GetNetif(S907X_DEV0_ID),host_name);
	

    if(dhcp_server_init) {
        dhcp_server_init = 0;
        dhcps_deinit();
    }

	ret = wlan_sta_connect_with_ssid(argv, argc);
	if(!ret) {  	
		dhcpc_start(0, 0);
	} else {
		return HAL_ERROR;
	}

	ap_init.ssid 			= 	"s907x_test_staap";
	ap_init.ssid_len		= 	strlen(ap_init.ssid);
	ap_init.password		= 	"12345678";
	ap_init.password_len	= 	strlen(ap_init.password);
	ap_init.security 		=   (s907x_security_e)S907X_SECURITY_WPA2_AES;
	ap_init.channel			= 	1;
 
	ret = s907x_wlan_start_ap(&ap_init);
    if(!ret) {
        if(!dhcp_server_init) {
            dhcps_init(LwIP_GetNetif(1)); 
            dhcp_server_init = 1;
        }
    }
	return ret;
}

static int wlan_switch2monitor()
{
	int ret;

	s907x_wlan_off(); 
	wl_os_mdelay(50);
	ret = s907x_wlan_on(S907X_MODE_MONITOR); 

	return ret;
}
 
static int wlan_switchmode_hdl(char  **argv, int argc)
{
	int ret = HAL_ERROR;
	u8 mode;
	int cnt = 0;
    int interval = 0;
	if(argc < 4) {
		return HAL_ERROR;
	}	 
	cnt = atoi(argv[1]);
	if(argc >= 5) {
        interval = atoi(argv[4]);
    } else {
        interval = 1;
    } 
	while(cnt--)
	{ 
		mode = wl_get_random32() %4;
		switch(mode) 
		{
			case 0:
				ret = wlan_switch2sta(argv, argc);		
			break;
			case 1:
				ret = wlan_switch2ap();	
			break;	
			case 2:
				ret = wlan_switch2staap(argv, argc);	
			break;
			case 3:
				ret = wlan_switch2monitor();		
			break;
			default:
				ret = HAL_ERROR;
			break;
		}
		if(ret != HAL_OK) {
            HAL_TEST_DBG("switch mode = %d failed\n", mode);
			break;
		}
        HAL_TEST_DBG("switch mode = %d success try next!\n", mode);
		wl_os_mdelay(interval*1000);
        
	}	

	return ret;
}
   

static int wlan_get_heapsize(char  **argv, int argc)
{
    u32 heapsize;

    heapsize = wl_get_freeheapsize();
    HAL_TEST_DBG("system remain size = %d\n", heapsize);
    
    return HAL_OK;
}


int wlan_connectap_hdl(void)
{
    
  

    return HAL_OK;
}

int wlan_hostname_hdl(char  **argv, int argc)
{
    int cmd;
    int id;
 
    if(argc < 3) {
        return HAL_ERROR;
    }
    cmd = atoi(argv[1]);
    id  = atoi(argv[2]);

    if(id > 1) {
        id = 1;
    }
    if(cmd == 0) {
        
    } else {
        if(argc >= 4)
            lwip_set_hostname( LwIP_GetNetif(id), argv[3]);
    }

    HAL_TEST_DBG("hostname is %s \n", lwip_get_hostname(LwIP_GetNetif(id)));
    return HAL_OK;
}

int wlan_mp_hdl(char  **argv, int argc)
{
    int id;
    int ret;
 
    if(argc < 2) {
        return HAL_ERROR;
    } 
    id = atoi(argv[1]);
    if(id == 1) {
        s907x_wlan_enter_mp();
        s907x_wlan_off(); 
        wl_os_mdelay(50); 
        ret = s907x_wlan_on(S907X_MODE_STA);    
    } else if(id == 0){
        s907x_wlan_exit_mp();  
        HAL_TEST_DBG("leave mp system reboot\n");    
        HAL_NVIC_SystemReset();
    }

    return ret;
}        
 


void wlan_test(void *context)
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

    if(!strcmp(item, "smode")) {
		ret = wlan_smode_hdl(argv, argc);                                    
    } else if(!strcmp(item, "gmode")) {
	    ret = wlan_gmode_hdl(argv, argc);   
    } else if(!strcmp(item, "smac")) {
		ret = wlan_smac_hdl(argv, argc); 
    } else if(!strcmp(item, "gmac")) {
		ret = wlan_gmac_hdl(argv, argc); 
    } else if(!strcmp(item, "sphymode")) {
		ret = wlan_sphymode_hdl(argv, argc);
    } else if(!strcmp(item, "gphymode")) {
		ret = wlan_gphymode_hdl(argv, argc);
    } else if(!strcmp(item, "scountry")) {
		ret = wlan_scountry_hdl(argv, argc);	
    } else if(!strcmp(item, "gcountry")) {
		ret = wlan_gcountry_hdl(argv, argc); 
    } else if(!strcmp(item, "scan")) {
		ret = wlan_scan_hdl(argv, argc);
    } else if(!strcmp(item, "sta")) {
		ret = wlan_sta_hdl(argv, argc);
    } else if(!strcmp(item, "ap")) {
		ret = wlan_ap_hdl(argv, argc);
    } else if(!strcmp(item, "event")) {
		ret = wlan_event_hdl(argv, argc);
    } else if(!strcmp(item, "monitor")) {
		ret = wlan_monitor_hdl(argv, argc);
    } else if(!strcmp(item, "test_modeswitch")) {
		ret = wlan_switchmode_hdl(argv, argc);
    }  else if(!strcmp(item, "test_connectap")) {
		ret = wlan_connectap_hdl();
    } else if(!strcmp(item, "heapsize")) {
        ret = wlan_get_heapsize(argv, argc);
    } else if(!strcmp(item, "hostname")) {
        ret = wlan_hostname_hdl(argv, argc);
    } else if(!strcmp(item, "mp")) {
        ret = wlan_mp_hdl(argv, argc);
    }
	if(!ret) {
		HAL_TEST_DBG("test wlan item = %s success\n", item); 
	} else {
		HAL_TEST_DBG("test wlan item = %s failed ret = %d\n", item, ret); 
	}
}

#endif
