#ifndef UTILS_H
#define UTILS_H





#define RUN_MODE_DEFAULT                                (0xFFFFFFFF)


//default
#define	RUN_MODE_MP_MASK						        (RUN_MODE_DEFAULT)
#define	RUN_MODE_TEST_MASK					            (0x12345679)
#define	RUN_MODE_NORMAL_MASK					        (0x12345678)
        
typedef enum
{
    s907x_mode_test,
    s907x_mode_mp,  
    s907x_mode_normal,
    s907x_mode_max
}run_mode_e;


#define MP_START_CMD                                     "mp start"   //switch to mp
#define MP_STOP_CMD                                      "mp stop"    //switch to normal

#define TEST_START_CMD                                   "test start" //switch to test
#define TEST_STOP_CMD                                    "test stop"  //switch to normal












//for keil libc
#if defined (__CC_ARM)

int atoi(const char *src);

#endif

char *strsep(char **s, const char *ct);

//flash read
int flash_write(u32 addr, u8 *pbuf, int len);
//flash write with erase
void flash_read(u32 addr, u8 *pbuf, int len);

int mode_switch_hdl(void *context);

run_mode_e get_s907_run_mode(void);




#endif