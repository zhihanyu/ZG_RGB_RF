/* Copyright (C) 2012 mbed.org, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
 
#include "httpc_core.h"
#include "lwip/sockets.h"
#include "stdio.h"
#include "lwip/netdb.h"
#include "lwip/tcp.h"
#include "lwip/err.h"
#if CONFIG_NET_HTTPC_OVER_TLS
#include "mbedtls/debug.h"
#endif


#define HTTPC_SEND_BUF_SIZE  512

#define MIN(x,y) (((x)<(y))?(x):(y))
#define MAX(x,y) (((x)>(y))?(x):(y))

#if defined(MBEDTLS_DEBUG_C)
#define DEBUG_LEVEL 2
#endif

//MartinPan
#if CONFIG_NET_HTTPC_OVER_TLS
static int httpc_tls_send_all(mbedtls_ssl_context *ssl, const char *data, size_t length);
static int httpc_tls_nonblock_recv(void *ctx, unsigned char *buf, size_t len);
int mbedtls_platform_set_calloc_free( void * (*calloc_func)( size_t, size_t ),void (*free_func)( void * ) );
#endif

void wl_get_random_bytes(unsigned char *output, size_t output_len)
{
    int i = 0;

    for(i = 0; i < output_len; i++) {
        output[i] = 'x' + i;
    }

}

void *http_malloc(u32 size)
{
    return wl_zmalloc(size);
}

void http_free(void *mem)
{
    wl_free(mem);
}

static void httpc_debug( void *ctx, int level, const char *file, int line, const char *str )
{
    HTTP_DBG("%s", str);
}

static int httpc_random_func(void *p_rng, unsigned char *output, size_t output_len)
{
    wl_get_random_bytes(output, output_len);
    return 0;
}

static void httpc_base64enc(char *out, const char *in)
{
    const char code[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=" ;
    int i = 0, x = 0, l = 0;

    for (; *in; in++) 
    {
        x = x << 8 | *in;
        for (l += 8; l >= 6; l -= 6) 
        {
            out[i++] = code[(x >> (l - 6)) & 0x3f];
        }
    }
    if (l > 0) 
    {
        x <<= 6 - l;
        out[i++] = code[x & 0x3f];
    }
    for (; i % 4;) 
    {
        out[i++] = '=';
    }
    out[i] = '\0' ;
}

HTTPC_RESULT httpc_conn(httpc_t *client, char *host)
{
    struct addrinfo hints, *addr_list, *cur;
    HTTPC_RESULT ret = HTTPC_OK;
    char port[10] = {0};
    
    memset( &hints, 0, sizeof( hints ) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    snprintf(port, sizeof(port), "%d", client->remote_port) ;
    if ( getaddrinfo( host, port , &hints, &addr_list ) != 0 ) 
    {
        HTTP_DBG("getaddrinfo != 0, return HTTPC_ERR_DNS");
        return HTTPC_ERR_DNS;
    }

    /* Try the sockaddrs until a connection succeeds */
    ret = HTTPC_ERR_DNS;
    for ( cur = addr_list; cur != NULL; cur = cur->ai_next ) 
    {
        client->socket = (int) socket( cur->ai_family, cur->ai_socktype,
                                        cur->ai_protocol );
        if ( client->socket < 0 ) 
        {
            ret = HTTPC_ERR_CONN;
            continue;
        }

        if ( connect( client->socket, cur->ai_addr, (int)cur->ai_addrlen ) == 0 ) 
        {
            ret = HTTPC_OK;
            break;
        }

        close( client->socket );
        ret = HTTPC_ERR_CONN;
    }

    freeaddrinfo( addr_list );

    return ret;
}

