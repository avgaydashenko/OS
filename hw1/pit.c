#include "pit.h"

extern uint64_t handler_pit;

void pit_init() {

	out8(PIT_PORT_CONTROL, COMMAND_FOR_CONTROL);
	out8(PIT_PORT_DATA, get_bits(PARTITION_FACTOR, 0, 8));
	out8(PIT_PORT_DATA, get_bits(PARTITION_FACTOR, 8, 16));


	descriptor_set(ICV_2_MASTER, handler_pit,
		INTERRUPT_FLAG_PRESENT | INTERRUPT_FLAG_INT64);
}

void pit_handler() {

	serial_port_write_line("tick");	
	send_end_of_interrupt(1);
}