#include "s907x.h"

#include "FreeRTOS.h"
#include "task.h"

  
StackType_t IdleTaskStack[idle_task_size];
StackType_t TimerTaskStack[configTIMER_TASK_STACK_DEPTH];


static StaticTask_t IdleTaskTCB;
static StaticTask_t TimerTaskTCB;

  
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, 
                                    StackType_t **ppxIdleTaskStackBuffer, 
                                    uint32_t *pulIdleTaskStackSize )
{
    *ppxIdleTaskTCBBuffer = &IdleTaskTCB;
    *ppxIdleTaskStackBuffer = IdleTaskStack;
    *pulIdleTaskStackSize = idle_task_size;
}

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, 
                                    StackType_t **ppxTimerTaskStackBuffer, 
                                    uint32_t *pulTimerTaskStackSize )
{
    *ppxTimerTaskTCBBuffer = &TimerTaskTCB;
    *ppxTimerTaskStackBuffer = TimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
 

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    printf("vApplicationStackOverflowHook \n\r");
    printf("task name : %s, TCB : %x\n\r", pcTaskName, xTask);
  
}

void vApplicationTickHook( void )
{
#if defined (__CC_ARM)
	__asm(" nop");
#else
	asm(" nop");
#endif
}

void vApplicationMallocFailedHook( void )
{
#if defined (__CC_ARM)
	__asm(" nop");
#else
	asm(" nop");
#endif
}


int  freertos_ready_to_sleep()
{
#if S907X_LOW_POWER == 0
	return 0;
#else
	return freertos_ll_ready_to_sleep();
#endif
} 

void freertos_pre_sleep_processing(unsigned int *expected_idle_time)
{
#if S907X_LOW_POWER == 0
	return ;
#else
	freertos_ll_pre_sleep_processing(expected_idle_time);
#endif

}

void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime )
{
#if S907X_LOW_POWER == 0
	return ;
#else
	vPort_ll_SuppressTicksAndSleep(xExpectedIdleTime);
#endif
}

void freertos_post_sleep_processing(unsigned int *expected_idle_time)
{
#if S907X_LOW_POWER == 0
	return ;
#else
	freertos_ll_post_sleep_processing(expected_idle_time);
#endif
}


