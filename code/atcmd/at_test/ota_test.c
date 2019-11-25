#include "s907x.h"
#include "system_types.h"
#include "string.h"
#include "lwip_conf.h" 
#include "ota_test.h"
#include "at_cmd.h"
#include "httpc_api.h"
#include "ota_hash.h"
           
#if M_AT_TEST

#define OTA_FW_DB_MSG           "OTA_dual_ota2_firmware_20190917_1547.bin"

typedef enum {
    OTA_INITIALIZED = 0,
    OTA_FINISH,
    OTA_BREAKPOINT
} OTA_RES_TYPE_E;

#if CONFIG_OTA_DUAL_IMAGES
typedef enum {
    OTA_FLASH_BIN_BOOT_IDX  = 0,
    OTA_FLASH_BIN_WLAN_IDX  = 1,
    OTA_FLASH_BIN_OTA_IDX   = 2,
}OTA_FLASH_BIN_IDX_E;

typedef enum {
    OTA_BOOT_PARAM_SIG_OTA_BIN = 2,
    OTA_BOOT_PARAM_SIG_OTA_WLAN_BIN = 3,
}OTA_BOOT_PARAM_SIG_TYPE_E;

typedef struct sig_
{
    union  {
        struct sig_bit
        {
            u64 version:16;    //  0x0101
            u64 type:4;        //  0:boot 1:ota1 2:wlan
            u64 date:24;        //  0x181231 y-m-d
            u64 author_id:4;   //  id  
            u64 sig:16;        //  0x9070
        }bits;
        u64 val;
    }sig;
}sig_t;

#if defined (__ICCARM__)
typedef __packed struct {
    unsigned int    upg_addr;
    unsigned int    orig_addr;
    unsigned int    boot_addr;
    sig_t           signature;
    unsigned char   MD5[OTA_IMAGE_MD5_HASH_LEN];
    unsigned int    ota_len;
    unsigned int    wlan_len;
}ota_boot_param_t;

#elif defined ( __GNUC__ )
#pragma pack(1)
typedef  struct {
    unsigned int    upg_addr;
    unsigned int    orig_addr;
    unsigned int    boot_addr;
    sig_t           signature;
    unsigned char   MD5[OTA_IMAGE_MD5_HASH_LEN];
    unsigned int    ota_len;
    unsigned int    wlan_len;
}ota_boot_param_t;
#pragma pack(0)

#elif defined ( __CC_ARM )
#pragma pack(push, 1)
typedef  struct {
    unsigned int    upg_addr;
    unsigned int    orig_addr;
    unsigned int    boot_addr;
    sig_t           signature;
    unsigned char   MD5[OTA_IMAGE_MD5_HASH_LEN];
    unsigned int    ota_len;
    unsigned int    wlan_len;
}ota_boot_param_t;
#pragma pack(pop)
#endif

#else
typedef enum {
    OTA_FLASH_BIN_OTA_IDX   = 0,
}OTA_FLASH_BIN_IDX_E;

typedef __packed struct  {
    unsigned int    dst_adr;
    unsigned int    src_adr;
    unsigned int    len;
    unsigned short  crc;
    unsigned int    upg_flag;
    unsigned char   boot_count;
    unsigned int    rec_size;
    unsigned int    splict_size;
    int             off_bp;         /*Break point offset*/
    OTA_RES_TYPE_E  res_type;       /*result type: OTA_FINISH, OTA_BREAKPOINT*/
    unsigned short  param_crc;      /*Parameter crc*/
} ota_boot_param_t;
#endif

typedef struct ota_flash_partition_ {
    int addr;
    int offset;
    int block_size;
}ota_flash_partition_t;

ota_flash_partition_t g_st_ota_flash = {0};

static char g_ac_ota_bin_url[OTA_TEST_URL_LEN_MAX] = {0};
static char g_ac_ota_bin_name[OTA_TEST_BIN_NAME_LEN] = OTA_TEST_OTA_NAME_DEF;


