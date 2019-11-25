#ifndef WLAN_API_H
#define WLAN_API_H




 
#define S907X_DEV0_ID           0
#define S907X_DEV1_ID           1


#define S907X_DEFAULT_AP_CHANNEL   1
#define S907X_DEFAULT_SCAN_AP_NUMS 32
#define S907X_DEFAULT_CONN_TO      10000
#define S907X_DEFAULT_RETRY_CNT    20
#define S907X_MAX_CLIENTS  		   3


#define CONN_MODE_BLOCKING			0
#define CONN_MODE_UNBLOCKING		1


/* 
	monitor mode select
*/

#define S907X_MONITOR_MICO  	1


#define S907X_MONITOR_SEL		0//S907X_MONITOR_MICO


#if  S907X_MONITOR_SEL == S907X_MONITOR_MICO

#include "mico_wlan.h"

typedef struct monitor_type{
	u8 m_type;	
	monitor_cb_t callback;
}monitor_t,*pmonitor_t;

#else
typedef void (*com_monitor_cb_t)(uint8_t*data, int len);
typedef struct monitor_type{
	u8 m_type;
	com_monitor_cb_t callback;
}monitor_t,*pmonitor_t;
#endif

//connect result & status
typedef enum
{
	S907X_IDLE,
	S907X_CONNECTING,
	//result
	S907X_CONNECTED, //connect ap only
	S907X_NO_NETWORK,
	S907X_CONNECT_HANDSHAKE_DONE,
	S907X_CONNECT_TIMEOUT,
	S907X_CONNECT_PASSWORD_ERROR,
	S907X_CONNECT_ERROR_UNKNW,
}s907x_conn_reslt_e;

//s907x wlan mode
typedef enum {
	S907X_MODE_NONE = 0,
	S907X_MODE_STA,
	S907X_MODE_AP,
	S907X_MODE_STA_AP,
	S907X_MODE_MONITOR,
	S907X_MODE_P2P,
    S907X_MODE_MAX
}s907x_mode_e;
 
//s907x security type
typedef enum {
   S907X_SECURITY_NONE,        
   S907X_SECURITY_WEP,         
   S907X_SECURITY_WPA_TKIP,    
   S907X_SECURITY_WPA_AES,     
   S907X_SECURITY_WPA2_TKIP,   
   S907X_SECURITY_WPA2_AES,    
   S907X_SECURITY_WPA2_MIXED,  
   S907X_SECURITY_AUTO,   
}s907x_security_e;

typedef enum {
	S907X_PHYMODE_B = 0,
	S907X_PHYMODE_BG,
	S907X_PHYMODE_BGN
}s907x_phymode_e;
 
typedef enum  {
	S907X_WLAN_TYPE_MGT         =	(0),
	S907X_WLAN_TYPE_CTRL        =	(BIT(2)),
	S907X_WLAN_TYPE_DATA        =	(BIT(3)),
	S907X_WLAN_TYPE_QOS_DATA	=   (BIT(7) | BIT(3)),	
}s907x_wlan_frame_type_e;


