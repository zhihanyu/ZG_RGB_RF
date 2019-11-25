/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
  *
 * 
  *
  */
#ifndef LWIP_HDR_LWIPOPTS_H
#define LWIP_HDR_LWIPOPTS_H

#include "s907x.h"
 

/**
 * SYS_LIGHTWEIGHT_PROT==1: if you want inter-task protection for certain
 * critical regions during buffer allocation, deallocation and memory
 * allocation and deallocation.
 */
#define SYS_LIGHTWEIGHT_PROT    1

/* Define LWIP_COMPAT_MUTEX if the port has no mutexes and binary semaphores
 should be used instead */
#define LWIP_COMPAT_MUTEX       1

#define ETHARP_TRUST_IP_MAC     0
#define IP_REASSEMBLY           1
#define IP_FRAG                 1
#define ARP_QUEUEING            0

/**
 * NO_SYS==1: Provides VERY minimal functionality. Otherwise,
 * use lwIP facilities.
 */
#define NO_SYS                  0

#ifndef CONFIG_DYNAMIC_TICKLESS
#define CONFIG_DYNAMIC_TICKLESS 0
#endif


/* ---------- Memory options ---------- */
/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
#define MEM_ALIGNMENT           4
  
/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. */

#if CONFIG_HIGH_TP
    #define MEM_SIZE                (23*1024)
#else
    #define MEM_SIZE                (5*1024)
#endif

/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
   sends a lot of data out of ROM (or other static memory), this
   should be set high. */
#define MEMP_NUM_PBUF           100
/* MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
   per active UDP "connection". */
#define MEMP_NUM_UDP_PCB        6
/* MEMP_NUM_TCP_PCB: the number of simulatenously active TCP
   connections. */
#define MEMP_NUM_TCP_PCB        10
/* MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP
   connections. */
#define MEMP_NUM_TCP_PCB_LISTEN 5
/* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP
   segments. */
#ifdef CONFIG_HIGH_TP
    #define MEMP_NUM_TCP_SEG        60
#else
    #define MEMP_NUM_TCP_SEG        20
#endif
/* MEMP_NUM_SYS_TIMEOUT: the number of simulateously active
   timeouts. */
#define MEMP_NUM_SYS_TIMEOUT    10

#define MEMP_NUM_NETCONN        8

/* ---------- Pbuf options ---------- */
/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool. */
#if CONFIG_HIGH_TP
    #define PBUF_POOL_SIZE          60
#else
    #define PBUF_POOL_SIZE          60
#endif

/* IP_REASS_MAX_PBUFS: Total maximum amount of pbufs waiting to be reassembled.*/

#define IP_REASS_MAX_PBUFS              30 


/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */
#define PBUF_POOL_BUFSIZE       500


/* ---------- TCP options ---------- */
#define LWIP_TCP                1
#define TCP_TTL                 255

/* Controls if TCP should queue segments that arrive out of
   order. Define to 0 if your device is low on memory. */
#define TCP_QUEUE_OOSEQ         1

/* TCP Maximum segment size. */
#define TCP_MSS                 (1500 - 40)	  /* TCP_MSS = (Ethernet MTU - IP header size - TCP header size) */

/* TCP sender buffer space (bytes). */
#ifdef CONFIG_HIGH_TP
    #define TCP_SND_BUF             (10*TCP_MSS)
#else
    #define TCP_SND_BUF             (5*TCP_MSS)
#endif
/*  TCP_SND_QUEUELEN: TCP sender buffer space (pbufs). This must be at least
  as much as (2 * TCP_SND_BUF/TCP_MSS) for things to work. */

#ifdef CONFIG_HIGH_TP
    #define TCP_SND_QUEUELEN        (6* TCP_SND_BUF/TCP_MSS)
#else
    #define TCP_SND_QUEUELEN        (4* TCP_SND_BUF/TCP_MSS)
#endif

/* TCP receive window. */
#ifdef CONFIG_HIGH_TP
    #define TCP_WND                 (8*TCP_MSS)
#else
    #define TCP_WND                 (2*TCP_MSS)
#endif

#define TCP_MAXRTX                  3

/* ---------- ICMP options ---------- */
#define LWIP_ICMP                       1

/* ---------- ARP options ----------- */
#define LWIP_ARP                        1

/* ---------- DHCP options ---------- */
/* Define LWIP_DHCP to 1 if you want DHCP configuration of
   interfaces. DHCP is not implemented in lwIP 0.5.1, however, so
   turning this on does currently not work. */
