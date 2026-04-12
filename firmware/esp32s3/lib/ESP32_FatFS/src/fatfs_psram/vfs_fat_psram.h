#pragma once

#include <esp_vfs_fat.h>
#include "psram_partition.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t esp_vfs_fat_psram_mount(
    const char* base_path,
    const psram_partition_t* psram,
    const esp_vfs_fat_mount_config_t* mount_config);

esp_err_t esp_vfs_fat_psram_unmount(const char* base_path, const psram_partition_t* psram);

#ifdef __cplusplus
}
#endif
