#ifndef RUN_MODE_H
#define RUN_MODE_H


#include "s907x.h"



#define	MODE_TEST					0x00
#define	MODE_MP						0X01
#define	MODE_NORMAL					0xff

#define	TEST_MODE_HEAD_LEN			        4
#define	MP_MODE_HEAD_LEN			        2

#define	MODE_TEST_ASSERT(cmd)		               (cmd[0] == 't' && \
                                                       cmd[1] == 'e' && \
                                                       cmd[2] == 's' && \
                                                       cmd[3] == 't' )

#define	MODE_MP_ASSERT(cmd)			       (cmd[0] == 'm' && \
							cmd[1] == 'p' )


#define	TEST_START_ASSERT(cmd)		                (cmd[TEST_MODE_HEAD_LEN + 1] == 's' && \
                                                        cmd[TEST_MODE_HEAD_LEN + 2] == 't' && \
                                                        cmd[TEST_MODE_HEAD_LEN + 3] == 'a' && \
                                                        cmd[TEST_MODE_HEAD_LEN + 4] == 'r' && \
                                                        cmd[TEST_MODE_HEAD_LEN + 5] == 't' )

#define	TEST_STOP_ASSERT(cmd)		                (cmd[TEST_MODE_HEAD_LEN + 1] == 's' && \
                                                        cmd[TEST_MODE_HEAD_LEN + 2] == 't' && \
                                                        cmd[TEST_MODE_HEAD_LEN + 3] == 'o' && \
                                                        cmd[TEST_MODE_HEAD_LEN + 4] == 'p' )

#define	MP_START_ASSERT(cmd)		                (cmd[MP_MODE_HEAD_LEN + 1] == 's' && \
                                                        cmd[MP_MODE_HEAD_LEN + 2] == 't' && \
                                                        cmd[MP_MODE_HEAD_LEN + 3] == 'a' && \
                                                        cmd[MP_MODE_HEAD_LEN + 4] == 'r' && \
                                                        cmd[MP_MODE_HEAD_LEN + 5] == 't' )

#define	MP_STOP_ASSERT(cmd)			        (cmd[MP_MODE_HEAD_LEN + 1] == 's' && \
                                                        cmd[MP_MODE_HEAD_LEN + 2] == 't' && \
                                                        cmd[MP_MODE_HEAD_LEN + 3] == 'o' && \
                                                        cmd[MP_MODE_HEAD_LEN + 4] == 'p' )

u8 get_s907_run_mode(void);
void set_s907_run_mode(u8 set);
int run_mode_check(void);
int is_mode_change(void *context, int m);
int run_mode_change_hdl(void *context);






















#endif


