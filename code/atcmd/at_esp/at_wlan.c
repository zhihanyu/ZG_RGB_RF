/* Copyright Statement:
 *
 * (C) 2016-2018  SCI Inc. All rights reserved.
 */

#include "s907x.h"
#include "lwip_conf.h"
#include "at_cmd.h"
#include "at_wlan.h"
#include "hal_sleep.h" 
#include "dhcps.h"
#include "lwip/ip_addr.h" 
#include "at_smartlink.h"

/* 
AT+CWMODE_CUR 设置 Wi-Fi 模式 (STA/AP/STA+AP)，不保存到 Flash
AT+CWMODE_DEF 设置 Wi-Fi 模式 (STA/AP/STA+AP)，保存到 Flash
AT+CWJAP_CUR 连接 AP，不保存到 Flash
AT+CWJAP_DEF 连接 AP，保存到 Flash
AT+CWLAPOPT 设置 AT+CWLAP 指令扫描结果的属性
AT+CWLAP 扫描附近的 AP 信息
AT+CWQAP 与 AP 断开连接
AT+CWSAP_CUR 设置 s907x SoftAP 配置，不保存到 Flash
AT+CWSAP_DEF 设置 s907x SoftAP模式 (STA/AP/STA+AP)，保存到 Flash
AT+CWLIF 获取连接到 s907x SoftAP 的 station 的信息
AT+CWDHCP_CUR 设置 DHCP，不保存到 Flash
AT+CWDHCP_DEF 设置 DHCP，保存到 Flash
AT+CWDHCPS_CUR 设置 s907x SoftAP DHCP 分配的 IP 范围，不保存到 Flash
AT+CWDHCPS_DEF 设置 s907x SoftAP DHCP 分配的 IP 范围，保存到 Flash
AT+CWAUTOCONN 设置上电时是否?自动连接 AP
AT+CIPSTAMAC_CUR 设置 s907x Station 的 MAC 地址，不保存到 Flash
AT+CIPSTAMAC_DEF 设置 s907x Station 的 MAC 地址，保存到 Flash
AT+CIPAPMAC_CUR 设置 s907x SoftAP 的 MAC 地址，不保存到 Flash
AT+CIPAPMAC_DEF 设置 s907x SoftAP 的 MAC 地址，保存到 Flash
AT+CIPSTA_CUR 设置 s907x Station 的 IP 地址，不保存到 Flash
AT+CIPSTA_DEF 设置 s907x Station 的 IP 地址，保存到 Flash
AT+CIPAP_CUR 设置 s907x SoftAP 的 IP 地址，不保存到 Flash
AT+CIPAP_DEF 设置 s907x SoftAP 的 IP 地址，保存到 Flash
AT+CWSTARTSMART 开始 SmartConfig
AT+CWSTOPSMART 停止 SmartConfig
AT+CWSTARTDISCOVER 开启可被局域网内的微信探测的模式
AT+CWSTOPDISCOVER 关闭可被局域网内的微信探测的模式
AT+WPS 设置 WPS 功能
AT+MDNS 设置 MDNS 功能
AT+CWHOSTNAME 设置 s907x Station 的主机名称
AT+CWCOUNTRY_CUR 设置 s907x 当前 WiFi 国家码
AT+CWCOUNTRY_DEF 设置 s907x 默认 WiFi 国家码
*/

#if M_AT_ESP
typedef union{
    int mask;
    struct{
        u8 ecn_en : 1;
        u8 ssid_en : 1;
        u8 rssi_en : 1;
        u8 mac_en : 1;
        u8 ch_en : 1;
        u8 freq_offset_en : 1;
        u8 freq_cal_en : 1;
        u8 pairwise_cipher_en : 1;
        
        u8 group_cipher_en : 1;
        u8 bgn_en : 1;
        u8 wps_en : 1;
        u32 rsvd : 21;
    }bit;
}print_mask;

struct ap_sta_info{
    struct ip_addr ip;
    u8 bssid[6];
};

typedef enum {
    ATWLAN_SAP_OPEN         = 0,
    ATWLAN_SAP_WPA_PSK      = 2,
    ATWLAN_SAP_WPA2_PSK     = 3,
    ATWLAN_SAP_WPA_WPA2_PSK = 4,
}atwlan_sap_enc_e;

typedef enum {
    ATWLAN_JAP_ERR_OK       = 0,
    ATWLAN_JAP_ERR_TIMEOUT  = 1,
    ATWLAN_JAP_ERR_PWD_FAIL = 2,
    ATWLAN_JAP_ERR_NO_AP    = 3,
    ATWLAN_JAP_ERR_CON_FAIL = 4,
}atwlan_jap_err_e;
 
#define VALID_SAP_ENC_TYPE(enc)             ((ATWLAN_SAP_OPEN == (enc)) || (ATWLAN_SAP_WPA_PSK <= (enc) && (enc) <= ATWLAN_SAP_WPA_WPA2_PSK))
#define MAC_ADDR_IS_ZERO(hwaddr)            ((hwaddr[0] == 0) && (hwaddr[1] == 0) && (hwaddr[2] == 0) && (hwaddr[3] == 0) && (hwaddr[4] == 0) && (hwaddr[5] == 0))
#define DURING_IP_POOL(start, end, new_ip)  ((start <= new_ip) && (new_ip <= end))
#define ZERO_IP_ADDR                        "0.0.0.0"
static wlan_target_infor_t scan_target;
static print_mask scan_print_mask;
static struct ap_sta_info sta_info[AT_WLAN_MAX_STA];
static u8 sta_connect_to_ap = 0;
static u8 g_atwlan_dhcps_init = 0;
static u8 g_atwlan_dhcp_stat = (AT_STA_DHCP_EN|AT_AP_DHCP_EN);
static atwlan_dhcps_lease_pool_t g_atwlan_dhcps_lease;
static atwlan_if_addr_cfg_t g_atwlan_sta_if;
static atwlan_if_addr_cfg_t g_atwlan_ap_if;
static u8 g_atwlan_system_ready = 0;

u8 g_atwlan_sta_ip_static = 0;
atwlan_sap_cfg_t g_atwlan_cur_ap_cfg;

extern uint8_t dhcp_option_lease_time[4];




void atwlan_set_system_ready(void)
{
    g_atwlan_system_ready = 1;    
}

u8 atwlan_get_system_ready(void)
{
    return g_atwlan_system_ready;    
}

static u8 at_wlan_get_if_idx(char* if_name)
{
    u8 mode = s907x_wlan_get_mode();
    u8 if_idx = S907X_DEV0_ID;
    if(!strcmp(if_name,AT_WLAN_AP)){
        if(mode == S907X_MODE_STA_AP)
            if_idx = S907X_DEV1_ID;
    }
        
    return if_idx;
}

void at_wlan_calc_dhcps_lease_pool(const atwlan_if_addr_cfg_t *ap_if, atwlan_dhcps_lease_pool_t *dhcps_lease)
{
    struct ip_addr ap_if_subnet;
    u8 host = ip4_addr4(&ap_if->ip);
    u8 start = host;
    u8 end = host + AT_WLAN_DHCPS_POOL_RANGE;
    
    memset(&ap_if_subnet,0,sizeof(struct ip_addr));
    if (end < start) {
        end = host - 1;
        start = end - AT_WLAN_DHCPS_POOL_RANGE + 1;
    }
    else {
        start = host + 1;
        end = start + AT_WLAN_DHCPS_POOL_RANGE - 1;
    }

    ip_addr_get_network(&ap_if_subnet, &ap_if->ip, &ap_if->netmask);

    dhcps_lease->manual = 0;
    dhcps_lease->lease_time = DHCPS_LEASE_TIME_DEF;
    dhcps_lease->start_ip.addr = ap_if_subnet.addr | htonl(start);
    dhcps_lease->end_ip.addr = ap_if_subnet.addr | htonl(end);
}
static int at_wlan_valid_dhcps_lease(const atwlan_if_addr_cfg_t *ap_if, const atwlan_dhcps_lease_pool_t *dhcps_lease)
{
    struct ip_addr ap_if_subnet;
    struct ip_addr dhcps_lease_subnet;
    
    memset(&ap_if_subnet,0,sizeof(struct ip_addr));
    memset(&dhcps_lease_subnet,0,sizeof(struct ip_addr));

    AT_DBG_MSG("ap interface ip[%x], gw[%x], netmask[%x].", ap_if->ip.addr, ap_if->gw.addr, ap_if->netmask.addr);
    AT_DBG_MSG("dhcps lease pool start ip[%x], end ip[%x].", dhcps_lease->start_ip.addr, dhcps_lease->end_ip.addr);
    ip_addr_get_network(&dhcps_lease_subnet, &dhcps_lease->start_ip, &ap_if->netmask);
    ip_addr_get_network(&ap_if_subnet, &ap_if->ip, &ap_if->netmask);
    if ((0 == dhcps_lease->start_ip.addr)
        || (0xFF == dhcps_lease->start_ip.addr)
        || (0x0 == dhcps_lease->end_ip.addr)
        || (0xFF == dhcps_lease->end_ip.addr)){
        AT_DBG_MSG("dhcps lease ip address is 0.");
        return AT_RET_ERR;
    }
    if ((dhcps_lease->start_ip.addr == dhcps_lease_subnet.addr)
        || (dhcps_lease->end_ip.addr == dhcps_lease_subnet.addr)) {
        AT_DBG_MSG("dhcps lease ip address' host address is 0.");
        return AT_RET_ERR;
    }

    if(dhcps_lease_subnet.addr != ap_if_subnet.addr) {
        AT_DBG_MSG("subnet of dhcps lease is not equal to interface ap");
        return AT_RET_ERR;
    }

    if (DURING_IP_POOL(dhcps_lease->start_ip.addr, dhcps_lease->end_ip.addr, ap_if->ip.addr)) {
        AT_DBG_MSG("Dhcps lease pool include the interface ip address");
        return AT_RET_ERR;
    }

    return AT_RET_OK;
}

void at_wlan_init_wifi_if(const u8 mode, atwlan_if_addr_cfg_t *sta_if, atwlan_if_addr_cfg_t *ap_if)
{
    u8 sta_if_idx = 0;
    u8 ap_if_idx = 0;
    if (S907X_MODE_STA_AP == mode) {
        sta_if_idx = S907X_DEV0_ID;
        ap_if_idx = S907X_DEV1_ID;
        if (0 == sta_if->ip.addr) {
            memset(sta_if, 0, sizeof(atwlan_if_addr_cfg_t));
        }
        netif_set_addr(LwIP_GetNetif(sta_if_idx), &sta_if->ip, &sta_if->netmask, &sta_if->gw);
        if (0 == ap_if->ip.addr) {
            ap_if->ip.addr = inet_addr(AT_WLAN_AP_IP_DEF);
            ap_if->gw.addr = inet_addr(AT_WLAN_AP_GW_DEF);
            ap_if->netmask.addr = inet_addr(AT_WLAN_AP_NETMASK);
        }
        netif_set_addr(LwIP_GetNetif(ap_if_idx), &ap_if->ip, &ap_if->netmask, &ap_if->gw);
    }
    else if (S907X_MODE_STA == mode) {
        sta_if_idx = S907X_DEV0_ID;
        if (0 == sta_if->ip.addr) {
            memset(sta_if, 0, sizeof(atwlan_if_addr_cfg_t));
        }
        netif_set_addr(LwIP_GetNetif(sta_if_idx), &sta_if->ip, &sta_if->netmask, &sta_if->gw);
    }
    else if (S907X_MODE_AP == mode) {
        ap_if_idx = S907X_DEV0_ID;
        if (0 == ap_if->ip.addr) {
            ap_if->ip.addr = inet_addr(AT_WLAN_AP_IP_DEF);
            ap_if->gw.addr = inet_addr(AT_WLAN_AP_GW_DEF);
            ap_if->netmask.addr = inet_addr(AT_WLAN_AP_NETMASK);
        }
        netif_set_addr(LwIP_GetNetif(ap_if_idx), &ap_if->ip, &ap_if->netmask, &ap_if->gw);
    }
}

void at_wlan_init_dhcp_lease(const atwlan_if_addr_cfg_t *ap_if, atwlan_dhcps_lease_pool_t *orig_dhcps_lease)
{
    int ret = AT_RET_ERR;
    atwlan_dhcps_lease_pool_t dhcps_lease;

    memcpy(&dhcps_lease, orig_dhcps_lease, sizeof(atwlan_dhcps_lease_pool_t));
    ret = at_wlan_valid_dhcps_lease(ap_if, &dhcps_lease);
    if (AT_RET_OK != ret) {
        at_wlan_calc_dhcps_lease_pool(ap_if, &dhcps_lease);
        memcpy(orig_dhcps_lease, &dhcps_lease, sizeof(dhcps_lease));
    }

    dhcps_set_lease_time(dhcps_lease.lease_time * 60);
    dhcps_set_addr_pool(1, &dhcps_lease.start_ip, &dhcps_lease.end_ip);
    dhcps_set_iprange(&dhcps_lease.start_ip, &dhcps_lease.end_ip);
}

