#include "memory_map.h"

size_t memory_map_size;
struct memory_map_entry memory_map[MMAP_SIZE];

static struct memory_map_entry kernel;

static void cut_last_elem() {
    if (memory_map[memory_map_size - 1].address >= kernel.address) {
        if (memory_map[memory_map_size - 1].length + memory_map[memory_map_size - 1].address <= kernel.address + kernel.length) {
            --memory_map_size;
            return;
        }
        if (memory_map[memory_map_size - 1].address >= kernel.address + kernel.length) {
            return;
        }
        uint64_t end = memory_map[memory_map_size - 1].address + memory_map[memory_map_size - 1].length;
        memory_map[memory_map_size - 1].address = kernel.address + kernel.length;
        memory_map[memory_map_size - 1].length = end - memory_map[memory_map_size - 1].address;
    } else {
        if (kernel.address >= memory_map[memory_map_size - 1].address + memory_map[memory_map_size - 1].length) {
            return;
        }
        if (memory_map[memory_map_size - 1].length + memory_map[memory_map_size - 1].address >= kernel.address + kernel.length) {
            uint64_t end = memory_map[memory_map_size - 1].address + memory_map[memory_map_size - 1].length;
            memory_map[memory_map_size - 1].length = kernel.address - memory_map[memory_map_size - 1].address;

            memory_map[memory_map_size].address = kernel.address + kernel.length;
            memory_map[memory_map_size].length = end - memory_map[memory_map_size].address;
            memory_map[memory_map_size].type = memory_map[memory_map_size - 1].type;
            ++memory_map_size;
            return;
        }
        memory_map[memory_map_size - 1].length = kernel.address - memory_map[memory_map_size].address;
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

void get_memory_map() {
    memory_map_size = 0;

    kernel.address = (uint64_t)text_phys_begin;
    kernel.length = (uint64_t)bss_phys_end - kernel.address;
    kernel.type = 0;
    
    uint32_t mmap_length = *(uint32_t *) (uint64_t) (mboot_info + 44);
    uint32_t mmap_address = *(uint32_t *) (uint64_t) (mboot_info + 48);

    uint32_t cnt_skip = 0;
    while (cnt_skip < mmap_length) {
        uint32_t size = *(uint32_t *)(uint64_t) (mmap_address + cnt_skip);
        cnt_skip += 4;
        memory_map[memory_map_size].address =*(uint64_t*)(uint64_t)(mmap_address + cnt_skip);
        memory_map[memory_map_size].length = *(uint64_t*)(uint64_t)(mmap_address + cnt_skip + 8);
        memory_map[memory_map_size].type = *(uint32_t*)(uint64_t)(mmap_address + cnt_skip + 16);
        cnt_skip += size;
        ++memory_map_size;

        cut_last_elem();
    }

    memory_map[memory_map_size] = kernel;
    ++memory_map_size;

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