HTTPC_RESULT httpc_recv(httpc_t *client, char *buf, int min_len, int max_len, int *p_read_len)
{
    int ret = 0;
    size_t readLen = 0;
while (readLen < max_len) {
        buf[readLen] = '\0';
        if (client->is_http) 
        {
        #if 1
            if (readLen < min_len) 
            {
                ret = recv(client->socket, buf + readLen, min_len - readLen, 0);
                HTTP_DBG("recv [blocking] return:%d", ret);
            } else {
                ret = recv(client->socket, buf + readLen, max_len - readLen, MSG_DONTWAIT);
                HTTP_DBG("recv [not blocking] return:%d", ret);
                //if (ret == -1 && errno == EWOULDBLOCK) 
                if (ret == -1)
                {
                    HTTP_DBG("recv [not blocking] EWOULDBLOCK");
                    break;
                }
            }
        #else
            ret = recv(client->socket, buf + readLen, max_len - readLen, 0);
        #endif
        }
#if CONFIG_NET_HTTPC_OVER_TLS
        else {            
            httpc_tls_t *ssl = (httpc_tls_t *)client->ssl;
        #if 1
            if (readLen < min_len) 
            {                
                mbedtls_ssl_set_bio(&ssl->ssl_ctx, &ssl->net_ctx, mbedtls_net_send, mbedtls_net_recv, NULL);
                ret = mbedtls_ssl_read(&ssl->ssl_ctx, (unsigned char *)buf + readLen, min_len - readLen);
                HTTP_DBG("mbedtls_ssl_read [blocking] return:%d", ret);
            } else {
                mbedtls_ssl_set_bio(&ssl->ssl_ctx, &ssl->net_ctx, mbedtls_net_send, httpc_tls_nonblock_recv, NULL);
                ret = mbedtls_ssl_read(&ssl->ssl_ctx, (unsigned char *)buf + readLen, max_len - readLen);
                HTTP_DBG("mbedtls_ssl_read [not blocking] return:%d", ret);
                //if (ret == -1 && errno == EWOULDBLOCK) 
                if (ret == -1)
                {
                    HTTP_DBG("mbedtls_ssl_read [not blocking] EWOULDBLOCK");
                    break;
                }
            }
        #else         
            mbedtls_ssl_set_bio(&ssl->ssl_ctx, &ssl->net_ctx, mbedtls_net_send, mbedtls_net_recv, NULL);
            ret = mbedtls_ssl_read(&ssl->ssl_ctx, (unsigned char *)buf + readLen, max_len - readLen);
        #endif
        
            if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) 
            {/* read already complete(if call mbedtls_ssl_read again, it will return 0(eof)) */
                break;
            }
        }
#endif

        if (ret > 0) {
            readLen += ret;
        } else if (ret == 0) {
            break;
        } else {
            HTTP_ERR("Connection error (recv returned %d)", ret);
            *p_read_len = readLen;
            return HTTPC_ERR_CONN;
        }
    }

    HTTP_DBG("Read %d bytes", readLen);
    *p_read_len = readLen;
    buf[readLen] = '\0';

    return HTTPC_OK;
}


HTTPC_RESULT httpc_parse_url(const char *url, char *scheme, size_t max_scheme_len, char *host, size_t maxhost_len, int *port, char *path, size_t max_path_len)
{
    char *scheme_ptr = (char *) url;
    char *host_ptr = (char *) strstr(url, "://");
    size_t host_len = 0;
    size_t path_len;
    char *port_ptr;
    char *path_ptr;
    char *fragment_ptr;

    if (host_ptr == NULL) 
    {
        HTTP_WARN("Could not find host");
        return HTTPC_ERR_PARSE; /* URL is invalid */
    }

    if ( max_scheme_len < host_ptr - scheme_ptr + 1 ) 
    { /* including NULL-terminating char */
        HTTP_WARN("Scheme str is too small (%d >= %d)", max_scheme_len, host_ptr - scheme_ptr + 1);
        return HTTPC_ERR_PARSE;
    }
    memcpy(scheme, scheme_ptr, host_ptr - scheme_ptr);
    scheme[host_ptr - scheme_ptr] = '\0';

    host_ptr += 3;

    port_ptr = strchr(host_ptr, ':');
    if ( port_ptr != NULL ) 
    {
        uint16_t tport;
        host_len = port_ptr - host_ptr;
        port_ptr++;
        if ( sscanf(port_ptr, "%hu", &tport) != 1) 
        {
            HTTP_WARN("Could not find port");
            return HTTPC_ERR_PARSE;
        }
        *port = (int)tport;
    } else {
        *port = 0;
    }
    path_ptr = strchr(host_ptr, '/');
    if ( host_len == 0 ) 
    {
        host_len = path_ptr - host_ptr;
    }

    if ( maxhost_len < host_len + 1 ) 
    { /* including NULL-terminating char */
        HTTP_WARN("Host str is too small (%d >= %d)", maxhost_len, host_len + 1);
        return HTTPC_ERR_PARSE;
    }
    memcpy(host, host_ptr, host_len);
    host[host_len] = '\0';

    fragment_ptr = strchr(host_ptr, '#');
    if (fragment_ptr != NULL) 
    {
        path_len = fragment_ptr - path_ptr;
    } else {
        path_len = strlen(path_ptr);
    }

    if ( max_path_len < path_len + 1 ) 
    { /* including NULL-terminating char */
        HTTP_WARN("Path str is too small (%d >= %d)", max_path_len, path_len + 1);
        return HTTPC_ERR_PARSE;
    }
    memcpy(path, path_ptr, path_len);
    path[path_len] = '\0';

    return HTTPC_OK;
}

