#ifndef __SLAB_H__
#define __SLAB_H__

#include <stddef.h>

#include "paging.h"
#include "serial_port.h"
#include "memory_map.h"
#include "allocator.h"

#define CNT_PAGES 4

struct slabctl {
    uint16_t block_size;
    uint16_t alignment;
    uint16_t head;
    uint16_t cnt_ref;
    struct slabctl* next;
    void* slab_list_head;
};

struct bufctl {
    void* buf_adr;
    struct bufctl* next_ctl;
    struct slab* slab_ctl;
};

void* heads;

struct slabctl** slab_init (unsigned int size, unsigned int al);

void* slab_allocate (struct slabctl** slab_sys);
void slab_free(void *addr);

#endif /* __SLAB_H__ */