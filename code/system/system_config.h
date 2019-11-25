#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H


#ifdef __cplusplus
 extern "C" {
#endif
//IAR
#if defined (__ICCARM__)

#define _SLINK(s) #s
#define IAR_SECTION(_name) 	            _Pragma( _SLINK(location=_name))

#define SECTION(_name)                  IAR_SECTION(_name)    
#define WEAK_DECLEAR   					__weak



#else //gcc or keil          
#define SECTION(_name)                  __attribute__ ((__section__(_name)))
#define WEAK_DECLEAR   					__attribute__((weak))
#define __weak       					WEAK_DECLEAR
#define _TIMER_T_DECLARED
#define _TIMEVAL_DEFINED


#endif

//heap region for keil
#if defined (__CC_ARM)
extern unsigned int Image$$USER_CODE_REGION$$Length;
extern char Image$$USER_CODE_REGION$$Base;
extern char Image$$USER_CODE_REGION$$Limit;

extern unsigned int Image$$OTA_RAM_DATA$$Length;
extern char Image$$OTA_RAM_DATA$$Base;
extern char Image$$OTA_RAM_DATA$$Limit;

extern unsigned int Image$$OTA_RAM_BSS$$Length;
extern char Image$$OTA_RAM_BSS$$Base;
extern char Image$$OTA_RAM_BSS$$Limit;
#endif

//low power config
#define S907X_LOW_POWER     0
//heap ram end config
#define MSP_TOP             0x10046FFF
#define HEAP_DATA_END       (MSP_TOP - 0x2000)
//LWIP config
#define CONFIG_HIGH_TP      0
//LWIP select
#define  LWIP_TUYA          1 //1.4.1
#define  LWIP_SCI           0 //2.0.3
#define  LWIP_VERSION       LWIP_TUYA

//LWIP STA Static IP ADDRESS
#define IP_ADDR0   192
#define IP_ADDR1   168
#define IP_ADDR2   1
#define IP_ADDR3   100
//LWIP STA Static IP ADDRESSNETMASK
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0
//LWIP STA Static Gateway Address
#define GW_ADDR0   192
#define GW_ADDR1   168
#define GW_ADDR2   1
#define GW_ADDR3   1
//LWIP AP Static IP ADDRESS
#define AP_IP_ADDR0   192
#define AP_IP_ADDR1   168
#define AP_IP_ADDR2   55
#define AP_IP_ADDR3   1
//LWIP AP Static NETMASK
#define AP_NETMASK_ADDR0   255
#define AP_NETMASK_ADDR1   255
#define AP_NETMASK_ADDR2   255
#define AP_NETMASK_ADDR3   0

/*Gateway Address*/
#define AP_GW_ADDR0   	 192
#define AP_GW_ADDR1   	 168
#define AP_GW_ADDR2   	 55
#define AP_GW_ADDR3   	 1  
//Mbtls select
#define  MBEDTLS_TUYA		1
#define  MBEDTLS_SCI		2
#define  MBEDTLS_VERSION    MBEDTLS_SCI
//Mbtls config
#define CONFIG_NET_TLS_ENABLE           1
#if CONFIG_NET_TLS_ENABLE
#define CONFIG_NET_HTTPD_OVER_TLS       1
#define CONFIG_NET_HTTPC_OVER_TLS       1
#define CONFIG_NET_MQTT_OVER_TLS        0
#else
#define CONFIG_NET_HTTPD_OVER_TLS       0
#define CONFIG_NET_HTTPC_OVER_TLS       0
#define CONFIG_NET_MQTT_OVER_TLS        0
#endif
//OTA config
#define CONFIG_OTA_DUAL_IMAGES          1   // Default 0 using SingleImage as AOS, set to 1 using dual images(pingpong)

//for coustom
#define TUYA_BUILD			0
#define ZG_BUILD                        1


//system section config
#define SYSTEM_ENTRY_SECTION             SECTION(".system.entry.data")
#define HEAP_START_SECTION               SECTION(".heap.start.data")


//sdk version
#define VERSION_LENGTH          40
// for platform
#define AOS_STR                 "A"
#define FREERTOS_STR            "F"
// for power consumption
#define FUNC_POWERSAVE          "P"
#define FUNC_NORMAL             "N"
// for data format
#define FORMAT_PASSTHROUGH      "T"
#define FORMAT_RAWDATA          "R"
#define FORMAT_CJSON            "C"
// for eval_board
#define EVAL_BOARD              (1)
// need to define by developer start
#define COUSTOM_STR             "SCI"

#define DATA_FORMAT             (FORMAT_CJSON)
#define MAJOR_VERSION           (1)
#define MINOR_VERSION           (0)
#define REVISION_NUMBER         (4)
// need to define by developer over
#ifdef S907X_AOS
#define SDK_PLATFORM_STR        AOS_STR
#else
#define SDK_PLATFORM_STR        FREERTOS_STR
#endif

#ifdef S907X_AOS    
	#ifdef AOS_COMP_PWRMGMT    
	#define SDK_FUNCTION_STR        FUNC_POWERSAVE   
	#else    
	#define SDK_FUNCTION_STR        FUNC_NORMAL   
	#endif
#else
	#if S907X_LOW_POWER    
	#define SDK_FUNCTION_STR        FUNC_POWERSAVE   
	#else    
	#define SDK_FUNCTION_STR        FUNC_NORMAL   
	#endif
#endif


#ifndef COUSTOM_STR
#define COUSTOM_STR             "ND"
#endif
#ifndef MAJOR_VERSION
#define MAJOR_VERSION           "1"
#endif

#ifndef MINOR_VERSION
#define MINOR_VERSI             "0"
#endif
#ifndef REVISION_NUMBER
#define REVISION_NUMBER         "0"
#endif

// HF-20190821-APE-1.0.X
#if (EVAL_BOARD == 1)
#define VERSION_FMT             "%s-%04d%02d%02d-%s%s%s%s-%d.%d.%d"
#else
#define VERSION_FMT             "%s-%04d%02d%02d-%s%s%s-%d.%d.%d"
#endif

#define YEAR ((((__DATE__[7] - '0') * 10 + (__DATE__[8] - '0')) * 10 \
    + (__DATE__[9] - '0')) * 10 + (__DATE__[10] - '0'))