int httpc_parse_host(char *url, char *host, size_t maxhost_len)
{
    char *host_ptr = (char *) strstr(url, "://");
    size_t host_len = 0;
    char *port_ptr;
    char *path_ptr;

    if (host_ptr == NULL) 
    {
        HTTP_WARN("Could not find host");
        return HTTPC_ERR_PARSE; /* URL is invalid */
    }
    host_ptr += 3;

    port_ptr = strchr(host_ptr, ':');
    if ( port_ptr != NULL ) 
    {
        uint16_t tport;
        host_len = port_ptr - host_ptr;
        port_ptr++;
        if ( sscanf(port_ptr, "%hu", &tport) != 1) 
        {
            HTTP_WARN("Could not find port");
            return HTTPC_ERR_PARSE;
        }
    }

    path_ptr = strchr(host_ptr, '/');
    if ( host_len == 0 ) 
    {
        host_len = path_ptr - host_ptr;
    }

    if ( maxhost_len < host_len + 1 ) 
    { /* including NULL-terminating char */
        HTTP_WARN("Host str is too small (%d >= %d)", maxhost_len, host_len + 1);
        return HTTPC_ERR_PARSE;
    }
    memcpy(host, host_ptr, host_len);
    host[host_len] = '\0';

    return HTTPC_OK;
}

static int httpc_tcp_send_all(int sock_fd, const char *data, int length)
{
    int written_len = 0;

    while (written_len < length) {
        int ret = send(sock_fd, data + written_len, length - written_len, 0);
        if (ret > 0) 
        {
            written_len += ret;
            continue;
        } else if (ret == 0) 
        {
            return written_len;
        } else {
            return -1; /* Connnection error */
        }
    }

    return written_len;
}

static int httpc_get_info(httpc_t *client, char *send_buf, int *send_idx, char *buf, size_t len)   /* 0 on success, err code on failure */
{
    int ret ;
    int cp_len ;
    int idx = *send_idx;

    if (len == 0) 
    {
        len = strlen(buf);
    }

    do {
        if ((HTTPC_SEND_BUF_SIZE - idx) >= len) 
        {
            cp_len = len ;
        } else {
            cp_len = HTTPC_SEND_BUF_SIZE - idx ;
        }

        memcpy(send_buf + idx, buf, cp_len) ;
        idx += cp_len ;
        len -= cp_len ;

        if (idx == HTTPC_SEND_BUF_SIZE) 
        {
            if (client->is_http == false) 
            {
                HTTP_ERR("send buffer overflow");
                return HTTPC_ERR_XX ;
            }
            ret = httpc_tcp_send_all(client->socket, send_buf, HTTPC_SEND_BUF_SIZE) ;
            if (ret) 
            {
                return (ret) ;
            }
        }
    } while (len) ;

    *send_idx = idx;
    return HTTPC_OK ;
}

static int httpc_send_auth(httpc_t *client, char *send_buf, int *send_idx)
{
    char b_auth[(int)((HTTPC_AUTHB_SIZE + 3) * 4 / 3 + 1)] ;
    char base64buff[HTTPC_AUTHB_SIZE + 3] ;

    httpc_get_info(client, send_buf, send_idx, "Authorization: Basic ", 0) ;
    sprintf(base64buff, "%s:%s", client->auth_user, client->auth_password) ;
    HTTP_DBG("bAuth: %s", base64buff) ;
    httpc_base64enc(b_auth, base64buff) ;
    b_auth[strlen(b_auth) + 1] = '\0' ;
    b_auth[strlen(b_auth)] = '\n' ;
    HTTP_DBG("b_auth:%s", b_auth) ;
    httpc_get_info(client, send_buf, send_idx, b_auth, 0) ;
    return HTTPC_OK ;
}


