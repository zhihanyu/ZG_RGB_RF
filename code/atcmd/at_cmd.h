#ifndef AT_CMD_H
#define AT_CMD_H

#include "at_common.h"

#define AT_HEAD_ASSERT(cmd)  		(cmd[0] == 'A' && \
							 		 cmd[1] == 'T' && \
							 		 cmd[2] == '+') 	

#define AT_MODE_R					0
#define AT_MODE_W					1
#define AT_MODE_ACT                 2    

#define AT_BUF_MAXLEN               AT_NET_MAX_RECV_SEND_BUFF_SIZE
#define AT_HEAD_LEN         		(3)
#define AT_BODY_LEN         		(32)
#define AT_VALUE_LEN			    (AT_BUF_MAXLEN - AT_BODY_LEN - 3)
		
#define AT_READ_TAG					'?'
#define AT_WRITE_TAG				'='
#define AT_TAIL_TAG					'\0'
#define AT_STR_TAG                  '"'
#define AT_ACT_TAG                  AT_TAIL_TAG
#define AT_OK  						"\r\nOK\r\n"
#define AT_ERROR                    "\r\nERROR\r\n"
#define AT_FAIL                    "\r\nFAIL\r\n"
#define AT_BUSY                     "\r\nBUSY\r\n"
#define AT_SET_MAX_ARGC 			16

#define AT_BODY(cmd)				(&cmd[AT_HEAD_LEN])
#define AT_VALUE(cmd, bodylen)		(&cmd[AT_HEAD_LEN + bodylen + 1])

#define AT_RET_OK                   0
#define AT_RET_ERR                  -1

#define AT_TASK_SEMA_TIMEOUT		20

#define AT_SET_BIT(val, bit)        ((val) |= (bit))
#define AT_CLR_BIT(val, bit)        ((val) &= ~(bit))

typedef enum {
    AT_TASK_OP_NORMAL_CMD = 0,
    AT_TASK_OP_RECV_UART_DATA,
    AT_TASK_OP_SEND_DATA_BLOCK,
    AT_TASK_OP_BUTT
}AT_TASK_OPERATE_MODE;
  

#define at_rsp                      printf


typedef void (*at_func) (void *context);
typedef void (*at_func_cb)(void *data, int len, void *arg);

typedef struct at_cmd_
{
	char *body;
	char *set;
	u16   hash;
	u16   setsize;
    int   mode;
}at_cmd_t;

typedef struct at_item_
{	
	at_cmd_t  cmd;
	at_func   func;
	_list 	  node;
}at_item_t;

typedef struct at_hdl_
{	
	sema_t sema;
	int   lock;
	int   op_mode;	
	char  rxbuf[AT_BUF_MAXLEN];
	_list tbl;
	int   exit;
}at_hdl_t;

void at_cmd_set_passth_mode(u32 enable, void *object, int is_isr);
int at_cmd_get_operate_mode(void);
void *at_cmd_get_passth_conn(void);
void at_cmd_set_operate_mode(int op_mode);
int at_parse_param_str(char **argv);
int at_parse_set(char *set, char **argv, char tag);
void at_add_cmd(at_item_t *tbl, int len);
void at_remove_cmd(at_item_t *tbl, int len);
int at_hdl_int(void);
int at_hdl_deint(void);
int at_get_param(char **value, char *val);

#endif

