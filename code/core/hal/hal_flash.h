#ifndef HAL_FLASH_H
#define HAL_FLASH_H

    
//flash layout

#define FLASH_1M              0
#define FLASH_2M              1
#define FLASH_SIZE            FLASH_2M

#define FLASH_PAGE_SIZE	        (256)
#define SECTOR_SIZE		        (0x1000)

#define EraseChip				0
#define EraseBlock				1
#define EraseSector				2


#define FLASH_ADDR_BASE                 (0x18000000)

//user area
#define FLASH_DATA_USER_START           (0x18002000)
#define FLASH_DATA_USER_END             (0x1800FFFF)

//wlan area
#define FLASH_WLAN_START                (0x18010000)
#define FLASH_WLAN_SIZE                 (320*1024)

//parameter1
#define FLASH_PARAMETER_1               (0x18008000)
#define FLASH_PARAMETER_SIZE            (3*1024)

//flash mode area
#define	FLASH_MODE_AREA 			    (0x1800B000)
#define	FLASH_MODE_SIZE  			    (1*1024)


#if FLASH_SIZE == FLASH_1M

#define FLASH_OTA1                      (0x18060000)
#define FLASH_OTA1_SZ                   (320*1024)
        
#define FLASH_OTA2                      (0x180B0000)
#define FLASH_OTA2_SZ                   (320*1024)

#define  FLASH_END                      (0x180FFFFF)

#elif FLASH_SIZE == FLASH_2M

#define FLASH_OTA1                      (0x18060000)    
#define FLASH_OTA1_SZ                   (832*1024)
        
#define FLASH_OTA2                      (0x18130000)    
#define FLASH_OTA2_SZ                   (832*1024)

#define FLASH_END                       (0x181FFFFF)

#endif




/*
    para->
    len must small than SECTOR_SIZE
    addr must 4-bytes align
*/
void s907x_hal_flash_write(u32 addr, u8 *pbuf, int len);
/*
    para->
    addr must 4-bytes align
*/
void s907x_hal_flash_read(u32 addr, u8 *pbuf, int len);

//mode 
void s907x_hal_flash_erase(int erase_type, u32 addr);

void s907x_hal_flash_read_resume_tick(u32 addr, u8 *pbuf, int len);
void s907x_hal_flash_write_resume_tick(u32 addr, u8 *pbuf, int len);
void s907x_hal_flash_erase_resume_tick(int erase_type, u32 addr);

#endif
