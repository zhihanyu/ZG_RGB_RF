#include "s907x.h"
#include "at_network.h"
#include "wlan_api.h"
#include "at_wlan.h"
#include "dhcps.h"
#include "lwip_conf.h"
#pragma section=".ota1_ram.bss"
 
static thread_hdl_t wlan_task_hdl;
static thread_hdl_t user_main_task_hdl;
static u8 version_s907x[40];
int errno;
   
extern struct netif xnetif[2]; 

#if defined ( __GNUC__ )
extern uint8_t __bss_start__[];
extern uint8_t __bss_end__[];
#endif

void main_task(void* context)
{
	while(1){
		wl_os_mdelay(2000);
    printf("main task running..\n");   
	}
} 
   
int mbedtls_hardware_poll( void *data,
                           unsigned char *output, size_t len, size_t *olen )
{
 
	 return 0;
}


static void mbedtls_free(void * ptr)
{
	wl_free(ptr);
}

static void* mbedtls_calloc(size_t nmemb, size_t size)
{
	void *mem;

	mem = wl_zmalloc(nmemb*size);
	if(!mem) {
		printf("mbedtls_calloc malloc failed\n");
	}

	return mem;
}




char * get_app_version_s907x( void )
{    
	memset( version_s907x, 0, VERSION_LENGTH );
	
	#if (EVAL_BOARD == 1)    
	sprintf( version_s907x, VERSION_FMT,
          COUSTOM_STR,   
            YEAR, MONTH+1, DAY, 
	SDK_PLATFORM_STR,  
	SDK_FUNCTION_STR,    
	DATA_FORMAT,   
	"E", 
	MAJOR_VERSION,   
	MINOR_VERSION, 
	REVISION_NUMBER );

	#else    
	sprintf( version_s907x, VERSION_FMT, 
	COUSTOM_STR, 
	YEAR, MONTH+1, DAY, 
	SDK_PLATFORM_STR, 
	SDK_FUNCTION_STR, 
	DATA_FORMAT, 
	MAJOR_VERSION,  
	MINOR_VERSION, 
	REVISION_NUMBER );
	#endif    
	return version_s907x;

}
 

static void wlan_task(void *context)
{
	int ret;
    __set_MSP(MSP_TOP);
#if M_AT_ENABLE
    at_hdl_int();
#endif

#if CONFIG_OTA_DUAL_IMAGES 
    ota_init_boot_params();
#endif
	
	USER_DBG("lwip init...");
	LwIP_Init();
  
	mbedtls_platform_set_calloc_free(mbedtls_calloc, mbedtls_free);
	USER_DBG("wlan start..."); 
	printf("heap size %d\n", xPortGetFreeHeapSize());
	
	//s907x_wlan_exit_mp();	
#if M_AT_ESP	 
	at_esp_init();
#else
	//test heap malloc code
	s907x_wlan_off();	
	wl_os_mdelay(57);
	ret = s907x_wlan_on(S907X_MODE_STA);	
	if(ret)
		goto exit; 	

    if(get_s907_run_mode() == s907x_mode_normal){
        //user app
        wl_create_thread("main thread", 512, MAIN_TASK_PRIO, (thread_func_t)main_task, NULL);
	}else if(get_s907_run_mode() == s907x_mode_mp){
		s907x_wlan_enter_mp();
        s907x_wlan_off(); 
        wl_os_mdelay(50); 
        s907x_wlan_on(S907X_MODE_STA);
	} else if(get_s907_run_mode() == s907x_mode_test){
        //DoNothing wait recv at cmd
	} 
#endif		


exit:
	
	printf("heap size %d\n", xPortGetFreeHeapSize());	
	wl_destory_threadself();
  
}   
  
void wlan_start(void)
{
    wlan_task_hdl = wl_create_thread("wlan task", WLAN_INIT_STACK_SZ, WLAN_INIT_PRIO, (thread_func_t)wlan_task, NULL);
} 
   
    