void wlan_flash_init(void)
{
    u8 mode=0,reconnect_mark=0,country_code=0;
    u8 sta_mac[6]={0},ap_mac[6]={0};
    atwlan_dhcps_lease_pool_t dhcps_lease;
    memset(&dhcps_lease,0,sizeof(atwlan_dhcps_lease_pool_t));

    mode = 2;
    reconnect_mark = 1;
    country_code = 0;
    g_atwlan_dhcp_stat = (AT_AP_DHCP_EN|AT_STA_DHCP_EN);

    strcpy(g_atwlan_cur_ap_cfg.ssid, AT_WLAN_AP_SSID_DEF);
    strcpy(g_atwlan_cur_ap_cfg.password, AT_WLAN_AP_PWD_DEF);
    g_atwlan_cur_ap_cfg.channel = 1;
    g_atwlan_cur_ap_cfg.is_hidded_ssid = 0;
    g_atwlan_cur_ap_cfg.authmode = 5;
    g_atwlan_cur_ap_cfg.max_conn = AT_WLAN_MAX_STA;

    dhcps_lease.manual = 0;
    dhcps_lease.lease_time = DHCPS_LEASE_TIME_DEF;
    dhcps_lease.start_ip.addr = 0;
    dhcps_lease.end_ip.addr = 0;

    g_atwlan_sta_if.ip.addr = 0;
    g_atwlan_sta_if.gw.addr = 0;
    g_atwlan_sta_if.netmask.addr = 0;
    
    g_atwlan_ap_if.ip.addr = 0;
    g_atwlan_ap_if.gw.addr = 0;
    g_atwlan_ap_if.netmask.addr = 0;

    flash_write(WIFI_OPMODE_ADDR,            &mode,                      WIFI_OPMODE_SIZE);
    flash_write(WIFI_SAP_DEF_ADDR,           (u8*)&g_atwlan_cur_ap_cfg,  WIFI_SAP_DEF_SIZE);
    flash_write(WIFI_DHCP_DEF_ADDR,          &g_atwlan_dhcp_stat,        WIFI_DHCP_DEF_SIZE);
    flash_write(WIFI_DHCPS_DEF_ADDR,         (u8*)&dhcps_lease,          WIFI_DHCPS_DEF_SIZE);
    flash_write(WIFI_RECON_ADDR,             &reconnect_mark,            WIFI_RECON_SIZE);
    flash_write(WIFI_STA_MAC_DEF_ADDR,       sta_mac,                    WIFI_STA_MAC_DEF_SIZE);
    flash_write(WIFI_AP_MAC_DEF_ADDR,        ap_mac,                     WIFI_AP_MAC_DEF_SIZE);
    flash_write(WIFI_STA_IP_ADDR_DEF_ADDR,   (u8*)&g_atwlan_sta_if,      WIFI_STA_IP_ADDR_DEF_SIZE);
    flash_write(WIFI_AP_IP_ADDR_DEF_ADDR,    (u8*)&g_atwlan_ap_if,       WIFI_AP_IP_ADDR_DEF_SIZE);
    flash_write(WIFI_COUNTRY_DEF_ADDR,       &country_code,              WIFI_COUNTRY_DEF_SIZE);
    
}

void at_wlan_init_def_para()
{
    u8 mode=0;
	u8 i;
	scan_print_mask.mask = 0xffffffff;
    flash_read(WIFI_OPMODE_ADDR,             &mode,                      WIFI_OPMODE_SIZE);
    if ((mode < S907X_MODE_STA) || (S907X_MODE_STA_AP < mode)){
        wlan_flash_init();
    }

	for(i=0;i<AT_WLAN_MAX_STA;i++)
		memset(&sta_info[i], 0, sizeof(struct ap_sta_info));
    memset(&g_atwlan_cur_ap_cfg, 0, sizeof(atwlan_sap_cfg_t));
    g_atwlan_dhcp_stat = 0;
    memset(&g_atwlan_dhcps_lease, 0, sizeof(atwlan_dhcps_lease_pool_t));
    memset(&g_atwlan_sta_if, 0, sizeof(atwlan_if_addr_cfg_t));
    memset(&g_atwlan_ap_if, 0, sizeof(atwlan_if_addr_cfg_t));

    flash_read(WIFI_OPMODE_ADDR,             &mode,                      WIFI_OPMODE_SIZE);
    flash_read(WIFI_SAP_DEF_ADDR,            (u8*)&g_atwlan_cur_ap_cfg,  WIFI_SAP_DEF_SIZE);
    flash_read(WIFI_DHCP_DEF_ADDR,           &g_atwlan_dhcp_stat,        WIFI_DHCP_DEF_SIZE);
    flash_read(WIFI_DHCPS_DEF_ADDR,          (u8*)&g_atwlan_dhcps_lease, WIFI_DHCPS_DEF_SIZE);
    flash_read(WIFI_STA_IP_ADDR_DEF_ADDR,    (u8*)&g_atwlan_sta_if,      WIFI_STA_IP_ADDR_DEF_SIZE);
    flash_read(WIFI_AP_IP_ADDR_DEF_ADDR,     (u8*)&g_atwlan_ap_if,       WIFI_AP_IP_ADDR_DEF_SIZE);
 
    AT_DBG_MSG("sta if ip [%s].", inet_ntoa(g_atwlan_sta_if.ip));
    AT_DBG_MSG("sta if gw [%s].", inet_ntoa(g_atwlan_sta_if.gw));
    AT_DBG_MSG("sta if netmask [%s].", inet_ntoa(g_atwlan_sta_if.netmask));
    AT_DBG_MSG("ap if ip [%s].", inet_ntoa(g_atwlan_ap_if.ip));
    AT_DBG_MSG("ap if gw [%s].", inet_ntoa(g_atwlan_ap_if.gw));
    AT_DBG_MSG("ap if netmask [%s].", inet_ntoa(g_atwlan_ap_if.netmask));
    AT_DBG_MSG("dhcp lease start ip [%s].", inet_ntoa(g_atwlan_dhcps_lease.start_ip));
    AT_DBG_MSG("dhcp lease end ip [%s].", inet_ntoa(g_atwlan_dhcps_lease.end_ip));
    AT_DBG_MSG("dhcp state [%d].", g_atwlan_dhcp_stat);
    at_wlan_init_wifi_if(mode, &g_atwlan_sta_if, &g_atwlan_ap_if);
    at_wlan_init_dhcp_lease(&g_atwlan_ap_if, &g_atwlan_dhcps_lease);

    g_atwlan_sta_ip_static = 1;
    if (g_atwlan_dhcp_stat & AT_STA_DHCP_EN) {
        g_atwlan_sta_ip_static = 0;
    }
}
#if 0
static int at_get_param(char **value, char *val)
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
#endif
void at_wlan_add_ip(struct ip_addr ip,u8 *bssid)
{
    u8 i=0;
    u8 *p_bssid = NULL;
    for(i=0;i<AT_WLAN_MAX_STA;i++){
        p_bssid = sta_info[i].bssid;
        if(!memcmp(p_bssid,bssid,AT_WLAN_BSSID_LEN)){
			if(sta_info[i].ip.addr != ip.addr){
				memcpy(&sta_info[i].ip , &ip,sizeof(struct ip_addr));
				at_rsp(AT_WLAN_DIST_STA_IP,
					p_bssid[0], p_bssid[1], p_bssid[2], 
					p_bssid[3],p_bssid[4], p_bssid[5],
					inet_ntoa(sta_info[i].ip));
			}
            break;
        }
    }
}

static void at_wlan_add_sta(u8 aid,u8 *bssid)

{
    memcpy(sta_info[aid-1].bssid, bssid, AT_WLAN_BSSID_LEN);
}

static void at_wlan_del_sta(u8 aid)

{
    memset(&sta_info[aid-1], 0, sizeof(struct ap_sta_info));
}

static void at_wlan_flush_sta(void)
{
    u8 idx = 0;
    u8 *bssid = NULL;

    for (idx = 0; idx < AT_WLAN_MAX_STA; idx++) {
        bssid = sta_info[idx].bssid;
        if (MAC_ADDR_IS_ZERO(bssid)) {
            continue;
        }
        at_rsp(AT_WLAN_DIST_STA_IP,
                bssid[0], bssid[1], bssid[2], 
                bssid[3], bssid[4], bssid[5],
                ZERO_IP_ADDR);
        at_wlan_del_sta(idx);
    }
}

static void at_wlan_get_if_addr(const u8 if_idx, atwlan_if_addr_cfg_t *if_addr)
{
    struct netif *netif = LwIP_GetNetif(if_idx);

    memcpy(&(if_addr->ip), LwIP_GetIP(netif), sizeof(if_addr->ip));
    memcpy(&(if_addr->gw), LwIP_GetGW(netif), sizeof(if_addr->gw));
    memcpy(&(if_addr->netmask), LwIP_GetMASK(netif), sizeof(if_addr->netmask));
}
static int at_wlan_to_update_lease_pool(const atwlan_if_addr_cfg_t *orig_if_addr,
                                const atwlan_if_addr_cfg_t *new_if_addr, const atwlan_dhcps_lease_pool_t *orig_lease)
{
    struct ip_addr orig_subnet = {0}, new_subnet = {0};
    
    ip_addr_get_network(&orig_subnet, &orig_if_addr->ip, &orig_if_addr->netmask);
    ip_addr_get_network(&new_subnet, &new_if_addr->ip, &new_if_addr->netmask);
    if (orig_subnet.addr != new_subnet.addr)
    {
        return AT_RET_OK;
    }

    if (!orig_lease->manual)
    {
        return AT_RET_OK;
    }

    if (DURING_IP_POOL(orig_lease->start_ip.addr, orig_lease->end_ip.addr, new_if_addr->ip.addr))
    {
        return AT_RET_OK;
    }
    return AT_RET_ERR;
}

static void at_wlan_event_cb_scan_down(s907x_event_data *s907x_data, void *context)
{
    at_rsp("SCAN DOWN\n");
}

static void at_wlan_event_sta_connected(s907x_event_data *s907x_data, void *context)
{
    at_rsp(AT_WLAN_WIFI_CONNECTED);
}

void at_wlan_event_sta_got_ip(u32 addr)
{
    at_rsp(AT_WLAN_WIFI_GOT_IP);
	//s907x_wlan_enable_sleep(1);
}

static void at_wlan_event_sta_disconnected(s907x_event_data *s907x_data, void *context)
{
    at_rsp(AT_WLAN_WIFI_DISCONNECTED);
	//s907x_wlan_enable_sleep(0);
}
 
static void at_wlan_event_sta_authchange(s907x_event_data *s907x_data, void *context)
{
    at_rsp(AT_WLAN_WIFI_AUTH_CHANGED);
}

static void at_wlan_event_ap_sta_connected(s907x_event_data *s907x_data, void *context)
{
    at_rsp(AT_WLAN_STA_CONNECTED, s907x_data->esp_data.sta_connected.mac[0], s907x_data->esp_data.sta_connected.mac[1], 
        s907x_data->esp_data.sta_connected.mac[2], s907x_data->esp_data.sta_connected.mac[3],s907x_data->esp_data.sta_connected.mac[4],
        s907x_data->esp_data.sta_connected.mac[5]);
    at_wlan_add_sta(s907x_data->esp_data.sta_connected.aid,s907x_data->esp_data.sta_connected.mac);
}

static void at_wlan_event_ap_sta_disconnected(s907x_event_data *s907x_data, void *context)
{
    at_rsp(AT_WLAN_STA_DISCONNECTED, s907x_data->esp_data.sta_disconnected.mac[0], s907x_data->esp_data.sta_disconnected.mac[1], 
    s907x_data->esp_data.sta_disconnected.mac[2], s907x_data->esp_data.sta_disconnected.mac[3],s907x_data->esp_data.sta_disconnected.mac[4], 
    s907x_data->esp_data.sta_disconnected.mac[5]);
    at_wlan_del_sta(s907x_data->esp_data.sta_connected.aid);
}

static void at_wlan_event_ap_probereq_received(s907x_event_data *s907x_data, void *context)
{
    at_rsp(AT_WLAN_AP_PROBE_RECEIVED, s907x_data->esp_data.ap_probereqrecved.rssi, s907x_data->esp_data.ap_probereqrecved.mac[0], s907x_data->esp_data.ap_probereqrecved.mac[1], 
    s907x_data->esp_data.ap_probereqrecved.mac[2], s907x_data->esp_data.ap_probereqrecved.mac[3],s907x_data->esp_data.ap_probereqrecved.mac[4], s907x_data->esp_data.ap_probereqrecved.mac[5]);

} 

void at_wlan_event_reg(u8 mode)
{
    if(mode&S907X_MODE_STA){
        s907x_wlan_event_reg(S907X_EVENT_STAMODE_SCAN_DONE, at_wlan_event_cb_scan_down, NULL);
        s907x_wlan_event_reg(S907X_EVENT_STAMODE_CONNECTED, at_wlan_event_sta_connected, NULL);
        s907x_wlan_event_reg(S907X_EVENT_STAMODE_DISCONNECTED, at_wlan_event_sta_disconnected, NULL);
        s907x_wlan_event_reg(S907X_EVENT_STAMODE_AUTHCHANGE, at_wlan_event_sta_authchange, NULL);
    }
    if(mode&S907X_MODE_AP){
        s907x_wlan_event_reg(S907X_EVENT_APMODE_STA_CONNECTED, at_wlan_event_ap_sta_connected, NULL);
        s907x_wlan_event_reg(S907X_EVENT_APMODE_STA_DISCONNECTED, at_wlan_event_ap_sta_disconnected, NULL);
        //s907x_wlan_event_reg(S907X_EVENT_APMODE_PROBE_REG_RECEIVED, s907x_event_ap_probereq_received, NULL);
    }
}


static void s907x_scan_debug(s907x_scan_info_t *pscan_info)
{
	char *output,buf[256];
	int cnt=0;
	if(sta_connect_to_ap)
		return;
	
	memset(buf,0,256);
	output = buf;
	cnt += sprintf(output, "%s", AT_WLAN_CWLAP);
	
	if(scan_print_mask.bit.ecn_en){
		cnt += sprintf(output+cnt, "%d", pscan_info->security);
	}
	
	if(scan_print_mask.bit.ssid_en){
		if(cnt > strlen(AT_WLAN_CWLAP))
			cnt += sprintf(output+cnt, ",");
		cnt += sprintf(output+cnt, "\"%s\"", pscan_info->ssid);
	}

	if(scan_print_mask.bit.rssi_en){
		if(cnt > strlen(AT_WLAN_CWLAP))
			cnt += sprintf(output+cnt, ",");
		cnt += sprintf(output+cnt, "%d", pscan_info->rssi);
	}

	if(scan_print_mask.bit.mac_en){
		if(cnt > strlen(AT_WLAN_CWLAP))
			cnt += sprintf(output+cnt, ",");
		cnt += sprintf(output+cnt, AT_WLAN_MAC_FMT_STR, pscan_info->bssid[0], pscan_info->bssid[1], 
			pscan_info->bssid[2], pscan_info->bssid[3], pscan_info->bssid[4], pscan_info->bssid[5]);
	}

	if(scan_print_mask.bit.ch_en){
		if(cnt > strlen(AT_WLAN_CWLAP))
			cnt += sprintf(output+cnt, ",");
		cnt += sprintf(output+cnt, "%d", pscan_info->channel);
	}
	cnt += sprintf(output+cnt, "%s", ")\r\n");
	at_rsp(buf);
}

