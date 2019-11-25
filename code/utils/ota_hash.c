/*
* ota_hash.c synchronized from AOS.
*/
#include "s907x.h"
#include "system_debug.h"
#include "hal_flash.h"
#include "mbedtls/md5.h"
#include "mbedtls/sha256.h"
#include "ota_test.h"
#include "ota_hash.h"
#include "ota_md5.h"
                         
int ota_hash_get_ctx_size(OTA_HASH_E type, unsigned int *size)
{
    if (NULL == size) {
        return OTA_CRYPTO_INVALID_ARG;
    }
    switch(type) {
        case SHA256:
        case MD5:
            break;
        default:
            AT_DBG_ERR("invalid type(%d)", type);
            return OTA_CRYPTO_INVALID_TYPE;
    }
    *size = sizeof(ota_hash_ctx_t);
    return OTA_CRYPTO_SUCCESS;
} 
 
/* below is about check image*/
int ota_init_bin_md5_context(ota_hash_param_t *image_md5_ctx)
{
    image_md5_ctx->hash_method = MD5;
    ota_hash_get_ctx_size(MD5, (unsigned int*)&image_md5_ctx->ctx_size);
    if (image_md5_ctx->ctx_hash == NULL) {
        image_md5_ctx->ctx_hash = (void *)wl_malloc(image_md5_ctx->ctx_size);
    }
    if (image_md5_ctx->ctx_hash == NULL) {
        image_md5_ctx->hash_method = HASH_NONE;
        image_md5_ctx->ctx_size    = 0;
        return -1;
    }

    memset(image_md5_ctx->ctx_hash, 0, image_md5_ctx->ctx_size);
    return 0;
}

void ota_destroy_bin_md5_context(ota_hash_param_t *image_md5_ctx)
{
    if (image_md5_ctx->ctx_hash) {
        wl_free(image_md5_ctx->ctx_hash);
    }
    image_md5_ctx->ctx_hash    = NULL;
    image_md5_ctx->hash_method = HASH_NONE;
    image_md5_ctx->ctx_size    = 0;
}

void ota_sha256_free(ota_sha256_context *ctx)
{
    mbedtls_sha256_free((mbedtls_sha256_context*)ctx);
}

void ota_sha256_init(ota_sha256_context *ctx)
{
    mbedtls_sha256_init((mbedtls_sha256_context*)ctx);
}

void ota_sha256_starts(ota_sha256_context *ctx, int is224)
{
#if TUYA_BUILD
    mbedtls_sha256_starts_ret((mbedtls_sha256_context*)ctx, is224);
#else
    mbedtls_sha256_starts((mbedtls_sha256_context*)ctx, is224);
#endif
}

void ota_sha256_update(ota_sha256_context *ctx, const unsigned char *input, unsigned int ilen)
{
#if TUYA_BUILD
    mbedtls_sha256_update_ret((mbedtls_sha256_context*)ctx, input, ilen);
#else
    mbedtls_sha256_update((mbedtls_sha256_context*)ctx, input, ilen);
#endif
}

void ota_sha256_finish(ota_sha256_context *ctx, unsigned char output[32])
{
#if TUYA_BUILD
    mbedtls_sha256_finish_ret((mbedtls_sha256_context*)ctx, output);
#else
    mbedtls_sha256_finish((mbedtls_sha256_context*)ctx, output);
#endif
}

void ota_md5_free(ota_md5_context *ctx)
{
#if OTA_USING_MBEDTLS_MD5
    mbedtls_md5_free((mbedtls_md5_context *)ctx);
#else
    MD5Init((MD5_CTX *)ctx);
#endif
}

void ota_md5_init(ota_md5_context *ctx)
{
#if OTA_USING_MBEDTLS_MD5
    mbedtls_md5_init((mbedtls_md5_context *)ctx);
#else
    MD5Init((MD5_CTX *)ctx);
#endif
}

