#ifndef __PIT_H__
#define __PIT_H__

#include "ioport.h"
#include "interrupt.h"
#include "serial_port.h"
#include "memory.h"

#define PIT_PORT_CONTROL 0x43
#define PIT_PORT_DATA	 0x40

#define PARTITION_FACTOR 0b1111111111111111

#define COMMAND_FOR_CONTROL 0b00110100

void pit_init();
void pit_handler();

uint64_t handler_pit;

#endif /* __PIT_H__ */