static void s907x_wlan_scan_cb(s907x_scan_result_t *presult)
{
    struct scan_config *scan_cfg;
    ASSERT(presult);
    
    scan_cfg = (struct scan_config*)presult->context; 
    
    if(presult->max_nums < 1) {
        wl_send_sema((sema_t*)&scan_target.sema);
        return;
    } 
    if(scan_cfg){
        if(scan_cfg->bssid) {
			if(!memcmp(scan_cfg->bssid, presult->scan_info.bssid,6)){
				s907x_scan_debug(&presult->scan_info);
	            if(scan_cfg->ssid && !strcmp(scan_cfg->ssid, presult->scan_info.ssid)){
	                scan_target.match =  TRUE;
	                scan_target.channel = presult->scan_info.channel;
	                scan_target.security = presult->scan_info.security;
	            }
			}
        }
        else if(!scan_cfg->bssid && scan_cfg->ssid && !strcmp(scan_cfg->ssid, presult->scan_info.ssid)){
            scan_target.match =  TRUE;
            scan_target.channel = presult->scan_info.channel;
            scan_target.security = presult->scan_info.security;
            s907x_scan_debug(&presult->scan_info);
        }
    }
    else{
        s907x_scan_debug(&presult->scan_info);
    }
    
    if(presult->id == presult->max_nums - 1) {
        wl_send_sema((sema_t*)&scan_target.sema);
    }
}

static int atwlan_get_args_cwmode(const int argc, char ** const argv, u8 *out_mode)
{
    int mode;
    PROCESS_ERROR(argc != 1);

    mode = atoi(argv[0]);
    PROCESS_ERROR(!((S907X_MODE_STA <= mode) && ( mode <= S907X_MODE_STA_AP)));
    *out_mode = (u8)mode;
    return AT_RET_OK;
err:
    return AT_RET_ERR;
}

static int atwlan_get_args_cwjap(const int argc, char ** const argv, atwlan_jap_cfg_t *jap_cfg)
{
    int index = 0;
    int pci_en = 0;
    u8  bssid[6]={0};

    PROCESS_ERROR(!((2 <= argc) && (argc <= 4)));
    PROCESS_ERROR(at_parse_param_str(&argv[index]));
    PROCESS_ERROR(strlen(argv[index]) > AT_WLAN_SSID_LEN);
    jap_cfg->ssid = argv[index];
    index ++;

    PROCESS_ERROR(at_parse_param_str(&argv[index]));
    PROCESS_ERROR(strlen(argv[index]) > AT_WLAN_PWD_LEN);
    jap_cfg->password = argv[index];
    index ++;
    
    ARGC_INDEX_VALID(){
        if(argv[index]){
            PROCESS_ERROR(at_parse_param_str(&argv[index]));
            PROCESS_ERROR(strlen(argv[index]) != AT_WLAN_MAC_LEN);
            sscanf(argv[index], AT_WLAN_MAC_FMT, &bssid[0], &bssid[1], &bssid[2], &bssid[3],&bssid[4],&bssid[5]);
			memcpy(jap_cfg->bssid,bssid,6);
            jap_cfg->bssid_set = 1;
        }
        index ++;
    }

    ARGC_INDEX_VALID(){
        if(argv[index]){
            pci_en = atoi(argv[index]);
            PROCESS_ERROR(!((0 <= pci_en) && (pci_en <= 1)));
            jap_cfg->pci_en = (u8)pci_en;
        }
        index ++;
    }
    return AT_RET_OK;

err:
    return AT_RET_ERR;
}
static int atwlan_get_args_cwsap(const int argc, char ** const argv, const u8 mode, atwlan_sap_cfg_t *sap_cfg)
{
    int value = 0;
    int index = 0;

    PROCESS_ERROR(!((4 <= argc) && (argc <= 6)));

    PROCESS_ERROR(at_parse_param_str(&argv[index]));
    PROCESS_ERROR(strlen(argv[index]) > AT_WLAN_SSID_LEN);
    strcpy(sap_cfg->ssid,argv[index]);
    index ++;

    PROCESS_ERROR(at_parse_param_str(&argv[index]));
    PROCESS_ERROR(strlen(argv[index]) > AT_WLAN_PWD_LEN);
    strcpy(sap_cfg->password,argv[index]);
    index ++;

    value = atoi(argv[index]);
    PROCESS_ERROR(!((1 <= value) && (value <= AT_WLAN_MAX_CHANNEL)));
    sap_cfg->channel = (u8)value;
    index ++;

    value = atoi(argv[index]);
    PROCESS_ERROR(!VALID_SAP_ENC_TYPE(value));
    if(S907X_SECURITY_NONE == value) 
        sap_cfg->authmode = S907X_SECURITY_NONE;
    else 
        sap_cfg->authmode = S907X_SECURITY_WPA2_AES;

    index ++;

    ARGC_INDEX_VALID(){
        if(argv[index]){
            value = atoi(argv[index]);
            PROCESS_ERROR(!((1 <= value) && (value <= AT_WLAN_SAP_MAX_STA)));
            sap_cfg->max_conn = (u8)value;
        }
        index ++;//max conn
    }

    ARGC_INDEX_VALID(){
        if(argv[index]){
            value = atoi(argv[index]);
            PROCESS_ERROR(!((0 <= value) && (value <= 1)));
            sap_cfg->is_hidded_ssid = (u8)value;
        }
        index ++;
    }

    return AT_RET_OK;

err:
    return AT_RET_ERR;
}
static int atwlan_get_args_cwdhcp(const int argc, char ** const argv, u8 *out_dhcp_if, u8 *out_enable)
{
    u8 enable = 0;
    u8 dhcp_if = ATWLAN_DHCP_IF_AP;
    int index = 0;
    u8 mode;

    mode = s907x_wlan_get_mode();

    PROCESS_ERROR(argc != 2);

    dhcp_if = atoi(argv[index]);
    PROCESS_ERROR(!((ATWLAN_DHCP_IF_AP <= dhcp_if) && (dhcp_if <= ATWLAN_DHCP_IF_APSTA)));
    if(dhcp_if == ATWLAN_DHCP_IF_AP)
        PROCESS_ERROR(!((mode == S907X_MODE_AP) || (mode == S907X_MODE_STA_AP)));
    else if(dhcp_if == ATWLAN_DHCP_IF_STA)
        PROCESS_ERROR(!((mode == S907X_MODE_STA) || (mode == S907X_MODE_STA_AP)));
    else if(dhcp_if == ATWLAN_DHCP_IF_APSTA)
        PROCESS_ERROR(mode != S907X_MODE_STA_AP);
    index ++;

    enable = atoi(argv[index]);
    PROCESS_ERROR(!((0 <= enable) && (enable <= 1)));
    *out_dhcp_if = dhcp_if;
    *out_enable = enable;
    return AT_RET_OK;

err:
    return AT_RET_ERR;
}

static int atwlan_get_args_cwdhcps(const int argc, char ** const argv, atwlan_dhcps_lease_pool_t *out_dhcps_lease)
{
    int value = 0;
    int index = 0;
    int if_idx = 0;
    int ret = HAL_ERROR;
    atwlan_if_addr_cfg_t ap_if;
    atwlan_dhcps_lease_pool_t dhcps_lease = {0};

    PROCESS_ERROR(argc != 1 && argc != 4);

    value = atoi(argv[index]);
    PROCESS_ERROR(!((0 <= value) && (value <= 1)));
    index ++;

    if(!value){
        if_idx = at_wlan_get_if_idx(AT_WLAN_AP);
        at_wlan_get_if_addr(if_idx, &ap_if);
        at_wlan_calc_dhcps_lease_pool(&ap_if, &dhcps_lease);
    }
    else{
        dhcps_lease.manual = 1;
        value = atoi(argv[index]);
        PROCESS_ERROR(!((1 <= value) && (value <= AT_WLAN_MAX_DHCPS_LEASE_TIME)));
        dhcps_lease.lease_time = value;
        index ++;

        PROCESS_ERROR(at_parse_param_str(&argv[index]));
        ret = inet_aton(argv[index], &(dhcps_lease.start_ip));
        PROCESS_ERROR(0 == ret);
        index ++;

        PROCESS_ERROR(at_parse_param_str(&argv[index]));
        ret = inet_aton(argv[index], &(dhcps_lease.end_ip));
        PROCESS_ERROR(0 == ret);
        index ++;
    }

    memcpy(out_dhcps_lease, &dhcps_lease, sizeof(atwlan_dhcps_lease_pool_t));
    return AT_RET_OK;

err:
    return AT_RET_ERR;
}
static int atwlan_get_args_cip(const int argc, char ** const argv, atwlan_if_addr_cfg_t *if_addr)
{
    int index = 0;
    int ret = 0;
    PROCESS_ERROR(!((1 <= argc) && (argc <= 3)));
    memset(if_addr, 0, sizeof(atwlan_if_addr_cfg_t));

    PROCESS_ERROR(at_parse_param_str(&argv[index]));
    ret = inet_aton(argv[index], &(if_addr->ip));
    index ++;
    ARGC_INDEX_VALID(){
        if(argv[index]){
            PROCESS_ERROR(at_parse_param_str(&argv[index]));
            ret = inet_aton(argv[index], &(if_addr->gw));
            PROCESS_ERROR(0 == ret);
        }
        index++;
    }
    ARGC_INDEX_VALID(){
        if(argv[index]){
            PROCESS_ERROR(at_parse_param_str(&argv[index]));
            ret = inet_aton(argv[index], &(if_addr->netmask));
            PROCESS_ERROR(0 == ret);
        }
        index++;
    }
    return AT_RET_OK;

err:
    return AT_RET_ERR;
}

static int atwlan_get_args_cipsta(const int argc, char ** const argv, atwlan_if_addr_cfg_t *if_addr)
{
    int ret = AT_RET_ERR;
    int def_gw_host = 1;
    ip_addr_t if_subnet = {0};

    ret = atwlan_get_args_cip(argc, argv, if_addr);
    PROCESS_ERROR(AT_RET_OK != ret);

    if (0 == if_addr->netmask.addr) {
        inet_aton(AT_WLAN_AP_NETMASK, &(if_addr->netmask));
    }

    if (0 == if_addr->gw.addr) {
        ip_addr_get_network(&if_subnet, &if_addr->ip, &if_addr->netmask);
        if_addr->gw.addr = if_subnet.addr | htonl(def_gw_host);
    }

    return AT_RET_OK;

err:
    return AT_RET_ERR;
}

static int atwlan_get_args_cipap(const int argc, char ** const argv, atwlan_if_addr_cfg_t *if_addr)
{
    int ret = AT_RET_ERR;

    ret = atwlan_get_args_cip(argc, argv, if_addr);
    PROCESS_ERROR(AT_RET_OK != ret);

    if (0 == if_addr->netmask.addr) {
        inet_aton(AT_WLAN_AP_NETMASK, &(if_addr->netmask));
    }

    if (0 == if_addr->gw.addr) {
        if_addr->gw.addr = if_addr->ip.addr;
    }

    return AT_RET_OK;

err:
    return AT_RET_ERR;
}

void atwlan_init_mac(u8 mode)
{
	u8 mac[6]={0};
	u8 if_idx = 0;
	
	if(mode & S907X_MODE_STA){
		if_idx=at_wlan_get_if_idx(AT_WLAN_STA);
		flash_read(WIFI_STA_MAC_DEF_ADDR, mac, WIFI_STA_MAC_DEF_SIZE);
		s907x_wlan_set_mac_address_no_efuse(if_idx,mac);
	}

	if(mode & S907X_MODE_AP){
		if_idx=at_wlan_get_if_idx(AT_WLAN_AP);
		flash_read(WIFI_AP_MAC_DEF_ADDR, mac, WIFI_AP_MAC_DEF_SIZE);
		s907x_wlan_set_mac_address_no_efuse(if_idx,mac);
	}
}


int atwlan_cwmode(u8 mode)
{
    int ret = AT_RET_ERR;
	u8 i;
	u32 random; 
	
    s907x_wlan_off(); 
    wl_os_mdelay(50);
    ret = s907x_wlan_on(mode);
    
    if(!ret){
		//s907x_wlan_set_country(S907X_COUNTRY_CN);
		for(i=0;i<AT_WLAN_MAX_STA;i++)
			memset(&sta_info[i], 0, sizeof(struct ap_sta_info));
        memset(&g_atwlan_cur_ap_cfg,0,sizeof(atwlan_sap_cfg_t));
        dhcp_option_lease_time[0] = 0x00;
        dhcp_option_lease_time[1] = 0x00;
        dhcp_option_lease_time[2] = 0x1c;
        dhcp_option_lease_time[3] = 0x20;

        if(mode&S907X_MODE_STA){
            u8 mac[6];
            u8 host_name[AT_WLAN_HOSTNAME_LEN];
            s907x_wlan_get_mac_address(S907X_DEV0_ID,mac);
			if(mac[0]&BIT(0)){
				random = wl_get_random32();
				mac[0]=0xb4;
				mac[1]=0x04;
				mac[2]=0x18;
				mac[3]=random&0xff;
				mac[4]=(random >> 8)&0xff;
				mac[5]=(random >> 16)&0xff;
				s907x_wlan_set_mac_address(S907X_DEV0_ID,mac);
			}
			sprintf(host_name, STA_HOST_NAME,mac[3],mac[4],mac[5]);
            lwip_set_hostname(LwIP_GetNetif(S907X_DEV0_ID),host_name);
        }
        at_wlan_init_wifi_if(mode, &g_atwlan_sta_if, &g_atwlan_ap_if);
        at_wlan_event_reg(mode);
    }
    return ret;
}