static int ota_show_config(int argc, char **argv)
{
    if(argc != 1) {
        AT_DBG_ERR("Usage: AT+OTA=\"%s\"", OTA_TEST_SHOW_CONFIG);
        return AT_RET_ERR;
    }

    at_rsp("ota bin url[%s], name[%s].\r\n", g_ac_ota_bin_url, g_ac_ota_bin_name);
    return AT_RET_OK;
}

static void ota_start_write_flash()
{
    uint32 bin_idx = 0;
    uint32 sec_idx = 0;
    uint32 max_idx = 0;
    uint32 flash_addr = 0;

    max_idx = g_st_ota_flash.block_size / SECTOR_SIZE;

    for (sec_idx = 0; sec_idx < max_idx; sec_idx++)
    {
        flash_addr = g_st_ota_flash.addr + sec_idx * SECTOR_SIZE;
        s907x_hal_flash_erase(EraseSector, flash_addr);
    }
}

static int ota_write_flash(ota_flash_partition_t * pst_ota_flash, char *flash_buf, char *data, int data_len)
{
    int flash_addr = 0;
    int offset = 0;
    int wr_len = 0;

    if (pst_ota_flash->block_size < pst_ota_flash->offset + data_len) {
        AT_DBG_ERR("Overflow, current ota bin size[%x] over maxsize[%x]",
            pst_ota_flash->offset + data_len, pst_ota_flash->block_size);
        return AT_RET_ERR;
    }

    offset = pst_ota_flash->offset % SECTOR_SIZE;
    wr_len = SECTOR_SIZE - offset;
    // Write to flash every SECTOR_SIZE bytes
    if (wr_len <= data_len) {
        memcpy(flash_buf + offset, data, wr_len);
        flash_addr = pst_ota_flash->addr + pst_ota_flash->offset - offset;
        at_rsp(OTA_WRITING_TO_FLASH, SECTOR_SIZE);
        s907x_hal_flash_write(flash_addr, (u8 *)flash_buf, SECTOR_SIZE);

        memset(flash_buf, 0, SECTOR_SIZE);
        memcpy(flash_buf, data + wr_len, data_len - wr_len);
        pst_ota_flash->offset += data_len;
    }
    else {
        memcpy(flash_buf+offset, data, data_len);
        pst_ota_flash->offset += data_len;
    }
    
    return AT_RET_OK;
}

static void ota_finish_write_flash(char *flash_buf, ota_flash_partition_t *pst_ota_flash)
{
    int flash_addr = 0;
    int offset = 0;
    int wr_len = 0;

    offset = pst_ota_flash->offset;
    wr_len = offset % SECTOR_SIZE;
    if (0 != wr_len) {
        flash_addr = pst_ota_flash->addr + offset - wr_len;
        s907x_hal_flash_write(flash_addr, (u8 *)flash_buf, SECTOR_SIZE);
        AT_DBG_MSG("Receive data, [%d:0x%02X]", wr_len, wr_len);
        at_rsp(OTA_WRITING_TO_FLASH, wr_len);
        offset = offset - wr_len + SECTOR_SIZE;
    }
    else {
        AT_DBG_MSG("Receive data, [%d:0x%02X]", offset, offset);
    }

    memset(flash_buf, 0, SECTOR_SIZE);
    while (offset < pst_ota_flash->block_size)
    {
        flash_addr = pst_ota_flash->addr + offset;
        s907x_hal_flash_write(flash_addr, (u8 *)flash_buf, SECTOR_SIZE);
        offset += SECTOR_SIZE;
    }
}

