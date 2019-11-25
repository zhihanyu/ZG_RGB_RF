#ifndef WLAN_TEST_H
#define WLAN_TEST_H









#define STA_CMD_CONNECT_WITH_SSID 	0
#define STA_CMD_CONNECT_WITH_BSSID  1 
#define STA_CMD_DISCONNECT          2
#define STA_CMD_GET_LINKSTATUS		3
#define STA_CMD_GET_IP              4



#define AP_CMD_START 				0
#define AP_CMD_STOP  				1 
#define AP_CMD_MODIFY_SSID	        2
#define AP_CMD_GET_STA_INFO         3
#define AP_CMD_GET_IP               4
#define AP_CMD_DHCPS_IPRANGE        5


#define TEST_MONITOR_STOP    0
#define TEST_MONITOR_START   1
#define TEST_MONITOR_CHANNEL 2







void wlan_test(void *context);







#endif