static int atwlan_cwjap(atwlan_jap_cfg_t *jap_cfg,u32 *result)
{
    int ret= -1;
    s907x_sta_init_t init;
    u8 if_idx;
    atwlan_if_addr_cfg_t if_addr;
	s907x_scan_result_t scan_result;
	
	memset(&scan_result,0,sizeof(s907x_scan_result_t));
    memset(&init,0,sizeof(s907x_sta_init_t));
    memset(&if_addr,0,sizeof(atwlan_if_addr_cfg_t));
    atwlan_jap_err_e jap_err = ATWLAN_JAP_ERR_OK;
    if(!jap_cfg->bssid[0])
        jap_cfg->bssid = NULL;
	
    if_idx = at_wlan_get_if_idx(AT_WLAN_STA);    
    *result = S907X_IDLE;
	ret = s907x_wlan_scan_ssid(jap_cfg->ssid,strlen(jap_cfg->ssid),&scan_result);
	if(jap_cfg->bssid && memcmp(scan_result.scan_info.bssid,jap_cfg->bssid,6)){
		*result = S907X_NO_NETWORK;
		goto err;
	}
    if(!strcmp(scan_result.scan_info.ssid,jap_cfg->ssid)) { //scan_target.match
        //set security
        init.security = scan_result.scan_info.security;//scan_target.security;
        //set connection mode
        init.conn.mode =  CONN_MODE_BLOCKING;
        init.conn.blocking_timeout = S907X_DEFAULT_CONN_TO;
        init.conn.result = result;
        init.ssid = jap_cfg->ssid;
        init.ssid_len = strlen(init.ssid);
        init.password = jap_cfg->password; 
        init.password_len = strlen(init.password);

        ret = s907x_wlan_start_sta(&init);
        PROCESS_ERROR(ret);
        if(!ret){
            if(g_atwlan_sta_ip_static == 0) {
                if (g_atwlan_dhcp_stat & AT_STA_DHCP_EN) {
                    dhcpc_start(if_idx, DHCP_START);
                }
            }else{
                at_wlan_get_if_addr(if_idx, &if_addr);
                if (0 == if_addr.ip.addr) {
                    wl_os_mdelay(AT_WLAN_DELAY_TO_GET_IP);
                }

				at_wlan_get_if_addr(if_idx, &if_addr);
				if (0 == if_addr.ip.addr) {
					ret = AT_RET_ERR;
					*result = S907X_CONNECT_TIMEOUT;
				}
			}
		}
	} 
	else
		*result = S907X_NO_NETWORK;
err:
    if ((0 != ret) && (S907X_CONNECTED != *result)) {
        s907x_wlan_stop_sta();
        switch (*result) {
            case S907X_CONNECT_TIMEOUT:
                jap_err = ATWLAN_JAP_ERR_TIMEOUT;
                break;
            case S907X_CONNECT_PASSWORD_ERROR:
                jap_err = ATWLAN_JAP_ERR_PWD_FAIL;
                break;
            case S907X_NO_NETWORK:
                jap_err = ATWLAN_JAP_ERR_NO_AP;
                break;
            default:
                jap_err = ATWLAN_JAP_ERR_CON_FAIL;
                break;
        }
        at_rsp(AT_WLAN_JAP_FAIL, jap_err);
    }
 
    return ret; 
}

static int atwlan_cwsap(const u8 mode, const atwlan_sap_cfg_t *sap_cfg)
{
    u8  if_idx = 0;
    int ret = 0;
    s907x_ap_init_t ap_init;
    memset(&ap_init,0,sizeof(s907x_ap_init_t));
    ap_init.ssid = (char*)sap_cfg->ssid;
    ap_init.ssid_len = strlen(ap_init.ssid);

    ap_init.password = (char*)sap_cfg->password;
    ap_init.password_len = strlen(ap_init.password);
    ap_init.channel = sap_cfg->channel;
    ap_init.security = sap_cfg->authmode;
    ap_init.is_hidded_ssid = sap_cfg->is_hidded_ssid;

    if (g_atwlan_dhcp_stat & AT_AP_DHCP_EN) {
        if (g_atwlan_dhcps_init) {
            dhcps_deinit();
            g_atwlan_dhcps_init = 0;
        }
    }
	if(g_atwlan_cur_ap_cfg.channel){
		s907x_wlan_stop_ap();
	}	

	ret = s907x_wlan_start_ap(&ap_init);
    if(!ret){
        if_idx = at_wlan_get_if_idx(AT_WLAN_AP);
        if (g_atwlan_dhcp_stat & AT_AP_DHCP_EN) {
            dhcps_init(LwIP_GetNetif(if_idx));
            g_atwlan_dhcps_init = 1;
        } 
        memcpy(&g_atwlan_cur_ap_cfg,sap_cfg,sizeof(atwlan_sap_cfg_t));
        return AT_RET_OK;
    }
    return AT_RET_ERR;
}

static void atwlan_cwdhcp(u8 dhcp_if, u8 enable)
{
    u8 if_idx = 0;
    u8 orig_sta_ip_static = 0;

    // DHCP for AP interface

    if ((ATWLAN_DHCP_IF_AP == dhcp_if) || (ATWLAN_DHCP_IF_APSTA == dhcp_if)) {

        if (g_atwlan_dhcps_init) {

            dhcps_deinit();
            g_atwlan_dhcps_init = 0;
        }

        if (enable) {

            if_idx = at_wlan_get_if_idx(AT_WLAN_AP);

            AT_SET_BIT(g_atwlan_dhcp_stat, AT_AP_DHCP_EN);
            dhcps_init(LwIP_GetNetif(if_idx));
            g_atwlan_dhcps_init = 1;
        }
        else {
            AT_CLR_BIT(g_atwlan_dhcp_stat, AT_AP_DHCP_EN);
        }
    }

    // DHCP for STA interface
    if ((ATWLAN_DHCP_IF_STA == dhcp_if) || (ATWLAN_DHCP_IF_APSTA == dhcp_if)) {
        orig_sta_ip_static = g_atwlan_sta_ip_static;
        if (enable) {

            if_idx = at_wlan_get_if_idx(AT_WLAN_STA);

            AT_SET_BIT(g_atwlan_dhcp_stat, AT_STA_DHCP_EN);
            g_atwlan_sta_ip_static = 0;
            if ((1 == orig_sta_ip_static) && netif_is_link_up(LwIP_GetNetif(if_idx))) {
                dhcpc_start(if_idx, DHCP_START);
            }
        }
        else {
            AT_CLR_BIT(g_atwlan_dhcp_stat, AT_STA_DHCP_EN);
            g_atwlan_sta_ip_static = 1;
        }
    }
}

static int atwlan_cwdhcps(atwlan_dhcps_lease_pool_t *dhcps_lease)
{
    u8 if_idx = 0;
    int ret = AT_RET_ERR;
    atwlan_if_addr_cfg_t if_addr;
    u8 num, i;
    s907x_ap_client_infor_t *ap_clinet;
    
    memset(&if_addr,0,sizeof(atwlan_if_addr_cfg_t));
    if_idx = at_wlan_get_if_idx(AT_WLAN_AP);
    at_wlan_get_if_addr(if_idx, &if_addr);
    ret = at_wlan_valid_dhcps_lease(&if_addr, dhcps_lease);
    if (AT_RET_OK != ret) {
        return ret;
    }
    // Deauth associated sta
    num = s907x_wlan_ap_get_client_nums();
    if(num){
        ap_clinet = wl_zmalloc(sizeof(s907x_ap_client_infor_t)*num);
        if(!ap_clinet) {
            AT_DBG_ERR("malloc ap client error");
            return AT_RET_ERR;
        }
        ret = s907x_wlan_ap_get_client_infor(ap_clinet, num);
        for( i = 0; i < num; i++) {
            s907x_wlan_ap_deauth_sta(ap_clinet->hw_addr);
            ap_clinet++;
        }
        wl_free(ap_clinet);
    }

    if (g_atwlan_dhcps_init) {
        dhcps_deinit();
        g_atwlan_dhcps_init = 0;
    }
    dhcps_set_lease_time(dhcps_lease->lease_time * 60);
    dhcps_set_addr_pool(1, &dhcps_lease->start_ip, &dhcps_lease->end_ip);
    dhcps_set_iprange(&dhcps_lease->start_ip, &dhcps_lease->end_ip);

    dhcps_init(LwIP_GetNetif(if_idx));
    g_atwlan_dhcps_init = 1;

    memcpy(&g_atwlan_dhcps_lease, dhcps_lease, sizeof(g_atwlan_dhcps_lease));

    return AT_RET_OK;
}

static void atwlan_cipsta(u8 if_idx, const atwlan_if_addr_cfg_t *if_addr)
{
    netif_set_addr(LwIP_GetNetif(if_idx), (ip_addr_t *)&if_addr->ip,(ip_addr_t *) &if_addr->netmask, (ip_addr_t *)&if_addr->gw);
    memcpy(&g_atwlan_sta_if, if_addr, sizeof(atwlan_if_addr_cfg_t));
    g_atwlan_sta_ip_static = 1;
}

static void atwlan_cipap(u8 if_idx, const atwlan_if_addr_cfg_t *orig_if_addr, atwlan_if_addr_cfg_t *new_if_addr)
{
    int ret = AT_RET_ERR;
    // Check to update the DHCPS Lease Pool
    // If Need to update the lease pool
    //  Subnetwork changed    
    ret = at_wlan_to_update_lease_pool(orig_if_addr, new_if_addr, &g_atwlan_dhcps_lease);
    if (AT_RET_OK == ret) {
        at_wlan_calc_dhcps_lease_pool(new_if_addr, &g_atwlan_dhcps_lease);
        dhcps_set_lease_time(g_atwlan_dhcps_lease.lease_time * 60);
        dhcps_set_addr_pool(1, &g_atwlan_dhcps_lease.start_ip, &g_atwlan_dhcps_lease.end_ip);
    }

    // Set the address to the related interface
    netif_set_addr(LwIP_GetNetif(if_idx), &new_if_addr->ip, &new_if_addr->netmask, &new_if_addr->gw);
    memcpy(&g_atwlan_ap_if, new_if_addr, sizeof(atwlan_if_addr_cfg_t));
}

//AT+CWMODE_CUR 设置 Wi-Fi 模式 (STA/AP/STA+AP)，不保存到 Flash
static void atwlan_cwmode_cur (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    char *argv[AT_SET_MAX_ARGC]={0};
    int argc,ret=AT_RET_ERR;
    u8 mode;
    
    ASSERT(cmd);
    if(!cmd->setsize && cmd->mode == AT_MODE_R){
        mode = s907x_wlan_get_mode();
        at_rsp(AT_WLAN_MODE_CUR, mode);
        PROCESS_SUCCESS(1);
    }
    else{
        PROCESS_ERROR(cmd->mode != AT_MODE_W);
		if(!strcmp(cmd->set,"?")){
			at_rsp(AT_WLAN_MODE_CUR_GET);
			PROCESS_SUCCESS(1);
		}
    }
    
    argc = at_get_param(argv, cmd->set);
    ret = atwlan_get_args_cwmode(argc, argv, &mode);
    PROCESS_ERROR(AT_RET_OK != ret);
    
    ret = atwlan_cwmode((u8)mode);
    PROCESS_SUCCESS(!ret);

err:
    at_rsp(AT_ERROR);
    return;
    
success:
    at_rsp(AT_OK);

}     

//AT+CWMODE_DEF 设置 Wi-Fi 模式 (STA/AP/STA+AP)，保存到 Flash
static void atwlan_cwmode_def (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    char *argv[AT_SET_MAX_ARGC]={0};
    int argc,ret=AT_RET_ERR;
    u8 mode;
    
    ASSERT(cmd);
    if(!cmd->setsize && cmd->mode == AT_MODE_R){
        #if WLAN_PARA_SAVE
        flash_read(WIFI_OPMODE_ADDR, &mode, WIFI_OPMODE_SIZE);
        #endif
        at_rsp(AT_WLAN_MODE_DEF, mode);
        PROCESS_SUCCESS(1);
    }
    else{
        PROCESS_ERROR(cmd->mode != AT_MODE_W);
		if(!strcmp(cmd->set,"?")){
			at_rsp(AT_WLAN_MODE_DEF_GET);
			PROCESS_SUCCESS(1);
		}
    }
    
    argc = at_get_param(argv, cmd->set);
    ret = atwlan_get_args_cwmode(argc, argv, &mode);
    PROCESS_ERROR(AT_RET_OK != ret);
    ret = atwlan_cwmode(mode);
    PROCESS_SAVE_FLASH(!ret);

err:
    at_rsp(AT_ERROR);
    return;


save_to_flash:
#if WLAN_PARA_SAVE
    flash_write(WIFI_OPMODE_ADDR, &mode, WIFI_OPMODE_SIZE);
#endif    

success:
    at_rsp(AT_OK);
}     

//AT+CWJAP_CUR 连接 AP，不保存到 Flash
static void atwlan_cwjap_cur (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    int ret = AT_RET_ERR;
    char *argv[AT_SET_MAX_ARGC]={0};
    int argc;
    u8 bssid[6]={0},mode;
    u32 result;
    u8 recon_en;
    atwlan_jap_cfg_t jap_cfg;
    s907x_link_info_t link_infor;
    memset(&jap_cfg,0,sizeof(atwlan_jap_cfg_t));
    memset(&link_infor,0,sizeof(s907x_link_info_t));
    
    PROCESS_ERROR(0 == atwlan_get_system_ready());
    
    mode = s907x_wlan_get_mode();
    PROCESS_ERROR(!((mode == S907X_MODE_STA) || (mode == S907X_MODE_STA_AP)));

    ASSERT(cmd);
    if(!cmd->setsize && cmd->mode == AT_MODE_R){
        s907x_wlan_get_link_infor(&link_infor);
        if(link_infor.is_connected){
            at_rsp(AT_WLAN_JAP_CUR, 
                    link_infor.ssid, 
                    link_infor.bssid[0], link_infor.bssid[1], link_infor.bssid[2], 
                    link_infor.bssid[3], link_infor.bssid[4], link_infor.bssid[5],
                    link_infor.channel, 
                    link_infor.rssi);
        }
        else{
            at_rsp(AT_WLAN_JAP_NO_AP);
        }
        PROCESS_SUCCESS(1);
    }
    else 
        PROCESS_ERROR(cmd->mode != AT_MODE_W);
    
    argc = at_get_param(argv, cmd->set);
	
    jap_cfg.bssid = bssid;
    ret = atwlan_get_args_cwjap(argc, argv, &jap_cfg);
    PROCESS_ERROR(AT_RET_OK != ret);

    ret = atwlan_cwjap(&jap_cfg, &result);
    PROCESS_SUCCESS(AT_RET_OK == ret);
    
err:
    at_rsp(AT_ERROR);
    return;    
    
success:
#if WLAN_PARA_SAVE
    //recon_en = 1;
    //flash_write(WIFI_RECON_ADDR, &recon_en, WIFI_RECON_SIZE);
#endif
    at_rsp(AT_OK);
}     

