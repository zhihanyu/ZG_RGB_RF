/**
  ******************************************************************************
  * @file    netconf.h
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    07-October-2011 
  * @brief   This file contains all the functions prototypes for the netconf.c 
  *          file.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NETCONF_H
#define __NETCONF_H

#ifdef __cplusplus
 extern "C" {
#endif


#include "lwip/err.h"
#include "lwip/dns.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/tcp.h"
#include "lwip/tcpip.h"



// macros
#define MAX_NETIF_NUM 2

/* Private typedef -----------------------------------------------------------*/
typedef enum 
{ 
	DHCP_START=0,
	DHCP_WAIT_ADDRESS,
	DHCP_ADDRESS_ASSIGNED,
	DHCP_RELEASE_IP,
	DHCP_STOP,
	DHCP_TIMEOUT
} DHCP_State_TypeDef;

typedef struct wlan_net_func_ 
{
	//void network_set_adapter(u32 adapter);
	void (*dhcps_init)(struct netif *adapter);
	void (*dhcps_deinit)(void);
	uint8_t (*dhcp_start)(uint8_t idx, uint8_t dhcp_state);
	void (*dhcp_stop)(struct netif *adapter);
	void (*network_set_link_up)(struct netif *adapter);
	void (*network_set_link_down)(struct netif *adapter);
	void (*network_set_up)(struct netif *adapter);
	void (*network_set_down)(struct netif *adapter);
	void (*ethernet_recv)(struct netif *adapter, int len);
	void (*low_power_enter)(void);
	void (*low_power_exit)(void);
	void  (*network_get_ip)(void);
	void  (*network_check_ip)(struct netif *adapter, u8 *set_ip, u8 *is_ok);
	void  (*set_net_address)(struct netif *adapter, u8 *hwaddr);
	void  (*get_net_address)(struct netif *adapter, u8 *hwaddr);
	void  (*get_net_ipaddr)(struct netif *adapter, u32 *ip);
    void  (*set_net_ipaddr)(void);  
}wlan_net_func_t;

#define MAX_DHCP_TRIES 5

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void LwIP_Init(void);
uint8_t dhcpc_start(uint8_t idx, uint8_t dhcp_state);
struct netif *LwIP_GetNetif(int id);
unsigned char* LwIP_GetMAC(struct netif *pnetif);
unsigned char* LwIP_GetIP(struct netif *pnetif);
unsigned char* LwIP_GetGW(struct netif *pnetif);
uint8_t* LwIP_GetMASK(struct netif *pnetif);
uint8_t* LwIP_GetBC(struct netif *pnetif);
#if LWIP_DNS
const struct ip_addr* LwIP_GetDNS(void);
void LwIP_SetDNS(struct ip_addr* dns);
#else
const struct ip_addr* LwIP_GetDNS(void);
void LwIP_SetDNS(struct ip_addr* dns);
#endif
int lwip_get_id(struct netif* pnetif);
#if LWIP_AUTOIP
void LwIP_AUTOIP(struct netif *pnetif);
#endif
#if LWIP_IPV6
void LwIP_AUTOIP_IPv6(struct netif *pnetif);
#endif
void   lwip_set_autoreconnect_cb(void (*set_static_ip) (void));
int    lwip_set_hostname(struct netif *pnetif, const char *name);
const  char*  lwip_get_hostname(struct netif *pnetif);
#ifdef __cplusplus
} 
#endif

#endif /* __NETCONF_H */


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
