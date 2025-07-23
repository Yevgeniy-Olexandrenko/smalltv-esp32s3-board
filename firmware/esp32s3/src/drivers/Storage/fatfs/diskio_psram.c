#include <string.h>
#include <diskio_impl.h>
#include <ffconf.h>
#include <ff.h>
#include <esp_log.h>
#include "diskio_psram.h"

static const char* TAG = "diskio_psram";

const psram_partition_t* ff_psram_handles[FF_VOLUMES] = { NULL };

DSTATUS ff_psram_initialize (BYTE pdrv)
{
    const psram_partition_t* psram = ff_psram_handles[pdrv];
    assert(psram);
    return psram->buf ? RES_OK : STA_NOINIT;
}

DSTATUS ff_psram_status (BYTE pdrv)
{
    const psram_partition_t* psram = ff_psram_handles[pdrv];
    assert(psram);
    return psram->buf ? RES_OK : STA_NOINIT;
}

DRESULT ff_psram_read (BYTE pdrv, BYTE* buff, DWORD sector, UINT count)
{
    const psram_partition_t* psram = ff_psram_handles[pdrv];
    assert(psram);
    if (!psram->buf || sector + count > psram->sectors)
    {
        ESP_LOGE(TAG, "ff_psram_read failed");
        return RES_ERROR;
    }
    memcpy(buff, psram->buf + sector * PSRAM_SEC_SIZE, count * PSRAM_SEC_SIZE);
    return RES_OK;
}

DRESULT ff_psram_write (BYTE pdrv, const BYTE* buff, DWORD sector, UINT count)
{
    const psram_partition_t* psram = ff_psram_handles[pdrv];
    assert(psram);
    if (!psram->buf || sector + count > psram->sectors)
    {
        ESP_LOGE(TAG, "ff_psram_write failed");
        return RES_ERROR;
    }
    memcpy(psram->buf + sector * PSRAM_SEC_SIZE, buff, count * PSRAM_SEC_SIZE);
    return RES_OK;
}

DRESULT ff_psram_ioctl (BYTE pdrv, BYTE cmd, void* buff)
{
    const psram_partition_t* psram = ff_psram_handles[pdrv];
    assert(psram);
    switch(cmd) {
        case CTRL_SYNC:
            return RES_OK;
        case GET_SECTOR_COUNT:
            *((DWORD*) buff) = psram->sectors;
            return RES_OK;
        case GET_SECTOR_SIZE:
            *((WORD*) buff) = PSRAM_SEC_SIZE;
            return RES_OK;
        case GET_BLOCK_SIZE:
            return RES_ERROR;
    }
    return RES_ERROR;
}

esp_err_t ff_diskio_register_psram (unsigned char pdrv, const psram_partition_t* psram)
{
    if (pdrv >= FF_VOLUMES) 
    {
        return ESP_ERR_INVALID_ARG;
    }
    static const ff_diskio_impl_t psram_impl = 
    {
        .init   = &ff_psram_initialize,
        .status = &ff_psram_status,
        .read   = &ff_psram_read,
        .write  = &ff_psram_write,
        .ioctl  = &ff_psram_ioctl
    };
    ff_diskio_register(pdrv, &psram_impl);
    ff_psram_handles[pdrv] = psram;
    return ESP_OK;
}

uint8_t ff_diskio_get_pdrv_psram (const psram_partition_t* psram)
{
    for (int i = 0; i < FF_VOLUMES; i++) 
    {
        if (psram == ff_psram_handles[i]) return i;
    }
    return 0xff;
}

void ff_diskio_clear_pdrv_psram (const psram_partition_t* psram)
{
    for (int i = 0; i < FF_VOLUMES; i++) 
    {
        if (psram == ff_psram_handles[i])
        {
            ff_psram_handles[i] = NULL;
            break;
        }
    }
}

esp_err_t ff_diskio_write_sectors_psram (
    const psram_partition_t* psram, 
    const void* src,
    size_t start_sector, 
    size_t sector_count)
{
    uint8_t pdrv = ff_diskio_get_pdrv_psram(psram);
    if (pdrv >= FF_VOLUMES) 
    {
        return ESP_ERR_INVALID_ARG;
    }
    if (ff_psram_write(pdrv, src, start_sector, sector_count) != RES_OK)
    {
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t ff_diskio_read_sectors_psram (
    const psram_partition_t* psram, 
    void* dst,
    size_t start_sector, 
    size_t sector_count)
{
    uint8_t pdrv = ff_diskio_get_pdrv_psram(psram);
    if (pdrv >= FF_VOLUMES) 
    {
        return ESP_ERR_INVALID_ARG;
    }
    if (ff_psram_read(pdrv, dst, start_sector, sector_count) != RES_OK)
    {
        return ESP_FAIL;
    }
    return ESP_OK;
}
