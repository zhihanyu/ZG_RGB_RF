
#include "s907x.h"
#include "httpc_core.h"


void *http_malloc(u32 size);
void  http_free(void *mem);



void *httpc_client_new()
{
    httpc_t *client = NULL;
    httpc_data_t *client_data = NULL;

    client = (httpc_t*)http_malloc(sizeof(httpc_t));
    client_data = (httpc_data_t*)http_malloc(sizeof(httpc_data_t));

    if (!client || !client_data)
    {
        HTTP_DBG("Memory malloc error.");
    }

    memset(client, 0, sizeof(httpc_t));
    memset(client_data, 0, sizeof(httpc_data_t));

    client->client_data  = client_data;

    return (void *)client;
}

void httpc_client_free(void *vclient)
{
    httpc_t *client = (httpc_t *)vclient;

    http_free(client->client_data);
    http_free(client);       
}

HTTPC_RESULT httpc_set_basic_auth(void *vclient, char *user, char *password)
{
    httpc_t *client = (httpc_t *)vclient;

    if ((strlen(user) + strlen(password)) >= HTTPC_AUTHB_SIZE) 
    {
        return HTTPC_ERR_XX;
    }

    if (!client)
    {
        return HTTPC_ERR_XX;
    }

    client->auth_user = user;
    client->auth_password = password;

    return HTTPC_OK ;
}

HTTPC_RESULT httpc_set_tls_certs(void *vclient, const char *client_cert, const char *client_pk, const char *server_cert)
{
#if CONFIG_NET_HTTPC_OVER_TLS
    httpc_t *client = (httpc_t *)vclient;

    if (!client)
    {
        return HTTPC_ERR_XX;
    }

    client->client_cert = client_cert;
    client->client_pk = client_pk;
    client->server_cert = server_cert;

    client->client_cert_len = strlen(client_cert) + 1;
    client->client_pk_len = strlen(client_pk) + 1;
    client->server_cert_len = strlen(server_cert) + 1;
    HTTP_DBG("==>Debug: client_cert_len: %d, pk_len: %d, server_cert_len: %d\n",
            client->client_cert_len, client->client_pk_len, client->server_cert_len);
#endif

    return HTTPC_OK;
}

HTTPC_RESULT httpc_set_header_buf(void *vclient, char *hdr_buf, int len)
{
    httpc_t *client = (httpc_t *)vclient;
    httpc_data_t *client_data = NULL;

    if (!client)
    {
        return HTTPC_ERR_XX;
    }

    client_data = client->client_data;

    client_data->header_buf = hdr_buf;
    client_data->header_buf_len = len;

    return HTTPC_OK;
} 

HTTPC_RESULT httpc_set_response_buf(void *vclient, char *rsp_buf, int len)
{
    httpc_t *client = (httpc_t *)vclient;
    httpc_data_t *client_data = NULL;

    if (!client)
    {
        return HTTPC_ERR_XX;
    }

    client_data = client->client_data;

    client_data->response_buf = rsp_buf;
    client_data->response_buf_len = len;

    return HTTPC_OK;
}

HTTPC_RESULT httpc_get_response_buf(const void *vclient, char **rsp_buf)
{
    httpc_t *client = (httpc_t *)vclient;

    if (!client)
    {
        return HTTPC_ERR_XX;
    }

    *rsp_buf = client->client_data->response_buf;
    return HTTPC_OK;
}


HTTPC_RESULT httpc_set_request_buf(void *vclient, const char *req_buf, int len)
{
    httpc_t *client = (httpc_t *)vclient;
    httpc_data_t *client_data = NULL;

    if (!client)
    {
        return HTTPC_ERR_XX;
    }

    client_data = client->client_data;

    client_data->post_buf = req_buf;
    client_data->post_buf_len = len;

    return HTTPC_OK;
}

HTTPC_RESULT httpc_set_custom_header(void *vclient, char *header)
{
    httpc_t *client = (httpc_t *)vclient;

    if (!client)
    {
        return HTTPC_ERR_XX;
    }

    client->header = header;
    return HTTPC_OK;
}