typedef enum  
{
	S907X_WLAN_SUBTYPE_ASSOCREQ       = ( 0 ),
	S907X_WLAN_SUBTYPE_ASSOCRSP       = (BIT(4) ),
	S907X_WLAN_SUBTYPE_REASSOCREQ     = (BIT(5) ),
	S907X_WLAN_SUBTYPE_REASSOCRSP     = (BIT(5) | BIT(4) ),
	S907X_WLAN_SUBTYPE_PROBEREQ       = (BIT(6) ),
	S907X_WLAN_SUBTYPE_PROBERSP       = (BIT(6) | BIT(4) ),
	S907X_WLAN_SUBTYPE_BEACON         = (BIT(7) ),
	S907X_WLAN_SUBTYPE_ATIM           = (BIT(7) | BIT(4) ),
	S907X_WLAN_SUBTYPE_DISASSOC       = (BIT(7) | BIT(5) ),
	S907X_WLAN_SUBTYPE_AUTH           = (BIT(7) | BIT(5) | BIT(4) ),
	S907X_WLAN_SUBTYPE_DEAUTH         = (BIT(7) | BIT(6) ),
	S907X_WLAN_SUBTYPE_ACTION         = (BIT(7) | BIT(6) | BIT(4) ),
	S907X_WLAN_SUBTYPE_PSPOLL         = (BIT(7) | BIT(5) | S907X_WLAN_TYPE_CTRL),
	S907X_WLAN_SUBTYPE_RTS            = (BIT(7) | BIT(5) | BIT(4) | S907X_WLAN_TYPE_CTRL),
	S907X_WLAN_SUBTYPE_CTS            = (BIT(7) | BIT(6) | S907X_WLAN_TYPE_CTRL),
	S907X_WLAN_SUBTYPE_ACK            = (BIT(7) | BIT(6) | BIT(4) | S907X_WLAN_TYPE_CTRL),
	S907X_WLAN_SUBTYPE_DATA           = (0 | S907X_WLAN_TYPE_DATA),
	S907X_WLAN_SUBTYPE_DATA_CFACK     = (BIT(4) | S907X_WLAN_TYPE_DATA),
	S907X_WLAN_SUBTYPE_DATA_CFPOLL    = (BIT(5) | S907X_WLAN_TYPE_DATA),
	S907X_WLAN_SUBTYPE_DATA_CFACKPOLL = (BIT(5) | BIT(4) | S907X_WLAN_TYPE_DATA),
	S907X_WLAN_SUBTYPE_DATA_NULL      = (BIT(6) | S907X_WLAN_TYPE_DATA),
	S907X_WLAN_SUBTYPE_QOS_DATA_NULL  = (BIT(6) | S907X_WLAN_TYPE_QOS_DATA),
}s907x_wlan_frame_subtype_e; 

#define GET_WLAN_FRAME_TYPE(pbuf)       (*(u16*)(pbuf) & (BIT(3) | BIT(2)))
#define GET_WLAN_FRAME_SUBTYPE(pbuf)    (*(u16*)(pbuf) & (BIT(7) | BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2)) )



typedef struct s907x_ap_init_
{
	char *ssid;
	int   ssid_len;
	char *password;
	int   password_len;  
	u8    channel;
	int   is_hidded_ssid;
	s907x_security_e security; //only support S907X_SECURITY_NONE or S907X_SECURITY_WPA2_AES
}s907x_ap_init_t;

typedef struct s907x_ap_client_infor_
{
	u8  hw_addr[6];
	u8  rsvd[2];
	s32 rssi;
}s907x_ap_client_infor_t;


typedef struct s907x_auto_conn_
{
	int   enable;
	u16   interval_s;
	u8    cnt;
    int   use_staticip;
}s907x_auto_conn_t;

enum s907x_country_code_e{
	S907X_COUNTRY_CN,//1~13		01
	S907X_COUNTRY_US,//1~13		05
	S907X_COUNTRY_JP,//1~14		03
	S907X_COUNTRY_FR,//1~13		01
	S907X_COUNTRY_AU,//1~13		01
	S907X_COUNTRY_EU,//1~13		01
	S907X_COUNTRY_MAX//1~13		
};
struct scan_config {
    u8 *ssid;		// Note: ssid == NULL, don't filter ssid.
    u8 *bssid;		// Note: bssid == NULL, don't filter bssid.
    u8 channel;		// Note: channel == 0, scan all channels, otherwise scan set channel.
    u8 show_hidden;	// Note: show_hidden == 1, can get hidden ssid routers' info.
};
typedef struct wlan_target_infor_
{
    sema_t  sema;
    char    ssid[33];
    u8      channel;
	u8      match;
    s907x_security_e     security;
}wlan_target_infor_t;