#if CONFIG_OTA_DUAL_IMAGES
static int ota_skip_and_get_ota_header(char **in_data, int *in_data_len, void *in_ota_header)
{
    int ret = AT_RET_OK;
    int bin_size = 0;
    int offset = 0;
    int wr_len = 0;
    int data_len = *in_data_len;
    char *data = *in_data;
    ota_image_encap_header_t *ota_header = in_ota_header;
    int header_left = OTA_ENCAP_HEADER_LEN - ota_header->len;
    ota_image_header_item_t *pst_wlan_hdr = NULL;
    ota_image_header_item_t *pst_ota_hdr = NULL;
    static unsigned int s_cur_skipped_len;
    int should_skip_len;

    if (ota_header->len < OTA_ENCAP_HEADER_LEN) {
        if (data_len <= header_left) {
            memcpy(ota_header->data + ota_header->len, data, data_len);
            ota_header->len += data_len;
            *in_data_len = 0;
            return AT_RET_OK;
        }
        else {
            memcpy(ota_header->data + ota_header->len, data, header_left);
            ota_header->len += header_left;
            data_len -= header_left;
            data += header_left;
        }
    }

    pst_wlan_hdr = (ota_image_header_item_t *)((char *)ota_header + OTA_WLAN_OFFSET);
    pst_ota_hdr = (ota_image_header_item_t *)((char *)ota_header + OTA_OTA1_OFFSET);
    bin_size = pst_wlan_hdr->len + pst_ota_hdr->len + sizeof(ota_image_t);

    if (g_st_ota_flash.block_size < bin_size) {
        AT_DBG_ERR("Insufficient storage space in flash,wlan bin size[%x] ota + tail size[%x] ota partition max size[%x]",
            pst_wlan_hdr->len , pst_ota_hdr->len + sizeof(ota_image_t), g_st_ota_flash.block_size);
        return AT_RET_ERR;
    }

    if(g_st_ota_flash.addr != pst_ota_hdr->addr) {
        should_skip_len = bin_size;
        if((s_cur_skipped_len + data_len) > should_skip_len){

            offset = should_skip_len - s_cur_skipped_len;
            if(data_len - offset > OTA_ENCAP_HEADER_LEN){
                memset(ota_header, 0, sizeof(ota_image_encap_header_t));
                
                ota_header->len = OTA_ENCAP_HEADER_LEN;
                memcpy(ota_header->data, data + offset, OTA_ENCAP_HEADER_LEN);
                
                pst_wlan_hdr = (ota_image_header_item_t *)((char *)ota_header + OTA_WLAN_OFFSET);
                pst_ota_hdr = (ota_image_header_item_t *)((char *)ota_header + OTA_OTA1_OFFSET);
                
                AT_DBG_MSG("ota header addr = %x  len = %x", pst_ota_hdr->addr , pst_ota_hdr->len);
                AT_DBG_MSG("wlan header addr = %x  len = %x", pst_wlan_hdr->addr, pst_wlan_hdr->len);
                
                s_cur_skipped_len += offset; 
                data_len -= offset + OTA_ENCAP_HEADER_LEN;
                data += offset + OTA_ENCAP_HEADER_LEN;
                
                AT_DBG_MSG("should_skip_len = %x ota_discard_length = %x ", should_skip_len, s_cur_skipped_len);
                AT_DBG_MSG("OTA1 discard finish OTA2 set header and write");
            }else {
                memset(ota_header->data, 0 , OTA_ENCAP_HEADER_LEN);
                
                offset = should_skip_len - s_cur_skipped_len;
                ota_header->len = data_len - offset;
                memcpy(ota_header->data , data + offset, ota_header->len);
                
                s_cur_skipped_len += offset;
                data_len = 0;
                AT_DBG_MSG("OTA1 discard finsh OTA2 set head--1");
            }
        }else if(((s_cur_skipped_len + data_len) == should_skip_len)){
            memset(ota_header->data, 0 , OTA_ENCAP_HEADER_LEN);
            
            s_cur_skipped_len += data_len;
            data_len = 0;
            
            AT_DBG_MSG("OTA1 discard finsh OTA2 set head--2");
        }else {
            s_cur_skipped_len += data_len;
            data_len = 0;
            AT_DBG_MSG("ota1 discard length = %x  target length = %x", s_cur_skipped_len, should_skip_len);
        } 
                    
    }

    *in_data_len = data_len;
    *in_data = data;
    return ret;
}

