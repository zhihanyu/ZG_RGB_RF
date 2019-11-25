#ifndef SYSTEM_TASK_H
#define SYSTEM_TASK_H



//bigger is higer
#define IDLE_PRIORITY				(0U)
#define MAX_PRIORITIES              (11U)

#define MAIN_TASK_PRIO				(IDLE_PRIORITY + 1)	
#define MAIN_TASK_STACK_SZ			(512) 

#define SLINK_TASK_PRIO				(IDLE_PRIORITY + 1)	
#define SLINK_TASK_STACK_SZ			(512) 

#define AT_TASK_PRIO				(IDLE_PRIORITY + 2)	
#define AT_TASK_STACK_SZ			(4096) //3072

#define HTTP_CLIENT_PRIO            (IDLE_PRIORITY + 3)
#define HTTP_CLIENT_STACK_SZ        (512)


#define MAIN_WLAN_TX_PRIO			(IDLE_PRIORITY + 1)	
#define MAIN_WLAN_TX_STACK_SZ		(512) 

#define MAIN_UART_RX_PRIO			(IDLE_PRIORITY + 3)	
#define MAIN_UART_RX_STACK_SZ		(512) 

#define WLAN_INIT_PRIO			    (IDLE_PRIORITY + 7)	
#define WLAN_INIT_STACK_SZ		    (4096)	
  	  
#define PING_TASK_PRIO              (IDLE_PRIORITY + 1)
#define PING_TASK_STACK_SZ          (512)   

#define IPERF_UDP_TASK_PRIO         (IDLE_PRIORITY + 1)
#define IPERF_UDP_TASK_STACK_SZ     (512)   

#define SCAN_TASK_PRIO              (IDLE_PRIORITY + 1)
#define SCAN_TASK_STACK_SZ          (512)   

#define TIMER_CHECK_PRIO			(IDLE_PRIORITY + 11)	
#define TIMER_CHECK_STACK_SZ		(2048)

#define MQTT_TEST_PRIO              (IDLE_PRIORITY + 10)
#define MQTT_TEST_STACK_SZ          (4096)











#endif


