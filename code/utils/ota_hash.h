#ifndef OTA_HASH_H
#define OTA_HASH_H

#include "system_config.h"

#define OTA_IMAGE_MD5_LEN          (16)
#define OTA_IMAGE_RESERVER_SIZE    (2)

#define OTA_BUF_VERIFY 512

#define AOS_SINGLE_TAG (0xabababab)
#define AOS_KERNEL_TAG (0xcdcdcdcd)
#define AOS_APP_TAG    (0xefefefef)

#define INIT_CTX_MAGIC(m)     (m = 0x12345678)
#define IS_VALID_CTX_MAGIC(m) (0x12345678 == m)
#define CLEAN_CTX_MAGIC(m)    (m = 0x0)

typedef enum {
    HASH_NONE   = 0,
    SHA256      = 3,
    MD5         = 6,
} OTA_HASH_E;
    
typedef enum {    
    OTA_PARAM_FAIL          = -17,
    OTA_UPGRADE_FAIL        = -13,
    OTA_VERIFY_HASH_FAIL    = -11,
    OTA_INIT                = 0,
}OTA_STATUS_E;
    
typedef enum {
    OTA_CRYPTO_ERROR = (int)0xffff0000,
    OTA_CRYPTO_NOSUPPORT,
    OTA_CRYPTO_INVALID_KEY,
    OTA_CRYPTO_INVALID_TYPE,
    OTA_CRYPTO_INVALID_CONTEXT,
    OTA_CRYPTO_INVALID_PADDING,
    OTA_CRYPTO_INVALID_AUTHENTICATION,
    OTA_CRYPTO_INVALID_ARG,
    OTA_CRYPTO_INVALID_PACKET,
    OTA_CRYPTO_LENGTH_ERR,
    OTA_CRYPTO_OUTOFMEM,
    OTA_CRYPTO_SHORT_BUFFER,
    OTA_CRYPTO_NULL,
    OTA_CRYPTO_ERR_STATE,
    OTA_CRYPTO_SUCCESS = 0,
} OTA_VERIFY_E;
    
typedef enum {
    OTA_CRYPTO_STATUS_CLEAN        = 0,
    OTA_CRYPTO_STATUS_INITIALIZED  = 1,
    OTA_CRYPTO_STATUS_PROCESSING   = 2,
    OTA_CRYPTO_STATUS_FINISHED     = 3,
} OTA_CRYPTO_STATUS_E;

typedef struct
{
    unsigned int   image_magic;
    unsigned int   image_size;
    unsigned char  image_md5_value[OTA_IMAGE_MD5_LEN];
    unsigned char  image_reserver[OTA_IMAGE_RESERVER_SIZE];
    unsigned short image_crc16;
} ota_image_t;

/*Verify API*/
typedef struct
{
    unsigned int total[2];
    unsigned int state[4];
    unsigned char buffer[64];
}ota_md5_context;
typedef struct {
    unsigned int total[2];
    unsigned int state[8];
    unsigned char buffer[64];
    int is224;
}ota_sha256_context;

typedef struct {
    unsigned int magic;
    unsigned int status;
    OTA_HASH_E type;
#if defined(__CC_ARM)
		#pragma anon_unions
#endif
    union {
        ota_md5_context md5_ctx;
        ota_sha256_context sha256_ctx;
    };
} ota_hash_ctx_t;


typedef struct
{
    OTA_HASH_E hash_method;
    int         ctx_size;
    void       *ctx_hash;
} ota_hash_param_t;



int ota_check_image(void* ota_header, unsigned int ota_addr, unsigned int size);

#endif
