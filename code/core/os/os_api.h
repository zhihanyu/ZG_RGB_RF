#ifndef OS_API_H
#define OS_API_H


#include "dlist.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
 
struct timer_list{
	struct list_head entry;
	void (*func)(unsigned long);
	unsigned long data;
	unsigned long expires;
}; 
//typedef 
typedef void *sema_t;
typedef void *mutex_t;
typedef void *queue_t;
typedef void *thread_hdl_t;



typedef struct timer_list os_timer_t;
typedef void (*timer_callback)(unsigned long);
typedef void (*thread_func_t)(void *);

#include "system.h"


#define IS_SEMA_INIT(x)       ((x))
#define SEMA_SET_VAL(x,y)     ((x) = (y)) 


#ifndef TASK_NAME_MAX_LEN
	#define TASK_NAME_MAX_LEN 16
#endif

#ifndef portMAX_DELAY
    #define portMAX_DELAY 0xFFFFFFFF
#endif

typedef enum 
{
	sema_binary,
	sema_counter,
}sema_e;



#define wl_init_list(x)  INIT_LIST_HEAD((x))
#define wl_list_add(n,h) list_add_tail((n), (h))
#define wl_list_del(n) 	 list_del((n))


typedef struct thread_make_
{
	sema_t start;
	sema_t end;
}thread_make_t;
/**
  * @brief  memory set
  * @param  dec : Pointer to buffer
  * @param  val : value will be set to buffer
  * @param  num : how many buffer length will be set
  * @retval 
  */
void *wl_memset(void *dec, int val, size_t num );	
/**
  * @brief  memory copy
  * @param  dec : Pointer to destination buffer
  * @param  src : Pointer to source buffer
  * @param  num : how many source buffer length will be copy 
  * @retval 
  */
void *wl_memcpy(void *dec, const void *src, size_t num);
/**
  * @brief  wl_memcmp copy
  * @param  a : Pointer to  buffer a
  * @param  b : Pointer to  buffer b
  * @param  len : how many source buffer length will be compare 
  * @retval if len of buffer a equals len of buffer b reuturn 1 else return 0
  */
int wl_memcmp(const void *a, const void *b, size_t len);

/**
  * @brief wl_init_sema:  create and init  semaphores
  * @param  sema:  pointer of semaphores
  * @param  val:  init value 
  * @param  type:  type of  semaphores
  * @retval None
  */
void wl_init_sema(sema_t *sema, u32 val, sema_e type);

/**
  * @brief wl_free_sema: free memory of  semaphores
  * @param  sema:  pointer of semaphores to be freed
  * @retval None
  */
void wl_free_sema(sema_t *sema);

/**
  * @brief wl_send_sema:  
  * @param  sema:  pointer of semaphores
  * @retval None
  */
void wl_send_sema(sema_t *sema);

/**
  * @brief wl_send_sema_fromisr:  
  * @param  sema:  pointer of semaphores
  * @retval None
  */
void wl_send_sema_fromisr(sema_t *sema);

/**
  * @brief wl_wait_sema:  
  * @param  sema:  pointer of  semaphores
  * @param  timeout:  delay time in ms
  * @retval if success return TRUE, or return FALSE
  */
u32 wl_wait_sema(sema_t *sema, u32 timeout);

/**
  * @brief wl_init_mutex:  create and init  mutex
  * @param  pmutex:  pointer of mutex
  * @retval None
  */
void wl_init_mutex(mutex_t *pmutex);

/**
  * @brief wl_free_mutex:  free  memory of a  mutex
  * @param  pmutex:  pointer of mutex to be freeed
  * @retval None
  */
void wl_free_mutex(mutex_t *pmutex);

/**
  * @brief wl_unlock_mutex:  unlock/get a  mutex
  * @param  pmutex:  pointer of mutex to be unlocked
  * @retval None
  */
void wl_lock_mutex(mutex_t *plock);

/**
  * @brief wl_unlock_mutex_to:  unlock/get  a  mutex in timeout 
  * @param  pmutex:  pointer of mutex to be unlocked
  * @param  timeout_ms:  time in ms of blocktime
  * @retval if ok ,return TURE, or return FALSE
  */