static void init_ota_bss()
{
    u8* bss_start;
    u8* bss_end;
    int ota_bss_len; 
    int i = 0;
    u32 *set;
		
      
#if defined (__CC_ARM)
	
	extern u8 HEAP_START_SECTION heap_start[];
		
    bss_start   =   (u8*)&Image$$OTA_RAM_BSS$$Base;
    bss_end     =   (u8*)&Image$$OTA_RAM_BSS$$Limit;
    ota_bss_len =   heap_start - bss_start;

    u8* rom_start = (u8*)&Image$$USER_CODE_REGION$$Base;
    u8* rom_end = (u8*)&Image$$USER_CODE_REGION$$Limit;
    int ota_rom_len = Image$$USER_CODE_REGION$$Length;

    u8* ram_start   =   (u8*)&Image$$OTA_RAM_DATA$$Base;
    u8* ram_end     =   (u8*)&Image$$OTA_RAM_DATA$$Limit;
    int ota_ram_len =   Image$$OTA_RAM_DATA$$Length;

#elif defined ( __GNUC__ )
	bss_start   =   (uint8_t*)__bss_start__;
	bss_end     =   (uint8_t*)__bss_end__;
	ota_bss_len = (bss_end - bss_start);
  
#elif defined (__ICCARM__)

    bss_start   =   (u8*)__section_begin(".ota1_ram.bss");
    bss_end     =   (u8*)__section_end(".ota1_ram.bss");
    ota_bss_len = (bss_end - bss_start);
#endif

    set = (u32*)bss_start;
    for( i = 0; i < ota_bss_len; i+=4) {
        *set ++ = 0;
    } 
}           

//s9070 evbboad
//output no pu/pd
//inpput pu or pd
const board_pin_t board_pin[]=
{
    /*pin                 type         GPIOx              pinmux                       dir                     pu/pd            sleed pu/pd*/
    {PIN1,              PIN_POWER,     GPIONULL,       GPIO_AFP_NC,                GPIO_MODE_NC,           GPIO_NOPULL,        GPIO_NOPULL},
    {PIN2,              PIN_CHIP_EN,   GPIONULL,       GPIO_AFP_NC,                GPIO_MODE_NC,           GPIO_NOPULL,        GPIO_NOPULL},
    {PIN3,              PIN_GPIO,      GPIO3,          GPIO3_AFP_GPIO,             GPIO_MODE_OUTPUT,       GPIO_NOPULL,        GPIO_NOPULL},
    {PIN4,              PIN_GPIO,      GPIO4,          GPIO4_AFP_GPIO,             GPIO_MODE_OUTPUT,       GPIO_NOPULL,        GPIO_NOPULL},
    {PIN5,              PIN_GPIO,      GPIO5,          GPIO5_AFP_GPIO,             GPIO_MODE_OUTPUT,       GPIO_NOPULL,        GPIO_NOPULL},
    {PIN6,              PIN_NC,         GPIO7,          GPIO7_AFP_GPIO,             GPIO_MODE_OUTPUT,       GPIO_NOPULL,        GPIO_NOPULL},
    {PIN7,              PIN_NC,        GPIONULL,       GPIO_AFP_NC,                GPIO_MODE_NC,           GPIO_NOPULL,        GPIO_NOPULL},
    {PIN8,              PIN_GPIO,      GPIO6,          GPIO6_AFP_GPIO,             GPIO_MODE_NC,           GPIO_NOPULL,        GPIO_NOPULL},    
    {PIN9,              PIN_GND,       GPIONULL,       GPIO_AFP_NC,                GPIO_MODE_NC,           GPIO_NOPULL,        GPIO_NOPULL},  
    {PIN10,             PIN_NC,        GPIONULL,       GPIO_AFP_NC,                GPIO_MODE_NC,           GPIO_NOPULL,        GPIO_NOPULL},  
    {PIN11,             PIN_GPIO,      GPIO18,         GPIOX_AFP_GPIO,             GPIO_MODE_OUTPUT,        GPIO_NOPULL,      GPIO_NOPULL},  
    {PIN12,             PIN_GPIO,      GPIO23,         GPIOX_AFP_GPIO,             GPIO_MODE_OUTPUT,       GPIO_NOPULL,     GPIO_NOPULL},  
    {PIN13,             PIN_GND,       GPIONULL,       GPIO_AFP_NC,                GPIO_MODE_NC,           GPIO_NOPULL,        GPIO_NOPULL},  
    {PIN14,             PIN_GPIO,       GPIO19,         GPIO19_AFP_GPIO,           GPIO_MODE_OUTPUT,      GPIO_NOPULL,       GPIO_NOPULL},  
    {PIN15,             PIN_GPIO,       GPIO22,         GPIO22_AFP_GPIO,           GPIO_MODE_OUTPUT,      GPIO_NOPULL,       GPIO_NOPULL},  
    {PIN16,             PIN_GPIO,      GPIO1,          GPIO1_AFP_LOG_TXD,          GPIO_MODE_AFP,          GPIO_PULLUP,        GPIO_PULLUP},  
    {PIN17,             PIN_GPIO,      GPIO2,          GPIO2_AFP_LOG_RXD,          GPIO_MODE_AFP,          GPIO_PULLUP,        GPIO_PULLUP},  
    {PIN18,             PIN_GND,       GPIONULL,       GPIO_AFP_NC,                GPIO_MODE_NC,           GPIO_NOPULL,        GPIO_NOPULL},
//for s9070A no layout pins                 

    {PIN_NO_LAYOUT,     PIN_GPIO,      GPIO0,          GPIO0_AFP_GPIO,             GPIO_MODE_OUTPUT,       GPIO_NOPULL,        GPIO_NOPULL},

    {PIN_NO_LAYOUT,     PIN_GPIO,      GPIO8,          GPIOX_AFP_GPIO,             GPIO_MODE_OUTPUT,       GPIO_NOPULL,        GPIO_NOPULL},
    {PIN_NO_LAYOUT,     PIN_GPIO,      GPIO9,          GPIOX_AFP_GPIO,             GPIO_MODE_OUTPUT,       GPIO_NOPULL,        GPIO_NOPULL},
    {PIN_NO_LAYOUT,     PIN_GPIO,      GPIO10,         GPIOX_AFP_GPIO,             GPIO_MODE_OUTPUT,       GPIO_NOPULL,        GPIO_NOPULL},
    {PIN_NO_LAYOUT,     PIN_GPIO,      GPIO11,         GPIOX_AFP_GPIO,             GPIO_MODE_OUTPUT,       GPIO_NOPULL,        GPIO_NOPULL},
    {PIN_NO_LAYOUT,     PIN_GPIO,      GPIO20,         GPIOX_AFP_GPIO,             GPIO_MODE_OUTPUT,       GPIO_NOPULL,        GPIO_NOPULL},
    {PIN_NO_LAYOUT,     PIN_GPIO,      GPIO21,         GPIOX_AFP_GPIO,             GPIO_MODE_OUTPUT,       GPIO_NOPULL,        GPIO_NOPULL},
};


