#include "serial_port.h"
#include "interrupt.h"
#include "pit.h"
#include "memory_map.h"
#include "allocator.h"
#include "paging.h"
#include "slab.h"
#include "thread.h"
#include "test.h"
#include "file_system.h"
#include "initramfs.h"

void main(void) {

	start_critical_section();

	serial_port_init();
	allocator_init();
	map_init();
	malloc_small_init();

    thread_pool_init();

	interrupt_init();
	idt_init();

	pit_init();
	set_interrupt_enable_flag();

    file_system_init();
    initramfs_to_fs();
    file_system_print();

	end_critical_section();

	while(1);	
}