HTTPC_RESULT httpc_set_local_tls_server(void *vclient, char *ipaddr)
{
    httpc_t *client = (httpc_t *)vclient;

    if (!client)
    {
        return HTTPC_ERR_XX;
    }
#if CONFIG_NET_HTTPC_OVER_TLS
    client->local_server = ipaddr;
#endif
    return HTTPC_OK;
}

int httpc_get_response_code(void *vclient)
{
    httpc_t *client = (httpc_t *)vclient;

    if (!client)
    {
        return HTTPC_ERR_XX;
    }

    return client->response_code;
}

int httpc_get_response_len(const void *vclient, const HTTPC_RESPONSE_LEN_TYPE type, int *rsp_len)
{
    int response_len = 0;
    httpc_t *client = (httpc_t *)vclient;
    if (!client)
    {
        return HTTPC_ERR_XX;
    }

    switch (type) {
        case HTTPC_RSP_CONTENT_LEN:
            response_len = client->client_data->response_content_len;
            break;
        
        case HTTPC_RETRIEVE_LEN:
            response_len = client->client_data->retrieve_len;
            break;
        
        case HTTPC_POST_BUFF_LEN:
            response_len = client->client_data->post_buf_len;
            break;
        
        case HTTPC_RSP_BUFF_LEN:
            response_len = client->client_data->response_buf_len;
            break;
        
        case HTTPC_HEADER_BUFF_LEN:
            response_len = client->client_data->header_buf_len;
            break;        
    }

    *rsp_len = response_len;
    return HTTPC_OK;
}

HTTPC_RESULT httpc_get_header_value(const char *header_buf, const char *name, int *val_pos, int *val_len)
{
    const char *data = header_buf;
    const char *crlf_ptr, *colon_ptr, *key_ptr, *value_ptr;
    int key_len, value_len;

    if (header_buf == NULL || name == NULL || val_pos == NULL  || val_len == NULL )
        return HTTPC_ERR_XX;
    
    while (true) {
        crlf_ptr = strstr(data, "\r\n");
        colon_ptr = strstr(data, ": ");        
        if (colon_ptr) 
        {                         
            key_len = colon_ptr - data;
            value_len = crlf_ptr - colon_ptr - strlen(": ");            
            key_ptr = data;
            value_ptr = colon_ptr + strlen(": ");
            
            HTTP_DBG("Response header: %.*s: %.*s", key_len, key_ptr, value_len, value_ptr);
            if (0 == strncasecmp(key_ptr, name, key_len)) 
            {
                *val_pos = value_ptr - header_buf;
                *val_len = value_len;
                return HTTPC_OK;
            } else {                 
                data = crlf_ptr + 2;
                continue;
            }
        } else 
            return HTTPC_ERR_XX;
    }
}



HTTPC_RESULT httpc_connect(const char *url, void *vclient)
{
    HTTPC_RESULT ret = HTTPC_ERR_CONN;
    char host[HTTPC_MAX_HOST_LEN] = {0};
    char scheme[8] = {0};
    char path[HTTPC_MAX_URL_LEN] = {0};

    httpc_t *client = (httpc_t *)vclient;

    if (!client)
    {
        return HTTPC_ERR_XX;
    }
    
    /* First we need to parse the url (http[s]://host[:port][/[path]]) */
    ret = httpc_parse_url(url, scheme, sizeof(scheme), host, sizeof(host), &(client->remote_port), path, sizeof(path));
    if (ret != HTTPC_OK) 
    {
        HTTP_ERR("httpc_parse_url returned %d", ret);
        return ret;
    }

    // http or https
    if (strcmp(scheme, "https") == 0) 
        client->is_http = false;
    else if (strcmp(scheme, "http") == 0)
        client->is_http = true;

    // default http 80 port, https 443 port
    if (client->remote_port == 0) 
    {
        if (client->is_http) 
        { 
            client->remote_port = HTTP_PORT;
        } else 
        {
            client->remote_port = HTTPS_PORT;
        }
    }
        
    HTTP_DBG("http?:%d, port:%d, host:%s", client->is_http, client->remote_port, host);

    client->socket = -1;
    if (client->is_http) 
        ret = httpc_conn(client, host);
#if CONFIG_NET_HTTPC_OVER_TLS
    else {
        ret = httpc_tls_conn(client, host);
        if (HTTPC_OK == ret) 
        {
            httpc_tls_t *ssl = (httpc_tls_t *)client->ssl;
            client->socket = ssl->net_ctx.fd;
        }
    }
#endif

    HTTP_DBG("httpc_connect() result:%d, client:%p", ret, client);
    return ret;
}

