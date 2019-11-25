
include $(RULES_DIR)/.config

NET_DHCPC_SOURCES   = ./dhcpc/dhcpc.c

NET_DHCPS_SOURCES   = ./dhcps/dhcps.c

NET_PING_SOURCES    = ./ping/ping.c

NET_HTTPC_SOURCES   = ./httpc/httpc_api.c    \
                        ./httpc/httpc_core.c

NET_MQTT_SOURCES    = ./mqtt/MQTTPacket/src/MQTTConnectClient.c    \
                    ./mqtt/MQTTPacket/src/MQTTDeserializePublish.c    \
                    ./mqtt/MQTTPacket/src/MQTTFormat.c    \
                    ./mqtt/MQTTPacket/src/MQTTPacket.c    \
                    ./mqtt/MQTTPacket/src/MQTTSerializePublish.c    \
                    ./mqtt/MQTTPacket/src/MQTTSubscribeClient.c    \
                    ./mqtt/MQTTPacket/src/MQTTSubscribeServer.c    \
                    ./mqtt/MQTTPacket/src/MQTTUnsubscribeClient.c    \
                    ./mqtt/MQTTPacket/samples/transport.c
 
ifeq ($(CONFIG_NET_LWIP), 1)
ifeq ($(CONFIG_NET_LWIP14), 1)
lwip_path    = ./lwip/lwip_v1.4.1
else
ifeq ($(CONFIG_NET_LWIP20), 1)
lwip_path   = ./lwip/lwip_v2.0.3
endif
endif
                 
ifeq ($(CONFIG_NET_LWIP_APP_HTTPD), 1)
NET_LWIP_APP_SOURCES    += $(lwip_path)/src/apps/httpd/httpd.c
endif

ifeq ($(CONFIG_NET_LWIP_APP_IPERF), 1)
NET_LWIP_APP_SOURCES    += $(lwip_path)/src/apps/lwiperf/sci_lwiperf.c
endif

ifeq ($(CONFIG_NET_LWIP_APP_SNTP), 1)
NET_LWIP_APP_SOURCES    += $(lwip_path)/src/apps/sntp/sntp.c             \
                        $(lwip_path)/src/apps/sntp/sntp_api.c         \
                        $(lwip_path)/src/apps/sntp/sntp_time.c
endif

NET_LWIP_SOURCES    = $(lwip_path)/src/api/api_lib.c    \
                        $(lwip_path)/src/api/api_msg.c    \
                        $(lwip_path)/src/api/err.c    \
                        $(lwip_path)/src/api/netbuf.c    \
                        $(lwip_path)/src/api/netdb.c    \
                        $(lwip_path)/src/api/netifapi.c    \
                        $(lwip_path)/src/api/sockets.c    \
                        $(lwip_path)/src/api/tcpip.c    \
                        $(lwip_path)/src/core/ipv4/autoip.c    \
                        $(lwip_path)/src/core/ipv4/icmp.c    \
                        $(lwip_path)/src/core/ipv4/igmp.c    \
                        $(lwip_path)/src/core/ipv4/inet.c    \
                        $(lwip_path)/src/core/ipv4/inet_chksum.c    \
                        $(lwip_path)/src/core/ipv4/ip.c    \
                        $(lwip_path)/src/core/ipv4/ip_addr.c    \
                        $(lwip_path)/src/core/ipv4/ip_frag.c    \
                        $(lwip_path)/src/core/ipv6/icmp6.c    \
                        $(lwip_path)/src/core/ipv6/inet6.c    \
                        $(lwip_path)/src/core/ipv6/ip6.c    \
                        $(lwip_path)/src/core/ipv6/ip6_addr.c    \
                        $(lwip_path)/src/core/snmp/asn1_dec.c    \
                        $(lwip_path)/src/core/snmp/asn1_enc.c    \
                        $(lwip_path)/src/core/snmp/mib2.c    \
                        $(lwip_path)/src/core/snmp/mib_structs.c    \
                        $(lwip_path)/src/core/snmp/msg_in.c    \
                        $(lwip_path)/src/core/snmp/msg_out.c    \
                        $(lwip_path)/src/core/def.c    \
                        $(lwip_path)/src/core/dhcp.c    \
                        $(lwip_path)/src/core/dns.c    \
                        $(lwip_path)/src/core/init.c    \
                        $(lwip_path)/src/core/lwip_timers.c    \
                        $(lwip_path)/src/core/mem.c    \
                        $(lwip_path)/src/core/memp.c    \
                        $(lwip_path)/src/core/netif.c    \
                        $(lwip_path)/src/core/pbuf.c    \
                        $(lwip_path)/src/core/raw.c    \
                        $(lwip_path)/src/core/stats.c    \
                        $(lwip_path)/src/core/sys.c    \
                        $(lwip_path)/src/core/tcp.c    \
                        $(lwip_path)/src/core/tcp_in.c    \
                        $(lwip_path)/src/core/tcp_out.c    \
                        $(lwip_path)/src/core/udp.c    \
                        $(lwip_path)/src/netif/ppp/auth.c    \
                        $(lwip_path)/src/netif/ppp/chap.c    \
                        $(lwip_path)/src/netif/ppp/chpms.c    \
                        $(lwip_path)/src/netif/ppp/fsm.c    \
                        $(lwip_path)/src/netif/ppp/ipcp.c    \
                        $(lwip_path)/src/netif/ppp/lcp.c    \
                        $(lwip_path)/src/netif/ppp/lwip_md5.c    \
                        $(lwip_path)/src/netif/ppp/magic.c    \
                        $(lwip_path)/src/netif/ppp/pap.c    \
                        $(lwip_path)/src/netif/ppp/ppp.c    \
                        $(lwip_path)/src/netif/ppp/ppp_oe.c    \
                        $(lwip_path)/src/netif/ppp/randm.c    \
                        $(lwip_path)/src/netif/ppp/vj.c    \
                        $(lwip_path)/src/netif/etharp.c    \
                        $(lwip_path)/src/netif/slipif.c   \
                        $(lwip_path)/port/sci/freertos/sys_arch.c 