static int ota_write_flash_dual(char *flash_buf, char *in_data, int in_data_len, void *in_ota_header)
{
    int ret = AT_RET_OK;
    int bin_size = 0;
    int offset = 0;
    int wr_len = 0;
    int data_len = in_data_len;
    char *data = in_data;
    ota_image_encap_header_t *ota_header = in_ota_header;
    int header_left = OTA_ENCAP_HEADER_LEN - ota_header->len;
    
    ota_image_header_item_t *pst_ota_hdr = NULL;
    ota_image_header_item_t *pst_wlan_hdr = NULL;

    ret = ota_skip_and_get_ota_header(&data, &data_len, ota_header);
    if ((AT_RET_OK != ret) || (0 == data_len)) {
        return ret;
    }
    
    pst_wlan_hdr = (ota_image_header_item_t *)((char *)ota_header + OTA_WLAN_OFFSET);
    pst_ota_hdr = (ota_image_header_item_t *)((char *)ota_header + OTA_OTA1_OFFSET);
    bin_size = pst_wlan_hdr->len + pst_ota_hdr->len + sizeof(ota_image_t);

    if (0 != pst_ota_hdr->len  && (g_st_ota_flash.offset < bin_size)) { 
        wr_len = data_len;
        if (bin_size < g_st_ota_flash.offset  + wr_len) {
            wr_len = bin_size - g_st_ota_flash.offset;
        }

        ret = ota_write_flash(&g_st_ota_flash, flash_buf, data, wr_len);
        if (AT_RET_ERR == ret) {
            AT_DBG_ERR("Write image to flash fail.");
            return ret;
        }

        if (g_st_ota_flash.offset == bin_size) {
            ota_finish_write_flash(flash_buf, &g_st_ota_flash);
            AT_DBG_MSG("data_len[%d], \r\n\t wlan + ota : hdr len[%d:0x%x], write data len[%d:0x%x]",
                data_len, bin_size, bin_size, g_st_ota_flash.offset, g_st_ota_flash.offset);
        }

        data_len -= wr_len;
        data += wr_len;

        if (0 == data_len) {
            return ret;
        }
    }          


    return ret;
}
#else
static int ota_write_flash_single(char *flash_buf, char *data, int data_len, void *in_ota_header)
{
    ota_image_encap_header_t *ota_header = in_ota_header;
    int ret = AT_RET_OK;
    int bin_size = 0;
    int header_left = OTA_ENCAP_HEADER_LEN - ota_header->len;
    char *pc_flash = NULL;
    ota_flash_partition_t *pst_ota_flash = &g_st_ota_flash;
    ota_image_header_item_t *pst_wlan_hdr = NULL;
    ota_image_header_item_t *pst_ota_hdr = NULL;
    
    if (ota_header->len < OTA_ENCAP_HEADER_LEN) {
        if (data_len <= header_left) {
            memcpy(ota_header->data + ota_header->len, data, data_len);
            ota_header->len += data_len;
        }
        else {
            memcpy(ota_header->data + ota_header->len, data, header_left);
            ota_header->len += header_left;
        }
    }
    
    ret = ota_write_flash(pst_ota_flash, flash_buf, data, data_len);
    
    header_left = OTA_ENCAP_HEADER_LEN - ota_header->len;
    if (0 == header_left) {
        pc_flash = (char *)pst_ota_flash;
        pst_wlan_hdr = (ota_image_header_item_t *)(pc_flash + OTA_WLAN_OFFSET);
        pst_ota_hdr = (ota_image_header_item_t *)(pc_flash + OTA_OTA1_OFFSET);
        bin_size = pst_wlan_hdr->len + pst_ota_hdr->len + OTA_ENCAP_HEADER_LEN + OTA_ENCAP_HEADER_LEN;
        if (bin_size == pst_ota_flash->offset) {
            ota_finish_write_flash(flash_buf, pst_ota_flash);
        }
        else if (bin_size < pst_ota_flash->offset) {
            AT_DBG_ERR("Download images length exceeds expected.");
            ret = AT_RET_ERR;
        }
    }
    
    return ret;
    
}
#endif
static void ota_init_flash_params(void)
{
#if CONFIG_OTA_DUAL_IMAGES
    ota_boot_param_t ota_param;
    int base_addr = 0;
    int block_size = 0;

    flash_read(FLASH_PARAMETER_1, (u8 *)&ota_param, sizeof(ota_param));
    if (FLASH_OTA1 - FLASH_ADDR_BASE == ota_param.orig_addr) {
        base_addr = FLASH_OTA2;
        block_size = FLASH_OTA2_SZ;
    }
    else if (FLASH_OTA2 - FLASH_ADDR_BASE == ota_param.orig_addr) {
        base_addr = FLASH_OTA1;
        block_size = FLASH_OTA1_SZ;
    }
    else {
        AT_DBG_ERR("Unknown orig addr[%08x], to init the boot param.", ota_param.orig_addr);
        memset((u8 *)&ota_param, 0xFF, sizeof(ota_param));
        ota_param.orig_addr = FLASH_OTA1 - FLASH_ADDR_BASE;
        flash_write(FLASH_PARAMETER_1, (u8 *)&ota_param, sizeof(ota_param));
        
        base_addr = FLASH_OTA2;
        block_size = FLASH_OTA2_SZ;
    }
    g_st_ota_flash.addr = base_addr;
    g_st_ota_flash.offset = 0;
    g_st_ota_flash.block_size = block_size;
#else
    g_st_ota_flash.addr = FLASH_OTA2;
    g_st_ota_flash.offset = 0;
    g_st_ota_flash.block_size = FLASH_OTA2_SZ;
#endif
}