/*unblocking mode
	1. async poll check result
    2. use   coutom_async_sema
  blocking
	1. sync check result
*/
typedef struct s907x_conn_type_
{
	u8     mode;             //should be CONN_MODE_BLOCKING or CONN_MODE_UNBLOCKING 
	u32*   result;           //only used in CONN_MODE_UNBLOCKING
    u32    blocking_timeout; //only used in CONN_MODE_BLOCKING unit:ms
	sema_t coutom_async_sema;//only used in CONN_MODE_UNBLOCKING
}s907x_conn_type_t;

typedef struct s907x_sta_init_
{
	char *ssid;
	int   ssid_len;
	char *password;
	int   password_len;  
	s907x_security_e	security;
	s907x_conn_type_t   conn;
    s907x_auto_conn_t   auto_conn;
}s907x_sta_init_t;

#pragma pack(1)

typedef struct s907x_scan_info_
{
	char ssid[33];
	int  ssid_len;
	u8   bssid[6];
	u8   channel;
	int  security;
	s32  rssi;
	//void *object;
	u32   rsvd[2];	
}s907x_scan_info_t;

#pragma pack()


typedef struct s907x_scan_result_
{
	int max_nums;
	int id;
	void *context;
	s907x_scan_info_t scan_info;
}s907x_scan_result_t;

typedef struct s907x_link_info_
{
	int is_connected;
	int rssi;
	u8  channel;
	u8  ssid[33];
	u8  bssid[6];
}s907x_link_info_t;

typedef void (*s907x_scan) (s907x_scan_result_t *presult);


typedef enum
{
	m_disable,
	m_bcmc,	  //b/m packets only
    m_all,    //802.11 all packkets          
}monitor_mode_e;

typedef void (*s907x_monitor_cb) (u8 *pbuf, int len, void *context);


typedef struct s907x_monitor_
{
	int mode;
	void *coustom_context;
	s907x_monitor_cb cb;
}s907x_monitor_t;


typedef struct _cus_ie{
	u8 *ie;		// IE + Len + data
	u8 type;	// beacon¡¢probe_rsp¡¢probe_req
}custom_ie;


//s907x event data 

//s907x event data for esp8266 coustom
typedef struct {
	unsigned char ssid[32];
	unsigned char ssid_len;
	unsigned char bssid[6];
	unsigned char channel;
} Event_StaMode_Connected_t;

typedef struct {
	unsigned char ssid[32];
	unsigned char ssid_len;
	unsigned char bssid[6];
	unsigned char reason;
} Event_StaMode_Disconnected_t;

typedef struct {
	unsigned char old_mode;
	unsigned char new_mode;
} Event_StaMode_AuthMode_Change_t;


typedef struct {
	unsigned char mac[6];
	unsigned char aid;
} Event_SoftAPMode_StaConnected_t;

typedef struct {
	unsigned char mac[6];
	unsigned char aid;
} Event_SoftAPMode_StaDisconnected_t;

typedef struct {
	int rssi;
	unsigned char mac[6];
} Event_SoftAPMode_ProbeReqRecved_t;

typedef union {
	Event_StaMode_Connected_t			connected;
	Event_StaMode_Disconnected_t		disconnected;
	Event_StaMode_AuthMode_Change_t		auth_change;
	Event_SoftAPMode_StaConnected_t		sta_connected;
	Event_SoftAPMode_StaDisconnected_t	sta_disconnected;
	Event_SoftAPMode_ProbeReqRecved_t   ap_probereqrecved;
} s907x_event_esp_data;

//s907x event data for rtl8710 coustom
typedef union {
	s907x_event_esp_data esp_data;
} s907x_event_data;

typedef enum
{    
    //for esp8266 data event
    S907X_EVENT_STAMODE_SCAN_DONE = 0,
    S907X_EVENT_STAMODE_CONNECTED,
    S907X_EVENT_STAMODE_DISCONNECTED,
    S907X_EVENT_STAMODE_AUTHCHANGE,
    S907X_EVENT_APMODE_STA_CONNECTED,
    S907X_EVENT_APMODE_STA_DISCONNECTED,
    S907X_EVENT_APMODE_PROBE_REG_RECEIVED,
	S907X_EVENT_MAX
}s907x_event_e;

