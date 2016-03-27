#ifndef __MEMORY_MAP_H__
#define __MEMORY_MAP_H__

#include <stdint.h>
#include <stddef.h>

#include "serial_port.h"
#include "util.h"

#define MMAP_SIZE 32

struct memory_map_entry {
    uint64_t address;
    uint64_t length;
    uint32_t type;
};

extern const uint32_t mboot_info;

extern char text_phys_begin[];
extern char bss_phys_end[];

extern size_t memory_map_size;
extern struct memory_map_entry memory_map[];

void get_memory_map();
void print_memory_map();

#endif /* __MEMORY_MAP_H__ */