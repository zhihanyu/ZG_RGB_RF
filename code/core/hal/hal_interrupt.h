#ifndef HAL_INTERRUPT_H
#define HAL_INTERRUPT_H









typedef void(*interrupt_func)(void* arg);



void s907x_hal_reg_interrupt_event(u32 peripheral, interrupt_func func, void* context);
void s907x_hal_unreg_interrupt_event(u32 peripheral);




























#endif
