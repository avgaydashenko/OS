#include "interrupt.h"
#include "memory.h"
#include "util.h"

#define MASK 0b11111111

static struct idt_descriptor descriptor[INTERRUPT_COUNT];
static struct idt_ptr idt;

void interrupt_init() {

	out8(MASTER_PIC_COMMAND, LEGACY_PIC_INIT);
	out8(MASTER_PIC_DATA, ICV_2_MASTER);
	out8(MASTER_PIC_DATA, bit(MASTER_TO_SLAVE_PORT));	
	out8(MASTER_PIC_DATA, LEGACY_PIC_MODE);

	out8(SLAVE_PIC_COMMAND, LEGACY_PIC_INIT);
	out8(SLAVE_PIC_DATA, ICV_2_SLAVE);
	out8(SLAVE_PIC_DATA, MASTER_TO_SLAVE_PORT);
	out8(SLAVE_PIC_DATA, LEGACY_PIC_MODE);

	out8(MASTER_PIC_DATA, MASK ^ bit(0));
	out8(SLAVE_PIC_DATA, MASK);
}

void send_end_of_interrupt(uint8_t is_master) {
	
	if (!is_master)
		out8(SLAVE_PIC_COMMAND, LEGACY_PIC_EOI);

	out8(MASTER_PIC_COMMAND, LEGACY_PIC_EOI);
}

void idt_init() {

	idt.size = sizeof(descriptor) - 1;
	idt.base = (uint64_t) &descriptor;

	for (int i = 0; i < INTERRUPT_COUNT; i++)
		descriptor_set(i, (uint64_t) &handler_empty,
			INTERRUPT_FLAG_PRESENT | INTERRUPT_FLAG_INT64);

	uint8_t errors[] = {8, 10, 11, 12, 13, 14, 17, 30};
	for (int i = 0; i < 8; i++)
		descriptor_set(errors[i], (uint64_t) &handler_pop,
			INTERRUPT_FLAG_PRESENT | INTERRUPT_FLAG_INT64);	
	
	set_idt(&idt);
}

void descriptor_set(uint8_t id, uint64_t handler, uint8_t flags) {	
	descriptor[id].offset_low = get_bits(handler, 0, 16);
	descriptor[id].seg_selector = KERNEL_CODE;
	descriptor[id].reserved = 0;
	descriptor[id].flag = flags;
	descriptor[id].offset_middle = get_bits(handler, 16, 32);
	descriptor[id].offset_high = get_bits(handler, 32, 64);
	descriptor[id].reserved2 = 0;
}