#include "serial_port.h"
#include "interrupt.h"
#include "pit.h"
#include "memory_map.h"
#include "allocator.h"
#include "paging.h"
#include "slab.h"
#include "thread.h"
#include "test.h"

void main(void) {

	start_critical_section();

	serial_port_init();
	allocator_init();
	map_init();

    thread_pool_init();

    test_switch_and_arg();
    test_finish();
    test_lock();
    test_join();

	interrupt_init();
	idt_init();

	pit_init();
	set_interrupt_enable_flag();

	end_critical_section();

    //test_timer_interrupt();
    test_slab();

	while(1);	
}
