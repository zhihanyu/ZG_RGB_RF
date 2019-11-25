#ifndef AT_WLAN_H
#define AT_WLAN_H

#include "lwip/ip_addr.h"

#define    WIFI_OPMODE_SIZE         (0x1) 
#define WIFI_OPMODE_ADDR            (FLASH_DATA_USER_START)    

#define WIFI_JAP_DEF_SIZE           (0x68)
#define WIFI_JAP_DEF_ADDR           (WIFI_OPMODE_ADDR+WIFI_OPMODE_SIZE)

#define WIFI_SAP_DEF_SIZE           (0x64)
#define WIFI_SAP_DEF_ADDR           (WIFI_JAP_DEF_ADDR+WIFI_JAP_DEF_SIZE)

#define WIFI_DHCP_DEF_SIZE          (0x1)
#define WIFI_DHCP_DEF_ADDR          (WIFI_SAP_DEF_ADDR+WIFI_SAP_DEF_SIZE)

#define WIFI_DHCPS_DEF_SIZE         (0x10)
#define WIFI_DHCPS_DEF_ADDR         (WIFI_DHCP_DEF_ADDR+WIFI_DHCP_DEF_SIZE)

#define WIFI_RECON_SIZE             (0x1)
#define WIFI_RECON_ADDR             (WIFI_DHCPS_DEF_ADDR+WIFI_DHCPS_DEF_SIZE)

#define WIFI_STA_MAC_DEF_SIZE       (0x6)
#define WIFI_STA_MAC_DEF_ADDR       (WIFI_RECON_ADDR+WIFI_RECON_SIZE)

#define WIFI_AP_MAC_DEF_SIZE        (0x6)
#define WIFI_AP_MAC_DEF_ADDR        (WIFI_STA_MAC_DEF_ADDR+WIFI_STA_MAC_DEF_SIZE)

#define WIFI_STA_IP_ADDR_DEF_SIZE   (0xc)
#define WIFI_STA_IP_ADDR_DEF_ADDR   (WIFI_AP_MAC_DEF_ADDR+WIFI_AP_MAC_DEF_SIZE)

#define WIFI_AP_IP_ADDR_DEF_SIZE    (0xc)
#define WIFI_AP_IP_ADDR_DEF_ADDR    (WIFI_STA_IP_ADDR_DEF_ADDR+WIFI_STA_IP_ADDR_DEF_SIZE)

#define WIFI_COUNTRY_DEF_SIZE       (0x4)
#define WIFI_COUNTRY_DEF_ADDR       (WIFI_AP_IP_ADDR_DEF_ADDR+WIFI_AP_IP_ADDR_DEF_SIZE)

#define AT_COMMA_TAG                ','


#define STA_HOST_NAME               "sta_%02X%02X%02X"
#define AT_WLAN_MAC_FMT_STR         "\"%02x:%02x:%02x:%02x:%02x:%02x\""
#define AT_WLAN_MAC_FMT             "%02x:%02x:%02x:%02x:%02x:%02x"

#define AT_WLAN_STA_CONNECTED       "+STA_CONNECTED:\"%02x:%02x:%02x:%02x:%02x:%02x\"\r\n"
#define AT_WLAN_STA_DISCONNECTED    "+STA_DISCONNECTED:\"%02x:%02x:%02x:%02x:%02x:%02x\"\r\n"
#define AT_WLAN_AP_PROBE_RECEIVED   "+AP_PROBE_RECEIVED:RSSI(%d),\"%02x:%02x:%02x:%02x:%02x:%02x\"\r\n"
#define AT_WLAN_DIST_STA_IP         "+DIST_STA_IP:\"%02x:%02x:%02x:%02x:%02x:%02x\",\"%s\"\r\n"

#define AT_WLAN_WIFI_GOT_IP         "WIFI GOT IP\r\n"
#define AT_WLAN_WIFI_CONNECTED      "WIFI CONNECTED \r\n"
#define AT_WLAN_WIFI_DISCONNECTED   "WIFI DISCONNECTED\r\n"
#define AT_WLAN_WIFI_AUTH_CHANGED   "STA AUTHCHANGE \r\n"

#define AT_WLAN_MODE_CUR            "+CWMODE_CUR:%d\r\n"
#define AT_WLAN_MODE_CUR_GET        "+CWMODE_CUR:1,2,3\r\n"

#define AT_WLAN_MODE_DEF            "+CWMODE_DEF:%d\r\n"
#define AT_WLAN_MODE_DEF_GET        "+CWMODE_DEF:1,2,3\r\n"

