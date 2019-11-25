#ifndef HAL_WDG_H
#define HAL_WDG_H





typedef struct wdg_hdl_
{
	u32 time_ms;
	hal_cb_t it;
}wdg_hdl_t;








hal_status_e s907x_hal_wdg_deinit(wdg_hdl_t *wdg);
hal_status_e s907x_hal_wdg_init(wdg_hdl_t *wdg);
void s907x_hal_wdg_refresh(wdg_hdl_t *wdg);
void s907x_hal_wdg_start(wdg_hdl_t *wdg);
void s907x_hal_wdg_start_it(wdg_hdl_t *wdg);



#endif