static int ota_write_boot_params(void *ota_header, int rx_bin_len)
{
    ota_boot_param_t ota_param, ota_param_r;

#if CONFIG_OTA_DUAL_IMAGES
    ota_image_header_item_t *pst_wlan_hdr = NULL;
    ota_image_header_item_t *pst_ota_hdr = NULL;

    memset(&ota_param, 0xFF, sizeof(ota_param));
    memset(&ota_param_r, 0xFF, sizeof(ota_param_r));

    pst_wlan_hdr = (ota_image_header_item_t *)((char *)ota_header + OTA_WLAN_OFFSET);
    pst_ota_hdr = (ota_image_header_item_t *)((char *)ota_header + OTA_OTA1_OFFSET);
   
    flash_read(FLASH_PARAMETER_1, (u8 *)&ota_param, sizeof(ota_param));
    ota_param.upg_addr = g_st_ota_flash.addr - FLASH_ADDR_BASE;
    ota_param.signature.sig.bits.version = 0x0101;
    ota_param.signature.sig.bits.type = OTA_BOOT_PARAM_SIG_OTA_BIN;
    ota_param.signature.sig.bits.date = 0x190812;
    ota_param.signature.sig.bits.author_id = 0;
    ota_param.signature.sig.bits.sig = OTA_DUAL_SIGNATURE;

    ota_param.wlan_len = 0xFFFFFFFF;
    ota_param.ota_len = 0xFFFFFFFF;
    if (0 == pst_ota_hdr->len) {
        return AT_RET_ERR;
    }
    ota_param.ota_len = pst_ota_hdr->len;
    
    if (0 != pst_wlan_hdr->len) {
        ota_param.wlan_len = pst_wlan_hdr->len;
    }
#else
    memset(&ota_param, 0, sizeof(ota_param));
    memset(&ota_param_r, 0, sizeof(ota_param_r));
   
    // boot parameter used in bootloader.
    // Following parameters value is needed to upgrade firmware.
    // src_adr: FLASH_OTA2 address in bootloader [0x00130000]
    ota_param.src_adr = FLASH_OTA2 - 0x18000000;
    // res_type: not 0.
    ota_param.res_type = OTA_FINISH;
    // len: not 0.
    ota_param.len = rx_bin_len;
#endif

    flash_write(FLASH_PARAMETER_1, (u8 *)&ota_param, sizeof(ota_param));
    flash_read(FLASH_PARAMETER_1, (u8 *)&ota_param_r, sizeof(ota_param_r));
    return (!memcmp(&ota_param, &ota_param_r, sizeof(ota_param)) ? AT_RET_OK : AT_RET_ERR);
}