#define AT_WLAN_CWLAP               "+CWLAP:("
#define AT_WLAN_JAP_CUR             "+CWJAP_CUR:\"%s\",\"%02x:%02x:%02x:%02x:%02x:%02x\",%d,%d\r\n"
#define AT_WLAN_JAP_CUR_RST         "+CWJAP_CUR:%d\r\n"
#define AT_WLAN_JAP_DEF             "+CWJAP_DEF:\"%s\",\"%02x:%02x:%02x:%02x:%02x:%02x\",%d,%d\r\n"
#define AT_WLAN_JAP_FAIL            "+CWJAP:%d\r\n"
#define AT_WLAN_JAP_NO_AP           "No AP\r\n"

#define AT_WLAN_SAP_CUR             "+CWSAP_CUR:\"%s\",\"%s\",%d,%d,%d,%d\r\n"
#define AT_WLAN_SAP_DEF             "+CWSAP_EF:\"%s\",\"%s\",%d,%d,%d,%d\r\n"

#define AT_WLAN_LIF                 "%s,%02x:%02x:%02x:%02x:%02x:%02x\r\n"
#define AT_WLAN_LIF_NULL            "\r\n"		//+CWLIF

#define AT_WLAN_DHCP_CUR            "+CWDHCP_CUR:%d\r\n"
#define AT_WLAN_DHCP_DEF            "+CWDHCP_DEF:%d\r\n"

#define AT_WLAN_DHCPS_CUR           "+CWDHCPS_CUR:%d,%s,%s\r\n"
#define AT_WLAN_DHCPS_DEF           "+CWDHCPS_DEF:%d,%s,%s\r\n"


#define AT_WLAN_IPSTAMAC_CUR        "+CIPSTAMAC_CUR:\"%02x:%02x:%02x:%02x:%02x:%02x\"\r\n"
#define AT_WLAN_IPSTAMAC_DEF        "+CIPSTAMAC_DEF:\"%02x:%02x:%02x:%02x:%02x:%02x\"\r\n"
    
#define AT_WLAN_IPAPMAC_CUR         "+CIPAPMAC_CUR:\"%02x:%02x:%02x:%02x:%02x:%02x\"\r\n"
#define AT_WLAN_IPAPMAC_DEF         "+CIPAPMAC_DEF:\"%02x:%02x:%02x:%02x:%02x:%02x\"\r\n"

#define AT_WLAN_IPSTA_CUR           "+CIPSTA_CUR:%s:\"%s\"\r\n"
#define AT_WLAN_IPSTA_DEF           "+CIPSTA_DEF:%s:\"%s\"\r\n"

#define AT_WLAN_IPAP_CUR            "+CIPAP_CUR:%s:\"%s\"\r\n"
#define AT_WLAN_IPAP_DEF            "+CIPAP_DEF:%s:\"%s\"\r\n"
#define AT_WLAN_IPADDR              "ip"
#define AT_WLAN_GATEWAY             "gateway"
#define AT_WLAN_NETMASK             "netmask"


#define AT_WLAN_HOSTNAME            "+CWHOSTNAME:\"%s\""
#define AT_WLAN_HOSTNAME_NULL       "+CWHOSTNAME:null\r\n"


#define AT_WLAN_COUNTRY_CUR         "+CWCOUNTRY_CUR:%s"
#define AT_WLAN_COUNTRY_DEF         "+CWCOUNTRY_DEF:%s"

#define AT_WLAN_AP_SSID_DEF         "S_AP"
#define AT_WLAN_AP_PWD_DEF          "12345678"
#define AT_WLAN_AP_IP_DEF           "192.168.1.1"
#define AT_WLAN_AP_GW_DEF           "192.168.1.1"
#define AT_WLAN_AP_NETMASK          "255.255.255.0"

#define AT_RET_OK                   0
#define AT_RET_ERR                  -1

#define AT_WLAN_STA                 "sta"
#define AT_WLAN_AP                  "ap"

#define AT_WLAN_MAX_STA             3
#define AT_WLAN_SAP_MAX_STA         8

#define WLAN_PARA_SAVE                  1
#define AT_WLAN_HOSTNAME_LEN            32
#define AT_WLAN_SSID_LEN                32
#define AT_WLAN_PWD_LEN                 64
#define AT_WLAN_MAC_LEN                 17
#define AT_WLAN_MAX_CHANNEL             14
#define AT_WLAN_BSSID_LEN               6
#define DHCPS_LEASE_TIME_DEF            120
#define AT_WLAN_MAX_DHCPS_LEASE_TIME    2880
#define AT_WLAN_DHCPS_POOL_RANGE        100