#define LWIP_DHCP               1


/* ---------- UDP options ---------- */
#define LWIP_UDP                1
#define UDP_TTL                 255
/* ---------- DNS options ---------- */
#define LWIP_DNS                        1

/* ---------- UPNP options --------- */
#define LWIP_UPNP		0

/* ---------- SO_SNDRCVTIMEO_NONSTANDARD options --------- */
#define LWIP_SO_SNDRCVTIMEO_NONSTANDARD 1

/* ---------- SO_REUSE options --------- */
#define SO_REUSE                        1

/* Support Multicast */
#define LWIP_IGMP                   1
#define LWIP_RAND()                 wl_get_random32()
#define LWIP_SRAND()

#define LWIP_MTU_ADJUST 		1

/* Support TCP Keepalive */
#define LWIP_TCP_KEEPALIVE				1

#define LWIP_SCI_AT_PORT                1
#define LWIP_TCP_WINDOWS_EXT            LWIP_SCI_AT_PORT

/*LWIP_UART_ADAPTER==1: Enable LWIP_UART_ADAPTER when CONFIG_GAGENT is enabled, 
   because some GAGENT functions denpond on the following macro definitions.*/
#define LWIP_UART_ADAPTER                   0

#if LWIP_UART_ADAPTER || CONFIG_ETHERNET
#undef  LWIP_SO_SNDTIMEO        
#define LWIP_SO_SNDTIMEO                		    1

#undef  SO_REUSE        
#define SO_REUSE                        			1

#undef MEMP_NUM_NETCONN                	
#define MEMP_NUM_NETCONN                	        10

#undef TCP_WND                
#define TCP_WND                                       (4*TCP_MSS)

#define TCP_KEEPIDLE_DEFAULT			10000UL
#define TCP_KEEPINTVL_DEFAULT			1000UL
#define TCP_KEEPCNT_DEFAULT			    10U
#endif

#if CONFIG_EXAMPLE_UART_ATCMD || CONFIG_EXAMPLE_SPI_ATCMD 
#undef  LWIP_SO_SNDTIMEO        
#define LWIP_SO_SNDTIMEO                		1

#undef  SO_REUSE        
#define SO_REUSE                        		1

#undef SO_REUSE_RXTOALL
#define SO_REUSE_RXTOALL				        1

#undef MEMP_NUM_NETCONN                	
#define MEMP_NUM_NETCONN                	    10

#undef MEMP_NUM_TCP_PCB
#define MEMP_NUM_TCP_PCB				(MEMP_NUM_NETCONN)

#undef MEMP_NUM_UDP_PCB
#define MEMP_NUM_UDP_PCB				(MEMP_NUM_NETCONN)

#undef TCP_WND                
#define TCP_WND                          (4*TCP_MSS)

#define TCP_KEEPIDLE_DEFAULT			10000UL
#define TCP_KEEPINTVL_DEFAULT			1000UL
#define TCP_KEEPCNT_DEFAULT			    10U

#endif

#define ERRNO   1

#if CONFIG_EXAMPLE_AMAZON_ALEXA

#undef TCP_WND                
#define TCP_WND                                       	(4*TCP_MSS)

#undef TCP_SND_BUF
#define TCP_SND_BUF             (10*TCP_MSS)

#undef TCP_SND_QUEUELEN
#define TCP_SND_QUEUELEN        (4* TCP_SND_BUF/TCP_MSS)

#undef MEMP_NUM_TCP_SEG
#define MEMP_NUM_TCP_SEG        40

#endif

/* ---------- Statistics options ---------- */
#define LWIP_STATS 0
#define LWIP_PROVIDE_ERRNO 1
 
#define LWIP_NETIF_HOSTNAME             1
/*
   --------------------------------------
   ---------- Checksum options ----------
   --------------------------------------
*/

/* 
Certain platform allows computing and verifying the IP, UDP, TCP and ICMP checksums by hardware:
 - To use this feature let the following define uncommented.
 - To disable it and process by CPU comment the  the checksum.
*/
//Do checksum by lwip - WLAN nic does not support Checksum offload
//#define CHECKSUM_BY_HARDWARE 