int wl_lock_mutex_to(mutex_t *plock, u32 timeout_ms);


/**
  * @brief wl_lock_mutex: lock a  mutex
  * @param  pmutex:  pointer of mutex to be locked
  * @retval None
  */
void wl_unlock_mutex(mutex_t *plock);

/**
  * @brief wl_enter_critical: task enter critical state
  * @param  None
  * @retval None
  */
void wl_enter_critical(void);

/**
  * @brief wl_exit_critical: task exit critical state
  * @param  None
  * @retval None
  */
void wl_exit_critical(void);

/**
  * @brief wl_init_queue: create and init  message queue
  * @param  queue:  pointer of create queue
  * @param  msg_size: size of  item
  * @param  depth : number of  elements in  queue
  * @retval : if ok ,return 0,or return -1
  */
int wl_init_queue( queue_t* queue, u32 msg_size, u32 depth);

/**
  * @brief wl_send_queue:  send message to queue
  * @param  queue:  pointer of send queue
  * @param  message: message information
  * @param  timeout_ms : timeout value or 0 in case of no-timeout
  * @retval : if ok ,return 0,or return -1
  */
int wl_send_queue( queue_t* queue, void* message, u32 timeout_ms );

/**
  * @brief wl_wait_queue:  get or wait for message from a queue
  * @param  queue:  pointer of a queue
  * @param  message: message information
  * @param  timeout_ms : timeout value or 0 in case of no-timeout
  * @retval : if ok ,return 0,or return -1
  */
int wl_wait_queue( queue_t* queue, void* message, u32 timeout_ms );

/**
  * @brief wl_free_queue:  free  memory of  a queue
  * @param  queue:  pointer of aqueue
  * @retval : if ok ,return 0,or return -1
  */
int wl_free_queue( queue_t* queue );

/**
  * @brief wl_get_systemtick: get system counter
  * @param  : None
  * @retval :  system counter as 32-bit value
  */
u32 wl_get_systemtick(void);

/**
  * @brief wl_systemtick_to_ms: get system counter to ms
  * @param  :  system tick
  * @retval :  value in ms 
  */
u32 wl_systemtick_to_ms(u32 tick);

/**
  * @brief wl_ms_to_systemtick: get value ms to  system counter 
  * @param  :   time in ms
  * @retval :  value in system tick 
  */
u32 wl_ms_to_systemtick(u32 ms);

/**
  * @brief wl_os_mdelay:  os time delay
  * @param  :   delay time in ms
  * @retval : None
  */
void wl_os_mdelay(int ms);

/**
  * @brief wl_hal_udelay:  hal time delay
  * @param  :   time in us
  * @retval : None
  */
void wl_hal_udelay(int us);

/**
  * @brief wl_os_yield: os yield  task
  * @param  : None
  * @retval : None
  */
void wl_os_yield(void);

/**
  * @brief wl_atomic_set:  set atomic variable
  * @param  v : pointer of type atomic_t
  * @param  i  : required value
  * @retval : None
  */
void wl_atomic_set(atomic_t *v, int i);

/**
  * @brief wl_atomic_read:  read atomic variable
  * @param  v : pointer of type atomic_t
  * @retval :   value of @v 
  */
int wl_atomic_read(atomic_t *v);

/**
  * @brief wl_atomic_add:  add  atomic variable
  * @param  v : pointer of type atomic_t
  * @param  i  : required add value
  * @retval :  None
  */
void wl_atomic_add(atomic_t *v, int i);

/**
  * @brief wl_atomic_sub:  sub  atomic variable
  * @param  v : pointer of type atomic_t
  * @param  i  : required sub value
  * @retval :  None
  */
void wl_atomic_sub(atomic_t *v, int i);

/**
  * @brief wl_atomic_add_return:  add  atomic variable
  * @param  v : pointer of type atomic_t
  * @param  i  : required add value
  * @retval :   value of @v add @i
  */
int wl_atomic_add_return(atomic_t *v, int i);

/**
  * @brief wl_atomic_sub_return:  sub  atomic variable
  * @param  v : pointer of type atomic_t
  * @param  i  : required sub value
  * @retval :  value of @v sub @i
  */
