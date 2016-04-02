#include "lock.h"
#include "util.h"
#include "thread.h"
#include "interrupt.h"

void lock(struct spinlock *lock)
{
    const uint16_t ticket = __sync_fetch_and_add(&lock->users, 1);

    while (lock->ticket != ticket) {
        barrier();
        yield();
    }
    __sync_synchronize();
}

void unlock(struct spinlock *lock)
{
    __sync_synchronize();
    __sync_add_and_fetch(&lock->ticket, 1);
}

volatile int critical_section_level = 0;

void start_critical_section() {
    clear_interrupt_enable_flag();
    __sync_fetch_and_add(&critical_section_level, 1);
}

void end_critical_section() {
    if (__sync_fetch_and_add(&critical_section_level, -1) == 1) {
        set_interrupt_enable_flag();
    }
}