void ota_md5_starts(ota_md5_context *ctx)
{
#if OTA_USING_MBEDTLS_MD5
#if TUYA_BUILD
    mbedtls_md5_starts_ret((mbedtls_md5_context *)ctx);
#else
    
    mbedtls_md5_starts((mbedtls_md5_context *)ctx);
#endif
#else
    MD5Start((MD5_CTX *)ctx);
#endif
}

void ota_md5_update(ota_md5_context *ctx, const unsigned char *input, unsigned int ilen)
{
#if OTA_USING_MBEDTLS_MD5
#if TUYA_BUILD
    mbedtls_md5_update_ret((mbedtls_md5_context *)ctx, input, ilen);
#else
    mbedtls_md5_update((mbedtls_md5_context *)ctx, input, ilen);
#endif
#else
    MD5Update((MD5_CTX *)ctx, input, ilen);
#endif
}

void ota_md5_finish(ota_md5_context *ctx, unsigned char output[16])
{
#if OTA_USING_MBEDTLS_MD5
    #if TUYA_BUILD
        mbedtls_md5_finish_ret((mbedtls_md5_context *)ctx, output);
    #else
        mbedtls_md5_finish((mbedtls_md5_context *)ctx, output);
    #endif
#else
    MD5Final((MD5_CTX *)ctx, output);
#endif
}

int ota_hash_init(OTA_HASH_E type, void *context)
{
    ota_hash_ctx_t *hash_ctx;
    if (NULL == context) {
        return OTA_CRYPTO_INVALID_CONTEXT;
    }

    hash_ctx = (ota_hash_ctx_t *)context;
    if ((IS_VALID_CTX_MAGIC(hash_ctx->magic) &&
         hash_ctx->status != OTA_CRYPTO_STATUS_FINISHED) &&
         hash_ctx->status != OTA_CRYPTO_STATUS_CLEAN) {
         AT_DBG_ERR("bad:%d", (int)hash_ctx->status);
         return OTA_CRYPTO_ERR_STATE;
    }

    switch(type) {
        // SHA256 not test.
        case SHA256: {
            ota_sha256_init(&hash_ctx->sha256_ctx);
            ota_sha256_starts(&hash_ctx->sha256_ctx, 0);
            break;
        }
        // MD5 test OK.
        case MD5: {
            ota_md5_init(&hash_ctx->md5_ctx);
            ota_md5_starts(&hash_ctx->md5_ctx);
            break;
        }
        default:
            AT_DBG_ERR("invalid type:%d", type);
            return OTA_CRYPTO_INVALID_TYPE;
    }

    hash_ctx->type = type;
    hash_ctx->status = OTA_CRYPTO_STATUS_INITIALIZED;
    INIT_CTX_MAGIC(hash_ctx->magic);
    return OTA_CRYPTO_SUCCESS;
}

int ota_hash_update(const unsigned char *src, unsigned int size, void *context)
{
    ota_hash_ctx_t *hash_ctx;
    if (context == NULL) {
        return OTA_CRYPTO_INVALID_CONTEXT;
    }

    if (src == NULL && size != 0) {
        return OTA_CRYPTO_INVALID_ARG;
    }

    hash_ctx = (ota_hash_ctx_t *)context;
    if (!IS_VALID_CTX_MAGIC(hash_ctx->magic)) {
        return OTA_CRYPTO_INVALID_CONTEXT;
    }
    if ((hash_ctx->status != OTA_CRYPTO_STATUS_INITIALIZED) &&
        (hash_ctx->status != OTA_CRYPTO_STATUS_PROCESSING)) {
         AT_DBG_ERR("bad :%d", (int)hash_ctx->status);
         return OTA_CRYPTO_ERR_STATE;
    }
    switch(hash_ctx->type) {
        case SHA256: {
            ota_sha256_update(&hash_ctx->sha256_ctx,
                    (const unsigned char *)src, size);
            break;
        }
        case MD5: {
            ota_md5_update(&hash_ctx->md5_ctx,
                    (const unsigned char *)src, size);
            break;
        }
        default:
            AT_DBG_ERR("invalid:%d", hash_ctx->type);
            return OTA_CRYPTO_INVALID_TYPE;
    }

    hash_ctx->status = OTA_CRYPTO_STATUS_PROCESSING;
    return OTA_CRYPTO_SUCCESS;
}