## Not included file for lwip ##
#                        ./lwip/lwip_v1.4.1/port/lwip_mod/api_lib.c    \
#                        ./lwip/lwip_v1.4.1/port/lwip_mod/api_msg.c    \
#                        ./lwip/lwip_v1.4.1/port/lwip_mod/sockets.c    \
#                        ./lwip/lwip_v1.4.1/port/lwip_mod/sys_arch.c    \
#                        ./lwip/lwip_v1.4.1/port/lwip_mod/tcp.c    \
#                        ./lwip/lwip_v1.4.1/port/lwip_mod/tcpip.c    \
#                        ./lwip/lwip_v1.4.1/port/realtek/freertos/ethernetif.c    \
#                        ./lwip/lwip_v1.4.1/port/realtek/freertos/sys_arch.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/httpd/fs.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/httpd/fsdata.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/httpd/makefsdata/makefsdata.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/mdns/mdns.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/mqtt/mqtt.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/netbiosns/netbiosns.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/snmp/snmpv3.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/snmp/snmpv3_dummy.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/snmp/snmpv3_mbedtls.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/snmp/snmp_asn1.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/snmp/snmp_core.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/snmp/snmp_mib2.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/snmp/snmp_mib2_icmp.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/snmp/snmp_mib2_interfaces.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/snmp/snmp_mib2_ip.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/snmp/snmp_mib2_snmp.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/snmp/snmp_mib2_system.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/snmp/snmp_mib2_tcp.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/snmp/snmp_mib2_udp.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/snmp/snmp_msg.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/snmp/snmp_netconn.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/snmp/snmp_pbuf_stream.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/snmp/snmp_raw.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/snmp/snmp_scalar.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/snmp/snmp_table.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/snmp/snmp_threadsync.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/snmp/snmp_traps.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/sntp/sntp.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/sntp/sntp_api.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/sntp/sntp_time.c    \
#                        ./lwip/lwip_v1.4.1/src/apps/tftp/tftp_server.c    \
#                        ./lwip/lwip_v1.4.1/port/sci/freertos/ethernetif.c    \
#                        ./lwip/lwip_v1.4.1/src/netif/ethernetif.c 
endif 
#endif for CONFIG_NET_LWIP