static int ota_http_init_params(void **out_flash_buf, void **out_rsp_buf, void **out_httpc_cli)
{
    int ret = AT_RET_OK;
    void *flash_buf = NULL;
    void *rsp_buf = NULL;
    void *httpc_cli = NULL;
    
    flash_buf = http_malloc(SECTOR_SIZE);
    rsp_buf = http_malloc(OTA_TEST_DATA_BUFF_LEN);
    if ((NULL == flash_buf) || (NULL == rsp_buf)) {
        AT_DBG_ERR("Malloc http data buffer fail, flash buff[%x], response buff[%x].",
            flash_buf, rsp_buf);
        ret = AT_RET_ERR;
        goto exit;
    }
    httpc_cli = httpc_client_new();
    if (NULL == httpc_cli) {
        AT_DBG_ERR("Create new http client fail.");
        ret = AT_RET_ERR;
        goto exit;
    }
    memset(rsp_buf, 0, OTA_TEST_DATA_BUFF_LEN);
    httpc_set_response_buf(httpc_cli, rsp_buf, OTA_TEST_DATA_BUFF_LEN);

exit:
    *out_flash_buf = flash_buf;
    *out_rsp_buf = rsp_buf;
    *out_httpc_cli = httpc_cli;

    return ret;
}

static void ota_http_deinit_params(void *flash_buf, void *rsp_buf, void *httpc_cli)
{
    if (httpc_cli) {
        httpc_client_free(httpc_cli);
    }
    if (rsp_buf) {
        http_free(rsp_buf);
    }
    if (flash_buf) {
        http_free(flash_buf);
    }
}

static int ota_http_request_new_fw(void *httpc_cli)
{
    int httpc_ret = HTTPC_OK;
    char ota_bin_path[OTA_TEST_BIN_PATH_LEN] = "0";
    
    snprintf(ota_bin_path, OTA_TEST_BIN_PATH_LEN, "%s/%s", g_ac_ota_bin_url, g_ac_ota_bin_name);
    httpc_ret = httpc_connect(ota_bin_path, httpc_cli);
    if (HTTPC_OK != httpc_ret) {
        AT_DBG_ERR("Connect ota url fail.");
        return AT_RET_ERR;
    }

    httpc_ret = httpc_send_request(ota_bin_path, HTTPC_GET, httpc_cli);
    if (HTTPC_OK != httpc_ret) {
        AT_DBG_ERR("Send http request fail.");
        return AT_RET_ERR;
    }

    return AT_RET_OK;
}

