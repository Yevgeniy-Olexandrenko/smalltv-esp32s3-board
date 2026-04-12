#include <diskio_impl.h>
#include <vfs_fat_internal.h>
#include "diskio_psram.h"
#include "vfs_fat_psram.h"

static const char* TAG = "vfs_fat_psram";

esp_err_t esp_vfs_fat_psram_mount(
    const char* base_path,
    const psram_partition_t* psram,
    const esp_vfs_fat_mount_config_t* mount_config)
{
    esp_err_t result = ESP_OK;
    const size_t workbuf_size = 4096;
    void *workbuf = NULL;

    if (psram == NULL || psram->buf == NULL || psram->sectors == 0)
    {
        ESP_LOGE(TAG, "PSRAM partition is corrupted");
        return ESP_ERR_INVALID_STATE;
    }

    // connect driver to FATFS
    BYTE pdrv = 0xFF;
    if (ff_diskio_get_drive(&pdrv) != ESP_OK) 
    {
        ESP_LOGD(TAG, "the maximum count of volumes is already mounted");
        return ESP_ERR_NO_MEM;
    }
    ESP_LOGD(TAG, "using pdrv=%i", pdrv);
    char drv[3] = {(char)('0' + pdrv), ':', 0};

    result = ff_diskio_register_psram(pdrv, psram);
    if (result != ESP_OK) 
    {
        ESP_LOGE(TAG, "ff_diskio_register_psram failed pdrv=%i, error - 0x(%x)", pdrv, result);
        goto fail;
    }

    FATFS *fs;
    result = esp_vfs_fat_register(base_path, drv, mount_config->max_files, &fs);
    if (result == ESP_ERR_INVALID_STATE) 
    {
        // it's okay, already registered with VFS
    } 
    else if (result != ESP_OK) 
    {
        ESP_LOGD(TAG, "esp_vfs_fat_register failed 0x(%x)", result);
        goto fail;
    }

    // try to mount partition
    FRESULT fresult = f_mount(fs, drv, 1);
    if (fresult != FR_OK) 
    {
        ESP_LOGW(TAG, "f_mount failed (%d)", fresult);
        if (!(fresult == FR_NO_FILESYSTEM && mount_config->format_if_mount_failed)) 
        {
            result = ESP_FAIL;
            goto fail;
        }
        workbuf = ff_memalloc(workbuf_size);
        if (workbuf == NULL) 
        {
            result = ESP_ERR_NO_MEM;
            goto fail;
        }
        size_t alloc_unit_size = esp_vfs_fat_get_allocation_unit_size(
                PSRAM_SEC_SIZE,
                mount_config->allocation_unit_size);
        ESP_LOGI(TAG, "Formatting FATFS partition, allocation unit size=%d", alloc_unit_size);
        fresult = f_mkfs(drv, FM_ANY | FM_SFD, alloc_unit_size, workbuf, workbuf_size);
        if (fresult != FR_OK) 
        {
            result = ESP_FAIL;
            ESP_LOGE(TAG, "f_mkfs failed (%d)", fresult);
            goto fail;
        }
        free(workbuf);
        workbuf = NULL;
        ESP_LOGI(TAG, "Mounting again");
        fresult = f_mount(fs, drv, 0);
        if (fresult != FR_OK) 
        {
            result = ESP_FAIL;
            ESP_LOGE(TAG, "f_mount failed after formatting (%d)", fresult);
            goto fail;
        }
    }
    return ESP_OK;

fail:
    free(workbuf);
    esp_vfs_fat_unregister_path(base_path);
    ff_diskio_unregister(pdrv);
    return result;
}

esp_err_t esp_vfs_fat_psram_unmount(const char* base_path, const psram_partition_t* psram)
{
    if (psram == NULL || psram->buf == NULL || psram->sectors == 0)
    {
        ESP_LOGE(TAG, "PSRAM partition is corrupted");
        return ESP_ERR_INVALID_STATE;
    }
    BYTE pdrv = ff_diskio_get_pdrv_psram(psram);
    if (pdrv == 0xff) 
    {
        return ESP_ERR_INVALID_STATE;
    }
    char drv[3] = {(char)('0' + pdrv), ':', 0};

    f_mount(0, drv, 0);
    ff_diskio_unregister(pdrv);
    ff_diskio_clear_pdrv_psram(psram);

    // release partition driver
    esp_err_t err = esp_vfs_fat_unregister_path(base_path);
    return err;
}
