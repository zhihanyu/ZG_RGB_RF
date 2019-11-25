#ifndef HAL_H
#define HAL_H





#define __CM4_REV                 0x0001U  /*!< Core revision r0p1                            */
#define __MPU_PRESENT             1U       /*!< STM32F4XX provides an MPU                     */
#define __NVIC_PRIO_BITS          4U       /*!< STM32F4XX uses 4 Bits for the Priority Levels */
#define __Vendor_SysTickConfig    0U       /*!< Set to 1 if different SysTick Config is used  */
#define __FPU_PRESENT             1U       /*!< FPU present                                   */


typedef enum
{
/******  Cortex-M4 Processor Exceptions Numbers ****************************************************************/
  NonMaskableInt_IRQn         = -14,    /*!< 2 Non Maskable Interrupt                                          */
  MemoryManagement_IRQn       = -12,    /*!< 4 Cortex-M4 Memory Management Interrupt                           */
  BusFault_IRQn               = -11,    /*!< 5 Cortex-M4 Bus Fault Interrupt                                   */
  UsageFault_IRQn             = -10,    /*!< 6 Cortex-M4 Usage Fault Interrupt                                 */
  SVCall_IRQn                 = -5,     /*!< 11 Cortex-M4 SV Call Interrupt                                    */
  DebugMonitor_IRQn           = -4,     /*!< 12 Cortex-M4 Debug Monitor Interrupt                              */
  PendSV_IRQn                 = -2,     /*!< 14 Cortex-M4 Pend SV Interrupt                                    */
  SysTick_IRQn                = -1,     /*!< 15 Cortex-M4 System Tick Interrupt                                */
/******  s907x specific Interrupt Numbers **********************************************************************/

} IRQn_Type;

#define IS_NVIC_DEVICE_IRQ(IRQ)                ((IRQ) >= (IRQn_Type)0x00U)

#define NVIC_PRIORITYGROUP_0         			0x00000007U /*!< 0 bits for pre-emption priority
                                     			                 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         			0x00000006U /*!< 1 bits for pre-emption priority
                                     			                 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         			0x00000005U /*!< 2 bits for pre-emption priority
                                     			                 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         			0x00000004U /*!< 3 bits for pre-emption priority
                                     			                 1 bits for subpriority */
#define NVIC_PRIORITYGROUP_4         			0x00000003U /*!< 4 bits for pre-emption priority 0 bits for subpriority */

#define IS_NVIC_PRIORITY_GROUP(GROUP) 			(((GROUP) == NVIC_PRIORITYGROUP_0) || \
                                      			 ((GROUP) == NVIC_PRIORITYGROUP_1) || \
                                      			 ((GROUP) == NVIC_PRIORITYGROUP_2) || \
                                      			 ((GROUP) == NVIC_PRIORITYGROUP_3) || \
                                      			 ((GROUP) == NVIC_PRIORITYGROUP_4))

#include "s907x_cortex.h"
#include "os_api.h"
#include "core_cm4.h"
#include "hal_config.h"
#include "hal_def.h"
#include "hal_interrupt.h"


#if ( HAL_DMA_M == M_ON )
#include "hal_dma.h"
#endif

#if ( HAL_I2C_M == M_ON )
#include "hal_i2c.h"
#endif

#if ( HAL_I2S_M == M_ON )
#include "hal_i2s.h"
#endif

#if ( HAL_SPI_M == M_ON )
#include "hal_spi.h"
#endif

#if ( HAL_TIMER_M == M_ON )
#include "hal_timer.h"
#endif

#if ( HAL_UART_M == M_ON )
#include "hal_uart.h"
#endif

#if ( HAL_LOGUART_M == M_ON )
#include "hal_loguart.h"
#endif

#if ( HAL_ADC_M == M_ON )
#include "hal_adc.h"
#endif

#if ( HAL_RCC_M == M_ON )
#include "hal_rcc.h"
#endif

#if ( HAL_CLK_M == M_ON )
#include "hal_clk.h"
#endif


#if ( HAL_GPIO_M == M_ON )
#include "hal_gpio.h"
#endif


#if ( HAL_RTC_M == M_ON )
#include "hal_rtc.h"
#endif


#include "hal_pinmux.h"

#if ( HAL_WDG_M == M_ON )
#include "hal_wdg.h"
#endif




#if ( HAL_FLASH_M == M_ON )
#include "hal_flash.h"
#endif

#if ( HAL_SLEEP_M == M_ON )
#include "hal_sleep.h"
#endif
























#endif