int wl_atomic_sub_return(atomic_t *v, int i);

/**
  * @brief wl_create_thread:  create  thread
  * @param  name : just a name for  task to aid debugging
  * @param  stack_size : stack size
  * @param  priority :  priority assgined to  task
  * @param  func : function that implements  tsak
  * @param  thctx : task handler
  * @retval : if create thread sucess ,return thread_hdl_t ,or return NULL
  */
thread_hdl_t wl_create_thread(const char *name, u32  stack_size, u32 priority, thread_func_t func, void *thctx);

/**
  * @brief wl_currrnet_threadid:  get current thread id
  * @param  Noner
  * @retval : thread id
  */
thread_hdl_t wl_currrnet_threadid(void);

/**
  * @brief wl_destory_thread:  delete thread
  * @param  hdl :  thread to be deleted
  * @retval : None
  */
void wl_destory_thread(thread_hdl_t hdl);

/**
  * @brief wl_destory_threadself : delete calling task
  * @param  None
  * @retval : None
  */
void wl_destory_threadself(void);

/**
  * @brief wl_get_prio : get the priority of the task 
  * @param  the task to be getted the priority
  * @retval : the prioroty 
  */
u32 wl_get_prio(thread_hdl_t task);

/**
  * @brief wl_set_prio : set the priority of the task 
  * @param  task: the task will be settted new priority
  * @param  prio : the new priority 
  * @retval : None
  */
void wl_set_prio(thread_hdl_t task, int prio);

/**
  * @brief wl_init_timer:  init  timer handler ,and set timer handler function
  * @param  ptimer : pointer of timer to be inited
  * @param  pfunc : function  of timer handler
  * @param  context : data of  ptimer
  * @retval : None
  */
void wl_init_timer(os_timer_t *ptimer, timer_callback pfunc, void* context);

/**
  * @brief wl_start_timer:  start timer handler and set delay time
  * @param  ptimer : pointer of timer to be started
  * @param  ms : delay time
  * @retval : None
  */
void wl_start_timer(os_timer_t *ptimer, u32 ms);

/**
  * @brief wl_stop_timer:  stop timer handler
  * @param  ptimer : pointer of timer to be stopped
  * @retval : None
  */
void wl_stop_timer(os_timer_t *ptimer);

/**
  * @brief wl_destory_timer:  delete timer handler 
  * @param  ptimer : pointer of timer to be deleted
  * @retval : None
  */
void wl_destory_timer(os_timer_t *ptimer);

/**
  * @brief wl_get_freeheapsize : system dynamic memory heap left 
  * @retval : None
  */
u32 wl_get_freeheapsize(void);

/**
  * @brief wl_malloc:  allocate  memory
  * @param  sz : size of memeroy will be allocated
  * @retval : if success ,return  pointer to allocated memory , or return NULL
  */
void* wl_malloc(u32 sz);

/**
  * @brief wl_zmalloc:  allocate and zero memory   
  * @param  sz :how many source buffer length will be allocated and zero
  * @retval if success ,return  pointer to allocated memory , or return NULL
  */
void* wl_zmalloc(u32 sz);

/**
  * @brief wl_zmalloc:  reallocate and zero memory   
  * @param  sz :how many source buffer length will be allocated and zero
  * @retval if success ,return  pointer to reallocated memory , or return NULL
  */
void* wl_realloc(void *ptr, size_t sz);
/**
  * @brief wl_free:  free allocated memory
  * @param  pbuf :   array will be free
  * @param  sz :how many source buffer length will be free
  * @retval None
  */ 
void wl_free(void *pbuf);
 
/**
  * @brief wl_get_random32:  get random number 
  * @param  None
  * @retval seed : the generated random number
  */
u32 wl_get_random32(void);

/**
  * @brief wl_os_init:  init  handler for OS 
  * @param  None
  * @retval None
  */
void wl_os_init(void);

/**
  * @brief wl_os_start:  enable the schedule , start the RTOS kernel
  * @param  None
  * @retval None
  */
void wl_os_start(void);


#endif