//AT+CWJAP_DEF 连接 AP，保存到 Flash
static void atwlan_cwjap_def (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    int ret = AT_RET_ERR;
    char *argv[AT_SET_MAX_ARGC]={0};
    int argc;
    u8 mode;
    u32 result;
    u8 recon_en;
    u8 bssid[6]={0};
	
    atwlan_jap_cfg_t jap_cfg;
    s907x_link_info_t link_infor;
    atwlan_sta_cfg_def_t sta_cfg;
    
    memset(&jap_cfg,0,sizeof(atwlan_jap_cfg_t));
    memset(&link_infor,0,sizeof(s907x_link_info_t));
    memset(&sta_cfg,0,sizeof(atwlan_sta_cfg_def_t));
    memset(&link_infor,0,sizeof(s907x_link_info_t));

    PROCESS_ERROR(0 == atwlan_get_system_ready());

    mode = s907x_wlan_get_mode();
    PROCESS_ERROR(!((mode == S907X_MODE_STA) || (mode == S907X_MODE_STA_AP)));
    
    ASSERT(cmd);
    if(!cmd->setsize && cmd->mode == AT_MODE_R){
        s907x_wlan_get_link_infor(&link_infor);
        if(link_infor.is_connected){
            at_rsp(AT_WLAN_JAP_DEF, 
                    link_infor.ssid, 
                    link_infor.bssid[0], link_infor.bssid[1], link_infor.bssid[2], 
                    link_infor.bssid[3], link_infor.bssid[4], link_infor.bssid[5],
                    link_infor.channel,
                     link_infor.rssi);
        }
        else{
            at_rsp(AT_WLAN_JAP_NO_AP);
        }
        PROCESS_SUCCESS(1);
    }
    else 
        PROCESS_ERROR(cmd->mode != AT_MODE_W);
    
    argc = at_get_param(argv, cmd->set);
	jap_cfg.bssid = bssid;
    ret = atwlan_get_args_cwjap(argc, argv, &jap_cfg);
    PROCESS_ERROR(AT_RET_OK != ret);

    ret = atwlan_cwjap(&jap_cfg,&result);
    PROCESS_SAVE_FLASH(AT_RET_OK == ret);
	
err:
	memset(&sta_cfg,0,sizeof(atwlan_sta_cfg_def_t));
	flash_write(WIFI_JAP_DEF_ADDR, (u8*)&sta_cfg, WIFI_JAP_DEF_SIZE);
    at_rsp(AT_ERROR);
    return;    
save_to_flash:    
#if WLAN_PARA_SAVE
    //recon_en = 1;
    strcpy(sta_cfg.ssid,jap_cfg.ssid);
    strcpy(sta_cfg.password,jap_cfg.password);
    memcpy(sta_cfg.bssid, jap_cfg.bssid, AT_WLAN_BSSID_LEN);
    sta_cfg.bssid_set = jap_cfg.bssid_set;
    AT_DBG_MSG("%s,%s", sta_cfg.ssid, sta_cfg.password);
    flash_write(WIFI_JAP_DEF_ADDR, (u8*)&sta_cfg, WIFI_JAP_DEF_SIZE);

    //flash_write(WIFI_RECON_ADDR, &recon_en, WIFI_RECON_SIZE);        
#endif

success:
    at_rsp(AT_OK);
}     

//AT+CWLAPOPT 设置 AT+CWLAP 指令扫描结果的属性
static void atwlan_cwlapopt     (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    char *argv[AT_SET_MAX_ARGC]={0};
    int argc;
    u8 index=0;
    int sort_enable, mask;
    
    PROCESS_ERROR(0 == atwlan_get_system_ready());
    
    ASSERT(cmd);
    PROCESS_ERROR(cmd->mode != AT_MODE_W);

    argc = at_get_param(argv, cmd->set);
    PROCESS_ERROR(argc != 2);
    
    sort_enable = atoi(argv[index]);
    PROCESS_ERROR(!((0 <= sort_enable) && (sort_enable <= 1)));
    index++;

    mask = atoi(argv[index]);
    PROCESS_ERROR(!(0 <= mask));
    scan_print_mask.mask = mask;
    
    PROCESS_SUCCESS(1);
    
err:
    at_rsp(AT_ERROR);
    return;
    
success:
    at_rsp(AT_OK);
}     

//AT+CWLAP 扫描附近的 AP 信息    
static void atwlan_cwlap (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    char *argv[AT_SET_MAX_ARGC]={0};
    int argc,ret=AT_RET_ERR;
    u8 index=0,bssid[6],mode;
    int channel,scan_type,scan_time_min=0,scan_time_max=0;
    struct scan_config scan_cfg;

    PROCESS_ERROR(0 == atwlan_get_system_ready());

    mode = s907x_wlan_get_mode();
    PROCESS_ERROR(!((mode == S907X_MODE_STA) || (mode == S907X_MODE_STA_AP)));

    ASSERT(cmd);
    if(cmd->mode == AT_MODE_ACT){
        wl_init_sema(&scan_target.sema, 0, sema_binary);
        ret = s907x_wlan_scan(s907x_wlan_scan_cb, S907X_DEFAULT_SCAN_AP_NUMS, NULL);
        if(ret) {
            wl_free_sema(&scan_target.sema);
            PROCESS_FAIL(ret);
        }
        wl_wait_sema(&scan_target.sema, portMAX_DELAY);     
        wl_free_sema(&scan_target.sema);
        PROCESS_SUCCESS(1);
    }
    else 
        PROCESS_ERROR(cmd->mode != AT_MODE_W);
    argc = at_get_param(argv, cmd->set);
    PROCESS_ERROR(!((1 <= argc) && (argc <= 6)));
    memset(&scan_cfg,0,sizeof(struct scan_config));
	
    ARGC_INDEX_VALID(){ 
        if(argv[index]){
		    PROCESS_ERROR(at_parse_param_str(&argv[index]));
		    PROCESS_ERROR(strlen(argv[index]) > AT_WLAN_SSID_LEN);
		    scan_cfg.ssid = argv[index];
        }
    	index ++;
    }
    
    ARGC_INDEX_VALID(){
        if(argv[index]){
            PROCESS_ERROR(at_parse_param_str(&argv[index]));
            PROCESS_ERROR(strlen(argv[index]) != AT_WLAN_MAC_LEN);
            sscanf(argv[index], AT_WLAN_MAC_FMT, &bssid[0], &bssid[1], &bssid[2], &bssid[3],&bssid[4],&bssid[5]);
			scan_cfg.bssid = bssid;
        }
        index ++;
    }
    
    ARGC_INDEX_VALID(){
        if(argv[index]){
            channel = atoi(argv[index]);
            PROCESS_ERROR(!((1 <= channel) && (channel <= AT_WLAN_MAX_CHANNEL)));
        }
        index ++;
    }

    ARGC_INDEX_VALID(){
        if(argv[index]){
            scan_type = atoi(argv[index]);
            PROCESS_ERROR(!((0 <= scan_type) && (scan_type <= 1)));
        }
        index ++;
    }

    ARGC_INDEX_VALID(){
        if(argv[index]){
            scan_time_min = atoi(argv[index]);
            PROCESS_ERROR(!((0 <= scan_time_min) && (scan_time_min <= 1500)));
        }
        index ++;
    }

    ARGC_INDEX_VALID(){
        if(argv[index]){
            scan_time_max = atoi(argv[index]);
            PROCESS_ERROR(!((0 <= scan_time_max) && (scan_time_max <= 1500)));
        }
        index ++;
    }
    PROCESS_ERROR(scan_time_min > scan_time_max);
    
    wl_init_sema(&scan_target.sema, 0, sema_binary);
    ret = s907x_wlan_scan(s907x_wlan_scan_cb, S907X_DEFAULT_SCAN_AP_NUMS, &scan_cfg);
    if(ret) {
        wl_free_sema(&scan_target.sema);
        PROCESS_ERROR(ret);
    }
    wl_wait_sema(&scan_target.sema, portMAX_DELAY);     
    wl_free_sema(&scan_target.sema);
    PROCESS_SUCCESS(1);

fail:
    at_rsp(AT_FAIL);
    return;
err:
    at_rsp(AT_ERROR);
    return;

success:
    at_rsp(AT_OK);
} 

//AT+CWQAP 与 AP 断开连接    
static void atwlan_cwqap (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    int ret=AT_RET_ERR;
    u8 mode;

    PROCESS_ERROR(0 == atwlan_get_system_ready());

    mode = s907x_wlan_get_mode();
    PROCESS_ERROR(!((mode == S907X_MODE_STA) || (mode == S907X_MODE_STA_AP)));
    
    ASSERT(cmd);
    if(cmd->mode == AT_MODE_ACT){
        ret = s907x_wlan_stop_sta();
        PROCESS_SUCCESS(!ret);
    }
    else
        PROCESS_ERROR(1);
    
err:
    at_rsp(AT_ERROR);
    return;

success:
    at_rsp(AT_OK);
}     

//AT+CWSAP_CUR 设置 s907x SoftAP 配置，不保存到 Flash    
static void atwlan_cwsap_cur (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    int ret = AT_RET_ERR;
    char *argv[AT_SET_MAX_ARGC]={0};
    int argc;
    u8 mode;
    atwlan_sap_cfg_t sap_cfg;
    
    PROCESS_ERROR(0 == atwlan_get_system_ready());
    
    memset(&sap_cfg,0,sizeof(atwlan_sap_cfg_t));
    mode = s907x_wlan_get_mode();
    PROCESS_ERROR(!((mode == S907X_MODE_AP) || (mode == S907X_MODE_STA_AP)));

    ASSERT(cmd);
    if(!cmd->setsize && cmd->mode == AT_MODE_R){
		PROCESS_SUCCESS(!g_atwlan_cur_ap_cfg.channel);
        at_rsp(AT_WLAN_SAP_CUR, 
            g_atwlan_cur_ap_cfg.ssid, 
            g_atwlan_cur_ap_cfg.password, 
            g_atwlan_cur_ap_cfg.channel, 
            g_atwlan_cur_ap_cfg.authmode, 
            g_atwlan_cur_ap_cfg.max_conn, 
            g_atwlan_cur_ap_cfg.is_hidded_ssid);
        PROCESS_SUCCESS(1);
    }
    else 
        PROCESS_ERROR(cmd->mode != AT_MODE_W);

    argc = at_get_param(argv, cmd->set);
    ret = atwlan_get_args_cwsap(argc, argv, mode, &sap_cfg);
    PROCESS_ERROR(AT_RET_OK != ret);

    ret = atwlan_cwsap(mode, &sap_cfg);
    PROCESS_SUCCESS(AT_RET_OK == ret);
err:
    at_rsp(AT_ERROR);
    return;

success:
    sap_cfg.max_conn = AT_WLAN_MAX_STA;
    at_rsp(AT_OK);
}     

//AT+CWSAP_DEF 设置 s907x SoftAP模式 (STA/AP/STA+AP)，保存到 Flash    
static void atwlan_cwsap_def (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    int ret = AT_RET_ERR;
    char *argv[AT_SET_MAX_ARGC]={0};
    int argc;
    u8 mode;
    atwlan_sap_cfg_t sap_cfg;
    memset(&sap_cfg,0,sizeof(atwlan_sap_cfg_t));
    
    PROCESS_ERROR(0 == atwlan_get_system_ready());
    
    mode = s907x_wlan_get_mode();
    PROCESS_ERROR(!((mode == S907X_MODE_AP) || (mode == S907X_MODE_STA_AP)));
    
    ASSERT(cmd);
    if(!cmd->setsize && cmd->mode == AT_MODE_R){
        #if WLAN_PARA_SAVE
        flash_read(WIFI_SAP_DEF_ADDR, (u8*)&sap_cfg, WIFI_SAP_DEF_SIZE);
        #endif
        at_rsp(AT_WLAN_SAP_CUR, 
            sap_cfg.ssid, 
            sap_cfg.password, 
            sap_cfg.channel, 
            sap_cfg.authmode, 
            sap_cfg.max_conn, 
            sap_cfg.is_hidded_ssid);
        PROCESS_SUCCESS(1);
    }
    else 
        PROCESS_ERROR(cmd->mode != AT_MODE_W);

    argc = at_get_param(argv, cmd->set);
    ret = atwlan_get_args_cwsap(argc, argv, mode, &sap_cfg);
    PROCESS_ERROR(AT_RET_OK != ret);

    ret = atwlan_cwsap(mode, &sap_cfg);
    PROCESS_SAVE_FLASH(AT_RET_OK == ret);
    
err:
    at_rsp(AT_ERROR);
    return;
    
save_to_flash: 
#if WLAN_PARA_SAVE    
    sap_cfg.max_conn = AT_WLAN_MAX_STA;
    flash_write(WIFI_SAP_DEF_ADDR, (u8*)&sap_cfg, WIFI_SAP_DEF_SIZE);
#endif

success:
    at_rsp(AT_OK);
}     

//AT+CWLIF 获取连接到 s907x SoftAP 的 station 的信息    
static void atwlan_cwlif (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    u8 mode;
    u8 cli_idx = 0, sta_idx = 0, num = 0;
    u8 *hw_addr = NULL;    
    s907x_ap_client_infor_t *ap_cli_info=NULL;

    PROCESS_ERROR(0 == atwlan_get_system_ready());

	mode = s907x_wlan_get_mode();
	PROCESS_ERROR(!((mode == S907X_MODE_AP) || (mode == S907X_MODE_STA_AP)));
	
	ASSERT(cmd);
	if(cmd->mode == AT_MODE_ACT){	
		num = s907x_wlan_ap_get_client_nums();
		at_rsp(AT_WLAN_LIF_NULL);
		if(num){	
			ap_cli_info = (s907x_ap_client_infor_t *)wl_malloc(sizeof(s907x_ap_client_infor_t)*num);
			
			PROCESS_FAIL(!ap_cli_info);
			s907x_wlan_ap_get_client_infor(ap_cli_info,num);
			
			for(cli_idx=0;cli_idx<num;cli_idx++){
				for(sta_idx=0;sta_idx<AT_WLAN_MAX_STA;sta_idx++){
					hw_addr = ap_cli_info[cli_idx].hw_addr;
					if(!memcmp(sta_info[sta_idx].bssid,hw_addr,AT_WLAN_BSSID_LEN)){
						at_rsp(AT_WLAN_LIF,
								inet_ntoa(sta_info[sta_idx].ip),
								hw_addr[0], hw_addr[1], hw_addr[2], 
								hw_addr[3], hw_addr[4], hw_addr[5]);
						break;
					}
				}
			}
			wl_free(ap_cli_info);
		}
			
		PROCESS_SUCCESS(1);
	}
	PROCESS_ERROR(1);
fail:
    at_rsp(AT_FAIL);
    return;
        
err:
    at_rsp(AT_ERROR);
    return;
    
success:
    at_rsp(AT_OK);
}     

