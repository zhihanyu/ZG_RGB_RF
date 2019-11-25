#ifndef HAL_LOGUART_H
#define HAL_LOGUART_H




typedef struct loguart_
{
    hal_cb_t recv_hdl;
    char     recv_byte;
    char    *rxbuf;
    u8       rxlen;
    u8       busy;    
    u8       user_define;
    void *   finsh_sema;
	void *   object;
}loguart_t;




void loguart_init(loguart_t *loguart);
void loguart_deinit(loguart_t *loguart);








#endif
