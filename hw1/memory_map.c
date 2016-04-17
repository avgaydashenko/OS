#include "memory_map.h"
#include "initramfs.h"

size_t memory_map_size;
struct memory_map_entry memory_map[MMAP_SIZE];

void* init_ref;
void* init_ref_end;

static struct memory_map_entry kernel;

static void cut_last_elem(struct memory_map_entry elem) {
    if (memory_map[memory_map_size - 1].address >= elem.address) {
        if (memory_map[memory_map_size - 1].length + memory_map[memory_map_size - 1].address <= elem.address + elem.length) {
            --memory_map_size;
            return;
        }
        if (memory_map[memory_map_size - 1].address >= elem.address + elem.length) {
            return;
        }
        uint64_t end = memory_map[memory_map_size - 1].address + memory_map[memory_map_size - 1].length;
        memory_map[memory_map_size - 1].address = elem.address + elem.length;
        memory_map[memory_map_size - 1].length = end - memory_map[memory_map_size - 1].address;
    } else {
        if (elem.address >= memory_map[memory_map_size - 1].address + memory_map[memory_map_size - 1].length) {
            return;
        }
        if (memory_map[memory_map_size - 1].length + memory_map[memory_map_size - 1].address >= elem.address + elem.length) {
            uint64_t end = memory_map[memory_map_size - 1].address + memory_map[memory_map_size - 1].length;
            memory_map[memory_map_size - 1].length = elem.address - memory_map[memory_map_size - 1].address;

            memory_map[memory_map_size].address = elem.address + elem.length;
            memory_map[memory_map_size].length = end - memory_map[memory_map_size].address;
            memory_map[memory_map_size].type = memory_map[memory_map_size - 1].type;
            ++memory_map_size;
            return;
        }
        memory_map[memory_map_size - 1].length = elem.address - memory_map[memory_map_size].address;
    }
}

static void sort_mmap() {
    for (size_t i = 0; i < memory_map_size; ++i) {
        for (size_t j = i + 1; j < memory_map_size; ++j) {
            if (memory_map[i].address >= memory_map[j].address) {
                struct memory_map_entry k = memory_map[i];
                memory_map[i] = memory_map[j];
                memory_map[j] = k;
            }
        }
    }
}


void find_initranfs_mode(uint32_t count, uint32_t* addr, uint32_t* start, uint32_t* end) {
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t* pos = addr + i * 4;
        char* mod_start = (char *) (uint64_t)*pos, *mod_end = (char *) (uint64_t)*(pos + 1);

        if (mod_end - mod_start > 6) {
            mod_start[0] = '0';
            mod_start[1] = '7';
            mod_start[2] = '0';
            mod_start[3] = '7';
            mod_start[4] = '0';
            mod_start[5] = '1';

            *start = (uint32_t)(uint64_t) mod_start;
            *end = (uint32_t)(uint64_t) mod_end;
            init_ref = mod_start;
            init_ref_end = mod_end;
            return;
        }
    }
}

void get_memory_map() {
    memory_map_size = 0;

    kernel.address = (uint64_t)text_phys_begin;
    kernel.length = (uint64_t)bss_phys_end - kernel.address;
    kernel.type = 0;

    uint32_t flags = *(uint32_t *) (uint64_t) mboot_info;

    uint32_t mmap_length = *(uint32_t *) (uint64_t) (mboot_info + 44);
    uint32_t mmap_addr = *(uint32_t *) (uint64_t) (mboot_info + 48);

    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t start_del = 0, end_del = 0;

    struct memory_map_entry initramfs;
    if (((flags >> 3) & 1)) {
        mods_addr = *(uint32_t *) (uint64_t) (mboot_info + 24);
        mods_count = *(uint32_t *) (uint64_t) (mboot_info + 20);
        find_initranfs_mode(mods_count, (uint32_t*)(uint64_t) mods_addr, &start_del, &end_del);
    }

    initramfs.address = start_del;
    initramfs.length = end_del - start_del;
    initramfs.type = 0;

    uint32_t cnt_skip = 0;
    while (cnt_skip < mmap_length) {
        uint32_t size = *(uint32_t *)(uint64_t) (mmap_addr + cnt_skip);
        cnt_skip += 4;
        memory_map[memory_map_size].address =*(uint64_t*)(uint64_t)(mmap_addr + cnt_skip);
        memory_map[memory_map_size].length = *(uint64_t*)(uint64_t)(mmap_addr + cnt_skip + 8);
        memory_map[memory_map_size].type = *(uint32_t*)(uint64_t)(mmap_addr + cnt_skip + 16);
        cnt_skip += size;
        ++memory_map_size;

        cut_last_elem(kernel);
        if (end_del - start_del > 0) {
            cut_last_elem(initramfs);
        }

    }

    memory_map[memory_map_size] = kernel;
    ++memory_map_size;

    if (initramfs.length > 0) {
        memory_map[memory_map_size] = initramfs;
        ++memory_map_size;
    }

    sort_mmap();

    serial_port_write_line("Get memory map: successful.\n");
}

void print_memory_map() {

    serial_port_write_line("MEMORY MAP:\n");

    for (size_t i = 0; i < memory_map_size; ++i) {
        serial_port_write_num((unsigned long long) memory_map[i].address);
        serial_port_write_line(" - ");
        serial_port_write_num((unsigned long long int) (memory_map[i].address + memory_map[i].length - 1));
        serial_port_write_line(", ");
        
        if (memory_map[i].type == 1) 
            serial_port_write_line("available");
        else if (memory_map[i].type == 0)
            serial_port_write_line("kernel");
        else
            serial_port_write_line("reserved");

        serial_port_write_line("\n");
    }

    serial_port_write_line("Print memory map: successful.\n");
}