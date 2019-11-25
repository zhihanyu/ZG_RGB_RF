#ifndef OTA_TEST_H
#define OTA_TEST_H


#define OTA_TEST_SHOW_CONFIG    "show_cfg"
#define OTA_TEST_START          "start"
#define OTA_TEST_DEBUG_MSG      "debug_msg"

#define OTA_TEST_OTA_NAME_DEF   "ota1.bin"

#define OTA_TEST_DOWNLOAD_OK    "Firmware download success, reboot\r\n"

#define OTA_START_TO_UPDATE     "OTA procedure begin...\r\n"
#define OTA_START_HTTP_DOWNLOAD "Start to download via http.\r\n"
#define OTA_WRITING_TO_FLASH    "Writing %d bytes to flash.\r\n"

#define OTA_DUAL_SIGNATURE      0x9070

#define OTA_TEST_URL_LEN_MAX    64
#define OTA_TEST_BIN_NAME_LEN   32
#define OTA_TEST_BIN_PATH_LEN   (OTA_TEST_URL_LEN_MAX + OTA_TEST_BIN_NAME_LEN)
#define OTA_TEST_DATA_BUFF_LEN  2048

#define OTA_ENCAP_HEADER_LEN    128
#define OTA_IMAGE_SIGNATURE_LEN 8
#define OTA_IMAGE_MD5_HASH_LEN  32


// OTA config setting
#define OTA_CONFIG_FW_MD5           1
#define OTA_CONFIG_FW_SHA256        0

#define OTA_USING_MBEDTLS_MD5       1


#define OTA_BOOT_OFFSET     4
#define OTA_WLAN_OFFSET     OTA_BOOT_OFFSET + 8
#define OTA_OTA1_OFFSET     OTA_WLAN_OFFSET + 8
typedef struct
{
    u32 len ;
    u32 addr;
}ota_image_header_item_t;

typedef struct ota_image_encap_header_ {
    char data[OTA_ENCAP_HEADER_LEN];
    int len;
}ota_image_encap_header_t;

#if CONFIG_OTA_DUAL_IMAGES
#define OTA_FLASH_BIN_NUM_MAX       3
#define OTA_FLASH_BIN_OTA_SIZE      0x50000
#define OTA_FLASH_BIN_WLAN_SIZE     0x50000
#else
#define OTA_FLASH_BIN_NUM_MAX       1
#endif


void ota_test(void *context);

#if CONFIG_OTA_DUAL_IMAGES
void ota_init_boot_params(void);
#endif

#endif