//AT+CWDHCP_CUR 设置 DHCP，不保存到 Flash
static void atwlan_cwdhcp_cur (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    int ret = AT_RET_ERR;
    char *argv[AT_SET_MAX_ARGC]={0};
    int argc;
    u8 enable=0, dhcp_if=0;
    
    PROCESS_ERROR(0 == atwlan_get_system_ready());
    
    ASSERT(cmd);
    if(!cmd->setsize && cmd->mode == AT_MODE_R){
        at_rsp(AT_WLAN_DHCP_CUR, g_atwlan_dhcp_stat);
        PROCESS_SUCCESS(1);
    }
    else 
        PROCESS_ERROR(cmd->mode != AT_MODE_W);
    
    argc = at_get_param(argv, cmd->set);
    ret = atwlan_get_args_cwdhcp(argc, argv, &dhcp_if, &enable);
    PROCESS_ERROR(AT_RET_OK != ret);

    atwlan_cwdhcp(dhcp_if, enable);

    PROCESS_SUCCESS(1);

err:
    at_rsp(AT_ERROR);
    return;

success:
    at_rsp(AT_OK);
}     

//AT+CWDHCP_DEF 设置 DHCP，保存到 Flash
static void atwlan_cwdhcp_def (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    int ret = AT_RET_ERR;
    char *argv[AT_SET_MAX_ARGC]={0};
    int argc;
    u8 dhcp_stat = 0, dhcp_if=0, enable=0;

    PROCESS_ERROR(0 == atwlan_get_system_ready());

    ASSERT(cmd);
    if(!cmd->setsize && cmd->mode == AT_MODE_R){
        #if WLAN_PARA_SAVE
        flash_read(WIFI_DHCP_DEF_ADDR, &dhcp_stat, WIFI_DHCP_DEF_SIZE);
        #endif
        at_rsp(AT_WLAN_DHCP_DEF, dhcp_stat);
        PROCESS_SUCCESS(1);
    }
    else 
        PROCESS_ERROR(cmd->mode != AT_MODE_W);
    
    argc = at_get_param(argv, cmd->set);
    ret = atwlan_get_args_cwdhcp(argc, argv, &dhcp_if, &enable);
    PROCESS_ERROR(AT_RET_OK != ret);

    atwlan_cwdhcp(dhcp_if, enable);

    PROCESS_SAVE_FLASH(1);

err:
    at_rsp(AT_ERROR);
    return;
    
save_to_flash:    
#if WLAN_PARA_SAVE
    flash_write(WIFI_DHCP_DEF_ADDR, &g_atwlan_dhcp_stat, WIFI_DHCP_DEF_SIZE);
#endif

success:
    at_rsp(AT_OK);
}     

//AT+CWDHCPS_CUR 设置 s907x SoftAP DHCP 分配的 IP 范围，不保存到 Flash
static void atwlan_cwdhcps_cur (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    int ret = AT_RET_ERR;
    char *argv[AT_SET_MAX_ARGC]={0};
    int argc;
    u8 mode;
    atwlan_dhcps_lease_pool_t dhcps_lease;
    char start_ip[IP4ADDR_STRLEN_MAX] = {0};
    char end_ip[IP4ADDR_STRLEN_MAX] = {0};
    
    PROCESS_ERROR(0 == atwlan_get_system_ready());
    
    memset(&dhcps_lease,0,sizeof(atwlan_dhcps_lease_pool_t));
    mode = s907x_wlan_get_mode();
    PROCESS_ERROR(!((mode == S907X_MODE_AP) || (mode == S907X_MODE_STA_AP)));
    PROCESS_ERROR(!(g_atwlan_dhcp_stat & AT_AP_DHCP_EN));
    
    ASSERT(cmd);
    if(!cmd->setsize && cmd->mode == AT_MODE_R){
        dhcps_get_lease_time(&dhcps_lease.lease_time);
        dhcps_get_addr_pool(&dhcps_lease.start_ip, &dhcps_lease.end_ip);
        at_rsp(AT_WLAN_DHCPS_CUR, 
                (dhcps_lease.lease_time/60), 
                inet_ntoa_r(dhcps_lease.start_ip, start_ip, IP4ADDR_STRLEN_MAX), 
                inet_ntoa_r(dhcps_lease.end_ip, end_ip, IP4ADDR_STRLEN_MAX));
        PROCESS_SUCCESS(1); 
    }
    else 
        PROCESS_ERROR(cmd->mode != AT_MODE_W);
    
    argc = at_get_param(argv, cmd->set);
    ret = atwlan_get_args_cwdhcps(argc, argv, &dhcps_lease);
    PROCESS_ERROR(AT_RET_OK !=  ret);
    
	ret = atwlan_cwdhcps(&dhcps_lease);
    PROCESS_SUCCESS(AT_RET_OK == ret);
    
err:
    at_rsp(AT_ERROR);
    return;
    
success:
    at_rsp(AT_OK);
}     

//AT+CWDHCPS_DEF 设置 s907x SoftAP DHCP 分配的 IP 范围，保存到 Flash    
static void atwlan_cwdhcps_def (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    int ret = AT_RET_ERR;
    char *argv[AT_SET_MAX_ARGC]={0};
    int argc;
    u8 mode;
    atwlan_dhcps_lease_pool_t dhcps_lease;
    char start_ip[IP4ADDR_STRLEN_MAX] = {0};
    char end_ip[IP4ADDR_STRLEN_MAX] = {0};    

    PROCESS_ERROR(0 == atwlan_get_system_ready());

    memset(&dhcps_lease,0,sizeof(atwlan_dhcps_lease_pool_t));

    mode = s907x_wlan_get_mode();
    PROCESS_ERROR(!((mode == S907X_MODE_AP) || (mode == S907X_MODE_STA_AP)));
    PROCESS_ERROR(!(g_atwlan_dhcp_stat & AT_AP_DHCP_EN));

    ASSERT(cmd);
    if(!cmd->setsize && cmd->mode == AT_MODE_R){
    #if WLAN_PARA_SAVE
        flash_read(WIFI_DHCPS_DEF_ADDR, (u8*)&dhcps_lease, WIFI_DHCPS_DEF_SIZE);
    #endif
        at_rsp(AT_WLAN_DHCPS_DEF, 
                (dhcps_lease.lease_time/60), 
                inet_ntoa_r(dhcps_lease.start_ip, start_ip, IP4ADDR_STRLEN_MAX), 
                inet_ntoa_r(dhcps_lease.end_ip, end_ip, IP4ADDR_STRLEN_MAX));
        PROCESS_SUCCESS(1);
    }
    else 
        PROCESS_ERROR(cmd->mode != AT_MODE_W);

    argc = at_get_param(argv, cmd->set);
    ret = atwlan_get_args_cwdhcps(argc, argv, &dhcps_lease);
    PROCESS_ERROR(AT_RET_OK != ret);

	ret = atwlan_cwdhcps(&dhcps_lease);
    PROCESS_SAVE_FLASH(AT_RET_OK == ret);
    
err:
    at_rsp(AT_ERROR);
    return;

save_to_flash:    
#if WLAN_PARA_SAVE
    flash_write(WIFI_DHCPS_DEF_ADDR, (u8*)&dhcps_lease, WIFI_DHCPS_DEF_SIZE);
#endif

success:
    at_rsp(AT_OK);
}     

//AT+CWAUTOCONN 设置上电时是否自动连接 AP    
static void atwlan_cwautoconn (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    char *argv[AT_SET_MAX_ARGC]={0};
    int argc;
    int enable,index=0;
    u8 enable_flash;
    
    PROCESS_ERROR(0 == atwlan_get_system_ready());
    
    ASSERT(cmd);
    PROCESS_ERROR(cmd->mode != AT_MODE_W);
    
    argc = at_get_param(argv, cmd->set);
    PROCESS_ERROR(argc != 1);
    
    enable = atoi(argv[index]);
    PROCESS_ERROR(!((0 <= enable) && (enable <= 1)));
    PROCESS_SAVE_FLASH(1);
    
err:
    at_rsp(AT_ERROR);
    return;
    
save_to_flash:        
#if WLAN_PARA_SAVE
    enable_flash = (u8)enable;
    flash_write(WIFI_RECON_ADDR, &enable_flash, WIFI_RECON_SIZE);        
#endif
    at_rsp(AT_OK);
}     

