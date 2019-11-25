#include "s907x.h"
#include "flash_test.h"


   
#if M_AT_TEST

//example
hal_test_name_map_t flash_test_map[] = 
{
	{0, "FLASH_READ"},
	{1, "FLASH_WRITE"},
	{2, "FLASH_ERASE"},
};
 

 
void flash_read_test(hal_test_t *test)
{
	u32 addr = test->arg[0];
	u32 len = test->arg[1];
	u8 *pbuf;

	pbuf = wl_malloc(len);
    if(!pbuf) {
        return;
    }
	s907x_hal_flash_read(addr, pbuf, len);
    HAL_TEST_DBG("addr = 0x%08x,data = \n",addr);
    HAL_DBG_ARRARY(pbuf,len,ARY_U8,16);
    wl_free(pbuf);
	
} 

//write random word to flash
void flash_write_test(hal_test_t *test)
{
	u32 addr = test->arg[0];
    int cnt  = test->arg[1];
 	u8 *pbuf;
    u32 *pu32;
    int i;

	pbuf = wl_malloc(cnt*sizeof(int));
    if(!pbuf) {
        return;
    }  
    pu32 = (u32*)pbuf;
    for( i = 0; i < cnt; i++) {
        *pu32++ = wl_get_random32();
    }

    //write u32 * cnt
	s907x_hal_flash_write(addr, pbuf, cnt*sizeof(int));
    HAL_TEST_DBG("flash write success\n");
}  

void flash_erase(hal_test_t *test)
{
	u32 erase_type = test->arg[0];
	u32 addr = test->arg[1];


	s907x_hal_flash_erase(erase_type, addr);
}




void flash_test(hal_test_t *test)
{
	ASSERT(test);
	ASSERT(test->no < (sizeof(flash_test_map)/sizeof(hal_test_name_map_t)));

	HAL_TEST_DBG("test no %d = %s\n", flash_test_map[test->no].type, flash_test_map[test->no].name);   

	switch(test->no) 
	{
		case FLASH_READ:
			flash_read_test(test);
			break;
		case FLASH_WRITE:
			flash_write_test(test);
			break;
		case FLASH_ERASE:         
            flash_erase(test);  
            break;
		default:
			HAL_TEST_DBG("no such case\n");
			break;
	}
    
    
}

#endif