HTTPC_RESULT httpc_send_request(const char *url, int method, void *vclient)
{
    HTTPC_RESULT ret = HTTPC_ERR_CONN;
    httpc_t *client = (httpc_t *)vclient;
    httpc_data_t *client_data = NULL;

    if (!client)
    {
        return HTTPC_ERR_XX;
    }

    client_data = client->client_data;
    if (client->socket < 0) 
    {
        return ret;
    }

    ret = httpc_send_header(client, url, method, client_data);
    if (ret != HTTPC_OK) 
    {
        return ret;
    }

    if (method == HTTPC_POST || method == HTTPC_PUT) 
    {
        ret = httpc_send_userdata(client, client_data);
    }
   
    HTTP_DBG("httpc_send_request() result:%d, client:%p", ret, client);
    return ret;
}

HTTPC_RESULT httpc_recv_response(httpc_t *vclient)
{
    int reclen = 0;
    HTTPC_RESULT ret = HTTPC_ERR_CONN;
    // TODO: header format:  name + value must not bigger than HTTPC_CHUNK_SIZE.
    char buf[HTTPC_CHUNK_SIZE] = {0}; // char buf[HTTPC_CHUNK_SIZE*2] = {0};

    httpc_t *client = (httpc_t *)vclient;
    httpc_data_t *client_data = NULL;

    if (!client)
    {
        return HTTPC_ERR_XX;
    }

    if (client->socket < 0) 
    {
        return (HTTPC_RESULT)ret;
    }

    client_data = client->client_data;

    if (client_data->is_more) 
    {
        client_data->response_buf[0] = '\0';
        ret = httpc_retrieve_content(client, buf, reclen, client_data);
    } else {
        ret = httpc_recv(client, buf, 1, HTTPC_CHUNK_SIZE - 1, &reclen);
        if (ret != HTTPC_OK) 
        {
            return ret;
        }

        buf[reclen] = '\0';

        if (reclen) 
        {
            HTTP_DBG("reclen:%d, buf:\n%s", reclen, buf);
            ret = httpc_response_parse(client, buf, reclen, client_data);
        }
    }

    HTTP_DBG("httpc_recv_response() result:%d, client:%p", ret, client);
    return ret;
}

void httpc_close(void *vclient)
{
    httpc_t *client = (httpc_t *)vclient;

    if (!client)
    {
        return;
    }

    if (client->is_http) 
    {
        if (client->socket >= 0)
            close(client->socket);
    }
#if CONFIG_NET_HTTPC_OVER_TLS
    else 
        httpc_tls_close(client);
#endif

    client->socket = -1;
    HTTP_DBG("httpc_close() client:%p", client);
}





static HTTPC_RESULT httpc_common(void *client, const char *url, int method)
{
    HTTPC_RESULT ret = HTTPC_ERR_CONN;

    if (!client)
    {
        return HTTPC_ERR_XX;
    }

    ret = httpc_connect(url, client);

    if (ret == HTTPC_OK) 
    {
        ret = httpc_send_request(url, method, client);

        if (ret == HTTPC_OK) 
        {
            ret = httpc_recv_response(client);
        }
    }

    httpc_close(client);

    if (ret == HTTPC_MORE_DATA)
    {
        ret = HTTPC_OK;
    }

    return ret;
}

HTTPC_RESULT httpc_get(void *client, const char *url)
{
    return httpc_common(client, url, HTTPC_GET);
}

HTTPC_RESULT httpc_post(void *client, const char *url)
{
    return httpc_common(client, url, HTTPC_POST);
}

HTTPC_RESULT httpc_put(void *client, const char *url)
{
    return httpc_common(client, url, HTTPC_PUT);
}

HTTPC_RESULT httpc_delete(void *client, const char *url)
{
    return httpc_common(client, url, HTTPC_DELETE);
}