HTTPC_RESULT httpc_send_header(httpc_t *client, const char *url, int method, httpc_data_t *client_data)
{
    char scheme[8] = {0};
    char host[HTTPC_MAX_HOST_LEN] = {0};
    char path[HTTPC_MAX_URL_LEN] = {0};
    int len;
    char send_buf[HTTPC_SEND_BUF_SIZE] = {0};
    char buf[HTTPC_SEND_BUF_SIZE] = {0};
    char *meth = (method == HTTPC_GET) ? "GET" : (method == HTTPC_POST) ? "POST" : (method == HTTPC_PUT) ? "PUT" : (method == HTTPC_DELETE) ? "DELETE" : (method == HTTPC_HEAD) ? "HEAD" : "";
    int ret, port;

    /* First we need to parse the url (http[s]://host[:port][/[path]]) */
    ret = httpc_parse_url(url, scheme, sizeof(scheme), host, sizeof(host), &(port), path, sizeof(path));
    if (ret != HTTPC_OK) 
    {
        HTTP_ERR("httpc_parse_url returned %d", ret);
        return (HTTPC_RESULT)ret;
    }

    /* Send request */
    memset(send_buf, 0, HTTPC_SEND_BUF_SIZE);
    len = 0 ; /* Reset send buffer */

    snprintf(buf, sizeof(buf), "%s %s HTTP/1.1\r\nHost: %s\r\n", meth, path, host); /* Write request */
    ret = httpc_get_info(client, send_buf, &len, buf, strlen(buf));
    if (ret) 
    {
        HTTP_ERR("Could not write request");
        return HTTPC_ERR_CONN;
    }

    /* Send all headers */
    if (client->auth_user) 
    {
        httpc_send_auth(client, send_buf, &len) ; /* send out Basic Auth header */
    }

    /* Add user header information */
    if (client->header) 
    {
        httpc_get_info(client, send_buf, &len, (char *)client->header, strlen(client->header));
    }

    if ( client_data->post_buf != NULL ) 
    {
        snprintf(buf, sizeof(buf), "Content-Length: %d\r\n", client_data->post_buf_len);
        httpc_get_info(client, send_buf, &len, buf, strlen(buf));

        if (client_data->post_content_type != NULL)  
        {
            snprintf(buf, sizeof(buf), "Content-Type: %s\r\n", client_data->post_content_type);
            httpc_get_info(client, send_buf, &len, buf, strlen(buf));
        }
    }

    /* Close headers */
    httpc_get_info(client, send_buf, &len, "\r\n", 0);

    HTTP_DBG("Trying to write %d bytes http header:\n%s", len, send_buf);

#if CONFIG_NET_HTTPC_OVER_TLS
    if (client->is_http == false) 
    {
        httpc_tls_t *ssl = (httpc_tls_t *)client->ssl;
        if (httpc_tls_send_all(&ssl->ssl_ctx, send_buf, len) != len) 
        {
            HTTP_ERR("SSL_write failed");
            return HTTPC_ERR_XX;
        }
        return HTTPC_OK;
    }
#endif

    ret = httpc_tcp_send_all(client->socket, send_buf, len);
    if (ret > 0) 
    {
        HTTP_DBG("Written %d bytes, socket = %d", ret, client->socket);
    } else if ( ret == 0 ) 
    {
        HTTP_WARN("ret == 0,Connection was closed by server");
        return HTTPC_CLOSED; /* Connection was closed by server */
    } else {
        HTTP_ERR("Connection error (send returned %d)", ret);
        return HTTPC_ERR_CONN;
    }

    return HTTPC_OK;
}

HTTPC_RESULT httpc_send_userdata(httpc_t *client, httpc_data_t *client_data)
{
    int ret = 0;

    if (client_data->post_buf && client_data->post_buf_len) 
    {
        HTTP_DBG("client_data->post_buf:%s", client_data->post_buf);
#if CONFIG_NET_HTTPC_OVER_TLS
        if (client->is_http == false) 
        {
            httpc_tls_t *ssl = (httpc_tls_t *)client->ssl;
            if (httpc_tls_send_all(&ssl->ssl_ctx, client_data->post_buf, client_data->post_buf_len) != client_data->post_buf_len) 
            {
                HTTP_ERR("SSL_write failed");
                return HTTPC_ERR_XX;
            }
        } else
#endif
        {
            ret = httpc_tcp_send_all(client->socket, client_data->post_buf, client_data->post_buf_len);
            if (ret > 0) 
            {
                HTTP_DBG("Written %d bytes", ret);
            } else if ( ret == 0 ) 
            {
                HTTP_WARN("ret == 0,Connection was closed by server");
                return HTTPC_CLOSED; /* Connection was closed by server */
            } else {
                HTTP_ERR("Connection error (send returned %d)", ret);
                return HTTPC_ERR_CONN;
            }
        }
    }

    return HTTPC_OK;
}

