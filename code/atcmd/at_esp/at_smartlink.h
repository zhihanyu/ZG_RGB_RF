#ifndef ESP_SMARTLINK_H
#define ESP_SMARTLINK_H


#define SMART_LINK_NO_TIME_LIMIT		0xFFFFFFFF

typedef enum {
    SL_STATUS_WAITTING = 0,            
    SL_STATUS_FINDING_CHANNEL,      
    SL_STATUS_GETTING_SSID_PSWD,   
    SL_STATUS_SUCCESS,               
    SL_STATUS_TIMEOUT,
    SL_STATUS_HIDDEN_ERR,
    SL_STATUS_OFFSET_ERR
}sl_status;

typedef struct sc_got_data_
{
	char *ssid;
	char *password;
	s907x_security_e security;
	u32 phone_ip;
}sc_got_data_t;


typedef void (*sl_callback_t)(sl_status status, void *pdata);

void smart_link_start(sl_callback_t cb,int timeout_sec);
void smart_link_stop(void);
#endif
