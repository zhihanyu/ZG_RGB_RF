#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/dhcp.h"
#include "lwip/dns.h"
#include "ethernetif.h"
#include "lwip_conf.h"
#include "wlan_api.h"

  
err_t dhcp_release_unicast(struct netif *netif);
/**
  * @brief  LwIP_DHCP_Process_Handle
  * @param  None
  * @retval None
  */
uint8_t dhcpc_start(uint8_t idx, uint8_t dhcp_state)
{
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw;
	uint32_t IPaddress;
	uint8_t iptab[4];
	uint8_t DHCP_state;
	int mscnt = 0;
	struct netif *pnetif = NULL;
	struct dhcp* dhcp;
	DHCP_state = dhcp_state;
	
	if(idx > 1)
		idx = 1;

	pnetif = LwIP_GetNetif(idx);
	if(DHCP_state == 0){
		pnetif->ip_addr.addr = 0;
		pnetif->netmask.addr = 0;
		pnetif->gw.addr = 0;
	}

	for (;;)
	{
		//printf("DHCP_state => %d\n\r",DHCP_state);
		switch (DHCP_state)
		{
			case DHCP_START:
			{
				dhcp_start(pnetif);
				IPaddress = 0;
				DHCP_state = DHCP_WAIT_ADDRESS;
			}
			break; 

		case DHCP_WAIT_ADDRESS:
		{
			//dhcp = netif_dhcp_data(pnetif);
            dhcp = pnetif->dhcp;
			/* If DHCP stopped by wifi_disconn_hdl*/
			if(dhcp->state == 0) 
			{
				IP4_ADDR(&ipaddr, IP_ADDR0 ,IP_ADDR1 , IP_ADDR2 , IP_ADDR3 );
				IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
				IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
				netif_set_addr(pnetif, &ipaddr , &netmask, &gw);
				printf("dhcp stop!\n");
				return DHCP_STOP;
			}
			
			/* Read the new IP address */
			IPaddress = pnetif->ip_addr.addr;

			if (IPaddress!=0) 
			{
				DHCP_state = DHCP_ADDRESS_ASSIGNED;	
		
				/* Stop DHCP */
				// dhcp_stop(pnetif);  /* can not stop, need to renew, Robbie*/

				iptab[0] = (uint8_t)(IPaddress >> 24);
				iptab[1] = (uint8_t)(IPaddress >> 16);
				iptab[2] = (uint8_t)(IPaddress >> 8);
				iptab[3] = (uint8_t)(IPaddress);
#if M_AT_ESP
				at_wlan_event_sta_got_ip(IPaddress);
#else
				printf("id %d ip address : %d.%d.%d.%d\n", idx, iptab[3], iptab[2], iptab[1], iptab[0]);
#endif
				return DHCP_ADDRESS_ASSIGNED;
			}
			else
			{
				/* DHCP timeout */
				if (dhcp->tries > MAX_DHCP_TRIES)
				{
					DHCP_state = DHCP_TIMEOUT;

					/* Stop DHCP */
					dhcp_stop(pnetif);

					/* Static address used */
					IP4_ADDR(&ipaddr, IP_ADDR0 ,IP_ADDR1 , IP_ADDR2 , IP_ADDR3 );
					IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
					IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
					netif_set_addr(pnetif, &ipaddr , &netmask, &gw);

					iptab[0] = IP_ADDR3;
					iptab[1] = IP_ADDR2;
					iptab[2] = IP_ADDR1;
					iptab[3] = IP_ADDR0;
					printf("id %d DHCP timeout\n",idx);
					printf("static ip address : %d.%d.%d.%d\n", iptab[3], iptab[2], iptab[1], iptab[0]);


                    if(idx == MAX_NETIF_NUM -1)
                       netif_set_up(pnetif);

					return DHCP_TIMEOUT;
				} else {
					//sys_msleep(DHCP_FINE_TIMER_MSECS);
					wl_os_mdelay(DHCP_FINE_TIMER_MSECS);
					dhcp_fine_tmr();
					mscnt += DHCP_FINE_TIMER_MSECS;
					if (mscnt >= DHCP_COARSE_TIMER_SECS*1000) 
					{
						dhcp_coarse_tmr();
						mscnt = 0;
					}
				}
			}
		}
		break;
		case DHCP_RELEASE_IP:
			//dhcp_release_unicast(pnetif);
			return DHCP_RELEASE_IP;
		case DHCP_STOP:
			dhcp_stop(pnetif);
			return DHCP_STOP;
		default: 
			break;
	}

	}   
}

void LwIP_ReleaseIP(uint8_t idx)
{
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw;
	struct netif *pnetif = LwIP_GetNetif(idx);
	
	IP4_ADDR(&ipaddr, 0, 0, 0, 0);
	IP4_ADDR(&netmask, 255, 255, 255, 0);
	IP4_ADDR(&gw, 0, 0, 0, 0);
	
	netif_set_addr(pnetif, &ipaddr , &netmask, &gw);
}

uint8_t* LwIP_GetMAC(struct netif *pnetif)
{
	return (uint8_t *) (pnetif->hwaddr);
}

uint8_t* LwIP_GetIP(struct netif *pnetif)
{
	return (uint8_t *) &(pnetif->ip_addr);
}

uint8_t* LwIP_GetGW(struct netif *pnetif)
{
	return (uint8_t *) &(pnetif->gw);
}

uint8_t* LwIP_GetMASK(struct netif *pnetif)
{
	return (uint8_t *) &(pnetif->netmask);
}

 
#if LWIP_DNS
const struct ip_addr* LwIP_GetDNS(void)
{
#if LWIP_VERSION == LWIP_SCI		  
	return (dns_getserver(0));   
#elif LWIP_VERSION == LWIP_TUYA
	return (&dns_getserver(0));   	  
#endif	
}  
 
void LwIP_SetDNS(struct ip_addr* dns)
{
	dns_setserver(0, dns);
}
#endif


#if LWIP_IPV6
/* Get IPv6 address with lwip 1.5.0 */
void LwIP_AUTOIP_IPv6(struct netif *pnetif)
{
	uint8_t *ipv6 = (uint8_t *) &(pnetif->ip6_addr[0].addr[0]);

	netif_create_ip6_linklocal_address(pnetif, 1);
	printf("\nIPv6 link-local address: %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
	       ipv6[0], ipv6[1],  ipv6[2],  ipv6[3],  ipv6[4],  ipv6[5],  ipv6[6], ipv6[7],
	       ipv6[8], ipv6[9], ipv6[10], ipv6[11], ipv6[12], ipv6[13], ipv6[14], ipv6[15]);
}
#endif
