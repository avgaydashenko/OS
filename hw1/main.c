#include "serial_port.h"
#include "interrupt.h"
#include "pit.h"

void main(void) {
	serial_port_init();

	serial_port_write_line("First part done\n");

	interrupt_init();
	idt_init();

	serial_port_write_line("Second part done\n");

	pit_init();
	set_interrupt_enable_flag();

	serial_port_write_line("Third part done\n");

	while(1);	
}
