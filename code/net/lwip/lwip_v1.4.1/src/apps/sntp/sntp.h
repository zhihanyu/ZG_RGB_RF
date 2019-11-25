#ifndef LWIP_HDR_APPS_SNTP_H
#define LWIP_HDR_APPS_SNTP_H


#include "lwip/ip_addr.h"
#include "lwip/opt.h"


/** SNTP macro to change system time in seconds
 * Define SNTP_SET_SYSTEM_TIME_US(sec, us) to set the time in microseconds instead of this one
 * if you need the additional precision.
 */
#ifndef SNTP_SET_SYSTEM_TIME
#define SNTP_SET_SYSTEM_TIME(sec)   sntp_set_system_time(sec)
#endif

/** The maximum number of SNTP servers that can be set */
#ifndef SNTP_MAX_SERVERS
#define SNTP_MAX_SERVERS           3
#endif

/** Set this to 1 to implement the callback function called by dhcp when
 * NTP servers are received. */
#ifndef SNTP_GET_SERVERS_FROM_DHCP
#define SNTP_GET_SERVERS_FROM_DHCP 0
#endif

/* Set this to 1 to support DNS names (or IP address strings) to set sntp servers */
#ifndef SNTP_SERVER_DNS
#define SNTP_SERVER_DNS            1
#endif

/** One server address/name can be defined as default if SNTP_SERVER_DNS == 1:
 * #define SNTP_SERVER_ADDRESS "pool.ntp.org"
 */

/**
 * SNTP_DEBUG: Enable debugging for SNTP.
 */
#ifndef SNTP_DEBUG
#define SNTP_DEBUG                  LWIP_DBG_OFF
#endif

/** SNTP server port */
#ifndef SNTP_PORT
#define SNTP_PORT                   123
#endif

/** Set this to 1 to allow config of SNTP server(s) by DNS name */
#ifndef SNTP_SERVER_DNS
#define SNTP_SERVER_DNS             0
#endif

/** Sanity check:
 * Define this to
 * - 0 to turn off sanity checks (default; smaller code)
 * - >= 1 to check address and port of the response packet to ensure the
 *        response comes from the server we sent the request to.
 * - >= 2 to check returned Originate Timestamp against Transmit Timestamp
 *        sent to the server (to ensure response to older request).
 * - >= 3 @todo: discard reply if any of the LI, Stratum, or Transmit Timestamp
 *        fields is 0 or the Mode field is not 4 (unicast) or 5 (broadcast).
 * - >= 4 @todo: to check that the Root Delay and Root Dispersion fields are each
 *        greater than or equal to 0 and less than infinity, where infinity is
 *        currently a cozy number like one second. This check avoids using a
 *        server whose synchronization source has expired for a very long time.
 */
#ifndef SNTP_CHECK_RESPONSE
#define SNTP_CHECK_RESPONSE         0
#endif

/** According to the RFC, this shall be a random delay
 * between 1 and 5 minutes (in milliseconds) to prevent load peaks.
 * This can be defined to a random generation function,
 * which must return the delay in milliseconds as u32_t.
 * Turned off by default.
 */
#ifndef SNTP_STARTUP_DELAY
#define SNTP_STARTUP_DELAY          0
#endif

/** If you want the startup delay to be a function, define this
 * to a function (including the brackets) and define SNTP_STARTUP_DELAY to 1.
 */
#ifndef SNTP_STARTUP_DELAY_FUNC
#define SNTP_STARTUP_DELAY_FUNC     SNTP_STARTUP_DELAY
#endif

/** SNTP receive timeout - in milliseconds
 * Also used as retry timeout - this shouldn't be too low.
 * Default is 3 seconds.
 */
#ifndef SNTP_RECV_TIMEOUT
#define SNTP_RECV_TIMEOUT           3000
#endif

/** SNTP update delay - in milliseconds
 * Default is 1 hour. Must not be beolw 15 seconds by specification (i.e. 15000)
 */
#ifndef SNTP_UPDATE_DELAY
#define SNTP_UPDATE_DELAY           600000  //1min
#endif

/** SNTP macro to get system time, used with SNTP_CHECK_RESPONSE >= 2
 * to send in request and compare in response.
 */
#ifndef SNTP_GET_SYSTEM_TIME
#define SNTP_GET_SYSTEM_TIME(sec, us)     do { (sec) = 0; (us) = 0; } while(0)
#endif

/** Default retry timeout (in milliseconds) if the response
 * received is invalid.
 * This is doubled with each retry until SNTP_RETRY_TIMEOUT_MAX is reached.
 */
#ifndef SNTP_RETRY_TIMEOUT
#define SNTP_RETRY_TIMEOUT          SNTP_RECV_TIMEOUT
#endif

/** Maximum retry timeout (in milliseconds). */
#ifndef SNTP_RETRY_TIMEOUT_MAX
#define SNTP_RETRY_TIMEOUT_MAX      (SNTP_RETRY_TIMEOUT * 10)
#endif

/** Increase retry timeout with every retry sent
 * Default is on to conform to RFC.
 */
#ifndef SNTP_RETRY_TIMEOUT_EXP
#define SNTP_RETRY_TIMEOUT_EXP      1
#endif

//----






#ifdef __cplusplus
extern "C" {
#endif

/* SNTP operating modes: default is to poll using unicast.
   The mode has to be set before calling sntp_init(). */
#define SNTP_OPMODE_POLL            0
#define SNTP_OPMODE_LISTENONLY      1
void sntp_setoperatingmode(u8_t operating_mode);
u8_t sntp_getoperatingmode(void);

void sntp_init(void);
void sntp_stop(void);
u8_t sntp_enabled(void);

void sntp_setserver(u8_t idx, const ip_addr_t *addr);
ip_addr_t sntp_getserver(u8_t idx);

#if SNTP_SERVER_DNS
void sntp_setservername(u8_t idx, char *server);
char *sntp_getservername(u8_t idx);
#endif /* SNTP_SERVER_DNS */

#if SNTP_GET_SERVERS_FROM_DHCP
void sntp_servermode_dhcp(int set_servers_from_dhcp);
#else /* SNTP_GET_SERVERS_FROM_DHCP */
#define sntp_servermode_dhcp(x)
#endif /* SNTP_GET_SERVERS_FROM_DHCP */

#ifdef __cplusplus
}
#endif

#endif /* LWIP_HDR_APPS_SNTP_H */