static int ota_http_download_new_fw(char *flash_buf, char *rsp_buf, void *httpc_cli, void *ota_header)
{
    int ret = AT_RET_ERR;
    int httpc_ret = HTTPC_OK;
    int response_code = 0;
    int cur_data_len = 0;
    int cur_retrieve_len = 0;
    int last_retrieve_len = 0;
    int rsp_content_len = 0;
    int total_recv_len = 0;
    int out_recv_len = -1;

    at_rsp(OTA_START_HTTP_DOWNLOAD);
    
    ota_start_write_flash();
    do {
        httpc_ret = httpc_recv_response(httpc_cli);
        response_code = httpc_get_response_code(httpc_cli);
        if ((200 != response_code) ||
            ((HTTPC_OK != httpc_ret) && (HTTPC_MORE_DATA != httpc_ret))) {
            AT_DBG_ERR("httpc receive response fail, response code[%d], ret[%d]",
                response_code, httpc_ret);
            return out_recv_len;
        }

        if (0 == last_retrieve_len) {
            httpc_get_response_len(httpc_cli, HTTPC_RSP_CONTENT_LEN, &rsp_content_len);
            last_retrieve_len = rsp_content_len;
        }

        httpc_get_response_len(httpc_cli, HTTPC_RETRIEVE_LEN, &cur_retrieve_len);
        cur_data_len = last_retrieve_len - cur_retrieve_len;
        total_recv_len += cur_data_len;
        last_retrieve_len = cur_retrieve_len;

#if CONFIG_OTA_DUAL_IMAGES
        ret = ota_write_flash_dual(flash_buf, rsp_buf, cur_data_len, ota_header);
#else
        ret = ota_write_flash_single(flash_buf, rsp_buf, cur_data_len, ota_header);
#endif
        
    }while (HTTPC_MORE_DATA == httpc_ret);

    AT_DBG_MSG("Receive all the bin data. response content len[%d], received data len[%d].",
        rsp_content_len, total_recv_len);
    // Check the total data len equal to content len in response.
    if ((AT_RET_OK != ret) || (rsp_content_len != total_recv_len)) {
        AT_DBG_ERR("Download ota bin through http fail.");
        return out_recv_len;
    }

    out_recv_len = total_recv_len;
    return out_recv_len;
    
}

static int ota_http_update_image(void)
{
    int ret = AT_RET_ERR;
    int httpc_ret = HTTPC_OK;
    int received_len = 0;
    char *rsp_buf = NULL;
    char *flash_buf = NULL;
    void *httpc_cli = NULL;
    ota_image_encap_header_t ota_header;
    memset(&ota_header, 0, sizeof(ota_image_encap_header_t));

    ota_init_flash_params();
    at_rsp(OTA_START_TO_UPDATE);

    httpc_ret = ota_http_init_params((void *)&flash_buf, (void *)&rsp_buf, &httpc_cli);
    if (HTTPC_OK != httpc_ret) {
        AT_DBG_ERR("Init http parameters fail.");
        ret = AT_RET_ERR;
        goto exit;
    }

    httpc_ret = ota_http_request_new_fw(httpc_cli);
    if (HTTPC_OK != httpc_ret) {
        AT_DBG_ERR("Request new firmware fail.");
        ret = AT_RET_ERR;
        goto exit;
    }
    
    received_len = ota_http_download_new_fw(flash_buf, rsp_buf, httpc_cli, (void *)&ota_header);
    
    if (received_len <= 0) {
        AT_DBG_ERR("Http download new firmware fail.");
        ret = AT_RET_ERR;
        goto exit;
    }
    
    // Calculate the crc value(crc of received data) equal to crc value in ota bin(the last 4bytes).
#if CONFIG_OTA_DUAL_IMAGES
    httpc_ret = ota_check_image((void *)&ota_header, g_st_ota_flash.addr, received_len);
#else
    httpc_ret = ota_check_image(NULL, g_st_ota_flash.addr, received_len);
#endif
    if (httpc_ret < 0) {
        AT_DBG_ERR("Image check fail, ret[%d],", httpc_ret);
        ret = AT_RET_ERR;
        goto exit;
    }

    ret = ota_write_boot_params((void *)&ota_header, received_len);
    if (AT_RET_OK != ret) {
        AT_DBG_ERR("Write boot param fail.");
        ret = AT_RET_ERR;
        goto exit;
    }

exit:
    ota_http_deinit_params(flash_buf, rsp_buf, httpc_cli);

    return ret;    
}

