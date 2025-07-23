#pragma once

#include <esp_err.h>
#include "psram_partition.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t ff_diskio_register_psram(unsigned char pdrv, const psram_partition_t* psram);
uint8_t ff_diskio_get_pdrv_psram(const psram_partition_t* psram);
void ff_diskio_clear_pdrv_psram(const psram_partition_t* psram);

esp_err_t ff_diskio_write_sectors_psram(
    const psram_partition_t* psram, 
    const void* src,
    size_t start_sector, 
    size_t sector_count);

esp_err_t ff_diskio_read_sectors_psram(
    const psram_partition_t* psram, 
    void* dst,
    size_t start_sector, 
    size_t sector_count);

#ifdef __cplusplus
}
#endif