typedef enum {
	S907X_WLAN_RX_BEACON,    /* receive beacon packet */
	S907X_WLAN_RX_PROBE_REQ, /* receive probe request packet */
	S907X_WLAN_RX_PROBE_RES, /* receive probe response packet */
	S907X_WLAN_RX_ACTION,    /* receive action packet */
	S907X_WLAN_RX_MANAGEMENT,/* receive ALL management packet */
	S907X_WLAN_RX_DATA,      /* receive ALL data packet */
	S907X_WLAN_RX_MCAST_DATA,/* receive ALL multicast and broadcast packet */

	S907X_WLAN_RX_ALL,       /* receive ALL 802.11 packet */
    S907X_WLAN_RX_MAX,
}s907X_monitor_filter;


typedef union {
    s907x_event_e    event_id;
	s907x_event_data event_info;
} s907x_event;


typedef void (*s907x_event_callback)(s907x_event_data *event_data, void *context);
typedef void (*wifi_rx_mgnt_callback)(u8 *buf,int buf_len,int type,char rssi);

//function api
//on or off
int s907x_wlan_on(int mode);
int s907x_wlan_off(void);
int s907x_wlan_is_running(u8 s907x_device_id);
int s907x_wlan_get_mode(void);
//internal network interface
int s907x_wlan_set_netfunc(void *hook);
void s907x_wlan_set_netif(int id, unsigned int netif);

//event register
int s907x_wlan_event_reg(s907x_event_e event, s907x_event_callback event_callback, void *context);
int s907x_wlan_event_unreg(s907x_event_e event,s907x_event_callback event_callback);

//ap
int s907x_wlan_start_ap(s907x_ap_init_t *init);
int s907x_wlan_stop_ap(void);
int s907x_wlan_ap_deauth_sta(u8 *hw_addr);
int s907x_wlan_ap_get_client_nums(void);
int s907x_wlan_ap_get_client_infor(s907x_ap_client_infor_t *infor, int max_client);
int s907x_wlan_add_ie(u8 s907x_device_id, u8 *ie, int len);
int s907x_wlan_del_ie(u8 s907x_device_id);

//sta
int s907x_wlan_start_sta(s907x_sta_init_t *init);
int s907x_wlan_stop_sta(void);
int s907x_wlan_scan(s907x_scan scan_cb, int max_ap_nums, void *context);
int s907x_wlan_scan_ssid(const char *ssid, int ssid_len, s907x_scan_result_t *result);
int s907x_wlan_get_link_infor(s907x_link_info_t *link_infor);
int s907x_wlan_tx_mgt_frame(u8 s907x_device_id, u8 *pbuf, int len);
int s907x_wlan_enable_autoreconn(u8 cnt, u16 interval, int static_ip);
int s907x_wlan_disable_autoreconn(void);

//monitor
int s907x_wlan_start_monitor(s907x_monitor_t *pmonitor);
int s907x_wlan_stop_monitor(void);
int s907x_wlan_set_channel(u8 s907x_device_id, u8 channel);
u8  s907x_wlan_get_channel(u8 s907x_device_id);

//mac
int s907x_wlan_set_mac_address(u8 s907x_device_id,  u8 *mac);
int s907x_wlan_set_mac_address_no_efuse(u8 s907x_device_id,  u8 *mac);
int s907x_wlan_get_mac_address(u8 s907x_device_id,  u8 *mac);

//phy mode
int s907x_wlan_set_phy_mode(u8 mode);
u8  s907x_wlan_get_phy_mode(void);

//country code
int s907x_wlan_set_country(int country_code);
int s907x_wlan_get_country(void);

//mp
int s907x_wlan_enter_mp(void);
int s907x_wlan_exit_mp(void);
int s907x_wlan_hanlde_mp(char *pbuf);


void s907x_set_rx_mgnt_callback(wifi_rx_mgnt_callback cb);

//sleep
void s907x_wlan_enable_sleep(u8 sleep);

#endif