HTTPC_RESULT httpc_retrieve_content(httpc_t *client, char *data, int len, httpc_data_t *client_data)
{
    int count = 0;
    int templen = 0;
    int crlf_pos = 0;
    int max_len = 0;
    int new_trf_len = 0;
    HTTPC_RESULT ret = HTTPC_OK;
    /* Receive data */
    HTTP_DBG("Receiving data :\n%s", data);
    client_data->is_more = true;

    if (client_data->response_content_len == -1 && client_data->is_chunked == false) 
    {
        while(true)
        {
            if (count + len < client_data->response_buf_len - 1) 
            {
                memcpy(client_data->response_buf + count, data, len);
                count += len;
                client_data->response_buf[count] = '\0';
            } else {
                memcpy(client_data->response_buf + count, data, client_data->response_buf_len - 1 - count);
                client_data->response_buf[client_data->response_buf_len - 1] = '\0';
                return HTTPC_MORE_DATA;
            }

            max_len = MIN(HTTPC_CHUNK_SIZE - 1, client_data->response_buf_len - 1 - count);
            ret = httpc_recv(client, data, 1, max_len, &len);

            /* Receive data */
            HTTP_DBG("data len: %d %d", len, count);

            if (ret == HTTPC_ERR_CONN) 
            {
                HTTP_DBG("ret == HTTPC_ERR_CONN");
                return ret;
            }

            if (len == 0) 
            {/* read no more data */
                HTTP_DBG("no more len == 0");
                client_data->is_more = false;
                return HTTPC_OK;
            }
        }
    }

    while (true) {
        size_t readLen = 0;

        if ( client_data->is_chunked && client_data->retrieve_len <= 0) 
        {
            /* Read chunk header */
            bool foundCrlf;
            int n;
            do {               
                HTTP_DBG("len: %d", len);
                foundCrlf = false;
                crlf_pos = 0;
                data[len] = 0;
                if (len >= 2) 
                {
                    for (; crlf_pos < len - 2; crlf_pos++) 
                    {
                        if ( data[crlf_pos] == '\r' && data[crlf_pos + 1] == '\n' ) 
                        {
                            foundCrlf = true;
                            break;
                        }
                    }
                }
                if (!foundCrlf) 
                { /* Try to read more */
                    if ( len < HTTPC_CHUNK_SIZE ) 
                    {
                        ret = httpc_recv(client, data + len, 0, HTTPC_CHUNK_SIZE - len - 1, &new_trf_len);
                        len += new_trf_len;
                        if (ret == HTTPC_ERR_CONN) 
                        {
                            return ret;
                        } else {
                            continue;
                        }
                    } else {
                        return HTTPC_ERR_XX;
                    }
                }
            } while (!foundCrlf);
            data[crlf_pos] = '\0';
            n = sscanf(data, "%x", &readLen);/* chunk length */
            client_data->retrieve_len = readLen;
            client_data->response_content_len += client_data->retrieve_len;
            if (n != 1) 
            {
                HTTP_ERR("Could not read chunk length");
                return HTTPC_ERR_PRTCL;
            }

            memmove(data, &data[crlf_pos + 2], len - (crlf_pos + 2)); /* Not need to move NULL-terminating char any more */
            len -= (crlf_pos + 2);

            if ( readLen == 0 ) 
            {
                /* Last chunk */
                client_data->is_more = false;
                HTTP_DBG("no more (last chunk)");
                break;
            }
        } else {
            readLen = client_data->retrieve_len;
        }

        HTTP_DBG("Retrieving %d bytes, len:%d", readLen, len);

        do {
            HTTP_DBG("readLen %d, len:%d", readLen, len);
            templen = MIN(len, readLen);
            if (count + templen < client_data->response_buf_len - 1) 
            {
                memcpy(client_data->response_buf + count, data, templen);
                count += templen;
                client_data->response_buf[count] = '\0';
                client_data->retrieve_len -= templen;
            } else {
                memcpy(client_data->response_buf + count, data, client_data->response_buf_len - 1 - count);
                client_data->response_buf[client_data->response_buf_len - 1] = '\0';
                client_data->retrieve_len -= (client_data->response_buf_len - 1 - count);
                return HTTPC_MORE_DATA;
            }

            if ( len >= readLen ) 
            {
                HTTP_DBG("memmove %d %d %d", readLen, len, client_data->retrieve_len);
                memmove(data, &data[readLen], len - readLen); /* chunk case, read between two chunks */
                len -= readLen;
                readLen = 0;
                client_data->retrieve_len = 0;
            } else {
                readLen -= len;
            }
            
            if (readLen) 
            {
                // int max_len = MIN(HTTPC_CHUNK_SIZE - 1, client_data->response_buf_len - 1 - count);                
                max_len = MIN(MIN(HTTPC_CHUNK_SIZE - 1, client_data->response_buf_len - 1 - count), readLen);
                ret = httpc_recv(client, data, 1, max_len, &len);
                if (ret == HTTPC_ERR_CONN) 
                {
                    return ret;
                }
            }
        } while (readLen);

        if ( client_data->is_chunked ) 
        {
            if (len < 2) 
            {
                /* Read missing chars to find end of chunk */
                ret = httpc_recv(client, data + len, 2 - len, HTTPC_CHUNK_SIZE - len - 1, &new_trf_len);
                if (ret == HTTPC_ERR_CONN) 
                {
                    return ret;
                }
                len += new_trf_len;
            }
            if ( (data[0] != '\r') || (data[1] != '\n') ) 
            {
                HTTP_ERR("Format error, %s", data); /* after memmove, the beginning of next chunk */
                return HTTPC_ERR_PRTCL;
            }
            memmove(data, &data[2], len - 2); /* remove the \r\n */
            len -= 2;
        } else {
            HTTP_DBG("no more(content-length)");
            client_data->is_more = false;
            break;
        }

    }

    return HTTPC_OK;
}

