#ifndef _INCLUDED_MEMORY_H_
#define _INCLUDED_MEMORY_H_

#define EFLAGS_AC_BIT       0x00040000
#define CR0_CACHE_DISABLE   0x60000000

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MEMORY_INFO_STATUS_FREE     0x00000000
#define MEMORY_INFO_STATUS_USED     0x00000001
#define MEMORY_INFO_STATUS_NODATA   0x00000002
#define MEMORY_INFO_STATUS_END      0x00000003

#define MEMORY_MANAGEMENT_DATA_SIZE 1024

typedef struct {
    void *base_addr;
    uint32_t size;
    uint32_t status;
}memory_info;

typedef struct  {
    uint32_t end_point;
    memory_info data[MEMORY_MANAGEMENT_DATA_SIZE];
}memory_data;

void memory_management_init(memory_data* memory_data);
void* memory_allocate(memory_data* memory_data, uint32_t size);
bool memory_free(memory_data* memory_data, void *address);

uint32_t memtest(uint32_t start, uint32_t end);
// uint32_t memtest_sub(uint32_t start, uint32_t end);

void init_memory(memory_data* memory_data);

extern char _bss_end, _text_start, _kernel_start, _kernel_end;

uint32_t get_size_of_kernel(void);




#endif

