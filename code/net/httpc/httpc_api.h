
#ifndef __HTTPC_API_H__
#define __HTTPC_API_H__



typedef enum {
    HTTPC_GET,
    HTTPC_POST,
    HTTPC_PUT,
    HTTPC_DELETE,
    HTTPC_HEAD
} HTTPC_REQUEST_TYPE;

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

void *http_malloc(u32 size);

void http_free(void *mem);
/*
 brief            Alloc memory for http client struct and http client data struct.
 param[out]       client: a pointer to http client struct.
 param[out]       client_data: a pointer to http client data struct.
 return           Refer to HTTPC_RESULT.
*/
void *httpc_client_new();

/*
 brief            Release memory for http client struct and http client data struct.
 param[out]       client: a pointer to http client struct.
 param[out]       client_data: a pointer to http client data struct.
 return           None.
*/
void httpc_client_free(void *client);

/*
 brief            Set the user/password for http connection.
 param[in, out]   client: a pointer to the http client struct.
 param[in]        user: user name string.
 param[in]        password: password string.
 return           Refer to HTTPC_RESULT.
*/
HTTPC_RESULT httpc_set_basic_auth(void *client, const char *user, const char *password);

/*
 brief            Set the client's certificate, private key and server's certificate for https connection.
 param[in, out]   client: a pointer to the http client struct.
 param[in]        client_cert: client's certificate.
 param[in]        client_key: client's private key.
 param[in]        server_cert: server's certificate.
 return           Refer to HTTPC_RESULT.
*/
HTTPC_RESULT httpc_set_tls_certs(void *client, const char *client_cert, const char *client_key, const char *server_cert);

/*
 brief            Set the buffer pointer in which to restore response header data.
 param[in, out]   data: a pointer to http client data struct. Instance to collect the data returned by the request.
 param[in]        rsp_buf: a pointer to buffer for header data in http response.
 param[in]        len: buffer len for http response body data.
 return           Refer to HTTPC_RESULT.
*/
HTTPC_RESULT httpc_set_header_buf(void *client, char *hdr_buf, int len);

/*
 brief            Set the buffer pointer in which to restore response body data.
 param[in, out]   data: a pointer to http client data struct. Instance to collect the data returned by the request.
 param[in]        rsp_buf: a pointer to buffer for body data in http response.
 param[in]        len: buffer len for http response body data.
 return           Refer to HTTPC_RESULT.
*/
HTTPC_RESULT httpc_set_response_buf(void *client, char *rsp_buf, int len);
HTTPC_RESULT httpc_get_response_buf(const void *vclient, char **rsp_buf);

/*
 brief            Set the buffer pointer in which to restore request data.
 param[in, out]   data: a pointer to http client data struct. Instance to collect the data returned by the request.
 param[in]        req_buf: a pointer to buffer for http request data.
 param[in]        len: buffer len for http request data.
 return           Refer to HTTPC_RESULT.
*/
HTTPC_RESULT httpc_set_request_buf(void *client, const char *req_buf, int len);

/*
 brief            This function sets a custom header.
 param[in, out]   client: a pointer to the http client struct.
 param[in]        header is a custom header string.
 return           Refer to HTTPC_RESULT.
 */
HTTPC_RESULT httpc_set_custom_header(void *client, char *header);

HTTPC_RESULT httpc_set_local_tls_server(void *client, char *ipaddr);

/*
 brief            This function gets the HTTP response code assigned to the last request.
 param[in, out]   client: a pointer to the http client struct.
 return           The HTTP response code of the last request.
 */
int httpc_get_response_code(void *client);

int httpc_get_response_len(const void *vclient, const HTTPC_RESPONSE_LEN_TYPE type, int *rsp_len);

/*
 brief            This function get specified response header value.
 param[in]        header_buf is the response header buffer.
 param[in]        name is the specified http response header name.
 param[in/out]    val_pos is the position of header value in #header_buf.
 param[in/out]    val_len is header value length.
 return           0, if value is got. Others, if errors occurred.
 */
HTTPC_RESULT httpc_get_header_value(const char *header_buf, const char *name, int *val_pos, int *val_len);

/*
 brief            Executes a GET request on a given URL.
 param[in]        url: the URL to run http request.
 param[in, out]   client: a pointer to the http client struct.
 param[in, out]   data: a pointer to http response data. Instance to collect the data returned by the request.
 return           Refer to HTTPC_RESULT.
 */
HTTPC_RESULT httpc_get(void *client, const char *url);

/*
 brief            Executes a POST request on a given URL.
 param[in]        url: the URL to run http request.
 param[in, out]   client: a pointer to the http client struct.
 param[in, out]   data: a pointer to http client data struct. Instance to collect the data returned by the request. It also contains the data to be posted.
 return           Refer to HTTPC_RESULT.
 */
HTTPC_RESULT httpc_post(void *client, const char *url);

/*
 brief            Executes a PUT request on a given URL. It blocks until completion.
 param[in]        url: the URL to run http request.
 param[in, out]   client: a pointer to the http client struct.
 param[in, out]   data: a pointer to http client data struct. Instance to collect the data returned by the request. It also contains the data to be put.
 return           Refer to HTTPC_RESULT.
 */
HTTPC_RESULT httpc_put(void *client, const char *url);

/*
 brief            Executes a DELETE request on a given URL. It blocks until completion.
 param[in]        url: the URL to run http request.
 param[in, out]   client: a pointer to the http client struct.
 param[in, out]   data: a pointer to http client data struct. Instance to collect the data returned by the request.
 return           Refer to HTTPC_RESULT.
 */
HTTPC_RESULT httpc_delete(void *client, const char *url);

/*
 brief            Establishes an HTTP connection with the remote server by given URL.
 param[in]        url: the URL to run http request.
 param[in, out]   client: a pointer to the http client struct.
 return           Refer to HTTPC_RESULT.
 */
HTTPC_RESULT httpc_connect(void *client, const char *url);

/*
 brief            This function sends an HTTP(GET or POST) request to the given URL.
 param[in]        url: the URL to run http request.
 param[in]        method is HTTPC_REQUEST_TYPE.
 param[in, out]   client: a pointer to the http client struct.
 param[in]        data: a pointer to http client data struct. Instance to collect the data to be posted.
 return           Refer to HTTPC_RESULT.
 */
HTTPC_RESULT httpc_send_request(const char *url, int method, void *client);

/*
 brief            This function receives the response from a server for the last request.
 param[in, out]   client: a pointer to the http client struct.
 param[out]       data: a pointer to http client data struct. Instance to collect the data returned by the request.
 return           Refer to HTTPC_RESULT.
 */
HTTPC_RESULT httpc_recv_response(void *client);

/*
 brief            This function closes the HTTP connection.
 param[in, out]   client: a pointer to the http client struct.
 */
void httpc_close(void *client);


#endif /* __HTTPC_API_H__ */