int ota_hash_final(unsigned char *dgst, void *context)
{
    ota_hash_ctx_t *hash_ctx;
    if (context == NULL) {
        return OTA_CRYPTO_INVALID_CONTEXT;
    }
    if (dgst == NULL) {
        return OTA_CRYPTO_ERROR;
    }
    hash_ctx = (ota_hash_ctx_t *)context;
    if (!IS_VALID_CTX_MAGIC(hash_ctx->magic)) {
        return OTA_CRYPTO_INVALID_CONTEXT;
    }
    if ((hash_ctx->status != OTA_CRYPTO_STATUS_INITIALIZED) &&
        (hash_ctx->status != OTA_CRYPTO_STATUS_PROCESSING)) {
         AT_DBG_ERR("bad :%d", (int)hash_ctx->status);
         return OTA_CRYPTO_ERR_STATE;
    }
    switch(hash_ctx->type) {
        case SHA256: {
            ota_sha256_finish(&hash_ctx->sha256_ctx, (unsigned char *)dgst);
            ota_sha256_free(&hash_ctx->sha256_ctx);
            break;
        }
        case MD5: {
            ota_md5_finish(&hash_ctx->md5_ctx, (unsigned char *)dgst);
            ota_md5_free(&hash_ctx->md5_ctx);
            break;
        }
        default:
            AT_DBG_ERR("invalid :%d", hash_ctx->type);
            return OTA_CRYPTO_INVALID_TYPE;
    } 

    CLEAN_CTX_MAGIC(hash_ctx->magic);
    hash_ctx->status = OTA_CRYPTO_STATUS_FINISHED;
    return OTA_CRYPTO_SUCCESS;
}

static int ota_check_md5(const unsigned char *cur_hash, const char *download_hash)
{
    if (cur_hash == NULL || download_hash == NULL) {
        return -1;
    }
    char digest_str[33] = {0};
    int  i = 0;
    for (; i < 16; i++) {
        snprintf(digest_str + i * 2, 2 + 1, "%02X", cur_hash[i]);
    }
    AT_DBG_MSG("md5 src=%s dst=%s", download_hash, digest_str);
    if (strncmp(digest_str, download_hash, 32)) {
        return -1;
    }
    return 0;
}

int ota_hal_read(const int off_set, char *out_buf, int out_buf_len)
{
    if (off_set % 4) {
        return -1;
    }
    s907x_hal_flash_read(FLASH_OTA2+off_set, (u8 *)out_buf, out_buf_len);

    return 0;
}