static int ota_start(int argc, char **argv)
{
    int ret = AT_RET_ERR;
    if((argc < 2) || (3 < argc)) {
        at_rsp(AT_ERROR);
        return ret;
    }
    
    at_parse_param_str(&argv[1]);
    memset(g_ac_ota_bin_url, 0, OTA_TEST_URL_LEN_MAX);
    memcpy(g_ac_ota_bin_url, argv[1], strlen(argv[1]));

    if (argv[2]) {
        at_parse_param_str(&argv[2]);
        memset(g_ac_ota_bin_name, 0, OTA_TEST_BIN_NAME_LEN);
        memcpy(g_ac_ota_bin_name, argv[2], strlen(argv[2]));
    }
    
    ret = ota_http_update_image();
    if (AT_RET_OK != ret) {
        at_rsp(AT_ERROR);
        at_rsp("Download ota bin fail.");
        return ret;
    }
       
    at_rsp(AT_OK);
    at_rsp(OTA_TEST_DOWNLOAD_OK);
    HAL_NVIC_SystemReset();
    return ret;
}   
               

static void ota_debug_message(int argc, char **argv)
{
    at_rsp("OTA firmware message %s.\r\n", OTA_FW_DB_MSG);
    return;
}

void ota_test(void *context)
{
    char  *argv[AT_SET_MAX_ARGC];
    int    argc;
    char  *item;
    
    at_cmd_t *cmd = (at_cmd_t *)context;

    argc = at_get_param(argv, cmd->set);
    if(!argc) {
        return;
    }
    at_parse_param_str(&argv[0]);
    item = argv[0];

    if(!strncmp(item, OTA_TEST_SHOW_CONFIG, strlen(OTA_TEST_SHOW_CONFIG))) {
        ota_show_config(argc, argv);
    }
    else if (!strncmp(item, OTA_TEST_START, strlen(OTA_TEST_START))) {
        ota_start(argc, argv);
    }
    else if (!strncmp(item, OTA_TEST_DEBUG_MSG, strlen(OTA_TEST_DEBUG_MSG))) {
        ota_debug_message(argc, argv);
    }
    else {
        at_rsp("Unkown command %s.", argv[0]);
        at_rsp(AT_ERROR);
    }
}

#if CONFIG_OTA_DUAL_IMAGES
static int ota_first_boot_after_update(ota_boot_param_t *boot_param)
{
    if (0xFFFFFFFF == boot_param->upg_addr) {
        return FALSE;
    }

    if ((boot_param->signature.sig.bits.type != OTA_BOOT_PARAM_SIG_OTA_BIN) 
            || (boot_param->signature.sig.bits.sig != OTA_DUAL_SIGNATURE)) {
        return FALSE;
    }

    return TRUE;
}

void ota_init_boot_params()
{
    ota_boot_param_t boot_param, boot_param_r;

    memset(&boot_param, 0, sizeof(boot_param));
    
    flash_read(FLASH_PARAMETER_1, (u8 *)&boot_param, sizeof(boot_param));
    if (TRUE != ota_first_boot_after_update(&boot_param)) {
        AT_DBG_MSG("Not reboot after update ota firmware");
        return;
    }

    if (boot_param.boot_addr != boot_param.upg_addr) {
        boot_param.upg_addr = 0xFFFFFFFF;
    }
    else {
        boot_param.upg_addr = 0xFFFFFFFF;
        boot_param.orig_addr = boot_param.boot_addr;
    }
    boot_param.wlan_len = 0xFFFFFFFF;
    flash_write(FLASH_PARAMETER_1, (u8 *)&boot_param, sizeof(boot_param));
    flash_read(FLASH_PARAMETER_1, (u8 *)&boot_param_r, sizeof(boot_param_r));
    if (memcmp(&boot_param, &boot_param_r, sizeof(boot_param))) {
        AT_DBG_ERR("Init boot params fail.");
    }
    return;
}

#endif

#else
void ota_init_boot_params()
{
}
#endif

