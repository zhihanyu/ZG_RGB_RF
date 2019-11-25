
#ifndef __HTTPCLIENT_H__
#define __HTTPCLIENT_H__

#include <stdint.h>
#include <stdbool.h>
#include "lwip/sockets.h"
#include "system_config.h"



#include "mbedtls/net.h"
#include "mbedtls/ssl.h"
#include "mbedtls/certs.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"


#ifdef __cplusplus
extern "C"
{
#endif

#define HTTPC_AUTHB_SIZE     128

#define HTTPC_CHUNK_SIZE     512

#define HTTPC_MAX_HOST_LEN   64
#define HTTPC_MAX_URL_LEN    256


//#define CONFIG_NET_HTTPC_OVER_TLS  1
 
/*defgroup httpc_define Define
  *
  */
/*brief   This macro defines the deault HTTP port.  */
#define HTTP_PORT       80

/*brief   This macro defines the deault HTTPS port.  */
#define HTTPS_PORT      443

/**
 *
 */

/* 
 * Define http client enum
 * brief    This enumeration defines the HTTP request type. 
 */
typedef enum {
    HTTPC_GET,
    HTTPC_POST,
    HTTPC_PUT,
    HTTPC_DELETE,
    HTTPC_HEAD
} HTTPC_REQUEST_TYPE;

/*
 * brief     Defines the httpclient API result.
 * NOTE: Should keep same to httpc_api.h
 */
typedef enum {
    HTTPC_OK = 0,                           /* <Success */
    HTTPC_ERR_PARSE,                        /* <URL parse error */
    HTTPC_ERR_DNS,                          /* <Counld not resolve hostname */
    HTTPC_ERR_PRTCL,                        /* <Protocol error */
    HTTPC_ERR_NOTFOUND,                     /* <HTTP 404 error */
    HTTPC_ERR_REFUSED,                      /* <HTTP 403 error */
    HTTPC_ERR_XX,                           /* <HTTP XXX error */
    HTTPC_ERR_TIMEOUT,                      /* <Connection timeout */
    HTTPC_ERR_CONN,                         /* <Connection failed */
    HTTPC_CLOSED,                           /* < Connection was closed by remote host*/
    HTTPC_MORE_DATA,                        /* <More data needs to be retrieved */
} HTTPC_RESULT;
    
typedef enum {
    HTTPC_RSP_CONTENT_LEN = 0,
    HTTPC_RETRIEVE_LEN,
    HTTPC_POST_BUFF_LEN,
    HTTPC_RSP_BUFF_LEN,
    HTTPC_HEADER_BUFF_LEN,
} HTTPC_RESPONSE_LEN_TYPE;

/** @brief   This structure defines the HTTP data structure.  */
typedef struct {
    bool is_more;                /**< Indicates if more data needs to be retrieved. */
    bool is_chunked;             /**< Response data is encoded in portions/chunks.*/
    int retrieve_len;            /**< Content length to be retrieved. */
    int response_content_len;    /**< Response content length. */
    int post_buf_len;            /**< Post data length. */
    int response_buf_len;        /**< Response body buffer length. */
    int header_buf_len;          /**< Response head buffer lehgth. */
    char *post_content_type;     /**< Content type of the post data. */
    const char *post_buf;        /**< User data to be posted. */
    char *response_buf;          /**< Buffer to store the response body data. */
    char *header_buf;            /**< Buffer to store the response head data. */
} httpc_data_t;

/** @defgroup httpc_struct Struct
  * @{
  */
/** @brief   This structure defines the httpc_t structure.  */
typedef struct {
    int socket;                     /**< Socket ID. */
    int remote_port;                /**< HTTP or HTTPS port. */
    int response_code;              /**< Response code. */
    char *header;                   /**< Request custom header. */
    char *auth_user;                /**< Username for basic authentication. */
    char *auth_password;            /**< Password for basic authentication. */
    bool is_http;                   /**< Http connection? if 1, http; if 0, https. */
#if CONFIG_NET_HTTPC_OVER_TLS
    const char *server_cert;        /**< Server certification. */
    const char *client_cert;        /**< Client certification. */
    const char *client_pk;          /**< Client private key. */
    int server_cert_len;            /**< Server certification lenght, server_cert buffer size. */
    int client_cert_len;            /**< Client certification lenght, client_cert buffer size. */
    int client_pk_len;              /**< Client private key lenght, client_pk buffer size. */
    void *ssl;                      /**< Ssl content. */
    const char *local_server;       /**< Local https server to test. */
#endif
    httpc_data_t *client_data;      /**< HTTP data struct. */
} httpc_t;


HTTPC_RESULT httpc_parse_url(const char *url, char *scheme, size_t max_scheme_len, char *host, size_t maxhost_len, int *port, char *path, size_t max_path_len);
HTTPC_RESULT httpc_conn(httpc_t *client, char *host);

HTTPC_RESULT httpc_send_header(httpc_t *client, const char *url, int method, httpc_data_t *client_data);
HTTPC_RESULT httpc_send_userdata(httpc_t *client, httpc_data_t *client_data);
HTTPC_RESULT httpc_recv(httpc_t *client, char *buf, int min_len, int max_len, int *p_read_len);
HTTPC_RESULT httpc_retrieve_content(httpc_t *client, char *data, int len, httpc_data_t *client_data);
HTTPC_RESULT httpc_response_parse(httpc_t *client, char *data, int len, httpc_data_t *client_data);

HTTPC_RESULT httpc_tls_conn(httpc_t *client, char *host);
int httpc_tls_close(httpc_t *client);


/* The following content is used in the HttpClient module. */
#ifndef DEBUG_LEVEL_NONE
#define HTTPCLIENT_DEBUG 1
#else
#define HTTPCLIENT_DEBUG 0
#endif

#if CONFIG_NET_HTTPC_OVER_TLS
typedef struct {
    mbedtls_ssl_context ssl_ctx;        /* mbedtls ssl context */
    mbedtls_net_context net_ctx;        /* Fill in socket id */
    mbedtls_ssl_config ssl_conf;        /* SSL configuration */
    //mbedtls_entropy_context entropy;
    //mbedtls_ctr_drbg_context ctr_drbg;
    //mbedtls_x509_crt_profile profile;
    mbedtls_x509_crt cacert;
    mbedtls_x509_crt clicert;
    mbedtls_pk_context pkey;
} httpc_tls_t;
#endif

#ifdef __cplusplus
}
#endif

#endif /* __HTTPCLIENT_H__ */

