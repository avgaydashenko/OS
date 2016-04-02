#include "allocator.h"

static struct spinlock allocator_lock;

void allocator_init() {

    get_memory_map();
    print_memory_map();

    size_t max_mem_size = ((memory_map[memory_map_size - 1].address + memory_map[memory_map_size - 1].length));

    boot_size = (max_mem_size/(2<<20))*PAGE_SIZE*2;
    size_t descriptors_size = max_mem_size/PAGE_SIZE;

    max_order = 1;
    while ((1ll<<max_order) <= (int)descriptors_size) {
        ++max_order;
    }

    size_t head_size = max_order * sizeof(head[0]);

    boot_size += head_size + descriptors_size;

    for (size_t i = 0; i < memory_map_size; ++i) {
        if (memory_map[i].type == 1) {
            if (memory_map[i].length >= boot_size) {
                boot_mem = va(memory_map[i].address);
                memory_map[i].address += boot_size;
                memory_map[i].length -= boot_size;
                break;
            }
        }
    }

    head = get_mem(head_size, 0);
    descriptors = get_mem(descriptors_size, 0);

    for (size_t i = 0; i < (size_t)max_order; ++i) {
        head[i] = -1;
    }

    for (size_t i = 0; i < memory_map_size; ++i) {
        if (memory_map[i].type == 1) {
            while (memory_map[i].length >= PAGE_SIZE) {
                uint64_t start_addr = ((memory_map[i].address + 1) / PAGE_SIZE) * PAGE_SIZE;
                if (start_addr + PAGE_SIZE > memory_map[i].address + memory_map[i].length) {
                    memory_map[i].length = PAGE_SIZE - 1;
                    continue;
                }
                descriptors[start_addr/PAGE_SIZE].in_list = 1;
                descriptors[start_addr/PAGE_SIZE].order = 0;
                free_page(va(start_addr), 0);
                memory_map[i].address = start_addr + PAGE_SIZE;
                memory_map[i].length -= PAGE_SIZE;
            }
        }
    }

    serial_port_write_line("Initialise allocator: successful.\n");
}

void add_page(int id, int k) {
    descriptors[id].in_list = 1;
    descriptors[id].order = k;

    if (head[k] == -1) {
        head[k] = id;
        descriptors[id].next_page_id = id;
        descriptors[id].prev_page_id = id;
    } else {
        descriptors[id].next_page_id = head[k];
        descriptors[id].prev_page_id = descriptors[head[k]].prev_page_id;
        descriptors[descriptors[head[k]].prev_page_id].next_page_id = id;
        descriptors[head[k]].prev_page_id = id;
    }
}

void rem_page(int id, int k) {
    if (descriptors[id].prev_page_id == id) {
        head[k] = -1;
    } else {
        if (head[k] == id) {
            head[k] = descriptors[id].next_page_id;
        }
        descriptors[descriptors[id].next_page_id].prev_page_id = descriptors[id].prev_page_id;
        descriptors[descriptors[id].prev_page_id].next_page_id = descriptors[id].next_page_id;
    }
    descriptors[id].in_list = 0;
}

void* get_page(int k) {

    lock(&allocator_lock);

    int lv = k;
    while (head[lv] == -1 && lv < max_order) {
        ++lv;
    }
    if (lv == max_order) {
        unlock(&allocator_lock);
        return 0;
    }
    for (; lv > k; --lv) {
        int num_p = head[lv];
        rem_page(num_p, lv);
        add_page(num_p, lv - 1);
        add_page(num_p ^ (1 << (lv - 1)), lv - 1);
    }
    int val = head[k];
    rem_page(head[k], k);

    unlock(&allocator_lock);
    return va(val * (uint64_t)PAGE_SIZE);
}

void free_page(void* page_addr, int k) {

    lock(&allocator_lock);

    int id = pa(page_addr)/PAGE_SIZE;
    while (1) {
        int pid = id ^ (1 << k);
        if (descriptors[pid].in_list == 1 && descriptors[pid].order == k) {
            rem_page(pid, k);
            id = min(pid, id);
            ++k;
        } else {
            add_page(id, k);
            break;
        }
    }

    unlock(&allocator_lock);
}

void* get_mem(size_t mem_size, size_t alignment) {
    char* res = boot_mem;
    if (alignment != 0) {
        res = (char *) ((((uint64_t)res + alignment - 1) / (alignment)) * alignment);
    }
    boot_size -= ((uint64_t)res - (uint64_t)boot_mem) + mem_size;
    boot_mem = res + mem_size;

    for (size_t i = 0; i < mem_size; ++i) {
        res[i] = 0;
    }

    return res;
}