NET_MBEDTLS_TUYA_SOURCES	= ./mbedtls/aes.c    \
                        ./mbedtls/aesni.c    \
                        ./mbedtls/arc4.c    \
                        ./mbedtls/asn1parse.c    \
                        ./mbedtls/asn1write.c    \
                        ./mbedtls/base64.c    \
                        ./mbedtls/bignum.c    \
                        ./mbedtls/blowfish.c    \
                        ./mbedtls/camellia.c    \
                        ./mbedtls/ccm.c    \
                        ./mbedtls/certs.c    \
                        ./mbedtls/cipher.c    \
                        ./mbedtls/cipher_wrap.c    \
                        ./mbedtls/cmac.c    \
                        ./mbedtls/ctr_drbg.c    \
                        ./mbedtls/debug.c    \
                        ./mbedtls/des.c    \
                        ./mbedtls/dhm.c    \
                        ./mbedtls/ecdh.c    \
                        ./mbedtls/ecdsa.c    \
                        ./mbedtls/ecjpake.c    \
                        ./mbedtls/ecp.c    \
                        ./mbedtls/ecp_curves.c    \
                        ./mbedtls/entropy.c    \
                        ./mbedtls/entropy_poll.c    \
                        ./mbedtls/error.c    \
                        ./mbedtls/gcm.c    \
                        ./mbedtls/havege.c    \
                        ./mbedtls/hmac_drbg.c    \
                        ./mbedtls/md.c    \
                        ./mbedtls/md2.c    \
                        ./mbedtls/md4.c    \
                        ./mbedtls/md5.c    \
                        ./mbedtls/md_wrap.c    \
                        ./mbedtls/memory_buffer_alloc.c    \
                        ./mbedtls/net_sockets.c    \
                        ./mbedtls/oid.c    \
                        ./mbedtls/padlock.c    \
                        ./mbedtls/pem.c    \
                        ./mbedtls/pk.c    \
                        ./mbedtls/pkcs11.c    \
                        ./mbedtls/pkcs12.c    \
                        ./mbedtls/pkcs5.c    \
                        ./mbedtls/pkparse.c    \
                        ./mbedtls/pkwrite.c    \
                        ./mbedtls/pk_wrap.c    \
                        ./mbedtls/platform.c    \
                        ./mbedtls/ripemd160.c    \
                        ./mbedtls/rsa.c    \
                        ./mbedtls/rsa_internal.c    \
                        ./mbedtls/sha1.c    \
                        ./mbedtls/sha256.c    \
                        ./mbedtls/sha512.c    \
                        ./mbedtls/ssl_cache.c    \
                        ./mbedtls/ssl_ciphersuites.c    \
                        ./mbedtls/ssl_cli.c    \
                        ./mbedtls/ssl_cookie.c    \
                        ./mbedtls/ssl_srv.c    \
                        ./mbedtls/ssl_ticket.c    \
                        ./mbedtls/ssl_tls.c    \
                        ./mbedtls/threading.c    \
                        ./mbedtls/timing.c    \
                        ./mbedtls/version.c    \
                        ./mbedtls/version_features.c    \
                        ./mbedtls/x509.c    \
                        ./mbedtls/x509write_crt.c    \
                        ./mbedtls/x509write_csr.c    \
                        ./mbedtls/x509_create.c    \
                        ./mbedtls/x509_crl.c    \
                        ./mbedtls/x509_crt.c    \
                        ./mbedtls/x509_csr.c    \
                        ./mbedtls/xtea.c
                        
ifeq ($(CONFIG_NET_DHCPC), 1)
MODULE_SOURCES = $(NET_DHCPC_SOURCES)
endif

ifeq ($(CONFIG_NET_DHCPS), 1)
MODULE_SOURCES += $(NET_DHCPS_SOURCES)
endif

ifeq ($(CONFIG_NET_PING), 1)
MODULE_SOURCES += $(NET_PING_SOURCES)
endif

ifeq ($(CONFIG_NET_MQTT), 1)
MODULE_SOURCES += $(NET_MQTT_SOURCES)
endif

ifeq ($(CONFIG_NET_HTTPC), 1)
MODULE_SOURCES += $(NET_HTTPC_SOURCES)
endif

ifeq ($(CONFIG_NET_LWIP_APP), 1)
MODULE_SOURCES += $(NET_LWIP_APP_SOURCES)
endif

ifeq ($(CONFIG_NET_LWIP), 1)
MODULE_SOURCES += $(NET_LWIP_SOURCES)
endif

ifeq ($(CONFIG_NET_MBEDTLS_TUYA), 1)
MODULE_SOURCES += $(NET_MBEDTLS_TUYA_SOURCES)
endif

include $(RULES_DIR)/make_objects.mk

