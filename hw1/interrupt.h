#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <stdint.h>
#include "ioport.h"

#define MASTER_PIC 0x20
#define SLAVE_PIC  0xA0

#define MASTER_PIC_COMMAND (MASTER_PIC)
#define MASTER_PIC_DATA    (MASTER_PIC + 1)
#define SLAVE_PIC_COMMAND  (SLAVE_PIC)
#define SLAVE_PIC_DATA     (SLAVE_PIC + 1)

#define LEGACY_PIC_INIT 0b00010001
#define LEGACY_PIC_MODE 0b00000001

#define LEGACY_PIC_EOI 0x20

#define SLAVE_TO_MASTER_PORT 0b00000100
#define MASTER_TO_SLAVE_PORT 0b00000010

#define ICV_2_MASTER 0x20
#define ICV_2_SLAVE  0x28

#define INTERRUPT_COUNT 0x100

#define INTERRUPT_FLAG_PRESENT 0b10000000
#define INTERRUPT_FLAG_INT64   0b00001110

struct idt_ptr {
	uint16_t size;
	uint64_t base;
} __attribute__((packed));

struct idt_descriptor {
	uint16_t offset_low;
	uint16_t seg_selector;
	uint8_t reserved;
	uint8_t flag;
	uint16_t offset_middle;
	uint32_t offset_high;
	uint32_t reserved2;
} __attribute__((packed));

static inline void set_idt(const struct idt_ptr *ptr)
{ __asm__ volatile ("lidt (%0)" : : "a"(ptr)); }


static inline void clear_interrupt_enable_flag()
{ __asm__ volatile ("cli"); }

static inline void set_interrupt_enable_flag()
{ __asm__ volatile ("sti"); }

void interrupt_init();
void send_end_of_interrupt(uint8_t is_master);

void idt_init();
void descriptor_set(uint8_t id, uint64_t handler, uint8_t flags);

uint64_t handler_empty;
uint64_t handler_pop;

#endif /*__INTERRUPT_H__*/
