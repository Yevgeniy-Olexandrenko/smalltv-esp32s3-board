#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PSRAM_SEC_SIZE 512

typedef struct {
    uint8_t* buf;
    uint32_t sectors;
} psram_partition_t;

psram_partition_t* psram_partition_create(uint32_t sectors);
void psram_partition_delete(psram_partition_t* psram);

#ifdef __cplusplus
}
#endif
