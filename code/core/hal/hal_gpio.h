#ifndef HAL_GPIO_H
#define HAL_GPIO_H


typedef enum
{
    GPIO_PIN_RESET = 0,
    GPIO_PIN_SET
}gpio_status_e;



#define  GPIO_MODE_INPUT                        ((uint32_t)0x00000000)   /*!< Input Floating Mode                   */
#define  GPIO_MODE_OUTPUT                       ((uint32_t)0x00000001)   /*!< Output Push Pull Mode                 */
#define  GPIO_MODE_INT_RISING                   ((uint32_t)0x00000002)   /*!< GPIO Rising edge trigger Mode    */
#define  GPIO_MODE_INT_FALLING                  ((uint32_t)0x00000003)   /*!< GPIO Falling edge trigger Mode   */
#define  GPIO_MODE_INT_LEVEL_H					((uint32_t)0x00000004)	/*!< GPIO level h Mode                 */
#define  GPIO_MODE_INT_LEVEL_L					((uint32_t)0x00000005)  /*!< GPIO level l Mode                 */

#define  GPIO_MODE_AFP                          ((uint32_t)0xFFFFFFFE)
#define  GPIO_MODE_NC                           ((uint32_t)0xFFFFFFFF)

#define  GPIO_NOPULL                            ((uint32_t)0x00000000)   /*!< No Pull-up or Pull-down activation  */
#define  GPIO_PULLUP                            ((uint32_t)0x00000001)   /*!< Pull-up activation                  */
#define  GPIO_PULLDOWN                          ((uint32_t)0x00000002)   /*!< Pull-down activation                */
                    


typedef struct
{

  uint32_t mode;      /*!< Specifies the operating mode for the selected pins.
                           This parameter can be a value of @ref GPIO_mode */

  uint32_t pull;      /*!< Specifies the Pull-up or Pull-Down activation for the selected pins.
                           This parameter can be a value of @ref GPIO_pull */
}gpio_init_t;

//normal GPIO opt
hal_status_e  s907x_hal_gpio_init(u32 gpio_pin, gpio_init_t *init);
hal_status_e  s907x_hal_gpio_deinit(u32 gpio_pin);
hal_status_e  s907x_hal_gpio_write(u32 gpio_pin, gpio_status_e status);
gpio_status_e s907x_hal_gpio_read(u32 gpio_pin);
hal_status_e  s907x_hal_gpio_togglepin(u32 gpio_pin);
void          s907x_hal_gpio_it_start(u32 gpio_pin, hal_int_cb cb, void *context);
void          s907x_hal_gpio_it_stop(u32 gpio_pin);
hal_status_e  s907x_hal_gpio_set_io(u32 gpio_pin, u8 io);
hal_status_e  s907x_hal_gpio_set_pull(u32 gpio_pin, u8 pull);

#endif
