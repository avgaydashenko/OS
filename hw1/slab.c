#include "slab.h"

static struct spinlock slab_lock;

void* get_page_adr(void* adr) {
    return (void*)((uint64_t)adr&(~(PAGE_SIZE - 1)));
}

uint64_t get_buffsize(struct slabctl* slab) {
    return (uint64_t)max(2, (int)align_up(slab->block_size, slab->alignment));
}

void* small_slab_get_buffer_addr(struct slabctl* slab, int id) {
    uint64_t start_page_adr = align_up((uint64_t)get_page_adr(slab), slab->alignment);
    uint64_t buffsize = get_buffsize(slab);
    return (void*)(start_page_adr + buffsize*id);
}

void* big_slab_get_buffer_addr(struct slabctl* slab, int id, int cnt_page) {
    uint64_t start_page_adr = align_up((uint64_t)get_page_adr(slab)-(cnt_page - 1)*PAGE_SIZE, slab->alignment);
    uint64_t buffsize = get_buffsize(slab);
    return (void*)(start_page_adr + buffsize*id);
}

size_t small_slab_cnt(struct slabctl* slab) {
    uint64_t start_page_adr = align_up((uint64_t)get_page_adr(slab), slab->alignment);
    uint64_t buffsize = get_buffsize(slab);
    return ((uint64_t)slab - start_page_adr)/buffsize;
}

size_t big_slab_cnt(struct slabctl* slab, int cnt_page) {
    uint64_t start_page_adr = align_up((uint64_t)get_page_adr(slab)-(cnt_page - 1)*PAGE_SIZE, slab->alignment);
    uint64_t buffsize = get_buffsize(slab);
    return ((uint64_t)slab - start_page_adr)/buffsize;
}

void* allocate(unsigned int size, unsigned int al) {
    if (al == 0) {
        al = 1;
    }
    if (size*8 <= PAGE_SIZE) {
        void* buf = get_page(0);
        struct slabctl* slab_control = (struct slabctl*)((char*)buf + PAGE_SIZE - sizeof(struct slabctl));
        slab_control->alignment = al;
        slab_control->block_size = size;
        slab_control->cnt_ref = small_slab_cnt(slab_control);
        slab_control->head=0;

        descriptors[get_phys_address((virt_t)((char*)buf))].slab = slab_control;

        size_t cnt = small_slab_cnt(slab_control);

        for (uint16_t i = 0; i < cnt; ++i) {
            uint16_t * curbuff = small_slab_get_buffer_addr(slab_control, i);
            *curbuff = i + (uint16_t)1;
        }
        return slab_control;
    } else {
        void* buf = get_page(2);

        struct slabctl* slab_control = (struct slabctl*)((char*)buf + CNT_PAGES * PAGE_SIZE - sizeof(struct slabctl));
        for (int i = 0; i < CNT_PAGES; ++i) {
            descriptors[get_phys_address((virt_t)((char*)buf + i*PAGE_SIZE))].slab = slab_control;
        }

        slab_control->alignment = al;
        slab_control->block_size = size;
        slab_control->cnt_ref = big_slab_cnt(slab_control, CNT_PAGES);
        slab_control->head=0;

        size_t cnt = big_slab_cnt(slab_control, CNT_PAGES);
        for (uint16_t i = 0; i < cnt; ++i) {
            uint16_t * curbuff = big_slab_get_buffer_addr(slab_control, i, CNT_PAGES);
            *curbuff = i + (uint16_t)1;
        }
        return slab_control;
    }
}

void* allocate_block(struct slabctl* slab) {
    if (slab->block_size * 8 > PAGE_SIZE) {
        void* res = big_slab_get_buffer_addr(slab, slab->head, CNT_PAGES);
        slab->head = *((uint16_t*)res);
        slab->cnt_ref--;
        return res;
    }
    void* res = small_slab_get_buffer_addr(slab, slab->head);
    slab->head = *((uint16_t*)res);
    slab->cnt_ref--;
    return res;
}

void slab_free(void *addr) {
    void* pg_addr = get_page_adr(addr);
    struct slabctl* sl = descriptors[get_phys_address((virt_t)pg_addr)].slab;
    int id;
    if (sl->block_size * 8 > PAGE_SIZE) {
        uint64_t start_page_adr = align_up((uint64_t)get_page_adr(sl)-(CNT_PAGES - 1)*PAGE_SIZE, sl->alignment);
        uint64_t buffsize = get_buffsize(sl);

        id = (int)(((uint64_t)addr - start_page_adr)/buffsize);
    } else {
        uint64_t start_page_adr = align_up((uint64_t)get_page_adr(sl), sl->alignment);
        uint64_t buffsize = get_buffsize(sl);

        id = (int)(((uint64_t)addr - start_page_adr)/buffsize);
    }
    *((uint16_t*)addr) = sl->head;
    sl->head = id;
    sl->cnt_ref++;
    if (sl->cnt_ref == 1) {
        sl->next = *((struct slabctl**)sl->slab_list_head);
        *((struct slabctl**)sl->slab_list_head) = sl;
    }

    serial_port_write_line("Free slab: successful.\n");
}

void* slab_allocate (struct slabctl** slab_sys) {
    struct slabctl** head = slab_sys;
    void* ret = allocate_block(*head);
    if ((*head)->cnt_ref == 0) {
        if ((*head)->next == (*head)) {
            *head = allocate((*head)->block_size, (*head)->alignment);
            (*head)->slab_list_head = head;
            (*head)->next = *head;
        } else {
            (*head) = (*head)->next;
        }
    }

    return ret;
}

struct slabctl** slab_init (unsigned int size, unsigned int al) {

    lock(&slab_lock);

    if (heads == NULL) {
        heads = get_page(0);
    }
    struct slabctl** head = heads;
    heads = (void*)((uint64_t) heads + sizeof(struct slabctl**));
    *head = (struct slabctl*)allocate(size, al);
    (*head)->slab_list_head = head;
    (*head)->next = *head;

    serial_port_write_line("Initialise slab: successful.\n");

    unlock(&slab_lock);

    return head;
}