int ota_check_image(void *ota_header, unsigned int ota_addr, unsigned int size)
{
    int ret = 0;

    int i = 0;
    int off_set = 0;
    char image_md5_value[33] = {0};
    char download_md5_str_value[33] = {0};
    unsigned int read_size = 0;
    char *rd_buf = NULL;
    char test_buf[33] = { 0 };
    int bin_size = 0;
    ota_image_t ota_image_identity = { 0};
    ota_hash_param_t image_md5_ctx = { HASH_NONE, 0, NULL };
    ota_image_header_item_t *pst_ota_hdr = NULL;
    ota_image_header_item_t *pst_wlan_hdr = NULL;
    unsigned int wlan_addr = 0;

    if(size <= sizeof(ota_image_t)){
        ret = OTA_VERIFY_HASH_FAIL;
        return ret;
    }

    /* 1. Read ota image tail(ota_imaget_t) from flash. */
#if CONFIG_OTA_DUAL_IMAGES
    pst_wlan_hdr = (ota_image_header_item_t *)((char *)ota_header + OTA_WLAN_OFFSET);
    pst_ota_hdr = (ota_image_header_item_t *)((char *)ota_header + OTA_OTA1_OFFSET);
    bin_size = pst_wlan_hdr->len + pst_ota_hdr->len + sizeof(ota_image_t);
#else
    bin_size = size;
#endif
    off_set = bin_size - sizeof(ota_image_t);
    USER_DBG("bin s:%d off:%d", bin_size, off_set);
    s907x_hal_flash_read(ota_addr + off_set, (char*)&ota_image_identity, sizeof(ota_image_t));
    memset(test_buf, 0x00, sizeof(test_buf));
    for (i = 0; i < 16; i++) {
        snprintf((char *)(test_buf + i * 2), 2 + 1, "%02X", ota_image_identity.image_md5_value[i]);
    }
    AT_DBG_MSG("magic:0x%04x size:%d md5:%s crc16:0x%02x", 
            ota_image_identity.image_magic,
            ota_image_identity.image_size, 
            test_buf, 
            ota_image_identity.image_crc16);
    if ((ota_image_identity.image_magic != AOS_SINGLE_TAG) &&
        (ota_image_identity.image_magic != AOS_KERNEL_TAG) &&
        (ota_image_identity.image_magic != AOS_APP_TAG)) {
        ret = OTA_PARAM_FAIL;
        return ret;
    }
        
    /* 2. Init MD5 information. */
#if CONFIG_OTA_DUAL_IMAGES
    bin_size = ota_image_identity.image_size - OTA_ENCAP_HEADER_LEN;
#else
    bin_size = ota_image_identity.image_size;
#endif
    rd_buf   = wl_malloc(OTA_BUF_VERIFY);
    if (rd_buf == NULL) {
        ret = OTA_PARAM_FAIL;
        return ret;
    }
    if (ota_init_bin_md5_context(&image_md5_ctx) < 0) {
        ret = OTA_PARAM_FAIL;
        goto err;
    }
    ret = ota_hash_init(image_md5_ctx.hash_method, image_md5_ctx.ctx_hash);
    if (ret < 0) {
        AT_DBG_MSG("hash init.\n");
        goto err;
    }

    /* 3. Calcalute the MD5 for image restored in flash. */
#if CONFIG_OTA_DUAL_IMAGES
    ret = ota_hash_update((const u8 *)ota_header, OTA_ENCAP_HEADER_LEN, image_md5_ctx.ctx_hash);
    if (ret < 0) {
        AT_DBG_ERR("hash update err.\n");
        goto err;
    }
#endif
    off_set = 0;
    while (off_set < bin_size) {
        read_size = (bin_size - off_set >= OTA_BUF_VERIFY) ? (OTA_BUF_VERIFY) : (bin_size - off_set);
        s907x_hal_flash_read(ota_addr + off_set, rd_buf, read_size);
        off_set += read_size;
        ret = ota_hash_update((const u8 *)rd_buf, read_size, image_md5_ctx.ctx_hash);
        if (ret < 0) {
            AT_DBG_ERR("hash update err.\n");
            goto err;
        }
    }

    /* 4. Compare the MD5 calculated to MD5 received in ota_imaget_t. */
    memset(image_md5_value, 0x00, sizeof(image_md5_value));
    ret = ota_hash_final((unsigned char *)image_md5_value, image_md5_ctx.ctx_hash);
    if (ret < 0) {
        AT_DBG_ERR("hash final err.\n");
        goto err;
    }    
    memset(download_md5_str_value, 0x00, sizeof(download_md5_str_value));
    for (i = 0; i < 16; i++) {
        snprintf((char *)download_md5_str_value + i * 2, 2 + 1, "%02X", ota_image_identity.image_md5_value[i]);
    }
    ret = ota_check_md5((const unsigned char *)image_md5_value, (const char *)&download_md5_str_value);
    if (ret < 0) {
        AT_DBG_ERR("hash check err.\n");
        goto err;
    }
err:
    AT_DBG_MSG("OTA md5 ret:%d",ret);
    if(rd_buf) {
       wl_free(rd_buf);
       rd_buf = NULL;
    }
    ota_destroy_bin_md5_context(&image_md5_ctx);

    return ret;
}