#ifdef CHECKSUM_BY_HARDWARE
  /* CHECKSUM_GEN_IP==0: Generate checksums by hardware for outgoing IP packets.*/
  #define CHECKSUM_GEN_IP                 0
  /* CHECKSUM_GEN_UDP==0: Generate checksums by hardware for outgoing UDP packets.*/
  #define CHECKSUM_GEN_UDP                0
  /* CHECKSUM_GEN_TCP==0: Generate checksums by hardware for outgoing TCP packets.*/
  #define CHECKSUM_GEN_TCP                0 
  /* CHECKSUM_CHECK_IP==0: Check checksums by hardware for incoming IP packets.*/
  #define CHECKSUM_CHECK_IP               0
  /* CHECKSUM_CHECK_UDP==0: Check checksums by hardware for incoming UDP packets.*/
  #define CHECKSUM_CHECK_UDP              0
  /* CHECKSUM_CHECK_TCP==0: Check checksums by hardware for incoming TCP packets.*/
  #define CHECKSUM_CHECK_TCP              0
#else
  /* CHECKSUM_GEN_IP==1: Generate checksums in software for outgoing IP packets.*/
  #define CHECKSUM_GEN_IP                 1
  /* CHECKSUM_GEN_UDP==1: Generate checksums in software for outgoing UDP packets.*/
  #define CHECKSUM_GEN_UDP                1
  /* CHECKSUM_GEN_TCP==1: Generate checksums in software for outgoing TCP packets.*/
  #define CHECKSUM_GEN_TCP                1
  /* CHECKSUM_CHECK_IP==1: Check checksums in software for incoming IP packets.*/
  #define CHECKSUM_CHECK_IP               1
  /* CHECKSUM_CHECK_UDP==1: Check checksums in software for incoming UDP packets.*/
  #define CHECKSUM_CHECK_UDP              1
  /* CHECKSUM_CHECK_TCP==1: Check checksums in software for incoming TCP packets.*/
  #define CHECKSUM_CHECK_TCP              1
#endif


/*
   ----------------------------------------------
   ---------- Sequential layer options ----------
   ----------------------------------------------
*/
/**
 * LWIP_NETCONN==1: Enable Netconn API (require to use api_lib.c)
 */
#define LWIP_NETCONN                    1

/*
   ------------------------------------
   ---------- Socket options ----------
   ------------------------------------
*/
/**
 * LWIP_SOCKET==1: Enable Socket API (require to use sockets.c)
 */
#define LWIP_SOCKET                     1	

/*
   -----------------------------------
   ---------- DEBUG options ----------
   -----------------------------------
*/

#define LWIP_DEBUG                      0


/*
   ---------------------------------
   ---------- OS options ----------
   ---------------------------------
*/

#define TCPIP_THREAD_STACKSIZE          1024
#define TCPIP_MBOX_SIZE                 6
#define DEFAULT_UDP_RECVMBOX_SIZE       6
#define DEFAULT_TCP_RECVMBOX_SIZE       6
#define DEFAULT_RAW_RECVMBOX_SIZE		6
#define DEFAULT_ACCEPTMBOX_SIZE         6
#define DEFAULT_THREAD_STACKSIZE        500
#define TCPIP_THREAD_PRIO               (MAX_PRIORITIES - 2)

/* 
#define ETHARP_SUPPORT_STATIC_ENTRIES   1	  
#define LWIP_RANDOMIZE_INITIAL_LOCAL_PORTS 1
#define LWIP_DNS_LEGACY_SUPPORT 0
*/
#define SNTP_SERVER_DNS                 1
#if (LWIP_VERSION == LWIP_SCI)
#define LWIP_NETCONN_SEM_PER_THREAD     1
#define LWIP_COMPAT_MUTEX_ALLOWED       0
#define LWIP_TCPIP_CORE_LOCKING         0
//for lwip iperf patch 2.1.2
#define LWIP_ASSERT_CORE_LOCKED()
#elif (LWIP_VERSION == LWIP_TUYA)
#define LWIP_TCPIP_CORE_LOCKING         1
#endif

#define LWIP_TCPIP_TIMEOUT              1
#define LWIP_SO_RCVTIMEO                1
#define LWIP_SOCKET_SET_ERRNO           0
#undef LWIP_DEBUG
#define LWIP_RAW                        1
#define LWIP_AUTOIP                     1
#define TCPIP_THREAD_NAME              "TCP_IP" 

#define LWIP_NETIF_STATUS_CALLBACK      1

#define LWIP_IPV6                       0
#if LWIP_IPV6
#undef  MEMP_NUM_SYS_TIMEOUT
#define MEMP_NUM_SYS_TIMEOUT            13
#endif
     
#include "lwip/init.h"                  //for version control

#endif /* LWIP_HDR_LWIPOPTS_H */