//AT+CIPSTAMAC_CUR 设置 s907x Station 的 MAC 地址，不保存到 Flash    
static void atwlan_cipstamac_cur (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    char *argv[AT_SET_MAX_ARGC]={0};
    int argc,ret=AT_RET_ERR;
    u8 index=0,mac[6],mode;
    
    PROCESS_ERROR(0 == atwlan_get_system_ready());
    
    mode = s907x_wlan_get_mode();
    PROCESS_ERROR(!((mode == S907X_MODE_STA) || (mode == S907X_MODE_STA_AP)));
    
    ASSERT(cmd);
    if(!cmd->setsize && cmd->mode == AT_MODE_R){
        ret = s907x_wlan_get_mac_address(0,mac);
        if(!ret){
            at_rsp(AT_WLAN_IPSTAMAC_CUR,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
            PROCESS_SUCCESS(1);
        }
    }
    else 
        PROCESS_ERROR(cmd->mode != AT_MODE_W);
    
    argc = at_get_param(argv, cmd->set);
    PROCESS_ERROR(argc != 1);

    PROCESS_ERROR(at_parse_param_str(&argv[index]));
    PROCESS_ERROR(strlen(argv[index]) > AT_WLAN_MAC_LEN);
    
    sscanf(argv[index], AT_WLAN_MAC_FMT, &mac[0], &mac[1], &mac[2],&mac[3], &mac[4], &mac[5]);
    ret = s907x_wlan_set_mac_address_no_efuse(0,mac);
    PROCESS_SUCCESS(!ret);

err:
    at_rsp(AT_ERROR);
    return;
    
success:
    at_rsp(AT_OK);
}     

//AT+CIPSTAMAC_DEF 设置 s907x Station 的 MAC 地址，保存到 Flash    
static void atwlan_cipstamac_def (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    char *argv[AT_SET_MAX_ARGC]={0};
    int argc,ret=AT_RET_ERR;
    u8 index=0,mac[6],mode;
    
    PROCESS_ERROR(0 == atwlan_get_system_ready());
    
    mode = s907x_wlan_get_mode();
    PROCESS_ERROR(!((mode == S907X_MODE_STA) || (mode == S907X_MODE_STA_AP)));
    
    ASSERT(cmd);
    if(!cmd->setsize && cmd->mode == AT_MODE_R){
    #if WLAN_PARA_SAVE
        flash_read(WIFI_STA_MAC_DEF_ADDR, mac, WIFI_STA_MAC_DEF_SIZE);        
    #endif
        at_rsp(AT_WLAN_IPSTAMAC_DEF,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
        PROCESS_SUCCESS(1);
    }
    else 
        PROCESS_ERROR(cmd->mode != AT_MODE_W);

    argc = at_get_param(argv, cmd->set);
    PROCESS_ERROR(argc != 1);
    
    PROCESS_ERROR(at_parse_param_str(&argv[index]));
    PROCESS_ERROR(strlen(argv[index]) > AT_WLAN_MAC_LEN);
    
    sscanf(argv[index], AT_WLAN_MAC_FMT, &mac[0], &mac[1], &mac[2],&mac[3], &mac[4], &mac[5]);
    
    ret = s907x_wlan_set_mac_address(0,mac);
    PROCESS_SAVE_FLASH(!ret);

err:
    at_rsp(AT_ERROR);
    return;

save_to_flash:    
#if WLAN_PARA_SAVE
    flash_write(WIFI_STA_MAC_DEF_ADDR, mac, WIFI_STA_MAC_DEF_SIZE);     
#endif
    
success:
    at_rsp(AT_OK);
}     

//AT+CIPAPMAC_CUR 设置 s907x SoftAP 的 MAC 地址，不保存到 Flash        
static void atwlan_cipapmac_cur (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    char *argv[AT_SET_MAX_ARGC]={0};
    int argc,ret=AT_RET_ERR;
    u8 if_idx,mac[6],index=0,mode;
    
    PROCESS_ERROR(0 == atwlan_get_system_ready());
        
    mode = s907x_wlan_get_mode();
    PROCESS_ERROR(!((mode == S907X_MODE_AP) || (mode == S907X_MODE_STA_AP)));
    
    if_idx=at_wlan_get_if_idx(AT_WLAN_AP);
    
    ASSERT(cmd);
    if(!cmd->setsize && cmd->mode == AT_MODE_R){
        ret = s907x_wlan_get_mac_address(if_idx,mac);
        if(!ret){
            at_rsp(AT_WLAN_IPAPMAC_CUR,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
            PROCESS_SUCCESS(1);
        }
    }
    else 
        PROCESS_ERROR(cmd->mode != AT_MODE_W);

    argc = at_get_param(argv, cmd->set);
    PROCESS_ERROR(argc != 1);
    
    PROCESS_ERROR(at_parse_param_str(&argv[index]));
    PROCESS_ERROR(strlen(argv[index]) > AT_WLAN_MAC_LEN);
    
    sscanf(argv[index], AT_WLAN_MAC_FMT, &mac[0], &mac[1], &mac[2],&mac[3], &mac[4], &mac[5]);
    ret = s907x_wlan_set_mac_address_no_efuse(if_idx,mac);
    PROCESS_SUCCESS(!ret);

err:
    at_rsp(AT_ERROR);
    return;
    
success:
    at_rsp(AT_OK);
}     

//AT+CIPAPMAC_DEF 设置 s907x SoftAP 的 MAC 地址，保存到 Flash    
static void atwlan_cipapmac_def (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    char *argv[AT_SET_MAX_ARGC]={0};
    int argc, ret=AT_RET_ERR;
    u8 if_idx, index=0, mac[6]={0}, mode=0;
    
    PROCESS_ERROR(0 == atwlan_get_system_ready());
    
    mode = s907x_wlan_get_mode();
    PROCESS_ERROR(!((mode == S907X_MODE_AP) || (mode == S907X_MODE_STA_AP)));

    if_idx=at_wlan_get_if_idx(AT_WLAN_AP);
    
    ASSERT(cmd);
    if(!cmd->setsize && cmd->mode == AT_MODE_R){
    #if WLAN_PARA_SAVE
        flash_read(WIFI_AP_MAC_DEF_ADDR, mac, WIFI_AP_MAC_DEF_SIZE);        
    #endif
        at_rsp(AT_WLAN_IPAPMAC_DEF,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
        PROCESS_SUCCESS(1);
    }
    else 
        PROCESS_ERROR(cmd->mode != AT_MODE_W);
    
    argc = at_get_param(argv, cmd->set);
    PROCESS_ERROR(argc != 1);
    
    PROCESS_ERROR(at_parse_param_str(&argv[index]));
    PROCESS_ERROR(strlen(argv[index]) > AT_WLAN_MAC_LEN);
    
    sscanf(argv[index], AT_WLAN_MAC_FMT, &mac[0], &mac[1], &mac[2],&mac[3], &mac[4], &mac[5]);
    ret = s907x_wlan_set_mac_address(if_idx,mac);
    PROCESS_SAVE_FLASH(!ret);

err:
    at_rsp(AT_ERROR);
    return;
    
save_to_flash: 
#if WLAN_PARA_SAVE
    flash_write(WIFI_AP_MAC_DEF_ADDR, mac, WIFI_AP_MAC_DEF_SIZE);        
#endif
    
success:
    at_rsp(AT_OK);
}     

//AT+CIPSTA_CUR 设置 s907x Station 的 IP 地址，不保存到 Flash    
static void atwlan_cipsta_cur (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    int ret = AT_RET_ERR;
    char *argv[AT_SET_MAX_ARGC]={0};
    int argc;
    u8 mode;
    u8 if_idx;
    atwlan_if_addr_cfg_t if_addr;
    memset(&if_addr,0,sizeof(atwlan_if_addr_cfg_t));
    
    PROCESS_ERROR(0 == atwlan_get_system_ready());
    
    ASSERT(cmd);
    mode = s907x_wlan_get_mode();
    PROCESS_ERROR(!((mode == S907X_MODE_STA) || (mode == S907X_MODE_STA_AP)));
    if_idx = at_wlan_get_if_idx(AT_WLAN_STA);
    at_wlan_get_if_addr(if_idx, &if_addr);
    if(!cmd->setsize && cmd->mode == AT_MODE_R){
        if (!netif_is_link_up(LwIP_GetNetif(if_idx))) {
            memset(&if_addr, 0, sizeof(atwlan_if_addr_cfg_t));
        }
        at_rsp(AT_WLAN_IPSTA_CUR, AT_WLAN_IPADDR, inet_ntoa(if_addr.ip));
        at_rsp(AT_WLAN_IPSTA_CUR, AT_WLAN_GATEWAY, inet_ntoa(if_addr.gw));
        at_rsp(AT_WLAN_IPSTA_CUR, AT_WLAN_NETMASK, inet_ntoa(if_addr.netmask));
        PROCESS_SUCCESS(1);
    }
    else 
        PROCESS_ERROR(cmd->mode != AT_MODE_W);

    argc = at_get_param(argv, cmd->set);
    ret = atwlan_get_args_cipsta(argc, argv, &if_addr);
    PROCESS_ERROR(AT_RET_OK != ret);

    atwlan_cipsta(if_idx, &if_addr);
    PROCESS_SUCCESS(1);

err:
    at_rsp(AT_ERROR);
    return;
    
success:
    at_rsp(AT_OK);
}     

//AT+CIPSTA_DEF 设置 s907x Station 的 IP 地址，保存到 Flash
static void atwlan_cipsta_def (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    int ret = AT_RET_ERR;
    char *argv[AT_SET_MAX_ARGC]={0};
    int argc;
    u8 mode=0;
    u8 if_idx;
    u8 dhcp_stat = 0;
    atwlan_if_addr_cfg_t if_addr;
    memset(&if_addr,0,sizeof(atwlan_if_addr_cfg_t));
    
    PROCESS_ERROR(0 == atwlan_get_system_ready());
    
    ASSERT(cmd);
    mode = s907x_wlan_get_mode();
    PROCESS_ERROR(!((mode == S907X_MODE_STA) || (mode == S907X_MODE_STA_AP)));
    if(!cmd->setsize && cmd->mode == AT_MODE_R){
    #if WLAN_PARA_SAVE
        flash_read(WIFI_STA_IP_ADDR_DEF_ADDR, (u8*)&if_addr, WIFI_STA_IP_ADDR_DEF_SIZE);
    #endif
        at_rsp(AT_WLAN_IPSTA_DEF, AT_WLAN_IPADDR, inet_ntoa(if_addr.ip));
        at_rsp(AT_WLAN_IPSTA_DEF, AT_WLAN_GATEWAY, inet_ntoa(if_addr.gw));
        at_rsp(AT_WLAN_IPSTA_DEF, AT_WLAN_NETMASK, inet_ntoa(if_addr.netmask));
        PROCESS_SUCCESS(1);
    }
    else 
        PROCESS_ERROR(cmd->mode != AT_MODE_W);
    
    if_idx = at_wlan_get_if_idx(AT_WLAN_STA);
    at_wlan_get_if_addr(if_idx, &if_addr);
    argc = at_get_param(argv, cmd->set);
    ret = atwlan_get_args_cipsta(argc, argv, &if_addr);
    PROCESS_ERROR(AT_RET_OK != ret);
    
    atwlan_cipsta(if_idx, &if_addr);
    dhcp_stat = g_atwlan_dhcp_stat;
    AT_CLR_BIT(dhcp_stat, AT_STA_DHCP_EN);
    PROCESS_SAVE_FLASH(1);
    
err:
    at_rsp(AT_ERROR);
    return;

save_to_flash:
#if WLAN_PARA_SAVE
    flash_write(WIFI_STA_IP_ADDR_DEF_ADDR, (u8*)&if_addr, WIFI_STA_IP_ADDR_DEF_SIZE);
    flash_write(WIFI_DHCP_DEF_ADDR, &dhcp_stat, WIFI_DHCP_DEF_SIZE);
#endif

success:
    at_rsp(AT_OK);
}     

//AT+CIPAP_CUR 设置 s907x SoftAP 的 IP 地址，不保存到 Flash    
static void atwlan_cipap_cur (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    int ret = AT_RET_ERR;
    char *argv[AT_SET_MAX_ARGC]={0};
    int argc;
    u8 if_idx = S907X_DEV0_ID,mode;
    u8 flush_sta = FALSE;

    atwlan_if_addr_cfg_t orig_if_addr;
    atwlan_if_addr_cfg_t new_if_addr;
    memset(&orig_if_addr,0,sizeof(atwlan_if_addr_cfg_t));
    memset(&new_if_addr,0,sizeof(atwlan_if_addr_cfg_t));
    
    PROCESS_ERROR(0 == atwlan_get_system_ready());
    
    ASSERT(cmd);
    mode = s907x_wlan_get_mode();
    PROCESS_ERROR(!((mode == S907X_MODE_AP) || (mode == S907X_MODE_STA_AP)));

    if_idx = at_wlan_get_if_idx(AT_WLAN_AP);
    at_wlan_get_if_addr(if_idx, &orig_if_addr);
    if(!cmd->setsize && cmd->mode == AT_MODE_R){
        at_rsp(AT_WLAN_IPAP_CUR, AT_WLAN_IPADDR, inet_ntoa(orig_if_addr.ip));
        at_rsp(AT_WLAN_IPAP_CUR, AT_WLAN_GATEWAY, inet_ntoa(orig_if_addr.gw));
        at_rsp(AT_WLAN_IPAP_CUR, AT_WLAN_NETMASK, inet_ntoa(orig_if_addr.netmask));
        PROCESS_SUCCESS(1);
    }
    else 
        PROCESS_ERROR(cmd->mode != AT_MODE_W);

    flush_sta = TRUE;
    argc = at_get_param(argv, cmd->set);
    ret = atwlan_get_args_cipap(argc, argv, &new_if_addr);
    PROCESS_ERROR(AT_RET_OK != ret);

    atwlan_cipap(if_idx, &orig_if_addr, &new_if_addr);
    PROCESS_SUCCESS(1);


err:
    at_rsp(AT_ERROR);
    return;

success:
    at_rsp(AT_OK);
    if (flush_sta) {
        at_wlan_flush_sta();
    }
}     

//AT+CIPAP_DEF 设置 s907x SoftAP 的 IP 地址，保存到 Flash    
static void atwlan_cipap_def (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    int ret = AT_RET_ERR;
    char *argv[AT_SET_MAX_ARGC]={0};
    int argc;
    u8 if_idx=S907X_DEV0_ID,mode;
    u8 flush_sta = FALSE;
    
    atwlan_if_addr_cfg_t orig_if_addr = {0};
    atwlan_if_addr_cfg_t new_if_addr = {0};
    memset(&orig_if_addr,0,sizeof(atwlan_if_addr_cfg_t));
    memset(&new_if_addr,0,sizeof(atwlan_if_addr_cfg_t));    
    
    PROCESS_ERROR(0 == atwlan_get_system_ready());
    
    ASSERT(cmd);
    mode = s907x_wlan_get_mode();
    PROCESS_ERROR(!((mode == S907X_MODE_AP) || (mode == S907X_MODE_STA_AP)));
    
    if(!cmd->setsize && cmd->mode == AT_MODE_R){
    #if WLAN_PARA_SAVE
        flash_read(WIFI_AP_IP_ADDR_DEF_ADDR, (u8*)&orig_if_addr, WIFI_AP_IP_ADDR_DEF_SIZE);
    #endif
        at_rsp(AT_WLAN_IPAP_DEF, AT_WLAN_IPADDR, inet_ntoa(orig_if_addr.ip));
        at_rsp(AT_WLAN_IPAP_DEF, AT_WLAN_GATEWAY, inet_ntoa(orig_if_addr.gw));
        at_rsp(AT_WLAN_IPAP_DEF, AT_WLAN_NETMASK, inet_ntoa(orig_if_addr.netmask));
        PROCESS_SUCCESS(1);
    }
    else 
        PROCESS_ERROR(cmd->mode != AT_MODE_W);
    
    flush_sta = TRUE;
    if_idx = at_wlan_get_if_idx(AT_WLAN_AP);
    at_wlan_get_if_addr(if_idx, &orig_if_addr);
    argc = at_get_param(argv, cmd->set);
    ret = atwlan_get_args_cipap(argc, argv, &new_if_addr);
    PROCESS_ERROR(AT_RET_OK != ret);

    atwlan_cipap(if_idx, &orig_if_addr, &new_if_addr);
    PROCESS_SAVE_FLASH(1);

err:
    at_rsp(AT_ERROR);
    return;

save_to_flash:    
#if WLAN_PARA_SAVE
    flash_write(WIFI_AP_IP_ADDR_DEF_ADDR, (u8*)&new_if_addr, WIFI_AP_IP_ADDR_DEF_SIZE);
#endif    

success:
    at_rsp(AT_OK);
    if (flush_sta) {
        at_wlan_flush_sta();
    }
    
}     

static void atwlan_smartlink_connect(sc_got_data_t* sm_info)
{
    int ret = 0;
    int app_sock = 0;
    int tx_cnt = 0;
	struct sockaddr_in app_addr;
    char *pc_target = NULL;
    char app_data[AT_WLAN_SL_UDP_TOTAL_LEN] = {0};
    u8 sta_mac[AT_WLAN_SL_UDP_BSSID_LEN] = {0}; 
    s907x_sta_init_t init;
    atwlan_if_addr_cfg_t if_addr;
	s907x_scan_result_t scan_result;
		
	memset(&scan_result,0,sizeof(s907x_scan_result_t));
    memset(&init,0,sizeof(s907x_sta_init_t));
    memset(&if_addr,0,sizeof(atwlan_if_addr_cfg_t));
 
	printf("ssid:%s\r\n",sm_info->ssid);
	printf("password:%s\r\n",sm_info->password);

	s907x_wlan_scan_ssid(sm_info->ssid,strlen(sm_info->ssid),&scan_result);
    if(strcmp(scan_result.scan_info.ssid,sm_info->ssid)) { 
        printf("Scan fail.");
        return;
    }

    //set security
    init.security = scan_result.scan_info.security;
    //set connection mode
    init.conn.mode =  CONN_MODE_BLOCKING;
    init.conn.blocking_timeout = S907X_DEFAULT_CONN_TO;
    init.conn.result = S907X_IDLE;
    init.ssid = sm_info->ssid;
    init.ssid_len = strlen(init.ssid);
    init.password = sm_info->password;
    init.password_len = strlen(init.password);
    AT_DBG_MSG("Connect to related ap[%s] using pwd[%s].", sm_info->ssid, sm_info->password);
    ret = s907x_wlan_start_sta(&init);
    if(!ret){
		//printf("smartconfig connected wifi\r\n");
        if(g_atwlan_sta_ip_static == 0) {
            if (g_atwlan_dhcp_stat & AT_STA_DHCP_EN) {
                ret = dhcpc_start(S907X_DEV0_ID, DHCP_START);
				if(ret =! DHCP_ADDRESS_ASSIGNED)
					return;
            }
        }else{
            at_wlan_get_if_addr(S907X_DEV0_ID, &if_addr);
            if (0 == if_addr.ip.addr) {
                wl_os_mdelay(AT_WLAN_DELAY_TO_GET_IP);
            }

    		at_wlan_get_if_addr(S907X_DEV0_ID, &if_addr);
    		if (0 == if_addr.ip.addr) {
                AT_DBG_ERR("Fail to get ip address.");
    		}
    	}
        app_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (app_sock < 0) {
            AT_DBG_ERR("Create UDP socket fail.");
            return;
        }

        //conns->remote_host = inet_ntoa(conns->remote_addr);
        app_addr.sin_family = AF_INET;
        app_addr.sin_addr.s_addr = sm_info->phone_ip;
        app_addr.sin_port = htons(AT_WLAN_SL_UDP_SERVER_PORT);

        pc_target = app_data;
        *pc_target = strlen(sm_info->ssid) + strlen(sm_info->password) + AT_WLAN_SL_UDP_LEN_APEND_MAGIC;
        at_rsp("data len: %d.\r\n", *pc_target);

        pc_target += AT_WLAN_SL_UDP_BYTE0_LEN;
        s907x_wlan_get_mac_address(S907X_DEV0_ID, sta_mac);
        memcpy(pc_target, sta_mac, AT_WLAN_SL_UDP_BSSID_LEN);
        at_rsp("bssid[%2x:..:%2x]\r\n", pc_target[0], pc_target[5]);

        pc_target += AT_WLAN_SL_UDP_BSSID_LEN;
        at_wlan_get_if_addr(S907X_DEV0_ID, &if_addr);
        memcpy(pc_target, &if_addr.ip, AT_WLAN_SL_UDP_IP_LEN);
        at_rsp("ip address: %0x.\r\n", *(int *)pc_target);

        while (tx_cnt++ < AT_WLAN_SL_UDP_RETX_CNT) {
            ret = sendto(app_sock, app_data, AT_WLAN_SL_UDP_TOTAL_LEN, 0, (struct sockaddr *)&app_addr, sizeof(app_addr));
            AT_DBG_MSG("Send data to app, send len[%d].", ret);
            wl_os_mdelay(AT_WLAN_SL_UDP_PKT_INTERVAL);
        }

        close(app_sock);
    }
	else
		at_rsp("connect failed!!\r\n");
}

void atwlan_smartlink_cb(sl_status status, void *pdata)
{
    switch(status){
        case SL_STATUS_WAITTING: 
            printf("SL_STATUS_HIDDEN_ERR\n");
            break;
        case SL_STATUS_FINDING_CHANNEL: 
            printf("SL_STATUS_FINDING_CHANNEL\n");
            break;
        case SL_STATUS_GETTING_SSID_PSWD: 
            printf("SL_STATUS_GETTING_SSID_PSWD\n");
            break;
        case SL_STATUS_SUCCESS: 
            printf("SL_STATUS_SUCCESS\n");
            sc_got_data_t *sta_info = (sc_got_data_t*)pdata;
            atwlan_smartlink_connect(sta_info);
            break;
        case SL_STATUS_TIMEOUT: 
            printf("SL_STATUS_TIMEOUT\n");
            break;
        case SL_STATUS_HIDDEN_ERR: 
            printf("SL_STATUS_HIDDEN_ERR\n");
            break;
        case SL_STATUS_OFFSET_ERR: 
            printf("SL_STATUS_OFFSET_ERR\n");
            break;   
    }
}


//AT+CWSTARTSMART 开始 SmartConfig        
static void atwlan_cwstartsmart     (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    u8 mode;
    PROCESS_ERROR(0 == atwlan_get_system_ready());
    mode = s907x_wlan_get_mode();
    PROCESS_ERROR(!(mode == S907X_MODE_STA));
	
    ASSERT(cmd);
    if(cmd->mode == AT_MODE_ACT){
		s907x_wlan_event_unreg(S907X_EVENT_STAMODE_SCAN_DONE, at_wlan_event_cb_scan_down);
        smart_link_start(atwlan_smartlink_cb,SMART_LINK_NO_TIME_LIMIT);

        PROCESS_SUCCESS(1);
    }
    else
        PROCESS_ERROR(1);
        
err:
    at_rsp(AT_ERROR);
    return;
    
success:
    at_rsp(AT_OK);
}     

//AT+CWSTOPSMART 停止 SmartConfig
static void atwlan_cwstopsmart (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    
    ASSERT(cmd);
    
    if(cmd->mode == AT_MODE_ACT){
        smart_link_stop();
		s907x_wlan_event_reg(S907X_EVENT_STAMODE_SCAN_DONE, at_wlan_event_cb_scan_down, NULL);
        PROCESS_SUCCESS(1);
    }
    else
        PROCESS_ERROR(1);

err:
    at_rsp(AT_ERROR);
    return;
    
success:
    at_rsp(AT_OK);
}     

//AT+CWSTARTDISCOVER 开启可被局域网内的微信探测的模式
static void atwlan_cwstartdiscover (void *context)
{
    at_rsp(AT_OK);
}     

//AT+CWSTOPDISCOVER 关闭可被局域网内的微信探测的模式    
static void atwlan_cwstopdiscover (void *context)
{
    at_rsp(AT_OK);
}     

//AT+WPS 设置 WPS 功能        
static void atwlan_wps  (void *context)
{
    at_rsp(AT_OK);
}     

//AT+MDNS 设置 MDNS 功能    
static void atwlan_mdns     (void *context)
{
    at_rsp(AT_OK);
}     

//AT+CWHOSTNAME 设置 s907x Station 的主机名称
static void atwlan_cwhostname (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    char *argv[AT_SET_MAX_ARGC]={0};
    int argc;
    u8 index=0;
    u8 mode;
    
    PROCESS_ERROR(0 == atwlan_get_system_ready());
    
    mode = s907x_wlan_get_mode();
    PROCESS_ERROR(!((mode == S907X_MODE_STA) || (mode == S907X_MODE_STA_AP)));
    
    ASSERT(cmd);
    if(!cmd->setsize && cmd->mode == AT_MODE_R){
#if LWIP_NETIF_HOSTNAME
        at_rsp(AT_WLAN_HOSTNAME,lwip_get_hostname(LwIP_GetNetif(S907X_DEV0_ID)));
        PROCESS_SUCCESS(1);
#endif
    }
    else 
        PROCESS_ERROR(cmd->mode != AT_MODE_W);
    
    argc = at_get_param(argv, cmd->set);
    PROCESS_ERROR(argc != 1);

#if LWIP_NETIF_HOSTNAME
    PROCESS_ERROR(at_parse_param_str(&argv[index]));
    PROCESS_ERROR(strlen(argv[index]) > AT_WLAN_HOSTNAME_LEN);
    lwip_set_hostname( LwIP_GetNetif(S907X_DEV0_ID), argv[0]);
    PROCESS_SUCCESS(1);
#endif


err:
    at_rsp(AT_ERROR);
    return;
    
success:
    at_rsp(AT_OK);
}     

//AT+CWCOUNTRY_CUR 设置 s907x 当前 WiFi 国家码    
static void atwlan_cwcountry_cur  (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    char *argv[AT_SET_MAX_ARGC]={0};
    int argc,ret;
    u8 index=0,mode;
    int country_policy, start_channel, total_channel, country_code;
    u8 country[2]={0};
    
    PROCESS_ERROR(0 == atwlan_get_system_ready());
    
    mode = s907x_wlan_get_mode();
    PROCESS_ERROR(!((S907X_MODE_STA <= mode) && (mode <= S907X_MODE_STA_AP)));
    
    ASSERT(cmd);
    if(!cmd->setsize && cmd->mode == AT_MODE_R){
        country_code = s907x_wlan_get_country();
        if(country_code == S907X_COUNTRY_CN)
            strcpy(country,"CN");
        else if(country_code == S907X_COUNTRY_US)
            strcpy(country,"US");
        else if(country_code == S907X_COUNTRY_JP)
            strcpy(country,"JP");
        else if(country_code == S907X_COUNTRY_FR)
            strcpy(country,"FR");
        else if(country_code == S907X_COUNTRY_AU)
            strcpy(country,"AU");
        else if(country_code == S907X_COUNTRY_EU)
            strcpy(country,"EU");
        at_rsp(AT_WLAN_COUNTRY_CUR,country);
        PROCESS_SUCCESS(1);
    }
    else 
        PROCESS_ERROR(cmd->mode != AT_MODE_W);

    argc = at_get_param(argv, cmd->set);
    PROCESS_ERROR(argc != 4);

    country_policy = atoi(argv[index]);
    PROCESS_ERROR(!((0 <= country_policy) && (country_policy <= 1)));
    index ++;

    PROCESS_ERROR(at_parse_param_str(&argv[index]));
    PROCESS_ERROR(strlen(argv[index]) > 2);

    if(!strcmp(argv[index],"CN"))
        country_code = S907X_COUNTRY_CN;
    else if(!strcmp(argv[index],"US"))
        country_code = S907X_COUNTRY_US;
    else if(!strcmp(argv[index],"JP"))
        country_code = S907X_COUNTRY_JP;
    else if(!strcmp(argv[index],"FR"))
        country_code = S907X_COUNTRY_FR;
    else if(!strcmp(argv[index],"AU"))
        country_code = S907X_COUNTRY_AU;
    else if(!strcmp(argv[index],"EU"))
        country_code = S907X_COUNTRY_EU;
    index ++;

    start_channel = atoi(argv[index]);
    PROCESS_ERROR(!((1 <= start_channel) && (start_channel <= AT_WLAN_MAX_CHANNEL)));
    index ++;
    
    total_channel = atoi(argv[index]);
    PROCESS_ERROR(!((1 <= total_channel) && (total_channel <= AT_WLAN_MAX_CHANNEL)));
    index ++;
    
    ret = s907x_wlan_set_country(country_code);
    PROCESS_SUCCESS(!ret);

err:
    at_rsp(AT_ERROR);
    return;
    
success:
    at_rsp(AT_OK);
}     

//AT+CWCOUNTRY_DEF 设置 s907x 默认 WiFi 国家码    
static void atwlan_cwcountry_def  (void *context)
{
    at_cmd_t *cmd = (at_cmd_t *)context;
    char *argv[AT_SET_MAX_ARGC]={0};
    int argc,ret;
    u8 index=0,mode;
    int country_policy, start_channel, total_channel, country_code;
    u8 country[2]={0};
    
    PROCESS_ERROR(0 == atwlan_get_system_ready());
    
    mode = s907x_wlan_get_mode();
    PROCESS_ERROR(!((S907X_MODE_STA <= mode) && (mode <= S907X_MODE_STA_AP)));
    
    ASSERT(cmd);
    if(!cmd->setsize && cmd->mode == AT_MODE_R){
    #if WLAN_PARA_SAVE
        flash_read(WIFI_COUNTRY_DEF_ADDR, (u8*)&country_code, WIFI_COUNTRY_DEF_SIZE);    
    #endif
        if(country_code == S907X_COUNTRY_CN)
            strcpy(country,"CN");
        else if(country_code == S907X_COUNTRY_US)
            strcpy(country,"US");
        else if(country_code == S907X_COUNTRY_JP)
            strcpy(country,"JP");
        else if(country_code == S907X_COUNTRY_FR)
            strcpy(country,"FR");
        else if(country_code == S907X_COUNTRY_AU)
            strcpy(country,"AU");
        else if(country_code == S907X_COUNTRY_EU)
            strcpy(country,"EU");
        at_rsp(AT_WLAN_COUNTRY_DEF,country);
        PROCESS_SUCCESS(1);
    }
    else 
        PROCESS_ERROR(cmd->mode != AT_MODE_W);
    
    argc = at_get_param(argv, cmd->set);
    PROCESS_ERROR(argc != 4);

    country_policy = atoi(argv[index]);
    PROCESS_ERROR(!((0 <= country_policy) && (country_policy <= 1)));
    index ++;

    PROCESS_ERROR(at_parse_param_str(&argv[index]));
    PROCESS_ERROR(strlen(argv[index]) > 2);
    
    if(!strcmp(argv[index],"CN"))
        country_code = S907X_COUNTRY_CN;
    else if(!strcmp(argv[index],"US"))
        country_code = S907X_COUNTRY_US;
    else if(!strcmp(argv[index],"JP"))
        country_code = S907X_COUNTRY_JP;
    else if(!strcmp(argv[index],"FR"))
        country_code = S907X_COUNTRY_FR;
    else if(!strcmp(argv[index],"AU"))
        country_code = S907X_COUNTRY_AU;
    else if(!strcmp(argv[index],"EU"))
        country_code = S907X_COUNTRY_EU;
    index ++;

    start_channel = atoi(argv[index]);
    PROCESS_ERROR(!((1 <= start_channel) && (start_channel <= AT_WLAN_MAX_CHANNEL)));
    index ++;
    
    total_channel = atoi(argv[index]);
    PROCESS_ERROR(!((1 <= total_channel) && (total_channel <= AT_WLAN_MAX_CHANNEL)));
    index ++;
    
    ret = s907x_wlan_set_country(country_code);
    PROCESS_SAVE_FLASH(!ret);

err:
    at_rsp(AT_ERROR);
    return;
    
save_to_flash:        
#if WLAN_PARA_SAVE
    flash_write(WIFI_COUNTRY_DEF_ADDR, (u8*)&country_code, WIFI_COUNTRY_DEF_SIZE);    
#endif

success:
    at_rsp(AT_OK);
}     

 
at_item_t at_wlan_tbl[] = 
{
    //esp8266 wlan at cmmand
    {{"CWMODE_CUR"},atwlan_cwmode_cur},    
    {{"CWMODE_DEF"},atwlan_cwmode_def},    
    {{"CWJAP_CUR"},atwlan_cwjap_cur},    
    {{"CWJAP_DEF"},atwlan_cwjap_def},    
    {{"CWLAPOPT"},atwlan_cwlapopt},    
    {{"CWLAP"},atwlan_cwlap},    
    {{"CWQAP"},atwlan_cwqap},    
    {{"CWSAP_CUR"},atwlan_cwsap_cur},    
    {{"CWSAP_DEF"},atwlan_cwsap_def},    
    {{"CWLIF"},atwlan_cwlif},    
    {{"CWDHCP_CUR"},atwlan_cwdhcp_cur},    
    {{"CWDHCP_DEF"},atwlan_cwdhcp_def},    
    {{"CWDHCPS_CUR"},atwlan_cwdhcps_cur},    
    {{"CWDHCPS_DEF"},atwlan_cwdhcps_def},    
    {{"CWAUTOCONN"},atwlan_cwautoconn},    
    {{"CIPSTAMAC_CUR"},atwlan_cipstamac_cur},        
    {{"CIPSTAMAC_DEF"},atwlan_cipstamac_def},    
    {{"CIPAPMAC_CUR"},atwlan_cipapmac_cur},    
    {{"CIPAPMAC_DEF"},atwlan_cipapmac_def},    
    {{"CIPSTA_CUR"},atwlan_cipsta_cur},    
    {{"CIPSTA_DEF"},atwlan_cipsta_def},    
    {{"CIPAP_CUR"},atwlan_cipap_cur},    
    {{"CIPAP_DEF"},atwlan_cipap_def},    
    {{"CWSTARTSMART"},atwlan_cwstartsmart},    
    {{"CWSTOPSMART"},atwlan_cwstopsmart},    
    {{"CWSTARTDISCOVER"},atwlan_cwstartdiscover},    
    {{"CWSTOPDISCOVER"},atwlan_cwstopdiscover},    
    {{"WPS"},atwlan_wps},    
    {{"MDNS"},atwlan_mdns},    
    {{"CWHOSTNAME"},atwlan_cwhostname},    
    {{"CWCOUNTRY_CUR"},atwlan_cwcountry_cur},    
    {{"CWCOUNTRY_DEF"},atwlan_cwcountry_def},      
}; 
 
void at_wlan_init(void)
{
    at_add_cmd(&at_wlan_tbl[0], (sizeof(at_wlan_tbl)/sizeof(at_item_t)));    
}


void at_wlan_deinit(void)
{
    at_remove_cmd(&at_wlan_tbl[0], (sizeof(at_wlan_tbl)/sizeof(at_item_t)));    

}

#endif
