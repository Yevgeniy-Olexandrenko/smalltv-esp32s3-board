#include <string.h>
#include <esp_heap_caps.h>
#include "psram_partition.h"

psram_partition_t* psram_partition_create(uint32_t sectors)
{
    // allocate buffer for partition data
    uint32_t bytes = sectors * PSRAM_SEC_SIZE;
    void* buf = heap_caps_malloc(bytes, MALLOC_CAP_SPIRAM);
    if (!buf) 
    {
        return NULL;
    }
    memset(buf, 0xFF, bytes);

    // allocate partition descriptor
    psram_partition_t* psram = heap_caps_malloc(sizeof(*psram), MALLOC_CAP_SPIRAM);
    if (!psram) 
    {
        heap_caps_free(buf);
        return NULL;
    }
    psram->buf = (uint8_t*)buf;
    psram->sectors = sectors;
    return psram;
}

void psram_partition_delete(psram_partition_t* psram)
{
    if (psram)
    {
        // free buffer, then descriptor
        heap_caps_free(psram->buf);
        heap_caps_free(psram);
    }
}