#define AT_WLAN_SL_UDP_SERVER_PORT      18266       // UDP Server port 
#define AT_WLAN_SL_UDP_BYTE0_LEN        1
#define AT_WLAN_SL_UDP_BSSID_LEN        6
#define AT_WLAN_SL_UDP_IP_LEN           4
#define AT_WLAN_SL_UDP_TOTAL_LEN        (AT_WLAN_SL_UDP_BYTE0_LEN + AT_WLAN_SL_UDP_BSSID_LEN + AT_WLAN_SL_UDP_IP_LEN)
#define AT_WLAN_SL_UDP_LEN_APEND_MAGIC  9
#define AT_WLAN_SL_UDP_RETX_CNT         50
#define AT_WLAN_SL_UDP_PKT_INTERVAL     100         // 100ms

#define AT_WLAN_DELAY_TO_GET_IP         3000

#define AT_AP_DHCP_EN                   BIT(0)
#define AT_STA_DHCP_EN                  BIT(1)


typedef struct atwlan_sta_cfg_def {
    char ssid[AT_WLAN_SSID_LEN];        /**< SSID of target AP*/
    char password[AT_WLAN_PWD_LEN];     /**< password of target AP*/
    u8 bssid_set;                       /**< whether set MAC address of target AP or not. Generally, 
                                          station_config.bssid_set needs to be 0; and it needs to 
                                          be 1 only when users need to check the MAC address of the AP.*/
    u8 bssid[AT_WLAN_BSSID_LEN];        /**< MAC address of target AP*/
	u8 rsvd;
}atwlan_sta_cfg_def_t;

typedef struct atwlan_jap_cfg {
    char *ssid;         // Note: ssid == NULL, don't filter ssid.
    char *password;
    u8 *bssid;          // Note: bssid == NULL, don't filter bssid.
    u8 bssid_set;
    u8 pci_en;
    u8 rsv[2];
}atwlan_jap_cfg_t;

typedef struct atwlan_sap_cfg {
    char ssid[AT_WLAN_SSID_LEN];
    char password[AT_WLAN_PWD_LEN];
    u8 channel;         // Note: support 1 ~ 13
    u8 authmode;        // Note: Don't support AUTH_WEP in softAP mode.
    u8 max_conn; 
    u8 is_hidded_ssid;  // Note: default 0
}atwlan_sap_cfg_t;

typedef struct atwlan_if_addr_cfg {
    struct ip_addr ip;
    struct ip_addr gw;
    struct ip_addr netmask;
}atwlan_if_addr_cfg_t;

typedef struct atwlan_dhcps_lease_pool {
    u8 manual;                      // 0: Default values; 1: Manual values
    u8 rsvd[3];
    u32 lease_time;
    struct ip_addr start_ip;
    struct ip_addr end_ip;
}atwlan_dhcps_lease_pool_t;

typedef enum {
    ATWLAN_DHCP_IF_AP       = 0,
    ATWLAN_DHCP_IF_STA      = 1,
    ATWLAN_DHCP_IF_APSTA    = 2,
}atwlan_dhcp_if_e;

#define PROCESS_ERROR(error_status)             \
    do                                          \
    {                                           \
        if(error_status)                        \
        {                                       \
            printf("%s,%d\n",__func__,__LINE__);\
            goto err;                           \
        }                                       \
    } while(0)

#define PROCESS_FAIL(fail_status)               \
    do                                          \
    {                                           \
        if(fail_status)                         \
        {                                       \
            goto fail;                          \
        }                                       \
    } while(0)


#define PROCESS_SUCCESS(suc_status)             \
    do                                          \
    {                                           \
        if(suc_status)                          \
        {                                       \
            goto success;                       \
        }                                       \
    } while(0)


#define PROCESS_SAVE_FLASH(save_status)         \
    do                                          \
    {                                           \
        if(save_status)                         \
        {                                       \
            goto save_to_flash;                 \
        }                                       \
    } while(0)


#define ARGC_INDEX_VALID()        if(argc >= index)

int atwlan_cwmode(u8 mode);
void at_wlan_init(void);
void at_wlan_deinit(void);
void at_wlan_event_reg(u8 mode);
int flash_write(u32 addr, u8 *pbuf, int len);
void flash_read(u32 addr, u8 *pbuf, int len);
void at_wlan_event_sta_got_ip(u32 addr);
void wlan_flash_init(void);
void at_wlan_init_def_para(void);
void atwlan_set_system_ready(void);


#endif