HTTPC_RESULT httpc_response_parse(httpc_t *client, char *data, int len, httpc_data_t *client_data)
{
    int crlf_pos, ret = 0, major = 0, minor = 0;
    int header_buf_len = client_data->header_buf_len;
    char *header_buf = client_data->header_buf;

    // reset the header buffer
    memset(header_buf, 0, header_buf_len);
    
    client_data->response_content_len = -1;

    char *crlf_ptr = strstr(data, "\r\n");
    if (crlf_ptr == NULL) 
    {
        HTTP_ERR("\r\n not found");
        return HTTPC_ERR_PRTCL;
    }

    crlf_pos = crlf_ptr - data;
    data[crlf_pos] = '\0';

    /* Parse HTTP response */
    ret = sscanf(data, "HTTP/%d.%d %d %*[^\r\n]", &major, &minor, &(client->response_code));
    if ( 3 != ret ) 
    {
        HTTP_ERR("Not a correct HTTP answer : %s", data);
        return HTTPC_ERR_PRTCL;
    }

    if ( (client->response_code < 200) || (client->response_code >= 400) ) 
    {
        /* Did not return a 2xx code; TODO fetch headers/(&data?) anyway and implement a mean of writing/reading headers */
        HTTP_WARN("Response code %d", client->response_code);
    }

    HTTP_DBG("Reading headers : %s", data);

    memmove(data, &data[crlf_pos + 2], len - (crlf_pos + 2) + 1); /* Be sure to move NULL-terminating char as well */
    len -= (crlf_pos + 2);

    client_data->is_chunked = false;

    /* Now get headers */
    while ( true ) {
        char *colon_ptr, *key_ptr, *value_ptr;
        int key_len, value_len;
        
        crlf_ptr = strstr(data, "\r\n");
        if (crlf_ptr == NULL) 
        {
            if ( len < HTTPC_CHUNK_SIZE - 1 ) 
            {
                int new_trf_len, ret;
                ret = httpc_recv(client, data + len, 1, HTTPC_CHUNK_SIZE - len - 1, &new_trf_len);
                len += new_trf_len;
                data[len] = '\0';
                HTTP_DBG("Read %d chars; In buf: [%s]", new_trf_len, data);
                if (ret == HTTPC_ERR_CONN) 
                {
                    return (HTTPC_RESULT)ret;
                } else {
                    continue;
                }
            } else {
                HTTP_DBG("header len > chunksize");
                return HTTPC_ERR_XX;
            }
        }

        crlf_pos = crlf_ptr - data;        
        if (crlf_pos == 0) 
        { /* End of headers */
            memmove(data, &data[2], len - 2 + 1); /* Be sure to move NULL-terminating char as well */
            len -= 2;
            break;
        }
        
        colon_ptr = strstr(data, ": ");        
        if (colon_ptr) 
        {             
            if (header_buf_len >= crlf_pos + 2) 
            {
                /* copy response header to caller buffer */
                memcpy(header_buf, data, crlf_pos + 2);                                
                header_buf += crlf_pos + 2;
                header_buf_len -= crlf_pos + 2;
            }
            
            key_len = colon_ptr - data;
            value_len = crlf_ptr - colon_ptr - strlen(": ");            
            key_ptr = data;
            value_ptr = colon_ptr + strlen(": ");

            HTTP_DBG("Read header : %.*s: %.*s", key_len, key_ptr, value_len, value_ptr); 
            if (0 == strncasecmp(key_ptr, "Content-Length", key_len)) 
            {
                sscanf(value_ptr, "%d[^\r]", &(client_data->response_content_len));                
                client_data->retrieve_len = client_data->response_content_len;
            } else if (0 == strncasecmp(key_ptr, "Transfer-Encoding", key_len)) 
            {
                if (0 == strncasecmp(value_ptr, "Chunked", value_len)) 
                {
                    client_data->is_chunked = true;
                    client_data->response_content_len = 0;
                    client_data->retrieve_len = 0;
                }
            } 
           
            memmove(data, &data[crlf_pos + 2], len - (crlf_pos + 2) + 1); /* Be sure to move NULL-terminating char as well */
            len -= (crlf_pos + 2);
        } else {
            HTTP_ERR("Could not parse header");
            return HTTPC_ERR_XX;
        }
    }

    return httpc_retrieve_content(client, data, len, client_data);
}

#if CONFIG_NET_HTTPC_OVER_TLS
#if 1
static int httpc_tls_nonblock_recv( void *ctx, unsigned char *buf, size_t len )
{
    int ret;
    int fd = ((mbedtls_net_context *) ctx)->fd;
    if ( fd < 0 ) 
    {
        return ( MBEDTLS_ERR_NET_INVALID_CONTEXT );
    }

    ret = (int) recv( fd, buf, len, MSG_DONTWAIT );

    if ( ret < 0 ) 
    {
#if ( defined(_WIN32) || defined(_WIN32_WCE) ) && !defined(EFIX64) && \
    !defined(EFI32)
        if ( WSAGetLastError() == WSAECONNRESET ) 
        {
            return ( MBEDTLS_ERR_NET_CONN_RESET );
        }
#else
#if 0
        if ( errno == EPIPE || errno == ECONNRESET ) 
        {
            return ( MBEDTLS_ERR_NET_CONN_RESET );
        }

        if ( errno == EINTR ) 
        {
            return ( MBEDTLS_ERR_SSL_WANT_READ );
        }
#endif
#endif
        //if (ret == -1 && errno == EWOULDBLOCK) 
        if (ret == -1)
        {
            return ret;
        }
        return ( MBEDTLS_ERR_NET_RECV_FAILED );
    }

    return ( ret );
}
#endif

