#include "serial_port.h"
#include "interrupt.h"
#include "pit.h"
#include "memory_map.h"
#include "allocator.h"
#include "paging.h"
#include "slab.h"

void main(void) {
	serial_port_init();
	allocator_init();
	map_init();
	
    struct slabctl** mslab = slab_init(1000, 2);
    struct slabctl** mslab10 = slab_init(10, 2);

    for (int i = 0; i < 50000; ++i) {
        slab_allocate(mslab10);
        slab_allocate(mslab);
    }

	interrupt_init();
	idt_init();

	pit_init();
	set_interrupt_enable_flag();

	while(1);	
}
