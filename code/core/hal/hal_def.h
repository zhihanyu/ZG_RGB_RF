#ifndef HAL_DEF_H
#define HAL_DEF_H



#define HAL_DMA_RX							(0)
#define HAL_DMA_TX							(1)	

#define HAL_DIR_TX				 			(0)
#define HAL_DIR_RX				 			(1)

#define HAL_SLAVE_SEL			 			(0)
#define HAL_MASTER_SEL						(1)		
	
#define  HAL_OK     						 (  0 )
#define  HAL_ERROR  						 ( -1 )
#define  HAL_BUSY   						 ( -2 )
#define  HAL_TIMEOUT						 ( -3 )
#define  HAL_NO_MEMORY                       ( -4 )

typedef int hal_status_e;

typedef enum 
{
  HAL_UNLOCKED = 0x00,
  HAL_LOCKED   = 0x01  
} hal_lock_e;

typedef enum
{
	HAL_POLL,
	HAL_INT,
	HAL_DMA,
}hal_mode_e;

typedef enum 
{
	HAL_OFF,
	HAL_ON
}hal_switch_e;

typedef void (*hal_int_cb)(void *);

typedef struct hal_cb_
{
	hal_int_cb func;
	void       *context;
}hal_cb_t;

#define min(a, b)   ((a) < (b) ? (a) : (b))

#define max(a, b)   ((a) > (b) ? (a) : (b))

#define HAL_MAX_DELAY      0xFFFFFFFF

#define ASSERT(expr)	if((expr) == 0){														\
							printf("Assert failed %s %d", __func__, __LINE__);					\
							while(1);															\
						}

#define ASSERT_ARG(expr, arg) if((expr) == 0){					\
                                printf arg;				    	\
                                while(1);						\
                              }

#define ASSERT_PARAM(expr)	if((expr) == 0){													\
								printf("Assert failed %s %d", __func__, __LINE__);				\
							}

#define HAL_IS_FLAG_SET(isr, flag)   (((isr & flag)) ? 1 : 0)




















#endif