static int httpc_tls_send_all(mbedtls_ssl_context *ssl, const char *data, size_t length)
{
    size_t written_len = 0;

    while (written_len < length) {
        int ret = mbedtls_ssl_write(ssl, (unsigned char *)(data + written_len), (length - written_len));
        if (ret > 0) 
        {
            written_len += ret;
            continue;
        } else if (ret == 0) {
            return written_len;
        } else {
            return -1; /* Connnection error */
        }
    }

    return written_len;
}

static void* httpc_calloc_func(size_t nmemb, size_t size)
{
    size_t mem_size;
    void *ptr = NULL;

    mem_size = nmemb * size;
    ptr = http_malloc(mem_size);

    if(ptr)
        memset(ptr, 0, mem_size);

    return ptr;
}

HTTPC_RESULT httpc_tls_conn(httpc_t *client, char *host)
{
    int authmode = MBEDTLS_SSL_VERIFY_NONE;
    //const char *pers = "https";
    int value, ret = 0; 
    uint32_t flags;
    char port[10] = {0};
    httpc_tls_t *ssl;
    
    mbedtls_platform_set_calloc_free(httpc_calloc_func, http_free);
    client->ssl = http_malloc(sizeof(httpc_tls_t));
    if (!client->ssl) 
    {
        HTTP_DBG("Memory malloc error.");
        ret = -1;
        goto exit;
    }
    ssl = (httpc_tls_t *)client->ssl;
    
    if (client->server_cert)
        authmode = MBEDTLS_SSL_VERIFY_REQUIRED;
    
    /*
     * Initialize the RNG and the session data
     */
#if defined(MBEDTLS_DEBUG_C)
    mbedtls_debug_set_threshold(DEBUG_LEVEL);
#endif
    mbedtls_net_init(&ssl->net_ctx);
    mbedtls_ssl_init(&ssl->ssl_ctx);
    mbedtls_ssl_config_init(&ssl->ssl_conf);
    mbedtls_x509_crt_init(&ssl->cacert);
    mbedtls_x509_crt_init(&ssl->clicert);
    mbedtls_pk_init(&ssl->pkey);
#if 0
    mbedtls_ctr_drbg_init(&ssl->ctr_drbg);
    mbedtls_entropy_init(&ssl->entropy);    
    if ((value = mbedtls_ctr_drbg_seed(&ssl->ctr_drbg,
                               mbedtls_entropy_func, 
                               &ssl->entropy,
                               (const unsigned char*)pers,
                               strlen(pers))) != 0) {       
        HTTP_DBG("mbedtls_ctr_drbg_seed() failed, value:-0x%x.", -value);
        ret = -1;
        goto exit;
    }
#endif

    /*
     * Setup stuff
     */
    if ((value = mbedtls_ssl_config_defaults(&ssl->ssl_conf,
                                           MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {        
        HTTP_DBG("mbedtls_ssl_config_defaults() failed, value:-0x%x.", -value);
        ret = -1;
        goto exit;
    }

    //mbedtls_ssl_conf_rng(&ssl->ssl_conf, mbedtls_ctr_drbg_random, &ssl->ctr_drbg);
    mbedtls_ssl_conf_rng(&ssl->ssl_conf, httpc_random_func, NULL);
    mbedtls_ssl_conf_dbg(&ssl->ssl_conf, httpc_debug, NULL);

    /*
    * Load the Client certificate
    */
    if (client->client_cert && client->client_pk) 
    {
        ret = mbedtls_x509_crt_parse(&ssl->clicert, (const unsigned char *)client->client_cert, client->client_cert_len);        
        if (ret < 0) 
        {
            HTTP_DBG("Loading cli_cert failed! mbedtls_x509_crt_parse returned -0x%x.", -ret);
            goto exit;
        }
    
        ret = mbedtls_pk_parse_key(&ssl->pkey, (const unsigned char *)client->client_pk, client->client_pk_len, NULL, 0);                 
        if (ret != 0) 
        {
            HTTP_DBG("failed! mbedtls_pk_parse_key returned -0x%x.", -ret);
            goto exit;
        }
    }
    
    /*
    * Load the trusted CA
    */    
    /* cert_len passed in is gotten from sizeof not strlen */
    if (client->server_cert && ((value = mbedtls_x509_crt_parse(&ssl->cacert,
                                        (const unsigned char *)client->server_cert,
                                        client->server_cert_len)) < 0)) {
        HTTP_DBG("mbedtls_x509_crt_parse() failed, value:-0x%x.", -value);
        ret = -1;
        goto exit;
    }

    /*
     * Start the connection
     */
    snprintf(port, sizeof(port), "%d", client->remote_port) ;

    /* Debug for ssl connection, connect to local TLS server(https server), like 192.168.1.101 */
    if (client->local_server)
    {
        struct sockaddr_in sAddr;

        ssl->net_ctx.fd = (int) socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if( ssl->net_ctx.fd < 0 )
        {
            HTTP_DBG("socket failed.");
            goto exit;
        }

        sAddr.sin_family = AF_INET;
        sAddr.sin_port = htons(client->remote_port);
        sAddr.sin_addr.s_addr = inet_addr(client->local_server);
        if ((ret = connect(ssl->net_ctx.fd, (struct sockaddr*)&sAddr, sizeof(sAddr))) < 0)
        {
            close(ssl->net_ctx.fd);
            HTTP_DBG("connect failed, ret[%d].", ret);
            goto exit;
        }
    }
    else
    {
        if ((ret = mbedtls_net_connect(&ssl->net_ctx, host, port, MBEDTLS_NET_PROTO_TCP)) != 0) 
        {
            HTTP_DBG("failed! mbedtls_net_connect returned %d, port:%s.", ret, port);
            goto exit;
        }   
    }
    
    // TODO: add customerization encryption algorithm
#if 0
    memcpy(&ssl->profile, ssl->ssl_conf.cert_profile, sizeof(mbedtls_x509_crt_profile));    
    ssl->profile.allowed_mds = ssl->profile.allowed_mds | MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_MD5);
    mbedtls_ssl_conf_cert_profile(&ssl->ssl_conf, &ssl->profile);
#endif
    
    mbedtls_ssl_conf_authmode(&ssl->ssl_conf, authmode);
    mbedtls_ssl_conf_ca_chain(&ssl->ssl_conf, &ssl->cacert, NULL);

    if (client->client_cert && (ret = mbedtls_ssl_conf_own_cert(&ssl->ssl_conf, &ssl->clicert, &ssl->pkey)) != 0) 
    {
        HTTP_DBG(" failed! mbedtls_ssl_conf_own_cert returned %d.", ret );
        goto exit;
    }

    if ((value = mbedtls_ssl_setup(&ssl->ssl_ctx, &ssl->ssl_conf)) != 0) 
    {
        HTTP_DBG("mbedtls_ssl_setup() failed, value:-0x%x.", -value);
        ret = -1;
        goto exit;
    }   

    mbedtls_ssl_set_bio(&ssl->ssl_ctx, &ssl->net_ctx, mbedtls_net_send, mbedtls_net_recv, NULL);    
    
    /*
    * Handshake
    */
    while ((ret = mbedtls_ssl_handshake(&ssl->ssl_ctx)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) 
        {      
            HTTP_DBG("mbedtls_ssl_handshake() failed, ret:-0x%x.", -ret);
            ret = -1;
            goto exit;
        }
    }
        
    /*
     * Verify the server certificate
     */
    /* In real life, we would have used MBEDTLS_SSL_VERIFY_REQUIRED so that the
        * handshake would not succeed if the peer's cert is bad.  Even if we used
        * MBEDTLS_SSL_VERIFY_OPTIONAL, we would bail out here if ret != 0 */
    if ((flags = mbedtls_ssl_get_verify_result(&ssl->ssl_ctx)) != 0) 
    {
        char vrfy_buf[512];
        HTTP_DBG("svr_cert varification failed.");
        mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", flags);
        HTTP_DBG("%s", vrfy_buf);
    }
    else
        HTTP_DBG("svr_cert varification ok."); 
    
exit:
    HTTP_DBG("ret=%d.", ret);
    if (ret != 0)
    {
        ret = HTTPC_ERR_CONN;
    }
    return (HTTPC_RESULT)ret;
}

int httpc_tls_close(httpc_t *client)
{
    httpc_tls_t *ssl = (httpc_tls_t *)client->ssl;
    client->client_cert = NULL;
    client->server_cert = NULL;
    client->client_pk = NULL;

    if (!ssl)
        return -1;
    
    mbedtls_ssl_close_notify(&ssl->ssl_ctx);
    mbedtls_net_free(&ssl->net_ctx);
    mbedtls_x509_crt_free(&ssl->cacert);
    mbedtls_x509_crt_free(&ssl->clicert);
    mbedtls_pk_free(&ssl->pkey);
    mbedtls_ssl_free(&ssl->ssl_ctx);    
    mbedtls_ssl_config_free(&ssl->ssl_conf);
#if 0
    mbedtls_ctr_drbg_free(&ssl->ctr_drbg);
    mbedtls_entropy_free(&ssl->entropy);               
#endif
    
    http_free(ssl);       
    return 0;
}
#endif