#define MONTH (__DATE__[2] == 'c' ? 0 \
    : __DATE__[2] == 'b' ? 1 \
    : __DATE__[2] == 'r' ? (__DATE__[0] == 'M' ? 2 : 3) \
    : __DATE__[2] == 'y' ? 4 \
    : __DATE__[2] == 'n' ? 5 \
    : __DATE__[2] == 'l' ? 6 \
    : __DATE__[2] == 'g' ? 7 \
    : __DATE__[2] == 'p' ? 8 \
    : __DATE__[2] == 't' ? 9 \
    : __DATE__[2] == 'v' ? 10 : 11)

#define DAY ((__DATE__[4] == ' ' ? 0 : __DATE__[4] - '0') * 10 \
+ (__DATE__[5] - '0'))


//SDK version
#define SDK_COUSTOM         0        
#define SDK_FACTORY         1
#define S907X_SDK_VERSION   SDK_COUSTOM


#if S907X_SDK_VERSION == SDK_FACTORY
#ifdef S907X_AOS
#define S907X_SDK_VERSION_STR    "1.0.1_AI"
#else
#define S907X_SDK_VERSION_STR    "1.0.3_FI"
#endif

#else
#ifdef S907X_AOS
#define S907X_SDK_VERSION_STR    "1.0.1_AE"
#else
#define S907X_SDK_VERSION_STR    "1.0.3_FE"
#endif

#endif
//AT commad config
#define M_AT_ENABLE        1
#if M_AT_ENABLE == 1
#define M_AT_ESP           0
#define M_AT_TEST          1

//AT ESP config
#if M_AT_ESP && M_AT_TEST   
#error "close M_AT_TEST if you want to open M_AT_ESP"
#endif
#endif


//includes
#include "dlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h> /* va_list */
#include "system.h"
#include "at_cmd.h"
#include "hal_test.h"



#ifdef __cplusplus
 }
#endif

#endif
