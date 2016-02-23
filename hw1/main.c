#include "serial_port.h"
#include "interrupt.h"
#include "pit.h"

void main(void) {
	serial_port_init();

	interrupt_init();
	idt_init();

	pit_init();
	set_interrupt_enable_flag();

	while(1);	
}
