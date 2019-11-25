#ifndef SYSTEM_DEBUG_H
#define SYSTEM_DEBUG_H


#define BUFFERED_PRINTF     0

#undef  printf
#define printf     system_printf

#define Z_DEBUG()  printf("%s %d\r\n", __func__, __LINE__);

#define MAC_FMT    "%02x%02x%02x%02x%02x%02x"


#define ARY_U8      0
#define ARY_U16     1
#define ARY_U32     2
 
//debug arrary
u32 system_printf(const char *fmt, ...);

static inline void printf_arrary(u8 * buf, int len, int type, int nums)
{
		
	int i, j = 0;
    u8  *pu8;
    u16 *pu16;
    u32 *pu32;
    
	if(nums == 0) 
		nums = 8;

    if(type == ARY_U8) {
        pu8 = (u8 *)buf;
    } else if(type == ARY_U16) {
        //ASSERT len % 2
        len = len >> 1;
        pu16 = (u16 *)buf;
    } else if(type == ARY_U32) {
        //ASSERT len % 4
        len = len >> 2;
        pu32 = (u32 *)buf;
    }
	for( i = 0; i < len; i++) {
        if(type == ARY_U8) {
            printf("%02x ", pu8[i]);
        } else if(type == ARY_U16) {
            printf("%04x ", pu16[i]);
        } else if(type == ARY_U32) {
            printf("%08x ", pu32[i]);
        }
		if(++j >= nums) {
			j = 0;
			printf("\n");
		}
	}
	printf("\n");
}



#define S9070_ERR(_module, _message, ...) \
	    do { \
	        printf("Error | M[%s] | %s:%d ",  #_module, __FUNCTION__, __LINE__); \
	        printf((_message), ##__VA_ARGS__); \
	        printf("\n"); \
	    } while (0)

#define S9070_WARN(_module, _message, ...) \
		do { \
			printf("Warn | M[%s] | %s:%d ", #_module,  __FUNCTION__,  __LINE__); \
			printf((_message), ##__VA_ARGS__); \
			printf("\n"); \
		} while (0)

#define S9070_MSG(_module, _message, ...) \
		do { \
			printf("Msg | M[%s] | %s:%d ", #_module, __FUNCTION__,  __LINE__); \
			printf((_message), ##__VA_ARGS__); \
			printf("\n"); \
		} while (0)

		
//config debug enable
#define HALTEST_DEBUG_EN    1
#define HTTPC_DEBUG_EN      0
#define PING_DEBUG_EN       1
#define AT_CMD_DEBUG        0

//coustom lib debug
#define COLINK_DBG_EN       0



#if AT_CMD_DEBUG
#define AT_DBG_ERR(fmt, arg...)         S9070_ERR(at_cmd,fmt,##arg)
#define AT_DBG_WARN(fmt, arg...)        S9070_WARN(at_cmd,fmt,##arg)
#define AT_DBG_MSG(fmt, arg...)         S9070_MSG(at_cmd,fmt,##arg)
#else
#define AT_DBG_ERR(fmt, arg...)         
#define AT_DBG_WARN(fmt, arg...)        
#define AT_DBG_MSG(fmt, arg...)         
#endif

#define USER_DBG(fmt,arg...)             S9070_MSG(user,fmt,##arg)

#define LIB_DBG(fmt,arg...)              S9070_MSG(lib,fmt,##arg)

//hal test debug
#if HALTEST_DEBUG_EN
#define HAL_TEST_DBG(fmt,arg...)          S9070_MSG(hal_test,fmt,##arg)
#define HAL_DBG_ARRARY(buf,len,type,nums) printf_arrary(buf,len,type,nums)
#else
#define HAL_TEST_DBG(x, ...)     
#define HAL_DBG_ARRARY(x, len, nums)   
#endif


//httpc debug
#if HTTPC_DEBUG_EN
#define HTTP_ERR(fmt,arg...)    		S9070_ERR(httpc,fmt,##arg)
#define HTTP_WARN(fmt,arg...)   		S9070_WARN(httpc,fmt,##arg)
#define HTTP_DBG(fmt,arg...)   		 	S9070_MSG(httpc,fmt,##arg)
#else
#define HTTP_DBG(x, ...)
#define HTTP_WARN(x, ...)
#define HTTP_ERR(x, ...)
#endif

//ping 
#ifdef PING_DEBUG_EN
#define PING_LOGE(fmt,arg...)           S9070_ERR(httpc,fmt,##arg)
#define PING_LOGW(fmt,arg...)           S9070_WARN(httpc,fmt,##arg)
#define PING_LOGI(fmt,arg...)           S9070_MSG(httpc,fmt,##arg)
#else
#define PING_LOGE(fmt,arg...)           
#define PING_LOGW(fmt,arg...)           
#define PING_LOGI(fmt,arg...)    
       
#endif      



//coustom debug

#if COLINK_DBG_EN
#define COLINK_DBG              printf
#else
#define COLINK_DBG              
#endif




#define OTA_DEBUG printf


       
#define MICO_MSG(fmt,args...) \
		do { \
			if(mico_debug_en)\
				printf("[MICO_MSG]==>[%s:%d]: "fmt,__FUNCTION__,__LINE__,## args); \
		} while (0)















#endif
