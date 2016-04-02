#ifndef __LOCK_H__
#define __LOCK_H__

#include <stdint.h>

struct spinlock {
    uint16_t users;
    uint16_t ticket;
};

void lock(struct spinlock *lock);
void unlock(struct spinlock *lock);

void start_critical_section();
void end_critical_section();

#endif /* __LOCK_H__ */