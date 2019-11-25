#include "s907x.h"
#include "httpc_test.h"
#include "httpc_api.h"



#define HTTP_URL_GET    "http://www.iciba.com/"
#define HTTPS_URL_GET   "https://www.baidu.com/"
#define BUF_SIZE        (1024 * 1)

#if CONFIG_NET_HTTPC_OVER_TLS
#include <mbedtls/certs.h>

#define TLS_MBED_CRT                                                    \
"-----BEGIN CERTIFICATE-----\r\n"                                       \
"MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG\r\n"  \
"A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\r\n"  \
"b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\r\n"  \
"MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\r\n"  \
"YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\r\n"  \
"aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\r\n"  \
"jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\r\n"  \
"xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\r\n"  \
"1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\r\n"  \
"snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\r\n"  \
"U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\r\n"  \
"9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E\r\n"  \
"BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B\r\n"  \
"AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz\r\n"  \
"yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE\r\n"  \
"38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP\r\n"  \
"AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad\r\n"  \
"DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME\r\n"  \
"HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==\r\n"                              \
"-----END CERTIFICATE-----\r\n"

static const char mbedtls_crt[] = TLS_MBED_CRT;
#endif


static void httpc_thread(void *param)
{
    char *url_get = HTTP_URL_GET;    
    char *url_post = HTTPS_URL_GET;
    void *client = NULL;
    char *rsp_buf = NULL, *header = NULL;
    HTTPC_RESULT ret = HTTPC_OK;
    int val_pos, val_len;
    


    HTTP_DBG("example_httpc_thread()");

    rsp_buf = http_malloc(BUF_SIZE);    
    header = http_malloc(BUF_SIZE);
    if (rsp_buf == NULL || header == NULL) {
        HTTP_DBG("memory malloc failed.");
        return;
    }

    client = httpc_client_new();
    if (!client) {
        HTTP_DBG("New client failed.");
        return;
    }

    // Http "get"
    memset(header, 0, BUF_SIZE);
    memset(rsp_buf, 0, BUF_SIZE);
    httpc_set_header_buf(client, header, BUF_SIZE);
    httpc_set_response_buf(client, rsp_buf, BUF_SIZE);
    ret = httpc_get(client, url_get);
    if (ret != HTTPC_OK)
        goto fail; 
    HTTP_DBG("received data: \n%s", rsp_buf);

    // get response header
    if(0 == httpc_get_header_value(header, "Content-length", &val_pos, &val_len))
        HTTP_DBG("In received response\nContent-length: %.*s", val_len, header + val_pos);
    

    // Https "get"
    memset(header, 0, BUF_SIZE);
    memset(rsp_buf, 0, BUF_SIZE);
    httpc_set_header_buf(client, header, BUF_SIZE);
    httpc_set_response_buf(client, rsp_buf, BUF_SIZE);

#if CONFIG_NET_HTTPC_OVER_TLS
    httpc_set_tls_certs(client, NULL, NULL, NULL);

    HTTP_DBG("server_cert_len(mbedtls_crt): %d", sizeof(mbedtls_crt));
    httpc_set_tls_certs(client, NULL, NULL, mbedtls_crt);
#endif

    ret = httpc_get(client, url_post);  
    if (ret != HTTPC_OK)
        goto fail;        
    HTTP_DBG("received data: %s", rsp_buf); 

    // get response header
    if(0 == httpc_get_header_value(header, "Content-length", &val_pos, &val_len))
        HTTP_DBG("In received response\nContent-length: %.*s", val_len, header + val_pos);

fail:
    http_free(rsp_buf);
    http_free(header);

    // Print final log
    if (ret == HTTPC_OK)    
        HTTP_DBG("example httpc success.");
    else        
        HTTP_DBG("httpc_get fail, reason:%d.", ret);

	wl_destory_thread(NULL);
    httpc_client_free(client);

 
}


void httpc_test(hal_test_t *test)
{
	if(wl_create_thread(((const char*)"httpc_thread"), 2048, HTTP_CLIENT_PRIO, httpc_thread,  NULL) == NULL)
		printf("\n\r%s xTaskCreate(example_httpc_thread) failed", __FUNCTION__);
}