void s907x_gpio_init(int level)
{
    int i;

    //s907x_hal_pinmux_swd_off();

    for( i = 0; i < sizeof(board_pin)/sizeof(board_pin_t); i++) {
	//printf("config pin = %d type = %d !\n", board_pin[i].pin, board_pin[i].type);	
        if(board_pin[i].type != PIN_GPIO) {
            continue;
        }
	//printf("config pin = %d is gpio !\n", board_pin[i].pin);

	
        if(board_pin[i].afp == GPIO_AFP_NC) {
            continue;
        }
	    //printf("=> pin = %d AFP = %d !\n", board_pin[i].pin, board_pin[i].afp);	
        //config to AFP
        s907x_hal_pinmux_config(board_pin[i].gpio, board_pin[i].afp);
        //config pupd
        //s907x_hal_gpio_set_pull(board_pin[i].gpio, board_pin[i].pupd_normal);  
        //if gpio output low    
         if(board_pin[i].afp == GPIOX_AFP_GPIO) {
                gpio_init_t init;
		        //printf("config pin = %d gpio = %x\n", board_pin[i].pin, board_pin[i].gpio);
                init.mode = GPIO_MODE_OUTPUT;
                init.pull = board_pin[i].pupd_normal;
                s907x_hal_gpio_init(board_pin[i].gpio, &init);
                s907x_hal_gpio_write(board_pin[i].gpio, level);
        }  
    }
}

static void main_entry(void)
{   
	__set_MSP(MSP_TOP);
	printf("s907x sdk version %s\n", get_app_version_s907x());
		
    init_ota_bss();
	s907x_gpio_init(0);

    system_init();

    wl_os_init(); 

    wlan_start();

    wl_os_start();

    //nerver come here
    while(1);
}

 
static void ps_entry(void)
{	
    s907x_hal_wake_handle();
}

SYSTEM_ENTRY_SECTION system_entry_t system_entry = {
    main_entry,
    ps_entry,
    S9070_VALID_KEY,
};
