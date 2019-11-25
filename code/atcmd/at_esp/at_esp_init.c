#include "s907x.h"
#include "at_network.h"
#include "wlan_api.h"
#include "at_wlan.h"
#include "dhcps.h"
#include "lwip_conf.h"
#include "at_esp_init.h"

#if M_AT_ESP

#define MODE_STA_ENABLED(mode)          (((mode) & (BIT(0))) != 0)
#define MODE_AP_ENABLED(mode)           (((mode) & (BIT(1))) != 0)

extern struct netif xnetif[2];
extern atwlan_sap_cfg_t g_atwlan_cur_ap_cfg;
extern u8 g_atwlan_sta_ip_static;

static void wlan_scan_callback (s907x_scan_result_t *presult)
{
    wlan_target_infor_t *target;

    if(!presult) {
        return;
    }
    
    target = (wlan_target_infor_t*)presult->context;
    if(presult->max_nums == 0) {
        if(target && target->sema) {
            wl_send_sema((sema_t*)&target->sema);
        }
        return;
    }
     
    if(target &&
       presult->scan_info.ssid_len &&
       !strncmp(target->ssid, presult->scan_info.ssid, presult->scan_info.ssid_len)) {
        printf("scan match target ssid\n");    
        target->match = TRUE;
        target->channel = presult->scan_info.channel;
        target->security = presult->scan_info.security;
    }
 
    if(presult->id == presult->max_nums - 1) {
        if(target && target->sema) {
            wl_send_sema((sema_t*)&target->sema);
        }
    }
}

static void at_esp_sta_if_start()
{
    int ret;
    u8 reconnect_mark = 0;
    s907x_sta_init_t s907x_sta_init;
    s907x_scan_result_t scan_result;
    atwlan_sta_cfg_def_t sta_conf;
    s907x_link_info_t link_infor;

    flash_read(WIFI_RECON_ADDR, &reconnect_mark, WIFI_RECON_SIZE);
    if(0 == reconnect_mark){
        return;
    }

    memset(&sta_conf, 0, sizeof(atwlan_sta_cfg_def_t));
    flash_read(WIFI_JAP_DEF_ADDR, (uint8*)&sta_conf, WIFI_JAP_DEF_SIZE);
    if(strlen(sta_conf.ssid) == 0){
        return;
    }
    
    memset(&scan_result, 0, sizeof(s907x_scan_result_t));
    ret = s907x_wlan_scan_ssid(sta_conf.ssid, strlen(sta_conf.ssid), &scan_result);
    if(strcmp(scan_result.scan_info.ssid, sta_conf.ssid) != 0){
        return;
    }
    
    printf("start to connect\n");
    memset(&s907x_sta_init, 0, sizeof(s907x_sta_init_t));
    s907x_sta_init.ssid = sta_conf.ssid;
    s907x_sta_init.ssid_len = strlen(sta_conf.ssid);
    s907x_sta_init.password = sta_conf.password;
    s907x_sta_init.password_len = strlen(sta_conf.password);
    s907x_sta_init.security = scan_result.scan_info.security;
    s907x_sta_init.conn.mode = CONN_MODE_BLOCKING;
    s907x_sta_init.conn.blocking_timeout = 3000;
    s907x_sta_init.auto_conn.enable = 1;
    s907x_sta_init.auto_conn.interval_s = 10;
    s907x_sta_init.auto_conn.cnt = 1000;
    s907x_sta_init.auto_conn.use_staticip = 0;
    
    ret = s907x_wlan_start_sta(&s907x_sta_init);
    if(0 != ret) {
        return;
    }
    
    if(g_atwlan_sta_ip_static == 0) {
        dhcpc_start(0, 0);
    }
    
    s907x_wlan_get_link_infor(&link_infor);
    
    printf("channel:%d\n", link_infor.channel);
    
}

static void at_esp_ap_if_start(u8 mode)
{
    int ret;
    u8 mac[6];
    u32 random; 
    u8 idx = S907X_DEV0_ID;
    s907x_ap_init_t ap_init;
    
    if(mode == S907X_MODE_STA_AP) {
        idx = S907X_DEV1_ID;
    }
    
    s907x_wlan_get_mac_address(idx, mac);    
    if(mac[0]&BIT(0)) {
        random = wl_get_random32();
        mac[0] = 0xb4;
        mac[1] = 0x04;
        mac[2] = 0x18;
        mac[3] = random & 0xff;
        mac[4] = (random >> 8) & 0xff;
        mac[5] = (random >> 16) & 0xff;
        s907x_wlan_set_mac_address(idx, mac);
    }
    
    s907x_wlan_get_mac_address(idx, mac);
    
    flash_read(WIFI_SAP_DEF_ADDR, (u8*)&g_atwlan_cur_ap_cfg, WIFI_SAP_DEF_SIZE);
    memset(&ap_init, 0, sizeof(s907x_ap_init_t));
    ap_init.ssid = g_atwlan_cur_ap_cfg.ssid;
    ap_init.ssid_len = strlen(ap_init.ssid);
    ap_init.password = g_atwlan_cur_ap_cfg.password;
    ap_init.password_len = strlen(ap_init.password);
    ap_init.channel = g_atwlan_cur_ap_cfg.channel;
    ap_init.security = g_atwlan_cur_ap_cfg.authmode;
    ap_init.is_hidded_ssid = g_atwlan_cur_ap_cfg.is_hidded_ssid;
    
    printf("start ap\n");
    
    ret = s907x_wlan_start_ap(&ap_init);
    if(!ret){
       dhcps_init(&xnetif[idx]);
    }
}

void at_esp_init(void)
{
    int ret;
    u8 mode=0;
    
    s907x_wlan_exit_mp();    
     
    at_wlan_init_def_para(); 
    flash_read(WIFI_OPMODE_ADDR, &mode, WIFI_OPMODE_SIZE);
    printf("restored mode:%d\n", mode);
    
    ret = atwlan_cwmode(mode);    
    if(ret) {
        printf("get cwmode fail.\n");
        return;
    }
    
    printf("start to check sta\n");
    if(MODE_STA_ENABLED(mode)) {
        at_esp_sta_if_start();
    }    

    printf("start to check ap\n");
    if(MODE_AP_ENABLED(mode)) {
        at_esp_ap_if_start(mode);
    }
    
    atwlan_set_system_ready();    
    
    printf("at_esp_init done\n");        
}


void at_esp_deinit(void)
{


}

#endif
